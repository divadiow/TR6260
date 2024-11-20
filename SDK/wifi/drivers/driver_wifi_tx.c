#include "system.h"
#include "utils/includes.h"
#include "utils/common.h"
#include "../common/wpa_common.h"
#include "../common/ieee802_11_common.h"
#include "../common/eapol_common.h"
#include "utils/eloop.h"
#include "utils/wpa_debug.h"
#include "driver_wifi.h"
#include "driver_wifi_scan.h"
#include "driver_wifi_debug.h"
#include "nrc-wim-types.h"
//#include "lwip/pbuf.h"

#ifdef CONFIG_NO_STDOUT_DEBUG
#ifdef wpa_printf
#undef wpa_printf
#if 0
static void wpa_printf(int level, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	system_vprintf(fmt, ap);
	system_printf("\n");
    va_end(ap);
}
#else
#define wpa_printf(args...) do { } while (0)
#endif
#endif
#endif

static int nrc_add_ccmp_hdr(struct nrc_wpa_key *key, uint8_t* pos)
{
	if (!key)
		return 0;

	uint64_t pn = key->tsc;

	*pos++ = pn;		// PN 0
	*pos++ = pn >> 8;	// PN 1
	*pos++ = 0;		// RSV
	*pos++ = 0x20 | key->ix << 6;
	*pos++ = pn >> 16;	// PN 2
	*pos++ = pn >> 24;	// PN 3
	*pos++ = pn >> 32;	// PN 4
	*pos++ = pn >> 40;	// PN 5

	key->tsc++;

	return 8;
}

static int nrc_add_tkip_hdr(struct nrc_wpa_key *key, uint8_t* pos)
{
	if (!key)
		return 0;

	// Upper 32 bits of tsc for IV32 and lower 16 bits for IV16
	uint16_t iv16 = key->tsc & 0xFFFF;
	uint32_t iv32 = key->tsc >> 32;

	if (++iv16 == 0)
		iv32++;

	*pos++ = iv16 >> 8;	// TSC 1
	*pos++ = ((iv16 >> 8) | 0x20) & 0x7f;	// WEP Seed
	*pos++ = iv16 & 0xFF;	// TSC 0

	*pos++ = (key->ix << 6) | (1 << 5);	// Rsv|EXT IV|Key ID

	*pos++ = iv32;	//TSC 2
	*pos++ = iv32 >> 8;	//TSC 3
	*pos++ = iv32 >> 16;	//TSC 4
	*pos++ = iv32 >> 24;	//TSC 5

	key->tsc = iv32;
	key->tsc <<= 32;
	key->tsc |= iv16;

	return 8;
}

static bool is_weak_wep(struct nrc_wpa_key *key, uint32_t iv)
{
	uint8_t t = 0;
	if ((iv & 0xff00) == 0xff00) {
		uint8_t t = (iv >> 16) & 0xff;
		if (t >= 3 && t < 3 + key->key_len)
			return true;
	}
	return false;
}

static int nrc_add_wep_hdr(struct nrc_wpa_key *key, uint8_t* pos)
{
	if (!key)
		return 0;

	uint32_t iv = key->tsc;

	iv++;
	if (is_weak_wep(key, iv))
		iv += 0x100;

	*pos++ = (iv >> 16) & 0xff;
	*pos++ = (iv >> 8) & 0xff;
	*pos++ = iv & 0xff;
	*pos++ = key->ix << 6;

	key->tsc = iv;

	return 4;
}

int nrc_add_sec_hdr(struct nrc_wpa_key *key, uint8_t *pos)
{
	int sec = 0, cipher = 0;

	if (!key || key->cipher == WIM_CIPHER_TYPE_NONE)
		return 0;

	cipher = key->cipher;
	switch (cipher) {
		case WIM_CIPHER_TYPE_CCMP:
			sec = nrc_add_ccmp_hdr(key, pos);
			break;
		case WIM_CIPHER_TYPE_TKIP:
			sec = nrc_add_tkip_hdr(key, pos);
			break;
		case WIM_CIPHER_TYPE_WEP40:
		case WIM_CIPHER_TYPE_WEP104:
			sec = nrc_add_wep_hdr(key, pos);
			break;
		default:
			wpa_printf(MSG_ERROR, "Unsupp cipher (type: %d)", cipher);
	}
	return sec;
}

static SYS_BUF * alloc_sys_buf(int hif_len) {
#ifndef NRC7291_SDK_DUAL_CM3
	const int POOL_TX = 1;
	return sys_buf_alloc(POOL_TX, hif_len);
#else
	return  mbx_alloc_sys_buf_hif_len(hif_len);
#endif
}

SYS_BUF * alloc_sys_buf_try(int hif_len, int nTry) {
	SYS_BUF * buf = NULL;
	const int LMAC_ALLOC_SYS_BUF_DELAY = 5; // ms

	while (nTry--) {
		buf = alloc_sys_buf(hif_len);
		if (buf)
			break;
		vTaskDelay(pdMS_TO_TICKS(LMAC_ALLOC_SYS_BUF_DELAY));
	}
	return buf;
}

static int copy_to_sysbuf(SYS_BUF *to, uint8_t* from, uint16_t remain) {
	uint8_t *pos = from;
	int len = SYS_BUF_LENGTH(to) - sizeof(LMAC_TXHDR) - sizeof(PHY_TXVECTOR);
	memcpy(&to->payload, pos, len);
	pos += len; remain -= len;
	while ((to = SYS_BUF_LINK(to)) && remain > 0) {
		len = SYS_BUF_LENGTH(to) - sizeof(LMAC_TXHDR);
		memcpy(&to->more_payload, pos, min(len, remain));
		pos += len;
		remain -= len;
	}
	return pos - from;
}

static uint8_t get_ac(SYS_BUF *buffer) {
	GenericMacHeader *gmh = &TX_MAC_HDR(buffer);
	uint8_t ac = ACI_BE;  //AC_BK(0), AC_BE(1), AC_VI(2), AC_VO93)

	switch (gmh->type) {
		case FC_PV0_TYPE_DATA:
		if (gmh->subtype == FC_PV0_TYPE_DATA_QOS_DATA ||
			gmh->subtype == FC_PV0_TYPE_DATA_QOS_NULL)
			return TX_MAC_HDR(buffer).qos_tid;
		break;
		case FC_PV0_TYPE_MGMT:
		return ACI_VO;
		break;
	}
	return ACI_BE;
}

int nrc_raw_transmit(struct nrc_wpa_if* intf, uint8_t *frm, const uint16_t len,
				const int ac)
{
	const int nAllocTry = 10;
	int hif_len = len + sizeof(FRAME_HDR);
	SYS_BUF *buffer = NULL;
	uint8_t cipher = WIM_CIPHER_TYPE_NONE;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) frm;
	uint8_t *pos = (uint8_t *) (hdr + 1);

	buffer = alloc_sys_buf_try(hif_len, nAllocTry);

	if (!buffer)
		return -1;

	if (hdr->frame_control & WLAN_FC_ISWEP) {
		struct nrc_wpa_key *wpa_key = nrc_wpa_get_key(intf, hdr->addr1);
		if((hdr->frame_control & IEEE80211_FC(WLAN_FC_TYPE_DATA,WLAN_FC_STYPE_QOS_DATA)) == IEEE80211_FC(WLAN_FC_TYPE_DATA,WLAN_FC_STYPE_QOS_DATA))
			pos += 2;
        if (wpa_key && WIM_CIPHER_TYPE_INVALID != (int8_t)wpa_key->cipher)
		{    
			cipher = wpa_key->cipher;
		}
		
		nrc_add_sec_hdr(wpa_key, pos);
	}
	copy_to_sysbuf(buffer, frm, len);
	memset( &HIF_HDR(buffer) , 0 , sizeof(HIF_HDR) );

	HIF_HDR(buffer).type        = HIF_TYPE_FRAME;
	HIF_HDR(buffer).len         = hif_len;
	HIF_HDR(buffer).vifindex    = intf->vif_id;

	FRAME_HDR(buffer).info.tx.tlv_len    = 0;
	FRAME_HDR(buffer).info.tx.cipher     = cipher;
	FRAME_HDR(buffer).flags.tx.ac = ac;

#ifndef NRC7291_SDK_DUAL_CM3
	hal_lmac_uplink_request_sysbuf( buffer, NULL );
#else
	nrc_mbx_send_data_address(buffer);
#endif
	return 0;
}
#ifdef CONFIG_IEEE80211W				
int nrc_ccmp_transmit(struct nrc_wpa_if* intf, uint8_t *frm, const uint16_t len,
				const int ac)
{
	const int nAllocTry = 10;
	int hif_len;// = len + sizeof(FRAME_HDR);
	SYS_BUF *buffer = NULL;
	uint8_t cipher = WIM_CIPHER_TYPE_NONE;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) frm;
	uint8_t *pos = (uint8_t *) (hdr + 1);	
	struct nrc_wpa_key *wpa_key = nrc_wpa_get_key(intf, hdr->addr1);
	int result;
	char *enc_data;

	enc_data = os_malloc(len + 8 + 8);
	if(enc_data == NULL)
		return -1;

	#if 0
	result = ccmp_encrypt(intf->sta.key.key, frm, len, 24, &intf->sta.key.tsc, intf->sta.key.ix, enc_data, len + 8 + 8);
	if(result < 0)
	{
		system_printf("ccmp_encrypt return error!!\r\n");
		os_free(enc_data);
		return -2;
	}
	intf->sta.key.tsc++;
	#else
	os_memcpy(enc_data, frm, 24);
	nrc_add_sec_hdr(wpa_key, &enc_data[24]);
	os_memcpy(&enc_data[32], &frm[24], len - 24);
	ccmp_encrypt_V1(wpa_key->key, enc_data, len + 8, 24, (char *)enc_data, len + 8 + 8);
	result = len + 8 + 8;
	#endif

	hif_len = result +  sizeof(FRAME_HDR);
	buffer = alloc_sys_buf_try(hif_len, nAllocTry);

	if (!buffer)
	{
		os_free(enc_data);
		return -1;
	}

    if (wpa_key && WIM_CIPHER_TYPE_INVALID != (int8_t)wpa_key->cipher)
	{    
		cipher = wpa_key->cipher;
	}
	
	//nrc_add_sec_hdr(wpa_key, pos);

	copy_to_sysbuf(buffer, enc_data, hif_len);
	os_free(enc_data);
	
	memset( &HIF_HDR(buffer) , 0 , sizeof(HIF_HDR) );
	memset( &FRAME_HDR(buffer) , 0 , sizeof(FRAME_HDR) );
	
	HIF_HDR(buffer).type        = HIF_TYPE_FRAME;
	HIF_HDR(buffer).len         = hif_len;
	HIF_HDR(buffer).vifindex    = intf->vif_id;

	FRAME_HDR(buffer).info.tx.tlv_len    = 0;
	FRAME_HDR(buffer).info.tx.cipher     = cipher;
	FRAME_HDR(buffer).flags.tx.ac = ac;

#ifndef NRC7291_SDK_DUAL_CM3
	hal_lmac_uplink_request_sysbuf( buffer, NULL );
#else
	nrc_mbx_send_data_address(buffer);
#endif
	return 0;
}
#endif
int nrc_transmit(struct nrc_wpa_if* intf, uint8_t *frm, const uint16_t len)
{
	const struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)frm;
	u16 fc = le_to_host16(hdr->frame_control);
	
#ifdef CONFIG_IEEE80211W
	if(intf->pmf && (ieee80211_is_pmf_mgmt_frame(hdr) == 0))
		return nrc_ccmp_transmit(intf, frm, len, ACI_VO);
	else
#endif
		return nrc_raw_transmit(intf, frm, len, ACI_VO);
}

static int nrc_get_80211_hdr(struct ieee80211_hdr *hdr,
									struct nrc_wpa_if *intf,
									struct ieee8023_hdr *eth_hdr,
									struct nrc_wpa_key *wpa_key,
									bool qos)
{
	int hdrlen = sizeof(*hdr);
	os_memset(hdr, 0, sizeof(struct ieee80211_hdr));

	hdr->frame_control = _FC(DATA, DATA);

	if (qos) {
		hdr->frame_control = _FC(DATA, QOS_DATA);
		hdrlen += 2;
	}

	if (!intf->is_ap) {
		os_memcpy(hdr->addr1, intf->bss.bssid, ETH_ALEN); // BSSID
		os_memcpy(hdr->addr2, eth_hdr->src, ETH_ALEN); // SA
		os_memcpy(hdr->addr3, eth_hdr->dest, ETH_ALEN); // DA
		hdr->frame_control |= WLAN_FC_TODS;
	} else {
		os_memcpy(hdr->addr1, eth_hdr->dest, ETH_ALEN); // DA
		os_memcpy(hdr->addr2, intf->bss.bssid, ETH_ALEN); // BSSID
		os_memcpy(hdr->addr3, eth_hdr->src, ETH_ALEN); // SA
		hdr->frame_control |= WLAN_FC_FROMDS;
	}

	if (wpa_key && wpa_key->cipher != WIM_CIPHER_TYPE_NONE)
		hdr->frame_control |= WLAN_FC_ISWEP;

	return hdrlen;
}


int nrc_get_llc_len(uint16_t ethertype)
{
	return (ethertype >= ETH_P_802_3_MIN) ? 8 : 0;
}

int nrc_add_llc(uint16_t ethertype, uint8_t *pos)
{
	uint8_t* llc = pos;
	const uint8_t rfc1042_header[]			= { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };
	const uint8_t bridge_tunnel_header[]	= { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8 };

	if (ethertype < ETH_P_802_3_MIN)
		return 0;

	if (ethertype == ETH_P_AARP || ethertype == ETH_P_IPX) {
		os_memcpy(llc, bridge_tunnel_header, 6);
		llc += sizeof(bridge_tunnel_header);
	} else if (ethertype >= ETH_P_802_3_MIN) {
		os_memcpy(llc, rfc1042_header, 6);
		llc += sizeof(rfc1042_header);
	}

	*llc++ = (ethertype >> 8);
	*llc++ = (ethertype & 0xFF);

	return (llc - pos);
}

int nrc_add_qos(uint8_t *pos)
{
	/**
	 * Fill this value with IP TOS field
	*/
	*pos++ = 0x0;
	*pos++ = 0x0;

	return 2;
}

#define NRC_WPA_BUFFER_ALLOC_TRY (10)

/*
*   WiFi Chip send 802.11 raw packet (such as beacon/probe so on).
*   @ payload: 802.11 header + body
*   @ length: packet length
*
*   Return 0 means send packet success, otherwise fail.
*/
int wifi_drv_raw_xmit(uint8_t vif_id, const uint8_t *frame, const uint16_t len)
{
    struct nrc_wpa_if *intf = wpa_driver_get_interface(vif_id);
    uint16_t hif_len = 0, frame_len = len;
    int remain = 0, copy = 0;
    SYS_BUF *buffer = NULL, *cur_buf = NULL;
    uint8_t *pos;
    const uint8_t *frame_pos = frame;

    if (!frame || !len)
        return -1;

    hif_len = sizeof(FRAME_HDR) + len;
    buffer = alloc_sys_buf_try(hif_len, NRC_WPA_BUFFER_ALLOC_TRY);
    if (!buffer) {
        wpa_printf(MSG_ERROR, "Failed to allocate raw tx buf.");
        return -1;
    }

    cur_buf = buffer;
    pos = cur_buf->payload;
    remain = SYS_BUF_DATA_LENGTH(cur_buf) - sizeof(PHY_TXVECTOR);

    copy = min(frame_len, remain);
    os_memcpy(pos, frame_pos, copy);
    pos += copy; frame_len -= copy; frame_pos += copy;

    while (frame_len > 0) {
        cur_buf = SYS_BUF_LINK(cur_buf);
        pos = cur_buf->more_payload;
        remain = SYS_BUF_DATA_LENGTH(cur_buf);
        copy = min(frame_len, remain);

        os_memcpy(pos, frame_pos, copy);
        pos += copy; frame_len -= copy; frame_pos += copy;
    }

    memset(&HIF_HDR(buffer) , 0, sizeof(HIF_HDR));
    HIF_HDR(buffer).type = HIF_TYPE_FRAME;
    HIF_HDR(buffer).len = hif_len;
    HIF_HDR(buffer).vifindex = vif_id;

    FRAME_HDR(buffer).info.tx.tlv_len = 0;
    FRAME_HDR(buffer).info.tx.cipher = WIM_CIPHER_TYPE_NONE;
    FRAME_HDR(buffer).flags.tx.ac = get_ac(buffer);

    hal_lmac_uplink_request_sysbuf(buffer, NULL);

    return 0;
}

static ETH_EAPOL_ENC_E eth_eapol_enc = ETH_EAPOL_ENC_DISABLE;

void wpa_set_eth_eapol_enc(ETH_EAPOL_ENC_E flag)
{
	eth_eapol_enc = flag;
}
ETH_EAPOL_ENC_E  wpa_get_eth_eapol_enc(void)
{
	return eth_eapol_enc;
}

int nrc_transmit_from_8023(uint8_t vif_id, uint8_t *frame, const uint16_t len) {
	uint8_t *frames[] = {frame,};
	uint16_t lens[] = {len,};
	return nrc_transmit_from_8023_mb(vif_id, frames, lens, 1);
}

int nrc_transmit_from_8023_mb(uint8_t vif_id, uint8_t **frames, const uint16_t len[], int n_frames)
{
	struct nrc_wpa_if *intf = wpa_driver_get_interface(vif_id);
	struct ieee80211_hdr hdr;
	uint16_t hif_len = 0;
	struct ieee8023_hdr *eth_hdr = (struct ieee8023_hdr *) frames[0];
	bool qos = false;
	struct nrc_wpa_key *wpa_key = NULL;
	uint16_t ethertype;
	struct nrc_wpa_sta *dsta = NULL;
	int i = 0, j = 0, remain = 0;
	SYS_BUF *buffer = NULL, *cur_buf = NULL;
	uint8_t *pos, *limit;

	if (!frames || !frames[0])
		return 0;

	dsta = nrc_wpa_find_sta(intf, eth_hdr->dest);

	if (!dsta) {
		wpa_printf(MSG_ERROR, "Failed to find sta (" MACSTR ")",
				MAC2STR(eth_hdr->dest));
		return 0;
	}
	ethertype = (frames[0][12] << 8) | frames[0][13];

	if (dsta->qos && ethertype != ETH_P_EAPOL)
		qos = true;

	if (intf->is_ap)
		wpa_key = nrc_wpa_get_key(intf, eth_hdr->dest);
	else
		wpa_key = nrc_wpa_get_key(intf, intf->bss.bssid);

	//wpa_driver_debug_key(wpa_key);
	hif_len = sizeof(FRAME_HDR);
	hif_len += nrc_get_80211_hdr(&hdr, intf, eth_hdr, wpa_key, qos);
	hif_len += nrc_get_llc_len(ethertype);

	if(ethertype == ETH_P_EAPOL && (wpa_get_eth_eapol_enc() != ETH_EAPOL_ENC_ENABLE))
		hdr.frame_control &= ~WLAN_FC_ISWEP;
	
	if(ethertype != ETH_P_EAPOL || (ethertype == ETH_P_EAPOL && (wpa_get_eth_eapol_enc() == ETH_EAPOL_ENC_ENABLE)))
		hif_len += nrc_get_sec_hdr_len(wpa_key);
	
	for (i = 0; i < n_frames; i++)
		hif_len += len[i];
	hif_len -= HLEN_8023;

	buffer = alloc_sys_buf_try(hif_len, NRC_WPA_BUFFER_ALLOC_TRY);

	if (!buffer) {
		//TODO: add stats
		//wpa_printf(MSG_ERROR, "Failed to allocate tx buf.");
		return -1;
	}

	cur_buf = buffer;
	pos = cur_buf->payload;
	remain = SYS_BUF_DATA_LENGTH(cur_buf);

	//wpa_printf(MSG_ERROR, " n_frames = %d", n_frames);
	for (i = 0; i < n_frames; i++) {
		uint8_t *frame_pos = frames[i];
		uint16_t frame_len = len[i];
		int copy = 0;
		uint8_t *limit = pos + remain;

		if (i == 0) { // 80233 -> 80211
			//remain -= (sizeof(HIF_HDR) + sizeof(FRAME_HDR));
			remain -= sizeof(PHY_TXVECTOR);
			frame_pos += HLEN_8023;
			frame_len -= HLEN_8023;
			os_memcpy(pos, &hdr, sizeof(hdr));
			pos += sizeof(hdr);
			if (qos)
				pos += nrc_add_qos(pos);
			if(ethertype != ETH_P_EAPOL || (ethertype == ETH_P_EAPOL && (wpa_get_eth_eapol_enc() == ETH_EAPOL_ENC_ENABLE)))
				pos += nrc_add_sec_hdr(wpa_key, pos);
			pos += nrc_add_llc(ethertype, pos);
			remain -= (pos - cur_buf->payload);
		}
		copy = min(frame_len, remain);

		os_memcpy(pos, frame_pos, copy);
		remain -= copy;

		//frame_len -= remain; frame_pos += remain;
		pos += copy; frame_len -= copy; frame_pos += copy;

		while (frame_len > 0) {
			cur_buf = SYS_BUF_LINK(cur_buf);
			pos = cur_buf->more_payload;
			remain = SYS_BUF_DATA_LENGTH(cur_buf);
			copy = min(frame_len, remain);

			os_memcpy(pos, frame_pos, copy);
			pos += copy; frame_len -= copy; frame_pos += copy;
			remain -= copy;
		}
	}

	memset(&HIF_HDR(buffer) , 0, sizeof(HIF_HDR));

	HIF_HDR(buffer).type = HIF_TYPE_FRAME;
	HIF_HDR(buffer).len = hif_len;
	HIF_HDR(buffer).vifindex = vif_id;

	FRAME_HDR(buffer).info.tx.tlv_len = 0;
	FRAME_HDR(buffer).info.tx.cipher = wpa_key ? wpa_key->cipher : WIM_CIPHER_TYPE_NONE;
	FRAME_HDR(buffer).flags.tx.ac = get_ac(buffer);

#ifndef NRC7291_SDK_DUAL_CM3
	if (intf->is_ap && dsta->block_ack[get_ac(buffer)]){
		struct frame_tx_info ti;
		ti.v.ampdu = 1;
		hal_lmac_uplink_request_sysbuf(buffer, &ti);
	}
	else {
		hal_lmac_uplink_request_sysbuf( buffer, NULL );
	}
#else
	nrc_mbx_send_data_address(buffer);
#endif

	return 0;
}

#if 0
static int cmd_test_addba(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret = -1, vif = -1, i;
	uint16_t tid, ac, hif_len, mpdu_len;
	uint8_t addr[MAC_ADDR_LEN] = {0,};
	SYS_BUF* packet;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	tid = (uint16_t)strtoul(argv[2], NULL, 0);
	ac = TID_TO_AC(tid);
	vif = atoi(argv[1]);

	if (argc == 4){
		ret = util_cmd_parse_hwaddr(argv[3], addr);
		if (ret == -1)
			return CMD_RET_FAILURE;
		struct nrc_wpa_sta* sta = nrc_wpa_find_sta(wpa_driver_get_interface(vif), addr);
		if (!sta || sta->block_ack[ac])
			return CMD_RET_FAILURE;
		sta->block_ack[ac] = true;
	} else {
		memcpy( addr , system_modem_api_get_bssid(vif), MAC_ADDR_LEN);
		if (system_modem_api_get_aggregation(ac)){
			return CMD_RET_FAILURE;
		}
	}

	/* disregarding qos/payload field in GenericMacHeader */
	hif_len = sizeof(FRAME_HDR) + sizeof(GenericMacHeader) - 2 + sizeof(ADDBA_Req);
    packet = alloc_sys_buf_try(hif_len, NRC_WPA_BUFFER_ALLOC_TRY);

	if(!packet){
		return CMD_RET_FAILURE;
	}

	HIF_HDR(packet).vifindex				= vif;
	HIF_HDR(packet).len						= hif_len;
	HIF_HDR(packet).tlv_len					= 0;

	FRAME_HDR(packet).flags.tx.ac			= ACI_VO;

	memset( &TX_MAC_HDR(packet) , 0 , sizeof(GenericMacHeader) );

	TX_MAC_HDR(packet).version 				= FC_PV0;
	TX_MAC_HDR(packet).type					= FC_PV0_TYPE_MGMT;
	TX_MAC_HDR(packet).subtype 				= FC_PV0_TYPE_MGMT_ACTION;

	memcpy( TX_MAC_HDR(packet).address1 , addr , MAC_ADDR_LEN);
	memcpy( TX_MAC_HDR(packet).address3 , addr , MAC_ADDR_LEN);
	memcpy( TX_MAC_HDR(packet).address2 , system_modem_api_get_mac_address(vif) , MAC_ADDR_LEN);

	memset( &TX_MAC_HDR(packet).payload , 0 , sizeof(ADDBA_Req) );

	ADDBA_Req *addba = (ADDBA_Req*) TX_MAC_HDR(packet).payload;
	addba->category = WLAN_ACTION_BLOCK_ACK;
	addba->ba_action = WLAN_BA_ADDBA_REQUEST;
	addba->amsdu = WLAN_BA_AMSDU;
	addba->policy = WLAN_BA_POLICY_IMM_BA;
	addba->tid = tid;

	hal_lmac_uplink_request_sysbuf( packet, NULL );

	return CMD_RET_SUCCESS;
}

SUBCMD(test,
		addba,
		cmd_test_addba,
		"send ADDBA request",
		"test addba <vif> <tid> {sta-mac-addr}");

static int cmd_test_delba(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret = -1, vif = -1, i;
	uint16_t tid, ac, hif_len, mpdu_len;
	uint8_t addr[MAC_ADDR_LEN] = {0,};
	SYS_BUF* packet;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	tid = (uint16_t)strtoul(argv[2], NULL, 0);
	ac = TID_TO_AC(tid);
	vif = atoi(argv[1]);

	if (argc == 4){
		ret = util_cmd_parse_hwaddr(argv[3], addr);
		if (ret == -1)
			return CMD_RET_FAILURE;
		struct nrc_wpa_sta* sta = nrc_wpa_find_sta(wpa_driver_get_interface(vif), addr);
		if (!sta || !sta->block_ack[ac])
			return CMD_RET_FAILURE;
		sta->block_ack[ac] = false;
	} else {
		memcpy( addr , system_modem_api_get_bssid(vif), MAC_ADDR_LEN);
		if (!system_modem_api_get_aggregation(ac)){
			return CMD_RET_FAILURE;
		}
		system_modem_api_set_aggregation(ac, false);
	}

	/* disregarding qos/payload field in GenericMacHeader */
	hif_len = sizeof(FRAME_HDR) + sizeof(GenericMacHeader) - 2 + sizeof(DELBA_Req);
    packet = alloc_sys_buf_try(hif_len, NRC_WPA_BUFFER_ALLOC_TRY);

	if(!packet){
		return CMD_RET_FAILURE;
	}

	HIF_HDR(packet).vifindex				= vif;
	HIF_HDR(packet).len						= hif_len;
	HIF_HDR(packet).tlv_len					= 0;

	FRAME_HDR(packet).flags.tx.ac			= ACI_VO;

	memset( &TX_MAC_HDR(packet) , 0 , sizeof(GenericMacHeader) );

	TX_MAC_HDR(packet).version 				= FC_PV0;
	TX_MAC_HDR(packet).type					= FC_PV0_TYPE_MGMT;
	TX_MAC_HDR(packet).subtype 				= FC_PV0_TYPE_MGMT_ACTION;

	memcpy( TX_MAC_HDR(packet).address1 , addr , MAC_ADDR_LEN);
	memcpy( TX_MAC_HDR(packet).address3 , addr , MAC_ADDR_LEN);
	memcpy( TX_MAC_HDR(packet).address2 , system_modem_api_get_mac_address(vif) , MAC_ADDR_LEN);

	memset( &TX_MAC_HDR(packet).payload , 0 , sizeof(DELBA_Req) );

	DELBA_Req *delba = (DELBA_Req*) TX_MAC_HDR(packet).payload;
	delba->category = WLAN_ACTION_BLOCK_ACK;
	delba->ba_action = WLAN_BA_DELBA;
	delba->init = true;
	delba->tid = tid;
	delba->reason = WLAN_REASON_END_BA;

	hal_lmac_uplink_request_sysbuf( packet, NULL );

	wpa_printf(MSG_INFO,"BA_STOP:"MACSTR",TID:%d,AC:%d",MAC2STR(addr),tid,ac);

	return CMD_RET_SUCCESS;
}

SUBCMD(test,
		delba,
		cmd_test_delba,
		"send DELBA request",
		"test delba <vif> <tid> {sta-mac-addr}");
#endif

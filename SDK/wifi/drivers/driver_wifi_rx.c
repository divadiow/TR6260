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
#include "system_debug.h"
#include "crypto/ccmp.h"

extern scan_info g_scan_info;
uint8_t g_count = 0;
int8_t g_rssi_inst[MAX_RSSI_RECORD] = {0}; 


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

extern void wpa_driver_set_channel_width(uint8_t vif_id, uint8_t ch, uint8_t type);

static bool linearize(SYS_BUF *src, uint8_t *dst, int start_src_offset,
					int start_dst_offset, int length)
{
	ASSERT(dst);
	ASSERT(src);

	bool res = false;

	int src_offset = start_src_offset;
	int dst_offset = start_dst_offset;

	int src_buffer_length = LMAC_CONFIG_BUFFER_SIZE - src_offset;

	SYS_BUF *src_buffer = src;
	uint8_t *dst_buffer = dst + dst_offset;
	int remain = length;

	while(src_buffer && remain) {
		int copy_len = MIN(src_buffer_length, remain);

		memcpy(dst_buffer + dst_offset, (uint8_t*)src_buffer + src_offset, copy_len);
		dst_offset = 0;
		src_offset = SYS_HDR_SIZE;
		dst_buffer += copy_len;
		src_buffer = SYS_BUF_LINK(src_buffer);
		src_buffer_length = LMAC_CONFIG_BUFFER_SIZE - src_offset;
		remain -= copy_len;
	}
	if (!remain) res = true;

	return res;
}

#define P_OFFSET (sizeof(SYS_HDR) + sizeof(LMAC_RXBUF))

static void nrc_wpa_scan_sta_rx(struct nrc_wpa_rx_data *rx)
{
	uint16_t freq = 0;

#if defined(NRC7291_SDK_DUAL_CM3)
	freq = nrc_mbx_channel_get();
#elif LMAC_CONFIG_11N == 1 // TODO : Fix the below
	freq = system_modem_api_channel_to_mac80211_frequency(rx->rxi->center_freq);
#elif LMAC_CONFIG_11AH == 1
	freq = get_mac80211_frequency(0);
#endif

//#ifdef TEST_DNA_API_WIFI
#if 0
        extern int dna_wlan_scan_filter(unsigned char *bssid, unsigned char channel);
        if(dna_wlan_scan_filter(rx->u.mgmt->sa, (unsigned char)(rx->rxi->center_freq))==-1)
            return;
#endif
	scan_add(rx->intf->scan, freq, rx->rxh->rssi, rx->u.frame, rx->len);
}

static void nrc_mgmt_auth_sta_rx(struct nrc_wpa_if* intf, struct ieee80211_mgmt *mgmt,
						uint16_t len)
{
	union wpa_event_data event;
	os_memset(&event, 0, sizeof(event));
	int ie_offset = offsetof(struct ieee80211_mgmt, u.auth.variable);

	os_memcpy(event.auth.peer, mgmt->sa, ETH_ALEN);
	os_memcpy(event.auth.bssid, mgmt->sa, ETH_ALEN);

	event.auth.auth_type 		= mgmt->u.auth.auth_alg;
	event.auth.auth_transaction = mgmt->u.auth.auth_transaction;
	event.auth.status_code 		= mgmt->u.auth.status_code;

#if 0
	event.auth.ies_len = len - ie_offset;
	event.auth.ies = (uint8_t *) mgmt + ie_offset;
#else
	event.auth.ies = (uint8_t *) mgmt + ie_offset;
	if(mgmt->u.auth.auth_alg == 1 && len - ie_offset > 0)
	{
		event.auth.ies_len = *(event.auth.ies + 1) + 2;
	}
	else
	{
		event.auth.ies_len = len - ie_offset;
	}
#endif


	wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_AUTH, &event);
}
#ifdef CONFIG_IEEE80211W
extern struct nrc_wpa_key	igtk;
#endif
static int nrc_mgmt_deauth_event_sta_rx(struct nrc_wpa_if* intf, struct ieee80211_mgmt *mgmt,
						uint16_t len)
{
	union wpa_event_data event;
	os_memset(&event, 0, sizeof(event));
	int ie_offset = offsetof(struct ieee80211_mgmt, u.deauth.variable);
	int result;
	
#ifdef CONFIG_IEEE80211W
	if(is_multicast_ether_addr(mgmt->da))
	{
		if(intf->pmf && len < sizeof(struct ieee80211_mmie) + ie_offset)
		{
			event.unprot_deauth.sa = mgmt->sa;
			event.unprot_deauth.da = mgmt->da;
			event.unprot_deauth.reason_code = WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA;
			wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_UNPROT_DEAUTH, &event);
			return -1;
		}

		result = broadcast_mgmt_integrity_verify(igtk.key, igtk.ix, igtk.tsc, mgmt, len, &igtk.tsc);
		if(result < 0)
			return -2;
	}
	else
	{
		if(intf->pmf && !(mgmt->frame_control & WLAN_FC_ISWEP))
		{
			event.unprot_deauth.sa = mgmt->sa;
			event.unprot_deauth.da = mgmt->da;
			event.unprot_deauth.reason_code = WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA;
			wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_UNPROT_DEAUTH, &event);
			return -1;
		}
	}
#endif
	event.deauth_info.addr = mgmt->sa;

	event.deauth_info.reason_code = mgmt->u.deauth.reason_code;
	event.deauth_info.ie_len = len - ie_offset;
	event.deauth_info.ie = (uint8_t *) mgmt + ie_offset;
	event.deauth_info.locally_generated = 0;

    system_printf("deauth by ap, reason code %d\n", mgmt->u.deauth.reason_code);
	wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_DEAUTH, &event);

	return 0;
}

static void nrc_mgmt_deauth_sta_rx(struct nrc_wpa_if* intf, struct ieee80211_mgmt *mgmt,
						uint16_t len)
{
	if(nrc_mgmt_deauth_event_sta_rx(intf, mgmt, len) == 0)
	{
		wpa_driver_sta_sta_remove(intf);
    	hal_mac_revert_rate_set();
	}
}

static int nrc_mgmt_disassoc_sta_rx(struct nrc_wpa_if* intf, struct ieee80211_mgmt *mgmt,
						uint16_t len)
{
	union wpa_event_data event;
	os_memset(&event, 0, sizeof(event));
	int ie_offset = offsetof(struct ieee80211_mgmt, u.disassoc.variable);
	int result;

#ifdef CONFIG_IEEE80211W
	if(is_multicast_ether_addr(mgmt->da))
	{
		if(intf->pmf && len < sizeof(struct ieee80211_mmie) + ie_offset)
		{
			event.unprot_disassoc.sa = mgmt->sa;
			event.unprot_disassoc.da = mgmt->da;
			event.unprot_disassoc.reason_code = WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA;
			wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_UNPROT_DISASSOC, &event);
			return -1;
		}

		result = broadcast_mgmt_integrity_verify(igtk.key, igtk.ix, igtk.tsc, mgmt, len, &igtk.tsc);
		if(result < 0)
			return -2;
	}
	else
	{
		if(intf->pmf && !(mgmt->frame_control & WLAN_FC_ISWEP))
		{
			event.unprot_disassoc.sa = mgmt->sa;
			event.unprot_disassoc.da = mgmt->da;
			event.unprot_disassoc.reason_code = WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA;
			wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_UNPROT_DISASSOC, &event);
			return -1;
		}
	}
#endif

	event.disassoc_info.addr = mgmt->sa;
	event.disassoc_info.reason_code = mgmt->u.deauth.reason_code;
	event.disassoc_info.ie_len = len - ie_offset;
	event.disassoc_info.ie = (uint8_t *) mgmt + ie_offset;

    system_printf("disassoc by ap, reason code %d\n", mgmt->u.disassoc.reason_code);
	wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_DISASSOC, &event);
	wpa_driver_set_channel_width(intf->vif_id, nrc_global->cur_chan, 0); // reset to bandwidth 20M
	hal_mac_revert_rate_set();
	umac_bcn_monitor_stop(intf->vif_id);

    return 0;
}

static void nrc_mgmt_assoc_sta_rx(struct nrc_wpa_if* intf, RXINFO *rxi,
					struct ieee80211_mgmt *mgmt, uint16_t len)
{
	union wpa_event_data event;

	os_memset(&event, 0, sizeof(event));

	struct nrc_wpa_sta* sta = nrc_wpa_find_sta(intf, mgmt->sa);

	int ie_offset = offsetof(struct ieee80211_mgmt, u.assoc_resp.variable);
	uint8_t *ie = (uint8_t *) mgmt + ie_offset;
	struct ieee802_11_elems elems;
	uint16_t ies_len = len - ie_offset;
	int status = mgmt->u.assoc_resp.status_code;
	int i = 0;

	if (status == WLAN_STATUS_SUCCESS) {
		intf->associated = true;
		if (WLAN_FC_GET_STYPE(mgmt->frame_control) == WLAN_FC_STYPE_REASSOC_RESP)
			event.assoc_info.reassoc = 1;
		event.assoc_info.resp_ies_len = ies_len;
		event.assoc_info.resp_ies = ie;
		event.assoc_info.freq = system_modem_api_channel_to_mac80211_frequency(rxi->center_freq);//rxi->center_freq;
		event.assoc_info.authorized = 0;
		intf->sta.aid = (mgmt->u.assoc_resp.aid & 0x3FFF);

		wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_ASSOC, &event);
		wpa_driver_sta_sta_add(intf);
		os_memcpy(sta->addr, mgmt->sa, ETH_ALEN);
		sta->aid = 0;
		ieee802_11_parse_elems(ie, ies_len, &elems, 1);
		/* In 11ah, QoS bit needs to be set also when EDCA Param IE is visible.
		 * But, hook system underlying this translates EDCA IE to WMM IE.
		*/
		sta->qos = !!(elems.wmm);
        if(elems.ht_capabilities)
        {
            ie_ht_capabilities *ht_cap = (ie_ht_capabilities*)(elems.ht_capabilities-2);
			hal_lmac_rc_set_cursor_from_format(FORMAT_HT_MIXED,0);
			if(ht_cap->ht_capabilities_info.supported_channel_width)
            {
                extern unsigned char nv_bw40_enable;
                if(nv_bw40_enable && elems.ht_operation)
                {
	                ie_ht_operation *ht_op = (ie_ht_operation*)(elems.ht_operation-2);
                    wpa_driver_set_channel_width(intf->vif_id, ht_op->primary_channel, ht_op->secondary_channel_offset);
                }
			}
        }
		
		else if(!elems.ext_supp_rates)
		{
			hal_lmac_rc_set_cursor_from_format(FORMAT_11B,0);
		}
		else
			hal_lmac_rc_set_cursor_from_format(FORMAT_11AG,0);
	} else {
		intf->associated = false;
		os_memset(&intf->bss, 0, sizeof(intf->bss));
		event.assoc_reject.bssid = mgmt->sa;
		event.assoc_reject.resp_ies_len = ies_len;
		event.assoc_reject.resp_ies = ie;
		event.assoc_reject.status_code = status;
		event.assoc_reject.timed_out = 0;
		wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_ASSOC_REJECT, &event);
	}
}
static void nrc_addba_req_handler(struct ieee80211_mgmt* mgmt, struct nrc_wpa_if* intf)
{
extern SYS_BUF * alloc_sys_buf_try(int hif_len, int nTry);

	int i, tid, ac;
	uint16_t hif_len, mpdu_len, buf_size, timeout, dial_token;
	SYS_BUF* packet;
	const int nTry = 10;
	struct nrc_wpa_key *wpa_key = NULL;
	int wpa_key_len = 0;
	
#ifdef CONFIG_IEEE80211W
	if(intf->pmf)
	{
		if (intf->is_ap)
			wpa_key = nrc_wpa_get_key(intf, mgmt->sa);
		else
			wpa_key = nrc_wpa_get_key(intf, intf->bss.bssid);
		wpa_key_len = nrc_get_sec_hdr_len(wpa_key);
	}
#endif
	/* disregarding qos/payload field in GenericMacHeader */
	hif_len = sizeof(FRAME_HDR) + sizeof(GenericMacHeader) - 2 + sizeof(ADDBA_Resp) + wpa_key_len + (wpa_key_len ? 8 : 0);
    packet = alloc_sys_buf_try(hif_len, nTry);

	if(!packet){
		return;
	}

	HIF_HDR(packet).vifindex				= intf->vif_id;
	HIF_HDR(packet).len						= hif_len;
	HIF_HDR(packet).tlv_len					= 0;

	FRAME_HDR(packet).flags.tx.ac			= ACI_VO;
	FRAME_HDR(packet).info.tx.cipher = wpa_key ? wpa_key->cipher : WIM_CIPHER_TYPE_NONE;

	memset( &TX_MAC_HDR(packet) , 0 , sizeof(GenericMacHeader) );

	TX_MAC_HDR(packet).version 				= FC_PV0;
	TX_MAC_HDR(packet).type					= FC_PV0_TYPE_MGMT;
	TX_MAC_HDR(packet).subtype 				= FC_PV0_TYPE_MGMT_ACTION;
#ifdef CONFIG_IEEE80211W
	if(intf->pmf)
		TX_MAC_HDR(packet).protect = 1; //WLAN_FC_ISWEP;
#endif
	memcpy( TX_MAC_HDR(packet).address1 , mgmt->sa , MAC_ADDR_LEN);
	memcpy( TX_MAC_HDR(packet).address2 , mgmt->da , MAC_ADDR_LEN);
	memcpy( TX_MAC_HDR(packet).address3 , mgmt->bssid , MAC_ADDR_LEN);

	/* fields info from addba request action frame */
	tid = mgmt->u.action.u.ba_action.u.addba_req_action.tid;
	ac = TID_TO_AC(tid);
	buf_size = mgmt->u.action.u.ba_action.u.addba_req_action.buf_size;
	timeout = mgmt->u.action.u.ba_action.u.addba_req_action.timeout;
	dial_token = mgmt->u.action.u.ba_action.u.addba_req_action.dial_token;

	memset( &TX_MAC_HDR(packet).payload , 0 , sizeof(ADDBA_Resp) );
#ifdef CONFIG_IEEE80211W
	if(intf->pmf)
		nrc_add_sec_hdr(wpa_key, TX_MAC_HDR(packet).payload);
#endif

	ADDBA_Resp *addba_resp = (ADDBA_Resp*) (TX_MAC_HDR(packet).payload + wpa_key_len);
	addba_resp->category = WLAN_ACTION_BLOCK_ACK;
	addba_resp->ba_action = WLAN_BA_ADDBA_RESPONSE;
	addba_resp->dial_token = dial_token;

	if (intf->is_ap){
		if (nrc_wpa_find_sta(intf, mgmt->sa)->block_ack[ac]){
			addba_resp->status = WLAN_STATUS_REQUEST_DECLINED;
		} else {
			nrc_wpa_find_sta(intf, mgmt->sa)->block_ack[ac] = true;
			addba_resp->status = WLAN_STATUS_SUCCESS;
			wpa_printf(MSG_INFO,"BA_START:"MACSTR",TID:%d,AC:%d",MAC2STR(mgmt->sa),tid,ac);
		}
	} else {
		if (system_modem_api_get_aggregation(ac)){
			addba_resp->status = WLAN_STATUS_REQUEST_DECLINED;
		} else {
			system_modem_api_set_aggregation(ac, true);
			addba_resp->status = WLAN_STATUS_SUCCESS;
			wpa_printf(MSG_INFO,"BA_START:"MACSTR",TID:%d,AC:%d",MAC2STR(mgmt->sa),tid,ac);
		}
	}

	addba_resp->amsdu = WLAN_BA_NO_AMSDU;
	addba_resp->policy = WLAN_BA_POLICY_IMM_BA;
	addba_resp->buf_size = (buf_size) ? buf_size : WLAN_BA_MAX_BUF;
	addba_resp->tid = tid;
	addba_resp->timeout = timeout;

#ifdef CONFIG_IEEE80211W
	if(intf->pmf)
		ccmp_encrypt_V1(wpa_key->key, (char *)&TX_MAC_HDR(packet), hif_len -sizeof(FRAME_HDR) - 8, 24, (char *)& TX_MAC_HDR(packet), hif_len -sizeof(FRAME_HDR));
#endif	
	hal_lmac_uplink_request_sysbuf( packet, NULL );
}

static void nrc_addba_resp_handler(struct ieee80211_mgmt* mgmt, struct nrc_wpa_if* intf)
{
	uint16_t status = mgmt->u.action.u.ba_action.u.addba_resp_action.status;
	int tid = mgmt->u.action.u.ba_action.u.addba_resp_action.tid;
	int ac = TID_TO_AC(tid);

	if (status != WLAN_STATUS_SUCCESS){
		wpa_printf(MSG_INFO,"BA_ERR:"MACSTR",TID:%d,AC:%d",MAC2STR(mgmt->sa),tid,ac);
		return;
	}
	if (intf->is_ap){
		struct nrc_wpa_sta* sta = nrc_wpa_find_sta(intf, mgmt->sa);
		if (!sta)
			return;
		sta->block_ack[ac] = true;
	} else {
		system_modem_api_set_aggregation(ac, true);
	}
	wpa_printf(MSG_INFO,"BA_START:"MACSTR",TID:%d,AC:%d",MAC2STR(mgmt->sa),tid,ac);
}

static void nrc_delba_req_handler(struct ieee80211_mgmt* mgmt, struct nrc_wpa_if* intf)
{
	int tid  = mgmt->u.action.u.ba_action.u.delba_req_action.tid;
	int ac = TID_TO_AC(tid);

	if (intf->is_ap){
		struct nrc_wpa_sta* sta = nrc_wpa_find_sta(intf, mgmt->sa);
		if (!sta)
			return;
		sta->block_ack[ac] = false;
	} else {
		system_modem_api_set_aggregation(ac, false);
	}

	wpa_printf(MSG_INFO,"BA_STOP:"MACSTR",TID:%d,AC:%d",MAC2STR(mgmt->sa),tid,ac);
}

static void nrc_mgmt_action_blockack_rx(struct ieee80211_mgmt* mgmt, struct nrc_wpa_if* intf)
{
	switch (mgmt->u.action.u.ba_action.action) {
		case WLAN_BA_ADDBA_REQUEST:
		nrc_addba_req_handler(mgmt, intf);
		break;
		case WLAN_BA_ADDBA_RESPONSE:
		nrc_addba_resp_handler(mgmt, intf);
		break;
		case WLAN_BA_DELBA:
		nrc_delba_req_handler(mgmt, intf);
		break;
	}
}
 void wpa_driver_event_rx(void *eloop_data, void *user_ctx)
{
	struct nrc_wpa_rx_event *rxe = (struct nrc_wpa_rx_event *) user_ctx;
	union wpa_event_data event;
	struct os_time time;
	os_memset(&event, 0, sizeof(event));

	event.rx_mgmt.frame = rxe->frame;
	event.rx_mgmt.frame_len = rxe->len;
	event.rx_mgmt.freq = rxe->freq;
	//V(TT_WPAS, "[%d] %s(%d):%d \n", NOW - rxe->ref_time, __func__, __LINE__, rxe->subtype);
	wpa_supplicant_event(rxe->pv, EVENT_RX_MGMT, &event);
	//V(TT_WPAS, "[%d] %s(%d):%d \n", NOW - rxe->ref_time, __func__, __LINE__, rxe->subtype);

	os_free(rxe->frame);
	os_free(rxe);
}
#ifdef CONFIG_IEEE80211W
static void nrc_mgmt_action_query_rx(struct ieee80211_mgmt* mgmt, struct nrc_wpa_if* intf, struct nrc_wpa_rx_data *rx)
{
	struct nrc_wpa_rx_event *rxe = NULL;

	rxe = os_malloc(sizeof(*rxe));
	if (!rxe) {
		wpa_printf(MSG_ERROR, "failed to allocate rx_event");
		return;
	}
	
	rxe->frame = os_malloc(rx->len);
	if (!rxe->frame) {
		wpa_printf(MSG_ERROR, "failed to allocate rx_event frame");
		os_free(rxe);
		return;
	}

	os_memcpy(rxe->frame, rx->u.frame, rx->len);
	rxe->len = rx->len;
	rxe->pv = rx->intf->wpa_supp_ctx;
	rxe->freq = system_modem_api_channel_to_mac80211_frequency(rx->rxi->center_freq);
	rxe->pv = rx->intf->wpa_supp_ctx;
	rxe->ref_time = rx->ref_time;
	rxe->subtype = WLAN_FC_STYPE_ACTION;
	//V(TT_WPAS, "[%d] %s:%d \n", NOW - rx->ref_time, __func__, subtype);

	eloop_register_timeout(0, 0, wpa_driver_event_rx, 0, rxe);
}
#endif
static void nrc_mgmt_action_rx(struct ieee80211_mgmt* mgmt, struct nrc_wpa_if* intf, struct nrc_wpa_rx_data *rx)
{
	switch (mgmt->u.action.category) {
		case WLAN_ACTION_BLOCK_ACK:
		nrc_mgmt_action_blockack_rx(mgmt, intf);
		break;
		#ifdef CONFIG_IEEE80211W
		case WLAN_ACTION_SA_QUERY:
			nrc_mgmt_action_query_rx(mgmt, intf, rx);
			break;
		#endif
	}
}

static void _nrc_mgmt_sta_rx(uint16_t subtype, struct nrc_wpa_rx_data *rx)
{
    if (0 != os_memcmp(rx->u.mgmt->bssid, rx->intf->bss.bssid, 6)) {
		#if !defined(ENABLE_DNA)
        system_printf("mgmt %d recv, bssid mismatch\n", subtype);
		#endif
        return;
    }
#ifdef CONFIG_IEEE80211W
	if(rx->intf->pmf && (rx->u.mgmt->frame_control & WLAN_FC_ISWEP)) //fix h/w mic error, dump ccmp header
	{
		rx->len  = rx->len - 8;
		os_memcpy((char *)rx->u.frame + 24, (char *)rx->u.frame + 32, rx->len);
	}
#endif
		
	switch (subtype) {
		case WLAN_FC_STYPE_AUTH:
		nrc_mgmt_auth_sta_rx(rx->intf, rx->u.mgmt, rx->len);
		break;
		case WLAN_FC_STYPE_REASSOC_RESP:
		case WLAN_FC_STYPE_ASSOC_RESP:
		nrc_mgmt_assoc_sta_rx(rx->intf, rx->rxi, rx->u.mgmt, rx->len);
		break;
		case WLAN_FC_STYPE_DEAUTH:
		nrc_mgmt_deauth_sta_rx(rx->intf, rx->u.mgmt, rx->len);
		break;
		case WLAN_FC_STYPE_DISASSOC:
		nrc_mgmt_disassoc_sta_rx(rx->intf, rx->u.mgmt, rx->len);
		break;
		case WLAN_FC_STYPE_ACTION:
		nrc_mgmt_action_rx(rx->u.mgmt, rx->intf, rx);
		break;

	}
}

static int nrc_sta_status(struct nrc_wpa_rx_data *rx)
{
	if (!rx->intf->is_ap) {
        g_rssi_inst[g_count++] = (int8_t)rx->rxh->rssi;
        g_count = g_count % MAX_RSSI_RECORD;
	}
	
	return 0;
}

static void nrc_wpa_mgmt_sta_rx(uint16_t subtype, struct nrc_wpa_rx_data *rx)
{
    if (subtype == WLAN_FC_STYPE_PROBE_RESP || subtype == WLAN_FC_STYPE_BEACON) {
        if (hal_lmac_get_scanning(rx->intf->vif_id & 0x1)) {
            if (subtype == WLAN_FC_STYPE_PROBE_RESP && g_scan_info.passive)
                return;
            nrc_wpa_scan_sta_rx(rx);
        }
        return;
    }
    _nrc_mgmt_sta_rx(subtype, rx);
}

static void nrc_wpa_mgmt_ap_rx(uint16_t subtype, struct nrc_wpa_rx_data *rx)
{
	struct nrc_wpa_rx_event *rxe = NULL;
	struct nrc_wpa_sta* sta = NULL;

#ifdef CONFIG_IEEE80211W
	if(rx->intf->pmf && (rx->u.mgmt->frame_control & WLAN_FC_ISWEP)) //fix h/w mic error, dump ccmp header
	{
		rx->len  = rx->len - 8;
		os_memcpy((char *)rx->u.frame + 24, (char *)rx->u.frame + 32, rx->len);
	}
#endif

	switch (subtype) {
		case WLAN_FC_STYPE_BEACON:
		case WLAN_FC_STYPE_PROBE_RESP:
		case WLAN_FC_STYPE_PROBE_REQ:
		// Drop
		return;
		case WLAN_FC_STYPE_DEAUTH:
		sta = nrc_wpa_find_sta(rx->intf, rx->u.mgmt->sa);
		if (!sta)
			return;
		break;
		case WLAN_FC_STYPE_ACTION:
		nrc_mgmt_action_rx(rx->u.mgmt, rx->intf, rx);
		return;
	}

	rxe = os_malloc(sizeof(*rxe));

	if (!rxe) {
		wpa_printf(MSG_ERROR, "failed to allocate rx_event");
		//taskEXIT_CRITICAL();
		return;
	}
	rxe->frame = os_malloc(rx->len);

	if (!rxe->frame) {
		wpa_printf(MSG_ERROR, "failed to allocate rx_event frame");
		os_free(rxe);
		//taskEXIT_CRITICAL();
		return;
	}

	os_memcpy(rxe->frame, rx->u.frame, rx->len);
	rxe->len = rx->len;
	rxe->pv = rx->intf->wpa_supp_ctx;
	rxe->freq = system_modem_api_channel_to_mac80211_frequency(rx->rxi->center_freq);
	rxe->pv = rx->intf->wpa_supp_ctx;
	rxe->ref_time = rx->ref_time;
	rxe->subtype = subtype;
	//V(TT_WPAS, "[%d] %s:%d \n", NOW - rx->ref_time, __func__, subtype);

	eloop_register_timeout(0, 0, wpa_driver_event_rx, 0, rxe);
}
#if defined(ENABLE_DNA)
typedef void (*wifi_rx_frame_filter_t)(const uint8_t *frame, int len);
static wifi_rx_frame_filter_t  wifi_rx_frame_filter_cb= NULL;

int wifi_rx_frame_filter_register(wifi_rx_frame_filter_t cb)
{
	wifi_rx_frame_filter_cb = cb;
	return 0;
}

int wifi_rx_frame_filter_unregister(void)
{
	wifi_rx_frame_filter_cb = NULL;
	return 0;
}

#endif


static void nrc_wpa_mgmt_rx(uint16_t subtype, struct nrc_wpa_rx_data *rx)
{
	rx->ref_time = NOW;
	if (!rx->intf->is_ap)
		nrc_wpa_mgmt_sta_rx(subtype, rx);
	else {
		//V(TT_WPAS, "[%d/%d] %s:%d \n", NOW, NOW - rx->ref_time, __func__, subtype);
		nrc_wpa_mgmt_ap_rx(subtype, rx);
	}

	#if defined(ENABLE_DNA)
	if(wifi_rx_frame_filter_cb != NULL)
	{
		wifi_rx_frame_filter_cb(rx->u.frame,rx->len);
	}
	#endif
}

int mic_error_ignore_verify(struct nrc_wpa_rx_data *rx)
{
	int result = -1;

	if(rx->rxi->error_mic && is_multicast_ether_addr(rx->u.hdr->addr1))
	{
		result  = 0;
	}

	return result;
}
static int nrc_wpa_michael_mic_verify(struct nrc_wpa_rx_data *rx)
{
	union wpa_event_data event;

	if(!rx->rxi->error_mic)
		return 0;
	
	if(mic_error_ignore_verify(rx) == 0)
		return 0;
	
	wpa_printf(MSG_ERROR, "%s: mic error.", __func__);
	os_memset(&event, 0, sizeof(event));
	if (!is_broadcast_ether_addr(rx->u.hdr->addr3)) {
		event.michael_mic_failure.unicast = 1;
		event.michael_mic_failure.src = rx->u.hdr->addr2;
	}
	wpa_supplicant_event(rx->intf->wpa_supp_ctx, EVENT_MICHAEL_MIC_FAILURE,
					&event);

	return -1;
}

static int nrc_wpa_qos_null(struct nrc_wpa_rx_data *rx)
{
	//wpa_printf(MSG_ERROR, "%s", __func__);
	if (WLAN_FC_GET_STYPE(rx->u.hdr->frame_control)
					== WLAN_FC_STYPE_QOS_NULL) {
		return -1;
	}
	return 0;
}

static int nrc_wpa_eapol(struct nrc_wpa_rx_data *rx)
{
	//wpa_printf(MSG_ERROR, "%s", __func__);
	if (rx->is_8023) {
		extern int wpas_l2_packet_filter(uint8_t *buffer, int len);
		if (wpas_l2_packet_filter(rx->u.frame + rx->offset_8023, rx->len))
			return -1;
	}
	return 0;
}

extern void lwif_input_from_net80211(uint8_t vif_id, unsigned char *buffer, int data_len);
static int nrc_wpa_lwip(struct nrc_wpa_rx_data *rx)
{
	uint8_t *pos;
	int i =0;
	uint8_t vif_id= rx->intf->vif_id;
	//system_printf("%s vif_id:%d\n", __func__, vif_id);

	if (!rx->is_8023)
		return -1;

	lwif_input_from_net80211(vif_id, rx->u.frame + rx->offset_8023,
		rx->len);

#if 0
	system_printf("\n");
	pos = (uint8_t *)  rx->u.frame + rx->offset_8023;
	for (i = 0; i < rx->len; i++) {
		system_printf("0x%02X,",pos[i]);
	}
	system_printf("\n");
#endif

	return 0;
}

static int nrc_wpa_ap_process(struct nrc_wpa_rx_data *rx)
{
	struct ieee80211_hdr *hdr = rx->u.hdr;
	struct ieee80211_hdr forward_hdr = {0, };
	uint16_t fc = hdr->frame_control;
	struct nrc_wpa_sta *sta = NULL;
	uint8_t saddr[ETH_ALEN];
	int ret = 0, i = 0;

	if (!rx->intf->is_ap)
		return 0;

	if (!(fc & WLAN_FC_TODS))
		return -1;

	if ((fc & WLAN_FC_TODS) && (fc & WLAN_FC_FROMDS)) {
		wpa_printf(MSG_ERROR, "Incorrect DS");
		return -1;
	}

	// If DA is Soft AP, the frame is consumed (return 0)
	// If DA is another STA connected to Soft AP, the frame is forwared
	// If DA is multicast, the frame is consumed and forwared. (return 0)
	if (os_memcmp(hdr->addr3, rx->intf->addr, ETH_ALEN) == 0)
		return 0;

	if (is_multicast_ether_addr(hdr->addr3)) {
		ret = 0;
	} else {
		// TODO: If the number of connected STs is just one,
		//  there is no point in forwarding the frame.
		sta = nrc_wpa_find_sta(rx->intf, hdr->addr3);

		if (!sta) {
			wpa_printf(MSG_ERROR, "Unable forward frames to STA(" MACSTR "), "
						"(Failed to find sta_info)",
						MAC2STR(hdr->addr3));

		 	ret = -1;
		}
	}
#if 1
	os_memcpy(&forward_hdr, hdr, sizeof(forward_hdr));
	hdr->frame_control &= ~WLAN_FC_TODS;
	hdr->frame_control |= WLAN_FC_FROMDS;
	os_memcpy(saddr, hdr->addr2, ETH_ALEN);
	os_memcpy(hdr->addr2, hdr->addr1, ETH_ALEN);
	os_memcpy(hdr->addr1, hdr->addr3, ETH_ALEN);
	os_memcpy(hdr->addr3, saddr, ETH_ALEN);
	nrc_raw_transmit(rx->intf, (void *) rx->u.frame, rx->len, ACI_BE);

	if (ret == 0)
		os_memcpy(hdr, &forward_hdr, sizeof(forward_hdr));
#endif
	return ret;
}

static int nrc_wpa_to_8023(struct nrc_wpa_rx_data *rx)
{
/* convert IEEE 802.11 header + possible LLC headers into Ethernet
* header
* IEEE 802.11 address fields:
* ToDS FromDS Addr1 Addr2 Addr3 Addr4
*   0     0   DA    SA    BSSID n/a
*   0     1   DA    BSSID SA    n/a
*   1     0   BSSID SA    DA    n/a
*   1     1   RA    TA    DA    SA
*/
	struct ieee8023_hdr ehdr;
	uint16_t fc = rx->u.hdr->frame_control;

	os_memset(&ehdr, 0, sizeof(struct ieee8023_hdr));
	uint8_t *payload = (uint8_t *) (rx->u.hdr + 1);

	const uint8_t rfc1042_header[] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };
	const uint8_t bridge_tunnel_header[] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8 };

	int to_ds = (rx->u.hdr->frame_control & WLAN_FC_TODS) ? 1 : 0;
	int from_ds = (rx->u.hdr->frame_control & WLAN_FC_FROMDS) ? 1 : 0;

	if (rx->u.hdr->frame_control & (WLAN_FC_STYPE_QOS_DATA << 4))
		payload += 2;

	if (rx->u.hdr->frame_control & WLAN_FC_ISWEP) {
		struct nrc_wpa_key *wpa_key = nrc_wpa_get_key(rx->intf, rx->u.hdr->addr2);
		payload += nrc_get_sec_hdr_len(wpa_key);
	}

	if (to_ds && from_ds) {
		wpa_printf(MSG_ERROR, "Unsupported conversion to 802.3");
		return -1;
	} else if (to_ds && !from_ds && !rx->intf->is_ap) {
		wpa_printf(MSG_ERROR, "Unsupported conversion to 802.3 (not AP)");
		return -1;
	}

	if (to_ds) {
		os_memcpy(ehdr.dest, rx->u.hdr->addr3, ETH_ALEN);
		os_memcpy(ehdr.src, rx->u.hdr->addr2, ETH_ALEN);
	} else {
		os_memcpy(ehdr.dest, rx->u.hdr->addr1, ETH_ALEN);
		os_memcpy(ehdr.src, rx->u.hdr->addr3, ETH_ALEN);
	}

	//ehdr.ethertype = (payload[6] << 8) | payload[7];
	os_memcpy(&ehdr.ethertype, payload + sizeof(rfc1042_header), 2);
	rx->offset_8023 = payload - rx->u.frame - HLEN_8023;

	if ((os_memcmp(payload, rfc1042_header, 6) == 0 &&
		(htons(ehdr.ethertype) != ETH_P_AARP || htons(ehdr.ethertype) != ETH_P_IPX))
		|| os_memcmp(payload, bridge_tunnel_header, 6) == 0)
		rx->offset_8023 += 8; ///* remove RFC1042 or Bridge-Tunnel encapsulation
	else
		os_memset(&ehdr.ethertype, 0, 2);

	rx->len -= rx->offset_8023;
	os_memcpy(rx->u.frame + rx->offset_8023, &ehdr, HLEN_8023);
	rx->is_8023 = true;

	return 0;
}

static void nrc_wpa_data_rx(struct nrc_wpa_rx_data *rx)
{
	int res;
#define CALL_RXH(rxh)		\
	do {					\
		res = rxh(rx);		\
		if (res != 0)		\
		do {				\
			goto drop;		\
		} while(0);			\
	} while (0)
	CALL_RXH(nrc_wpa_qos_null);
	CALL_RXH(nrc_wpa_michael_mic_verify);
	CALL_RXH(nrc_wpa_ap_process);
	CALL_RXH(nrc_wpa_to_8023);
	CALL_RXH(nrc_wpa_eapol);
	CALL_RXH(nrc_wpa_lwip);
drop:
	return;
#undef CALL_RXH
}

#if defined (NRC7291_SDK_DUAL_CM0) || defined (NRC7291_SDK_DUAL_CM3)
//TDDO.
int system_memory_pool_number_of_link(struct _SYS_BUF* buf) {return 0;};
void discard(SYS_BUF* buffer) {};
#endif
extern int process_promiscuous_frame(void* rx_head, void* mac_head, uint8_t *frame, uint16_t len);
typedef void (*wifi_sta_rx_probe_req_t)(const uint8_t *frame, int len, int rssi);
static wifi_sta_rx_probe_req_t probe_req_cb = NULL;
int trs_wifi_set_sta_rx_probe_req(wifi_sta_rx_probe_req_t cb)
{
	probe_req_cb = cb;
	return 0;
}
void nrc_mac_rx(SYS_BUF *head)
{
	struct nrc_wpa_rx_data rx;
	GenericMacHeader *gmh = &head->rx_mac_header;
	rx.rxh = &head->lmac_rxhdr;
	rx.rxi = &head->lmac_rxhdr.rxinfo;
#if LMAC_CONFIG_11N == 1
	rx.intf = wpa_driver_get_interface(rx.rxi->received_vif); //TODO
#else
	//rx.intf = wpa_driver_get_interface(1); //Temporary modification for connection
	rx.intf = wpa_driver_get_interface(hal_lmac_get_vif_id(gmh)); //TODO
#endif
	rx.len = rx.rxi->mpdu_length;
	rx.buf = head;
	bool need_free = false;

	if (rx.len < IEEE80211_HDRLEN)
		goto drop;

	if (system_memory_pool_number_of_link(head) > 1) {
		need_free = true;
		rx.u.frame = os_malloc(rx.len);
		if (!rx.u.frame) {
			ASSERT(0);
			goto drop;
		}
		linearize(head, rx.u.frame, P_OFFSET, 0, rx.len);
	} else {
		rx.u.frame = (uint8_t *) head + P_OFFSET;
	}

#if 0
    { // for debug.
        debug_rx_tx_info_t info = {0, rx.intf->vif_id, .u.wifi.rssi = (int8_t)rx.rxh->rssi};
        wifi_dump_frame(rx.u.frame, &info);
    }
#endif
	if(probe_req_cb != NULL) {
		probe_req_cb(rx.u.frame,rx.len,(int8_t)rx.rxh->rssi);
	}

	nrc_sta_status(&rx);
    if (get_promiscuous_mode(0) || get_promiscuous_mode(1)){
         if (!process_promiscuous_frame(rx.rxh, gmh, rx.u.frame, rx.len))
             goto drop;
    } else {
        if (gmh->type == WLAN_FC_TYPE_DATA)
            nrc_wpa_data_rx(&rx);
        else if (gmh->type == WLAN_FC_TYPE_MGMT)
            nrc_wpa_mgmt_rx(gmh->subtype, &rx);
        else
            goto drop;
    }


drop:
	if (need_free && rx.u.frame)
		os_free(rx.u.frame);

	discard(head);
}

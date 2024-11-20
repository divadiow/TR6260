#include "system.h"

#include "utils/includes.h"
#include "utils/common.h"
#include "../common/wpa_common.h"
#include "utils/eloop.h"
#include "utils/wpa_debug.h"
#include "../common/ieee802_11_common.h"
#include "../../wpa_supplicant/config_ssid.h"
#include "hostapd.h"
#include "wpa_supplicant_i.h"
#include "bss.h"

#include "driver.h"
#include "driver_wifi.h"
#include "driver_wifi_scan.h"
#include "driver_wifi_debug.h"

#include "nrc-wim-types.h"
#include "umac_wim_builder.h"


#include "standalone.h"
#include "system_wifi.h"





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

uint8_t nv_bw40_enable = 0;
uint8_t nv_rate_set = 0xff;

void wpa_driver_set_channel_width(uint8_t vif_id, uint8_t ch, uint8_t type)
{
	struct wim_builder *wb = wim_builder_create(vif_id, WIM_CMD_SET, WB_MIN_SIZE);
    uint16_t freq = system_modem_api_channel_to_mac80211_frequency(ch);
	struct wim_channel_param wc = {freq, 0, 0};
    struct wim_channel_width_param wcw = {0,0,{0,0}};

    switch(type)
    {
        case 0: wc.type=1;/*SCN*/ wcw.chan_width=0;/*20M*/ wcw.prim_loc=0; break;
        case 1: wc.type=3;/*SCA*/ wcw.chan_width=1;/*40M*/ wcw.prim_loc=0; break;
		case 3: wc.type=2;/*SCB*/ wcw.chan_width=1;/*40M*/ wcw.prim_loc=1; break;
        default:wc.type=1;/*SCN*/ wcw.chan_width=0;/*20M*/ wcw.prim_loc=0; break;
    }

	wim_builder_append(wb, WIM_TLV_CHANNEL, &wc, sizeof(wc));
	wim_builder_append(wb, WIM_TLV_CHANNEL_WIDTH, &wcw, sizeof(wcw));
	wim_builder_run_wm(wb, true);
}

static const char *const us_op_class_cc[] = {
	"US", "CA", NULL
};

static const char *const eu_op_class_cc[] = {
	"AL", "AM", "AT", "AZ", "BA", "BE", "BG", "BY", "CH", "CY", "CZ", "DE",
	"DK", "EE", "EL", "ES", "FI", "FR", "GE", "HR", "HU", "IE", "IS", "IT",
	"LI", "LT", "LU", "LV", "MD", "ME", "MK", "MT", "NL", "NO", "PL", "PT",
	"RO", "RS", "RU", "SE", "SI", "SK", "TR", "UA", "UK", NULL
};

static const char *const jp_op_class_cc[] = {
	"JP", NULL
};

static const char *const cn_op_class_cc[] = {
	"CN", NULL
};

static int country_match(const char *const cc[], const char *const country)
{
	int i;

	if (country == NULL)
		return 0;
	for (i = 0; cc[i]; i++) {
		if (cc[i][0] == country[0] && cc[i][1] == country[1])
			return 1;
	}

	return 0;
}

static int wifi_driver_nrc_set_country(const char *country)
{
   wifi_country_t info;

   if (!country) 
       return -1;
   
   memset(&info, 0, sizeof(info));
   memcpy(info.cc, country, 2);

   info.schan = 1;
   if (country_match(us_op_class_cc, country)) {
       info.nchan = 11;
   } else if (country_match(eu_op_class_cc, country)) {
       info.nchan = 13;
   } else if (country_match(jp_op_class_cc, country)) {
       info.nchan = 14;
   } else if (country_match(cn_op_class_cc, country)) {
       info.nchan = 13;
   } else {
       info.nchan = 13;
   }

   system_modem_api_set_country_info(&info);
   return 0;
}

static void wpa_driver_get_wifi_nv(void)
{
	int ret = 0;
    uint16_t rate = 0;
	char nv_str[3];

	nv_bw40_enable = 0;
	wifi_country_t wifi_country_tmp;

    // init bandwidth
	ret = ef_get_env_blob(NV_WIFI_BW_MODE, nv_str, 2, NULL);
	if (ret) {
		nv_str[1] = 0;
        ret = atoi(nv_str);
		if(ret==NV_WIFI_BW_MODE_40M || ret==NV_WIFI_BW_MODE_AUTO)
            nv_bw40_enable = 1;		
	}

    // init country code
	// ret = ef_get_env_blob(NV_WIFI_COUNTRY, nv_str, 3, NULL);
	// if (ret) {
    //     wifi_driver_nrc_set_country(nv_str);
    // }
	// ¡ä¨®flash?¨¢¨¨?

	if(ef_get_env_blob(NV_WIFI_COUNTRY, wifi_country_tmp.cc, sizeof(wifi_country_tmp.cc), NULL)) 
	{
			wifi_driver_nrc_set_country(wifi_country_tmp.cc);
		//else
		//return;

		if (ef_get_env_blob(NV_WIFI_START_CHANNEL, &(wifi_country_tmp.schan), sizeof(wifi_country_tmp.schan), NULL) && 
			ef_get_env_blob(NV_WIFI_TOTAL_CHANNEL_NUMBEL, &(wifi_country_tmp.nchan), sizeof(wifi_country_tmp.nchan), NULL)) 
		{
			system_modem_api_set_country_info(&wifi_country_tmp);
		}
	}

    // init rate set
    ret = ef_get_env_blob(NV_WIFI_RATE_SET, &nv_str, 3, NULL);
    if (ret) {
        rate = atoi(nv_str);
        nv_rate_set = rate > 0 ? rate : 0xff;
        system_printf("load rate set 0x%x\n", nv_rate_set);
    }
}

void wpa_driver_wim_run(void *eloop_ctx, void *timeout_ctx);

//TODO:
extern struct wpa_ssid* wpa_get_ssid( struct wpa_supplicant* wpa_s );

#ifdef LTAG
#undef LTAG
#endif
#define LTAG "Drv: "

struct nrc_wpa *nrc_global;

static void wpa_driver_hostapd_logger_timeout(void *eloop_data, void *user_ctx)
{
	struct nrc_wpa_log_event *ev = (struct nrc_wpa_log_event *) user_ctx;
	wpa_printf(ev->level, ev->msg);
	os_free(ev->msg);
	os_free(ev);
}

static void nrc_wpa_hostapd_logger_cb(void *ctx, const u8 *addr,
	unsigned int module, int level, const char *txt, size_t len)
{
#if 0
	struct nrc_wpa_log_event *ev = (struct nrc_wpa_log_event *)
								 os_malloc(sizeof(*ev));
	if (!ev)
		return;

	ev->msg = os_malloc(len + 1);
	os_memcpy(ev->msg, txt, len);
	ev->msg[len] = '\0';
	ev->level = level;

	if (!ev->msg) {
		os_free(ev);
		return;
	}
	eloop_register_timeout(0, 0, wpa_driver_hostapd_logger_timeout, 0, ev);
#endif
}

struct nrc_wpa_if *wpa_driver_get_interface(int vif)
{
	if (nrc_global && nrc_global->intf[vif])
		return nrc_global->intf[vif];
	return NULL;
}

struct nrc_wpa_sta* nrc_wpa_find_sta(struct nrc_wpa_if *intf,
						 const uint8_t addr[ETH_ALEN])
{
	int i = 0;
	struct nrc_wpa_sta *sta = NULL;

	if (!intf->is_ap) {
		return &intf->sta;
	}
	else {
		if (is_multicast_ether_addr(addr))
			return &intf->sta;
		for (i = 0; i < NRC_WPA_SOFTAP_MAX_STA; i++) {
			if (!intf->ap_sta[i])
				continue;

			if (os_memcmp(intf->ap_sta[i]->addr, addr, ETH_ALEN) == 0)
				return intf->ap_sta[i];
		}
	}
	return NULL;
}

static void wpa_driver_scan_done_timeout(void *eloop_data, void *user_ctx)
{
	union wpa_event_data event;
	int vif = (int) user_ctx;
	struct nrc_wpa_if *intf = wpa_driver_get_interface(vif);
	wpa_printf(MSG_DEBUG, "nrc_scan_done..(vif: %d) \n", vif);
	scan_stop(intf->scan);

	os_memset(&event, 0, sizeof(event));
	event.scan_info.aborted = false;
	event.scan_info.nl_scan_event = 1; /* From normal scan */
	wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_SCAN_RESULTS, &event);
}

void nrc_scan_done(int vif)
{
	wpa_printf(MSG_ERROR, "%s", __func__);
	eloop_register_timeout(0, 0, wpa_driver_scan_done_timeout, 0, (void *) vif);
}

static void wpa_driver_set_bssid_wim(int vif_id, uint8_t* bssid)
{
	struct wim_builder *wb = NULL;
	wb = wim_builder_create(vif_id, WIM_CMD_SET, WB_MIN_SIZE);
	wim_builder_append(wb, WIM_TLV_BSSID, bssid, ETH_ALEN);
	wim_builder_run_wm(wb, true);
}

/**
 * get_bssid - Get the current BSSID
 * @priv: private driver interface data
 * @bssid: buffer for BSSID (ETH_ALEN = 6 bytes)
 *
 * Returns: 0 on success, -1 on failure
 *
 * Query kernel driver for the current BSSID and copy it to bssid.
 * Setting bssid to 00:00:00:00:00:00 is recommended if the STA is not
 * associated.
 */
static int wpa_driver_nrc_get_bssid(void *priv, u8 *bssid)
{
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);

	if (!intf->associated) {
		os_memset(bssid, 0, ETH_ALEN);
		return 0;
	}
	os_memcpy(bssid, intf->bss.bssid, ETH_ALEN);

	return 0;
}

int nrc_get_sec_hdr_len(struct nrc_wpa_key *key)
{
	if (!key)
		return 0;

	switch (key->cipher) {
		case WIM_CIPHER_TYPE_WEP40:
		case WIM_CIPHER_TYPE_WEP104:
		return 4;
		case WIM_CIPHER_TYPE_TKIP:
		case WIM_CIPHER_TYPE_CCMP:
		return 8;
	}
	return 0;
}

int nrc_get_channel_list(struct nrc_wpa_if *intf, uint16_t chs[], const int MAX_CHS)
{
	int i = 0;

	const uint16_t *sys_chs;
	int sys_n_chs = 0;

	system_api_get_supported_channels(&sys_chs, &sys_n_chs);

	for (i = 0; i < sys_n_chs; i++) {
		chs[i] = sys_chs[i];
	}
	return i;
}

/**
 * get_ssid - Get the current SSID
 * @priv: private driver interface data
 * @ssid: buffer for SSID (at least 32 bytes)
 *
 * Returns: Length of the SSID on success, -1 on failure
 *
 * Query kernel driver for the current SSID and copy it to ssid.
 * Returning zero is recommended if the STA is not associated.
 *
 * Note: SSID is an array of octets, i.e., it is not nul terminated and
 * can, at least in theory, contain control characters (including nul)
 * and as such, should be processed as binary data, not a printable
 * string.
 */
static int wpa_driver_nrc_get_ssid(void *priv, u8 *ssid)
{
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);

	if(!intf->associated) {
		wpa_printf(MSG_DEBUG, LTAG "%s() Not associated.", __func__);
		return -1;
	}
	os_memcpy(ssid, intf->bss.ssid, intf->bss.ssid_len);

	return intf->bss.ssid_len;
}

static int wpa_driver_nrc_get_aid(void *priv, uint16_t *aid)
{
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);

	if(!intf->associated) {
		wpa_printf(MSG_DEBUG, LTAG "%s() Not associated.", __func__);
		return -1;
	}
	*aid = intf->sta.aid;
	wpa_printf(MSG_ERROR, LTAG " %s() AID = %d \n", __func__, *aid);

	return 0;
}

static uint16_t ecw2cw(uint16_t ecw) {
	return (1 << ecw) - 1;
}
#if 0
static int wpa_driver_nrc_wim_tx_param(const struct chanAccParams *in,
				const int ac, struct wim_tx_queue_param *out) {
	if (ac >= WME_NUM_AC)
		return -1;

	out->ac = ac;
	out->txop = in->cap_wmeParams[ac].wmep_txopLimit;
	out->aifsn = in->cap_wmeParams[ac].wmep_aifsn;
	out->cw_min = ecw2cw(in->cap_wmeParams[ac].wmep_logcwmin);
	out->cw_max = ecw2cw(in->cap_wmeParams[ac].wmep_logcwmax);
	out->uapsd = 0;
	out->sta_type = 0;

	return 0;
}
#endif
static int wpa_driver_nrc_get_listen_interval() {
	struct wim_cap_param param;
	system_modem_api_get_capabilities(&param);
	return param.listen_interval;
}

enum wim_cipher_type nrc_to_wim_cipher_type(enum wpa_alg alg, int key_len)
{
	switch (alg) {
		case WPA_ALG_NONE:
		return WIM_CIPHER_TYPE_NONE;
		case WPA_ALG_WEP:
		return key_len == 5 ? WIM_CIPHER_TYPE_WEP40 : WIM_CIPHER_TYPE_WEP104;
		case WPA_ALG_TKIP:
		return WIM_CIPHER_TYPE_TKIP;
		case WPA_ALG_CCMP:
		return WIM_CIPHER_TYPE_CCMP;
		default:
		return WIM_CIPHER_TYPE_INVALID;
	}
}

static void wpa_clear_key(struct nrc_wpa_key *key)
{
	os_memset(key, 0, sizeof(struct nrc_wpa_key));
	key->cipher = WIM_CIPHER_TYPE_NONE;
}

static int wpa_driver_nrc_set_key_wim(struct nrc_wpa_if *intf,
			int cipher, const u8 *addr,
			int key_idx, const u8 *key, size_t key_len)
{
	int vif_id = 0, aid = 0;
	struct wim_builder *wb = NULL;
	struct wim_key_param param;
	int key_cmd = WIM_CMD_SET_KEY;

	if (cipher == WIM_CIPHER_TYPE_NONE)
		key_cmd = WIM_CMD_DISABLE_KEY;

	wb = wim_builder_create(intf->vif_id, key_cmd, WB_MIN_SIZE);

	param.cipher_type 	= cipher;
	param.key_index 	= key_idx;

	if (addr)
		os_memcpy(param.mac_addr, addr, ETH_ALEN);
	else
		os_memset(param.mac_addr, 0x0, ETH_ALEN);

	if (intf->is_ap && is_broadcast_ether_addr(addr))
		os_memcpy(param.mac_addr, intf->addr, ETH_ALEN);

	param.aid			= 0; // SHOULD FIX On Soft AP

	if (!addr || is_broadcast_ether_addr(addr))
		param.key_flags = WIM_KEY_FLAG_GROUP;
	else
		param.key_flags = WIM_KEY_FLAG_PAIRWISE;

	param.key_len 		= key_len;
	os_memcpy(param.key, key, key_len);

	wim_builder_append(wb, WIM_TLV_KEY_PARAM, &param, sizeof(param));
	wim_builder_run_wm(wb, true);

	return 0;
}

static int wpa_driver_nrc_remove_key_wim(struct nrc_wpa_if *intf, struct nrc_wpa_key *k)
{
	k->is_set = false;
	return wpa_driver_nrc_set_key_wim(intf, WIM_CIPHER_TYPE_NONE, k->addr, k->ix,
			k->key, k->key_len);
}

static int wpa_driver_nrc_set_key_wim2(struct nrc_wpa_if *intf, struct nrc_wpa_key *k)
{
	if (wpa_driver_nrc_set_key_wim(intf, k->cipher, k->addr, k->ix,
			k->key, k->key_len) == 0) {
		k->is_set = true;
		return 0;
	}
	return -1;
}

static void wpa_driver_sta_type_wim(struct nrc_wpa_if *intf, uint32_t sta_type)
{
	struct wim_builder *wb = wim_builder_create(intf->vif_id, WIM_CMD_SET, WB_MIN_SIZE);
	intf->sta_type = sta_type;
	wim_builder_append_u32(wb, WIM_TLV_STA_TYPE, intf->sta_type);
	wim_builder_run_wm(wb, true);
}
/**
 * set_key - Configure encryption key
 * @ifname: Interface name (for multi-SSID/VLAN support)
 * @priv: private driver interface data
 * @alg: encryption algorithm (%WPA_ALG_NONE, %WPA_ALG_WEP,
 *	%WPA_ALG_TKIP, %WPA_ALG_CCMP, %WPA_ALG_IGTK, %WPA_ALG_PMK,
 *	%WPA_ALG_GCMP, %WPA_ALG_GCMP_256, %WPA_ALG_CCMP_256,
 *	%WPA_ALG_BIP_GMAC_128, %WPA_ALG_BIP_GMAC_256,
 *	%WPA_ALG_BIP_CMAC_256);
 *	%WPA_ALG_NONE clears the key.
 * @addr: Address of the peer STA (BSSID of the current AP when setting
 *	pairwise key in station mode), ff:ff:ff:ff:ff:ff for
 *	broadcast keys, %NULL for default keys that are used both for
 *	broadcast and unicast; when clearing keys, %NULL is used to
 *	indicate that both the broadcast-only and default key of the
 *	specified key index is to be cleared
 * @key_idx: key index (0..3), usually 0 for unicast keys; 0..4095 for
 *	IGTK
 * @set_tx: configure this key as the default Tx key (only used when
 *	driver does not support separate unicast/individual key
 * @seq: sequence number/packet number, seq_len octets, the next
 *	packet number to be used for in replay protection; configured
 *	for Rx keys (in most cases, this is only used with broadcast
 *	keys and set to zero for unicast keys); %NULL if not set
 * @seq_len: length of the seq, depends on the algorithm:
 *	TKIP: 6 octets, CCMP/GCMP: 6 octets, IGTK: 6 octets
 * @key: key buffer; TKIP: 16-byte temporal key, 8-byte Tx Mic key,
 *	8-byte Rx Mic Key
 * @key_len: length of the key buffer in octets (WEP: 5 or 13,
 *	TKIP: 32, CCMP/GCMP: 16, IGTK: 16)
 *
 * Returns: 0 on success, -1 on failure
 *
 * Configure the given key for the kernel driver. If the driver
 * supports separate individual keys (4 default keys + 1 individual),
 * addr can be used to determine whether the key is default or
 * individual. If only 4 keys are supported, the default key with key
 * index 0 is used as the individual key. STA must be configured to use
 * it as the default Tx key (set_tx is set) and accept Rx for all the
 * key indexes. In most cases, WPA uses only key indexes 1 and 2 for
 * broadcast keys, so key index 0 is available for this kind of
 * configuration.
 *
 * Please note that TKIP keys include separate TX and RX MIC keys and
 * some drivers may expect them in different order than wpa_supplicant
 * is using. If the TX/RX keys are swapped, all TKIP encrypted packets
 * will trigger Michael MIC errors. This can be fixed by changing the
 * order of MIC keys by swapping te bytes 16..23 and 24..31 of the key
 * in driver_*.c set_key() implementation, see driver_ndis.c for an
 * example on how this can be done.
 */

struct nrc_wpa_key *nrc_wpa_get_key(struct nrc_wpa_if *intf, const uint8_t *addr)
{
	struct nrc_wpa_sta *wpa_sta = NULL;
	struct nrc_wpa_key *b_key = &intf->bss.broadcast_key;

	if (!addr || is_multicast_ether_addr(addr))
		return b_key;

	if (intf->is_ap && is_key_wep(b_key))
		return b_key;

	if (!intf->is_ap)
		return &intf->sta.key;

	wpa_sta = nrc_wpa_find_sta(intf, addr);

	if (!wpa_sta) {
			wpa_printf(MSG_ERROR, LTAG "Failed to find sta (" MACSTR ") "
				"(Key)",
				MAC2STR(addr));
			return NULL;
	}
	return &wpa_sta->key;
}
#ifdef CONFIG_IEEE80211W
struct nrc_wpa_key		igtk;
#endif
static int wpa_driver_nrc_set_key(const char *ifname, void *priv, enum wpa_alg alg,
	const u8 *addr, int key_idx, int set_tx,
	const u8 *seq, size_t seq_len,
	const u8 *key, size_t key_len)
{
	int ret = 0;
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	bool is_remove = (alg == WPA_ALG_NONE || (!is_wep(alg) && addr == NULL));
	struct nrc_wpa_key *wk = nrc_wpa_get_key(intf, addr);
    uint8_t bssid[ETH_ALEN];

    
	wpa_driver_nrc_get_bssid(intf, bssid);
    if (!intf->is_ap) {
        if (alg == WPA_ALG_WEP && addr == NULL) { // set wep key, use sta key.
            wk = &intf->sta.key;
            os_memcpy(wk->addr, bssid, ETH_ALEN);
        }

        if (alg == WPA_ALG_NONE && addr == NULL) { //del wep key, find wep key to del it.
            if (intf->sta.key.cipher == WIM_CIPHER_TYPE_WEP40 || intf->sta.key.cipher == WIM_CIPHER_TYPE_WEP104)
                wk = &intf->sta.key;
        }
    }
    
	wpa_printf(MSG_DEBUG, LTAG "%s(vif: %d, mac %s, alg %s, idx %d, "
				"set_tx %d seq. len : %d, key. len : %d)", __func__, intf->vif_id,
				addr ? "NOT_NULL" : "NULL", wpa_driver_alg_str(alg), key_idx,
				set_tx, (int)seq_len, (int)key_len);
	if (addr)
		wpa_printf(MSG_DEBUG, LTAG "%s : MAC " MACSTR "", __func__, MAC2STR(addr));

	if (!wk)
		return 0;

	if (is_remove && !wk->is_set)
		return 0;
#ifdef CONFIG_IEEE80211W
	if(nrc_to_wim_cipher_type(alg, key_len) == WIM_CIPHER_TYPE_INVALID)
	{
		wpa_printf(MSG_DEBUG, "WIM_CIPHER_TYPE_INVALID (alg:%s)\r\n", wpa_driver_alg_str(alg));
		if(alg == WPA_ALG_IGTK)
		{
			intf->pmf = 1;
			igtk.tsc = 0;
			igtk.ix = key_idx;
			igtk.key_len = key_len;
			os_memcpy(igtk.key, key, key_len);
		}
		return 0;
	}
#endif

	wk->cipher = nrc_to_wim_cipher_type(alg, key_len);
	wk->ix = key_idx;
	wk->key_len = key_len;
    if (addr)
    	os_memcpy(wk->addr, addr, ETH_ALEN);
	os_memcpy(wk->key, key, wk->key_len);

	if (alg == WPA_ALG_WEP) {
		wpa_driver_set_bssid_wim(intf->vif_id, bssid); /* BSSID should be set before set_key */
        if (intf->is_ap)
            os_memcpy(wk->addr, bssid, ETH_ALEN);
	}

	if (wk->is_set)
		wpa_driver_nrc_remove_key_wim(intf, wk);

	return wpa_driver_nrc_set_key_wim2(intf, wk);
}

/**
 * deinit - Deinitialize driver interface
 * @priv: private driver interface data from init()
 *
 * Shut down driver interface and processing of driver events. Free
 * private data buffer if one was allocated in init() handler.
 */
 static void wpa_driver_nrc_deinit(void *priv) {
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	wpa_printf(MSG_DEBUG, LTAG "%s(vif: %d)", __func__, intf->vif_id);
	scan_deinit(intf->scan);
 }

/**
 * set_param - Set driver configuration parameters
 * @priv: private driver interface data from init()
 * @param: driver specific configuration parameters
 *
 * Returns: 0 on success, -1 on failure
 *
 * Optional handler for notifying driver interface about configuration
 * parameters (driver_param).
 */
 static int wpa_driver_nrc_set_param(void *priv, const char *param) {
	if(param == NULL)
		return 0;
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	wpa_printf(MSG_DEBUG, LTAG "%s(vif: %d, param: %s)", __func__, intf->vif_id, param);

	return 0;
 }

/**
 * set_countermeasures - Enable/disable TKIP countermeasures
 * @priv: private driver interface data
 * @enabled: 1 = countermeasures enabled, 0 = disabled
 *
 * Returns: 0 on success, -1 on failure
 *
 * Configure TKIP countermeasures. When these are enabled, the driver
 * should drop all received and queued frames that are using TKIP.
 */
 static int wpa_driver_nrc_set_countermeasures(void *priv, int enabled){
 	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
 	wpa_printf(MSG_DEBUG, LTAG "%s(vif: %d, enabled=%d)",
 		__func__, intf->vif_id, enabled);
	return -1;
 }

 static int send_deauthenticate(struct nrc_wpa_if *intf, const u8 *a1,
 				const u8 *a2, const u8 *a3, int reason_code)
 {
 	struct ieee80211_mgmt mgmt = {0,};
	const int len = offsetof(struct ieee80211_mgmt, u.deauth.variable);

	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_DEAUTH);
	mgmt.u.deauth.reason_code = reason_code;
#ifdef CONFIG_IEEE80211W
	if(intf->pmf)
		mgmt.frame_control |= WLAN_FC_ISWEP;
#endif    
	os_memcpy(mgmt.da, a1, ETH_ALEN);
	os_memcpy(mgmt.sa, a2, ETH_ALEN);
	os_memcpy(mgmt.bssid, a3, ETH_ALEN);

	return nrc_transmit(intf, (uint8_t *) &mgmt, len);
 }

/**
 * deauthenticate - Request driver to deauthenticate
 * @priv: private driver interface data
 * @addr: peer address (BSSID of the AP)
 * @reason_code: 16-bit reason code to be sent in the deauthentication
 *	frame
 *
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_deauthenticate(void *priv, const u8 *addr, int reason_code)
{
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	int i = 0;

	wpa_printf(MSG_DEBUG, LTAG "%s(addr=" MACSTR ",reason=%d)", __func__,
				MAC2STR(addr), reason_code);

	if(!intf)  {
		wpa_printf(MSG_DEBUG, LTAG "%s unexpected error", __func__);
		return -1;
	}

	if (!intf->associated)
		return 0;

	if (!intf->is_ap) {
		send_deauthenticate(intf, addr, intf->addr, addr, reason_code);
		wpa_driver_sta_sta_remove(intf);
        hal_mac_revert_rate_set();
	} else {
		// Deauthenticate all connected STAs, AP entitiy send deauthuntication
		// frame though send_mgmt() indivisually otherwise.
		for (i = 0; i < NRC_WPA_SOFTAP_MAX_STA; i++) {
			if (intf->ap_sta[i])
				send_deauthenticate(intf, intf->ap_sta[i]->addr,
					intf->addr, intf->addr, reason_code);
		}
	}

	return 0;
}

static bool wpa_driver_is_privacy(struct wpa_driver_associate_params *p) {
	return !(!p->wpa_ie
		&& p->pairwise_suite == WPA_CIPHER_NONE
		&& p->group_suite == WPA_CIPHER_NONE
		&& p->key_mgmt_suite == WPA_KEY_MGMT_NONE);
}

static int wpa_driver_eid_ssid(uint8_t* eid, const uint8_t* ssid, uint8_t ssid_len) {
	*(eid++) = WLAN_EID_SSID;
	*(eid++) = ssid_len;
	if (ssid)
		os_memcpy(eid, ssid, ssid_len);
	return 2 + ssid_len;
}

static int to_rateie(int rate, bool basic) {
	return (rate / 5) | (basic ? 0x80 : 0x0);
}

static inline int rate_2_index(uint16_t rate)
{
    const int rate_table[12] = {10, 20, 55, 110, 180, 240, 360, 540, 60, 90, 120, 480}; 
    int i = 0;

    for (;i < ARRAY_SIZE(rate_table); ++i) {
        if (rate_table[i] == rate)
            return i;
    }

    return -1;
}

static int wpa_driver_update_rate_set(const uint8_t *bss_rate_ie, const uint8_t *bss_ext_rate_ie, 
    const uint8_t *bss_ht_cap_ie, uint8_t *mcs_set)
{
    rate_set_t rate_set;
    int i = 0, index;
    struct ieee80211_ht_capabilities *ht_cap = NULL;

    memset(&rate_set, 0, sizeof(rate_set));
    if (bss_rate_ie) {
        for (i = 0; i < bss_rate_ie[1]; ++i) {
            if ((index = rate_2_index(5 * (bss_rate_ie[i + 2] & 0x7f))) >= 0)
                rate_set.rate |= 1 << index;
        }
    }

    if (bss_ext_rate_ie) {
        for (i = 0; i < bss_ext_rate_ie[1]; ++i) {
            if ((index = rate_2_index(5 * (bss_ext_rate_ie[i + 2] & 0x7f))) >= 0)
                rate_set.rate |= 1 << index;
        }
    }

    if (bss_ht_cap_ie && bss_ht_cap_ie[1] > 6) {
        ht_cap = (struct ieee80211_ht_capabilities *)(bss_ht_cap_ie + 2);
        rate_set.mcs = ht_cap->supported_mcs_set[0] & nv_rate_set;
        *mcs_set = rate_set.mcs;
    }

    system_printf("---set rate--0x%02x:0x%04x\n", rate_set.mcs, rate_set.rate);
    //call lmac to update rate
    hal_mac_update_rate_set(&rate_set);

    return 0;
}
extern uint8_t  g_rc_format_nv;
static int wpa_driver_eid_supp_rates(struct nrc_wpa_if *intf, uint8_t *pos, 
    const uint8_t *bss_rate_ie, const uint8_t *bss_ext_rate_ie)
{
    int len = 0;
    int l1 = bss_rate_ie ? (2 + bss_rate_ie[1]) : 0;
    int l2 = bss_ext_rate_ie ? (2 + bss_ext_rate_ie[1]) : 0;
    uint8_t dss_rate[6]={0,0,0x82,0x84,0x8B,0x96};
    uint8_t nonht_rate[10]={0};

    if(g_rc_format_nv == RC_FORMAT_11B_ONLY_NV) 
    {
        if(l1>6)
        {
            memcpy(dss_rate,bss_rate_ie,2);
            dss_rate[1] = 4;
            len = sizeof(dss_rate);
            memcpy(pos, dss_rate, len);
        }
        else
        {
            len =l1;
            memcpy(pos, bss_rate_ie, len);
        }
            
    }
    else /*if(g_rc_format_nv == RC_FORMAT_11BGN_MIX_NV)*/
    {
        if (l1 > 0) {
            memcpy(pos, bss_rate_ie, l1);
        }

        if (l2 > 0) {
            memcpy(pos + l1, bss_ext_rate_ie, l2);
        }
        len = l1 + l2;
    }
    /*else if(g_rc_format_nv == RC_FORMAT_11G_ONLY_NV) 
    {
        nonht_rate[0] = bss_ext_rate_ie[0];
        nonht_rate[1] = 10;
        nonht_rate[2] = bss_rate_ie[5];
        nonht_rate[3] = bss_rate_ie[6];
        nonht_rate[4] = bss_rate_ie[8];
        nonht_rate[5] = bss_rate_ie[9];
        memcpy(nonht_rate+6,bss_ext_rate_ie+2,4);
        len = sizeof(nonht_rate);
        memcpy(pos,nonht_rate,len);
       
    }*/

	return len;
}

static int wpa_driver_wmm_elem(struct nrc_wpa_if *intf, uint8_t *eid)
{
	uint8_t* pos = eid;
	*pos++ = WLAN_EID_VENDOR_SPECIFIC;
	*pos++ = 7;
	*pos++ = 0x00; *pos++ = 0x50; *pos++ = 0xf2; // OUI_MICROSOFT
	*pos++ = WMM_OUI_TYPE;
	*pos++ = WMM_OUI_SUBTYPE_INFORMATION_ELEMENT;
	*pos++ = WMM_VERSION;
	*pos++ = 0;

	return (pos - eid);
}

static int wpa_driver_nrc_sta_associate(struct nrc_wpa_if *intf,
							struct wpa_driver_associate_params *p)
{
	uint8_t *frame = os_malloc(512);
	struct ieee80211_ht_capabilities *htcap = (struct ieee80211_ht_capabilities *)
							p->htcaps;
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *) frame;
	const int ie_offset = offsetof(struct ieee80211_mgmt, u.assoc_req.variable);
	uint8_t* ie = frame + ie_offset;
	uint8_t* ie_pos = ie;
    uint8_t rate_set = 0xff;

	if (!frame) {
		wpa_printf(MSG_ERROR, LTAG "Failed to allociate assoc req frame");
		return -1;
	}

	wpa_printf(MSG_DEBUG, LTAG "%s (vif: %d)", __func__, intf->vif_id);

	mgmt->frame_control = _FC(MGMT,ASSOC_REQ);
	mgmt->u.assoc_req.capab_info = WLAN_CAPABILITY_ESS;
	mgmt->u.assoc_req.listen_interval = 100; /* TODO */

	os_memcpy(mgmt->da,		p->bssid, ETH_ALEN);
	os_memcpy(mgmt->sa,		intf->addr, ETH_ALEN);
	os_memcpy(mgmt->bssid,	p->bssid, ETH_ALEN);

	if (wpa_driver_is_privacy(p)) {
		mgmt->u.assoc_req.capab_info |= WLAN_CAPABILITY_PRIVACY;
    }
	
	struct wpa_supplicant  *wpa_s = (struct wpa_supplicant *)intf->wpa_supp_ctx;
	if(wpa_s->current_bss->caps & WLAN_CAPABILITY_SHORT_PREAMBLE){
		mgmt->u.assoc_req.capab_info |= WLAN_CAPABILITY_SHORT_PREAMBLE;
	}
		
	if(wpa_s->current_bss->caps & WLAN_CAPABILITY_SHORT_SLOT_TIME){
		mgmt->u.assoc_req.capab_info |= WLAN_CAPABILITY_SHORT_SLOT_TIME;
	}

	ie_pos += wpa_driver_eid_ssid(ie_pos, p->ssid, p->ssid_len);

    //set rate to lmac
    wpa_driver_update_rate_set(p->bss_rate_ie, p->bss_ext_rate_ie, p->bss_ht_cap_ie, &rate_set);

    //support rate ie
    if (p->bss_rate_ie) {
	    ie_pos += wpa_driver_eid_supp_rates(intf, ie_pos, p->bss_rate_ie, p->bss_ext_rate_ie);
    }

	if (p->wpa_ie) {
		os_memcpy(ie_pos, p->wpa_ie, p->wpa_ie_len);
		ie_pos += p->wpa_ie_len;
	}

	if ((htcap)
        &&((g_rc_format_nv == RC_FORMAT_11BGN_MIX_NV)
        ||(g_rc_format_nv == RC_FORMAT_11N_ONLY_NV))) {
		const int htcap_len = sizeof(struct ieee80211_ht_capabilities);
		htcap->a_mpdu_params = 0x17; // TODO:
		htcap->supported_mcs_set[0] = rate_set; // TODO
		*ie_pos++ = WLAN_EID_HT_CAP;
		*ie_pos++ = htcap_len;
		os_memcpy(ie_pos, (uint8_t *) htcap, htcap_len);
		ie_pos += htcap_len;
	}

	ie_pos += wpa_driver_wmm_elem(intf, ie_pos);

	nrc_transmit(intf, frame, ie_offset + (ie_pos - ie));
	os_free(frame);

	return 0;
}

/**
 * associate - Request driver to associate
 * @priv: private driver interface data
 * @params: association parameters
 *
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_associate(void *priv, struct wpa_driver_associate_params *p) {
	struct nrc_wpa_if* intf = (struct nrc_wpa_if *)(priv);

	//wpa_printf(MSG_DEBUG, LTAG "%s(%p)", __func__, priv);
	wpa_printf(MSG_DEBUG, LTAG "[time]<assoc>");
	//wpa_driver_debug_assoc_params(p);

	if (!(p->mode == IEEE80211_MODE_INFRA ||
		  p->mode == IEEE80211_MODE_AP))
		return -1; // Only STA and AP modes are supported.

	intf->is_ap = (p->mode == IEEE80211_MODE_AP);

	intf->bss.ssid_len = p->ssid_len;
	intf->bss.beacon_int = p->beacon_int;
	os_memcpy(intf->bss.ssid, p->ssid, p->ssid_len);

	if (intf->is_ap)
		// wpa_driver_associate_params::BSSID is null,
		os_memcpy(intf->bss.bssid, intf->addr, ETH_ALEN);
	else
		os_memcpy(intf->bss.bssid, p->bssid, ETH_ALEN);

	if (intf->is_ap) {
		intf->associated = true;

		if (intf->sta_type != WIM_STA_TYPE_AP) {
			intf->sta_type = WIM_STA_TYPE_AP;
			wpa_driver_sta_type_wim(intf, intf->sta_type);
		}
	} else {
		wpa_driver_nrc_sta_associate(intf, p);
	}

	//if (intf->is_ap)
	//	hostapd_logger_register_cb(nrc_wpa_hostapd_logger_cb);

	return 0;
}

/**
 * remove_pmkid - Remove PMKSA cache entry to the driver
 * @priv: private driver interface data
 * @bssid: BSSID for the PMKSA cache entry
 * @pmkid: PMKID for the PMKSA cache entry
 *
 * Returns: 0 on success, -1 on failure
 *
 * This function is called when the supplicant drops a PMKSA cache
 * entry for any reason.
 *
 * If the driver generates RSN IE, i.e., it does not use wpa_ie in
 * associate(), remove_pmkid() can be used to synchronize PMKSA caches
 * between the driver and wpa_supplicant. If the driver uses wpa_ie
 * from wpa_supplicant, this driver_ops function does not need to be
 * implemented. Likewise, if the driver does not support WPA, this
 * function is not needed.
 */
 static int wpa_driver_nrc_remove_pmkid(void *priv, const u8 *bssid, const u8 *pmkid){
	wpa_printf(MSG_DEBUG, LTAG "%s(%p)", __func__, priv);
	return -1;
 }

/**
 * get_capa - Get driver capabilities
 * @priv: private driver interface data
 *
 * Returns: 0 on success, -1 on failure
 *
 * Get driver/firmware/hardware capabilities.
 */
 static int wpa_driver_nrc_get_capa(void *priv, struct wpa_driver_capa *capa) {
 	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);

	wpa_printf(MSG_DEBUG, LTAG "%s(vif: %d)", __func__, intf->vif_id);

	os_memset(capa, 0, sizeof(*capa));

	capa->key_mgmt = (WPA_DRIVER_CAPA_KEY_MGMT_WPA |
						WPA_DRIVER_CAPA_KEY_MGMT_WPA2 |
						WPA_DRIVER_CAPA_KEY_MGMT_WPA_PSK |
						WPA_DRIVER_CAPA_KEY_MGMT_WPA2_PSK);

	capa->enc 		= (WPA_DRIVER_CAPA_ENC_WEP40 |
						WPA_DRIVER_CAPA_ENC_WEP104 |
						WPA_DRIVER_CAPA_ENC_TKIP |
						WPA_DRIVER_CAPA_ENC_CCMP);

	capa->auth = WPA_DRIVER_AUTH_OPEN;

	capa->flags = (WPA_DRIVER_FLAGS_AP
					| WPA_DRIVER_FLAGS_PROBE_RESP_OFFLOAD
					| WPA_DRIVER_FLAGS_SME
 					| WPA_DRIVER_FLAGS_SAE
					| WPA_DRIVER_FLAGS_SET_KEYS_AFTER_ASSOC_DONE /* for WEP */ );

	capa->max_scan_ssids  = 1;

	return 0;
 }

/**
 * get_ifname - Get interface name
 * @priv: private driver interface data
 *
 * Returns: Pointer to the interface name. This can differ from the
 * interface name used in init() call. Init() is called first.
 *
 * This optional function can be used to allow the driver interface to
 * replace the interface name with something else, e.g., based on an
 * interface mapping from a more descriptive name.
 */
 static const char* wpa_driver_nrc_get_ifname(void *priv)
 {
	struct nrc_wpa_if* intf = (struct nrc_wpa_if *)(priv);
	return intf->if_name;
 }

/**
 * get_mac_addr - Get own MAC address
 * @priv: private driver interface data
 *
 * Returns: Pointer to own MAC address or %NULL on failure
 *
 * This optional function can be used to get the own MAC address of the
 * device from the driver interface code. This is only needed if the
 * l2_packet implementation for the OS does not provide easy access to
 * a MAC address.
 */
 static const u8* wpa_driver_nrc_get_mac_addr(void *priv) {
 	struct nrc_wpa_if* intf = (struct nrc_wpa_if *)(priv);
	return intf->addr;
 }

/**
 * set_operstate - Sets device operating state to DORMANT or UP
 * @priv: private driver interface data
 * @state: 0 = dormant, 1 = up
 * Returns: 0 on success, -1 on failure
 *
 * This is an optional function that can be used on operating systems
 * that support a concept of controlling network device state from user
 * space applications. This function, if set, gets called with
 * state = 1 when authentication has been completed and with state = 0
 * when connection is lost.
 */
 static int wpa_driver_nrc_set_operstate(void *priv, int state) {
	wpa_printf(MSG_DEBUG, LTAG "%s(%p,state=%d)", __func__,priv,state);
	return 0;
 }

/**
 * mlme_setprotection - MLME-SETPROTECTION.request primitive
 * @priv: Private driver interface data
 * @addr: Address of the station for which to set protection (may be
 * %NULL for group keys)
 * @protect_type: MLME_SETPROTECTION_PROTECT_TYPE_*
 * @key_type: MLME_SETPROTECTION_KEY_TYPE_*
 * Returns: 0 on success, -1 on failure
 *
 * This is an optional function that can be used to set the driver to
 * require protection for Tx and/or Rx frames. This uses the layer
 * interface defined in IEEE 802.11i-2004 clause 10.3.22.1
 * (MLME-SETPROTECTION.request). Many drivers do not use explicit
 * set protection operation; instead, they set protection implicitly
 * based on configured keys.
 */
 static int wpa_driver_nrc_mlme_setprotection(void *priv, const u8 *addr, int protect_type, int wpa_driver_nrc_key_type){
	return 0;
 }

#define DECLARE_CHANNEL(p,i,c,f,l,m,n)  (p+i)->chan = c; \
			(p+i)->freq = f; \
			(p+i)->flag = l; \
			(p+i)->max_tx_power = m; \
			(p+i)->min_nf = n;
/**
 * get_hw_feature_data - Get hardware support data (channels and rates)
 * @priv: Private driver interface data
 * @num_modes: Variable for returning the number of returned modes
 * flags: Variable for returning hardware feature flags
 * Returns: Pointer to allocated hardware data on success or %NULL on
 * failure. Caller is responsible for freeing this.
 */
static struct hostapd_hw_modes*
wpa_driver_nrc_get_hw_feature_data(void *priv, u16 *num_modes, u16 *flags) {
	struct nrc_wpa_if* intf = (struct nrc_wpa_if *)(priv);
	int i = 0, j = 0, k = 0, size = 0, n_chs = 0, max_tx_power = 20, min_nf = 48;
	struct hostapd_hw_modes* hw_modes = NULL;
	const uint16_t *chs = NULL;

	/* TODO: The below should be retrieved from F/W */
	int rates_a[] = { 60, 120, 240, -1 };
	int rates_b[] = { 10, 20, 55, 110, -1 };
	//int rates_g[] = {10, 20, 55, 110, 60, 90, 120, 180, 240, 360, 480, 540, -1};
	int rates_g[] = {10, 20, 55, 110, 60, 90, 120, 180, 240, 360, 480, 540, -1};
	int *rates = NULL;

	wpa_printf(MSG_DEBUG, LTAG "%s(vif: %d, mode: %d, flags: 0x%2X)", __func__,
		intf->vif_id, *num_modes, *flags);
	system_api_get_supported_channels(&chs, &n_chs);

	if (chs[0] > 4000) {
		*num_modes = 1;
		hw_modes = (struct hostapd_hw_modes *) os_zalloc(sizeof(*hw_modes) *
									(*num_modes));
		hw_modes[0].mode = HOSTAPD_MODE_IEEE80211A;
		hw_modes[0].num_channels = n_chs;
	} else {
		*num_modes = 2;
		hw_modes = (struct hostapd_hw_modes *) os_zalloc(sizeof(*hw_modes) *
									(*num_modes));
		hw_modes[0].mode = HOSTAPD_MODE_IEEE80211B;
		hw_modes[1].mode = HOSTAPD_MODE_IEEE80211G;
		hw_modes[0].num_channels = hw_modes[1].num_channels = n_chs;
	}
	for (i = 0; i < *num_modes; i++) {
		hw_modes[i].channels = os_zalloc(sizeof(*hw_modes[i].channels)
								* hw_modes[i].num_channels);

		for (j = 0; j < n_chs; j++) {
			uint8_t ieee;
			if (ieee80211_freq_to_chan(chs[j], &ieee) == NUM_HOSTAPD_MODES)
				return NULL;
			DECLARE_CHANNEL(hw_modes[i].channels, j, ieee, chs[j], 80,
						max_tx_power, min_nf);
		}

		switch (hw_modes[i].mode) {
			case HOSTAPD_MODE_IEEE80211B:
			rates = rates_b;
			break;
			case HOSTAPD_MODE_IEEE80211A:
			rates = rates_a;
			break;
			case HOSTAPD_MODE_IEEE80211G:
			rates = rates_g;
			break;
			default:
			wpa_printf(MSG_ERROR, LTAG "Failed to set rate due to wrong mode (%d)",
				hw_modes[i].mode);
			return NULL;
		}

		k = 0;
		while (rates[k++] >= 0)
			hw_modes[i].num_rates++;
		size = sizeof(int) * hw_modes[i].num_rates;
		hw_modes[i].rates = (int *) os_zalloc(size);
		os_memcpy(hw_modes[i].rates, rates, size);

		if (hw_modes[i].mode == HOSTAPD_MODE_IEEE80211A ||
			hw_modes[i].mode == HOSTAPD_MODE_IEEE80211G) {
			hw_modes[i].ht_capab		= HT_CAP_INFO_SHORT_GI20MHZ;
			hw_modes[i].mcs_set[0]		= 0xff;
			hw_modes[i].mcs_set[1]		= 0x0;
			hw_modes[i].a_mpdu_params	= 0;
			hw_modes[i].vht_capab		= 0;
		}
	}
	intf->hw_modes = hw_modes;

	return hw_modes;
 }


void wpa_driver_debug_event(enum wpa_event_type event, union wpa_event_data *data) {
	switch (event) {
		case EVENT_TX_STATUS:
		wpa_printf(MSG_DEBUG, LTAG "%s type(%d), stype(%d), dst(" MACSTR "), ack(%d)",
			__func__,data->tx_status.type, data->tx_status.stype
					,MAC2STR(data->tx_status.dst), data->tx_status.ack);
		break;
		default:
		break;
	}
}

struct wpa_driver_frame_rx_event {
	uint16_t len;
	uint16_t freq;
	uint8_t data[0];
};

#if 0
struct wpa_driver_delayed_event {
	struct nrc_wpa_if* intf;
	enum wpa_event_type type;
	union wpa_event_data event;
	uint8_t addr1[ETH_ALEN];
	uint8_t addr2[ETH_ALEN];
	uint8_t addr3[ETH_ALEN];
	uint8_t data[0];
};

static void wpa_driver_run_delayed_event(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_driver_delayed_event *ev = (struct wpa_driver_delayed_event *)
					timeout_ctx;

	if (!ev)
		return;

	wpa_driver_debug_event(ev->type, &ev->event);
	wpa_supplicant_event(ev->intf->wpa_supp_ctx, ev->type, &ev->event);

	os_free(ev);
	ev = NULL;
}

static void wpa_driver_nrc_tx_status(struct nrc_wpa_if* intf, const u8* data,
				size_t data_len, int ack) {
	struct wpa_driver_delayed_event *ev = os_zalloc(sizeof(*ev) + data_len);
	const struct ieee80211_mgmt *mgmt = (const struct ieee80211_mgmt *) data;

	ev->type = EVENT_TX_STATUS;
	os_memcpy(ev->addr1, mgmt->da, ETH_ALEN);
	os_memcpy(ev->data, data, data_len);

	os_memset(&ev->event, 0, sizeof(ev->event));
	ev->event.tx_status.type = WLAN_FC_GET_TYPE(mgmt->frame_control);
	ev->event.tx_status.stype = WLAN_FC_GET_STYPE(mgmt->frame_control);
	ev->event.tx_status.dst = ev->addr1;
	ev->event.tx_status.data = ev->data;
	ev->event.tx_status.data_len = data_len;
	ev->event.tx_status.ack = ack;
	//wpa_driver_debug_event(EVENT_TX_STATUS, &event);

	//wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_TX_STATUS, &event);
	eloop_register_timeout(0, 0, wpa_driver_run_delayed_event, NULL, (void *) ev);
}
#endif

static void wpa_driver_nrc_tx_status(struct nrc_wpa_if* intf, const u8* data,
				size_t data_len, int ack) {
	union wpa_event_data event;
	const struct ieee80211_mgmt *mgmt = (const struct ieee80211_mgmt *) data;

	os_memset(&event, 0, sizeof(event));
	event.tx_status.type = WLAN_FC_GET_TYPE(mgmt->frame_control);
	event.tx_status.stype = WLAN_FC_GET_STYPE(mgmt->frame_control);
	event.tx_status.dst = mgmt->da;
	event.tx_status.data = data;
	event.tx_status.data_len = data_len;
	event.tx_status.ack = ack;
	wpa_driver_debug_event(EVENT_TX_STATUS, &event);

	wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_TX_STATUS, &event);
}

/**
 * send_mlme - Send management frame from MLME
 * @priv: Private driver interface data
 * @data: IEEE 802.11 management frame with IEEE 802.11 header
 * @data_len: Size of the management frame
 * @noack: Do not wait for this frame to be acked (disable retries)
 * @freq: Frequency (in MHz) to send the frame on, or 0 to let the
 * driver decide
 * Returns: 0 on success, -1 on failure
 */
static int wpa_driver_nrc_send_frame(struct nrc_wpa_if* intf, uint32_t freq,
			const u8 *data, size_t data_len) {
	return nrc_transmit(intf, (uint8_t *) data, data_len);
 }

 static int wpa_driver_nrc_send_mgmt(void *priv, const u8 *data, size_t data_len,
			 int noack, unsigned int freq, const u16 *csa_offs,
			 size_t csa_offs_len) {
	struct nrc_wpa_if* intf = (struct nrc_wpa_if *)(priv);

	wpa_printf(MSG_DEBUG, LTAG "%s len(%d), noack(%d), freq(%d)",
			 __func__, (int) data_len, noack, freq);

	if (wpa_driver_nrc_send_frame(intf, freq, data, data_len) < 0) {
		return -1;
	}
	wpa_driver_nrc_tx_status(intf, data, data_len, noack ? 0 : 1);

	return 0;
}

/**
 * update_ft_ies - Update FT (IEEE 802.11r) IEs
 * @priv: Private driver interface data
 * @md: Mobility domain (2 octets) (also included inside ies)
 * @ies: FT IEs (MDIE, FTIE, ...) or %NULL to remove IEs
 * @ies_len: Length of FT IEs in bytes
 * Returns: 0 on success, -1 on failure
 *
 * The supplicant uses this callback to let the driver know that keying
 * material for FT is available and that the driver can use the
 * provided IEs in the next message in FT authentication sequence.
 *
 * This function is only needed for driver that support IEEE 802.11r
 * (Fast BSS Transition).
 */
 static int wpa_driver_nrc_update_ft_ies(void *priv, const u8 *md, const u8 *ies,
	size_t ies_len) {
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

#ifdef ENABLE_TEST_SCAN_RESULTS
static void nrc_test_net80211_rx(const uint8_t *frame, const uint16_t len)
{
	struct ieee80211com *ic = &nrc_sc()->sc_ic;

	struct mbuf *m	=	(struct mbuf *) net80211_nrc_malloc(sizeof(*m));
	uint8_t *buffer	=	(uint8_t *) net80211_nrc_malloc(len);
	os_memcpy(buffer, frame, len);

	m->m_len		= len;
	m->m_data		= (char *) buffer;
	m->m_pkthdr.len	= m->m_len;

	int type = ieee80211_input_all(ic, m, 0 /* RSSI */, -40 /* noise */);
}
#endif

/**
 * get_scan_results2 - Fetch the latest scan results
 * @priv: private driver interface data
 *
 * Returns: Allocated buffer of scan results (caller is responsible for
 * freeing the data structure) on success, NULL on failure
 */
 static struct wpa_scan_results* wpa_driver_nrc_get_scan_results2(void *priv) {
	struct nrc_wpa_if* intf = (struct nrc_wpa_if *)(priv);
	struct wpa_scan_results *res;

	res = get_scan_results(intf->scan);

	wpa_printf(MSG_DEBUG, LTAG "Received scan results (%lu BSSes)",
		res ? (unsigned long)res->num : 0);

	return res;
 }

/**
 * set_country - Set country
 * @priv: Private driver interface data
 * @alpha2: country to which to switch to
 * Returns: 0 on success, -1 on failure
 *
 * This function is for drivers which support some form
 * of setting a regulatory domain.
 */
 static int wpa_driver_nrc_set_country(void *priv, const char *country)
 {
    wifi_driver_nrc_set_country(country);

    return 0;
 }

/**
 * get_country - Get country
 * @priv: Private driver interface data
 * @alpha2: Buffer for returning country code (at least 3 octets)
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_get_country(void *priv, char *alpha2) 
 {
    wifi_country_t info;
    
    system_modem_api_get_country_info(&info);
    if (alpha2)
        memcpy(alpha2, info.cc, 2);
    
    return 0;
 }

/**
* global_init - Global driver initialization
* @ctx: wpa_global pointer
* Returns: Pointer to private data (global), %NULL on failure
*
* This optional function is called to initialize the driver wrapper
* for global data, i.e., data that applies to all interfaces. If this
* function is implemented, global_deinit() will also need to be
* implemented to free the private data. The driver will also likely
* use init2() function instead of init() to get the pointer to global
* data available to per-interface initializer.
*/
 #define memset os_memset
#if defined(_UMAC_S1G)
 #include "s1g_config.h"
#endif
 #undef memset

 static void* wpa_driver_nrc_global_init(void *ctx) {
	struct wim_builder *wb = NULL;

	wpa_printf(MSG_DEBUG, LTAG "%s \n", __func__);
	nrc_global = (struct nrc_wpa *) os_zalloc (sizeof(struct nrc_wpa));
    nrc_global->cur_chan = 1;
	struct nrc_wpa_if *intf = wpa_driver_get_interface(0);
	/* rssi init -70, avoid hwreset report 4 bug */
	intf->sta.rssi = -70;
	/* Start WIM */
	wb = wim_builder_create(0, WIM_CMD_START, WB_MIN_SIZE);
	wim_builder_run_wm(wb, true);

	return nrc_global;
 }

/**
 * global_deinit - Global driver deinitialization
 * @priv: private driver global data from global_init()
 *
 * Terminate any global driver related functionality and free the
 * global data structure.
 */
 static void wpa_driver_nrc_global_deinit(void *priv) {
	int i = 0;
	struct nrc_wpa *global = (struct nrc_wpa*) priv;
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);

	if(global) {
		for (i = 0; i < NRC_WPA_NUM_INTERFACES; i++) {
			if (global->intf[i])
				os_free(global->intf[i]);
		}
		os_free(global);
	}
}

static void wpa_driver_clear_key(struct nrc_wpa_key *key)
{
	os_memset(key, 0, sizeof(*key));
	key->cipher = WIM_CIPHER_TYPE_NONE;
}

static void wpa_driver_nrc_init_nrc(struct nrc_wpa_if* intf)
{
	struct wim_addr_param addr = {false, false, };
	struct wim_builder *wb = NULL;

	intf->sta_type = WIM_STA_TYPE_STA;
	os_memcpy(addr.addr, intf->addr, ETH_ALEN);

	wb = wim_builder_create(intf->vif_id, WIM_CMD_SET, WB_MIN_SIZE);
	wim_builder_append_u32(wb, WIM_TLV_STA_TYPE, intf->sta_type);
	wim_builder_append(wb, WIM_TLV_MACADDR_PARAM, &addr, sizeof(addr));
	wim_builder_run_wm(wb, true);

#if defined(_UMAC_S1G)
	s1g_config *s1g_cfg = s1g_config::getInstance();
	s1g_cfg->setS1GStaTypeSupport(NON_SENSOR);
#endif
}

/**
 * init2 - Initialize driver interface (with global data)
 * @ctx: context to be used when calling wpa_supplicant functions,
 * e.g., wpa_supplicant_event()
 * @ifname: interface name, e.g., wlan0
 * @global_priv: private driver global data from global_init()
 * Returns: Pointer to private data, %NULL on failure
 *
 * This function can be used instead of init() if the driver wrapper
 * uses global data.
 */
 static void* wpa_driver_nrc_init(void *ctx, const char *ifname, void *global_priv)
 {
	struct nrc_wpa *global	= (struct nrc_wpa *) global_priv;
	struct nrc_wpa_if* intf	= (struct nrc_wpa_if *) os_zalloc(sizeof(*intf));

	if (!intf) {
		wpa_printf(MSG_ERROR, LTAG "Failed to allocate if_info");
		return NULL;
	}

	if (os_strcmp(NRC_WPA_INTERFACE_NAME_0, ifname) == 0) {
		intf->vif_id = 0;
	} else if (os_strcmp(NRC_WPA_INTERFACE_NAME_1, ifname) == 0) {
		intf->vif_id = 1;
	} else {
		ASSERT(0);
	}
	wpa_printf(MSG_DEBUG, LTAG "%s (ifname: %s, vif: %d) \n", __func__
			,ifname ,intf->vif_id);

	global->intf[intf->vif_id]		= intf;
	strcpy(intf->if_name, ifname);
	//TODO:
	//get_standalone_macaddr(intf->vif_id, intf->addr);
	wifi_load_mac_addr(intf->vif_id, intf->addr);
	//os_memcpy(intf->addr, get_standalone_macaddr(), ETH_ALEN);
	intf->global		= global;
	intf->wpa_supp_ctx	= ctx;
	wpa_driver_clear_key(&intf->sta.key);
	wpa_driver_clear_key(&intf->bss.broadcast_key);
	intf->scan = scan_init();
	os_memcpy(intf->sta.addr, intf->addr, ETH_ALEN);
	wpa_driver_nrc_init_nrc(intf);

    intf->max_ap_sta_num = 0;
	return (void *)(intf);
 }

/**
 * get_interfaces - Get information about available interfaces
 * @global_priv: private driver global data from global_init()
 * Returns: Allocated buffer of interface information (caller is
 * responsible for freeing the data structure) on success, NULL on
 * failure
 */
static struct wpa_interface_info* wpa_driver_nrc_get_interfaces(void *global_priv){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return NULL;
}

void wpa_driver_nrc_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
	union wpa_event_data event;
	struct nrc_wpa_if* intf = (struct nrc_wpa_if *)(eloop_ctx);

	os_memset(&event, 0, sizeof(event));
	event.scan_info.aborted = false;
	event.scan_info.nl_scan_event = 1; /* From normal scan */

	wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_SCAN_RESULTS, &event);
}

#define SCAN_PF(x) (WIM_SCAN_PARAM_FLAG_##x)

static int wpa_driver_nrc_scan2_nrc(struct nrc_wpa_if* intf, struct wpa_driver_scan_params *p)
{
	struct wim_builder *wb = wim_builder_create(intf->vif_id, WIM_CMD_SCAN_START, WM_MAX_SIZE);
	struct wim_scan_param wimp = {{},{}, 0, 0, 0, 0, 0, 0, 0, {},{},{}};
	int i = 0, nchs = 0;
	int *freqs = p->freqs;
	uint16_t chs[WIM_MAX_SCAN_CHANNEL] = {0,};
	struct wpa_ssid* pref = wpa_get_ssid(intf->wpa_supp_ctx);

    if (!freqs) {
    	wimp.n_channels = nrc_get_channel_list(intf, chs, WIM_MAX_SCAN_CHANNEL);
    	os_memcpy(wimp.channel, chs, sizeof(uint16_t) * WIM_MAX_SCAN_CHANNEL);
    } else {
        wimp.n_channels = 1;
        wimp.channel[0] = freqs[0];
    }

	wimp.n_ssids = p->num_ssids;
	for (i = 0; i < p->num_ssids; i++) {
		wimp.ssid[i].ssid_len = p->ssids[i].ssid_len;
		os_memcpy(wimp.ssid[i].ssid, p->ssids[i].ssid, p->ssids[i].ssid_len);
	}

	if (p->bssid) {
		wimp.n_bssids++;
		memcpy(wimp.bssid[0].bssid, p->bssid, ETH_ALEN);
	}

    if (p->passive)
        wimp.passive = 1;

    if (p->scan_time > 0)
        wimp.scan_time = p->scan_time;

#if LMAC_CONFIG_11N == 1
	wimp.scan_flag = SCAN_PF(HT) | SCAN_PF(WMM) | SCAN_PF(DS);
#endif

    scan_start(intf->scan);
	i = 0;
	while (pref && i < WPAS_MAX_SCAN_SSIDS) {
        if (pref->mode != WPAS_MODE_AP) {
    		intf->scan->pref_ssids[i].ssid = pref->ssid;
    		intf->scan->pref_ssids[i].ssid_len = pref->ssid_len;
            i++;
        }
		pref = pref->next;
	}
	intf->scan->num_pref_ssids = i;
	scan_config(intf->scan, p);

	/* Start Scan */
	wim_builder_append(wb, WIM_TLV_SCAN_PARAM, &wimp, sizeof(wimp));
	wim_builder_run_wm(wb, true);

	return 0;
}
#undef SCAN_PF

static int wpa_driver_nrc_scan2(void *priv, struct wpa_driver_scan_params *p)
{
	struct nrc_wpa_if* intf = (struct nrc_wpa_if *)(priv);

	if (intf->is_ap) {
		wpa_printf(MSG_ERROR, LTAG "%s: scan is triggered on AP Mode", __func__);
		return -1;
	}

	wpa_printf(MSG_DEBUG, LTAG "[time]<scan>");
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);

	wpa_driver_nrc_scan2_nrc(intf, p);

	return 0;
}

 const char * cipher_txt(int cipher)
 {
	switch (cipher) {
		case WPA_CIPHER_NONE:
		return "NONE";
		case WPA_CIPHER_WEP40:
		return "WEP-40";
		case WPA_CIPHER_WEP104:
		return "WEP-104";
		case WPA_CIPHER_TKIP:
		return "TKIP";
		case WPA_CIPHER_CCMP:
		return "CCMP";
		default:
		return "UNKNOWN";
	}
 }

 char * key_mgmt_txt(unsigned int key_mgmt_suites)
 {
	static char buf[100];
	char*pos, *end;
	int ret;

	pos = buf;
	end = buf + 100;

	if (key_mgmt_suites & WPA_KEY_MGMT_PSK) {
		ret = os_snprintf(pos, end - pos, "%sWPA-PSK",
			pos == buf ? "" : " ");
		pos += ret;
	}

	if (key_mgmt_suites & WPA_KEY_MGMT_IEEE8021X) {
		ret = os_snprintf(pos, end - pos, "%sWPA-EAP",
			pos == buf ? "" : " ");
		pos += ret;
	}

	if (key_mgmt_suites & WPA_KEY_MGMT_IEEE8021X_NO_WPA) {
		ret = os_snprintf(pos, end - pos, "%sIEEE8021X",
			pos == buf ? "" : " ");
		pos += ret;
	}

	if (key_mgmt_suites & WPA_KEY_MGMT_NONE) {
		ret = os_snprintf(pos, end - pos, "%sNONE",
			pos == buf ? "" : " ");
		pos += ret;
	}

	if (key_mgmt_suites & WPA_KEY_MGMT_WPA_NONE) {
		ret = os_snprintf(pos, end - pos, "%sWPA-NONE",
			pos == buf ? "" : " ");
		pos += ret;
	}

#ifdef CONFIG_IEEE80211W
	if (key_mgmt_suites & WPA_KEY_MGMT_PSK_SHA256) {
		ret = os_snprintf(pos, end - pos, "%sWPA-PSK-SHA256",
			pos == buf ? "" : " ");
		pos += ret;
	}

	if (key_mgmt_suites & WPA_KEY_MGMT_IEEE8021X_SHA256) {
		ret = os_snprintf(pos, end - pos, "%sWPA-EAP-SHA256",
			pos == buf ? "" : " ");
		pos += ret;
	}
#endif /* CONFIG_IEEE80211W */

#ifdef CONFIG_WPS
	if (key_mgmt_suites & WPA_KEY_MGMT_WPS) {
		ret = os_snprintf(pos, end - pos, "%sWPS",
			pos == buf ? "" : " ");
		pos += ret;
	}
#endif /* CONFIG_WPS */

	if (pos == buf) {
		buf[0] = 0;
	}

	return buf;
 }

/**
 * set_acl - Set ACL in AP mode
 * @priv: Private driver interface data
 * @params: Parameters to configure ACL
 * Returns: 0 on success, -1 on failure
 *
 * This is used only for the drivers which support MAC address ACL.
 */
 static int wpa_driver_nrc_set_acl(void *priv, struct hostapd_acl_params *params){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * read_sta_data - Fetch station data
 * @priv: Private driver interface data
 * @data: Buffer for returning station information
 * @addr: MAC address of the station
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_read_sta_data(void *priv, struct hostap_sta_driver_data *data,
	const u8 *addr) {
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

static void wpa_driver_nrc_sta_cmd_wim(struct nrc_wpa_if* intf, int cmd,
				const uint8_t *addr, int sleep, int aid, int flags)
{
 	struct wim_sta_param p = {cmd, {} /*addr*/, sleep, aid, flags};
 	os_memcpy(p.addr, addr, ETH_ALEN);
 	struct wim_builder * wb = wim_builder_create(intf->vif_id, WIM_CMD_STA_CMD, WB_MIN_SIZE);
	wim_builder_append(wb, WIM_TLV_STA_PARAM, &p, sizeof(p));
	wim_builder_run_wm(wb, true);
}

/**
 * sta_add - Add a station entry
 * @priv: Private driver interface data
 * @params: Station parameters
 * Returns: 0 on success, -1 on failure
 *
 * This function is used to add a station entry to the driver once the
 * station has completed association. This is only used if the driver
 * does not take care of association processing.
 *
 * With TDLS, this function is also used to add or set (params->set 1)
 * TDLS peer entries.
 */
extern void wifi_handle_ap_connect_event(int vif, const uint8_t *addr,uint16_t aid,uint8_t type);
static int wpa_driver_nrc_sta_add(void *priv, struct hostapd_sta_add_params *p) {
	int i = 0;
	struct nrc_wpa_if* intf = (struct nrc_wpa_if *)(priv);
	struct nrc_wpa_sta** sta;
	struct nrc_wpa_key* bkey = &intf->bss.broadcast_key;
	wpa_printf(MSG_DEBUG, LTAG "%s " MACSTR "", __func__, MAC2STR(p->addr));

	if((0 != intf->max_ap_sta_num && intf->num_ap_sta < intf->max_ap_sta_num) || (0 == intf->max_ap_sta_num)) {
		for (i = 0; i < NRC_WPA_SOFTAP_MAX_STA; i++) {
			if (intf->ap_sta[i])
				continue;
			intf->ap_sta[i] = os_zalloc(sizeof(struct nrc_wpa_sta));
			os_memcpy(intf->ap_sta[i]->addr, p->addr, ETH_ALEN);
			intf->ap_sta[i]->aid = p->aid;

			if (p->flags & WPA_STA_WMM)
				intf->ap_sta[i]->qos = true;

			wpa_driver_nrc_sta_cmd_wim(intf, WIM_STA_CMD_ADD, p->addr, 0, p->aid, 0);
			wpa_clear_key(&intf->ap_sta[i]->key);
			wifi_handle_ap_connect_event(intf->vif_id, p->addr,p->aid,true);

			if (is_key_wep(bkey)) {
				wpa_driver_nrc_set_key_wim(intf, bkey->cipher,
					p->addr, bkey->ix, bkey->key, bkey->key_len);
			}
			intf->num_ap_sta++;
			return 0;
		}
	}
	return -1;
}

static int wpa_driver_nrc_sta_remove(void *priv, const u8 *addr) {
	int i = 0;
	struct nrc_wpa_if* intf = (struct nrc_wpa_if *)(priv);
	struct nrc_wpa_sta* sta;
	uint16_t aid = 0;

	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	struct nrc_wpa_key *wpa_key = NULL;

	if (intf->num_ap_sta <= 0)
		return 0;

	for (i = 0; i < NRC_WPA_SOFTAP_MAX_STA; i++) {
		if (!intf->ap_sta[i])
			continue;

		if (os_memcmp(intf->ap_sta[i]->addr, addr, ETH_ALEN) != 0)
			continue;
		wpa_key = &intf->ap_sta[i]->key;
		aid = intf->ap_sta[i]->aid;

		if (is_key_wep(wpa_key)) {
			wpa_driver_nrc_set_key_wim(intf, WIM_CIPHER_TYPE_NONE,
				addr, wpa_key->ix, wpa_key->key, wpa_key->key_len);
		}

		os_free(intf->ap_sta[i]);
		intf->ap_sta[i] = NULL;
		intf->num_ap_sta--;
		wpa_driver_nrc_sta_cmd_wim(intf, WIM_STA_CMD_REMOVE, addr, 0, aid, 0);
		wifi_handle_ap_connect_event(intf->vif_id, addr,aid,false);
		return 0;
	}
	return 0;
}

struct hostapd_data * wifi_ap_get_apdata(int vif_id)
{
	struct hostapd_data *hostapd;
	struct wpa_supplicant *wpa_s;
	extern struct wpa_supplicant *wpa_get_ctrl_iface(int vif_id);
	wpa_s = wpa_get_ctrl_iface(vif_id);
	hostapd = wpa_s->ap_iface->bss[0];
	return hostapd;
}

/**
 * set_rts - Set RTS threshold
 * @priv: Private driver interface data
 * @rts: RTS threshold in octets
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_set_rts(void *priv, int rts){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * set_frag - Set fragmentation threshold
 * @priv: Private driver interface data
 * @frag: Fragmentation threshold in octets
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_set_frag(void *priv, int frag){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * set_tx_queue_params - Set TX queue parameters
 * @priv: Private driver interface data
 * @queue: Queue number (0 = VO, 1 = VI, 2 = BE, 3 = BK)
 * @aifs: AIFS
 * @cw_min: cwMin
 * @cw_max: cwMax
 * @burst_time: Maximum length for bursting in 0.1 msec units
 */
 static int wpa_driver_nrc_set_tx_queue_params(void *priv, int queue, int aifs, int cw_min,
	int cw_max, int burst_time) {
 	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
 	struct wim_tx_queue_param txq_p	= {queue, 0, cw_min, cw_max, 0, };

 	struct wim_builder *wb = NULL;
 	int q_to_ac[WMM_AC_NUM] = {WMM_AC_VO, WMM_AC_VI, WMM_AC_BE, WMM_AC_BK};

	//wpa_printf(MSG_DEBUG, LTAG "%s(%d) : a(%d), cw:[%d, %d], b(%d)",
	//		__func__, queue, aifs, cw_min, cw_max);

 	if (queue >= WMM_AC_NUM)
 		return -1;

 	txq_p.ac = q_to_ac[queue];
 	txq_p.txop = (burst_time * 100 + 16) / 32; /*0.1 msec to TXOP param */
 	txq_p.sta_type = 0;
 	txq_p.aifsn = aifs;

#if 0
	wb = wim_builder_create(intf->vif_id, WIM_CMD_SET, WB_MIN_SIZE);
	wim_builder_append(wb, WIM_TLV_TXQ_PARAM, &txq_p, sizeof(txq_p));
	wim_builder_run_wm(wb, true);
#endif

	return 0;
 }

/**
 * if_add - Add a virtual interface
 * @priv: Private driver interface data
 * @type: Interface type
 * @ifname: Interface name for the new virtual interface
 * @addr: Local address to use for the interface or %NULL to use the
 *	parent interface address
 * @bss_ctx: BSS context for %WPA_IF_AP_BSS interfaces
 * @drv_priv: Pointer for overwriting the driver context or %NULL if
 *	not allowed (applies only to %WPA_IF_AP_BSS type)
 * @force_ifname: Buffer for returning an interface name that the
 *	driver ended up using if it differs from the requested ifname
 * @if_addr: Buffer for returning the allocated interface address
 *	(this may differ from the requested addr if the driver cannot
 *	change interface address)
 * @bridge: Bridge interface to use or %NULL if no bridge configured
 * @use_existing: Whether to allow existing interface to be used
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_if_add(void *priv, enum wpa_driver_if_type type,
	const char *ifname, const u8 *addr, void *bss_ctx,
	void **drv_priv, char *force_ifname, u8 *if_addr,
	const char *bridge, int use_existing,int setup_ap) {
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return 0;
 }

/**
 * if_remove - Remove a virtual interface
 * @priv: Private driver interface data
 * @type: Interface type
 * @ifname: Interface name of the virtual interface to be removed
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_if_remove(void *priv, enum wpa_driver_if_type type,
	const char *ifname){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * set_radius_acl_auth - Notification of RADIUS ACL change
 * @priv: Private driver interface data
 * @mac: MAC address of the station
 * @accepted: Whether the station was accepted
 * @session_timeout: Session timeout for the station
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_set_radius_acl_auth(void *priv, const u8 *mac, int accepted,
	u32 session_timeout){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * set_radius_acl_expire - Notification of RADIUS ACL expiration
 * @priv: Private driver interface data
 * @mac: MAC address of the station
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_set_radius_acl_expire(void *priv, const u8 *mac){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * set_supp_port - Set IEEE 802.1X Supplicant Port status
 * @priv: Private driver interface data
 * @authorized: Whether the port is authorized
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_set_supp_port(void *priv, int authorized){
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	intf->bss.authorized_1x = !!(authorized);
	wpa_printf(MSG_DEBUG, LTAG "%s %d ", __func__, authorized);
	return 0;
 }

/**
 * send_action - Transmit an Action frame
 * @priv: Private driver interface data
 * @freq: Frequency (in MHz) of the channel
 * @wait: Time to wait off-channel for a response (in ms), or zero
 * @dst: Destination MAC address (Address 1)
 * @src: Source MAC address (Address 2)
 * @bssid: BSSID (Address 3)
 * @data: Frame body
 * @data_len: data length in octets
 @ @no_cck: Whether CCK rates must not be used to transmit this frame
 * Returns: 0 on success, -1 on failure
 *
 * This command can be used to request the driver to transmit an action
 * frame to the specified destination.
 *
 * If the %WPA_DRIVER_FLAGS_OFFCHANNEL_TX flag is set, the frame will
 * be transmitted on the given channel and the device will wait for a
 * response on that channel for the given wait time.
 *
 * If the flag is not set, the wait time will be ignored. In this case,
 * if a remain-on-channel duration is in progress, the frame must be
 * transmitted on that channel; alternatively the frame may be sent on
 * the current operational channel (if in associated state in station
 * mode or while operating as an AP.)
 */
 static int wpa_driver_nrc_send_action(void *priv, unsigned int freq, unsigned int wait,
	const u8 *dst, const u8 *src, const u8 *bssid,
	const u8 *data, size_t data_len, int no_cck){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
#ifdef CONFIG_IEEE80211W
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	struct ieee80211_hdr *hdr;
	int result = -1;
	u8 *buf;

	buf = os_zalloc(24 + data_len);
	if (buf == NULL)
		return result;
	
	os_memcpy(buf + 24, data, data_len);
	
	hdr = (struct ieee80211_hdr *) buf;
	hdr->frame_control =
		IEEE80211_FC(WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_ACTION);
	if(intf->pmf)
		hdr->frame_control |= WLAN_FC_ISWEP;
	os_memcpy(hdr->addr1, dst, ETH_ALEN);
	os_memcpy(hdr->addr2, src, ETH_ALEN);
	os_memcpy(hdr->addr3, bssid, ETH_ALEN);

	result = nrc_transmit(intf, buf, 24 + data_len);
	os_free(buf);
	
	return result;
#else
	return 0;
#endif
 }

/**
 * send_action_cancel_wait - Cancel action frame TX wait
 * @priv: Private driver interface data
 *
 * This command cancels the wait time associated with sending an action
 * frame. It is only available when %WPA_DRIVER_FLAGS_OFFCHANNEL_TX is
 * set in the driver flags.
 */
 static void wpa_driver_nrc_send_action_cancel_wait(void *priv){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
 }

/**
 * remain_on_channel - Remain awake on a channel
 * @priv: Private driver interface data
 * @freq: Frequency (in MHz) of the channel
 * @duration: Duration in milliseconds
 * Returns: 0 on success, -1 on failure
 *
 * This command is used to request the driver to remain awake on the
 * specified channel for the specified duration and report received
 * Action frames with EVENT_RX_MGMT events. Optionally, received
 * Probe Request frames may also be requested to be reported by calling
 * probe_req_report(). These will be reported with EVENT_RX_PROBE_REQ.
 *
 * The driver may not be at the requested channel when this function
 * returns, i.e., the return code is only indicating whether the
 * request was accepted. The caller will need to wait until the
 * EVENT_REMAIN_ON_CHANNEL event indicates that the driver has
 * completed the channel change. This may take some time due to other
 * need for the radio and the caller should be prepared to timing out
 * its wait since there are no guarantees on when this request can be
 * executed.
 */
 static int wpa_driver_nrc_remain_on_channel(void *priv, unsigned int freq,
	unsigned int duration){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return 0;
 }

/**
 * cancel_remain_on_channel - Cancel remain-on-channel operation
 * @priv: Private driver interface data
 *
 * This command can be used to cancel a remain-on-channel operation
 * before its originally requested duration has passed. This could be
 * used, e.g., when remain_on_channel() is used to request extra time
 * to receive a response to an Action frame and the response is
 * received when there is still unneeded time remaining on the
 * remain-on-channel operation.
 */
 static int wpa_driver_nrc_cancel_remain_on_channel(void *priv){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return 0;
 }

/**
 * probe_req_report - Request Probe Request frames to be indicated
 * @priv: Private driver interface data
 * @report: Whether to report received Probe Request frames
 * Returns: 0 on success, -1 on failure (or if not supported)
 *
 * This command can be used to request the driver to indicate when
 * Probe Request frames are received with EVENT_RX_PROBE_REQ events.
 * Since this operation may require extra resources, e.g., due to less
 * optimal hardware/firmware RX filtering, many drivers may disable
 * Probe Request reporting at least in station mode. This command is
 * used to notify the driver when the Probe Request frames need to be
 * reported, e.g., during remain-on-channel operations.
 */
 static int wpa_driver_nrc_probe_req_report(void *priv, int report){
	return 0;
 }

/**
 * deinit_ap - Deinitialize AP mode
 * @priv: Private driver interface data
 * Returns: 0 on success, -1 on failure (or if not supported)
 *
 * This optional function can be used to disable AP mode related
 * configuration. If the interface was not dynamically added,
 * change the driver mode to station mode to allow normal station
 * operations like scanning to be completed.
 */
 static int wpa_driver_nrc_deinit_ap(void *priv){
	uint8_t null_bssid[ETH_ALEN] = {0,};
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	struct wim_builder *wb = wim_builder_create(intf->vif_id, WIM_CMD_SET, WB_MIN_SIZE);
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	intf->is_ap = false;

	if (intf->sta_type != WIM_STA_TYPE_STA) {
		intf->sta_type = WIM_STA_TYPE_STA;
		wim_builder_append_u32(wb, WIM_TLV_STA_TYPE, intf->sta_type);
	}

	wim_builder_append(wb, WIM_TLV_BSSID, null_bssid, ETH_ALEN);
	eloop_register_timeout(0, 0, wpa_driver_wim_run, (void *)intf, (void *)wb);
	intf->associated = false;

	wb = wim_builder_create(intf->vif_id, WIM_CMD_STOP, WB_MIN_SIZE);
	eloop_register_timeout(0, 0, wpa_driver_wim_run, (void *)intf, (void *)wb);

	return 0;
 }

/**
 * deinit_p2p_cli - Deinitialize P2P client mode
 * @priv: Private driver interface data
 * Returns: 0 on success, -1 on failure (or if not supported)
 *
 * This optional function can be used to disable P2P client mode. If the
 * interface was not dynamically added, change the interface type back
 * to station mode.
 */
 static int wpa_driver_nrc_deinit_p2p_cli(void *priv){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return 0;
 }

/**
 * suspend - Notification on system suspend/hibernate event
 * @priv: Private driver interface data
 */
 static void wpa_driver_nrc_suspend(void *priv){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
 }

/**
 * resume - Notification on system resume/thaw event
 * @priv: Private driver interface data
 */
 static void wpa_driver_nrc_resume(void *priv){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
 }

/**
 * signal_monitor - Set signal monitoring parameters
 * @priv: Private driver interface data
 * @threshold: Threshold value for signal change events; 0 = disabled
 * @hysteresis: Minimum change in signal strength before indicating a
 *	new event
 * Returns: 0 on success, -1 on failure (or if not supported)
 *
 * This function can be used to configure monitoring of signal strength
 * with the current AP. Whenever signal strength drops below the
 * %threshold value or increases above it, EVENT_SIGNAL_CHANGE event
 * should be generated assuming the signal strength has changed at
 * least %hysteresis from the previously indicated signal change event.
 */
 static int wpa_driver_nrc_signal_monitor(void *priv, int threshold, int hysteresis){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * get_noa - Get current Notice of Absence attribute payload
 * @priv: Private driver interface data
 * @buf: Buffer for returning NoA
 * @buf_len: Buffer length in octets
 * Returns: Number of octets used in buf, 0 to indicate no NoA is being
 * advertized, or -1 on failure
 *
 * This function is used to fetch the current Notice of Absence
 * attribute value from GO.
 */
 static int wpa_driver_nrc_get_noa(void *priv, u8 *buf, size_t buf_len){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * set_noa - Set Notice of Absence parameters for GO (testing)
 * @priv: Private driver interface data
 * @count: Count
 * @start: Start time in ms from next TBTT
 * @duration: Duration in ms
 * Returns: 0 on success or -1 on failure
 *
 * This function is used to set Notice of Absence parameters for GO. It
 * is used only for testing. To disable NoA, all parameters are set to
 * 0.
 */
 static int wpa_driver_nrc_set_noa(void *priv, u8 count, int start, int duration){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * set_p2p_powersave - Set P2P power save options
 * @priv: Private driver interface data
 * @legacy_ps: 0 = disable, 1 = enable, 2 = maximum PS, -1 = no change
 * @opp_ps: 0 = disable, 1 = enable, -1 = no change
 * @ctwindow: 0.. = change (msec), -1 = no change
 * Returns: 0 on success or -1 on failure
 */
 static int wpa_driver_nrc_set_p2p_powersave(void *priv, int legacy_ps, int opp_ps,
	int ctwindow){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * ampdu - Enable/disable aggregation
 * @priv: Private driver interface data
 * @ampdu: 1/0 = enable/disable A-MPDU aggregation
 * Returns: 0 on success or -1 on failure
 */
 static int wpa_driver_nrc_ampdu(void *priv, int ampdu){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * get_radio_name - Get physical radio name for the device
 * @priv: Private driver interface data
 * Returns: Radio name or %NULL if not known
 *
 * The returned data must not be modified by the caller. It is assumed
 * that any interface that has the same radio name as another is
 * sharing the same physical radio. This information can be used to
 * share scan results etc. information between the virtual interfaces
 * to speed up various operations.
 */
 static const char * wpa_driver_nrc_get_radio_name(void *priv){
	return "nrc";
 }

/**
 * set_wowlan - Set wake-on-wireless triggers
 * @priv: Private driver interface data
 * @triggers: wowlan triggers
 */
 static int wpa_driver_nrc_set_wowlan(void *priv, const struct wowlan_triggers *triggers){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * signal_poll - Get current connection information
 * @priv: Private driver interface data
 * @signal_info: Connection info structure
 */
 static int wpa_driver_nrc_signal_poll(void *priv, struct wpa_signal_info *signal_info){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * vendor_cmd - Execute vendor specific command
 * @priv: Private driver interface data
 * @vendor_id: Vendor id
 * @subcmd: Vendor command id
 * @data: Vendor command parameters (%NULL if no parameters)
 * @data_len: Data length
 * @buf: Return buffer (%NULL to ignore reply)
 * Returns: 0 on success, negative (<0) on failure
 *
 * This function handles vendor specific commands that are passed to
 * the driver/device. The command is identified by vendor id and
 * command id. Parameters can be passed as argument to the command
 * in the data buffer. Reply (if any) will be filled in the supplied
 * return buffer.
 *
 * The exact driver behavior is driver interface and vendor specific. As
 * an example, this will be converted to a vendor specific cfg80211
 * command in case of the nl80211 driver interface.
 */
 static int wpa_driver_nrc_vendor_cmd(void *priv, unsigned int vendor_id,
	unsigned int subcmd, const u8 *data, size_t data_len,
	struct wpabuf *buf){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * sched_scan - Request the driver to initiate scheduled scan
 * @priv: Private driver interface data
 * @params: Scan parameters
 * @interval: Interval between scan cycles in milliseconds
 * Returns: 0 on success, -1 on failure
 *
 * This operation should be used for scheduled scan offload to
 * the hardware. Every time scan results are available, the
 * driver should report scan results event for wpa_supplicant
 * which will eventually request the results with
 * wpa_driver_get_scan_results2(). This operation is optional
 * and if not provided or if it returns -1, we fall back to
 * normal host-scheduled scans.
 */
 static int wpa_driver_nrc_sched_scan(void *priv, struct wpa_driver_scan_params *params)
{
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
}

/**
 * stop_sched_scan - Request the driver to stop a scheduled scan
 * @priv: Private driver interface data
 * Returns: 0 on success, -1 on failure
 *
 * This should cause the scheduled scan to be stopped and
 * results should stop being sent. Must be supported if
 * sched_scan is supported.
 */
 static int wpa_driver_nrc_stop_sched_scan(void *priv){
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * poll_client - Probe (null data or such) the given station
 * @priv: Private driver interface data
 * @own_addr: MAC address of sending interface
 * @addr: MAC address of the station to probe
 * @qos: Indicates whether station is QoS station
 *
 * This function is used to verify whether an associated station is
 * still present. This function does not need to be implemented if the
 * driver provides such inactivity polling mechanism.
 */
 static void wpa_driver_nrc_poll_client(void *priv, const u8 *own_addr,
	const u8 *addr, int qos){
	//wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
 }

/**
 * radio_disable - Disable/enable radio
 * @priv: Private driver interface data
 * @disabled: 1=disable 0=enable radio
 * Returns: 0 on success, -1 on failure
 *
 * This optional command is for testing purposes. It can be used to
 * disable the radio on a testbed device to simulate out-of-radio-range
 * conditions.
 */
 static int wpa_driver_nrc_radio_disable(void *priv, int disabled){
	//wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * switch_channel - Announce channel switch and migrate the GO to the
 * given frequency
 * @priv: Private driver interface data
 * @settings: Settings for CSA period and new channel
 * Returns: 0 on success, -1 on failure
 *
 * This function is used to move the GO to the legacy STA channel to
 * avoid frequency conflict in single channel concurrency.
 */
 static int wpa_driver_nrc_switch_channel(void *priv, struct csa_settings *settings){
	//wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * add_tx_ts - Add traffic stream
 * @priv: Private driver interface data
 * @tsid: Traffic stream ID
 * @addr: Receiver address
 * @user_prio: User priority of the traffic stream
 * @admitted_time: Admitted time for this TS in units of
 *	32 microsecond periods (per second).
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_add_tx_ts(void *priv, u8 tsid, const u8 *addr, u8 user_prio,
	u16 admitted_time){
	//wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * del_tx_ts - Delete traffic stream
 * @priv: Private driver interface data
 * @tsid: Traffic stream ID
 * @addr: Receiver address
 * Returns: 0 on success, -1 on failure
 */
 static int wpa_driver_nrc_del_tx_ts(void *priv, u8 tsid, const u8 *addr){
	//wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * get_survey - Retrieve survey data
 * @priv: Private driver interface data
 * @freq: If set, survey data for the specified frequency is only
 *	being requested. If not set, all survey data is requested.
 * Returns: 0 on success, -1 on failure
 *
 * Use this to retrieve:
 *
 * - the observed channel noise floor
 * - the amount of time we have spent on the channel
 * - the amount of time during which we have spent on the channel that
 *   the radio has determined the medium is busy and we cannot
 *   transmit
 * - the amount of time we have spent receiving data
 * - the amount of time we have spent transmitting data
 *
 * This data can be used for spectrum heuristics. One example is
 * Automatic Channel Selection (ACS). The channel survey data is
 * kept on a linked list on the channel data, one entry is added
 * for each survey. The min_nf of the channel is updated for each
 * survey.
 */
 static int wpa_driver_nrc_get_survey(void *priv, unsigned int freq){
	//wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * status - Get driver interface status information
 * @priv: Private driver interface data
 * @buf: Buffer for printing tou the status information
 * @buflen: Maximum length of the buffer
 * Returns: Length of written status information or -1 on failure
 */
 static int wpa_driver_nrc_status(void *priv, char *buf, size_t buflen){
	//wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return -1;
 }

/**
 * set_mac_addr - Set MAC address
 * @priv: Private driver interface data
 * @addr: MAC address to use or %NULL for setting back to permanent
 * Returns: 0 on success, -1 on failure
 */
static void wpa_driver_nrc_init_nrc_xxx(struct nrc_wpa_if* intf)
{
	 struct wim_addr_param addr = {false, false, };
	 struct wim_builder *wb = NULL;

	 intf->sta_type = WIM_STA_TYPE_STA;
	 os_memcpy(addr.addr, intf->addr, ETH_ALEN);

	 wb = wim_builder_create(intf->vif_id, WIM_CMD_SET, WB_MIN_SIZE);
	 //wim_builder_append_u32(wb, WIM_TLV_STA_TYPE, intf->sta_type);
	 wim_builder_append(wb, WIM_TLV_MACADDR_PARAM, &addr, sizeof(addr));
	 wim_builder_run_wm(wb, true);
}
static int wpa_driver_nrc_set_mac_addr(void *priv, const u8 *addr){
	//wpa_printf(MSG_DEBUG, LTAG "%s", __func__);

	struct nrc_wpa_if* intf = (struct nrc_wpa_if *)(priv);
	os_memcpy(intf->addr, addr, ETH_ALEN);
	wpa_driver_nrc_init_nrc_xxx(intf);

	return 0;
}


static int wpa_driver_nrc_hapd_send_eapol(void *priv, const u8 *addr,
			const u8 *data, size_t data_len, int encrypt, const u8 *own_addr,
			u32 flags)
{
	const uint8_t rfc1042[6] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	int qos = flags & WPA_STA_WMM;
	struct ieee80211_hdr *hdr = NULL;
	uint16_t len = 0, i = 0;
	union wpa_event_data event;
    struct nrc_wpa_key *wpa_key = NULL;

	event.eapol_tx_status.dst = addr;
	event.eapol_tx_status.data = data;
	event.eapol_tx_status.data_len = data_len;
	event.eapol_tx_status.ack = 1;

	wpa_printf(MSG_DEBUG, LTAG "send_eapol");

	len = sizeof(*hdr) + (qos ? 2 : 0) + sizeof(rfc1042)
					+ 2 /* QoS Control Field */ + data_len;

    if (encrypt) {
        wpa_key = nrc_wpa_get_key(intf, addr);
        len += nrc_get_sec_hdr_len(wpa_key);

    }
	hdr = os_zalloc(len);

	if (!hdr) {
		wpa_printf(MSG_ERROR, "Failed to allocate EAPOL.");
		return -1;
	}
	uint8_t *pos = (uint8_t *) hdr;
	hdr->frame_control = IEEE80211_FC(WLAN_FC_TYPE_DATA, WLAN_FC_STYPE_DATA)
			| host_to_le16(WLAN_FC_FROMDS)
			| (encrypt ? host_to_le16(WLAN_FC_ISWEP) : 0)
			| (qos ? host_to_le16(WLAN_FC_STYPE_QOS_DATA << 4) : 0);

	memcpy(hdr->IEEE80211_DA_FROMDS, 	addr, ETH_ALEN);
	memcpy(hdr->IEEE80211_BSSID_FROMDS,	own_addr, ETH_ALEN);
	memcpy(hdr->IEEE80211_SA_FROMDS,	own_addr, ETH_ALEN);
	pos += sizeof(*hdr);

	if (qos) {
		/* QoS Control Field */
		*pos++ = 7;
		*pos++ = 0;
	}
    if (encrypt && wpa_key) {
		pos += nrc_add_sec_hdr(wpa_key, pos);
	}
	os_memcpy(pos, rfc1042, sizeof(rfc1042));
	pos += sizeof(rfc1042);
	WPA_PUT_BE16(pos, ETH_P_PAE);
	pos += 2;
	os_memcpy(pos, data, data_len);
	pos += data_len;

	if (wpa_driver_nrc_send_frame(intf, 0, (void *) hdr, len) < 0) {
		wpa_printf(MSG_ERROR, "Failed to send EAPOL.");
        os_free(hdr);
		return -1;
	}
	wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_EAPOL_TX_STATUS, &event);
    os_free(hdr);
	return 0;
}

static int wpa_driver_nrc_set_freq(void *priv, struct hostapd_freq_params *freq)
{
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	wpa_printf(MSG_DEBUG, "%s, freq=%d, channel=%d", __func__, freq->freq, freq->channel);
	return 0;
}

void wpa_driver_wim_run(void *eloop_ctx, void *timeout_ctx)
{
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *) eloop_ctx;
	struct wim_builder *wb = (struct wim_builder *) timeout_ctx;
	wpa_printf(MSG_DEBUG, LTAG "%s(vif: %d)", __func__, intf->vif_id);
	wim_builder_run_wm(wb, true);
}

static int wpa_driver_nrc_set_ap(void *priv, struct wpa_driver_ap_params *p)
{
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	struct wim_builder *wb = NULL;
	uint8_t *frame = NULL, *pos = NULL;
	struct wim_erp_param erp = {0, };
	int i = 0, size = 0;
	const int ADDITIONAL_IE_LEN = 100;
	uint8_t bssid[ETH_ALEN];

	wpa_printf(MSG_DEBUG, LTAG "%s(vif: %d)", __func__, intf->vif_id);

	int frame_len = p->head_len + p->tail_len;

	if (frame_len > WM_MAX_SIZE ||
		frame_len < IEEE80211_HDRLEN) {
		wpa_printf(MSG_ERROR, "%s, Incorrent frame length (%d)",
			__func__, frame_len);
		return -1;
	}

	wb = wim_builder_create(intf->vif_id, WIM_CMD_SET, WM_MAX_SIZE);

	if (!wb)
		return -1;

	wpa_driver_nrc_get_bssid(intf, bssid);
	wim_builder_append(wb, WIM_TLV_BSSID, bssid, ETH_ALEN);
	wim_builder_append_u16(wb, WIM_TLV_BCN_INTV, (uint16_t) p->beacon_int);
	wim_builder_append(wb, WIM_TLV_SSID, (void *) p->ssid, p->ssid_len);
    wim_builder_append_u32(wb, WIM_TLV_AP_HIDDEN_SSID, p->hide_ssid);
    

	if (p->ht_opmode >= 0) // HT in use
		wim_builder_append_u16(wb, WIM_TLV_HT_MODE, p->ht_opmode);

	erp.use_11b_protection 	= p->cts_protect;
	erp.use_short_preamble 	= p->preamble;
	erp.use_short_slot 		= p->short_slot_time;
	wim_builder_append(wb, WIM_TLV_ERP_PARAM, &erp, sizeof(erp));

	if (p->basic_rates) {
		//TODO: Base Rate Set
	}

	if (p->proberesp)
		wim_builder_append(wb, WIM_TLV_PROBE_RESP, p->proberesp, p->proberesp_len);

	if (p->freq) {
		struct wim_channel_param wp = {p->freq->freq, };
		wim_builder_append(wb, WIM_TLV_CHANNEL, &wp, sizeof(wp));
        nrc_global->cur_chan = system_modem_api_mac80211_frequency_to_channel(p->freq->freq);
	}

	pos = frame = (uint8_t *) os_malloc(p->head_len + p->tail_len
						+ ADDITIONAL_IE_LEN);

	if (!frame) {
		wpa_printf(MSG_ERROR, "Failed to allocate beacon frame.\n");
		return -1;
	}
	os_memcpy(pos, p->head, p->head_len);
	pos += p->head_len;
#if 0
	bssid[0] = 5;
	bssid[1] = 4;
	bssid[2] = p->dtim_period;
	bssid[3] = p->dtim_period;
	bssid[4] = 0;
	bssid[5] = 0;
	os_memcpy(pos, bssid, 6);
	pos += 6;
#endif
	os_memcpy(pos, p->tail, p->tail_len);
	pos += p->tail_len;
	if (p->privacy) {
	}

	wim_builder_append(wb, WIM_TLV_BEACON, frame, pos - frame);
	os_free(frame);
	wim_builder_append_u8(wb, WIM_TLV_BEACON_ENABLE, 1);

	eloop_register_timeout(0, 0, wpa_driver_wim_run, (void *) intf, (void *) wb);

	return 0;
}

struct wpa_driver_event_item {
	int id;
	size_t len;
	uint8_t data[0];
};

#if defined(NRC_USER_APP)
#include "nrc_user_app.h"
extern QueueHandle_t    x_wlan_mgr_queue_handle;
#endif
#if 0
static void wpa_driver_notify_event_to_app(int event_id)
{
#if defined(NRC_USER_APP)
	WLAN_MESSAGE message;
	MSG_ID_FORMAT message_id;
	switch (event_id) {
		case RTM_IEEE80211_ASSOC:
		case RTM_IEEE80211_REASSOC:
		message_id.id = WLAN_EVT_CONNECT_SUCCESS;
		break;
		case RTM_IEEE80211_DISASSOC:
		message_id.id = WLAN_EVT_DISCONNECT;
		break;
		default:
		return;
	}

	message.type = WLAN_EVENT;
	message.len = WLAN_ID_FM_SIZE;
	message.value = (void *) &message_id;

	xQueueSend( x_wlan_mgr_queue_handle, (void *) &message, (TickType_t) 0 );
#endif
}
#endif
static void wpa_driver_notify_event_to_lwip(int event_id)
{

}
#if 0
static void wpa_driver_notify_event_to_ext(int event_id) {
	wpa_driver_notify_event_to_app(event_id);
	wpa_driver_notify_event_to_lwip(event_id);
}
#endif

static int wpa_driver_notify_txq_to_wim(struct nrc_wpa_if *intf)
{
	#if 0
	int i = 0;
	struct chanAccParams p = {};
	struct wim_tx_queue_param txq_p;
	struct wim_builder *wb = NULL;

	const int ACs[] = { WME_AC_BE, WME_AC_BK, WME_AC_VI, WME_AC_VO };

	if (wpa_driver_nrc_get_wmm(intf, &p) < 0) {
		wpa_printf(MSG_ERROR, LTAG " %s() Failed to get WMM", __func__);
		return -1;
	}

	for (i = ARRAY_SIZE(ACs) - 1; i >= 0; i--) {
		if (wpa_driver_nrc_wim_tx_param(&p, ACs[i], &txq_p) < 0)
			return -1;

		wb = wim_builder_create(intf->vif_id, WIM_CMD_SET, WB_MIN_SIZE);
		wim_builder_append(wb, WIM_TLV_TXQ_PARAM, &txq_p, sizeof(txq_p));
		wim_builder_run_wm(wb, true);
	}
	#endif

	return 0;
}

void wpa_driver_sta_sta_cmd(struct nrc_wpa_if* intf, int cmd)
{
	struct wim_builder *wb = NULL;
	uint8_t bssid[ETH_ALEN] = {0,};
	uint16_t aid = 0;
	struct wim_sta_param p = {cmd, {}, 0, 0, 0};

	// On AP mode wpa_driver_nrc_sta_add() will handle WIM_STA_CMD_ADD
	if (intf->is_ap)
		return;

	wpa_driver_nrc_get_bssid(intf, bssid);
	os_memcpy(p.addr, bssid, ETH_ALEN);
	p.aid = 0; // AP

	if (cmd == WIM_STA_CMD_REMOVE) {
		intf->associated = false;
		wpa_driver_nrc_get_bssid(intf, bssid);
	}

	wb = wim_builder_create(intf->vif_id, WIM_CMD_SET, WB_MIN_SIZE);
	wim_builder_append(wb, WIM_TLV_BSSID, bssid, ETH_ALEN);
	wim_builder_append_u16(wb, WIM_TLV_AID, intf->sta.aid);
	wim_builder_run_wm(wb, true);

	wb = wim_builder_create(intf->vif_id, WIM_CMD_STA_CMD, WB_MIN_SIZE);
	wim_builder_append(wb, WIM_TLV_STA_PARAM, &p, sizeof(p));
	wim_builder_run_wm(wb, true);
}

void wpa_driver_sta_sta_add(struct nrc_wpa_if* intf)
{
	wpa_driver_sta_sta_cmd(intf, WIM_STA_CMD_ADD);
}

void wpa_driver_sta_sta_remove(struct nrc_wpa_if* intf)
{
	wpa_driver_sta_sta_cmd(intf, WIM_STA_CMD_REMOVE);
    wpa_driver_set_channel_width(intf->vif_id, nrc_global->cur_chan, 0); // reset to bandwidth 20M
}

static void wpa_driver_set_channel(void *priv, uint16_t freq) {
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	struct wim_builder *wb = wim_builder_create(intf->vif_id, WIM_CMD_SET, WB_MIN_SIZE);
	struct wim_channel_param wp = {freq, };
	wim_builder_append(wb, WIM_TLV_CHANNEL, &wp, sizeof(wp));
	wim_builder_run_wm(wb, true);
    nrc_global->cur_chan = system_modem_api_mac80211_frequency_to_channel(freq);
}

void wifi_drv_rx_only_init(void)
{
    hal_rf_rx_only_init();
    hal_phy_init();

    hal_lmac_init();
    hal_lmac_set_dl_callback(nrc_mac_rx);
    
    umac_info_init(0, 0);   // STA
    umac_scan_init( nrc_scan_done );
    
    system_default_setting(0);
}


static int  get_nrc_auth_type(int wpa_auth_alg)
{
	if (wpa_auth_alg & WPA_AUTH_ALG_OPEN)
		return 0;
	if (wpa_auth_alg & WPA_AUTH_ALG_SHARED)
		return 1;
	if (wpa_auth_alg & WPA_AUTH_ALG_FT)
		return 2;
	if (wpa_auth_alg & WPA_AUTH_ALG_SAE)
		return 3;

	return 0;
}

static int wpa_driver_nrc_authenticate(void *priv, struct wpa_driver_auth_params *p)
{
	#if 0
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	struct ieee80211_mgmt mgmt = {0,};
	const int len = offsetof(struct ieee80211_mgmt, u.auth.variable);

	wpa_printf(MSG_DEBUG, LTAG "%s freq(%d)", __func__, p->freq);
	intf->sta_type = WIM_STA_TYPE_STA;
    os_memcpy(intf->bss.bssid, p->bssid, ETH_ALEN);
    
	mgmt.frame_control =  IEEE80211_FC(WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_AUTH);

	os_memcpy(mgmt.da, p->bssid, ETH_ALEN);
	os_memcpy(mgmt.sa, intf->addr, ETH_ALEN);
	os_memcpy(mgmt.bssid, p->bssid, ETH_ALEN);

	mgmt.u.auth.auth_alg = get_nrc_auth_type(p->auth_alg);//(p->auth_alg == WPA_AUTH_ALG_SHARED) ? 1 : 0; 
	mgmt.u.auth.auth_transaction = 1;
	mgmt.u.auth.status_code = 0;

	wpa_driver_set_channel(priv, p->freq);

	nrc_transmit(intf, (uint8_t *) &mgmt, len);

	return 0;
	#else
	struct nrc_wpa_if *intf = (struct nrc_wpa_if *)(priv);
	uint8_t *frame = os_malloc(512);
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)frame;
	int len = offsetof(struct ieee80211_mgmt, u.auth.variable) + p->sae_data_len;

	wpa_printf(MSG_DEBUG, LTAG "%s freq(%d)", __func__, p->freq);
	intf->sta_type = WIM_STA_TYPE_STA;
    os_memcpy(intf->bss.bssid, p->bssid, ETH_ALEN);
    
	mgmt->frame_control =  IEEE80211_FC(WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_AUTH);

	os_memcpy(mgmt->da, p->bssid, ETH_ALEN);
	os_memcpy(mgmt->sa, intf->addr, ETH_ALEN);
	os_memcpy(mgmt->bssid, p->bssid, ETH_ALEN);

	mgmt->u.auth.auth_alg = get_nrc_auth_type(p->auth_alg);
	if(p->sae_data_len > 4)
		mgmt->u.auth.auth_transaction = *((le16 *)p->sae_data);
	else
		mgmt->u.auth.auth_transaction = 1;

	if(mgmt->u.auth.auth_alg == 3) //WPA_AUTH_ALG_SAE
		intf->pmf = 1;
	else
		intf->pmf = 0;
	
	mgmt->u.auth.status_code = 0;
	if(p->sae_data_len > 4)
	{
		os_memcpy(&mgmt->u.auth.variable[0], p->sae_data + 4, p->sae_data_len-4);
		len = len - 4;
	}
	//len += p->sae_data_len;

	wpa_driver_set_channel(priv, p->freq);

	nrc_transmit(intf, (uint8_t *) mgmt, len);

	os_free(frame);
	return 0;
	#endif
}

static int wpa_driver_stop_ap(void* priv)
{
	wpa_printf(MSG_DEBUG, LTAG "%s", __func__);
	return 0;
}

void nrc_wpas_disconnect(int vif_id)
{
	struct nrc_wpa_if* intf = wpa_driver_get_interface(vif_id);
	wpa_supplicant_event(intf->wpa_supp_ctx, EVENT_DISASSOC, NULL);
	wpa_driver_set_channel_width(intf->vif_id, nrc_global->cur_chan, 0); // reset to bandwidth 20M
}

const struct wpa_driver_ops wpa_driver_freertos_ops = {
	.name = "freeRTOS",
	.desc = "freeRTOS driver",
	.get_bssid = wpa_driver_nrc_get_bssid,
	.get_ssid = wpa_driver_nrc_get_ssid,
	.set_key = wpa_driver_nrc_set_key,
	.init = NULL,
	.deinit = wpa_driver_nrc_deinit,
	.set_param = wpa_driver_nrc_set_param,
	.set_countermeasures = wpa_driver_nrc_set_countermeasures,
	.deauthenticate = wpa_driver_nrc_deauthenticate,
	.associate = wpa_driver_nrc_associate,
	.add_pmkid = NULL,
	.remove_pmkid = wpa_driver_nrc_remove_pmkid,
	.flush_pmkid = NULL,
	.get_capa = wpa_driver_nrc_get_capa,
	.poll = NULL, //wpa_driver_nrc_poll,
	.get_ifindex = NULL,
	.get_ifname = wpa_driver_nrc_get_ifname,
	.get_mac_addr = wpa_driver_nrc_get_mac_addr,
	.set_operstate = wpa_driver_nrc_set_operstate,
	.mlme_setprotection = NULL,//wpa_driver_nrc_mlme_setprotection,
	.get_hw_feature_data = wpa_driver_nrc_get_hw_feature_data,
	.send_mlme = wpa_driver_nrc_send_mgmt,
	.update_ft_ies = wpa_driver_nrc_update_ft_ies,
	.get_scan_results2 = wpa_driver_nrc_get_scan_results2,
	.set_country = wpa_driver_nrc_set_country,
	.get_country = wpa_driver_nrc_get_country,
	.global_init = wpa_driver_nrc_global_init,
	.global_deinit = wpa_driver_nrc_global_deinit,
	.init2 = wpa_driver_nrc_init,
	.get_interfaces = wpa_driver_nrc_get_interfaces,
	.scan2 = wpa_driver_nrc_scan2,
	.authenticate = wpa_driver_nrc_authenticate,
	.set_ap = wpa_driver_nrc_set_ap,
	.set_acl = wpa_driver_nrc_set_acl,
	.hapd_init = NULL,
	.hapd_deinit = NULL,
	.set_ieee8021x = NULL, // wpa_driver_nrc_set_ieee8021x,
	.set_privacy = NULL, // wpa_driver_nrc_set_privacy,
	.get_seqnum = NULL,
	.flush = NULL,
	.set_generic_elem = NULL, // wpa_driver_nrc_set_generic_elem,
	.read_sta_data = wpa_driver_nrc_read_sta_data,
	.hapd_send_eapol = wpa_driver_nrc_hapd_send_eapol,
	.sta_deauth = NULL,
	.sta_disassoc = NULL,
	.sta_remove = wpa_driver_nrc_sta_remove,
	.hapd_get_ssid = NULL,
	.hapd_set_ssid = NULL,
	.hapd_set_countermeasures = NULL,
	.sta_add = wpa_driver_nrc_sta_add,
	.get_inact_sec = NULL,
	.sta_clear_stats = NULL,
	.set_freq = NULL,
	.set_rts = wpa_driver_nrc_set_rts,
	.set_frag = wpa_driver_nrc_set_frag,
	.sta_set_flags = NULL,
	.set_tx_queue_params = wpa_driver_nrc_set_tx_queue_params,
	.if_add = wpa_driver_nrc_if_add,
	.if_remove = wpa_driver_nrc_if_remove,
	.set_sta_vlan = NULL,
	.commit = NULL,
	.send_ether = NULL,
	.set_radius_acl_auth = wpa_driver_nrc_set_radius_acl_auth,
	.set_radius_acl_expire = wpa_driver_nrc_set_radius_acl_expire,
	.set_ap_wps_ie = NULL,
	.set_supp_port = wpa_driver_nrc_set_supp_port,
	.set_wds_sta = NULL,
	.send_action = wpa_driver_nrc_send_action,
	.send_action_cancel_wait = wpa_driver_nrc_send_action_cancel_wait,
	.remain_on_channel = wpa_driver_nrc_remain_on_channel,
	.cancel_remain_on_channel = wpa_driver_nrc_cancel_remain_on_channel,
	.probe_req_report = wpa_driver_nrc_probe_req_report,
	.deinit_ap = wpa_driver_nrc_deinit_ap,
	.deinit_p2p_cli = wpa_driver_nrc_deinit_p2p_cli,
	.suspend = wpa_driver_nrc_suspend,
	.resume = wpa_driver_nrc_resume,
	.signal_monitor = wpa_driver_nrc_signal_monitor,
	.send_frame = NULL,
	.get_noa = wpa_driver_nrc_get_noa,
	.set_noa = wpa_driver_nrc_set_noa,
	.set_p2p_powersave = wpa_driver_nrc_set_p2p_powersave,
	.ampdu = wpa_driver_nrc_ampdu,
	.get_radio_name = wpa_driver_nrc_get_radio_name,
	.send_tdls_mgmt = NULL,
	.tdls_oper = NULL,
	.wnm_oper = NULL,
	.set_qos_map = NULL,
	.br_add_ip_neigh = NULL,
	.br_delete_ip_neigh = NULL,
	.br_port_set_attr= NULL,
	.br_set_net_param= NULL,
	.set_wowlan = wpa_driver_nrc_set_wowlan,
	.signal_poll = wpa_driver_nrc_signal_poll,
	.set_authmode = NULL,
	.vendor_cmd = wpa_driver_nrc_vendor_cmd,
	.set_rekey_info = NULL,//wpa_driver_nrc_set_rekey_info,
	.sta_assoc = NULL,
	.sta_auth = NULL,
	.add_tspec = NULL,
	.add_sta_node = NULL,
	.sched_scan = wpa_driver_nrc_sched_scan,
	.stop_sched_scan = wpa_driver_nrc_stop_sched_scan,
	.poll_client = wpa_driver_nrc_poll_client,
	.radio_disable = wpa_driver_nrc_radio_disable,
	.switch_channel = wpa_driver_nrc_switch_channel,
	.add_tx_ts = wpa_driver_nrc_add_tx_ts,
	.del_tx_ts = wpa_driver_nrc_del_tx_ts,
	.tdls_enable_channel_switch = NULL,
	.tdls_disable_channel_switch = NULL,
	.start_dfs_cac = NULL,
	.stop_ap = wpa_driver_stop_ap,
	.get_survey = wpa_driver_nrc_get_survey,
	.status = wpa_driver_nrc_status,
	.roaming = NULL,
	.set_mac_addr = wpa_driver_nrc_set_mac_addr,
	.init_mesh = NULL,
	.join_mesh = NULL,
	.leave_mesh = NULL,
	.do_acs = NULL,
	.set_band = NULL,
	.get_pref_freq_list = NULL,
	.set_prob_oper_freq = NULL,
};

#ifdef LTAG
#undef LTAG
#endif

void wifi_drv_init(void)
{
    hal_rf_init();
    hal_phy_init();

    hal_lmac_init();
    hal_lmac_set_dl_callback(nrc_mac_rx);
    
    umac_info_init(0, 0);   // STA
    umac_scan_init( nrc_scan_done );
	
    wpa_driver_get_wifi_nv();
    
    system_default_setting(0);
}

int wifi_drv_set_ap_sta_num(uint8_t vif, int num)
{
    struct nrc_wpa_if *intf = wpa_driver_get_interface(vif);
    if(NULL == intf)
    {
        system_printf("intf null \n");
        return -1;
    }

    intf->max_ap_sta_num = num;
    
    return 0;
}

int wifi_drv_get_ap_rssi(char *out)
{
	struct nrc_wpa_if *intf = wpa_driver_get_interface(0);
	int8_t cnt = 0, rssi;
	int sum = 0;

	while (cnt < MAX_RSSI_RECORD) {
		sum += g_rssi_inst[cnt++];
	}
	
	rssi = sum/MAX_RSSI_RECORD;

	if (rssi >= 0) {
		if (0 == intf->sta.rssi)
			intf->sta.rssi = -70;
		*out = intf->sta.rssi + ((xTaskGetTickCount() % 10) - 5);
	} else {
    	*out = intf->sta.rssi = rssi;
	}
	return 0;
}

int wifi_ctl_drv(drv_ctl_t *ctl, void *result)
{
    int ret = 0;
    switch (ctl->cmd)
    {
        case SET_AP_STA_NUM:
            ret = wifi_drv_set_ap_sta_num(ctl->vif, *((unsigned char *)ctl->value));
            break;

        case GET_CONN_AP_RSSI:
            ret = wifi_drv_get_ap_rssi((char *)result);
            break;

		case SET_COUNTRY_CODE:
            ret = wpa_driver_nrc_set_country(NULL, (const char *)ctl->value);
            break;
		case GET_COUNTRY_CODE:
            ret = wpa_driver_nrc_get_country(NULL, (char *)result);
            break;
            
        default:
            break;
    }

    return ret;
}

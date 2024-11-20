#include "system.h"
#include "utils/includes.h"
#include "utils/common.h"
#include "driver.h"
#include "../common/ieee802_11_common.h"
#include "driver_wifi_scan.h"

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

extern bool scan_is_preferred(struct ieee802_11_elems *elems, struct ieee80211_mgmt *mgmt);

struct nrc_scan_info * scan_init()
{
	struct nrc_scan_info *scan = NULL;

	wpa_printf(MSG_ERROR, "Driver: %s()", __func__);
	scan = (struct nrc_scan_info *) os_zalloc(sizeof(*scan));
	if (!scan) {
		wpa_printf(MSG_ERROR, "Driver: %s() Failed to allocate scan_info", __func__);
		return NULL;
	}
	dl_list_init(&scan->scan_list);

	return scan;
}

static void scan_start_stop(struct nrc_scan_info *scan, bool start)
{
	if (!scan)
		return;
	scan->is_scanning = start;
}

void scan_start(struct nrc_scan_info *scan)
{
	wpa_printf(MSG_ERROR, "Driver: %s()", __func__);
	scan_start_stop(scan, true);
	scan_flush(scan);
}

void scan_stop(struct nrc_scan_info *scan) {
	wpa_printf(MSG_ERROR, "Driver: %s()", __func__);
	scan_start_stop(scan, false);
}

static void dump_scan_entry(struct nrc_wpa_scan_res *res)
{
#if defined(DRIVER_SCAN_DEBUG_DUMP_ENTRY)
	wpa_printf(MSG_ERROR, "BSSID(" MACSTR "), freq(%d), bi(%d), "
		"caps(%d), qual(%d), noise(%d), level(%d) \n",
		MAC2STR(res->res->bssid),
		res->res->freq,
		res->res->beacon_int,
		res->res->caps,
		res->res->qual,
		res->res->noise,
		res->res->level);

	wpa_hexdump(MSG_ERROR, "SCAN ENT", res->res + 1,
		res->res->beacon_ie_len + res->res->ie_len);
#endif //DRIVER_SCAN_DEBUG_DUMP_ENTRY
}

static bool scan_add_entry(struct nrc_scan_info *scan, uint16_t freq,
				uint16_t rssi, uint8_t* frame, uint16_t len)
{
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *) frame;
	const int ie_offset = offsetof(struct ieee80211_mgmt, u.beacon.variable);
	uint16_t ie_len = len - ie_offset;
	struct nrc_wpa_scan_res *res;

	res = (struct nrc_wpa_scan_res *) os_malloc(sizeof(*res));

	if (!res) {
		wpa_printf(MSG_ERROR, "Failed to allocate scan_res");
		return false;
	}

	res->res = os_zalloc(sizeof(*res->res) + ie_len);

	if (!res->res) {
		wpa_printf(MSG_ERROR, "Failed to allocate scan item (ie len:%d)"
			, ie_len);
		return false;
	}

	os_memcpy(res->res->bssid, mgmt->sa, ETH_ALEN);
	res->res->freq = freq;
	res->res->beacon_int = mgmt->u.beacon.beacon_int;
	res->res->caps = mgmt->u.beacon.capab_info;
	res->res->qual = 0;
	res->res->noise = 0;
	os_memcpy(&res->res->tsf, mgmt->u.beacon.timestamp, 8);
	res->res->level = rssi;
	res->res->ie_len = ie_len;
	os_memcpy(res->res + 1, frame + ie_offset ,ie_len);
	dump_scan_entry(res);

	dl_list_add(&scan->scan_list, &res->list);

	return true;
}

/**
  * 1. If the number of scan list exceeds MAX_SCAN_SSID_LIST or,
  * 2. If the channel of DS PARAM IE is not same as that of rxinfo or,
  * 3. If SSID is not filtered,
  * consider valid and return true.
*/
static bool scan_is_valid(struct nrc_scan_info *scan, uint8_t ch, uint16_t rssi,
				struct ieee802_11_elems* elems)
{
	int i;

	if (dl_list_len(&scan->scan_list) >= scan->params.max_num) {
		//umac_scan_hit();
		wpa_printf(MSG_ERROR, "%s: Maxium scan list reached  (%d)",
			__func__, scan->params.max_num);
		return false;
	}

	if (elems->ds_params
		&& elems->ds_params[0] != ch) {
		//wpa_printf(MSG_ERROR, "%s: DS PARAM IE is incorrect (ie: %d/actual: %d)",
		//	__func__, elems->ds_params[0], ch);
		if(!scan_is_all_channels())
			return false;
	}

	return (elems->ssid && elems->supp_rates);
}

#if 0
static bool scan_is_duplicated(struct nrc_scan_info *scan, uint8_t bssid[ETH_ALEN])
{
	struct nrc_wpa_scan_res *res, *tmp;

	if (!scan)
		return false;

	if (dl_list_empty(&scan->scan_list))
		return false;

	dl_list_for_each_safe(res, tmp, &scan->scan_list, struct nrc_wpa_scan_res, list) {
		if (res && res->res) {
			if (memcmp(res->res->bssid, bssid, ETH_ALEN) == 0)
				return true;
		}
	}

	return false;
}
#else
static bool scan_is_duplicated(struct nrc_scan_info *scan, uint8_t bssid[ETH_ALEN], signed char rssi)
{
	struct nrc_wpa_scan_res *res, *tmp;

	if (!scan)
		return false;

	if (dl_list_empty(&scan->scan_list))
		return false;

	dl_list_for_each_safe(res, tmp, &scan->scan_list, struct nrc_wpa_scan_res, list) {
		if (res && res->res) {
			if (memcmp(res->res->bssid, bssid, ETH_ALEN) == 0)
			{
                /*system_printf("bssid:%02x:%02x:%02x:%02x:%02x:%02x level=%d %d\r\n",bssid[0], bssid[1], bssid[2], 
                    bssid[3], bssid[4], bssid[5], (signed char)res->res->level , rssi);*/
                if((signed char)res->res->level < rssi)
                {
                    res->res->level = rssi;
                }
                return true;
            }
		}
	}

	return false;
}
#endif

bool scan_add(struct nrc_scan_info *scan, uint16_t freq, uint16_t rssi,
				uint8_t* frame, uint16_t len)
{
	struct ieee802_11_elems elems;
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *) frame;
	const int ie_offset = offsetof(struct ieee80211_mgmt, u.beacon.variable);
	uint8_t *ie = frame + ie_offset;
	uint16_t ie_len = len - ie_offset;
	uint8_t ch = 0;
	bool pref = false;
	uint16_t real_req;

	if (!scan || !scan->is_scanning) {
		wpa_printf(MSG_ERROR, "Driver: %s() Scan is not running.", __func__);
		return false;
	}

	if (!frame || len < ie_offset) {
		wpa_printf(MSG_ERROR, "Driver: %s Invalid argument (%s)", __func__,
			!scan ? "SCAN" : !frame || len < IEEE80211_HDRLEN ? "FRAME" : "UNKNOWN");
		return false;
	}

	if (scan_is_duplicated(scan, mgmt->bssid, (signed char)rssi))
		return false;

	ieee802_11_parse_elems(ie, ie_len, &elems, 1);
	ieee80211_freq_to_chan(freq, &ch);

	if (!scan_is_valid(scan, ch, rssi, &elems))
		return false;

	pref = scan_is_preferred(&elems, mgmt);

	if(elems.ssid == NULL || elems.ssid_len == 0)
		return false;
	
	wpa_printf(MSG_ERROR, "SSID(%d) : %s %s", ch,
		wpa_ssid_txt(elems.ssid, elems.ssid_len), "");

	if (!pref)
		return false;

	if(elems.ds_params)
	{
		//system_printf("(ie: %d/actual: %d)\r\n", elems.ds_params[0], ch);
		real_req = system_modem_api_channel_to_mac80211_frequency(elems.ds_params[0]);
	}
	else
		real_req = freq;
	
	return scan_add_entry(scan, real_req, rssi, frame, len);
}

void scan_config(struct nrc_scan_info *scan, struct wpa_driver_scan_params *p)
{
	int i = 0;
	if (!p)
		return;

	scan->params = *p;

	wpa_printf(MSG_ERROR, "Preferred SSID (Len:%d)", scan->num_pref_ssids);
	for (i = 0; i < scan->num_pref_ssids; i++) {
		struct wpa_driver_scan_ssid *ss = &scan->pref_ssids[i];

		wpa_printf(MSG_ERROR, "- %s", wpa_ssid_txt(ss->ssid, ss[i].ssid_len));
	}
}

struct wpa_scan_results* get_scan_results(struct nrc_scan_info *scan)
{
	struct nrc_wpa_scan_res *res, *tmp;
	int cnt = 0, i = 0;
	struct wpa_scan_results* results;

	wpa_printf(MSG_ERROR, "Driver: %s()", __func__);

	results = (struct wpa_scan_results *) os_zalloc(sizeof(*results));
	cnt = dl_list_len(&scan->scan_list);

	if (!results) {
		wpa_printf(MSG_ERROR, "Driver: Failed to allocate wpa_scan_results");
		return NULL;
	}

	if (!cnt) {
		results->num = 0;
		return results;
	}

	wpa_printf(MSG_ERROR, "scan item %d \n", cnt );

	results->res = os_zalloc(sizeof(*results->res) * cnt);

	if (!results->res) {
		wpa_printf(MSG_ERROR, "Driver: Failed to allocate scan item.");
		os_free(results);
		return NULL;
	}

	dl_list_for_each_safe(res, tmp, &scan->scan_list, struct nrc_wpa_scan_res, list) {
		results->res[i++] = res->res;
		// the inst of struct wpa_scan_res is being freed on
		// _wpa_supplicant_event_scan_results().
		dl_list_del(&res->list);
		os_free(res);
	}
	results->num = cnt;

	os_get_reltime(&results->fetch_time);

	return results;
}

static void flush_scan_list(struct dl_list* l)
{
	struct nrc_wpa_scan_res *res = NULL, *tmp = NULL;

	wpa_printf(MSG_ERROR, "Driver: %s()", __func__);

	if (dl_list_empty(l))
		return;

	dl_list_for_each_safe(res, tmp, l, struct nrc_wpa_scan_res, list) {
		if (res) {
			if (res->res)
				os_free(res->res);
			dl_list_del(&res->list);
			os_free(res);
		}
	}
}

void scan_flush(struct nrc_scan_info *scan)
{
	wpa_printf(MSG_ERROR, "Driver: %s()", __func__);
	flush_scan_list(&scan->scan_list);
	os_memset(&scan->params, 0, sizeof(scan->params));
}

void scan_deinit(struct nrc_scan_info *scan)
{
	scan_stop(scan);
	os_free(scan);
	scan = NULL;
}

#if defined(DRIVER_SCAN_DEBUG_TEST_SUITE)
static uint8_t test_probe_resp1[] = {
		0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0x02, 0x00, 0xeb, 0x1b, 0xe9, 0xcd, 0x02,
		0x00, 0xeb, 0x1b, 0xe9, 0xcd, 0x00, 0xb0, 0x00,
		0x00, 0x45, 0xce, 0x97, 0x00, 0x64, 0x00, 0x64,
		0x00, 0x01, 0x02, 0x00, 0x05, 0x68, 0x61, 0x6c,
		0x6f, 0x77, 0x01, 0x08, 0x00, 0x00, 0x00, 0x04,
		0x00, 0x00, 0x00, 0xfd, 0x03, 0x01, 0x01, 0x05,
		0x04, 0x00, 0x00, 0x03, 0x00, 0x2d, 0x1a, 0xcc,
		0xdd, 0x18, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01,
		0x00, 0x00, 0x03, 0xa4, 0x00, 0x00, 0x27, 0xa4,
		0x00, 0x00, 0x42, 0x43, 0x5e, 0x00, 0x62, 0x32,
		0x2f, 0xdd, 0x18, 0x00, 0x50, 0xf2, 0x01, 0x17,
		0x30, 0x01, 0x17, 0x95, 0x01, 0x1e, 0x96, 0x01,
		0x1e, 0x97, 0x01, 0x1e, 0x98, 0x01, 0x1e, 0x99,
		0x01, 0x1e, 0x9a, };

static uint8_t test_probe_resp2[] = {
		0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0x22, 0x20, 0x2b, 0x2b, 0xe9, 0xcd, 0x02,
		0x00, 0xeb, 0x1b, 0xe9, 0xcd, 0x00, 0xb0, 0x00,
		0x00, 0x45, 0xce, 0x97, 0x00, 0x64, 0x00, 0x64,
		0x00, 0x01, 0x02, 0x00, 0x05, 0x68, 0x61, 0x6c,
		0x6f, 0x77, 0x01, 0x08, 0x00, 0x00, 0x00, 0x04,
		0x00, 0x00, 0x00, 0xfd, 0x03, 0x01, 0x01, 0x05,
		0x04, 0x00, 0x00, 0x03, 0x00, 0x2d, 0x1a, 0xcc,
		0xdd, 0x18, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01,
		0x00, 0x00, 0x03, 0xa4, 0x00, 0x00, 0x27, 0xa4,
		0x00, 0x00, 0x42, 0x43, 0x5e, 0x00, 0x62, 0x32,
		0x2f, 0xdd, 0x18, 0x00, 0x50, 0xf2, 0x01, 0x17,
		0x30, 0x01, 0x17, 0x95, 0x01, 0x1e, 0x96, 0x01,
		0x1e, 0x97, 0x01, 0x1e, 0x98, 0x01, 0x1e, 0x99,
		0x01, 0x1e, 0x9a, };

void nrc_scan_test() {
	struct nrc_scan_info* scan = scan_init();

	scan_add(scan, 2412, 0, test_probe_resp1, sizeof(test_probe_resp1));
	scan_add(scan, 2412, 0, test_probe_resp1, sizeof(test_probe_resp1));
	scan_add(scan, 2412, 0, test_probe_resp2, sizeof(test_probe_resp2));

	scan_stop(scan);
	scan_deinit(scan);
}
#else
void nrc_scan_test();
#endif

#include "utils/includes.h"
#include "utils/common.h"
#include "utils/wpa_debug.h"
#include "driver.h"
#include "driver_wifi_scan.h"
#include "driver_wifi_debug.h"


//#define pr(...)  wpa_printf(MSG_DEBUG, "" __VA_ARGS__)
#define pr(...)  system_printf("" __VA_ARGS__);
#define EV 	"[Event] "
#define PR 	"[Param] "

void wpa_driver_debug_frame(uint8_t* frame, uint16_t len)
{
	struct ieee80211_mgmt *mgmt = (void *) frame;

	if (len < IEEE80211_HDRLEN) {
		pr("%s: len failed (%d)", __func__, len);
		return;
	}

	pr("%s: fc(%d), dur(%d),"
		" DA:" MACSTR ",SA:" MACSTR ",BSSID:" MACSTR
		"seq(%d) \n", __func__,  mgmt->frame_control, mgmt->duration,
		MAC2STR(mgmt->da), MAC2STR(mgmt->sa), MAC2STR(mgmt->bssid),
		mgmt->seq_ctrl);
}

 const char* wpa_driver_alg_str(enum wpa_alg alg)
 {
	switch (alg) {
		case WPA_ALG_NONE: return "None";
		case WPA_ALG_WEP: return "WEP";
		case WPA_ALG_TKIP: return "TKIP";
		case WPA_ALG_CCMP: return "CCMP";
		case WPA_ALG_IGTK: return "IGTK";
		case WPA_ALG_PMK: return "PMK";
		case WPA_ALG_GCMP: return "GCMP";
		case WPA_ALG_SMS4: return "SMS4";
		case WPA_ALG_KRK: return "KRK";
		case WPA_ALG_GCMP_256: return "GCMP_256";
		case WPA_ALG_CCMP_256: return "CCMP_256";
		case WPA_ALG_BIP_GMAC_128: return "GMAC_128";
		case WPA_ALG_BIP_GMAC_256: return "GMAC_256";
		case WPA_ALG_BIP_CMAC_256: return "CMAC_256";
		default: return "Unknown";
	}
 }

void wpa_driver_debug_assoc_params(struct wpa_driver_associate_params *p) {
	if (!p)
		return;

	pr(PR "association\n");
	if(p->bssid) {
		pr("bssid(" MACSTR")", MAC2STR(p->bssid));
	}
	else {
		pr("bssid(NULL)");
	}
	if(p->bssid_hint) {
		pr(", bssid_hint(" MACSTR ")", MAC2STR(p->bssid_hint));
	}
	else {
		pr(", bssid_hint(NULL)");
	}
	if(p->ssid)
		wpa_hexdump_ascii(MSG_DEBUG, "ssid", p->ssid, p->ssid_len);
	else
		pr(", ssid(NULL)");

	pr("auth algo(%d), disableHT(%d), drop_unexcrypted(%d)"
		", beacon interval(%d), bg scan period(%d), fixed bssid=(%d)"
		", fixed freq(%d), pairwise suite(%d), group suite=(%d)"
		", key_mgmt suite=(%d) \n"
		,p->auth_alg, p->disable_ht, p->drop_unencrypted, p->beacon_int
		, 0, p->fixed_bssid, p->fixed_freq, p->pairwise_suite
		,p->group_suite, p->key_mgmt_suite);

	if(p->wpa_ie_len > 0) {
		wpa_hexdump(MSG_DEBUG, "WPA IE", p->wpa_ie, p->wpa_ie_len);
	}
	else {
		pr( "WPA IE: null \n");
	}
	pr("freq mode(%d), freq(%d), channel(%d), ht_enabled=(%d)"
		"sec_channel_offset=(%d), vht_enabled=(%d), center_freq1=(%d)"
		"center_freq2=(%d), bandwidth=(%d) \n"
		,p->freq.mode, p->freq.freq, p->freq.channel, p->freq.ht_enabled
		,p->freq.sec_channel_offset, p->freq.vht_enabled, p->freq.center_freq1
		,p->freq.center_freq2, p->freq.bandwidth);
}

void wpa_driver_debug_key(struct nrc_wpa_key *key)
{
	pr(PR "key\n");
	pr("wk_keytsc(%lld), wk_keyix(%d), keylen: (%d), ic_cipher: (%d) \n"
		,key->tsc, key->ix, key->key_len
		,key->cipher);
}

#undef pr

#ifndef DRIVER_NRC_H
#define DRIVER_NRC_H


#include <stdbool.h>
#include "system_common.h"
#include "driver.h"
#include "driver_wifi_tx.h"
#include "wifi_sniffer.h"

struct wpa_supplicant;
struct nrc_wpa;

//#define NRC_MAC_TEST_SELF_SCAN

#define NRC_WPA_NUM_INTERFACES			(2)
#define NRC_WPA_INTERFACE_NAME_0		("wlan0")
#define NRC_WPA_INTERFACE_NAME_1		("wlan1")

#define NRC_WPA_SOFTAP_MAX_STA			(4)
#define NRC_WPA_NUM_BEACON_MISS_ALLOWED	(20)
#define NRC_WPA_MIN_DURATION_BEACON_MISS_ALLOWED 	(3)	// In second
#define NRC_WPA_MAX_KEY_LEN 			(32)

#define _FC(T,ST)	IEEE80211_FC( WLAN_FC_TYPE_##T , WLAN_FC_STYPE_##ST )

#define ETH_P_AARP			(0x80F3)
#define ETH_P_IPX			(0x8137)
#define ETH_P_802_3_MIN		(0x0600)
#define HLEN_8023	(sizeof(struct ieee8023_hdr))

#define MAX_RSSI_RECORD (10)
extern int8_t g_rssi_inst[]; 

struct nrc_wpa_key {
	uint64_t tsc;	/* key transmit sequence counter */
	uint16_t ix;	/* Key Index */
	uint8_t cipher; /* WIM Cipher */
	uint8_t addr[ETH_ALEN];
	uint8_t key[NRC_WPA_MAX_KEY_LEN];
	uint16_t key_len;
	bool is_set;
};

struct nrc_wpa_sta {
	struct nrc_wpa_key		key;
	uint8_t 	addr[6];
	uint16_t	aid;
	uint8_t 	state;
	bool		qos;
	uint16_t 	last_mgmt_stype;
	bool		block_ack[MAX_AC];
	int8_t 		rssi;
};

struct nrc_wpa_bss {
	uint8_t	bssid[6];
	uint8_t	ssid[32];
	uint8_t	ssid_len;
	struct nrc_wpa_key	broadcast_key;
	uint16_t beacon_int;
	//struct os_time last_beacon_update;
	bool authorized_1x;
};

struct nrc_wpa_if {
	int	vif_id;
	int sta_type;
	char if_name[6];
	uint8_t addr[6];
	void *wpa_supp_ctx;
	bool associated;
	bool is_ap;
	bool pending_rx_mgmt;
	struct nrc_wpa_sta sta;
	struct nrc_wpa_bss bss;
	struct nrc_scan_info* 	scan;
	struct hostapd_hw_modes* hw_modes;
	struct nrc_wpa	*global;
	struct nrc_wpa_sta *ap_sta[NRC_WPA_SOFTAP_MAX_STA];
	int max_ap_sta_num;
	int num_ap_sta;
	bool pmf;
};

struct nrc_wpa {
    uint16_t  cur_chan;
	struct    wpa_supplicant	*wpa_s;
	struct    nrc_wpa_if 	*intf[NRC_WPA_NUM_INTERFACES];
};

struct nrc_wpa_rx_data {
	struct nrc_wpa_if* intf;
	struct nrc_wpa_sta* sta;
	LMAC_RXHDR *rxh;
	RXINFO	*rxi;
	uint16_t len;
	union {
		uint8_t *frame;
		struct ieee80211_mgmt *mgmt;
		struct ieee80211_hdr *hdr;
	} u;
	bool is_8023;
	uint16_t offset_8023;
	SYS_BUF* buf;
	uint32_t ref_time;
};

struct nrc_wpa_rx_event {
	uint8_t *frame;
	uint16_t len;
	uint16_t freq;
	void* pv;
	uint16_t subtype;
	uint32_t ref_time;
};

struct nrc_wpa_log_event {
	char *msg;
	uint16_t len;
	int level;
};

typedef struct {
    wifi_promiscuous_filter_t filter;
    wifi_promiscuous_cb_t promisc_cb;
} wifi_promiscuous_cfg_t;

extern struct nrc_wpa *nrc_global;
extern uint8_t g_standalone_addr[6];

typedef struct {
    uint8_t  cmd;
    uint8_t  vif;
    uint16_t len;
    uint8_t  value[0];
}drv_ctl_t;

typedef enum
{
    SET_AP_STA_NUM,
    GET_CONN_AP_RSSI,
    SET_COUNTRY_CODE,
    GET_COUNTRY_CODE,
} drv_ctrl_cmd_e;

struct nrc_wpa_if *wpa_driver_get_interface(int vif);
struct nrc_wpa_sta* nrc_wpa_find_sta(struct nrc_wpa_if *intf, const uint8_t addr[ETH_ALEN]);
struct nrc_wpa_key *nrc_wpa_get_key(struct nrc_wpa_if *intf, const uint8_t *addr);
void wpa_driver_sta_sta_add(struct nrc_wpa_if* intf);
void wpa_driver_sta_sta_remove(struct nrc_wpa_if* intf);
int wpas_l2_packet_filter(uint8_t *buffer, int len);
int nrc_transmit(struct nrc_wpa_if* intf, uint8_t *frm, const uint16_t len);
int nrc_raw_transmit(struct nrc_wpa_if* intf, uint8_t *frm, const uint16_t len,
				const int ac);
int nrc_get_sec_hdr_len(struct nrc_wpa_key *key);
void nrc_wpas_disconnect(int vif_id);

// Helper functions
static inline bool is_wep(uint8_t cipher) {
	return (cipher == WIM_CIPHER_TYPE_WEP40 || cipher == WIM_CIPHER_TYPE_WEP104);
}

static inline bool is_key_wep(struct nrc_wpa_key *key) {
	return (key && is_wep(key->cipher));
}

/*[Begin] [wangxia-2019/1/5]*/
void nrc_scan_done(int vif);
void nrc_mac_rx(SYS_BUF *head);
/*[End] [wangxia-2019/1/5]*/

int wifi_drv_get_ap_rssi(char *rssi);
int wifi_ctl_drv(drv_ctl_t *ctl, void *result);

#endif // DRIVER_NRC_H

#ifndef UMAC_SCAN_H
#define UMAC_SCAN_H

#include "nrc-wim-types.h"

#define UMAC_SCAN_CONFIG_MAX_PROBEREQ_LEN    512
#define UMAC_SCAN_CONFIG_INTERVAL           (100) // 100 ms

typedef struct {
    struct _SYS_BUF*        probereq;
    uint16_t        probereq_len;
    void            (*scan_done_cb)(int);
    uint8_t         n_channels;
    uint16_t        scan_channel[WIM_MAX_SCAN_CHANNEL];
    uint16_t        scan_channel_index;
    uint16_t        current_channel;
    uint8_t			*pos_dsparam_ie;
    uint32_t 		flags;
    uint8_t            scan_hit;
    uint8_t         passive;
    uint32_t        scan_time;
    uint8_t         enable;
    uint8_t         vif_id;
	uint8_t         ssid_len;
    uint8_t         ssid[IEEE80211_MAX_SSID_LEN];
	uint8_t         bssid_set;
    uint8_t         bssid[6];
    void            *delay_param;
} scan_info;

void        umac_scan_init( void (*cb)(int) );
void        umac_scan_deinit(void);
void        umac_scan_set_param(int vif_id, struct wim_scan_param* param);
void        umac_scan_set_probereq_ie(uint16_t len , uint8_t* ie);
uint8_t*    umac_scan_get_probereq_ie_buffer(uint16_t len);
void        umac_scan_start();
int32_t     umac_scan_done(void*);
void        umac_scan_set_return_channel(uint16_t channel);
void        umac_scan_set_channel(uint16_t channel);
bool        scan_is_all_channels(void);

#define UMAC_SCAN_MAC_HDR_LEN   24
#define UMAC_SCAN_EID_SSID      0

#endif

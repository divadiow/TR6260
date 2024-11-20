/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:    system_event.h
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       V1.0
 * Author:        lixiao
 * Date:          2018-12-21
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef _SYSTEM_EVENT_H
#define _SYSTEM_EVENT_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/
#include "system_wifi.h"
#include "defs.h"

/****************************************************************************
* 	                                        Macros
****************************************************************************/
typedef enum {
    SYSTEM_EVENT_WIFI_READY = 0,           /**< WiFi ready */
    SYSTEM_EVENT_SCAN_DONE,                /**< finish scanning AP */
    SYSTEM_EVENT_STA_START,                /**< station start */
    SYSTEM_EVENT_STA_STOP,                 /**< station stop */
    SYSTEM_EVENT_STA_CONNECTED,            /**< station connected to AP */
    SYSTEM_EVENT_STA_DISCONNECTED,         /**< station disconnected from AP */
    SYSTEM_EVENT_STA_AUTHMODE_CHANGE,      /**< the auth mode of AP connected by station changed */
    SYSTEM_EVENT_STA_GTK_REKEY,            /**< station in gtk rekey */
    SYSTEM_EVENT_STA_GOT_IP,               /**< station got IP from connected AP */
    SYSTEM_EVENT_STA_LOST_IP,              /**< station lost IP and the IP is reset to 0 */
    SYSTEM_EVENT_STA_4WAY_HS_FAIL,         /**< station 4-Way Handshake failed, pre-shared key may be incorrect*/
    SYSTEM_EVENT_STA_WPS_ER_SUCCESS,       /**< station wps succeeds in enrollee mode */
    SYSTEM_EVENT_STA_WPS_ER_FAILED,        /**< station wps fails in enrollee mode */
    SYSTEM_EVENT_STA_WPS_ER_TIMEOUT,       /**< station wps timeout in enrollee mode */
    SYSTEM_EVENT_STA_WPS_ER_PIN,           /**< station wps pin code in enrollee mode */
    SYSTEM_EVENT_AP_START,                 /**< soft-AP start */
    SYSTEM_EVENT_AP_STOP,                  /**< soft-AP stop */
    SYSTEM_EVENT_AP_STACONNECTED,          /**< a station connected to soft-AP */
    SYSTEM_EVENT_AP_STADISCONNECTED,       /**< a station disconnected from soft-AP */
    SYSTEM_EVENT_AP_STAIPASSIGNED,         /**< soft-AP assign an IP to a connected station */
    SYSTEM_EVENT_AP_PROBEREQRECVED,        /**< Receive probe request packet in soft-AP interface */
    SYSTEM_EVENT_GOT_IP6,                  /**< station or ap or ethernet interface v6IP addr is preferred */
    SYSTEM_EVENT_ETH_START,                /**< ethernet start */
    SYSTEM_EVENT_ETH_STOP,                 /**< ethernet stop */
    SYSTEM_EVENT_ETH_CONNECTED,            /**< ethernet phy link up */
    SYSTEM_EVENT_ETH_DISCONNECTED,         /**< ethernet phy link down */
    SYSTEM_EVENT_ETH_GOT_IP,               /**< ethernet got IP from connected AP */
    SYSTEM_EVENT_WPA_INTERNAL_STATE,       /**< wpa_supplicant internal state  */
    SYSTEM_EVENT_STA_ASSOC_REJECT,         /**< station assoc fail*/
    SYSTEM_EVENT_MAX
} system_event_id_t;

/****************************************************************************
* 	                                        Types
****************************************************************************/
typedef struct {
    uint32_t status;          /**< status of scanning APs */
    uint8_t  number;
    uint8_t  scan_id;
} system_event_sta_scan_done_t;

typedef struct {
    uint8_t ssid[WIFI_SSID_MAX_LEN];         /**< SSID of connected AP */
    uint8_t ssid_len;                        /**< SSID length of connected AP */
    uint8_t bssid[6];                        /**< BSSID of connected AP*/
    uint8_t channel;                         /**< channel of connected AP*/
    wifi_auth_mode_e authmode;
} system_event_sta_connected_t;

typedef struct {
    uint8_t ssid[WIFI_SSID_MAX_LEN];         /**< SSID of disconnected AP */
    uint8_t ssid_len;                        /**< SSID length of disconnected AP */
    uint8_t bssid[6];                        /**< BSSID of disconnected AP */
    uint8_t reason;                          /**< reason of disconnection */
} system_event_sta_disconnected_t;

typedef struct {
    wifi_auth_mode_e old_mode;         /**< the old auth mode of AP */
    wifi_auth_mode_e new_mode;         /**< the new auth mode of AP */
} system_event_sta_authmode_change_t;

typedef struct {
    ip_info_t ip_info;
    bool ip_changed;
} system_event_sta_got_ip_t;

typedef struct {
    uint8_t mac[6];           /**< MAC address of the station connected to soft-AP */
    uint8_t aid;              /**< the aid that soft-AP gives to the station connected to  */
    ip_info_t ip_info;
} system_event_ap_staconnected_t;

typedef struct {
    uint8_t mac[6];           /**< MAC address of the station disconnects to soft-AP */
    uint8_t aid;              /**< the aid that soft-AP gave to the station disconnects to  */
    ip_info_t ip_info;
} system_event_ap_stadisconnected_t;

typedef struct {
    uint8_t in_out;           /**< sta enter(1)/exit(0) gtk rekey */
}system_event_sta_gtk_rekey_t;


typedef struct {
    enum wpa_states state; 
}system_event_wpa_internal_state_t;


typedef union {
    system_event_sta_connected_t               connected;          /**< station connected to AP */
    system_event_sta_disconnected_t            disconnected;       /**< station disconnected to AP */
    system_event_sta_scan_done_t               scan_done;          /**< station scan (APs) done */
    system_event_sta_authmode_change_t         auth_change;        /**< the auth mode of AP station connected to changed */
    system_event_sta_got_ip_t                  got_ip;             /**< station got IP, first time got IP or when IP is changed */
    system_event_ap_staconnected_t             sta_connected;      /**< a station connected to soft-AP */
    system_event_ap_stadisconnected_t          sta_disconnected;   /**< a station disconnected to soft-AP */
    system_event_sta_gtk_rekey_t               in_out_rekey;       /**< station enter/exit gtk rekey */
	system_event_wpa_internal_state_t          wpa_internal_state;
} system_event_info_t;

typedef struct {
    uint8_t               vif;
    system_event_id_t     event_id;      /**< event ID */
    system_event_info_t   event_info;    /**< event information */
} system_event_t;

typedef sys_err_t (*system_event_handler_t)(system_event_t *event);
typedef sys_err_t (*system_event_cb_t)(void *ctx, system_event_t *event);

/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/
sys_err_t sys_event_loop_init(system_event_cb_t cb, void *ctx);
sys_err_t sys_event_process_default(system_event_t *event);
system_event_cb_t sys_event_loop_set_cb(system_event_cb_t cb, void *ctx);
sys_err_t sys_event_send(system_event_t *event);
void sys_event_set_default_wifi_handlers();
void sys_event_reset_wifi_handlers(system_event_id_t eventid, system_event_handler_t func);
void wifi_handle_sta_connect_event(system_event_t *event);


#endif/*_FILENAME_H*/


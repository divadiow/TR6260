/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:    
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       V1.0
 * Author:        lixiao
 * Date:          2018-12-12
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

/****************************************************************************
* 	                                           Include files
****************************************************************************/
#include "system.h"
#include "standalone.h"
#include "system_wifi.h"
#include "system_network.h"
#include "system_event.h"
#include "system_modem_api.h"
#include "drv_spiflash.h"
#include "nv_config.h"
#include "easyflash.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "utils/common.h"
#include "wpa_supplicant_i.h"
#include "config.h"
#include "apps/dhcpserver/dhcpserver.h"
#include "driver.h"
#include "hostapd.h"
#include "driver_wifi.h"
#include "amtNV.h"


/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define  WIFI_CMD_LEN  						 100
#define  CHIP_TYPE_ADDR_EFUSE		 		 4
#define  MAC_OTP_ADDR_GD25Q80E_FLASH 		 0x1030
#define  read_mac_valid                      0x0
#define  SCAN_DELAY_TIMEMS                   (1500)
/****************************************************************************
* 	                                           Local Types
****************************************************************************/


/****************************************************************************
* 	                                           Local Constants
****************************************************************************/

/****************************************************************************
* 	                                           Local Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Global Constants
****************************************************************************/

/****************************************************************************
* 	                                          Global Variables
****************************************************************************/
network_db_t network_db;
wifi_nv_info_t wifi_nv_info;
wifi_ap_config_t g_softap_conf;
static uint8_t wifi_init_complete_flag = 0;
/****************************************************************************
* 	                                          Global Function Prototypes
****************************************************************************/
extern int wpa_cmd_receive_str(int vif_id, char *cmd);
extern struct wpa_supplicant *wpa_get_ctrl_iface(int vif_id);
extern sys_err_t wpa_get_bss_mode(struct wpa_bss *bss, uint8_t *auth, uint8_t *cipher);
extern int wifi_drv_get_ap_rssi(char *rssi);

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/
    
/*caller must ensure idx is valid index.*/
inline struct netif *get_netif_by_index(int idx)
{
    if (!IS_VALID_VIF(idx)) {
        SYS_LOGE("!!!!get netif[%d], return null\n", idx);
        return NULL;
    }
    return network_db.netif_db[idx].net_if;
}

inline void set_netif_by_index(struct netif *nif, int idx)
{
    if (!IS_VALID_VIF(idx))
        return;

    network_db.netif_db[idx].net_if = nif;
}

inline netif_db_t *get_netdb_by_index(int idx)
{
    if (!IS_VALID_VIF(idx))
        return NULL;
    return &network_db.netif_db[idx];
}

/*******************************************************************************
 * Function: wifi_ctrl_iface
 * Description:  send cmd to wpa supplicant
 * Parameters: 
 *   Input: cmd to send, format as "set_network 0 ssid "test_ap_ccmp""
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
int wifi_ctrl_iface(int vif, char *cmd)
{
    if (!cmd || !cmd[0])
        return -1;
    //SYS_LOGV("--%s--vif[%d]:%s\n", __func__, vif, cmd);
    return wpa_cmd_receive_str(vif, cmd);
}

static void wifi_netif_control(int vif, bool enable)
{
    struct netif *nif;
    if(enable){
        // wifi_remove_config_all(vif);
        wifi_add_config(vif);
        nif = get_netif_by_index(vif);
        netif_set_up(nif);
    } else {
        wifi_remove_config_all(vif);
        nif = get_netif_by_index(vif);
        netif_set_down(nif);
    }
}
void wifi_set_opmode(wifi_work_mode_e opmode)
{
    uint8_t sta_enable = 0, ap_enable = 0;
    
    netif_db_t *nif;
    if (opmode > WIFI_MODE_AP_STA || opmode < WIFI_MODE_STA)
        return;

    if (WIFI_MODE_AP_STA == opmode) {
        sta_enable = ap_enable = 1;
#ifndef TUYA_SDK_ADPT
        wifi_netif_control(STATION_IF,true);
        wifi_netif_control(SOFTAP_IF,true);
#endif
    } else if (WIFI_MODE_STA == opmode) {
        sta_enable = 1;
#ifndef TUYA_SDK_ADPT
        wifi_netif_control(STATION_IF,true);
        wifi_netif_control(SOFTAP_IF,false);
#endif
    } else if (WIFI_MODE_AP == opmode) {
        ap_enable = 1;
#ifndef TUYA_SDK_ADPT
        wifi_netif_control(SOFTAP_IF ,true);
        wifi_netif_control(STATION_IF ,false);
#endif
    }
    system_modem_api_enable_mac_address(STATION_IF, sta_enable);
    system_modem_api_enable_mac_address(SOFTAP_IF, ap_enable);
        
	network_db.mode = opmode;
	return;
}

wifi_work_mode_e wifi_get_opmode(void)
{
	return network_db.mode;
}

/*******************************************************************************
 * Function: wifi_set_ready_flag
 * Description:  set wifi_set_ready_flag, only called by system_event_wifi_init_ready()
 * Parameters: 
 *   Input: init_complete_flag
 *
 *   Output:
 *
 * Returns: void
 *
 *
 * Others: 
 ********************************************************************************/
void wifi_set_ready_flag(uint8_t init_complete_flag)
{
	if (init_complete_flag != 0){
        wifi_init_complete_flag = 1;
   }else{
        wifi_init_complete_flag = 0;
   }
}

/*******************************************************************************
 * Function: wifi_is_ready
 * Description:  set AP/STA info after wifi_is_ready() return true
 * Parameters: 
 *   Input: none
 *
 *   Output: true or false
 *
 * Returns: bool
 *
 *
 * Others: 
 ********************************************************************************/
bool wifi_is_ready(void)
{
	if (wifi_init_complete_flag == 0){
        return false;
   }else{
        return true;
   }
}

sys_err_t wifi_system_init(void)
{
    static uint8_t started = 0;

    if (!started) {
        started = 1;
        memset(&network_db, 0, sizeof(network_db));
        memset(&wifi_nv_info, 0, sizeof(wifi_nv_info));
	memset(&g_softap_conf,0,sizeof(g_softap_conf));
#ifdef TUYA_SDK_ADPT
        network_db.mode = WIFI_MODE_AP_STA;
#elif ENABLE_EZIOT
        network_db.mode = WIFI_MODE_AP_STA;
#else
        network_db.mode = WIFI_MODE_STA;
#endif
    }
    return SYS_OK;
}

void wifi_set_status(int vif, wifi_status_e status)
{
    netif_db_t *nif;

    if (!IS_VALID_VIF(vif))
        return;

    nif = get_netdb_by_index(vif);
    nif->info.wifi_status = status;
}

wifi_status_e wifi_get_status(int vif)
{
    netif_db_t *nif;

    if (!IS_VALID_VIF(vif))
        return STA_STATUS_STOP;

    nif = get_netdb_by_index(vif);
    return nif->info.wifi_status;
}

wifi_status_e wifi_get_ap_status()
{
    return wifi_get_status(SOFTAP_IF);
}

wifi_status_e wifi_get_sta_status()
{
    return wifi_get_status(STATION_IF);
}

int check_wifi_link_on(int vif)
{
    if (!IS_VALID_VIF(vif))
        return 0;

    wifi_status_e status = wifi_get_status(vif);
    if (STA_STATUS_CONNECTED == status || AP_STATUS_STARTED == status)
        return 1;

    return 0;    
}

void wifi_hanlde_sta_connect_event_nosave(system_event_t *event)
{
    netif_db_t *nif;

    if (!IS_VALID_VIF(event->vif))
        return;

    nif = get_netdb_by_index(event->vif);
    if (event->event_id == SYSTEM_EVENT_STA_DISCONNECTED) {
        if (STA_STATUS_STOP != wifi_get_status(event->vif))
            wifi_set_status(event->vif, STA_STATUS_DISCON);
        wifi_station_dhcpc_stop(event->vif);
        nif->info.channel = 0;
    } else if (event->event_id == SYSTEM_EVENT_STA_CONNECTED) {
        wifi_set_status(event->vif, STA_STATUS_CONNECTED);
        wifi_station_dhcpc_start(event->vif);
        nif->info.channel = event->event_info.connected.channel;
    }
}

void wifi_handle_sta_connect_event(system_event_t *event)
{
    netif_db_t *nif;

    if (!IS_VALID_VIF(event->vif))
        return;

    nif = get_netdb_by_index(event->vif);
    if (event->event_id == SYSTEM_EVENT_STA_DISCONNECTED) {
        if (STA_STATUS_STOP != wifi_get_status(event->vif))
            wifi_set_status(event->vif, STA_STATUS_DISCON);
        wifi_station_dhcpc_stop(event->vif);
        nif->info.channel = 0;
    } else if (event->event_id == SYSTEM_EVENT_STA_CONNECTED) {
        wifi_set_status(event->vif, STA_STATUS_CONNECTED);
        wifi_station_dhcpc_start(event->vif);
        nif->info.channel = event->event_info.connected.channel;
        wifi_update_nv_sta_connected((char *)event->event_info.connected.ssid, event->event_info.connected.bssid, 
            event->event_info.connected.channel);
    }
}
void wifi_handle_ap_staipassigned_event(int vif, struct dhcps_msg* m)
{
	system_event_t event;
	char ipaddr[16]={0};

    if(NULL == m)
		return;

    memset(&event, 0, sizeof(system_event_t));
	event.event_id = SYSTEM_EVENT_AP_STAIPASSIGNED;
	snprintf(ipaddr, 15, IP4_ADDR_STR, m->yiaddr[0], m->yiaddr[1], m->yiaddr[2], m->yiaddr[3]);
	ipaddr_aton(ipaddr,&event.event_info.sta_connected.ip_info.ip);
	memcpy(event.event_info.sta_connected.mac,m->chaddr,ETH_ALEN);
    event.vif = vif;

    sys_event_send(&event);

	return;
}

void wifi_handle_ap_connect_event(int vif, const uint8_t *addr,uint16_t aid,uint8_t type)
{
	system_event_t event;
	if(NULL == addr)
		return;

	memset(&event,0,sizeof(system_event_t));
    event.vif = vif;
	memcpy(event.event_info.sta_connected.mac,addr,ETH_ALEN);
	event.event_info.sta_connected.aid = aid;
    event.event_id = type ? SYSTEM_EVENT_AP_STACONNECTED : SYSTEM_EVENT_AP_STADISCONNECTED;

	sys_event_send(&event);
	return;
}

int wifi_remove_config_all(int vif)
{
    return wifi_ctrl_iface(vif, "remove_network all");
}

int wifi_add_config(int vif)
{
    return wifi_ctrl_iface(vif, "add_network");
}

int wifi_config_ssid(int vif, unsigned char *ssid)
{
	char buf[WIFI_CMD_LEN] = {0};
	char *pos = buf, *end = buf + WIFI_CMD_LEN - 1;
	int i = 0;
    
    if (!ssid || !ssid[0])
        return -1;

    sprintf(buf, "%s", "set_network 0 ssid ");
    pos += strlen(buf);
    while (pos != end && ssid[i] != '\0' && i < WIFI_SSID_MAX_LEN) {
        sprintf(pos, "%2X", ssid[i++]);
        pos += 2;
    }
        
    return wifi_ctrl_iface(vif, buf);
}

int wifi_config_hidden_ssid(int vif, uint8_t hidden)
{
    char buf[WIFI_CMD_LEN] = {0};

    sprintf(buf, "set_network %d ignore_broadcast_ssid %d", 0, hidden ? 1 : 0);
    return wifi_ctrl_iface(vif, buf);
}

int wifi_config_ap_mode(int vif)
{
    char buf[32] = {0};

    sprintf(buf, "set_network %d mode 2", 0);
    return wifi_ctrl_iface(vif, buf);
}

int wifi_config_channel(int vif, int channel)
{
    int frequency = 0;
    char buf[128] = {0};

    if (channel < 1 || channel > 14)
        return -1;
    
    frequency = system_modem_api_channel_to_mac80211_frequency(channel);
    snprintf(buf, sizeof(buf), "set_network %d frequency %d", 0, frequency);
    return wifi_ctrl_iface(vif, buf);
}

int wifi_config_commit(int vif)
{
    char buf[32] = {0};

    sprintf(buf, "select_network %d", 0);
    return wifi_ctrl_iface(vif, buf);
}

int wifi_config_encrypt(int vif, char *pwd, wifi_auth_mode_e mode)
{
    char buf[128] = {0}, *pairwise = NULL, *group = NULL, *key_mgmt = NULL;
    
    if (!pwd || !pwd[0]) {
        mode = AUTH_OPEN;
    }

    switch (mode) {
        case AUTH_OPEN:
        case AUTH_WEP:
            pairwise = key_mgmt = "NONE";
            group = "GTK_NOT_USED";
            break;
        case AUTH_WPA_PSK:
            pairwise = group = "TKIP";
            key_mgmt = "WPA-PSK";
            break;
        case AUTH_WPA2_PSK:
        case AUTH_WPA_WPA2_PSK:
            pairwise = group = "CCMP";
            key_mgmt = "WPA-PSK";
            break;
        default: break;
    }

    if (pairwise && group && key_mgmt) {
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "set_network %d pairwise %s", 0, pairwise);
        wifi_ctrl_iface(vif, buf);
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "set_network %d group %s", 0, group);
        wifi_ctrl_iface(vif, buf);
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "set_network %d key_mgmt %s", 0, key_mgmt); 
        wifi_ctrl_iface(vif, buf);
    }

    if (AUTH_WPA_WPA2_PSK == mode || AUTH_WPA2_PSK == mode || AUTH_WPA_PSK == mode) {
        wifi_set_password(vif, pwd);
    } else if (AUTH_WEP == mode) {
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "set_network %d wep_key0 \"%s\"", 0, pwd);
        wifi_ctrl_iface(vif, buf);
    }

    return SYS_OK;
}

sys_err_t wifi_sniffer_start(wifi_promiscuous_cb_t cb, wifi_promiscuous_filter_t *filter)
{
    wifi_set_promiscuous_filter(filter);
    wifi_set_promiscuous_rx_cb(cb);
    wifi_set_promiscuous(true);

    return SYS_OK;
}

sys_err_t wifi_sniffer_stop(void)
{
    wifi_set_promiscuous(0);
    wifi_set_promiscuous_rx_cb(NULL);

    return SYS_OK;
}

sys_err_t wifi_rf_set_channel(uint8_t channel)
{
    int frequency = 0;

    frequency = system_modem_api_channel_to_mac80211_frequency(channel);
    if (false == system_modem_api_set_channel(0, frequency))
        return SYS_ERR;
    return SYS_OK;
}

uint8_t wifi_rf_get_channel(void)
{
    return (uint8_t)system_modem_api_get_current_channel_number();
}

sys_err_t wifi_send_raw_pkt(const uint8_t *frame, const uint16_t len)
{
    if (!wifi_drv_raw_xmit(STATION_IF, frame, len))
        return SYS_OK;
    else
        return SYS_ERR;
}

sys_err_t wifi_sta_auto_conn_conf(uint8_t *val,uint8_t type)
{
	/*type =0:write 1:read */
	if(type == 0)
	{
		if(ef_set_env_blob(NV_WIFI_AUTO_CONN, val, sizeof(val)))
		{
			return SYS_ERR;
		}
	}
	else 
		ef_get_env_blob(NV_WIFI_AUTO_CONN,val, sizeof(val), NULL);
	return SYS_OK;
}

sys_err_t wifi_sta_auto_conn_start(void)
{
    wifi_nv_info_t *nv_info = &wifi_nv_info;

    if(nv_info->auto_conn == 1 && nv_info->sta_ssid[0] != 0){
        wifi_set_sta_by_nv(nv_info);
    }

    return SYS_OK;
}

sys_err_t wifi_softap_auto_start(void)
{
	/*\BB\F1??\BA\F3\D7?\AF\C6\F4\D3\C3soft ap*/
	wifi_config_u wificonfig;
	wifi_ap_config_t softap_conf;
	memset(&softap_conf,0,sizeof(softap_conf));
	memset(&wificonfig,0,sizeof(wificonfig));
	if(SYS_OK == wifi_load_ap_nv_info(&softap_conf))
	{
		memcpy(&wificonfig.ap,&softap_conf,sizeof(softap_conf));
		wifi_start_softap(&wificonfig);

	}
	return SYS_OK;
}

sys_err_t wifi_load_ap_nv_info(wifi_ap_config_t *softap_info)
{
    if (!softap_info)
        softap_info = &g_softap_conf;
    
    memset(softap_info, 0, sizeof(*softap_info));
    if (0 >= ef_get_env_blob(NV_AP_SSID, softap_info->ssid, sizeof(softap_info->ssid) - 1, NULL)) {
        return SYS_ERR;
    }
	
    ef_get_env_blob(NV_AP_PWD, softap_info->password, sizeof(softap_info->password) - 1, NULL);
    ef_get_env_blob(NV_AP_CHANNEL, &softap_info->channel, sizeof(softap_info->channel), NULL);
    ef_get_env_blob(NV_AP_AUTHMOD, &softap_info->authmode, sizeof(softap_info->authmode), NULL);
    ef_get_env_blob(NV_AP_MAX_STA_NUM,&softap_info->max_connect, sizeof(softap_info->max_connect), NULL);
    ef_get_env_blob(NV_AP_HIDDEN_SSID,&softap_info->hidden_ssid, sizeof(softap_info->hidden_ssid), NULL);

    system_printf("AP wifi nv:ssid[%s],pwd[%s],channel[%d]",softap_info->ssid, softap_info->password, softap_info->channel);

    return SYS_OK;
}

sys_err_t wifi_save_ap_nv_info(wifi_ap_config_t *softap_info)
{
    int ret = 0;

    if (!softap_info)
        return SYS_ERR_INVALID_ARG;

    ret |= ef_set_env_blob(NV_AP_SSID, softap_info->ssid, strlen((char *)softap_info->ssid));
    ret |= ef_set_env_blob(NV_AP_PWD, softap_info->password, strlen(softap_info->password));
    ret |= ef_set_env_blob(NV_AP_CHANNEL, &softap_info->channel, sizeof(softap_info->channel));
    ret |= ef_set_env_blob(NV_AP_AUTHMOD, &softap_info->authmode, sizeof(softap_info->authmode));
    ret |= ef_set_env_blob(NV_AP_MAX_STA_NUM, &softap_info->max_connect, sizeof(softap_info->max_connect));
    ret |= ef_set_env_blob(NV_AP_HIDDEN_SSID, &softap_info->hidden_ssid, sizeof(softap_info->hidden_ssid));

    if (ret) { //roll back.
        ef_del_env(NV_AP_SSID);
        ef_del_env(NV_AP_PWD);
        ef_del_env(NV_AP_CHANNEL);
        ef_del_env(NV_AP_AUTHMOD);
        ef_del_env(NV_AP_MAX_STA_NUM);
        ef_del_env(NV_AP_HIDDEN_SSID);
        return SYS_ERR;
    }
    
    return SYS_OK;
}

sys_err_t wifi_get_softap_info(wifi_ap_config_t *softap_info)
{
    if (WIFI_MODE_AP!= wifi_get_opmode() && WIFI_MODE_AP_STA != wifi_get_opmode()) {
        return SYS_ERR_WIFI_MODE;
    }
    
    // if (STA_STATUS_STOP < wifi_get_status(STATION_IF)) {
    //     return SYS_ERR_WIFI_BUSY;
    // }
    
	uint8_t cipher = 0;
	at_get_ap_ssid_passwd_chanel((char *)softap_info->ssid, softap_info->password, &softap_info->channel);
	softap_info->authmode = g_softap_conf.authmode;
	return SYS_OK;
}

sys_err_t wifi_start_softap(wifi_config_u *config)
{
    netif_db_t *nif;
    
    if (WIFI_MODE_AP != wifi_get_opmode() && WIFI_MODE_AP_STA != wifi_get_opmode()) {
        return SYS_ERR_WIFI_MODE;
    }

    // fix the channel,  should set sta's channel if sta started.
    if (WIFI_MODE_AP_STA == wifi_get_opmode()) {
        nif = get_netdb_by_index(STATION_IF);
        if (nif->info.channel)
           config->ap.channel =  nif->info.channel;
    }
    
    if (WIFI_MODE_AP == wifi_get_opmode()) {
        wifi_remove_config_all(STATION_IF); // clear sta config.
    }
    
    wifi_remove_config_all(SOFTAP_IF);
    wifi_add_config(SOFTAP_IF);

    wifi_config_ssid(SOFTAP_IF, config->ap.ssid);
    wifi_config_hidden_ssid(SOFTAP_IF, config->ap.hidden_ssid);
    wifi_config_encrypt(SOFTAP_IF, config->ap.password, config->ap.authmode);
    wifi_config_channel(SOFTAP_IF, config->ap.channel);
    wifi_config_ap_mode(SOFTAP_IF);
    wifi_config_commit(SOFTAP_IF);

    return SYS_OK;
}

sys_err_t wifi_stop_softap(void)
{
    wifi_remove_config_all(SOFTAP_IF);
    wifi_softap_dhcps_stop(SOFTAP_IF);

    return SYS_OK;
}

sys_err_t wifi_start_station(wifi_config_u *config)
{
    if (WIFI_MODE_STA != wifi_get_opmode() && WIFI_MODE_AP_STA != wifi_get_opmode()) {
        return SYS_ERR_WIFI_MODE;
    }
    
    if (STA_STATUS_STOP < wifi_get_status(STATION_IF)) {
        return SYS_ERR_WIFI_BUSY;
    }
    
    if (WIFI_MODE_STA == wifi_get_opmode()) {
        wifi_remove_config_all(SOFTAP_IF); // clear ap config.
    }

    wifi_remove_config_all(STATION_IF);
    wifi_add_config(STATION_IF);

    wifi_config_ssid(STATION_IF, config->sta.ssid);
    wifi_set_password(STATION_IF, config->sta.password);
    if (!(IS_ZERO_MAC(config->sta.bssid) || IS_MULTCAST_MAC(config->sta.bssid)))
        wifi_set_bssid(config->sta.bssid);
    wifi_config_channel(STATION_IF, config->sta.channel);
    wifi_config_commit(STATION_IF);
    wifi_set_status(STATION_IF, STA_STATUS_START);

    return SYS_OK;
}
#ifdef TEST_DNA_API_WIFI
#include "dna_wlan.h"
static sys_err_t wifi_set_psk(unsigned char type, unsigned char *psk, unsigned char len)
{
	char buf[WIFI_CMD_LEN], tmp[33];
	memset(buf, 0, sizeof(buf));
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, psk, len<33?len:32);

	wifi_ctrl_iface(STATION_IF, "set_network 0 scan_ssid 1");

	if(type<2) 
	{
		wifi_ctrl_iface(STATION_IF, "set_network 0 key_mgmt NONE");//OPEN
		if(type==1) 
		{
			sprintf(buf, "set_network 0 wep_key0 \"%s\"", psk);//WEP
			wifi_ctrl_iface(STATION_IF, buf);
		}
	}
	else
	{
		wifi_ctrl_iface(STATION_IF, "set_network 0 key_mgmt WPA-PSK");
		if(type==2) //TKIP
		{
			wifi_ctrl_iface(STATION_IF, "set_network 0 pairwise TKIP");
			wifi_ctrl_iface(STATION_IF, "set_network 0 group TKIP");
		}
		else if(type==3) // CCMP
		{
			wifi_ctrl_iface(STATION_IF, "set_network 0 pairwise CCMP");
			wifi_ctrl_iface(STATION_IF, "set_network 0 group CCMP TKIP");
		}
		sprintf(buf, "set_network 0 psk \"%s\"", tmp); // AUTO
		wifi_ctrl_iface(STATION_IF, buf);
	}
	return SYS_OK;
}
sys_err_t wifi_start_station_test(dna_wifi_network_t *config)
{
    if (WIFI_MODE_STA != wifi_get_opmode() && WIFI_MODE_AP_STA != wifi_get_opmode()) {
        return SYS_ERR_WIFI_MODE;
    }
    
    if (STA_STATUS_STOP < wifi_get_status(STATION_IF)) {
        return SYS_ERR_WIFI_BUSY;
    }
    
    if (WIFI_MODE_STA == wifi_get_opmode()) {
        wifi_remove_config_all(SOFTAP_IF); // clear ap config.
    }

    wifi_remove_config_all(STATION_IF);
    wifi_add_config(STATION_IF);
    wifi_config_ssid(STATION_IF, (config->ssid));
	wifi_set_psk(config->sec_type, config->psk, config->psk_len);
    wifi_config_commit(STATION_IF);
    wifi_set_status(STATION_IF, STA_STATUS_START);

    return SYS_OK;
}
#endif

sys_err_t wifi_stop_station(void)
{
    wifi_remove_config_all(STATION_IF);
    wifi_set_status(STATION_IF, STA_STATUS_STOP);
    return 0;
}

sys_err_t wifi_set_password(int vif, char *password)
{
	char buf[WIFI_CMD_LEN];
	int password_len;
	memset(buf, 0, sizeof(buf));

	if(password && password[0])
	{
		password_len = strnlen(password, 64);
		if(password_len == 5)
		{
			wifi_ctrl_iface(vif, "set_network 0 key_mgmt NONE");
			sprintf(buf, "set_network 0 wep_key0 \"%s\"", password);
		}
		else
		{
			if(password_len == 64)
				sprintf(buf, "set_network 0 psk %.*s", password_len, password);
			else	
				sprintf(buf, "set_network 0 psk \"%s\"", password);
		}
	}
	else
        sprintf(buf, "set_network 0 key_mgmt NONE");
	return wifi_ctrl_iface(vif, buf);
}

sys_err_t wifi_set_bssid(uint8_t *bssid)
{
    char buf[WIFI_CMD_LEN] = {0};

    sprintf(buf, "set_network 0 bssid %02x:%02x:%02x:%02x:%02x:%02x",
        bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
    return wifi_ctrl_iface(STATION_IF, buf);
}

sys_err_t wifi_set_scan_hidden_ssid(void)
{
	char buf[WIFI_CMD_LEN];
    
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "set_network 0 scan_ssid 1");
	return wifi_ctrl_iface(STATION_IF, buf);	
}

sys_err_t wifi_disconnect(void)
{
	char buf[WIFI_CMD_LEN];
    
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "disable_network 0");
	return wifi_ctrl_iface(STATION_IF, buf);
}

/*******************************************************************************
 * Function: wifi_ctrl_iface
 * Description:  send cmd to wpa supplicant
 * Parameters: 
 *   Input: on_off: 0 -- shut down RF, 1 -- open RF.
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
sys_err_t wifi_radio_set(unsigned char on_off)
{

    if(WIFI_WLAN_RADIO_OFF == on_off)
    {
        nrf_power_off();
    }
    else if(WIFI_WLAN_RADIO_ON == on_off)
    {
        nrf_power_on();
    }
    else
    {
        system_printf("\n para is invalid on_off=%d\n",on_off);
        return SYS_ERR;
    }
    return SYS_OK;
}

sys_err_t wifi_chip_powerup(void)
{
    return SYS_OK;
}

sys_err_t wifi_chip_powerdown(void)
{
    return SYS_OK;
}

sync_scan_db_t *get_snyc_scan_db(void)
{
    static sync_scan_db_t sync_scan_db = {0};
    static int8_t once = 0;

    if (!once) {
        once = 1;
        sync_scan_db.sem = xSemaphoreCreateBinary();
    }
    
    return &sync_scan_db;
}

sys_err_t wifi_event_scan_handler(int vif)
{
    sync_scan_db_t *db = get_snyc_scan_db();

    if (!db->sem)
        return SYS_OK;

    if (db->waiting)
        xSemaphoreGive(db->sem);

    return SYS_OK;
}

sys_err_t wifi_wait_scan_done(unsigned int timeoutms)
{
    BaseType_t ret;
    sync_scan_db_t *db = get_snyc_scan_db();

    if (!db->sem)
        return SYS_ERR;

    db->waiting = 1;
    ret = xSemaphoreTake(db->sem, pdMS_TO_TICKS(timeoutms));
    db->waiting = 0;
    
    if (pdTRUE == ret) {
        return SYS_OK;
    }

    return SYS_ERR;
}

sys_err_t wifi_scan_start(bool block, const wifi_scan_config_t *config)
{
    char buf[WIFI_CMD_LEN] = {0}, *pos, *end;
	unsigned int timeout;
    unsigned int scan_time;

    pos = buf;
    end = buf + WIFI_CMD_LEN;

    sprintf(buf, "flush_bss 0");
    wifi_ctrl_iface(STATION_IF, buf);
    memset(buf, 0, sizeof(buf));

    pos += snprintf(pos, end - pos - 1, "%s ", "scan");
    if (config) {
        if (config->max_item) {
            pos += snprintf(pos, end - pos - 1, "max=%d ", config->max_item);
            if (*(pos - 1) == '\0') --pos; // when format has %d, this toolchain's snprintf will add '\0', we should delete it. 
        }
        if (config->passive) {
            pos += snprintf(pos, end - pos - 1, "passive=%d ", config->passive ? 1 : 0);
            if (*(pos - 1) == '\0') --pos;
        }
        if (config->scan_time) {
            pos += snprintf(pos, end - pos - 1, "scan_time=%ld ", config->scan_time);
            if (*(pos - 1) == '\0') --pos;
        }
        if (config->channel > 0 && config->channel <= 13) {
            pos += snprintf(pos, end - pos - 1, "freq=%ld ", 
                system_modem_api_channel_to_mac80211_frequency(config->channel));
            if (*(pos - 1) == '\0') --pos;
        }
        if (config->bssid) {
            pos += snprintf(pos, end - pos - 1, "bssid="MAC_STR" ", MAC_VALUE(config->bssid));
        }

        // must be the last one.
        if (config->ssid && config->ssid[0]) {
            pos += snprintf(pos, end - pos - 1, "scan_ssid=\"%s\"", config->ssid);
        }
    }

    SYS_LOGI("--%s", buf);

    wifi_ctrl_iface(STATION_IF, buf);
    
    if (!block) {
        return SYS_OK;
    }

	if(config)
	{
		scan_time = config->scan_time ? config->scan_time : 100;
	
		if (config->channel > 0 && config->channel <= 14)
		{
			timeout = scan_time + SCAN_DELAY_TIMEMS;
		}
		else
		{
			timeout = scan_time * 14 + SCAN_DELAY_TIMEMS;
		}
	}
	else
	{
		timeout = 100 * 14 + SCAN_DELAY_TIMEMS;
	}

    
    return wifi_wait_scan_done(timeout);
}

sys_err_t wifi_get_scan_result(unsigned int index, wifi_info_t *info)
{
	return wpa_get_scan_result(index, info);
}


sys_err_t wifi_get_wifi_info(wifi_info_t *info)
{
	return wpa_get_wifi_info(info);
}

sys_err_t wifi_set_mac_addr(int vif, uint8_t *mac)
{
    struct netif *nif;

    if (!IS_VALID_VIF(vif))
        return SYS_ERR_INVALID_ARG;
    
	nif = get_netif_by_index(vif);
	at_update_netif_mac(vif, mac);
	wpa_update_mac_addr(vif, mac);
    
	return SYS_OK;
}

sys_err_t wifi_get_mac_addr(wifi_interface_e vif, unsigned char *mac)
{
    struct netif *nif = NULL;
    
	if(!mac)
		return SYS_ERR_INVALID_ARG;

    nif = get_netif_by_index(vif);
	memcpy(mac, nif->hwaddr, 6);
    
	return SYS_OK;
}

static uint8_t hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static void delay(volatile unsigned int data)
{
	volatile unsigned int indx;

	for (indx = 0; indx < data; indx++) {

	};
}


static uint8_t read_mac_addr(wifi_read_mac_mode_e mode, uint8_t *mac)
{
	int i,ret = 0;
	char tmp_mac[18] = {0};
	//char tmp_mac2[18] = {0};
	//char tmp_mac3[18] = {0};
	//uint8_t tmp_mac1[6] = {0};

	switch (mode) {
		case USER_NV:
			i = ef_get_env_blob(NV_WIFI_STA_MAC, tmp_mac, 18, NULL);
			if (!i || (hex2num(tmp_mac[1]) & 0x1)) {
				//system_printf("user_nv read mac err!\n");
				ret = -1;
			} else {
				for (i=0; i<6; i++)
					mac[i] = hex2num(tmp_mac[i*3]) * 0x10 + hex2num(tmp_mac[i*3+1]);
				ret = 0;
			}
			break;
		case AMT_NV:
			i = amt_nv_read(AMT_NV_MAC, (unsigned char *)tmp_mac, 18);
			if ((0 != i) || (hex2num(tmp_mac[1]) & 0x1)) {
				//system_printf("amt_nv read mac err!\n");
				ret = -1;
			} else {
				for (i=0; i<6; i++)
					mac[i] = hex2num(tmp_mac[i*3]) * 0x10 + hex2num(tmp_mac[i*3+1]);
				ret = 0;
			}
			break;
		case EFUSE:
			ret = -1;
			break;
		case OTP:
			spiFlash_OTP_Read(MAC_OTP_ADDR_GD25Q80E_FLASH, 6, (unsigned char *)mac);
			if (mac[0] & 0x1) {
				//system_printf("otp read mac err!\n");
				ret = -1;
			} else {				
				ret = 0;
			}
			break;
		case DEFAULT_MAC:
#if defined(MAC_ADDR_STANDALONE)
			memcpy(tmp_mac, (const char*) MAC_ADDR_STANDALONE, (size_t)18);
			for (i=0; i<6; i++)
				mac[i] = hex2num(tmp_mac[i*3]) * 0x10 + hex2num(tmp_mac[i*3+1]);
			ret = 0;
#endif
			break;
		default:
			ret = -1;
			break;
		}
	return ret;
}


sys_err_t wifi_load_mac_addr(int vif_id, uint8_t *mac)
{
	uint8_t chip_type = hal_efuse_read(CHIP_TYPE_ADDR_EFUSE) & 0xF;
	//system_printf("chip_type:%d\n", chip_type);

	if(read_mac_valid == read_mac_addr(USER_NV, mac)) {
		//system_printf("get mac from user_nv\n");
		goto mac;
	} else if(read_mac_valid == read_mac_addr(AMT_NV, mac)) {
		//system_printf("get mac from amt_nv\n");
		goto mac;
	} else if((chip_type == 0x3) && (read_mac_valid == read_mac_addr(EFUSE, mac))) {
		//system_printf("get mac from efuse\n");
		goto mac;
	} else if(read_mac_valid == read_mac_addr(OTP, mac)) {
		//system_printf("get mac from otp\n");
		goto mac;
	} else if(read_mac_valid == read_mac_addr(DEFAULT_MAC, mac)) {
		//system_printf("default mac\n");
		goto mac;
	} else {
		system_printf("read mac addr error!\n");
		return SYS_ERR;
	}
	

mac:
	if((mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]) == 0)
	{
		read_mac_addr(DEFAULT_MAC, mac);
		return SYS_ERR;
	}
	if (WIFI_MODE_AP == wifi_get_opmode()) { // ap only mode.
		mac[5] += !vif_id;
	} else {
		mac[5] += vif_id;
	}
	//system_printf("mac[]:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return SYS_OK;
}


sys_err_t wifi_get_ip_addr(wifi_interface_e vif, unsigned int *ip)
{
    struct netif *nif = NULL;

	if(!ip)
		return SYS_ERR_INVALID_ARG;

    nif = get_netif_by_index(vif);
	*ip = nif->ip_addr.addr;
    
	return SYS_OK;
}

sys_err_t wifi_get_mask_addr(wifi_interface_e vif, unsigned int *mask)
{
    struct netif *nif = NULL;
    
	if(!mask)
		return SYS_ERR_INVALID_ARG;

    nif = get_netif_by_index(vif);
	*mask = nif->netmask.addr;
    
	return SYS_OK;
}

sys_err_t wifi_get_gw_addr(wifi_interface_e vif, unsigned int *gw)
{
    struct netif *nif = NULL;
        
	if(!gw)
		return SYS_ERR_INVALID_ARG;
    
    nif = get_netif_by_index(STATION_IF);
	*gw = nif->gw.addr;
    
	return SYS_OK;
}

sys_err_t wifi_get_dns_addr(unsigned int index, unsigned int *dns)
{
	if((index > DNS_MAX_SERVERS) || (!dns))
		return SYS_ERR_INVALID_ARG;
	
	*dns = dns_getserver(index)->addr;
	return SYS_OK;
}

sys_err_t wifi_load_nv_cfg(void)
{
    uint8_t opmode;
    char    mode_str[2] = {0};

    wifi_load_nv_info(NULL); //load wifi connect info.

    //get wifi op mode, default value is already sta+ap
    if (1 == ef_get_env_blob(NV_WIFI_OP_MODE, mode_str, 1, NULL)) {
        opmode = atoi(mode_str);
#ifndef TUYA_SDK_ADPT
        if (WIFI_MODE_AP_STA == opmode || WIFI_MODE_AP == opmode)
#else
        if (WIFI_MODE_STA == opmode || WIFI_MODE_AP == opmode)
#endif
            network_db.mode = opmode;
    }

    return SYS_OK;
}

sys_err_t wifi_load_nv_info(wifi_nv_info_t *nv_info)
{
    if (!nv_info)
        nv_info = &wifi_nv_info;
    
    memset(nv_info, 0, sizeof(*nv_info));
    if (0 >= ef_get_env_blob(NV_WIFI_STA_SSID, nv_info->sta_ssid, sizeof(nv_info->sta_ssid) - 1, NULL)) {
        return SYS_ERR;
    }
    nv_info->sync = 1;
    ef_get_env_blob(NV_WIFI_STA_PWD, nv_info->sta_pwd, sizeof(nv_info->sta_pwd) - 1, NULL);
    ef_get_env_blob(NV_WIFI_STA_BSSID, nv_info->sta_bssid, sizeof(nv_info->sta_bssid), NULL);
    ef_get_env_blob(NV_WIFI_STA_CHANNEL, &nv_info->channel, sizeof(nv_info->channel), NULL);
    ef_get_env_blob(NV_WIFI_STA_PMK, nv_info->sta_pmk, sizeof(nv_info->sta_pmk), NULL);
    ef_get_env_blob(NV_WIFI_AUTO_CONN, &nv_info->auto_conn, sizeof(nv_info->auto_conn), NULL);

    SYS_LOGE("wifi nv:ssid[%s],pwd[%s],bssid[%02x:%02x:%02x:%02x:%02x:%02x],channel[%d],pmk[%02x%02x%02x]",
        nv_info->sta_ssid, nv_info->sta_pwd, nv_info->sta_bssid[0], nv_info->sta_bssid[1], nv_info->sta_bssid[2], 
        nv_info->sta_bssid[3], nv_info->sta_bssid[4], nv_info->sta_bssid[5], nv_info->channel, nv_info->sta_pmk[0], 
        nv_info->sta_pmk[1], nv_info->sta_pmk[2]);
    return SYS_OK;
}

sys_err_t wifi_save_nv_info(wifi_nv_info_t *nv_info)
{
    int ret = 0;

    if (!nv_info)
        return SYS_ERR_INVALID_ARG;
    if (!nv_info->sta_ssid[0])
       return SYS_OK; 

    ret |= ef_set_env_blob(NV_WIFI_STA_SSID, nv_info->sta_ssid, strlen(nv_info->sta_ssid));
    ret |= ef_set_env_blob(NV_WIFI_STA_PWD, nv_info->sta_pwd, strlen(nv_info->sta_pwd));
    ret |= ef_set_env_blob(NV_WIFI_STA_BSSID, nv_info->sta_bssid, sizeof(nv_info->sta_bssid));
    ret |= ef_set_env_blob(NV_WIFI_STA_CHANNEL, &nv_info->channel, sizeof(nv_info->channel));
    ret |= ef_set_env_blob(NV_WIFI_STA_PMK, nv_info->sta_pmk, sizeof(nv_info->sta_pmk));

    if (ret) { //roll back.
        ef_del_env(NV_WIFI_STA_SSID);
        ef_del_env(NV_WIFI_STA_PWD);
        ef_del_env(NV_WIFI_STA_BSSID);
        ef_del_env(NV_WIFI_STA_CHANNEL);
        ef_del_env(NV_WIFI_STA_PMK);
        return SYS_ERR;
    }

    nv_info->sync = 1;
    
    return SYS_OK;
}

sys_err_t wifi_clean_nv_info(void){
	ef_del_env(NV_WIFI_STA_SSID);
    ef_del_env(NV_WIFI_STA_PWD);
    ef_del_env(NV_WIFI_STA_BSSID);
    ef_del_env(NV_WIFI_STA_CHANNEL);
    ef_del_env(NV_WIFI_STA_PMK);
    
    return SYS_OK;
}


sys_err_t wifi_set_sta_by_nv(wifi_nv_info_t *nv_info)
{
    wifi_config_u sta_config;
    
    memset(&sta_config, 0, sizeof(sta_config));

    if (!nv_info)
        nv_info = &wifi_nv_info;
    if (!nv_info->sta_ssid[0])
        return SYS_ERR;

    memcpy(sta_config.sta.ssid, nv_info->sta_ssid, strlen(nv_info->sta_ssid));
    memcpy(sta_config.sta.bssid, nv_info->sta_bssid, sizeof(nv_info->sta_bssid));
    sta_config.sta.channel = nv_info->channel;
    memcpy(sta_config.sta.password, nv_info->sta_pwd, strlen(nv_info->sta_pwd));

    return wifi_start_station(&sta_config);
}

#if 0
static int set_nv_wifi(cmd_tbl_t *t, int argc, char *argv[])
{
    wifi_set_sta_by_nv(&wifi_nv_info);
    return CMD_RET_SUCCESS;
}
SUBCMD(set, nv_wifi, set_nv_wifi,  "", "");
#endif

void wifi_update_nv_sta_start(char *ssid, char *pwd, uint8_t *pmk)
{
    wifi_nv_info_t *nv_info = &wifi_nv_info;

    if (!ssid)
        return;
    
    if (strcmp(nv_info->sta_ssid, ssid)) {
        nv_info->sync = 0;
        strlcpy(nv_info->sta_ssid, ssid, sizeof(nv_info->sta_ssid));
    }

    if (!pwd && strlen(nv_info->sta_pwd) != 0) {
        nv_info->sync = 0;
        memset(nv_info->sta_pwd, 0, sizeof(nv_info->sta_pwd));
    } else if (pwd && strcmp(nv_info->sta_pwd, pwd)) {
        nv_info->sync = 0;
        strcpy(nv_info->sta_pwd, pwd);
    }

    if (!nv_info->sync) {
        if (pmk)
            memcpy(nv_info->sta_pmk, pmk, sizeof(nv_info->sta_pmk));
        else
            memset(nv_info->sta_pmk, 0, sizeof(nv_info->sta_pmk));
    }
}

void wifi_update_nv_sta_connected(char *ssid, uint8_t *bssid, uint8_t channel)
{
    wifi_nv_info_t *nv_info = &wifi_nv_info;

    if (strcmp(ssid, nv_info->sta_ssid)) {
        SYS_LOGE("update wifi nv, but ssid mismatch!!!");
        return;
    }

    if (!(IS_ZERO_MAC(bssid) || IS_MULTCAST_MAC(bssid))) {
        if (memcmp(bssid, nv_info->sta_bssid, sizeof(nv_info->sta_bssid))) {
            nv_info->sync = 0;
            memcpy(nv_info->sta_bssid, bssid, sizeof(nv_info->sta_bssid));
        }
    }

    if (nv_info->channel != channel) {
        nv_info->sync = 0;
        nv_info->channel = channel;
    }

    if (!nv_info->sync) {
        SYS_LOGE("save wifi nv.");
        if (wifi_save_nv_info(nv_info) != SYS_OK)
           SYS_LOGE("wifi save nv failed!!!");
    }
}

int wifi_use_nv_pmk(char *ssid, char *pwd, uint8_t *pmk)
{
    wifi_nv_info_t *nv_info = &wifi_nv_info;

    // if (!nv_info->sync)
    //     return 0;
    if (!ssid || strcmp(nv_info->sta_ssid, ssid))
        return 0;
    if (!pwd && strlen(nv_info->sta_pwd) != 0)
        return 0;
    if (pwd && strcmp(nv_info->sta_pwd, pwd))
        return 0;
    if(strlen((char *)nv_info->sta_pmk) == 0)
        return 0;
        
    memcpy(pmk, nv_info->sta_pmk, sizeof(nv_info->sta_pmk));
    
    return 1;
}

sys_err_t wifi_get_ap_rssi(char *rssi)
{
    int rst = SYS_OK;
    
    if (!rssi || (0 != wifi_drv_get_ap_rssi((void *)rssi))) {
        return SYS_ERR;
    }
	
	return SYS_OK;
}

/****************************************************************
*****************************************************************
****************************************************************/





/****************************************************************
*****************************************************************
****************************************************************/

sys_err_t wifi_set_ap_sta_num(const unsigned char num)
{   
    int rst = SYS_OK;
    drv_ctl_t *ctrl = NULL;

    if (0 == num) {
        system_printf("num 0\n");
        return SYS_ERR;
    }
    
    ctrl = os_malloc(sizeof(drv_ctl_t) + sizeof(char));
    if (NULL == ctrl) {
        system_printf("malloc drv_ctl_t fail\n");
        return SYS_ERR;
    }
    
    ctrl->cmd = SET_AP_STA_NUM;
    ctrl->vif = SOFTAP_IF;
    ctrl->len = 1;
    *((unsigned char *)ctrl->value) = num;

    if (0 != wifi_ctl_drv(ctrl, NULL)) {
        system_printf("wifi_ctl_drv proc fail\n");
        rst = SYS_ERR;
    }

    os_free(ctrl);
    
	return rst;
}

sys_err_t wifi_get_conn_ap_rssi(char *rssi)
{
    drv_ctl_t ctrl;
    int rst = SYS_OK;

    ctrl.cmd = GET_CONN_AP_RSSI;
    ctrl.vif = STATION_IF;
    ctrl.len = 0;
    
    if (!rssi || (0 != wifi_ctl_drv(&ctrl, (void *)rssi))) {
        return SYS_ERR;
    }
	
	return SYS_OK;
}

int wifi_set_country_code(const char *p_country_code)
{
    int rst = SYS_OK;
    drv_ctl_t *ctrl = os_malloc(sizeof(drv_ctl_t) + sizeof(char)*2);

    ctrl->cmd = SET_COUNTRY_CODE;
    ctrl->vif = STATION_IF;
    ctrl->len = 0;
	memcpy(ctrl->value,  p_country_code, 2);

    if (!p_country_code || (0 != wifi_ctl_drv(ctrl, NULL))) {
        rst =  SYS_ERR;
    }
    
	os_free(ctrl);
    
	return SYS_OK;
}

int wifi_get_country_code(char *p_country_code)
{
    drv_ctl_t ctrl;
    
    ctrl.cmd = GET_COUNTRY_CODE;
    ctrl.vif = STATION_IF;
    ctrl.len = 0;

    if(!p_country_code || (0 != wifi_ctl_drv(&ctrl, (char *)p_country_code)))
    {
        return SYS_ERR;
    }
	return SYS_OK;
}

int wifi_set_scan_method(wifi_scan_method_e method)
{
	extern bool scan_hit_multi;

	if(method == WIFI_ALL_CHANNEL_SCAN)
		scan_hit_multi = true;
	else if(method == WIFI_FAST_SCAN)
		scan_hit_multi = false;
	else 
		return SYS_ERR;

	return SYS_OK;
}

int wifi_get_scan_method(wifi_scan_method_e *method)
{
	extern bool scan_hit_multi;

	if(scan_hit_multi)
		*method = WIFI_ALL_CHANNEL_SCAN;
	else
		*method = WIFI_FAST_SCAN;

	return SYS_OK;
}



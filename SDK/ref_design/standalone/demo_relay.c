#ifdef ENABLE_LWIP_NAPT
#include "system.h"
#include "system_wifi.h"
#include "system_event.h"
#include "system_network.h"
#include "system_lwip.h"

static uint8_t relay_enable = 0;
static uint8_t channel_sta = 0, channel_ap = 0;
char softap_pwd[64] = {0};


static sys_err_t softap_start(int channel)
{
    int ret;
    ip_info_t ip_info;
    wifi_config_u config;
    uint8_t mac[6] = {0}, ip_part2, ip_part3;
    char ssid[33] = {0}, *pwd = softap_pwd;
    
    if (channel < 0 || channel > 14)
        return SYS_ERR_INVALID_ARG;

    wifi_get_mac_addr(SOFTAP_IF, mac);
    ip_part2 = mac[4];
    ip_part3 = mac[5];
    sprintf(ssid, "%s%02x%02x", "6260_ap_", mac[4], mac[5]);

    memset(&config, 0, sizeof(config));
    strlcpy((char *)config.ap.ssid, ssid, sizeof(config.ap.ssid));
    config.ap.channel = channel;
    if (strlen(pwd)) {
        strlcpy(config.ap.password, pwd, sizeof(config.ap.password));
        config.ap.authmode = AUTH_WPA_WPA2_PSK;
    } else {
        config.ap.authmode = AUTH_OPEN;
    }

    ret = wifi_start_softap(&config);
	if (SYS_OK != ret) {
        system_printf("start softap fialed. return %d\n", ret);
        return ret;
    }

    memset(&ip_info, 0, sizeof(ip_info));
	IP4_ADDR(&ip_info.ip, 10, ip_part2, ip_part3, 1);
	IP4_ADDR(&ip_info.gw, 10, ip_part2, ip_part3, 1);
	IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    
    set_softap_ipconfig(&ip_info);

    return SYS_OK;
}

static sys_err_t station_connect(char *ssid, const char *pwd)
{
    wifi_config_u sta_cfg;

    if (!ssid || ssid[0] == '\0')
        return SYS_ERR_INVALID_ARG;
    
    memset(&sta_cfg, 0, sizeof(sta_cfg));
    strlcpy((char *)sta_cfg.sta.ssid, ssid, sizeof(sta_cfg.sta.ssid));
    if (pwd) {
        strlcpy(sta_cfg.sta.password, pwd, sizeof(sta_cfg.sta.password));
    }

    return wifi_start_station(&sta_cfg);
}

static sys_err_t handle_wifi_event(void *ctx, system_event_t *event)
{
    int vif;

    vif = event->vif;
	switch (event->event_id) {
        case SYSTEM_EVENT_STA_CONNECTED:
        {
            if (channel_sta != event->event_info.connected.channel) {
                channel_sta = event->event_info.connected.channel;
            }
            break;
        }
        case SYSTEM_EVENT_STA_GOT_IP:
        {
            if (channel_sta != channel_ap) {
                system_printf("station channel changed, from %d to %d\n", channel_ap, channel_sta);
                channel_ap = channel_sta;
                wifi_stop_softap();
                softap_start(channel_ap);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return SYS_OK;
}

static void start_relay(char *ssid, char *sta_pwd, char *ap_pwd)
{
    sys_err_t ret;
    
    if (relay_enable)
        return;

    wifi_set_opmode(WIFI_MODE_AP_STA);
    enable_lwip_napt(SOFTAP_IF, 1);
    sys_event_loop_set_cb(handle_wifi_event, NULL);

    memset(softap_pwd, 0, sizeof(softap_pwd));
    if (ap_pwd) {
        strlcpy(softap_pwd, ap_pwd, sizeof(softap_pwd));
    }

    if (SYS_OK != (ret = station_connect(ssid, sta_pwd))) {
        system_printf("start sta connect failed. [err:%d]\n", ret);
        sys_event_loop_set_cb(NULL, NULL);
        return;
    }
    
    relay_enable = 1;

    return;
}

static void stop_relay(void)
{
    sys_event_loop_set_cb(NULL, NULL);
    enable_lwip_napt(SOFTAP_IF, 0);

    wifi_stop_station();
    wifi_stop_softap();

    channel_sta = channel_ap = 0;
    relay_enable = 0;
}

static void show_status(void)
{
    system_printf("relay is %s.\n", relay_enable ? "running" : "stoped");
    return;
}

static int do_relay(cmd_tbl_t *t, int argc, char *argv[])
{
    if (argc < 2) {
        return CMD_RET_USAGE;
    }

    if (0 == strcmp("start", argv[1])) {
        if (argc > 2)
            start_relay(argv[2], argc > 3 ? argv[3] : NULL, argc > 4 ? argv[4] : NULL);
    } else if (0 == strcmp("stop", argv[1])) {
        stop_relay();
    } else if (0 == strcmp("status", argv[1])) {
        show_status();
    }
    
    return CMD_RET_SUCCESS;
}
CMD(relay, do_relay, "start/stop relay", "relay [start/stop/status]");
#endif

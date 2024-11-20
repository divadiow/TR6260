#include "system_wifi.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "smartconfig.h"
#include "smartconfig_notify.h"
#include "cJSON.h"
#include "system_network.h"
#define DHCP_START_IP 0x9B0A0A0A
#define DHCP_END_IP 0xFD0A0A0A

void wifi_sta_connnect(const char *ssid, const char *pwd)
{
    int vif = 0;
    
    wifi_remove_config_all(vif);
    wifi_add_config(vif);

    wifi_config_ssid(STATION_IF, (unsigned char *)ssid);
	if(pwd && pwd[0] != '\0')
		wifi_set_password(STATION_IF, (char *)pwd);
	else
		wifi_set_password(STATION_IF, NULL);

    return;
}
static sc_result_t result;
void connect_wifi_task(void *param) 
{
    static int count = 0;
    wifi_sta_connnect(result.ssid, result.pwd);
    wifi_config_commit(0);
    // waiting until station got the IP
    unsigned int sta_ip;
    wifi_get_ip_addr(STATION_IF, &sta_ip);
    while(sta_ip == 0 && count <= 100){
        vTaskDelay((portTickType)(100 / portTICK_RATE_MS));
        wifi_get_ip_addr(STATION_IF, &sta_ip);
        count++;
    }
    if(sta_ip != 0){
        sc_notify_t msg = {0};
        msg.random = result.random;
        sc_notify_send(&msg);
    }

    vTaskDelete(NULL);
}


void sc_callback(smartconfig_status_t status, void *pdata)
{
    int lock_chn;
    if (status == SC_STATUS_LOCK_CHANNEL){
        memcpy(&lock_chn,pdata,sizeof(int));
        system_printf("target AP is in channel %d\n",lock_chn);
    }
    else if(status == SC_STATUS_GOT_SSID_PSWD) {
        system_printf("got ssid and password\n");
        memcpy(&result,pdata,sizeof(sc_result_t));
        system_printf("Result:\nssid_crc:[%x]\nkey_len:[%d]\nkey:[%s]\nrandom:[0x%02x]\nssid:[%s]", 
            result.reserved,
            result.pwd_length,
            result.pwd,
            result.random,
            result.ssid);
        smartconfig_stop();

    } else if(status == SC_STATUS_STOP){
        /* optional operation: connect to AP and send notify message to cellphone */

        xTaskCreate(connect_wifi_task, (const char *)"connect_wifi_task", 512, NULL, 5, NULL);
       
        /*  optional operation end */
    } else {
        system_printf("smartconfig status %d\n",status);
    }
}

static int sc_test_func(cmd_tbl_t *t, int argc, char *argv[])
{
    smartconfig_start(sc_callback);
	return CMD_RET_SUCCESS;
}
SUBCMD(test, sc_test, sc_test_func, "", "");



//AP CONFIGF
char softap_pwd[64] = {0};
static int ap_listen_sock;
#define MAX_LEN 4096

int ap_parse_data(char* buff,int n){

    system_printf("buff %s\n",buff);
    cJSON *request_root = NULL;
    cJSON *item_ssid = NULL;
    cJSON *item_pwd = NULL;
    request_root = cJSON_Parse(buff);
    if (request_root == NULL) {
        system_printf("JSON Parse Error\n");
        return -1;
    }

    item_ssid = cJSON_GetObjectItem(request_root, "ssid");
    if (item_ssid == NULL || !cJSON_IsString(item_ssid)) {
        cJSON_Delete(request_root);
        return -1;
    }

    item_pwd = cJSON_GetObjectItem(request_root, "pwd");
    if (item_pwd == NULL || !cJSON_IsString(item_pwd)) {
        cJSON_Delete(request_root);
        return -1;
    }
    system_printf("ssid %s pwd %s\n",item_ssid->valuestring,item_pwd->valuestring);
    if(strlen(item_ssid->valuestring) <= 32 && strlen(item_pwd->valuestring) <= 64){
        wifi_sta_connnect(item_ssid->valuestring,item_pwd->valuestring);
        wifi_config_commit(0);
    } else {
        cJSON_Delete(request_root);
        return -1;
    }
    cJSON_Delete(request_root); 
    return 0;
}


int tcp_server_start(void)
{
    struct sockaddr_in srcAddr;
    struct sockaddr_in clientAddr; 
    char addr_str[64];
    uint32_t addrLen = sizeof(clientAddr);
    int on = 1;
    srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_port = htons(8080);
    
    ap_listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (ap_listen_sock < 0) {
        system_printf("Unable to create socket: errno %d\n", errno);
        return -1;
    }
    int ret = setsockopt(ap_listen_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret) {
        system_printf("failed");
        close(ap_listen_sock);
        return -1;
    } else {
        system_printf("OK");
    }

    system_printf("Socket created fd:%d\n", ap_listen_sock);

    int err = bind(ap_listen_sock, (struct sockaddr *)&srcAddr, sizeof(srcAddr));
    if (err != 0) {
        system_printf("Socket unable to bind: errno %d\n", errno);
        close(ap_listen_sock);
        return -1;
    }
    system_printf("Socket binded\n");

    err = listen(ap_listen_sock, 5);
    if (err != 0) {
        system_printf("Error occured during listen: errno %d\n", errno);
        close(ap_listen_sock);
        return -1;
    }
    system_printf("Socket listening\n");
    while (true) {
        int sock = accept(ap_listen_sock, (struct sockaddr *)&clientAddr, &addrLen);
        if (sock < 0) {
            system_printf("Unable to accept connection: errno %d\n", errno);
            close(ap_listen_sock);
            return -1;
        }
        inet_ntoa_r(((struct sockaddr_in *)&clientAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str)-1);
        system_printf("Received connection from %s:%d\n", addr_str,((struct sockaddr_in *)&clientAddr)->sin_port);

        char *buff = (char *)os_zalloc(MAX_LEN);
        int n = recv(sock, buff, MAX_LEN, 0);
        buff[n] = '\0';
        system_printf("recv msg from client: %s\n", buff);
        if(n > 0){
            ret = ap_parse_data(buff,n);
            if(ret == 0){
                close(ap_listen_sock);
                close(sock);
                os_free(buff);
                return 0;
            }
        } 
        close(sock);
        os_free(buff);
        continue;
    }

    return 0;
}

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
    sprintf(ssid, "TRS-%02x%02x%02x", mac[3], mac[4], mac[5]);

    memset(&config, 0, sizeof(config));
    strlcpy((char *)config.ap.ssid, ssid, sizeof(config.ap.ssid));
    config.ap.channel = channel;
    if (strlen(pwd)) {
        strlcpy(config.ap.password, pwd, sizeof(config.ap.password));
        config.ap.authmode = AUTH_WPA_WPA2_PSK;
    } else {
        config.ap.authmode = AUTH_OPEN;
    }

    while (!wifi_is_ready()){
        system_printf("err! set AP/STA information must after wifi initialized!\n");
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    ret = wifi_start_softap(&config);
	if (SYS_OK != ret) {
        system_printf("start softap fialed. return %d\n", ret);
        return ret;
    }

    memset(&ip_info, 0, sizeof(ip_info));
	IP4_ADDR(&ip_info.ip, 10, 10, 10, 1);
	IP4_ADDR(&ip_info.gw, 10, 10, 10, 1);
	IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    set_softap_ipconfig(&ip_info);

    struct dhcps_lease dhcp_cfg_info;
    dhcp_cfg_info.enable = true;
    dhcp_cfg_info.start_ip.addr = DHCP_START_IP;
    dhcp_cfg_info.end_ip.addr = DHCP_END_IP;

    extern bool wifi_softap_set_dhcps_lease(struct dhcps_lease *lease_cfg);
    wifi_softap_set_dhcps_lease(&dhcp_cfg_info);
    tcp_server_start();
    return SYS_OK;
}


void ap_config_start(void *param)
{
    softap_start(1);
    vTaskDelete(NULL);
}

static int ap_config_func(cmd_tbl_t *t, int argc, char *argv[])
{
    wifi_set_opmode(WIFI_MODE_AP_STA);

    xTaskCreate(ap_config_start, (const char *)"ap_config_start", 1024, NULL, 5, NULL);
    
	return CMD_RET_SUCCESS;
}
SUBCMD(test, ap_config, ap_config_func, "", "");
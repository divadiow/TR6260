#include "system.h"
#include "system_event.h"

#define  WIFI_CONNECT_BEST_TASK_STACK_SIZE     (2048)
#define  WIFI_SCAN_MAX_SSID_COUNT              (16)
#define  WIFI_SCAN_MAIN_CHANNEL_TIME           (200)
#define  WIFI_SCAN_DEFAULT_CHANNEL_TIME        (100)
#define  WIFI_SCAN_START_CHANNEL               (1)
#define  WIFI_CONNEET_EVENT_MAX_COUNT          (4)
#define  WIFI_CONNECT_MIN_RSSI                 (-100)

typedef enum wifi_connect_cmd{
	WIFI_CONNECT_START,
	WIFI_CONNECT_STOP,	
}wifi_connect_cmd_t;

typedef struct scan_cache{
	uint8_t  channel;
	int8_t	  rssi;	
	uint8_t   bssid[6];
}scan_cache_t;

typedef struct wifi_queue_msg{
    uint8_t cmd;          /* 命令类型 */
	wifi_config_u config;
}wifi_queue_msg_t;

static enum wpa_states gstate;
static const unsigned short gstate_timeout[WPA_COMPLETED + 1]={2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 5000};
static unsigned int gstate_timems;
static TaskHandle_t wifi_policy_task_handle = NULL;
static QueueHandle_t  wifi_policy_queue_handle = NULL; 
static bool assoc_reject = false;
static bool scan_mainchannel = true;//Zl20201130:判断是否需要扫描1 6 11 3个主要信道
static bool wifi_policy_start = false;


sys_err_t system_event_wpa_assoc_reject(system_event_t *event)
{
    int reason = event->event_info.disconnected.reason;
    system_printf("system_event_wpa_assoc_reject :%d\r\n", reason);
	assoc_reject = true;
	
    return SYS_OK;
}

sys_err_t system_event_wpa_state_default(system_event_t *event)
{
    enum wpa_states state = event->event_info.wpa_internal_state.state;
    system_printf("system_event_wpa_state_default :%d\r\n", state);

	if(gstate != state && gstate < state)
	{
		gstate = state;
		gstate_timems = IN32(0x9004c8) / 1000;
	}

	return SYS_OK;
}
int wifi_policy_get_scan_results(unsigned char *ssid, scan_cache_t *scan_list)
{
	wifi_scan_config_t scan_config = {0};
	wifi_info_t wifi_info = {0};
	uint8_t record_count = 0, scan_num = 0;
	int i = 0,j = 0, k= 0;
	sys_err_t ret;
	
	if(scan_mainchannel) //Zl20201124:扫描1 6 11信道
	{
		for(i = 0; i < 3;i++)
		{
			memset(&wifi_info, 0, sizeof(wifi_info));
			memset(&scan_config, 0, sizeof(scan_config));
			scan_config.ssid = ssid; 
			scan_config.scan_time = WIFI_SCAN_MAIN_CHANNEL_TIME;
			scan_config.channel = WIFI_SCAN_START_CHANNEL + 5*i;

			ret = wifi_scan_start(1, &scan_config);
			if (ret != SYS_OK){
				system_printf("wifi_scan_start return err[%d]\n", ret);
				continue;
			}

			scan_num = wpa_get_scan_num();
			system_printf("scan ap_num: %d\n", scan_num);

			if(0 == scan_num)
				continue;

			if(WIFI_SCAN_MAX_SSID_COUNT == scan_num)
				break;

			for(j = 0;j < scan_num; j++)
			{
				wpa_get_scan_result(j, &wifi_info);
				system_printf("main_scan ssid:%s; bssid:%02x:%02x:%02x:%02x:%02x:%02x; channel:%d; rssi:%d \n", wifi_info.ssid, 
					wifi_info.bssid[0], wifi_info.bssid[1], wifi_info.bssid[2], wifi_info.bssid[3], wifi_info.bssid[4], 
					wifi_info.bssid[5], wifi_info.channel, wifi_info.rssi);

				if (strcmp((const char *)wifi_info.ssid, (const char *)ssid) == 0){
					system_printf("record_count %d\n",record_count);
					for(k = 0; k < record_count; k++)
					{
						if(memcmp(scan_list[k].bssid, wifi_info.bssid, 6) == 0)
							break;
					}
					if(k == record_count)
					{
						memcpy(scan_list[record_count].bssid, wifi_info.bssid, 6);
						scan_list[record_count].channel = wifi_info.channel;
						scan_list[record_count].rssi = wifi_info.rssi;
						record_count++;	
					}				
				}

				if(WIFI_SCAN_MAX_SSID_COUNT == record_count)//Zl20201130:若同名ssid超过16个，则丢弃剩余bssid
					break;
			}
		}
	}

	
	if(0 == record_count)
	{		//Zl20201124:若扫描1 6 11都没扫到ssid，则屏蔽扫描1 6 11信道。并且做全信道扫描。
		system_printf("enter 0 == allchannel scan!\r\n");
		scan_mainchannel = false;
		
		memset(&wifi_info, 0, sizeof(wifi_info));
		memset(&scan_config, 0, sizeof(wifi_scan_config_t));
		scan_config.ssid = ssid; 
		scan_config.scan_time = WIFI_SCAN_DEFAULT_CHANNEL_TIME;

		sys_err_t ret = wifi_scan_start(1, &scan_config);
		if (ret != SYS_OK){
			system_printf("wifi_scan_start return err[%d]\n", ret);
			return false;
		}
		
		scan_num = wpa_get_scan_num();
		system_printf("scan ap_num: %d\n", scan_num);

		if(0 == scan_num)
			return 0;

		for(i = 0;i < scan_num;i++)
		{
			wpa_get_scan_result(i, &wifi_info);
			system_printf("all_scan ssid:%s; bssid:%02x:%02x:%02x:%02x:%02x:%02x; channel:%d; rssi:%d \n", wifi_info.ssid, 
				wifi_info.bssid[0], wifi_info.bssid[1], wifi_info.bssid[2], wifi_info.bssid[3], wifi_info.bssid[4], 
				wifi_info.bssid[5], wifi_info.channel, wifi_info.rssi);

			if (strcmp((const char *)wifi_info.ssid, (const char *)ssid) == 0){
				for(k = 0; k < record_count; k++)
				{
					if(memcmp(scan_list[k].bssid, wifi_info.bssid, 6) == 0)
						break;
				}
				if(k == record_count)
				{
					memcpy(scan_list[record_count].bssid, wifi_info.bssid, 6);
					scan_list[record_count].channel = wifi_info.channel;
					scan_list[record_count].rssi = wifi_info.rssi;
					record_count++;	
				}					
			}

			if(((1 == wifi_info.channel) || (6 == wifi_info.channel) ||(11 == wifi_info.channel)) && (wifi_info.rssi > WIFI_CONNECT_MIN_RSSI))
				scan_mainchannel = true;//Zl20201130:若有扫描到1或6或11有信号强度>INT8_MIN_RSSI的信号，则下次重新进入扫描1 6 11  

			if(WIFI_SCAN_MAX_SSID_COUNT == record_count)
				break;
		}		
	}

	return record_count;
}

int  wifi_policy_get_sort_list(uint8_t *ssid, scan_cache_t *scan_list)
{
	scan_cache_t scan_list_buff1[WIFI_SCAN_MAX_SSID_COUNT]={0};
	scan_cache_t scan_list_buff2[WIFI_SCAN_MAX_SSID_COUNT]={0};
	scan_cache_t temp;
	int ap_num1, ap_num2, wifi_bssid_out_ap_num = 0;
	int i,j,k;
	
	ap_num1 = wifi_policy_get_scan_results(ssid, scan_list_buff1);
	ap_num2 = wifi_policy_get_scan_results(ssid, scan_list_buff2);

	if(0 == ap_num1){//Zl20201130:仅第一次扫描不到ssid，将第二次扫描结果复制到wifi_bssid_out
		system_printf("0 == ap_num1!\n");
		memcpy(scan_list, scan_list_buff1, ap_num1 * sizeof(scan_cache_t)); 
		wifi_bssid_out_ap_num = ap_num1;
	}
	if(0 == ap_num2){//Zl20201130:仅第二次扫描不到ssid，将第一次扫描结果复制到wifi_bssid_out
		system_printf("0 == ap_num2!\n");
		memcpy(scan_list, scan_list_buff2, ap_num2 * sizeof(scan_cache_t)); 
		wifi_bssid_out_ap_num = ap_num2;
	}
	if(0 != ap_num1 && 0 != ap_num2){
		for (i = 0; i < ap_num1; i++) {//Zl20201124:两次扫描rssi结果取平均
			for(j = 0; j < ap_num2; j++){
				if(memcmp(scan_list_buff1[i].bssid, scan_list_buff2[j].bssid, 6) == 0){
					memcpy(scan_list[wifi_bssid_out_ap_num].bssid, scan_list_buff2[j].bssid, 6); 
					scan_list[wifi_bssid_out_ap_num].rssi = (scan_list_buff1[i].rssi + scan_list_buff2[j].rssi)/2;
					scan_list[wifi_bssid_out_ap_num].channel = scan_list_buff2[j].channel;
					wifi_bssid_out_ap_num ++;
				}
			}
		}

		for (i = 0; i < ap_num1; i++) {//Zl20201124:将Zlwifi_ap_num1中只扫到1次的ssid，+（-100）后取平均
			for(j = 0; j < wifi_bssid_out_ap_num; j++){
				if(memcmp(scan_list_buff1[i].bssid, scan_list[j].bssid, 6) ==0){
					break;
				}
			}

			if(j == wifi_bssid_out_ap_num){
				memcpy(scan_list[wifi_bssid_out_ap_num].bssid, scan_list_buff1[i].bssid, 6); 
				scan_list[wifi_bssid_out_ap_num].rssi = (scan_list_buff1[i].rssi + WIFI_CONNECT_MIN_RSSI)/2;
				scan_list[wifi_bssid_out_ap_num].channel = scan_list_buff1[j].channel;
				wifi_bssid_out_ap_num ++;
			}
		}

		for (i = 0; i < ap_num2; i++) {//Zl20201124:将Zlwifi_ap_num2中只扫到1次的ssid，+（-100）后取平均
			for(j = 0; j < wifi_bssid_out_ap_num; j++){
				if(memcmp(scan_list_buff2[i].bssid, scan_list[j].bssid, 6) ==0){
					break;
				}
			}

			if(j == wifi_bssid_out_ap_num){
				memcpy(scan_list[wifi_bssid_out_ap_num].bssid, scan_list_buff2[i].bssid, 6); 
				scan_list[wifi_bssid_out_ap_num].rssi = (scan_list_buff2[i].rssi + WIFI_CONNECT_MIN_RSSI)/2;
				scan_list[wifi_bssid_out_ap_num].channel = scan_list_buff2[j].channel;
				wifi_bssid_out_ap_num ++;
			}
		}
	}

	for(i = 0; i < (wifi_bssid_out_ap_num - 1); i++){//Zl20201124:信号强度降序排序
		for(j = i+1; j < wifi_bssid_out_ap_num; j++){
			if(scan_list[i].rssi < scan_list[j].rssi){
				memcpy(&temp, &scan_list[i], sizeof(scan_cache_t));
				memcpy(&scan_list[i], &scan_list[j], sizeof(scan_cache_t));
				memcpy(&scan_list[j], &temp, sizeof(scan_cache_t));
			}
		}
	}
	
	system_printf("************************scan results,scan_list %d************************\n",
					wifi_bssid_out_ap_num);
	
	for(i = 0;i < wifi_bssid_out_ap_num;i++){//Zl20201124:打印信号强度排序后结果
		 if (scan_list[i].rssi != WIFI_CONNECT_MIN_RSSI){
		 	system_printf("scan_list num%d ssid:%s; bssid:%02x:%02x:%02x:%02x:%02x:%02x; channel:%d; rssi:%d \n",
				i , ssid, scan_list[i].bssid[0], scan_list[i].bssid[1], scan_list[i].bssid[2],
				scan_list[i].bssid[3], scan_list[i].bssid[4], scan_list[i].bssid[5],
				scan_list[i].channel, scan_list[i].rssi);
		}	  
	}

	return wifi_bssid_out_ap_num;
}
int  wifi_policy_select_one_connect(wifi_config_u *config)
{
	int result;
	unsigned  int cur_timems;
	struct ip_info if_ip;
	
	gstate_timems = IN32(0x9004c8) / 1000;
	gstate = 3;

    wifi_ctrl_iface(STATION_IF, "flush_bss 0");
	
	wifi_start_station(config);
	
	while(1)
	{
		
		if(!wifi_policy_start)
			return 0;
		
		cur_timems = IN32(0x9004c8) / 1000;	
		if(gstate_timems != 0 && cur_timems > gstate_timems && cur_timems - gstate_timems >= gstate_timeout[gstate])
		{
			system_printf("func:%s line:%d cur_timems=%d =%d %d %d gstate=%d\r\n", __func__, __LINE__, cur_timems, gstate_timems, cur_timems - gstate_timems, gstate_timeout[gstate], gstate);
			gstate_timems = 0;
			wifi_stop_station();
			return -1;
		}

		if(assoc_reject)
		{
			system_printf("func:%s line:%d\r\n", __func__, __LINE__);
			gstate_timems = 0;
			assoc_reject = false;
			wifi_stop_station();
			return -1;
		}
		
		vTaskDelay(pdMS_TO_TICKS(50));

		if(STA_STATUS_CONNECTED != wifi_get_sta_status())
		{
			continue;
		}

		wifi_get_ip_info(STATION_IF,&if_ip);
		if(ip4_addr_isany_val(if_ip.ip)) //等待获取ip后再处理
		{
			continue;
		}

		system_printf("connect done!!!\r\n");
		break;
	}

	return 0;
}
int  wifi_policy_rssi_connect(wifi_config_u *config)
{
	bool retry = true;
	int result = -1, count, i;
	scan_cache_t scan_cache_list[WIFI_SCAN_MAX_SSID_COUNT];
	
	count = wifi_policy_get_sort_list(config->sta.ssid, scan_cache_list);
	if(count > 0)
	{	
		if(!wifi_policy_start)
			return -1;

		for(i = 0; i < count ; i++)
		{
			if(retry && i == 2){
				i = 0;
				retry = false;
			}		
				
			system_printf("start connect num%d ssid:%s; bssid:%02x:%02x:%02x:%02x:%02x:%02x; channel:%d; rssi:%d \n",
				i , config->sta.ssid, scan_cache_list[i].bssid[0], scan_cache_list[i].bssid[1], scan_cache_list[i].bssid[2],
				scan_cache_list[i].bssid[3], scan_cache_list[i].bssid[4], scan_cache_list[i].bssid[5],
				scan_cache_list[i].channel, scan_cache_list[i].rssi);
			
			memcpy(config->sta.bssid, scan_cache_list[i].bssid, 6);
			config->sta.channel = scan_cache_list[i].channel;

			result = wifi_policy_select_one_connect(config);
			if(result == 0)
				break;
		}
	}
	else
	{
		result = wifi_policy_select_one_connect(config);
	}

	return result;
}
void wifi_daemon_task(void *pvParameters)	//线程主函数
{	
	int result;
	wifi_queue_msg_t queue_msg={0};
	
	while(1)
	{	
		if(xQueueReceive(wifi_policy_queue_handle, &queue_msg, portMAX_DELAY) == pdPASS)
		{
			switch(queue_msg.cmd)
			{
				case WIFI_CONNECT_START:
					result = wifi_policy_rssi_connect(&queue_msg.config);
					if(result != 0 && wifi_policy_start)
					{
						xQueueSend(wifi_policy_queue_handle, &queue_msg, 50 / portTICK_PERIOD_MS);
					}
					break;
				case WIFI_CONNECT_STOP:
					wifi_stop_station();
					break;
				default:
					break;
			}
		}
	}
}

int wifi_policy_connect(wifi_config_u *config)
{
	int result;
	wifi_queue_msg_t queue_msg;

	if (!(IS_ZERO_MAC(config->sta.bssid) || IS_MULTCAST_MAC(config->sta.bssid)))
	{
		return wifi_start_station(config);
	}
	
	memset(&queue_msg, 0, sizeof(wifi_queue_msg_t));

	queue_msg.cmd = WIFI_CONNECT_START;
	memcpy(&queue_msg.config, config, sizeof(wifi_config_u));
	
	if(wifi_policy_task_handle == NULL)
	{
		sys_event_reset_wifi_handlers(SYSTEM_EVENT_STA_ASSOC_REJECT, system_event_wpa_assoc_reject);
		sys_event_reset_wifi_handlers(SYSTEM_EVENT_WPA_INTERNAL_STATE, system_event_wpa_state_default);
		wifi_set_scan_method(WIFI_ALL_CHANNEL_SCAN);
		
	    result = xTaskCreate(wifi_daemon_task, "wifi_daemon_task", WIFI_CONNECT_BEST_TASK_STACK_SIZE/sizeof(StackType_t), NULL, 2, &wifi_policy_task_handle);
	    if (result != pdPASS)
	    {
	        system_printf("wifi_daemon_task failed: %d\n", result);
	        return -SYS_ERR;
	    }

		wifi_policy_queue_handle = xQueueCreate(WIFI_CONNEET_EVENT_MAX_COUNT, sizeof(wifi_queue_msg_t));
		if(wifi_policy_queue_handle  == NULL)
		{
			vTaskDelete(wifi_policy_task_handle);
			wifi_policy_task_handle = NULL;
			return -SYS_ERR;
		}

	}

	wifi_policy_start = true;
	result = xQueueSend(wifi_policy_queue_handle, &queue_msg, 50 / portTICK_PERIOD_MS);
	if(result != pdPASS)
	{
		 system_printf("xQueueSend failed: %d\n", result);
		 return -SYS_ERR;
	}

	return SYS_OK;
}

int wifi_policy_disconnect(void)
{
	int result;
	wifi_queue_msg_t queue_msg;

	if(wifi_policy_queue_handle == NULL)
		return -SYS_ERR;
	
	wifi_policy_start = false;
	memset(&queue_msg, 0, sizeof(wifi_queue_msg_t));

	queue_msg.cmd = WIFI_CONNECT_STOP;
	result = xQueueSend(wifi_policy_queue_handle, &queue_msg, 50 / portTICK_PERIOD_MS);
	if(result != pdPASS)
	{
		 system_printf("xQueueSend failed: %d\n", result);
		 return -SYS_ERR;
	}

	return SYS_OK;
}

static int wifi_best_connect(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("wifi_best_connect\r\n");

	int i;
	wifi_config_u config;

	for(i = 0; i < argc; i++)
	{
		system_printf("[%d]: %s\r\n", i, argv[i]);
	}

	memset(&config, 0, sizeof(wifi_config_u));

	if(argc >= 2)
	{
		strcpy((char *)config.sta.ssid, argv[1]);
		system_printf("ssid:%s\r\n", config.sta.ssid);
	}
	if(argc >= 3)
	{
		strcpy(config.sta.password , argv[2]);
		system_printf("password:%s\r\n", config.sta.password);
	}
	
	wifi_policy_connect(&config);
	
	return CMD_RET_SUCCESS;
}


CMD(wifi_best_connect,
    wifi_best_connect,
    "wifi_best_connect",
    "wifi_best_connect");

static int wifi_best_disconnect(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("wifi_best_discon\r\n");

	wifi_policy_disconnect();
	return CMD_RET_SUCCESS;
}


CMD(wifi_best_disconnect,
    wifi_best_disconnect,
    "wifi_best_disconnect",
    "wifi_best_disconnect");



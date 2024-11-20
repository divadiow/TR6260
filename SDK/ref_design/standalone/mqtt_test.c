#include "system_wifi.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "system_queue.h"
/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define IS_ASCII(c) (c > 0x1F && c < 0x7F)
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

/****************************************************************************
* 	                                          Global Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/

#include "mqtt_client.h"
#include "platform_tr6260.h"
struct mqtt_client_item
{
	uint32_t client_id;
	trs_mqtt_client_handle_t client;
	STAILQ_ENTRY(mqtt_client_item) next;
};

typedef struct mqtt_client_list_t * mqtt_client_handle_t;
typedef struct mqtt_client_item * mqtt_client_item_t;

STAILQ_HEAD(mqtt_client_list_t, mqtt_client_item);
static mqtt_client_handle_t g_mqtt_head = NULL;
trs_mqtt_client_config_t mqtt_cfg = {0};
static int default_mqtt_handle(trs_mqtt_event_handle_t event)
{
    trs_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            system_printf("MQTT_EVENT_CONNECTED\n");

            break;
        case MQTT_EVENT_DISCONNECTED:
            system_printf("MQTT_EVENT_DISCONNECTED\n");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            system_printf("MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            system_printf("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            system_printf("MQTT_EVENT_PUBLISHED, msg_id=%d\n", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            system_printf("MQTT_EVENT_DATA\n");
            system_printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            system_printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            system_printf("MQTT_EVENT_ERROR\n");
            break;
    }
    return 0;
}

extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

int mqtt_start(cmd_tbl_t *t, int argc, char *argv[])
{
	uint8_t i;
    memset(&mqtt_cfg,0,sizeof(trs_mqtt_client_config_t));

    mqtt_cfg.event_handle = default_mqtt_handle;
#if 1
    mqtt_cfg.cert_pem = (const char *)server_root_cert_pem_start;
#endif
	for(i=1; i<argc; i+=2)
	{
		if(strcmp(argv[i], "uri")==0)
		{
            mqtt_cfg.uri = argv[i+1];
		}
		else if(strcmp(argv[i], "port")==0)
		{
			mqtt_cfg.port = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "client_id")==0)
		{
			mqtt_cfg.client_id = (const char*)argv[i+1];
		}
		else if(strcmp(argv[i], "username")==0)
		{
			mqtt_cfg.username = (const char*)argv[i+1];
		}
		else if(strcmp(argv[i], "password")==0)
		{
			mqtt_cfg.password = (const char*)argv[i+1];
		}
		else if(strcmp(argv[i], "clean_session")==0)
		{
			mqtt_cfg.disable_clean_session = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "keepalive")==0)
		{
			mqtt_cfg.keepalive = atoi(argv[i+1]);
		}
    }
	if(g_mqtt_head == NULL) {
		g_mqtt_head = (mqtt_client_handle_t)mqtt_calloc(1, sizeof(struct mqtt_client_list_t));
		STAILQ_INIT(g_mqtt_head);
	}
	trs_mqtt_client_handle_t t_client = trs_mqtt_client_init(&mqtt_cfg);
	if(t_client == NULL){
        system_printf("mqtt init failed\n");
		mqtt_free(g_mqtt_head);
		g_mqtt_head = NULL;
        return CMD_RET_FAILURE;
    }

	mqtt_client_item_t item =(mqtt_client_item_t)mqtt_calloc(1, sizeof(struct mqtt_client_item));
    item->client = t_client;
	item->client_id++;
	STAILQ_INSERT_TAIL(g_mqtt_head, item, next);
	
    if (0 != trs_mqtt_client_start(item->client)){
        system_printf("mqtt start failed\n");
		trs_mqtt_client_stop(item->client);
		trs_mqtt_client_destroy(item->client);
		STAILQ_REMOVE(g_mqtt_head, item, mqtt_client_item, next);
		mqtt_free(item);
        return CMD_RET_FAILURE;
    }
	system_printf("mqtt start item->client_id %d done\n",item->client_id);
    return CMD_RET_SUCCESS;

}
SUBCMD(test,mqtt_start,mqtt_start, "mqtt_test", "mqtt_test");

int mqtt_query(cmd_tbl_t *t, int argc, char *argv[])
{
    mqtt_client_item_t item = NULL;
    STAILQ_FOREACH(item, g_mqtt_head, next) {
		if(item->client_id == 0){
			break;
		}
		system_printf("client id %d url:%s status %d\n",item->client_id, trs_mqtt_client_get_url(item->client), trs_mqtt_client_get_states(item->client));
	}
	return CMD_RET_SUCCESS;
}

SUBCMD(test, mqtt_query, mqtt_query, "", "");

int mqtt_stop(cmd_tbl_t *t, int argc, char *argv[])
{
	mqtt_client_item_t item = NULL;
	uint32_t client_id = 0xFFFFFFFF;
	uint8_t i;
	for(i=1; i<argc; i+=2)
	{
		if(strcmp(argv[i], "id")==0)
		{
            client_id = atoi(argv[i+1]);
		}
	}
	if(client_id == 0xFFFFFFFF){
		system_printf("client id error\n");
		return CMD_RET_FAILURE;
	}
	STAILQ_FOREACH(item, g_mqtt_head, next) {
		if(item->client_id == 0){
			break;
		}
		if (item->client_id == client_id) {
			//trs_mqtt_client_stop(item->client);
			trs_mqtt_client_destroy(item->client);
			STAILQ_REMOVE(g_mqtt_head, item, mqtt_client_item, next);
			mqtt_free(item);
			break;
		}
	}
	return CMD_RET_SUCCESS;
}

SUBCMD(test, mqtt_stop, mqtt_stop, "", "");

int mqtt_sub(cmd_tbl_t *t, int argc, char *argv[])
{
    int i = 0;
    char * topic = NULL;
    int qos = 0;
	mqtt_client_item_t item;
	uint32_t client_id = 0xFFFFFFFF;
    for(i=1; i<argc; i+=2)
	{
		if(strcmp(argv[i], "id")==0)
		{
            client_id = atoi(argv[i+1]);
		}
		if(strcmp(argv[i], "topic")==0)
		{
            topic = argv[i+1];
		}
		else if(strcmp(argv[i], "qos")==0)
		{
			qos = atoi(argv[i+1]);
		}
    }
	if(client_id == 0xFFFFFFFF){
		system_printf("client id error\n");
		return CMD_RET_FAILURE;
	}
	STAILQ_FOREACH(item, g_mqtt_head, next) {
		if(item->client_id == 0){
			break;
		}
		if (item->client_id == client_id) {
			trs_mqtt_client_subscribe(item->client,topic,qos );
			break;
		}
	}
	if(item == NULL) {
		system_printf("not start mqtt\n");
        return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

SUBCMD(test, mqtt_sub, mqtt_sub, "", "");

int mqtt_unsub(cmd_tbl_t *t, int argc, char *argv[])
{
    int i = 0;
    char * topic = NULL;
    int qos = 0;
	mqtt_client_item_t item;
	uint32_t client_id = 0xFFFFFFFF;
    for(i=1; i<argc; i+=2)
	{
		if(strcmp(argv[i], "id")==0)
		{
            client_id = atoi(argv[i+1]);
		}
		if(strcmp(argv[i], "topic")==0)
		{
            topic = argv[i+1];
		}
    }
	if(client_id == 0xFFFFFFFF){
		system_printf("client id error\n");
		return CMD_RET_FAILURE;
	}
	STAILQ_FOREACH(item, g_mqtt_head, next) {
		if(item->client_id == 0){
			break;
		}
		if (item->client_id == client_id) {
			trs_mqtt_client_unsubscribe(item->client,topic);
			break;
		}
	}
	if(item == NULL) {
		system_printf("not start mqtt\n");
        return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

SUBCMD(test, mqtt_unsub, mqtt_unsub, "", "");


int mqtt_pub(cmd_tbl_t *t, int argc, char *argv[])
{
    char *topic = NULL;
    char *data = NULL;
    int qos = 0;
    int retain = 0;
    int i =0;
	mqtt_client_item_t item;
	uint32_t client_id = 0xFFFFFFFF;
    for(i=1; i<argc; i+=2)
	{
		if(strcmp(argv[i], "id")==0)
		{
            client_id = atoi(argv[i+1]);
		}
		if(strcmp(argv[i], "topic")==0)
		{
            topic = argv[i+1];
		}
		else if(strcmp(argv[i], "data")==0)
		{
			data = argv[i+1];
		}else if(strcmp(argv[i], "qos")==0)
		{
			qos = atoi(argv[i+1]);
		}else if(strcmp(argv[i], "retain")==0)
		{
			retain = atoi(argv[i+1]);
        }
    }
	STAILQ_FOREACH(item, g_mqtt_head, next) {
		if(item->client_id == 0){
			break;
		}
		if (item->client_id == client_id) {
			trs_mqtt_client_publish(item->client,topic,data,strlen(data),qos,retain);
			break;
		}
	}
    if(item == NULL) {
		system_printf("not start mqtt\n");
        return CMD_RET_FAILURE;
    }
	return CMD_RET_SUCCESS;
}

SUBCMD(test, mqtt_pub, mqtt_pub, "", "");



typedef struct {
	unsigned int ipaddr;
	unsigned int mask;
	unsigned int gateway;
	unsigned int dns1;
	unsigned int dns2;
	unsigned char ssid[32];
	unsigned char psk[32];
	unsigned char ssid_len;
	unsigned char psk_len;
	unsigned char sec_type;
	unsigned char channel;
	unsigned char bssid[6];
	unsigned char reserved[2];
}wifi_network_t;


int wlan_sta_start(wifi_network_t *network)
{
    int ret;
    ip_info_t ipInfoTmp;
    wifi_config_u config;

    memset(&config, 0, sizeof(config));
	memcpy(config.sta.ssid, network->ssid, MIN(network->ssid_len, sizeof(config.sta.ssid) - 1));
	if(network->psk_len>0 && network->psk[0])
		memcpy(config.sta.password, network->psk, MIN(sizeof(config.sta.password), network->psk_len));
    ret = wifi_start_station(&config);
	if (SYS_ERR_WIFI_MODE == ret)
        return -1;
    else if (SYS_ERR_WIFI_BUSY == ret)
        return -1;
	return 0;
}


static int test_mqtt_create_sta(cmd_tbl_t *t, int argc, char *argv[])
{
	wifi_network_t network;
	uint8_t i;

	if(argc < 3) return CMD_RET_FAILURE;

	uint8_t channel = 0;
	memset((char*)&network, 0, sizeof(network));

	for(i=1; i<argc; i+=2)
	{
		if(strcmp(argv[i], "type")==0)
		{
			network.sec_type = atoi(argv[i+1]);
			if(network.sec_type>5)
				return CMD_RET_FAILURE;
		}
		else if(strcmp(argv[i], "ch")==0)
		{
			network.channel = atoi(argv[i+1]);
			if(network.channel < 1 || network.channel > 14)
				return CMD_RET_FAILURE;
			channel = network.channel;
		}
		else if(strcmp(argv[i], "ssid")==0)
		{
			network.ssid_len = strlen(argv[i+1]);
			if(network.ssid_len < 33)
				memcpy(network.ssid, argv[i+1], network.ssid_len);
			else
				return CMD_RET_FAILURE;
		}
		else if(strcmp(argv[i], "psk")==0)
		{
			network.psk_len = strlen(argv[i+1]);
			if(network.psk_len < 33)
				memcpy(network.psk, argv[i+1], network.psk_len);
			else
				return CMD_RET_FAILURE;
		}
		else return CMD_RET_FAILURE;
	}

	if(network.ipaddr != 0)
	{
		if(network.gateway==0)
			network.gateway = (network.ipaddr&0x00FFFFFF)|0x01000000;
		if(network.mask==0)
			network.mask = 0x00FFFFFF; // 255.255.255.0
		if(network.dns1==0)
			network.dns1 = 0x72727272; // 114.114.114.114
		if(network.dns2==0)
			network.dns2 = 0x08080808; // 8.8.8.8
	}

	wlan_sta_start(&network);

	return CMD_RET_SUCCESS;
}
SUBCMD(test, create_sta, test_mqtt_create_sta, "", "");

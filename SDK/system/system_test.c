/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:    system_test.c
 * File Mark:    
 * Description:  only for system test.
 * Others:        
 * Version:       V1.0
 * Author:        lixiao
 * Date:          2018-02-12
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
#include "system_wifi.h"
#include "system_lwip.h"
#include "system_network.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "wifi_sniffer.h"
/****************************************************************************
* 	                                           Local Macros
****************************************************************************/

/****************************************************************************
* 	                                           Local Types
****************************************************************************/
extern sys_err_t wifi_get_ap_rssi(char *rssi);

struct cli_cmd {
    const char *name;
    int (*handler)(int argc, char *argv[]);
    const char *desc; /* Short description */
    const char *usage; /* usage */
};

#define CMDENTRY(cmd, fn, d, u) \
	{							\
		.name = #cmd, 			\
		.handler = fn, 			\
		.desc = d, 				\
		.usage = u,				\
	}
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
int set_wifi_opmode(int argc, char *argv[])
{
    uint8_t mode = WIFI_MODE_STA;
    char    mode_str[2];

    if (argc < 2)
        return CMD_RET_FAILURE;
    
    mode = atoi(argv[1]);
    if (mode > WIFI_MODE_AP_STA)
        mode = WIFI_MODE_AP_STA;

    wifi_set_opmode(mode);
    sprintf(mode_str, "%1d", mode);
    ef_set_env_blob(NV_WIFI_OP_MODE, mode_str, 1);

    return CMD_RET_SUCCESS;
}

int get_wifi_opmode(int argc, char *argv[])
{
    int mode = wifi_get_opmode();
    char *mode_str = NULL;

    switch(mode) {
        case WIFI_MODE_STA:
            mode_str = "station mode";
            break;
        case WIFI_MODE_AP:
            mode_str = "softap mode";
            break;
        case WIFI_MODE_AP_STA:
            mode_str = "station + softap mode";
            break;
        default:
            mode_str = "unknow mode";
            break;
        }

    SYS_LOGE("%s", mode_str);
    return CMD_RET_SUCCESS;
}

int set_wifi_channel(int argc, char *argv[])
{
    uint8_t channel;

    if (argc < 2)
        return CMD_RET_USAGE;
    
    channel = atoi(argv[1]);
    if (channel < 0 || channel > 13)
        return CMD_RET_FAILURE;

    if (wifi_rf_set_channel(channel))
        return CMD_RET_FAILURE;

    return CMD_RET_SUCCESS;
}
#ifdef ENABLE_LWIP_NAPT
extern void napt_debug_print(); 
static int set_napt(int argc, char *argv[])
{
    if (argc < 2)
        return CMD_RET_USAGE;

    if (0 == strcmp("enable", argv[1])) {
        enable_lwip_napt(SOFTAP_IF, 1);
    } else if (0 == strcmp("disable", argv[1])) {
        enable_lwip_napt(SOFTAP_IF, 0);
    } else if (0 == strcmp("show", argv[1])) {
        napt_debug_print();
    } else {
        return CMD_RET_USAGE;
    }
     
    return CMD_RET_SUCCESS;
}
#endif

int __attribute__((weak)) machtalk_run(int argc, char *argv[])
{
    return CMD_RET_SUCCESS;
}

// -i ip -m mask -g gw -d1 dns1 -d2 dns2
int set_ipinfo(int argc, char *argv[])
{
	int i = 0;
	char cmd[512] = {0};
    char *str = NULL, *buf = cmd;
    ip_info_t ipinfo;

	if (argc <= 1)
	    return CMD_RET_USAGE;

	sprintf(buf, " %s", argv[1]);
	for (i = 2; i < argc; i++) {
        sprintf(buf, "%s %s", buf, argv[i]);
	}

    memset(&ipinfo, 0, sizeof(ipinfo));
    for (;;) {
    	str = strtok_r(NULL, " ", &buf);
    	if(str == NULL || str[0] == '\0') {
    		break;
    	} else if(strcmp(str, "-i") == 0){
			str = strtok_r(NULL, " ", &buf);
			ip4addr_aton(str, &ipinfo.ip);
		} else if(strcmp(str, "-m") == 0){
			str = strtok_r(NULL, " ", &buf);
			ip4addr_aton(str, &ipinfo.netmask);
		} else if(strcmp(str, "-g") == 0){
			str = strtok_r(NULL, " ", &buf);
			ip4addr_aton(str, &ipinfo.gw);
		}else if(strcmp(str, "-d1") == 0){
			str = strtok_r(NULL, " ", &buf);
			ip4addr_aton(str, &ipinfo.dns1);
		}else if(strcmp(str, "-d2") == 0){
			str = strtok_r(NULL, " ", &buf);
			ip4addr_aton(str, &ipinfo.dns2);
		}
    }
    wifi_set_ip_info(STATION_IF, &ipinfo);
    return CMD_RET_SUCCESS;
}

static const struct cli_cmd set_cmd[] = {
	CMDENTRY(wifi_opmode, set_wifi_opmode, "", ""),
    CMDENTRY(channel, set_wifi_channel, "", ""),
    CMDENTRY(ipinfo, set_ipinfo, "", ""),
#ifdef ENABLE_LWIP_NAPT
    CMDENTRY(napt, set_napt, "", ""),
#endif
};

static const struct cli_cmd get_cmd[] = {
	CMDENTRY(wifi_opmode, get_wifi_opmode, "", ""),
};

static const struct cli_cmd run_cmd[] = {
	CMDENTRY(machtalk, machtalk_run, "", ""),
};

const struct cli_cmd *cli_find_cmd(char *cmd, const struct cli_cmd *table, int nr)
{
	const struct cli_cmd *t;

	for (t = table; t < table + nr; t++) {
		if (strcmp(cmd, t->name) == 0 &&
			strlen(t->name) == strlen(cmd))
			return t;
	}
	return NULL;
}

static int cmd_test_set(cmd_tbl_t *t, int argc , char *argv[])
{
	const struct cli_cmd *cmd;

	argc--;
	argv++;

	if (argc == 0)
		return CMD_RET_USAGE;

	cmd = cli_find_cmd(argv[0], set_cmd, ARRAY_SIZE(set_cmd));
	if (cmd == NULL)
		return CMD_RET_USAGE;

	return cmd->handler(argc, argv);
}
SUBCMD(test, set, cmd_test_set, "system test set", "test set {name value}");

static int cmd_test_get(cmd_tbl_t *t, int argc , char *argv[])
{
	const struct cli_cmd *cmd;

	argc--;
	argv++;

	if (argc == 0)
		return CMD_RET_USAGE;

	cmd = cli_find_cmd(argv[0], get_cmd, ARRAY_SIZE(get_cmd));
	if (cmd == NULL)
		return CMD_RET_USAGE;

	return cmd->handler(argc, argv);

}
SUBCMD(test, get, cmd_test_get, "system test get", "test get {name}");

static int cmd_test_run(cmd_tbl_t *t, int argc , char *argv[])
{
	const struct cli_cmd *cmd;

	argc--;
	argv++;

	if (argc == 0)
		return CMD_RET_USAGE;

	cmd = cli_find_cmd(argv[0], run_cmd, ARRAY_SIZE(run_cmd));
	if (cmd == NULL)
		return CMD_RET_USAGE;

	return cmd->handler(argc, argv);

}
SUBCMD(test, run, cmd_test_run, "system test run app", "test run {name value}");

#define FILTER_TYPE_NONE    0
#define FILTER_TYPE_SSID    1
#define FILTER_TYPE_MAC     2
#define FILTER_TYPE_LEN     3
#define FILTER_TYPE_MAC2    4
#define FILTER_TYPE_MAC2_UC 5 //unicast
#define FILTER_TYPE_MAC2_MC 6 //multicast
#define FILTER_TYPE_MAC2_BC 7 //broadcast

typedef struct {
    uint8_t extend;    // value's meanings releate to type & subtype.
    uint8_t type;
    uint16_t subtype;
} wifi_sf_filter_t;

struct sf_cfg_t
{
	unsigned char filter_type;
	unsigned char mac2;
	unsigned char mac[6];
	unsigned char ssid[32];
	unsigned char ssid_len;
	unsigned int  data_len;
    unsigned int  cnt_match;
	unsigned int  cnt_mismatch;
    unsigned int  time;
	unsigned int  seq_num;
	wifi_sf_filter_t filter;
}sf_cfg;

void sf_callback(wifi_promiscuous_pkt_t *pkt_hdr, int len, wifi_promiscuous_pkt_type_t frame_type)
{
	unsigned int i;
    const uint8_t *frame = pkt_hdr->payload;
    
	if(sf_cfg.filter.type == (1<<FC_PV0_TYPE_MGMT))
	{
		if(sf_cfg.filter_type & (1<<FILTER_TYPE_SSID))
		{
			if(frame[37] == sf_cfg.ssid_len)
			{
				for(i=0; i<sf_cfg.ssid_len; i++)
				{
					if(frame[38+i] != sf_cfg.ssid[i])
						goto mismatch;
				}
			}
			else goto mismatch;
		}
		if(sf_cfg.filter_type & (1<<FILTER_TYPE_MAC))
		{
			for(i=0; i<6; i++)
			{
				if(frame[10+i] != sf_cfg.mac[i])  // 16:ap send, 10:sta send
					goto mismatch;
			}
		}
		goto match;
	}

	else if(sf_cfg.filter.type == (1<<FC_PV0_TYPE_DATA))
	{
		if(sf_cfg.filter_type & (1<<FILTER_TYPE_LEN))
		{
			if(len != sf_cfg.data_len)
				goto mismatch;
		}
		if(sf_cfg.filter_type & (1<<FILTER_TYPE_MAC))
		{
			for(i=0; i<6; i++)
			{
				if(frame[10+i] != sf_cfg.mac[i])  // 16:ap send, 10:sta send
					goto mismatch;
			}
		}
		if(sf_cfg.filter_type & (1<<FILTER_TYPE_MAC2))
		{
			if(sf_cfg.mac2 == FILTER_TYPE_MAC2_BC)
			{
				for(i=0; i<6; i++)
				{
					if(frame[16+i] != 0xFF)
						goto mismatch;
				}
			}
			else if(sf_cfg.mac2 == FILTER_TYPE_MAC2_MC)
			{
				if((frame[16]&0x01) != 1)
					goto mismatch;
			}
			else if(sf_cfg.mac2 == FILTER_TYPE_MAC2_UC)
			{
				if((frame[16]&0x01) != 0)
					goto mismatch;
			}
			else return;
		}
match:
		i = (frame[23]<<4)|(frame[22]>>4);
		if(sf_cfg.seq_num != i)
		{
			sf_cfg.seq_num = i;
			sf_cfg.cnt_match++;
		}
		system_printf("SeqNum=%d\n", i);
		return;
	}

	else return;

mismatch:
	sf_cfg.cnt_mismatch++;
}

int parse_mac(char *mac_str, unsigned char *mac_hex)
{
    int i;
    
	for(i=0; i<12; i++)
	{
		if(mac_str[i]>='0' && mac_str[i]<='9')
			mac_str[i] -= '0';
		else if(mac_str[i]>='a' && mac_str[i]<='f')
			mac_str[i] = mac_str[i]-'a'+10;
		else if(mac_str[i]>='A' && mac_str[i]<='F')
			mac_str[i] = mac_str[i]-'A'+10;
		else return -1;
	}
	
	for(i=0; i<6; i++)
		mac_hex[i] = (mac_str[i*2]<<4)+mac_str[i*2+1];

    return 0;
}

/*************************************************************************
    argv[0]   argv[1]       argv[2]                argv[3]
set   sf        off
set   sf      beacon      ch|mac|ssid      1|112233445566|test_8266
set   sf    data/qosdata  ch|mac|len|mac2  1|112233445566|123|bc|mc|uc
show  sf    
*************************************************************************/
static int set_sf_fun(cmd_tbl_t *t, int argc, char *argv[])
{
    int i, ch=0, type=0;
	char mac[12];
    wifi_promiscuous_filter_t filter_cfg;
	
    if (argc < 2)  return CMD_RET_FAILURE;
	memset(&sf_cfg, 0, sizeof(sf_cfg));
    memset(&filter_cfg, 0, sizeof(filter_cfg));

	// argv[0] = "sf"
    // argv[1] = "off/beacon/data/qosdata"
	if(strcmp(argv[1], "off")==0)
	{
		wifi_sniffer_stop();
		return CMD_RET_SUCCESS;
	}
	else if(argc < 4) return CMD_RET_FAILURE;
	else if(strcmp(argv[1], "beacon")==0)
	{
		sf_cfg.filter.type    = 1<<FC_PV0_TYPE_MGMT;
		sf_cfg.filter.subtype = 1<<FC_PV0_TYPE_MGMT_BEACON;
        filter_cfg.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT;
        type = 1;
	}
	else if(strcmp(argv[1], "data")==0)
	{
		sf_cfg.filter.type    = 1<<FC_PV0_TYPE_DATA;
		sf_cfg.filter.subtype = 1<<FC_PV0_TYPE_DATA_DATA;
        filter_cfg.filter_mask = WIFI_PROMIS_FILTER_MASK_DATA;
	}
	else if(strcmp(argv[1], "qosdata")==0)
	{
		sf_cfg.filter.type    = 1<<FC_PV0_TYPE_DATA;
		sf_cfg.filter.subtype = 1<<FC_PV0_TYPE_DATA_QOS_DATA;
        filter_cfg.filter_mask = WIFI_PROMIS_FILTER_MASK_DATA;
	}
	else return CMD_RET_FAILURE;

	for(i=2; i<argc; i+=2)
	{
		if(strcmp(argv[i], "ch")==0)
		{
			ch = atoi(argv[i+1]);
			if(ch < 1 || ch > 14)
				return CMD_RET_FAILURE;
		}
		else if(strcmp(argv[i], "mac")==0)
		{
			memcpy(mac, argv[i+1], 12);
			if(parse_mac(mac, sf_cfg.mac) != 0)
				return CMD_RET_FAILURE;
			sf_cfg.filter_type |= 1<<FILTER_TYPE_MAC;
		}
		else if(strcmp(argv[i], "ssid")==0 && type==1)
		{
			sf_cfg.ssid_len = strlen(argv[i+1]);
			if(sf_cfg.ssid_len < 33)
				memcpy(sf_cfg.ssid, argv[i+1], sf_cfg.ssid_len);
			else
				return CMD_RET_FAILURE;
			sf_cfg.filter_type |= 1<<FILTER_TYPE_SSID;
		}
		else if(strcmp(argv[i], "len")==0 && type==0)
		{
			sf_cfg.data_len = atoi(argv[i+1]);
			if(sf_cfg.data_len > 1500)
				return CMD_RET_FAILURE;
			sf_cfg.filter_type |= 1<<FILTER_TYPE_LEN;
		}
		else if(strcmp(argv[i], "mac2")==0 && type==0)
		{
			if(strcmp(argv[i+1], "bc")==0)
				sf_cfg.mac2 = FILTER_TYPE_MAC2_BC;
			else if(strcmp(argv[i+1], "mc")==0)
				sf_cfg.mac2 = FILTER_TYPE_MAC2_MC;
			else if(strcmp(argv[i+1], "uc")==0)
				sf_cfg.mac2 = FILTER_TYPE_MAC2_UC;
			else
				return CMD_RET_FAILURE;
			sf_cfg.filter_type |= 1<<FILTER_TYPE_MAC2;
		}
		else return CMD_RET_FAILURE;
	}

    wifi_set_promiscuous(0);
    wifi_set_promiscuous_rx_cb(NULL);

	if(ch>0 && wifi_get_sta_status()!=STA_STATUS_CONNECTED)
        wifi_rf_set_channel(ch);

	sf_cfg.cnt_match = 0;
	sf_cfg.cnt_mismatch = 0;
	sf_cfg.time = READ_REG(MAC_REG_TSF_0_LOWER_READONLY);

    wifi_set_promiscuous_filter(&filter_cfg);
    wifi_set_promiscuous_rx_cb((wifi_promiscuous_cb_t)sf_callback);
    wifi_set_promiscuous(true);

	return CMD_RET_SUCCESS;
}


static int show_sf_fun(cmd_tbl_t *t, int argc, char *argv[])
{
	unsigned int time;

	time = READ_REG(MAC_REG_TSF_0_LOWER_READONLY);
    if(time > sf_cfg.time)
		time -= sf_cfg.time;
	else
		time += 0xFFFFFFFF-sf_cfg.time;
	system_printf("Match    Count: %d\n", sf_cfg.cnt_match);
	system_printf("Mismatch Count: %d\n", sf_cfg.cnt_mismatch);
	system_printf("Elapsed  Time : %dms\n", time/1000);

	return CMD_RET_SUCCESS;
}

SUBCMD(set,  sf, set_sf_fun,  "", "");
SUBCMD(show, sf, show_sf_fun, "", "");

static int set_mac_fun(cmd_tbl_t *t, int argc, char *argv[])
{
	char mac[12] = {0};
    unsigned char new_mac[6];

	if(argc < 2) return CMD_RET_FAILURE;

    int vif = atoi(argv[1]) ? 1 : 0;
	memcpy(mac, argv[1], 12);
	if(parse_mac(mac, new_mac) != 0)
		return CMD_RET_FAILURE;

    wifi_set_mac_addr(vif, new_mac);

	return CMD_RET_SUCCESS;
}

SUBCMD(set,   mac, set_mac_fun,  "", "");

#if defined(STACK_LWIP)
static int cmd_dhcp(cmd_tbl_t *t, int argc, char *argv[])
{
	int vif = 0;

	if (argc > 2) {
		system_printf("Help: dhcp [intf name]\n");
		return CMD_RET_FAILURE;
	}
	else if (argc == 2) {
		char *if_num = strchr(argv[1], 'n');

		if (if_num)
			vif = if_num[1] - '0';
	}
	if (vif >= MAX_IF || vif < 0) {
		system_printf("%s: incorrect vif %d \n", __func__, vif);
		return CMD_RET_FAILURE;
	}

	wifi_station_dhcpc_start(vif);
	return CMD_RET_SUCCESS;
}

CMD(dhcp,
    cmd_dhcp,
    "dhcp command",
    "dhcp ");

static int cmd_dhcps(cmd_tbl_t *t, int argc, char *argv[])
{
	int vif = 1;

	if (argc > 2) {
		system_printf("Help: dhcp [intf name]\n");
		return CMD_RET_FAILURE;
	}
	else if (argc == 2) {
		char *if_num = strchr(argv[1], 'n');

		if (if_num)
			vif = if_num[1] - '0';
	}

	wifi_softap_dhcps_start(vif);

	return CMD_RET_SUCCESS;
}

CMD(dhcps,
    cmd_dhcps,
    "dhcps command",
    "dhcps ");

#endif

#ifdef LWIP_IPERF
static int cmd_iperf(cmd_tbl_t *t, int argc, char *argv[])
{
	int i = 0;
	char buf[512] = {0,};

	if (argc <= 1)
	        return CMD_RET_USAGE;

	sprintf(buf, "%s", argv[1]);
	for (i = 2; i < argc; i++) {
	        sprintf(buf, "%s %s", buf, argv[i]);
	}
	iperf_run(buf);
	return CMD_RET_SUCCESS;
}

CMD(iperf,
    cmd_iperf,
    "iperf command",
    "iperf [-s|-c host] [options]");
#endif /* LWIP_IPERF */

static int get_ap_rssi(cmd_tbl_t *t, int argc, char *argv[])
{
	if(1 != argc)
	{
		system_printf("input param num err\n");
		return -1;
	}
	
	int8_t rssi;
	wifi_get_ap_rssi((char *)&rssi);
	system_printf("Connected Ap Rssi:%d \n",rssi);

	return 0;
}

CMD(get_rssi,  get_ap_rssi, " ", " ");

static int station_rssi_connect(cmd_tbl_t *tt, int argc, char *argv[])
{	
	int i;
	wifi_config_u config;
	int scan_mode = WIFI_ALL_CHANNEL_SCAN;

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

	if(argc >= 4)
	{
		scan_mode = atoi(argv[3]);
	}
	
	wifi_set_scan_method(scan_mode);
	
	wifi_start_station(&config);

	return CMD_RET_SUCCESS;
}

CMD(station_rssi_connect, station_rssi_connect,  "station_rssi_connect",  "station_rssi_connect");
static int station_rssi_disconnect(cmd_tbl_t *tt, int argc, char *argv[])
{
	wifi_stop_station();	
	return CMD_RET_SUCCESS;

}

CMD(station_rssi_disconnect, station_rssi_disconnect,  "station_rssi_disconnect",  "station_rssi_disconnect");


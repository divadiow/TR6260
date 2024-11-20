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
#include "lwip/apps/lwiperf.h"
#include "apps/ping/ping.h"
#include "lwip/err.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/sys.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "system_network.h"
#include "system_wifi.h"

/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
bool use_dhcp_server = true;
bool use_dhcp_client = true;

#ifdef LWIP_IPERF
/*
 * Definitions for IP type of service (ip_tos)
 */
#ifndef IPTOS_LOWDELAY
#define IPTOS_LOWDELAY          0x10
#define IPTOS_THROUGHPUT        0x08
#define IPTOS_RELIABILITY       0x04
#define IPTOS_LOWCOST           0x02
#define IPTOS_MINCOST           IPTOS_LOWCOST
#endif /* IPTOS_LOWDELAY */

/*
 * Definitions for DiffServ Codepoints as per RFC2474
 */
#ifndef IPTOS_DSCP_AF11
#define	IPTOS_DSCP_AF11		0x28
#define	IPTOS_DSCP_AF12		0x30
#define	IPTOS_DSCP_AF13		0x38
#define	IPTOS_DSCP_AF21		0x48
#define	IPTOS_DSCP_AF22		0x50
#define	IPTOS_DSCP_AF23		0x58
#define	IPTOS_DSCP_AF31		0x68
#define	IPTOS_DSCP_AF32		0x70
#define	IPTOS_DSCP_AF33		0x78
#define	IPTOS_DSCP_AF41		0x88
#define	IPTOS_DSCP_AF42		0x90
#define	IPTOS_DSCP_AF43		0x98
#define	IPTOS_DSCP_EF		0xb8
#endif /* IPTOS_DSCP_AF11 */
    
#ifndef IPTOS_DSCP_CS0
#define	IPTOS_DSCP_CS0		0x00
#define	IPTOS_DSCP_CS1		0x20
#define	IPTOS_DSCP_CS2		0x40
#define	IPTOS_DSCP_CS3		0x60
#define	IPTOS_DSCP_CS4		0x80
#define	IPTOS_DSCP_CS5		0xa0
#define	IPTOS_DSCP_CS6		0xc0
#define	IPTOS_DSCP_CS7		0xe0
#endif /* IPTOS_DSCP_CS0 */
#ifndef IPTOS_DSCP_EF
#define	IPTOS_DSCP_EF		0xb8
#endif /* IPTOS_DSCP_EF */
#endif

/****************************************************************************
* 	                                           Local Types
****************************************************************************/
#ifdef LWIP_IPERF
enum iperf_role {IPERF_SERVER, IPERF_CLIENT, IPERF_HELP, INVALID_VALUE};
enum iperf_mode {IPERF_TCP, IPERF_UDP};
enum iperf_command_status {IPERF_COMMAND_NONE,IPERF_COMMAND_TRIGGERED};

struct iperf_info {
	u8_t role;
	u8_t mode;
	u8_t tos;
	u32_t bandwidth;
	u16_t port;
	u32_t duration;
	u8_t trigger;
	ip4_addr_t addr;
};

const struct {
    const char *name;
    int value;
} ipqos[] = {
    {"af11", IPTOS_DSCP_AF11},
    {"af12", IPTOS_DSCP_AF12},
    {"af13", IPTOS_DSCP_AF13},
    {"af21", IPTOS_DSCP_AF21},
    {"af22", IPTOS_DSCP_AF22},
    {"af23", IPTOS_DSCP_AF23},
    {"af31", IPTOS_DSCP_AF31},
    {"af32", IPTOS_DSCP_AF32},
    {"af33", IPTOS_DSCP_AF33},
    {"af41", IPTOS_DSCP_AF41},
    {"af42", IPTOS_DSCP_AF42},
    {"af43", IPTOS_DSCP_AF43},
    {"cs0", IPTOS_DSCP_CS0},
    {"cs1", IPTOS_DSCP_CS1},
    {"cs2", IPTOS_DSCP_CS2},
    {"cs3", IPTOS_DSCP_CS3},
    {"cs4", IPTOS_DSCP_CS4},
    {"cs5", IPTOS_DSCP_CS5},
    {"cs6", IPTOS_DSCP_CS6},
    {"cs7", IPTOS_DSCP_CS7},
    {"ef", IPTOS_DSCP_EF},
    {"lowdelay", IPTOS_LOWDELAY},
    {"throughput", IPTOS_THROUGHPUT},
    {"reliability", IPTOS_RELIABILITY},
    {NULL, -1}
};
static TaskHandle_t iperf_task_handle = NULL;
struct iperf_info iperf_info;
#endif

#ifdef LWIP_PING
//static TaskHandle_t ping_task_handle = NULL;
//static TaskHandle_t iperf_task_handle = NULL;

//static struct ping_info* ping;
#endif
//char* hostname;
//bool default_hostname;
/****************************************************************************
* 	                                           Local Constants
****************************************************************************/
#ifdef LWIP_IPERF
    const static char usage_shortstr[] = "Usage: iperf [-s|-c host] [options]\n"
                               "Try `iperf -h' for more information.\n";
    
    const char usage_longstr1[] = "\nUsage: iperf [-s|-c host] [options]\n\n"
                               "  -s, --server              run in server mode\n"
                               "  -c, --client    <host>    run in client mode, connecting to <host> \n"
                               "                            Default <host> is gateway addr. :";
    const char usage_longstr2[] = "  -u, --udp                 use UDP rather than TCP\n";
    const char usage_longstr3[] = "  -p, --port      #         server port to listen on/connect to 5001\n";
    const char usage_longstr4[] = "  -t, --time      #         time in seconds to transmit for (default ";
    const char usage_longstr5[] ="  -S, --tos N               set the IP type of service, 0-255.\n"
                               "                            The usual prefixes for octal and hex can be used,\n"
                               "                            i.e. 52, 064 and 0x34 all specify the same value.\n";
    const char usage_longstr6[] =                           "  -b, --bandwidth for UDP bandwidth to send at in bits/sec\n"
                               "                            The value should be integer value (ex) 100K\n"
                               "                            (default ";
    const u32_t lwiperf_default_udp_bandwidth = 10 * MEGA;
    
#endif  

#ifdef LWIP_PING
const char ping_usage_str[] = "Usage: ping [-c count] [-i interval] [-s packetsize] destination\n"
                           "      `ping destination stop' for stopping\n"
                           "      `ping -st' for checking current ping applicatino status\n";
#endif
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

#ifdef LWIP_PING

u8_t get_vif_by_dstip(ip4_addr_t* addr)
{
	u8_t vif_id = STATION_IF;
	int i;
    struct netif *itf = ip_route(NULL, addr);

    if (!itf)
        return vif_id;
	for(i = 0; i < MAX_IF; i++){
		if(itf == get_netif_by_index(i)){
			vif_id = i;
			break;
		}
	}
	return vif_id;
}

//void ping_usage(void)
//{
//	system_printf( "%s\r\n", ping_usage_str);
//}

void  ping_run(char *cmd)
{
	char *str = NULL;
	char *pcmdStr_log = NULL;
	char *pcmdStr = (char *)cmd;
	double interval = PING_DELAY_DEFAULT;
	u32_t  packet_size= PING_DATA_SIZE;
	u32_t count = 0;
	ip4_addr_t ping_addr;
	u8_t vif_id = 0xFF;
	ping_parm_t* ping_conn = NULL;
	ping_parm_t* ping_stop_conn = NULL;
	int ip4addr_aton_is_valid = false;

	ip4_addr_set_any(&ping_addr);

	if(pcmdStr != NULL)
		memcpy((char **)&pcmdStr_log, (char **)&pcmdStr, sizeof((char *)&pcmdStr));

	ping_conn = (ping_parm_t *)os_zalloc(sizeof(ping_parm_t));
	if (ping_conn == NULL) {
		system_printf("memory allocation fail! [%s]\n", __func__);
		return;
	}
    ping_mutex_init();
	str = strtok_r(NULL, " ", &pcmdStr_log);
	ip4addr_aton_is_valid = ip4addr_aton(str, &ping_addr);  // Check address existence

	if(ip4addr_aton_is_valid){
		vif_id = get_vif_by_dstip(&ping_addr);
		for(;;)
		{
			str = strtok_r(NULL, " ", &pcmdStr_log);
			if(str == NULL || str[0] == '\0'){
				break;
			}else if(strcmp(str , "stop") == 0) {
				ping_stop_conn = ping_get_session(&ping_addr);
				if(ping_stop_conn!= NULL){
					ping_stop_conn->force_stop = 1;
				}else{
					system_printf("Nothing to stop : wlan%d \n", vif_id);
				}
				os_free(ping_conn);
				return;
			}else if(strcmp(str, "-i") == 0){
				str = strtok_r(NULL, " ", &pcmdStr_log);
				interval = PING_DELAY_UNIT * atof(str);
			}else if(strcmp(str, "-c") == 0){
				str = strtok_r(NULL, " ", &pcmdStr_log);
				count = atoi(str);
			}else if(strcmp(str, "-s") == 0){
				str = strtok_r(NULL, " ", &pcmdStr_log);
				packet_size = atoi(str);
			}else{
				system_printf("Error!! '%s' is unknown option. Run help ping\n", str);
				os_free(ping_conn);
				return;
			}
		}
	}else{
		if(strcmp(str, "-st") == 0){
			ping_list_display();
		}else if(strcmp(str, "-h") == 0){
			system_printf( "%s\r\n", ping_usage_str);//ping_usage();
		}else{
			system_printf("Error!! There is no target address. run hep ping\n");
		}
		os_free(ping_conn);
		return;
	}

	if(ip4_addr_isany_val(ping_addr)){
		system_printf("Error!! There is no target address. run help ping\n");
		os_free(ping_conn);
		return;
	}

	ip4_addr_copy((ping_conn->addr), ping_addr);
	ping_conn->interval = (u32_t)interval;
	ping_conn->target_count = (u32_t)count;
	ping_conn->packet_size = (u32_t)packet_size;
	ping_conn->vif_id  = vif_id;

	if(ping_get_session(&ping_conn->addr) != NULL){
		system_printf("Ping application is already running\n");
		os_free(ping_conn);
		return;
	}

	//ping_mutex_init();
	ping_conn->task_handle = (xTaskHandle)ping_init((void*)ping_conn);
	if(ping_conn->task_handle == NULL){
		os_free(ping_conn);
		return;
	}
}
#endif /* LWIP_PING */

#ifdef LWIP_IPERF
static void lwiperf_report(void *arg, enum lwiperf_report_type report_type,
  const ip_addr_t* local_addr, u16_t local_port, const ip_addr_t* remote_addr, u16_t remote_port,
  u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec)
{
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(local_addr);
	LWIP_UNUSED_ARG(local_port);

	system_printf("[IPERF Report] type=%d, Remote: %s:%d, %lu [Bytes], Duration:%d [msec]",\
	(int)report_type, ipaddr_ntoa(remote_addr), \
	(int)remote_port, bytes_transferred, ms_duration);
	system_printf(", %d [kbits/s]\n", bandwidth_kbitpsec);
}

void iperf_usage(void)
{
	system_printf( "%s\r\n", usage_shortstr);
}

void iperf_detailed_usage(void)
{
	system_printf( "%s", usage_longstr1);
	system_printf("%" U16_F ".%" U16_F ".%" U16_F ".%" U16_F"\n", \
		ip4_addr1_16(&(netif_default->gw)), ip4_addr2_16(&(netif_default->gw)),
		ip4_addr3_16(&(netif_default->gw)), ip4_addr4_16(&(netif_default->gw)));
	system_printf( "%s", usage_longstr2);
	system_printf( "%s", usage_longstr3);
	system_printf( "%s", usage_longstr4);
	system_printf( "%d sec)\n", LWIPERF_DEFAULT_DURATION);
	system_printf( "%s", usage_longstr5);
	system_printf( "%s", usage_longstr6);
	system_printf( "%d bit/sec)\n", lwiperf_default_udp_bandwidth);
}

int parse_qos(const char *cp)
{
	unsigned int i;
	char *ep = NULL;
	long val;

	if (cp == NULL)
		return -1;
	for (i = 0; ipqos[i].name != NULL; i++) {
		if (strcasecmp(cp, ipqos[i].name) == 0)
			return ipqos[i].value;
	}
	/* Try parsing as an integer */
	val = strtol(cp, &ep, 0);
	if (*cp == '\0' || *ep != '\0' || val < 0 || val > 255)
		return -1;
	return val;
}



static void iperf_thread(void *arg);

int iperf_run(char *cmd)
{
	iperf_info.role = INVALID_VALUE;
	iperf_info.mode = IPERF_TCP;
	iperf_info.port = 5001;
	iperf_info.duration = LWIPERF_DEFAULT_DURATION;
	iperf_info.tos = IPTOS_DSCP_CS0;
	double bandwidth = lwiperf_default_udp_bandwidth;

	char *str = NULL;
	char *pcmdStr_log = NULL;
	char *pcmdStr = (char *)cmd;
	char suffix = '\0';
	int len = 0;

	ip4_addr_set_any(&iperf_info.addr);

	if(pcmdStr != NULL)
		memcpy((char **)&pcmdStr_log, (char **)&pcmdStr, sizeof((char *)&pcmdStr));

	for(;;) {
		str = strtok_r(NULL, " ", &pcmdStr_log);
		if(str == NULL || str[0] == '\0'){
			break;
		}else if(strcmp(str , "stop") == 0) {
			if(iperf_client_status_get() == LWIPERF_CLIENT_COMMAND_STATUS_START){
				iperf_client_status_set(LWIPERF_CLIENT_COMMAND_STATUS_STOP);
				system_printf("iperf client received stop request\n");
			}
			else{
				system_printf("Nothing to stop\n");
			}
			return 0;
		}else if(strcmp(str, "-u") == 0){
			iperf_info.mode = IPERF_UDP;
		}else if(strcmp(str, "-s") == 0){
			iperf_info.role = IPERF_SERVER;
		}else if(strcmp(str, "-c") == 0){
			iperf_info.role = IPERF_CLIENT;
			str = strtok_r(NULL, " ", &pcmdStr_log);
			ip4addr_aton(str, &iperf_info.addr);
		}else if(strcmp(str, "-t") == 0){
			str = strtok_r(NULL, " ", &pcmdStr_log);
			iperf_info.duration = atoi(str);
		}else if(strcmp(str, "-b") == 0){
			str = strtok_r(NULL, " ", &pcmdStr_log);
			len = strlen(str);
			suffix = str[len-1];
			switch(suffix)
			{
				case 'm': case 'M':
					str[len-1]='\0';
					bandwidth = atof(str)*MEGA;
					break;
				case 'k': case 'K':
					str[len-1]='\0';
					bandwidth = atof(str)*KILO;
					break;
				default:
					bandwidth = atof(str);
					break;
			}
		}else if(strcmp(str, "-p") == 0){
			str = strtok_r(NULL, " ", &pcmdStr_log);
			iperf_info.port = atoi(str);
		}else if(strcmp(str, "-S") == 0){
			str = strtok_r(NULL, " ", &pcmdStr_log);
			iperf_info.tos = parse_qos(str);
		}else if(strcmp(str, "-h") == 0){
			iperf_info.role = IPERF_HELP;
		}else{
			// NOP
		}
	}
	iperf_info.bandwidth = (u32_t)bandwidth;
	iperf_info.trigger  =IPERF_COMMAND_TRIGGERED;
	iperf_client_status_set(LWIPERF_CLIENT_COMMAND_STATUS_START);

#ifdef LWIP_IPERF
	if(!iperf_task_handle){
		iperf_task_handle = sys_thread_new("iperf_thread", iperf_thread, NULL, \
			LWIP_IPERF_TASK_STACK_SIZE, LWIP_IPERF_TASK_PRIORITY);
	}
#endif /* LWIP_IPERF */

	return -1;
}

static void iperf_thread(void *arg)
{
	system_printf("iperf_thread start!!!\n");
	LWIP_UNUSED_ARG(arg);
	while (1) {
	if(iperf_info.trigger){
		switch (iperf_info.role) {
			case IPERF_SERVER:
				if(iperf_info.mode==IPERF_TCP){
					system_printf( "iperf tcp server mode, port:%d\r\n", iperf_info.port);
					lwiperf_start_tcp_server_default(lwiperf_report,NULL, iperf_info.port);
				}else{
					system_printf( "iperf udp server mode, port:%d\r\n", iperf_info.port);
					lwiperf_start_udp_server_default(lwiperf_report,NULL, iperf_info.port);
				}
				break;
			case IPERF_CLIENT:
				if(iperf_info.mode==IPERF_TCP){
					system_printf( "iperf tcp client mode, port:%d duration:%d[sec]\r\n", iperf_info.port, iperf_info.duration);
					lwiperf_start_tcp_client_default(lwiperf_report,NULL, &iperf_info.addr, iperf_info.port,\
											iperf_info.duration,  iperf_info.tos);
				}else{
					system_printf( "iperf udp client mode, port:%d duration:%d[sec]\r\n", iperf_info.port, iperf_info.duration);
					lwiperf_start_udp_client_default(lwiperf_report,NULL, &iperf_info.addr, iperf_info.port,\
											iperf_info.duration,  iperf_info.tos , iperf_info.bandwidth );
				}
				break;
			case IPERF_HELP:
				iperf_detailed_usage();
				break;
			default:
				iperf_usage();
		}
		iperf_info.trigger  =IPERF_COMMAND_NONE;
	}
	sys_arch_msleep(100);
	}
}
#endif /* LWIP_IPERF */

static void set_netdb_ipconfig(int vif, struct ip_info *ipconfig)
{
    netif_db_t *netif_db = NULL;
    
    if (!IS_VALID_VIF(vif))
        return;

    netif_db = get_netdb_by_index(vif);
    netif_db->ipconfig = *ipconfig;
}

void set_softap_ipconfig(struct ip_info *ipconfig)
{
    set_netdb_ipconfig(SOFTAP_IF, ipconfig);
}

void set_sta_ipconfig(struct ip_info *ipconfig)
{
    netif_db_t *netif_db;

    netif_db = get_netdb_by_index(STATION_IF);
    netif_db->dhcp_stat = TCPIP_DHCP_STATIC;
    set_netdb_ipconfig(STATION_IF, ipconfig);
}
// ´ò¿ª dhcp
void wifi_dhcp_open(int vif)
{
	vif == SOFTAP_IF ? (use_dhcp_server = true) : (use_dhcp_client = true);
}

// ¹Ø±Õ dhcp
void wifi_dhcp_close(int vif)
{
	vif == SOFTAP_IF ? (use_dhcp_server = false) : (use_dhcp_client = false);
} 

bool get_wifi_dhcp_use_status(int vif)
{
	bool ret;

	vif == SOFTAP_IF ? (ret = use_dhcp_server) : (ret = use_dhcp_client);

	return ret;
}

int wifi_station_dhcpc_start(int vif)
{
	int8_t ret;
    struct netif *nif = NULL;
    netif_db_t *netif_db;

    if (!IS_VALID_VIF(vif))
        return false;
    
    nif = get_netif_by_index(vif);
    netif_db = get_netdb_by_index(vif);

    if (netif_db->dhcp_stat == TCPIP_DHCP_STATIC) {
        wifi_set_ip_info(vif, &netif_db->ipconfig);
        return true;
    }
    
	if(!use_dhcp_client)
		return false;

	netifapi_dhcp_release_and_stop(nif);

	ip_addr_set_zero(&nif->ip_addr);
	ip_addr_set_zero(&nif->netmask);
	ip_addr_set_zero(&nif->gw);
	netif_db->dhcp_stat = TCPIP_DHCP_STOPPED;
	

    ret = netifapi_dhcp_start(nif);
	if (!ret){
        netif_db->dhcp_stat = TCPIP_DHCP_STARTED;
	}
    SYS_LOGD("dhcpc start %s!", ret ? "fail" : "ok");
    
    return true;
}

int wifi_station_dhcpc_stop(int vif)
{
    struct netif *nif = NULL;
    netif_db_t * netif_db = NULL;

    if (!IS_VALID_VIF(vif))
        return false;
    
    nif = get_netif_by_index(vif);
    netif_db = get_netdb_by_index(vif);

    if (netif_db->dhcp_stat == TCPIP_DHCP_STATIC) {
        netif_set_addr(netif_db->net_if, IP4_ADDR_ANY4, IP4_ADDR_ANY4, IP4_ADDR_ANY4);
        if (STA_STATUS_STOP == wifi_get_status(vif)) { //clear static dhcpc info
            netif_db->dhcp_stat = TCPIP_DHCP_INIT;
            memset(&netif_db->ipconfig, 0, sizeof(netif_db->ipconfig));
        }
        return true;
    }
    
	if (netif_db->dhcp_stat == TCPIP_DHCP_STARTED) {
		netifapi_dhcp_release_and_stop(nif);
        netif_db->dhcp_stat = TCPIP_DHCP_STOPPED;
    	SYS_LOGV("stop dhcp client");
	}
    
	return true;
}

/*get dhcpc/dhcpserver status*/
dhcp_status_t wifi_station_dhcpc_status(int vif)
{
    netif_db_t * netif_db = NULL;

    if (!IS_VALID_VIF(vif))
        return TCPIP_DHCP_STATUS_MAX;
    
    netif_db = get_netdb_by_index(vif);
    return netif_db->dhcp_stat;
}

void wifi_softap_dhcps_start(int intf)
{
    struct ip_info ipinfo_tmp;
    netif_db_t *netif_db;

    if (!IS_VALID_VIF(intf))
        return;

    netif_db = get_netdb_by_index(intf);
    if (netif_db->dhcp_stat == TCPIP_DHCP_STARTED) {
        SYS_LOGD("start dhcps, dhcps already started.");
        return;
    }

    if (ip4_addr_isany(&netif_db->ipconfig.ip)) {
    	IP4_ADDR(&netif_db->ipconfig.ip, 10, 10, 10, 1);
    	IP4_ADDR(&netif_db->ipconfig.gw, 10, 10, 10, 1);
    	IP4_ADDR(&netif_db->ipconfig.netmask, 255, 255, 255, 0);
    }
#ifdef ENABLE_LWIP_NAPT
    if (ip4_addr_isany(&netif_db->ipconfig.dns1)) {
        netif_db->ipconfig.dns1 = (IP_ADDR_ANY != dns_getserver(0)) ? *dns_getserver(0) : netif_db->ipconfig.ip;
        netif_db->ipconfig.dns2 = (IP_ADDR_ANY != dns_getserver(1)) ? *dns_getserver(1) : netif_db->ipconfig.ip;
    }
#endif      
	wifi_set_ip_info(intf, &netif_db->ipconfig);
	if(!use_dhcp_server)
		return;
	dhcps_start(get_netif_by_index(intf), &netif_db->ipconfig);
    netif_db->dhcp_stat = TCPIP_DHCP_STARTED;
}

void wifi_softap_dhcps_stop(int intf)
{
    struct ip_info ipinfo_tmp;
    netif_db_t *netif_db;

    if (!IS_VALID_VIF(intf))
        return;

    netif_db = get_netdb_by_index(intf);
    if (netif_db->dhcp_stat != TCPIP_DHCP_STARTED) {
        SYS_LOGD("stop dhcps, but dhcps not started.");
        return;
    }

    memset(&ipinfo_tmp, 0, sizeof(ipinfo_tmp));
    wifi_set_ip_info(intf, &ipinfo_tmp);
    dhcps_stop();
    netif_db->dhcp_stat = TCPIP_DHCP_STOPPED;
}

dhcp_status_t wifi_softap_dhcps_status(int vif)
{
    netif_db_t * netif_db = NULL;
    dhcp_status_t ret = TCPIP_DHCP_INIT;

    if (!IS_VALID_VIF(vif))
        return TCPIP_DHCP_STATUS_MAX;

    netif_db = get_netdb_by_index(vif);
    ret = netif_db->dhcp_stat;
    
    return ret;
}

void wifi_ifconfig_help_display(void)
{
	system_printf("Usage:\n");
	system_printf("   ifconfig <interface> [<address>]\n");
	system_printf("   ifconfig <interface> [mtu <NN>]\n");
}

void wifi_ifconfig_display(wifi_interface_e if_index)
{
	struct netif *netif_temp = get_netif_by_index(if_index);

    if (!netif_is_up(netif_temp))
        return;
	system_printf("wlan%d     ", if_index);
	system_printf("HWaddr ");
	system_printf(MAC_STR,MAC_VALUE(netif_temp->hwaddr) );
	system_printf("   MTU:%d\n", netif_temp->mtu);
	system_printf("          inet addr:");
	system_printf(IP4_ADDR_STR,IP4_ADDR_VALUE(&(netif_temp->ip_addr)) );
	system_printf("   Mask:");
	system_printf(IP4_ADDR_STR,IP4_ADDR_VALUE(&(netif_temp->netmask)) );
	system_printf("   Gw:");
	system_printf(IP4_ADDR_STR,IP4_ADDR_VALUE(&(netif_temp->gw)) );
	system_printf("\n");
}

void wifi_ifconfig(char* cmd)
{
	char *str = NULL;
	char *pcmdStr_log = NULL;
	char *pcmdStr = (char *)cmd;
	struct ip_info info;
	struct netif *netif_temp;
	int i;

	if(pcmdStr != NULL)
		memcpy((char **)&pcmdStr_log, (char **)&pcmdStr, sizeof((char *)&pcmdStr));

	str = strtok_r(NULL, " ", &pcmdStr_log);
	if(str == NULL || str[0] == '\0'){
		wifi_ifconfig_display(STATION_IF);
		wifi_ifconfig_display(SOFTAP_IF);
	} else {
		 if(strcmp(str, "wlan0") == 0){
			netif_temp = get_netif_by_index(STATION_IF);
			i = STATION_IF;
		}else if(strcmp(str, "wlan1") == 0){
			netif_temp = get_netif_by_index(SOFTAP_IF);
			i = SOFTAP_IF;
		}else if(strcmp(str, "-h") == 0){
			wifi_ifconfig_help_display();
			return;
		}else{
			system_printf("%s is unsupported. run ifconfig -h\n", str);
			return;
		}

		str = strtok_r(NULL, " ", &pcmdStr_log);
		if(str == NULL || str[0] == '\0'){
			wifi_ifconfig_display(i);
		}else if(strcmp(str, "mtu") == 0){
			str = strtok_r(NULL, " ", &pcmdStr_log);
			netif_temp->mtu = atoi(str);
		}else{
			ip4addr_aton(str, &info.ip);
			IP4_ADDR(&info.gw, ip4_addr1_16(&info.ip), ip4_addr2_16(&info.ip), ip4_addr3_16(&info.ip), 1);
			IP4_ADDR(&info.netmask, 255, 255, 255, 0);
			netif_set_addr(netif_temp, &info.ip, &info.netmask, &info.gw);
		}
	}
}

bool wifi_set_ip_info(wifi_interface_e if_index, ip_info_t *info)
{
	netif_set_addr(get_netif_by_index(if_index), &info->ip,  &info->netmask, &info->gw);
    if (!ip4_addr_isany(&info->dns1) || !ip4_addr_isany(&info->dns1)) {
        dns_setserver(0, &info->dns1);
        dns_setserver(1, &info->dns2);
    }
	return true;
}

bool wifi_get_ip_info(wifi_interface_e if_index, struct ip_info *info)
{
    struct netif *nif = NULL;
    
    nif = get_netif_by_index(if_index);
	info->ip = nif->ip_addr;
	info->netmask = nif->netmask;
	info->gw = nif->gw;
    info->dns1 = *dns_getserver(0);
    info->dns1 = *dns_getserver(1);
    
	return true;
}



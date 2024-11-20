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

#ifndef _SYSTEM_NETWORK_H
#define _SYSTEM_NETWORK_H

#include "lwip/netifapi.h"
#include "system_wifi_def.h"

/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
****************************************************************************/


/****************************************************************************
* 	                                        Types
****************************************************************************/

/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/
extern void dhcps_stop(void);
extern void dhcps_start(struct netif *netif, struct ip_info* info);

bool wifi_get_ip_info(wifi_interface_e if_index, struct ip_info *info);
bool wifi_set_ip_info(wifi_interface_e if_index, struct ip_info *info);
void wifi_dhcp_open(int vif);
void wifi_dhcp_close(int vif);
bool get_wifi_dhcp_use_status(int vif);
int wifi_station_dhcpc_start(int vif);
int wifi_station_dhcpc_stop(int vif);
dhcp_status_t wifi_station_dhcpc_status(int vif);
void wifi_softap_dhcps_start(int intf);
void wifi_softap_dhcps_stop(int intf);
dhcp_status_t wifi_softap_dhcps_status(int vif);
void wifi_ifconfig(char* cmd);
void ping_run(char *cmd);
int iperf_run(char *cmd);
void set_softap_ipconfig(struct ip_info *ipconfig);
void set_sta_ipconfig(struct ip_info *ipconfig);



#endif/*_SYSTEM_NETWORK_H*/


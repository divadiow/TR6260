#ifndef _SYSTEM_WIFI_DEF_H
#define _SYSTEM_WIFI_DEF_H
#include "lwip/arch.h"
#include "lwip/ip_addr.h"

#define __maybe_unused __attribute__((unused))

#define WIFI_SSID_MAX_LEN   (32 + 1)
#define WIFI_PWD_MAX_LEN    (64) // pwd string support max length is 63

#define MAC_STR				    "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_VALUE(a)			(a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define IP4_ADDR_STR			"%" U16_F ".%" U16_F ".%" U16_F ".%" U16_F
#define IP4_ADDR_VALUE(addr)	ip4_addr1_16(addr), ip4_addr2_16(addr),ip4_addr3_16(addr), ip4_addr4_16(addr)

#define IS_ZERO_MAC(a)          (!((a)[0] | (a)[1] | (a)[2] | (a)[3] | (a)[4] | (a)[5]))
#define IS_BRODCAST_MAC(a)      (0xff == ((a)[0] & (a)[1] & (a)[2] & (a)[3] & (a)[4] & (a)[5]))
#define IS_MULTCAST_MAC(a)      ((a)[0] & 0x1)

#define IS_VALID_VIF(vif)       ((vif) < MAX_IF && (vif) >= STATION_IF)

#define IP2STR(ipaddr) ip4_addr1_16(ipaddr), \
        ip4_addr2_16(ipaddr), \
        ip4_addr3_16(ipaddr), \
        ip4_addr4_16(ipaddr)
#define IPSTR                   "%d.%d.%d.%d"

typedef enum {
    STATION_IF = 0,
    SOFTAP_IF,
    MAX_IF
} wifi_interface_e;

/* status of DHCP client or DHCP server */
typedef enum {
    TCPIP_DHCP_INIT = 0,    /**< DHCP client/server in initial state */
    TCPIP_DHCP_STATIC,      /**static ip, don't use dhcpc to get address.**/
    TCPIP_DHCP_STARTED,     /**< DHCP client/server already been started */
    TCPIP_DHCP_STOPPED,     /**< DHCP client/server already been stopped */
    TCPIP_DHCP_STATUS_MAX
}dhcp_status_t;

typedef struct ip_info {
    ip_addr_t ip;      /**< IP address */
    ip_addr_t netmask; /**< netmask */
    ip_addr_t gw;      /**< gateway */
    ip_addr_t dns1;
    ip_addr_t dns2;
} ip_info_t;

#endif

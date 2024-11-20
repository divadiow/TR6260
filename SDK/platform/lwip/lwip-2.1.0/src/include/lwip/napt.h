#ifndef __LWIP_NAPT_H__
#define __LWIP_NAPT_H__

#include "lwip/opt.h"

#ifdef __cplusplus
extern "C" {
#endif

#if IP_FORWARD
#if IP_NAPT

/* Default size of the tables used for NAPT */
#define IP_NAPT_MAX 512
#define IP_PORTMAP_MAX 32

/* Timeouts in sec for the various protocol types */
#define IP_NAPT_TIMEOUT_MS_TCP        (300*1000)//(30*60*1000)
#define IP_NAPT_TIMEOUT_MS_TCP_DISCON (5*1000)
#define IP_NAPT_TIMEOUT_MS_UDP        (2*1000)
#define IP_NAPT_TIMEOUT_MS_ICMP       (2*1000)
#define IP_NAPT_TIMEOUT_MS_CHECK      (3 * 1000)

#define IP_NAPT_PORT_RANGE_START 49152
#define IP_NAPT_PORT_RANGE_END   61439

struct napt_table {
  u32_t down_len;
  u32_t up_len;
  u32_t last;
  u32_t src;
  u32_t dest;
  u16_t sport;
  u16_t dport;
  u16_t mport;
  u8_t proto;
  u8_t fin1 : 1;
  u8_t fin2 : 1;
  u8_t finack1 : 1;
  u8_t finack2 : 1;
  u8_t synack : 1;
  u8_t rst : 1;
  u16_t next, prev;
};

/**
 * Allocates and initializes the NAPT tables.
 *
 * @param max_nat max number of enties in the NAPT table (use IP_NAPT_MAX if in doubt)
 * @param max_portmap max number of enties in the NAPT table (use IP_PORTMAP_MAX if in doubt)
 */
void
ip_napt_init(u16_t max_nat);


/**
 * Enable/Disable NAPT for a specified interface.
 *
 * @param addr ip address of the interface
 * @param enable non-zero to enable NAPT, or 0 to disable.
 */
void
ip_napt_enable(u32_t addr, int enable);


/**
 * Enable/Disable NAPT for a specified interface.
 *
 * @param netif number of the interface
 * @param enable non-zero to enable NAPT, or 0 to disable.
 */
void
ip_napt_enable_no(u8_t number, int enable);


/**
 * Sets the NAPT timeout for TCP connections.
 *
 * @param secs timeout in secs
 */
void
ip_napt_set_tcp_timeout(u32_t secs);


/**
 * Sets the NAPT timeout for UDP 'connections'.
 *
 * @param secs timeout in secs
 */
void
ip_napt_set_udp_timeout(u32_t secs);

/**
 * NAPT for a forwarded packet. It checks weather we need NAPT and modify
 * the packet source address and port if needed.
 */
err_t ip_napt_forward(struct pbuf *p, struct ip_hdr *iphdr, struct netif *inp, struct netif *outp);

/**
 * NAPT for an input packet. It checks weather the destination is on NAPT
 * table and modifythe packet destination address and port if needed.
 */
void ip_napt_recv(struct pbuf *p, const struct ip_hdr *iphdr, struct netif *inp);

#endif /* IP_NAPT */
#endif /* IP_FORWARD */

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_NAPT_H__ */

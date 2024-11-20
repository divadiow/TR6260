/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define TCPIP_THREAD_NAME           "tcpip"
#define LWIP_HTTPD_MAX_TAG_NAME_LEN 20
#define LWIP_HTTPD_MAX_TAG_INSERT_LEN 1024
#define TCPIP_THREAD_PRIO LWIP_TASK_PRIORITY
#define TCPIP_THREAD_STACKSIZE LWIP_TASK_STACK_SIZE

#define DEFAULT_TCP_RECVMBOX_SIZE 5
#define DEFAULT_UDP_RECVMBOX_SIZE 5
#define DEFAULT_RAW_RECVMBOX_SIZE 5
#define DEFAULT_ACCEPTMBOX_SIZE 5
#define TCPIP_MBOX_SIZE 10

#define NO_SYS 0
#define LWIP_SOCKET (NO_SYS==0)
#define LWIP_NETCONN 1
#define LWIP_NETCONN_SEM_PER_THREAD     1

#define LWIP_SNMP 0
#define LWIP_IGMP 1
#define LWIP_ICMP 1


#define LWIP_NETIF_EXT_STATUS_CALLBACK  1
#define LWIP_NETIF_API                  1
#define LWIP_NETIF_HOSTNAME             1


/* DNS is not going to be used as this is a simple local example. */
#define LWIP_DNS						1
#define DNS_TABLE_SIZE 1
#define DNS_MAX_NAME_LENGTH 128

#define LWIP_HAVE_LOOPIF				0
#define TCP_LISTEN_BACKLOG				5
#define LWIP_SO_RCVTIMEO		   		1
#define LWIP_SO_RCVBUF			 		1
#define LWIP_SO_SNDRCVTIMEO_NONSTANDARD 1
#define LWIP_SO_SNDTIMEO                1
#define SO_REUSE                        1
#define LWIP_TCP_KEEPALIVE              1

#define LWIP_TCP_TIMESTAMPS             1

//#define LWIP_NOASSERT 1 // To suppress some errors for now (no debug output)
#ifndef ALIYUN
#define LWIP_DEBUG
#endif

#ifdef LWIP_DEBUG

#define LWIP_DBG_MIN_LEVEL        LWIP_DBG_LEVEL_ALL // LWIP_DBG_LEVEL_SERIOUS
#define PPP_DEBUG                  LWIP_DBG_OFF
#define MEM_DEBUG                  LWIP_DBG_OFF
#define MEMP_DEBUG                 LWIP_DBG_OFF
#define PBUF_DEBUG                 LWIP_DBG_OFF
#define API_LIB_DEBUG              LWIP_DBG_OFF
#define API_MSG_DEBUG              LWIP_DBG_OFF
#define TCPIP_DEBUG                LWIP_DBG_OFF
#define NETIF_DEBUG                LWIP_DBG_OFF
#define SOCKETS_DEBUG              LWIP_DBG_OFF
#define DNS_DEBUG                  LWIP_DBG_OFF
#define AUTOIP_DEBUG               LWIP_DBG_OFF
#define DHCP_DEBUG                 LWIP_DBG_OFF
#define DHCPS_DEBUG                LWIP_DBG_OFF
#define IP_DEBUG                   LWIP_DBG_OFF
#define NAPT_DEBUG                 LWIP_DBG_OFF
#define IP_REASS_DEBUG             LWIP_DBG_OFF
#define ICMP_DEBUG                 LWIP_DBG_OFF
#define IGMP_DEBUG                 LWIP_DBG_OFF
#define UDP_DEBUG                  LWIP_DBG_OFF
#define TCP_DEBUG                  LWIP_DBG_OFF
#define TCP_INPUT_DEBUG            LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG           LWIP_DBG_OFF
#define TCP_RTO_DEBUG              LWIP_DBG_OFF
#define TCP_CWND_DEBUG             LWIP_DBG_OFF
#define TCP_WND_DEBUG              LWIP_DBG_OFF
#define TCP_FR_DEBUG               LWIP_DBG_OFF
#define TCP_QLEN_DEBUG             LWIP_DBG_OFF
#define TCP_RST_DEBUG              LWIP_DBG_OFF
#define ETHARP_DEBUG               LWIP_DBG_OFF
#endif

#define LWIP_DBG_TYPES_ON (LWIP_DBG_ON|LWIP_DBG_TRACE|\
LWIP_DBG_STATE|LWIP_DBG_FRESH|LWIP_DBG_HALT)

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT        1

/**
 * LWIP_TCPIP_CORE_LOCKING
 * Creates a global mutex that is held during TCPIP thread operations.
 * Can be locked by client code to perform lwIP operations without changing
 * into TCPIP thread using callbacks. See LOCK_TCPIP_CORE() and
 * UNLOCK_TCPIP_CORE().
 * Your system should provide mutexes supporting priority inversion to use
this.
 */
#define LWIP_TCPIP_CORE_LOCKING         0

//#define LWIP_STATS_DISPLAY              1
#define LWIP_TIMERS                     1

/**
 * LWIP_TCPIP_CORE_LOCKING_INPUT: when LWIP_TCPIP_CORE_LOCKING is enabled,
 * this lets tcpip_input() grab the mutex for input packets as well,
 * instead of allocating a message and passing it to tcpip_thread.
 *
 * ATTENTION: this does not work when tcpip_input() is called from
 * interrupt context!
 */
#ifndef LWIP_TCPIP_CORE_LOCKING_INPUT
#define LWIP_TCPIP_CORE_LOCKING_INPUT   0
#endif

/* ---------- Memory options ---------- */
/**
 * MEM_LIBC_MALLOC==1: Use malloc/free/realloc provided by your C-library
 * instead of the lwip internal allocator. Can save code size if you
 * already use it.
 */
#define MEM_LIBC_MALLOC        1

/**
* MEMP_MEM_MALLOC==1: Use mem_malloc/mem_free instead of the lwip pool allocator.
* Especially useful with MEM_LIBC_MALLOC but handle with care regarding execution
* speed and usage from interrupts!
*/
#define MEMP_MEM_MALLOC                 1

/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                2000

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           8

/* MEMP_NUM_RAW_PCB: the number of UDP protocol control blocks. One
   per active RAW "connection". */
#define LWIP_RAW			1
#define MEMP_NUM_RAW_PCB		8

/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        6
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        10
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 8
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG       2*TCP_SND_QUEUELEN //  8
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    20


/* The following four are used only with the sequential API and can be
   set to 0 if the application only will use the raw API. */
/* MEMP_NUM_NETBUF: the number of struct netbufs. */
#define MEMP_NUM_NETBUF         12
/* MEMP_NUM_NETCONN: the number of struct netconns. */
#define MEMP_NUM_NETCONN        12
/* MEMP_NUM_APIMSG: the number of struct api_msg, used for
   communication between the TCP/IP stack and the sequential
   programs. */
#define MEMP_NUM_API_MSG        8
/* MEMP_NUM_TCPIPMSG: the number of struct tcpip_msg, which is used
   for sequential API communication and incoming packets. Used in
   src/api/tcpip.c. */
//#define MEMP_NUM_TCPIP_MSG      8

/* These two control is reclaimer functions should be compiled
   in. Should always be turned on (1). */
//#define MEM_RECLAIM             1
//#define MEMP_RECLAIM            1

/**
 * MEMP_OVERFLOW_CHECK: memp overflow protection reserves a configurable
 * amount of bytes before and after each memp element in every pool and fills
 * it with a prominent default value.
 *    MEMP_OVERFLOW_CHECK == 0 no checking
 *    MEMP_OVERFLOW_CHECK == 1 checks each element when it is freed
 *    MEMP_OVERFLOW_CHECK >= 2 checks each element in every pool every time
 *      memp_malloc() or memp_free() is called (useful but slow!)
 */
#ifndef MEMP_OVERFLOW_CHECK
#define MEMP_OVERFLOW_CHECK             0
#endif

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          10

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       1600

/* PBUF_LINK_HLEN: the number of bytes that should be allocated for a
   link level header. */
#define PBUF_LINK_HLEN          16

/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         1

/* TCP Maximum segment size. */
#define TCP_MSS                 1460

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             TCP_MSS*3

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN        6 * TCP_SND_BUF/TCP_MSS

/* TCP receive window. */
#define TCP_WND                 (2 * TCP_MSS)

/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX              3

/* Maximum number of retransmissions of SYN segments. */
#define TCP_SYNMAXRTX           4


#define TCP_OOSEQ_MAX_BYTES             (2 * TCP_MSS)

/* LWIP_TCP_SACK_OUT==1: TCP will support sending selective acknowledgements (SACKs) */
#ifndef LWIP_TCP_SACK_OUT
#define LWIP_TCP_SACK_OUT               1
#endif

/* ---------- ARP options ---------- */
#define LWIP_ARP		1
#define ARP_TABLE_SIZE 10
#define ETHARP_TABLE_MATCH_NETIF        1

/* ---------- IP options ---------- */
/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#ifdef ENABLE_LWIP_NAPT
#define IP_FORWARD              1
#define IP_NAPT                 1
#endif

/* IP reassembly and segmentation.These are orthogonal even
 * if they both deal with IP fragments */
#define IP_REASSEMBLY			1
#define IP_REASS_MAX_PBUFS		10
#define MEMP_NUM_REASSDATA		10
#define IP_FRAG					1

/* If defined to 1, IP options are allowed (but not parsed). If
   defined to 0, all packets with IP options are dropped. */
#define IP_OPTIONS_ALLOWED              1

/* ---------- ICMP options ---------- */
#define ICMP_TTL                255


/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. DHCP is not implemented in lwIP 0.5.1, however, so
   turning this on does currently not work. */
#define LWIP_DHCP               1


/* 1 if you want to do an ARP check on the offered address
   (recommended). */
#define DHCP_DOES_ARP_CHECK     ((LWIP_DHCP) && (LWIP_ARP))


/* Define LWIP_DHCPS to 1 if you want to run DHCP Server.*/
#define LWIP_DHCPS               1

/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255

/* ---------- Statistics options ---------- */
//#define STATS

#ifdef STATS
#define LINK_STATS 1
#define IP_STATS   1
#define ICMP_STATS 1
#define UDP_STATS  1
#define TCP_STATS  1
#define MEM_STATS  1
#define MEMP_STATS 1
#define SYS_STATS  1
#endif /* STATS */

#define LWIP_PROVIDE_ERRNO 1
//#define LWIP_TIMEVAL_PRIVATE 0

/* Used with IP headers only */
#define LWIP_CHKSUM_ALGORITHM 1

/* LWIP_STATS==1: Enable statistics collection in lwip_stats.  */
#define LWIP_STATS 0

// enable ipv4 src route
#define LWIP_HOOK_IP4_ROUTE_SRC(s, d)         (void *)ip4_route_src_hook(s, d)

//#ifdef MPW
#if 1
#define LWIP_TIMEVAL_PRIVATE 1
#else
#define LWIP_TIMEVAL_PRIVATE 0
#endif

#if LWIP_NETCONN_SEM_PER_THREAD
#define LWIP_NETCONN_THREAD_SEM_GET() sys_thread_sem_get()
#define LWIP_NETCONN_THREAD_SEM_ALLOC() sys_thread_sem_init()
#define LWIP_NETCONN_THREAD_SEM_FREE() sys_thread_sem_deinit()
#endif

#endif /* __LWIPOPTS_H__ */

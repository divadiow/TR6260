/**
 * @file
 * lwIP iPerf server implementation
 */

/**
 * @defgroup iperf Iperf server
 * @ingroup apps
 *
 * This is a simple performance measuring server to check your bandwith using
 * iPerf2 on a PC as client.
 * It is currently a minimal implementation providing an IPv4 TCP server only.
 *
 * @todo: implement UDP mode and IPv6
 */

/*
 * Copyright (c) 2014 Simon Goldschmidt
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
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmidt
 */

#include "lwip/apps/lwiperf.h"

#include "lwip/tcp.h"
#include "lwip/sys.h"

#include <string.h>

#include "system.h"
#include "lwip/udp.h"

static u8_t iperf_client_status;

/* Currently, only TCP-over-IPv4 is implemented (does iperf support IPv6 anyway?) */
#if LWIP_IPV4 && LWIP_TCP && LWIP_CALLBACK_API

/** Specify the idle timeout (in seconds) after that the test fails */
#ifndef LWIPERF_TCP_MAX_IDLE_SEC
#define LWIPERF_TCP_MAX_IDLE_SEC    10U
#endif
#if LWIPERF_TCP_MAX_IDLE_SEC > 255
#error LWIPERF_TCP_MAX_IDLE_SEC must fit into an u8_t
#endif

/* File internal memory allocation (struct lwiperf_*): this defaults to
   the heap */
#ifndef LWIPERF_ALLOC
#define LWIPERF_ALLOC(type)         mem_malloc(sizeof(type))
#define LWIPERF_FREE(type, item)    mem_free(item)
#endif

/** If this is 1, check that received data has the correct format */
#ifndef LWIPERF_CHECK_RX_DATA
#define LWIPERF_CHECK_RX_DATA       0
#endif

/** This is the Iperf settings struct sent from the client */
typedef struct _lwiperf_settings {
#define LWIPERF_FLAGS_ANSWER_TEST 0x80000000
#define LWIPERF_FLAGS_ANSWER_NOW  0x00000001
  u32_t flags;
  u32_t num_threads; /* unused for now */
  u32_t remote_port;
  u32_t buffer_len; /* unused for now */
  u32_t win_band; /* TCP window / UDP rate: unused for now */
  u32_t amount; /* pos. value: bytes?; neg. values: time (unit is 10ms: 1/100 second) */
} lwiperf_settings_t;

/** Basic connection handle */
struct _lwiperf_state_base;
typedef struct _lwiperf_state_base lwiperf_state_base_t;
struct _lwiperf_state_base {
	/* 1=tcp, 0=udp */
	u8_t tcp;
	/* 1=server, 0=client */
	u8_t server;
	lwiperf_state_base_t* next;
	lwiperf_state_base_t* related_server_state;
};

/** Connection handle for a TCP iperf session */
typedef struct _lwiperf_state {
	lwiperf_state_base_t base;
	struct tcp_pcb* server_pcb;
	struct tcp_pcb* conn_pcb;
	struct udp_pcb* udp_conn_pcb;
	int udp_client_status;
	u32_t packet_number;
	u32_t time_started;
	lwiperf_report_fn report_fn;
	void* report_arg;
	u8_t poll_count;
	u8_t next_num;
	u32_t bytes_transferred;
	lwiperf_settings_t settings;
	u8_t have_settings_buf;
} lwiperf_state_t;

/** List of active iperf sessions */
static lwiperf_state_base_t* lwiperf_all_connections;
#ifdef MPW
/** A const buffer to send from: we want to measure sending, not copying! */
const u8_t lwiperf_txbuf_const[1600] = {
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
  '0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
};

#else
extern const u8_t lwiperf_txbuf_const[1600];

#endif

static err_t lwiperf_tcp_poll(void *arg, struct tcp_pcb *tpcb);
static void lwiperf_tcp_err(void *arg, err_t err);

/** Add an iperf session to the 'active' list */
static void
lwiperf_list_add(lwiperf_state_base_t* item)
{
	if (lwiperf_all_connections == NULL) {
		lwiperf_all_connections = item;
	} else {
		item = lwiperf_all_connections;
	}
}

/** Remove an iperf session from the 'active' list */
static void
lwiperf_list_remove(lwiperf_state_base_t* item)
{
	lwiperf_state_base_t* prev = NULL;
	lwiperf_state_base_t* iter;
	for (iter = lwiperf_all_connections; iter != NULL; prev = iter, iter = iter->next) {
		if (iter == item) {
			if (prev == NULL) {
				lwiperf_all_connections = iter->next;
			} else {
				prev->next = item;
			}
			/* @debug: ensure this item is listed only once */
			for (iter = iter->next; iter != NULL; iter = iter->next) {
				LWIP_ASSERT("duplicate entry", iter != item);
			}
			break;
		}
	}
}

/** Call the report function of an iperf tcp session */
static void
lwip_conn_report(lwiperf_state_t* conn, enum lwiperf_report_type report_type)
{
	if ((conn != NULL) && (conn->report_fn != NULL)) {
		u32_t now, duration_ms, bandwidth_kbitpsec;
		now = sys_now();
		duration_ms = now - conn->time_started;
		if (duration_ms == 0) {
			bandwidth_kbitpsec = 0;
		} else {
			bandwidth_kbitpsec = (8U * conn->bytes_transferred / duration_ms) ;
		}

		if(report_type == LWIPERF_UDP_DONE_SERVER || report_type == LWIPERF_UDP_DONE_CLIENT) {
			conn->report_fn(conn->report_arg, report_type,
			&conn->udp_conn_pcb->local_ip, conn->udp_conn_pcb->local_port,
			&conn->udp_conn_pcb->remote_ip, conn->udp_conn_pcb->remote_port,
			conn->bytes_transferred, duration_ms, bandwidth_kbitpsec);
		}else {
			conn->report_fn(conn->report_arg, report_type,
			&conn->conn_pcb->local_ip, conn->conn_pcb->local_port,
			&conn->conn_pcb->remote_ip, conn->conn_pcb->remote_port,
			conn->bytes_transferred, duration_ms, bandwidth_kbitpsec);
		}
	}
}

/** Close an iperf tcp session */
static void
lwiperf_tcp_close(lwiperf_state_t* conn, enum lwiperf_report_type report_type)
{
	err_t err;

	lwip_conn_report(conn, report_type);
	lwiperf_list_remove(&conn->base);
	if (conn->conn_pcb != NULL) {
		tcp_arg(conn->conn_pcb, NULL);
		tcp_poll(conn->conn_pcb, NULL, 0);
		tcp_sent(conn->conn_pcb, NULL);
		tcp_recv(conn->conn_pcb, NULL);
		tcp_err(conn->conn_pcb, NULL);
		err = tcp_close(conn->conn_pcb);
		if (err != ERR_OK) {
			/* don't want to wait for free memory here... */
			tcp_abort(conn->conn_pcb);
		}
	} else {
		/* no conn pcb, this is the server pcb */
		err = tcp_close(conn->server_pcb);
		LWIP_ASSERT("error", err != ERR_OK);
	}
  LWIPERF_FREE(lwiperf_state_tcp_t, conn);
}

/** Try to send more data on an iperf tcp session */
static err_t
lwiperf_tcp_client_send_more(lwiperf_state_t* conn)
{
	int send_more;
	err_t err;
	u16_t txlen;
	u16_t txlen_max;
	void* txptr;
	u8_t apiflags;

	LWIP_ASSERT("conn invalid", (conn != NULL) && conn->base.tcp && (conn->base.server == 0));

	do {
		send_more = 0;
		if (conn->settings.amount & PP_HTONL(0x80000000)) {
			/* this session is time-limited */
			u32_t now = sys_now();
			u32_t diff_ms = now - conn->time_started;
			u32_t time = (u32_t)-(s32_t)lwip_htonl(conn->settings.amount);
			u32_t time_ms = time * 10;
			if (diff_ms >= time_ms || iperf_client_status_get() == LWIPERF_CLIENT_COMMAND_STATUS_STOP) {
				iperf_client_status_set(LWIPERF_CLIENT_COMMAND_STATUS_STOP);
				/* time specified by the client is over -> close the connection */
				lwiperf_tcp_close(conn, LWIPERF_TCP_DONE_CLIENT);
				return ERR_OK;
			}
		} else {
			/* this session is byte-limited */
			u32_t amount_bytes = lwip_htonl(conn->settings.amount);
			/* @todo: this can send up to 1*MSS more than requested... */
			if (amount_bytes >= conn->bytes_transferred) {
				/* all requested bytes transferred -> close the connection */
				lwiperf_tcp_close(conn, LWIPERF_TCP_DONE_CLIENT);
				return ERR_OK;
			}
		}

		if (conn->bytes_transferred < 24) {
			/* transmit the settings a first time */
			txptr = &((u8_t*)&conn->settings)[conn->bytes_transferred];
			txlen_max = (u16_t)(24 - conn->bytes_transferred);
			apiflags = TCP_WRITE_FLAG_COPY;
		} else if (conn->bytes_transferred < 48) {
			/* transmit the settings a second time */
			txptr = &((u8_t*)&conn->settings)[conn->bytes_transferred - 24];
			txlen_max = (u16_t)(48 - conn->bytes_transferred);
			apiflags = TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE;
			send_more = 1;
		} else {
			/* transmit data */
			/* @todo: every x bytes, transmit the settings again */
			txptr = LWIP_CONST_CAST(void*, &lwiperf_txbuf_const[conn->bytes_transferred % 10]);
			txlen_max = TCP_MSS;
			if (conn->bytes_transferred == 48) { /* @todo: fix this for intermediate settings, too */
				txlen_max = TCP_MSS - 24;
			}
			apiflags = 0; /* no copying needed */
			send_more = 1;
		}
		txlen = txlen_max;
		do {
			err = tcp_write(conn->conn_pcb, txptr, txlen, apiflags);
			if (err ==  ERR_MEM) {
				txlen /= 2;
			}
		} while ((err == ERR_MEM) && (txlen >= (TCP_MSS/2)));

		if (err == ERR_OK) {
			conn->bytes_transferred += txlen;
		} else {
			send_more = 0;
		}
	} while(send_more);

	tcp_output(conn->conn_pcb);
	return ERR_OK;
}

/** TCP sent callback, try to send more data */
static err_t
lwiperf_tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	lwiperf_state_t* conn = (lwiperf_state_t*)arg;
	/* @todo: check 'len' (e.g. to time ACK of all data)? for now, we just send more... */
	LWIP_ASSERT("invalid conn", conn->conn_pcb == tpcb);
	LWIP_UNUSED_ARG(tpcb);
	LWIP_UNUSED_ARG(len);

	conn->poll_count = 0;

	return lwiperf_tcp_client_send_more(conn);
}

/** TCP connected callback (active connection), send data now */
static err_t
lwiperf_tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	lwiperf_state_t* conn = (lwiperf_state_t*)arg;
	LWIP_ASSERT("invalid conn", conn->conn_pcb == tpcb);
	LWIP_UNUSED_ARG(tpcb);
	if (err != ERR_OK) {
		lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_REMOTE);
		return ERR_OK;
	}
	conn->poll_count = 0;
	conn->time_started = sys_now();
	return lwiperf_tcp_client_send_more(conn);
}

/** Start TCP connection back to the client (either parallel or after the
 * receive test has finished.
 */
static err_t
lwiperf_tx_start(lwiperf_state_t* conn)
{
	err_t err;
	lwiperf_state_t* client_conn;
	struct tcp_pcb* newpcb;
	ip_addr_t remote_addr;
	u16_t remote_port;

	client_conn = (lwiperf_state_t*)LWIPERF_ALLOC(lwiperf_state_t);
	if (client_conn == NULL) {
		return ERR_MEM;
	}
	newpcb = tcp_new();
	if (newpcb == NULL) {
		LWIPERF_FREE(lwiperf_state_t, client_conn);
		return ERR_MEM;
	}

	MEMCPY(client_conn, conn, sizeof(lwiperf_state_t));
	client_conn->base.server = 0;
	client_conn->server_pcb = NULL;
	client_conn->conn_pcb = newpcb;
	client_conn->time_started = sys_now(); /* @todo: set this again on 'connected' */
	client_conn->poll_count = 0;
	client_conn->next_num = 4; /* initial nr is '4' since the header has 24 byte */
	client_conn->bytes_transferred = 0;
	client_conn->settings.flags = 0; /* prevent the remote side starting back as client again */

	tcp_arg(newpcb, client_conn);
	tcp_sent(newpcb, lwiperf_tcp_client_sent);
	tcp_poll(newpcb, lwiperf_tcp_poll, 2U);
	tcp_err(newpcb, lwiperf_tcp_err);

	ip_addr_copy(remote_addr, conn->conn_pcb->remote_ip);
	remote_port = (u16_t)lwip_htonl(client_conn->settings.remote_port);

	err = tcp_connect(newpcb, &remote_addr, remote_port, lwiperf_tcp_client_connected);
	if (err != ERR_OK) {
		lwiperf_tcp_close(client_conn, LWIPERF_TCP_ABORTED_LOCAL);
		return err;
	}
	lwiperf_list_add(&client_conn->base);
	return ERR_OK;
}

/** Receive data on an iperf tcp session */
static err_t
lwiperf_tcp_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	u8_t tmp;
	u16_t tot_len;
	u32_t packet_idx;
	struct pbuf* q;
	lwiperf_state_t* conn = (lwiperf_state_t*)arg;

	LWIP_ASSERT("pcb mismatch", conn->conn_pcb == tpcb);
	LWIP_UNUSED_ARG(tpcb);

	if (err != ERR_OK) {
		lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_REMOTE);
		return ERR_OK;
	}
	if (p == NULL) {
		/* connection closed -> test done */
		if ((conn->settings.flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST|LWIPERF_FLAGS_ANSWER_NOW)) ==
		PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST)) {
			/* client requested transmission after end of test */
			lwiperf_tx_start(conn);
		}
		lwiperf_tcp_close(conn, LWIPERF_TCP_DONE_SERVER);
		return ERR_OK;
	}
	tot_len = p->tot_len;

	conn->poll_count = 0;

	if ((!conn->have_settings_buf) || ((conn->bytes_transferred -24) % (1024*128) == 0)) {
		/* wait for 24-byte header */
		if (p->tot_len < sizeof(lwiperf_settings_t)) {
			lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL_DATAERROR);
			pbuf_free(p);
			return ERR_VAL;
		}
		if (!conn->have_settings_buf) {
			if (pbuf_copy_partial(p, &conn->settings, sizeof(lwiperf_settings_t), 0) != sizeof(lwiperf_settings_t)) {
				//lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL);
				//pbuf_free(p);
				//return ERR_VAL;
			}
			conn->have_settings_buf = 1;
            #if 0
			if ((conn->settings.flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST|LWIPERF_FLAGS_ANSWER_NOW)) ==
			PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST|LWIPERF_FLAGS_ANSWER_NOW)) {
				/* client requested parallel transmission test */
				err_t err2 = lwiperf_tx_start(conn);
				if (err2 != ERR_OK) {
				lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL_TXERROR);
				pbuf_free(p);
				return err2;
				}
			}
            #endif
		} else {
			if (pbuf_memcmp(p, 0, &conn->settings, sizeof(lwiperf_settings_t)) != 0) {
				//lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL_DATAERROR);
				//pbuf_free(p);
				//return ERR_VAL;
			}
		}
		conn->bytes_transferred += sizeof(lwiperf_settings_t);
		if (conn->bytes_transferred <= 24) {
		conn->time_started = sys_now();
			tcp_recved(tpcb, p->tot_len);
			pbuf_free(p);
			return ERR_OK;
		}
		conn->next_num = 4; /* 24 bytes received... */
		tmp = pbuf_header(p, -24);
		LWIP_ASSERT("pbuf_header failed", tmp == 0);
	}

	packet_idx = 0;
	for (q = p; q != NULL; q = q->next) {
#if LWIPERF_CHECK_RX_DATA
		const u8_t* payload = (const u8_t*)q->payload;
		u16_t i;
		for (i = 0; i < q->len; i++) {
			u8_t val = payload[i];
			u8_t num = val - '0';
			if (num == conn->next_num) {
				conn->next_num++;
				if (conn->next_num == 10) {
					conn->next_num = 0;
				}
			} else {
				lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL_DATAERROR);
				pbuf_free(p);
				return ERR_VAL;
			}
		}
#endif
		packet_idx += q->len;
	}
	LWIP_ASSERT("count mismatch", packet_idx == p->tot_len);
	conn->bytes_transferred += packet_idx;
	tcp_recved(tpcb, tot_len);
	pbuf_free(p);
	return ERR_OK;
}

/** Error callback, iperf tcp session aborted */
static void
lwiperf_tcp_err(void *arg, err_t err)
{
	lwiperf_state_t* conn = (lwiperf_state_t*)arg;
	LWIP_UNUSED_ARG(err);
	lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_REMOTE);
}

/** TCP poll callback, try to send more data */
static err_t
lwiperf_tcp_poll(void *arg, struct tcp_pcb *tpcb)
{
	lwiperf_state_t* conn = (lwiperf_state_t*)arg;
	LWIP_ASSERT("pcb mismatch", conn->conn_pcb == tpcb);
	LWIP_UNUSED_ARG(tpcb);
	if (++conn->poll_count >= LWIPERF_TCP_MAX_IDLE_SEC) {
		lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL);
		return ERR_OK; /* lwiperf_tcp_close frees conn */
	}

	if (!conn->base.server) {
		lwiperf_tcp_client_send_more(conn);
	}

	return ERR_OK;
}

/** This is called when a new client connects for an iperf tcp session */
static err_t
lwiperf_tcp_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	lwiperf_state_t *s, *conn;
	if ((err != ERR_OK) || (newpcb == NULL) || (arg == NULL)) {
		return ERR_VAL;
	}

	s = (lwiperf_state_t*)arg;
	conn = (lwiperf_state_t*)LWIPERF_ALLOC(lwiperf_state_t);
	if (conn == NULL) {
		return ERR_MEM;
	}
	memset(conn, 0, sizeof(lwiperf_state_t));
	conn->base.tcp = 1;
	conn->base.server = 1;
	conn->base.related_server_state = &s->base;
	conn->server_pcb = s->server_pcb;
	conn->conn_pcb = newpcb;
	conn->time_started = sys_now();
	conn->report_fn = s->report_fn;
	conn->report_arg = s->report_arg;

	/* setup the tcp rx connection */
	tcp_arg(newpcb, conn);
	tcp_recv(newpcb, lwiperf_tcp_recv);
	tcp_poll(newpcb, lwiperf_tcp_poll, 2U);
	tcp_err(conn->conn_pcb, lwiperf_tcp_err);

	lwiperf_list_add(&conn->base);
	return ERR_OK;
}

/**
 * @ingroup iperf
 * Start a TCP iperf server on the default TCP port (5001) and listen for
 * incoming connections from iperf clients.
 *
 * @returns a connection handle that can be used to abort the server
 *          by calling @ref lwiperf_abort()
 */
void*
lwiperf_start_tcp_server_default(lwiperf_report_fn report_fn, void* report_arg, u16_t port)
{
	return lwiperf_start_tcp_server(IP_ADDR_ANY, port, report_fn, report_arg);
}

/**
 * @ingroup iperf
 * Start a TCP iperf server on a specific IP address and port and listen for
 * incoming connections from iperf clients.
 *
 * @returns a connection handle that can be used to abort the server
 *          by calling @ref lwiperf_abort()
 */
void*
lwiperf_start_tcp_server(const ip_addr_t* local_addr, u16_t local_port,
  lwiperf_report_fn report_fn, void* report_arg)
{
	err_t err;
	struct tcp_pcb* pcb;
	lwiperf_state_t* s;

	if (local_addr == NULL) {
		return NULL;
	}

	s = (lwiperf_state_t*)LWIPERF_ALLOC(lwiperf_state_t);
	if (s == NULL) {
		return NULL;
	}
	memset(s, 0, sizeof(lwiperf_state_t));
	s->base.tcp = 1;
	s->base.server = 1;
	s->report_fn = report_fn;
	s->report_arg = report_arg;

	pcb = tcp_new();
	if (pcb != NULL) {
		err = tcp_bind(pcb, local_addr, local_port);
		if (err == ERR_OK) {
			s->server_pcb = tcp_listen_with_backlog(pcb, 1);
		}
	}
	if (s->server_pcb == NULL) {
		if (pcb != NULL) {
			tcp_close(pcb);
		}
		LWIPERF_FREE(lwiperf_state_t, s);
		return NULL;
	}
	pcb = NULL;

	tcp_arg(s->server_pcb, s);
	tcp_accept(s->server_pcb, lwiperf_tcp_accept);

	lwiperf_list_add(&s->base);
	return s;
}

/**
 * @ingroup iperf
 * Abort an iperf session (handle returned by lwiperf_start_tcp_server*())
 */
void
lwiperf_abort(void* lwiperf_session)
{
	lwiperf_state_base_t* i, *dealloc, *last = NULL;

	for (i = lwiperf_all_connections; i != NULL; ) {
		if ((i == lwiperf_session) || (i->related_server_state == lwiperf_session)) {
			dealloc = i;
			i = i->next;
			if (last != NULL) {
				last->next = i;
			}
			LWIPERF_FREE(lwiperf_state_t, dealloc); /* @todo: type? */
		} else {
			last = i;
			i = i->next;
		}
	}
}


/*
 * Start TCP client connection to the server
 */
void*
lwiperf_start_tcp_client(const ip_addr_t* remote_addr, u16_t remote_port, u32_t duration,
  lwiperf_report_fn report_fn, void* report_arg, u8_t tos)
{
	err_t err;
	u16_t remote_port_temp = remote_port;
	lwiperf_state_t* client_conn;
	struct tcp_pcb* newpcb;

	client_conn = (lwiperf_state_t*)LWIPERF_ALLOC(lwiperf_state_t);

	if (client_conn == NULL) {
		return NULL;
	}
	newpcb = tcp_new();

	if (newpcb == NULL) {
		LWIPERF_FREE(lwiperf_state_t, client_conn);
		return NULL;
	}
	newpcb->tos = tos;

	memset(client_conn, 0, sizeof(lwiperf_state_t));
	client_conn->base.tcp = 1;

	client_conn->base.server = 0;
	client_conn->server_pcb = NULL;
	client_conn->conn_pcb = newpcb;
	client_conn->time_started = sys_now(); /* @todo: set this again on 'connected' */
	client_conn->poll_count = 0;
	client_conn->next_num = 4; /* initial nr is '4' since the header has 24 byte */
	client_conn->bytes_transferred = 0;

	client_conn->settings.flags = 0; /* prevent the remote side starting back as client again */
	client_conn->settings.amount =(s32_t)lwip_htonl((100*duration)*(-1)) ;

	client_conn->report_fn = report_fn;
	client_conn->report_arg = report_arg;

	tcp_arg(newpcb, client_conn);

	tcp_recv(newpcb, lwiperf_tcp_recv);
	tcp_sent(newpcb, lwiperf_tcp_client_sent);
	tcp_poll(newpcb, lwiperf_tcp_poll, 2U);
	tcp_err(newpcb, lwiperf_tcp_err);

	err = tcp_connect(newpcb, remote_addr, remote_port_temp, lwiperf_tcp_client_connected);
	if (err != ERR_OK) {
		lwiperf_tcp_close(client_conn, LWIPERF_TCP_ABORTED_LOCAL);
		return NULL;
	}
	lwiperf_list_add(&client_conn->base);
	return client_conn;
}


/**
 * Start TCP client connection to the server on the default UDP port (5001).
 *
 * @returns a connection handle that can be used to abort the server
 *          by calling @ref lwiperf_abort()
 */
void*
lwiperf_start_tcp_client_default(lwiperf_report_fn report_fn,
		void* report_arg, ip_addr_t *addr,u16_t port,u32_t duration, u8_t tos)
{
	if (ip4_addr_isany(addr)) {
		return lwiperf_start_tcp_client(&(netif_default->gw), port, duration, report_fn, report_arg, tos);
	}else{
		return lwiperf_start_tcp_client(addr, port, duration, report_fn, report_arg, tos);
	}
}

static u8_t payload_buffer[LWIP_MEM_ALIGN_BUFFER(LWIPERF_UDP_DATA_SIZE+1)];
static u8_t* payload = &payload_buffer[0];

/** Receive data on an iperf udp session in server mode*/
static void
lwiperf_udp_recv(void *arg, struct udp_pcb *upcb,
                               struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	s32_t  pcount;

	lwiperf_state_t *conn = (lwiperf_state_t*)arg;

	if (p != NULL) {
		 /* copy payload inside static buffer for processing */
		if (pbuf_copy_partial(p, payload, p->tot_len, 0) == p->tot_len) {
			memcpy(&pcount, payload, sizeof(pcount));
			pcount = lwip_htonl(pcount);
			if(pcount == 0){
				conn->udp_conn_pcb->remote_port= port;
				memcpy(&conn->udp_conn_pcb->remote_ip,addr,sizeof(ip_addr_t));
				conn->time_started= sys_now();
				conn->bytes_transferred = 0;
			}
			conn->bytes_transferred += (p->tot_len );

			/** Check the last packet and reply to the server */
			if(pcount<0){
				udp_sendto(upcb,p, addr, port);
				 lwip_conn_report(conn, LWIPERF_UDP_DONE_SERVER);
			}
		}
		/* free the pbuf */
		pbuf_free(p);
	}
}

/**
 * Start a UDP iperf server on a specific IP address and port.
 *
 * @returns a connection handle that can be used to abort the server
 *          by calling @ref lwiperf_abort()
 */
void*
lwiperf_start_udp_server(const ip_addr_t* local_addr, u16_t local_port,
  lwiperf_report_fn report_fn, void* report_arg)
{
	err_t err;
	struct udp_pcb *pcb;
	lwiperf_state_t* s;

	if (local_addr == NULL) {
		return NULL;
	}

	s = (lwiperf_state_t*)LWIPERF_ALLOC(lwiperf_state_t);
	if (s == NULL) {
		return NULL;
	}
	memset(s, 0, sizeof(lwiperf_state_t));

	/* create new UDP PCB structure */
	pcb = udp_new();
	if (!pcb) {
		system_printf("Error creating PCB. Out of Memory\r\n");
		return NULL;
	}
	s->base.tcp = 0;
	s->base.server =1;
	s->report_fn = report_fn;
	s->report_arg = report_arg;
	s->udp_conn_pcb = pcb;

	/* bind to @port */
	err = udp_bind(pcb, IP_ADDR_ANY, local_port);
	if (err != ERR_OK) {
		system_printf("Unable to bind to port %d: err = %d\r\n", local_port, err);
		return NULL;
	}
	udp_recv(pcb, lwiperf_udp_recv, s);
	lwiperf_list_add(&s->base);
	return s;
}


/**
 * Start a UDP iperf server on the default UDP port (5001) and listen for
 * incoming connections from iperf clients.
 *
 * @returns a connection handle that can be used to abort the server
 *          by calling @ref lwiperf_abort()
 */
void*
lwiperf_start_udp_server_default(lwiperf_report_fn report_fn, void* report_arg,u16_t port)
{
	return lwiperf_start_udp_server(IP_ADDR_ANY, port, report_fn, report_arg);
}

/** Receive data on an iperf udp session in client mode*/
static void
udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	lwiperf_state_t* conn = (lwiperf_state_t* )arg;

	if(conn->udp_client_status == LWIPERF_UDP_CLIENT_STOPPING){
		conn->udp_client_status = LWIPERF_UDP_CLIENT_STOP;
	}

	/* Free receive pbuf */
	if(p){
		pbuf_free(p);
	}

}

/** Try to send more data on an iperf udp session */
err_t lwiperf_udp_client_send_more(lwiperf_state_t* conn, int send_more, u16_t port, u32_t duration, u32_t bandwidth)
{
	struct pbuf *p;
	u32_t  packet_number;
	u32_t tv_sec;
	u32_t tv_usec;
	u32_t test_mode;
	u32_t flags;
	u32_t numThreads;
	u32_t listen_port;
	u32_t bufferlen;
	u32_t mUDPRate;
	u32_t mAmount;
	u32_t sent_time = sys_now();
	err_t err = ERR_MEM;

	p = pbuf_alloc(PBUF_TRANSPORT,LWIPERF_UDP_DATA_SIZE, PBUF_POOL);

	tv_sec =  lwip_htonl(sent_time/MILI_PER_SECOND);
	tv_usec =  lwip_htonl((sent_time%MILI_PER_SECOND)*1000);
	flags =  lwip_htonl(0);
	numThreads =  lwip_htonl(1);
	listen_port =  lwip_htonl(port);
	bufferlen =  lwip_htonl(0);
	mUDPRate =  lwip_htonl(bandwidth);
	mAmount = (s32_t)lwip_htonl((100*duration)*(-1));

	if(send_more){
		conn->packet_number++;
		packet_number = lwip_htonl(conn->packet_number);
		conn->udp_client_status = LWIPERF_UDP_CLIENT_RUNNING;
	}else{
		packet_number = ~lwip_htonl(conn->packet_number);
		conn->udp_client_status = LWIPERF_UDP_CLIENT_STOPPING;
		sys_arch_msleep(10);
		conn->packet_number++;
	}

	if (p != NULL)
	{
		/* copy data to pbuf */
		pbuf_take_at(p, (char*)(lwiperf_txbuf_const+LWIPERF_UDP_CLIENT_DATA_OFFSET),
		LWIPERF_UDP_DATA_SIZE-LWIPERF_UDP_CLIENT_DATA_OFFSET, LWIPERF_UDP_CLIENT_DATA_OFFSET);

		/* update iperf header information */
		MEMCPY(p->payload, &packet_number, 4);
		MEMCPY(p->payload+4, &tv_sec, 4);
		MEMCPY(p->payload+8, &tv_usec, 4);
		MEMCPY(p->payload+12, &flags, 4);
		MEMCPY(p->payload+16, &numThreads, 4);
		MEMCPY(p->payload+20, &listen_port, 4);
		MEMCPY(p->payload+24, &bufferlen, 4);
		MEMCPY(p->payload+28, &mUDPRate, 4);
		MEMCPY(p->payload+32, &mAmount, 4);

		/* send udp data */
		err = udp_send(conn->udp_conn_pcb , p);

		/* free pbuf */
		pbuf_free(p);
	}
    return err;
}

/** Close an iperf udp session */
static void
lwiperf_udp_close(lwiperf_state_t* conn, enum lwiperf_report_type report_type)
{
	//lwip_conn_report(conn, report_type);
	//  lwiperf_list_remove(&conn->base);
	LWIPERF_FREE(lwiperf_state_t, conn);
	conn->udp_client_status = LWIPERF_UDP_CLIENT_IDLE;
}

/** Start UDP connection to the server */
void*
lwiperf_start_udp_client(const ip_addr_t* remote_addr, u16_t remote_port,u32_t duration,
  lwiperf_report_fn report_fn, void* report_arg, u8_t tos, u32_t bandwidth)
{
	err_t err;
	int send_more;
	int waiting_count = 0;

	lwiperf_state_t* client_conn;
	struct udp_pcb* newpcb;
	u16_t listen_port = LWIPERF_UDP_PORT_DEFAULT;
	u16_t remote_port_temp = remote_port;
	u16_t time_delay;

	client_conn = (lwiperf_state_t*)LWIPERF_ALLOC(lwiperf_state_t);
	if (client_conn == NULL) {
		system_printf("Unable to allocate to client_conn\r\n");
		return NULL;
	}

	newpcb = udp_new();
	if (newpcb == NULL) {
		LWIPERF_FREE(lwiperf_state_t, client_conn);
		system_printf("Unable to create udp pcb\r\n");
		return NULL;
	}
	newpcb->tos = tos;

	memset(client_conn, 0, sizeof(lwiperf_state_t));
	client_conn->udp_client_status = LWIPERF_UDP_CLIENT_IDLE;

	err = udp_bind(newpcb, IP_ADDR_ANY, 0);
	if (err != ERR_OK) {
		system_printf("Unable to bind to port %d: err = %d\r\n", remote_port, err);
		return NULL;
	}

	udp_recv(newpcb, udp_receive_callback, client_conn);

	err = udp_connect(newpcb, remote_addr, remote_port_temp);
	if (err != ERR_OK) {
		system_printf("Unable to connect to port %d: err = %d\r\n", remote_port_temp, err);
		return NULL;
	}
	sys_arch_msleep(500);

	send_more = 1;

	client_conn->base.tcp = 0;
	client_conn->base.server = 0;
	client_conn->udp_conn_pcb = newpcb;
	client_conn->report_fn = report_fn;
	client_conn->report_arg = report_arg;
	client_conn->bytes_transferred = 0;
	client_conn->udp_conn_pcb->remote_port = remote_port;
	client_conn->settings.amount =(s32_t)lwip_htonl((100*duration)*(-1)) ;
	client_conn->packet_number = -1;
	client_conn->udp_client_status = LWIPERF_UDP_CLIENT_START;
	//time_delay = (u16_t)((BYTES_TO_BITS*LWIPERF_UDP_DATA_SIZE* MILI_PER_SECOND) / bandwidth) ;
	time_delay = (u16_t)(((BYTES_TO_BITS*LWIPERF_UDP_DATA_SIZE* MILI_PER_SECOND) / bandwidth) * 99) / 100;
    if (time_delay < portTICK_RATE_MS)
        time_delay = portTICK_RATE_MS;
	system_printf("iperf start: packet delay = %d ms, bandwidth = %d \n" , time_delay , bandwidth );
	sys_arch_msleep(500);

	client_conn->time_started = sys_now();
	do {
		/* this session is time-limited */
		u32_t now = sys_now();
		u32_t diff_ms = now - client_conn->time_started;
		u32_t time = (u32_t)-(s32_t)lwip_htonl(client_conn->settings.amount);
		u32_t time_ms = time * 10;
		if (diff_ms >= time_ms || iperf_client_status_get() == LWIPERF_CLIENT_COMMAND_STATUS_STOP) {
			iperf_client_status_set(LWIPERF_CLIENT_COMMAND_STATUS_STOP);
			send_more = 0;
		}
		err = lwiperf_udp_client_send_more(client_conn, send_more, listen_port, duration, bandwidth);
	        if (err != ERR_OK) {
	            system_printf("lwiperf_udp_client_send_more!! code[%d]\n", err);
	        }
		client_conn->bytes_transferred += LWIPERF_UDP_DATA_SIZE;
		sys_arch_msleep(time_delay);
	} while(send_more);

	lwip_conn_report(client_conn, LWIPERF_UDP_DONE_CLIENT);
	while(client_conn->udp_client_status != LWIPERF_UDP_CLIENT_STOP){
		sys_arch_msleep(10);
		if(waiting_count++ > 10){
			system_printf("Not recieve report packet from server!!\n");
			break;
		}
	}

	lwiperf_udp_close(client_conn, LWIPERF_UDP_DONE_CLIENT);
	udp_remove(newpcb);
	return NULL;
}

/** Start UDP connection to the server on the default UDP port (5001) */
void*
lwiperf_start_udp_client_default(lwiperf_report_fn report_fn, void* report_arg, ip_addr_t *addr,
      u16_t port, u32_t duration, u8_t tos, u32_t bandwidth)
{
      if (ip4_addr_isany(addr)) {
		return lwiperf_start_udp_client(&(netif_default->gw), port, duration, report_fn, report_arg, tos, bandwidth);
	}else{
		return lwiperf_start_udp_client(addr, port, duration, report_fn, report_arg, tos, bandwidth);
	}
}

void
iperf_client_status_set(int status)
{
	iperf_client_status = status;
}

u8_t
iperf_client_status_get(void)
{
	return iperf_client_status;
}

#endif /* LWIP_IPV4 && LWIP_TCP && LWIP_CALLBACK_API && LWIP_UDP*/

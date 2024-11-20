/*
 * WPA Supplicant - Layer2 packet handling example with dummy functions
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This file can be used as a starting point for layer2 packet implementation.
 */


#include "system.h"

#include "utils/includes.h"
#include "utils/common.h"
#include "utils/eloop.h"
#include "utils/wpa_debug.h"
#include "driver.h"
#include "scan.h"
#include "l2_packet.h"


extern int nrc_transmit_from_8023(uint8_t vif_id, uint8_t *frame, const uint16_t len);

#ifdef CONFIG_NO_STDOUT_DEBUG
#ifdef wpa_printf
#undef wpa_printf
#if 0
static void wpa_printf(int level, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	system_vprintf(fmt, ap);
	system_printf("\n");
    va_end(ap);
}
#else
#define wpa_printf(args...) do { } while (0)
#endif
#endif
#endif

#define MAX_L2_PACKET_FILTER			(3)

extern int nrc_transmit(uint8_t *frm, const uint16_t len);
static struct l2_packet_data *l2_head = NULL;

struct l2_packet_data {
	char ifname[100];
	void *rx_callback_ctx;
	void (*rx_callback)(void *ctx, const u8 *src, const u8 *buf, size_t len);
	unsigned short protocol;
	int l2_hdr;
	uint8_t addr[ETH_ALEN];
	struct l2_packet_data * next;
};

int wpas_l2_packet_filter(uint8_t *buffer, int len)
{
	struct l2_packet_data *l2 = l2_head;
	struct l2_ethhdr *hdr = (struct l2_ethhdr *) buffer;
	const int HLEN = sizeof(struct l2_ethhdr);

	if (len < HLEN)
		return 0;

	while (l2) {
		if (l2->protocol == hdr->h_proto) {
			if (memcmp(hdr->h_dest, l2->addr, ETH_ALEN) == 0) {
				l2->rx_callback(l2->rx_callback_ctx, hdr->h_source,
						buffer + HLEN, len - HLEN);
				return 1;
			}
		}
		l2 = l2->next;
	}

	return 0;
}

struct l2_packet_data * l2_packet_init(const char *ifname, const u8 *own_addr,
			unsigned short protocol,
			void (*rx_callback)(void *ctx, const u8 *src, const u8 *buf, size_t len),
			void *rx_callback_ctx,
			int l2_hdr)
{
	struct l2_packet_data *l2 = NULL, *l2_pos = NULL;

	wpa_printf(MSG_DEBUG, "l2_packet: %s (addr: " MACSTR
			", proto: 0x%X, l2_hdr: %d)", __func__, MAC2STR(own_addr),
			protocol, l2_hdr);

	if (protocol != ETH_P_EAPOL) {
		wpa_printf(MSG_ERROR, "l2_packet: %s Unsupported l2 proto (0x%X).",
			__func__, protocol);
		return NULL;
	}

	l2 = (struct l2_packet_data *) os_zalloc(sizeof(*l2));

	if (!l2) {
		wpa_printf(MSG_ERROR, "Failed to alloc l2_packet.");
		return NULL;
	}

	os_strlcpy(l2->ifname, ifname, sizeof(l2->ifname));

	l2->rx_callback = rx_callback;
	l2->rx_callback_ctx = rx_callback_ctx;
	l2->l2_hdr = l2_hdr;
	os_memcpy(l2->addr, own_addr, ETH_ALEN);
	l2->protocol = htons(protocol);
	l2->next = NULL;

	if (!l2_head)
		l2_head = l2;
	else {
		l2_pos = l2_head;
		while (l2_pos->next)
			l2_pos = l2_pos->next;
		l2_pos->next = l2;
	}

	return l2;
}

struct l2_packet_data * l2_packet_init_bridge(const char *br_ifname,
			const char *ifname, const u8 *own_addr,
			unsigned short protocol,
			void (*rx_callback)(void *ctx, const u8 *src_addr, const u8 *buf, size_t len),
			void *rx_callback_ctx,
			int l2_hdr)
{
	return NULL;
}

int l2_packet_send(struct l2_packet_data *l2, const u8 *dst_addr, u16 proto,
			const u8 *buf, size_t len)
{
	int ret = 0;
	uint8_t vif_id = 0;
	wpa_printf(MSG_DEBUG, "%s : len %d", __func__, (int)len);

	if (os_strcmp(l2->ifname, "wlan1") == 0){
		vif_id = 1;
	}else{
		vif_id = 0;
	}

	if (!l2 || !buf || len < sizeof(struct l2_ethhdr))
		return -1;

	if (l2->l2_hdr) {
		nrc_transmit_from_8023(vif_id, (uint8_t *) buf, len);
	}
	else {
		struct l2_ethhdr *eth = (struct l2_ethhdr *) os_malloc(sizeof(*eth)
										+ len);
		if (eth == NULL)
			return -1;

		os_memcpy(eth->h_dest, dst_addr, ETH_ALEN);
		os_memcpy(eth->h_source, l2->addr, ETH_ALEN);
		eth->h_proto = htons(proto);
		os_memcpy(eth + 1, buf, len);
		nrc_transmit_from_8023(vif_id,(uint8_t *) eth, len + sizeof(*eth));
		os_free(eth);
	}

	return ret;
}

int l2_packet_get_ip_addr(struct l2_packet_data *l2, char *buf, size_t len)
{
	/* TODO: get interface IP address */
	wpa_printf(MSG_DEBUG, "%s : len %d", __func__, (int)len);
	return -1;
}

void l2_packet_notify_auth_start(struct l2_packet_data *l2)
{
	/* Do nothing */
	wpa_printf(MSG_DEBUG, "%s:[time]<4way>", __func__);
}

int l2_packet_set_packet_filter(struct l2_packet_data *l2,
				enum l2_packet_filter_type type)
{
	wpa_printf(MSG_DEBUG, "%s", __func__);
	return -1;
}

int l2_packet_get_own_addr(struct l2_packet_data *l2, u8 *addr)
{
	wpa_printf(MSG_DEBUG, "%s", __func__);
	os_memcpy(addr, l2->addr, ETH_ALEN);
	return 0;
}
int l2_packet_set_own_addr(struct l2_packet_data *l2, u8 *addr)
{
	wpa_printf(MSG_DEBUG, "%s", __func__);
	os_memcpy(l2->addr, addr, ETH_ALEN);
	return 0;
}

void l2_packet_deinit(struct l2_packet_data *l2)
{
	struct l2_packet_data *l2_pos = l2_head;

	if (!l2)
		return;

	if (l2 == l2_head) {
		os_free(l2_head);
		l2_head = NULL;
	}

	while (l2_pos) {
		if (l2_pos->next == l2) {
			l2_pos->next = l2->next;
			os_free(l2);
		}
	}
}

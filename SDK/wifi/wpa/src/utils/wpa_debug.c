/*
 * wpa_supplicant/hostapd / Debug prints
 * Copyright (c) 2002-2013, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"

#include "common.h"



#ifdef _FREERTOS
#include <FreeRTOS.h>
#include "system.h"
#endif /* _FREERTOS */

int wpa_debug_level = MSG_EXCESSIVE;
int wpa_debug_show_keys = 0;

#ifndef CONFIG_NO_STDOUT_DEBUG
/**
 * wpa_printf - conditional printf
 * @level: priority level (MSG_*) of the message
 * @fmt: printf format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration.
 *
 * Note: New line '\n' is added to the end of the text when printing to stdout.
 */
void wpa_printf(int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (level >= wpa_debug_level) {
    	system_vprintf(fmt, ap);
    	system_printf("\n");
	}
	va_end(ap);
}


static void _wpa_hexdump(int level, const char *title, const u8 *buf,
			 size_t len, int show)
{
	size_t i;
    
    if (level < wpa_debug_level)
        return;

	system_printf("%s - hexdump(len=%lu):", title, (unsigned long) len);
	if (buf == NULL) {
		system_printf(" [NULL]");
	} else if (show) {
		for (i = 0; i < len; i++)
			system_printf(" %02x", buf[i]);
	} else {
		system_printf(" [REMOVED]");
	}
	system_printf("\n");
}

void wpa_hexdump(int level, const char *title, const void *buf, size_t len)
{
	_wpa_hexdump(level, title, buf, len, wpa_debug_show_keys);
}


void wpa_hexdump_key(int level, const char *title, const void *buf, size_t len)
{
	_wpa_hexdump(level, title, buf, len, wpa_debug_show_keys);
}


static void _wpa_hexdump_ascii(int level, const char *title, const void *buf,
			       size_t len, int show)
{
	size_t i, llen;
	const u8 *pos = buf;
	const size_t line_len = 16;

	if (level < wpa_debug_level)
		return;

	if (!show) {
		system_printf("%s - hexdump_ascii(len=%lu): [REMOVED]\n",
		       title, (unsigned long) len);
		return;
	}
	if (buf == NULL) {
		system_printf("%s - hexdump_ascii(len=%lu): [NULL]\n",
		       title, (unsigned long) len);
		return;
	}
	system_printf("%s - hexdump_ascii(len=%lu):\n", title, (unsigned long) len);
	while (len) {
		llen = len > line_len ? line_len : len;
		system_printf("    ");
		for (i = 0; i < llen; i++)
			system_printf(" %02x", pos[i]);
		for (i = llen; i < line_len; i++)
			system_printf("   ");
		system_printf("   ");
		for (i = 0; i < llen; i++) {
			if (isprint(pos[i]))
				system_printf("%c", pos[i]);
			else
				system_printf("_");
		}
		for (i = llen; i < line_len; i++)
			system_printf(" ");
		system_printf("\n");
		pos += llen;
		len -= llen;
	}
}


void wpa_hexdump_ascii(int level, const char *title, const void *buf,
		       size_t len)
{
	_wpa_hexdump_ascii(level, title, buf, len, wpa_debug_show_keys);
}


void wpa_hexdump_ascii_key(int level, const char *title, const void *buf,
			   size_t len)
{
	_wpa_hexdump_ascii(level, title, buf, len, wpa_debug_show_keys);
}
#endif

//#ifndef CONFIG_NO_WPA_MSG
#if 0
void wpa_msg_register_ifname_cb(wpa_msg_get_ifname_func func)
{
}

void wpa_msg(void *ctx, int level, const char *fmt, ...)
{
	va_list ap;
    
	if (level >= wpa_debug_level) {
		system_printf("WPA: ");
		va_start(ap, fmt);
		system_vprintf(fmt, ap);
		system_printf("\n");
		va_end(ap);
	}
}

void wpa_msg_ctrl(void *ctx, int level, const char *fmt, ...)
{
	va_list ap;
	char *buf;
	int buflen;
	int len;

	va_start(ap, fmt);
	buflen = vsnprintf(NULL, 0, fmt, ap) + 1;
	va_end(ap);

	buf = os_malloc(buflen);
	if (buf == NULL) {
		wpa_printf(MSG_ERROR, "wpa_msg_ctrl: Failed to allocate "
			   "message buffer");
		return;
	}
	va_start(ap, fmt);
	len = vsnprintf(buf, buflen, fmt, ap);
	va_end(ap);
    wpa_printf(level, "%s", buf);
	bin_clear_free(buf, buflen);
}


void wpa_msg_global(void *ctx, int level, const char *fmt, ...)
{
	va_list ap;
	char *buf;
	int buflen;
	int len;

	va_start(ap, fmt);
	buflen = vsnprintf(NULL, 0, fmt, ap) + 1;
	va_end(ap);

	buf = os_malloc(buflen);
	if (buf == NULL) {
		wpa_printf(MSG_ERROR, "wpa_msg_global: Failed to allocate "
			   "message buffer");
		return;
	}
	va_start(ap, fmt);
	len = vsnprintf(buf, buflen, fmt, ap);
	va_end(ap);
	wpa_printf(level, "%s", buf);
	bin_clear_free(buf, buflen);
}


void wpa_msg_global_ctrl(void *ctx, int level, const char *fmt, ...)
{
	va_list ap;
	char *buf;
	int buflen;
	int len;

	va_start(ap, fmt);
	buflen = vsnprintf(NULL, 0, fmt, ap) + 1;
	va_end(ap);

	buf = os_malloc(buflen);
	if (buf == NULL) {
		wpa_printf(MSG_ERROR,
			   "wpa_msg_global_ctrl: Failed to allocate message buffer");
		return;
	}
	va_start(ap, fmt);
	len = vsnprintf(buf, buflen, fmt, ap);
	va_end(ap);
    wpa_printf(level, "%s", buf);
	bin_clear_free(buf, buflen);
}


void wpa_msg_no_global(void *ctx, int level, const char *fmt, ...)
{
	va_list ap;
	char *buf;
	int buflen;
	int len;

	va_start(ap, fmt);
	buflen = vsnprintf(NULL, 0, fmt, ap) + 1;
	va_end(ap);

	buf = os_malloc(buflen);
	if (buf == NULL) {
		wpa_printf(MSG_ERROR, "wpa_msg_no_global: Failed to allocate "
			   "message buffer");
		return;
	}
	va_start(ap, fmt);
	len = vsnprintf(buf, buflen, fmt, ap);
	va_end(ap);
	wpa_printf(level, "%s", buf);
	bin_clear_free(buf, buflen);
}


void wpa_msg_global_only(void *ctx, int level, const char *fmt, ...)
{
	va_list ap;
	char *buf;
	int buflen;
	int len;

	va_start(ap, fmt);
	buflen = vsnprintf(NULL, 0, fmt, ap) + 1;
	va_end(ap);

	buf = os_malloc(buflen);
	if (buf == NULL) {
		wpa_printf(MSG_ERROR, "%s: Failed to allocate message buffer",
			   __func__);
		return;
	}
	va_start(ap, fmt);
	len = vsnprintf(buf, buflen, fmt, ap);
	va_end(ap);
	wpa_printf(level, "%s", buf);
	os_free(buf);
}
#endif

#ifndef CONFIG_NO_HOSTAPD_LOGGER

void hostapd_logger_register_cb(hostapd_logger_cb_func func)
{
}

void hostapd_logger(void *ctx, const u8 *addr, unsigned int module, int level,
		    const char *fmt, ...)
{
	va_list ap;
	char *buf;
	int buflen;
	int len;

	va_start(ap, fmt);
	buflen = vsnprintf(NULL, 0, fmt, ap) + 1;
	va_end(ap);

	buf = os_malloc(buflen);
	if (buf == NULL) {
		wpa_printf(MSG_ERROR, "hostapd_logger: Failed to allocate "
			   "message buffer");
		return;
	}
	va_start(ap, fmt);
	len = vsnprintf(buf, buflen, fmt, ap);
	va_end(ap);

	if (addr)
		wpa_printf(MSG_DEBUG, "hostapd_logger: STA " MACSTR " - %s",
			   MAC2STR(addr), buf);
	else
		wpa_printf(MSG_DEBUG, "hostapd_logger: %s", buf);
	os_free(buf);
}
#endif /* CONFIG_NO_HOSTAPD_LOGGER */

const char * debug_level_str(int level)
{
	switch (level) {
	case MSG_EXCESSIVE:
		return "EXCESSIVE";
	case MSG_MSGDUMP:
		return "MSGDUMP";
	case MSG_DEBUG:
		return "DEBUG";
	case MSG_INFO:
		return "INFO";
	case MSG_WARNING:
		return "WARNING";
	case MSG_ERROR:
		return "ERROR";
	default:
		return "?";
	}
}


int str_to_debug_level(const char *s)
{
	if (os_strcasecmp(s, "EXCESSIVE") == 0)
		return MSG_EXCESSIVE;
	if (os_strcasecmp(s, "MSGDUMP") == 0)
		return MSG_MSGDUMP;
	if (os_strcasecmp(s, "DEBUG") == 0)
		return MSG_DEBUG;
	if (os_strcasecmp(s, "INFO") == 0)
		return MSG_INFO;
	if (os_strcasecmp(s, "WARNING") == 0)
		return MSG_WARNING;
	if (os_strcasecmp(s, "ERROR") == 0)
		return MSG_ERROR;
	return -1;
}

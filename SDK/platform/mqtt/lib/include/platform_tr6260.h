/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 * Tuan PM <tuanpm at live dot com>
 */
#ifndef _ESP_PLATFORM_H__
#define _ESP_PLATFORM_H__

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

void *mqtt_calloc(size_t nmemb, size_t size);
void *mqtt_realloc(void *ptr, size_t size);
void *mqtt_malloc( size_t size );
void mqtt_free( void *pv );
char *platform_create_id_string();
uint16_t platform_random(int max);
long long platform_tick_get_ms();
void ms_to_timeval(int timeout_ms, struct timeval *tv);

#define MEM_CHECK(a, action) if (!(a)) {                                                      \
        system_printf("%s:%d (%s): %s", __FILE__, __LINE__, __FUNCTION__, "Memory exhausted");       \
        action;                                                                                         \
        }
#endif

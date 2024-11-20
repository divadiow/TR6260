#include "platform_tr6260.h"
#include "FreeRTOS.h"
#include "task.h"
#include "drv_trng.h"
#include "drv_rtc.h"

#define MAX_ID_STRING (32)

char *platform_create_id_string()
{
    char *id_string = mqtt_calloc(1, MAX_ID_STRING);
    sprintf(id_string, "T6260_%02x%02X%02X",1,2,3);
    return id_string;
}

uint16_t platform_random(int max)
{
    return trng_read()%max;
}

long long platform_tick_get_ms()
{
    long long milliseconds = xTaskGetTickCount()*10; // calculate milliseconds
    return milliseconds;
}

void ms_to_timeval(int timeout_ms, struct timeval *tv)
{
    tv->tv_sec = timeout_ms / 1000;
    tv->tv_usec = (timeout_ms - (tv->tv_sec * 1000)) * 1000;
}

void *mqtt_realloc(void *ptr, size_t size)
{
    char *ret;
    ret = (char *)os_realloc(ptr, size);
    if (ret == NULL) {
        return NULL;
    }

    return ret;
}

void *mqtt_calloc(size_t nmemb, size_t size)
{
    char *ret;
    ret = (char *)os_calloc(nmemb, size);
    if (ret == NULL) {
        return NULL;
    } else {
        memset(ret, 0, nmemb*size);
    }
    return ret;
}

void *mqtt_malloc( size_t size )
{
    char *ret;
    ret = (char *)os_malloc(size);
    if (ret == NULL) {
        return NULL;
    }

    return ret;
}

void mqtt_free( void *pv )
{
    os_free(pv);
}

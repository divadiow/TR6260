#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif
#include "FreeRTOS.h"

static size_t mbedtls_heap_peak = 0;
static size_t mbedtls_heap_current = 0;
//extern void *pvPortCalloc( size_t nmemb, size_t size );

void *trs_mbedtls_calloc_func(size_t nmemb, size_t size)
{
    char *ret;
    ret = (char *)pvPortCalloc(nmemb, size);
    if (ret == NULL) {
        return NULL;
    }

    mbedtls_heap_current += nmemb * size;
    if (mbedtls_heap_current > mbedtls_heap_peak) {
        mbedtls_heap_peak = mbedtls_heap_current;
    }
    return ret;
}

void trs_mbedtls_free_func( void *pv )
{
    size_t before_size, after_size;
    before_size = xPortGetFreeHeapSize();
    vPortFree(pv);
    after_size = xPortGetFreeHeapSize();
    mbedtls_heap_current -= (after_size - before_size);
}

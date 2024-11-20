/*
 * wpa_supplicant/hostapd / Empty OS specific functions
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This file can be used as a starting point when adding a new OS target. The
 * functions here do not really work as-is since they are just empty or only
 * return an error value. os_internal.c can be used as another starting point
 * or reference since it has example implementation of many of these functions.
 */

#include "includes.h"
#include "os.h"
#include <FreeRTOS.h>
#include <timers.h>
#include "system.h"

void os_sleep(os_time_t sec, os_time_t usec)
{
	vTaskDelay(pdMS_TO_TICKS(sec * 1000 + (usec+1) / 1000));
}

int os_get_time(struct os_time *t)
{
    int unit = 1000/configTICK_RATE_HZ;
	TickType_t cur = xTaskGetTickCount()*unit;
	t->sec = (cur + 1) / (unit*configTICK_RATE_HZ);
	t->usec = (cur % unit*configTICK_RATE_HZ)*1000;

	return 0;
}

int os_get_reltime(struct os_reltime *t)
{
	// TODO:
	struct os_time cur;
	os_get_time(&cur);
	t->sec = cur.sec;
	t->usec = cur.usec;
	return 0;
}

int os_mktime(int year, int month, int day, int hour, int min, int sec,
	      os_time_t *t)
{
#if 0
	struct tm tm;

	if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31 ||
		hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 ||
		sec > 60)
		return -1;

	os_memset(&tm, 0, sizeof(tm));
	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;

	*t = (os_time_t) mktime(&tm);
#endif
	return -1;
}

int os_gmtime(os_time_t t, struct os_tm *tm)
{
#if 0
	struct tm *tm2;
	time_t t2 = t;

	tm2 = gmtime(&t2);
	if (tm2 == NULL)
		return -1;
	tm->sec = tm2->tm_sec;
	truct tm tm;m->min = tm2->tm_min;
	tm->hour = tm2->tm_hour;
	tm->day = tm2->tm_mday;
	tm->month = tm2->tm_mon + 1;
	tm->year = tm2->tm_year + 1900;
#endif
	return -1;
}

int os_daemonize(const char *pid_file)
{
	return -1;
}

void os_daemonize_terminate(const char *pid_file)
{
}

int os_get_random(unsigned char *buf, size_t len)
{
	int i;
	for (i = 0 ;i < len; i++)
		buf[i] = os_random();
	return 0;
}

unsigned long os_random(void)
{
#if defined (NRC7291_SDK_DUAL_CM0) || defined (NRC7291_SDK_DUAL_CM3)
	return 0;
#else
	return system_modem_api_get_tsf_timer_low(0);
#endif
}

char * os_rel2abs_path(const char *rel_path)
{
	return NULL; /* strdup(rel_path) can be used here */
}

int os_program_init(void)
{
	return 0;
}

void os_parogram_deinit(void)
{
}

int os_setenv(const char *name, const char *value, int overwrite)
{
	return -1;
}

int os_unsetenv(const char *name)
{
	return -1;
}

char * os_readfile(const char *name, size_t *len)
{
	return NULL;
}

int os_fdatasync(FILE *stream)
{
	return 0;
}

#ifdef OS_NO_C_LIB_DEFINES

void * os_memcpy(void *dest, const void *src, size_t n)
{
	return memcpy(dest, src, n);
}

void * os_memmove(void *dest, const void *src, size_t n)
{
	return memmove(dest, src, n);
}

void * os_memset(void *s, int c, size_t n)
{
	return memset(s, c, n);
}

int my_memcmp(const void *s1, const void *s2, size_t n)
{
        const unsigned char *p1 = s1, *p2 = s2;

        if (n == 0)
                return 0;

        while (*p1 == *p2) {
                p1++;
                p2++;
                n--;
                if (n == 0)
                        return 0;
        }

        return *p1 - *p2;
}
int os_memcmp(const void *s1, const void *s2, size_t n)
{
	return my_memcmp(s1, s2, n);
}

size_t os_strlen(const char *s)
{
	return strlen(s);
}

int os_strcasecmp(const char *s1, const char *s2)
{
	/*
	 * Ignoring case is not required for main functionality, so just use
	 * the case sensitive version of the function.
	 */
	return os_strcmp(s1, s2);
}

int os_strncasecmp(const char *s1, const char *s2, size_t n)
{
	/*
	 * Ignoring case is not required for main functionality, so just use
	 * the case sensitive version of the function.
	 */
	return os_strncmp(s1, s2, n);
}

char * os_strchr(const char *s, int c)
{
	return strchr(s, c);
}

char * os_strrchr(const char *s, int c)
{
	return strrchr(s, c);
}

int os_strcmp(const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}

int os_strncmp(const char *s1, const char *s2, size_t n)
{
	return strncmp(s1, s2, n);
}

char * os_strncpy(char *dest, const char *src, size_t n)
{
	char *d = dest;

	while (n--) {
		*d = *src;
		if (*src == '\0')
			break;
		d++;
		src++;
	}

	return dest;
}

size_t os_strlcpy(char *dest, const char *src, size_t size)
{
	return strlcpy(dest, src, size);
}

int os_memcmp_const(const void *a, const void *b, size_t len)
{
	return memcmp(a, b, len);
}

char * os_strstr(const char *haystack, const char *needle)
{
	return strstr(haystack, needle);
}

#endif /* OS_NO_C_LIB_DEFINES */

int os_exec(const char *program, const char *arg, int wait_completion)
{
	return -1;
}

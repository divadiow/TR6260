/*
* Copyright (c) 2001-2003 Swedish Institute of Computer Science.
* Modifications Copyright (c) 2006 Christian Walter <wolti@sil.at>
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
* Author: Adam Dunkels <adam@sics.se>
* Modifcations: Christian Walter <wolti@sil.at>
*
* $Id: sys_arch.c,v 1.6 2006/09/24 22:04:53 wolti Exp $
*/

/* ------------------------ System includes ------------------------------- */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/* ------------------------ FreeRTOS includes ----------------------------- */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* ------------------------ lwIP includes --------------------------------- */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/sio.h"
#include "lwip/stats.h"
#include "lwip/timeouts.h"
#include "pthread_internal.h"

/* ------------------------ Defines --------------------------------------- */
#define MS_TO_TICKS( ms )         ( TickType_t )( ( TickType_t ) ( ms ) / portTICK_PERIOD_MS )
#define TICKS_TO_MS( ticks )   ( unsigned long )( ( TickType_t ) ( ticks ) * portTICK_PERIOD_MS )
#define MAX_FREE_POLL_CNT	50
#define RETRY_FREE_POLL_DELAY 10

/* --------------------------- Variables ---------------------------------- */
static sys_mutex_t g_lwip_mutex = NULL;
static pthread_key_t sys_thread_sem_key;
/* ------------------------ External functions ------------------------------ */

/* ------------------------ Start implementation -------------------------- */
static void sys_thread_sem_free(void* data);
void sys_init( void )
{
    if (ERR_OK != sys_mutex_new(&g_lwip_mutex)) {
		// Create a new mutex fail
    }

    // Create the pthreads key for the per-thread semaphore storage
    pthread_key_create(&sys_thread_sem_key, sys_thread_sem_free);
}

/*
* This optional function does a "fast" critical region protection and returns
* the previous protection level. This function is only called during very short
* critical regions. An embedded system which supports ISR-based drivers might
* want to implement this function by disabling interrupts. Task-based systems
* might want to implement this by using a mutex or disabling tasking. This
* function should support recursive calls from the same task or interrupt. In
* other words, sys_arch_protect() could be called while already protected. In
* that case the return value indicates that it is already protected.
*
* sys_arch_protect() is only required if your port is supporting an operating
* system.
*/
sys_prot_t sys_arch_protect( void )
{
	sys_mutex_lock(&g_lwip_mutex);
	return true;
}

/*
* This optional function does a "fast" set of critical region protection to the
* value specified by pval. See the documentation for sys_arch_protect() for
* more information. This function is only required if your port is supporting
* an operating system.
*/
void sys_arch_unprotect( sys_prot_t pval )
{
	sys_mutex_unlock(&g_lwip_mutex);
}

/* ------------------------ Start implementation ( Threads ) -------------- */

/*
* Starts a new thread named "name" with priority "prio" that will begin its
* execution in the function "thread()". The "arg" argument will be passed as an
* argument to the thread() function. The stack size to used for this thread is
* the "stacksize" parameter. The id of the new thread is returned. Both the id
* and the priority are system dependent.
*/
sys_thread_t
sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
	xTaskHandle CreatedTask;
	portBASE_TYPE result;

	result = xTaskCreate(thread, name, stacksize, arg, prio, &CreatedTask);

	if (result == pdTRUE) {
		return CreatedTask;
	} else {
		return NULL;
	}
}

/* ------------------------ Start implementation ( Semaphores ) ----------- */

/* Creates and returns a new semaphore. The "count" argument specifies
* the initial state of the semaphore.
*/
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	err_t xReturn = ERR_MEM;
	vSemaphoreCreateBinary(*sem);

	if ((*sem) != NULL) {
		if (count == 0) {
			xSemaphoreTake(*sem, 1);
		}

		xReturn = ERR_OK;
	} else {
		;
	}

	return xReturn;
}

/* Deallocates a semaphore */
void sys_sem_free(sys_sem_t *sem)
{
	SYS_STATS_DEC(sem.used);
	vSemaphoreDelete(*sem);
	//vQueueDelete( sem );
}

/* Signals a semaphore */
void sys_sem_signal(sys_sem_t *sem)
{
	LWIP_ASSERT( "sys_sem_signal: sem != SYS_SEM_NULL", sem != SYS_SEM_NULL );
	if (errQUEUE_FULL == xSemaphoreGive( *sem )) {
        system_printf("----queue full! %p\n", sem);
    }
}

/*
* Blocks the thread while waiting for the semaphore to be
* signaled. If the "timeout" argument is non-zero, the thread should
* only be blocked for the specified time (measured in
* milliseconds).
*
* If the timeout argument is non-zero, the return value is the number of
* milliseconds spent waiting for the semaphore to be signaled. If the
* semaphore wasn't signaled within the specified time, the return value is
* SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
* (i.e., it was already signaled), the function may return zero.
*
* Notice that lwIP implements a function with a similar name,
* sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	portBASE_TYPE   xStatus;
	TickType_t    xTicksStart, xTicksEnd, xTicksElapsed;
	u32_t           timespent;
//	vPrintString("sys_arch_sem_wait\n");
	LWIP_ASSERT( "sys_arch_sem_wait: sem != SYS_SEM_NULL", sem != SYS_SEM_NULL );
	xTicksStart = xTaskGetTickCount(  );

	if( timeout == 0 ) {
		do {
		xStatus = xSemaphoreTake( *sem, MS_TO_TICKS( 100 ) );
		} while( xStatus != pdTRUE );
	}else {
		xStatus = xSemaphoreTake( *sem, MS_TO_TICKS( timeout ) );
	}

	/* Semaphore was signaled. */
	if( xStatus == pdTRUE ) {
		xTicksEnd = xTaskGetTickCount(  );
		xTicksElapsed = xTicksEnd - xTicksStart;
		timespent = TICKS_TO_MS( xTicksElapsed );
	}else {
		timespent = SYS_ARCH_TIMEOUT;
	}
	return timespent;
}

err_t sys_mutex_trylock(sys_mutex_t *pxMutex)
{
	if (xSemaphoreTake(*pxMutex, 0) == pdTRUE)
		return ERR_OK;
	else
		return ERR_TIMEOUT;
}
/* ------------------------ Start implementation ( Mailboxes ) ------------ */

/*
Creates an empty mailbox.
*/
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	*mbox = mem_malloc(sizeof(struct sys_mbox));
	if (*mbox == NULL){
		return ERR_MEM;
	}

	(*mbox)->queue_handle = xQueueCreate(size, sizeof(void *));

	if ((*mbox)->queue_handle == NULL) {
		mem_free(*mbox);
		return ERR_MEM;
	}

	if (sys_mutex_new(&((*mbox)->lock)) != ERR_OK){
		vQueueDelete((*mbox)->queue_handle);
		mem_free(*mbox);
		return ERR_MEM;
	}
	(*mbox)->working = true;

	return ERR_OK;
}

void sys_delay_ms(uint32_t ms)
{
	vTaskDelay(ms / portTICK_PERIOD_MS);
}

/*
Deallocates a mailbox. If there are messages still present in the
mailbox when the mailbox is deallocated, it is an indication of a
programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
	uint16_t count = 0;
	bool post_null = true;

	(*mbox)->working = false;

	while ( count++ < MAX_FREE_POLL_CNT ){
		if (sys_mutex_trylock( &(*mbox)->lock )== ERR_OK){
			sys_mutex_unlock( &(*mbox)->lock );
			break;
		}

		if (post_null){
			if (sys_mbox_trypost( mbox, NULL) != ERR_OK){

			} else {
				post_null = false;
			}
		}
		sys_delay_ms(RETRY_FREE_POLL_DELAY);
	}

	if (uxQueueMessagesWaiting((*mbox)->queue_handle)) {
		xQueueReset((*mbox)->queue_handle);
		__asm__ volatile ("nop");
	}

	vQueueDelete((*mbox)->queue_handle);
	sys_mutex_free(&(*mbox)->lock);
	mem_free(*mbox);
	*mbox = NULL;
}

/*
* This function sends a message to a mailbox. It is unusual in that no error
* return is made. This is because the caller is responsible for ensuring that
* the mailbox queue will not fail. The caller does this by limiting the number
* of msg structures which exist for a given mailbox.
*/
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
	while (xQueueSendToBack((*mbox)->queue_handle, &msg, portMAX_DELAY) != pdTRUE);
}

/*
* Try to post the "msg" to the mailbox. Returns ERR_MEM if this one is full,
* else, ERR_OK if the "msg" is posted.
*/
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	err_t xReturn;

	if (xQueueSend((*mbox)->queue_handle, &msg, (portTickType)0) == pdTRUE) {
		xReturn = ERR_OK;
	} else {
		/* The queue was already full. */
		xReturn = ERR_MEM;
	}

	return xReturn;
}

/*
* Blocks the thread until a message arrives in the mailbox, but does
* not block the thread longer than "timeout" milliseconds (similar to
* the sys_arch_sem_wait() function). The "msg" argument is a result
* parameter that is set by the function (i.e., by doing "*msg =
* ptr"). The "msg" parameter maybe NULL to indicate that the message
* should be dropped.
*
* Note that a function with a similar name, sys_mbox_fetch(), is
* implemented by lwIP.
*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	void *dummyptr;
	portTickType StartTime, EndTime, Elapsed;
	unsigned long ulReturn;

	StartTime = xTaskGetTickCount();

	if (msg == NULL) {
		msg = &dummyptr;
	}

	if (*mbox == NULL){
		*msg = NULL;
		return -1;
	}

	sys_mutex_lock(&(*mbox)->lock);

	if (timeout != 0) {
		if (pdTRUE == xQueueReceive((*mbox)->queue_handle, &(*msg), timeout / portTICK_RATE_MS)) {
			EndTime = xTaskGetTickCount();
			Elapsed = (EndTime - StartTime) * portTICK_RATE_MS;

			if (Elapsed == 0) {
				Elapsed = 1;
			}
			ulReturn = Elapsed;
		} else {
			*msg = NULL;
			ulReturn = SYS_ARCH_TIMEOUT;
		}
	} else {
		while (1){
			if (pdTRUE == xQueueReceive((*mbox)->queue_handle, &(*msg), portMAX_DELAY)){
				break;
			}

			if ((*mbox)->working == false){
				*msg = NULL;
				break;
			}
		}

		EndTime = xTaskGetTickCount();
		Elapsed = (EndTime - StartTime) * portTICK_RATE_MS;

		if (Elapsed == 0) {
			Elapsed = 1;
		}
		ulReturn = Elapsed;
	}
	sys_mutex_unlock(&(*mbox)->lock);

	return ulReturn;
}

/*
* This is similar to sys_arch_mbox_fetch, however if a message is not present
* in the mailbox, it immediately returns with the code SYS_MBOX_EMPTY
* On success 0 is returned.
*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	void *pvDummy;
	unsigned long ulReturn;

	if (msg == NULL) {
		msg = &pvDummy;
	}

	if (xQueueReceive((*mbox)->queue_handle, &(*msg), 0UL)== pdTRUE) {
		ulReturn = ERR_OK;
	} else {
		ulReturn = SYS_MBOX_EMPTY;
	}

	return ulReturn;
}

/** Returns the current time in milliseconds. */
u32_t sys_now( void )
{
	TickType_t    xTicks = xTaskGetTickCount(  );
	return ( u32_t )TICKS_TO_MS( xTicks );
}

/** Create a new mutex
* @param mutex pointer to the mutex to create
* @return a new mutex */
err_t sys_mutex_new( sys_mutex_t *mutex )
{
	err_t xReturn = ERR_MEM;

	*mutex = xSemaphoreCreateMutex();

	if( *mutex != NULL ) {
		xReturn = ERR_OK;
		SYS_STATS_INC_USED(mutex);
	} else {
		xReturn = ERR_MEM;
		SYS_STATS_INC(mutex.err);
		LWIP_ASSERT( "sys_sem_new: xSemaphore == SYS_SEM_NULL", *mutex == NULL );
	}

	return xReturn;
}

/** Lock a mutex
* @param mutex the mutex to lock */
void sys_mutex_lock( sys_mutex_t *mutex )
{
	while( xSemaphoreTake( *mutex, portMAX_DELAY ) != pdTRUE );
}

/** Unlock a mutex
* @param mutex the mutex to unlock */
void sys_mutex_unlock(sys_mutex_t *mutex )
{
	xSemaphoreGive( *mutex );
}

/** Delete a semaphore
* @param mutex the mutex to delete */
void sys_mutex_free( sys_mutex_t *mutex )
{
	SYS_STATS_DEC(mutex.used);
	vQueueDelete( *mutex );
}

/** Check if a mutex is valid/allocated: return 1 for valid, 0 for invalid */
int sys_mutex_valid(sys_mutex_t *mutex)
{
	return (int)(*mutex);
}

/** Set a mutex invalid so that sys_mutex_valid returns 0 */
void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
	*mutex = NULL;
}

/** Set a dealy */
void sys_arch_msleep(int ms)
{
	vTaskDelay(ms / portTICK_RATE_MS);
}

/*
 * get per thread semphore
 */
sys_sem_t* sys_thread_sem_get(void)
{
  sys_sem_t *sem = pthread_getspecific(sys_thread_sem_key);

  if (!sem) {
      sem = sys_thread_sem_init();
  }
  
  LWIP_ASSERT( "sys_thread_sem_get: sem != NULL", sem != NULL );
  return sem;
}

static void sys_thread_sem_free(void* data) // destructor for TLS semaphore
{
  sys_sem_t *sem = (sys_sem_t*)(data);

  if (sem && *sem){
    vSemaphoreDelete(*sem);
  }

  if (sem) {
    mem_free(sem);
  }
}

sys_sem_t* sys_thread_sem_init(void)
{
  sys_sem_t *sem = (sys_sem_t*)mem_malloc(sizeof(sys_sem_t*));

  if (!sem){
    LWIP_ASSERT( "sys_thread_sem_init: sem != NULL", sem != NULL );
    return 0;
  }

  *sem = xSemaphoreCreateBinary();
  if (!(*sem)){
    mem_free(sem);
    LWIP_ASSERT( "sys_thread_sem_init1: *sem != NULL", 0);
    return 0;
  }

  pthread_setspecific(sys_thread_sem_key, sem);
  return sem;
}

void sys_thread_sem_deinit(void)
{
  sys_sem_t *sem = pthread_getspecific(sys_thread_sem_key);
  if (sem != NULL) {
    sys_thread_sem_free(sem);
    pthread_setspecific(sys_thread_sem_key, NULL);
  }
}

uint32_t sys_random(void)
{
    extern uint32_t trng_read(void);
    return trng_read();
}



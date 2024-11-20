/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: standalone.c 
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangchao
 * Date:          2019-1-7
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

/****************************************************************************
* 	                                           Include files
****************************************************************************/
#include "system.h"
#include "standalone.h"
#include "system_lwip.h"
#include "system_event.h"
#include "system_wifi.h"
#include "system_priority.h"
#include "amtNV.h"

/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define WPA_SUPPLICANT_STACK_SIZE (10000)

/****************************************************************************
* 	                                           Local Types
****************************************************************************/

/****************************************************************************
* 	                                           Local Constants
****************************************************************************/

/****************************************************************************
* 	                                           Local Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Global Constants
****************************************************************************/

/****************************************************************************
* 	                                          Global Variables
****************************************************************************/
#if 0
static const char* STRING_TASK_CREATED				= "%s task is created \n";
static const char* STRING_FAILED_TO_CREATE_TASK 	= "Failed to create %s task \n";
#endif
TaskHandle_t   wpa_task_handle;
StackType_t	   wpa_stack[WPA_SUPPLICANT_STACK_SIZE/sizeof(StackType_t)];
StaticTask_t   wpa_tcb;

/****************************************************************************
* 	                                          Global Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/
#if 0
#define DECLARE_TASK(NAME, STACK_SIZE) 								\
	struct NAME##_Task {											\
	StackType_t		stack[STACK_SIZE / sizeof(StackType_t)];		\
	StaticTask_t	tcb;											\
	TaskHandle_t	handle;											\
	};																\
	static struct NAME##_Task s_##NAME##_task;

#define CREATE_TASK(NAME, PRIO, FN)									\
	s_##NAME##_task.handle = xTaskCreateStatic(						\
				FN,#NAME, sizeof(s_##NAME##_task.stack) / sizeof(StackType_t), \
				NULL, PRIO, &s_##NAME##_task.stack[0],				\
				&s_##NAME##_task.tcb);								\
	if (s_##NAME##_task.handle)										\
		system_printf(STRING_TASK_CREATED, #NAME);					\
	else															\
		system_printf(STRING_FAILED_TO_CREATE_TASK, #NAME);

DECLARE_TASK(wpa_supplicant, WPA_SUPPLICANT_STACK_SIZE);
#endif


//extern void wpas_task_main(void *pvParams);
//extern void net80211_newracom_preinit();

/*******************************************************************************
 * Function: 
 * Description: 
 * Parameters: 
 *   Input:
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/

static uint8_t hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static sys_err_t system_event_sta_connected_call(void *param)
{
    wifi_handle_sta_connect_event(param);

    return SYS_OK;
}

/*******************************************************************************
 * Function: standalone_main
 * Description: 
 * Parameters: 
 *   Input:
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/	
int standalone_main()
{
	wpa_task_handle = xTaskCreateStatic(
				wpas_task_main,
				"wpa_supplicant",
				WPA_SUPPLICANT_STACK_SIZE/sizeof(StackType_t),
				NULL,
				THREAD_WPA_SUPPLICANT_PRI/*NRC_TASK_PRIORITY*/,
				&wpa_stack[0],
				&wpa_tcb);
	
	if (wpa_task_handle) 
		system_printf("[%s, %d] task creation succeed!(0x%x)\n", __func__, __LINE__, wpa_task_handle);	
	else
		system_printf("[%s, %d] task creation failed!(0x%x)\n", __func__, __LINE__);

    wifi_system_init();
#if defined(STACK_LWIP)
	wifi_lwip_init();
#endif
    sys_event_loop_init(NULL, NULL);
    sys_event_set_default_wifi_handlers();

	return 0;
}


/*******************************************************************************
 * Function: monitor_task_main
 * Description: 
 * Parameters: 
 *   Input:
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
#ifdef MONITOR
void monitor_task_main(void *pvParameters)
{
extern void monitor_rtc_print();

	for ( ;; ) {
		/* Print out the name of this task. */
		monitor_rtc_print();
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
#endif
/*******************************************************************************
 * Function: 
 * Description: 
 * Parameters: 
 *   Input:
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	   free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	   internally by FreeRTOS API functions that create tasks, queues, software
	   timers, and semaphores.  The size of the FreeRTOS heap is set by the
	   configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}





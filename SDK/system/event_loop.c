/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:    event_loop.c
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       V1.0
 * Author:        lixiao
 * Date:          2018-12-21
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
#include "system_event.h"
#include "driver_wifi.h"

/****************************************************************************
* 	                                           Local Macros
****************************************************************************/

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

/****************************************************************************
* 	                                          Global Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/
#define  DHCP_REQUEST_TIMEOUT_MS             (5000)

static void *s_event_queue = NULL;
static system_event_cb_t s_event_handler_cb = NULL;
static void *s_event_ctx = NULL;
static unsigned int dhcp_start_timems = 0;

static sys_err_t sys_event_post_to_user(system_event_t *event)
{
    if (s_event_handler_cb) {
        return (*s_event_handler_cb)(s_event_ctx, event);
    }
    return SYS_OK;
}

static void sys_event_loop_task(void *pvParameters)
{
    while (1) {
        system_event_t evt;
        if (xQueueReceive(s_event_queue, &evt, pdMS_TO_TICKS(250)) == pdPASS) {
            sys_err_t ret = sys_event_process_default(&evt);
            if (ret != SYS_OK) {
                SYS_LOGE("default event handler failed!");
            }
            ret = sys_event_post_to_user(&evt);
            if (ret != SYS_OK) {
                SYS_LOGE("post event to user fail!");
            }

			if(SYSTEM_EVENT_STA_CONNECTED == evt.event_id)
			{
				dhcp_start_timems = xTaskGetTickCount() * portTICK_PERIOD_MS;
			}
			else if(SYSTEM_EVENT_STA_GOT_IP == evt.event_id || SYSTEM_EVENT_STA_DISCONNECTED == evt.event_id)
			{
				dhcp_start_timems = 0;
			}
        }

		if(dhcp_start_timems != 0 && (xTaskGetTickCount() * portTICK_PERIOD_MS  > DHCP_REQUEST_TIMEOUT_MS + dhcp_start_timems))
		{
			system_printf("dhcp timeout!!! start:%d %d\r\n" ,dhcp_start_timems, xTaskGetTickCount() * portTICK_PERIOD_MS );
			dhcp_start_timems = 0;
			nrc_wpas_disconnect(0);
		}
    }
}

system_event_cb_t sys_event_loop_set_cb(system_event_cb_t cb, void *ctx)
{
    system_event_cb_t old_cb = s_event_handler_cb;
    s_event_handler_cb = cb;
    s_event_ctx = ctx;
    return old_cb;
}

sys_err_t sys_event_send(system_event_t *event)
{
    if (s_event_queue == NULL) {
        SYS_LOGE("Event loop not initialized via sys_event_loop_init, but sys_event_send called");
        return SYS_ERR_INVALID_STATE;
    }

    int ret = xQueueSendToBack(s_event_queue, event, 0);
    if (ret != true) {
        if (event) {
            SYS_LOGE("e=%d f", event->event_id);
        } else {
            SYS_LOGE("e null");
        }
        return SYS_ERR;
    }
    return SYS_OK;
}

QueueHandle_t sys_event_loop_get_queue(void)
{
    return s_event_queue;
}

sys_err_t sys_event_loop_init(system_event_cb_t cb, void *ctx)
{
    static bool s_event_init_flag = false;
    TaskHandle_t handle;

    if (s_event_init_flag) {
        return SYS_ERR;
    }

    s_event_queue = xQueueCreate(32, sizeof(system_event_t));
    if(s_event_queue == NULL)
        return SYS_ERR_NO_MEM;
	
    if(xTaskCreate(sys_event_loop_task, "sys_event_loop_task", 1024, NULL, 
                      SYSTEM_EVENT_LOOP_PRIORITY, &handle) != pdPASS) {
        SYS_LOGE("create event task failed.");
        return SYS_ERR_NO_MEM;
    }
    
    s_event_handler_cb = cb;
    s_event_ctx = ctx;
    s_event_init_flag = true;
    
    return SYS_OK;
}



/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:  system_priority.h  
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2019-3-19
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef _SYSTEM_PRI_H
#define _SYSTEM_PRI_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
****************************************************************************/
#define THREAD_SYSTEM_PRI					4
#define THREAD_IDLE_PRI						0
#define THREAD_IPERF_PRI					3
#define THREAD_PING_PRI						3
#define THREAD_TIMER_PRI					3
#define THREAD_WPA_SUPPLICANT_PRI			9
#define THREAD_TCP_IP_PRI					7
#define THREAD_LMAC_PRI						12
#define THREAD_EVENT_LOOP_PRI				10
#define THREAD_PSM_PRI						12
#define THREAD_MQTT_PRI						3
#define THREAD_MACHTALK_PRI					3
#define THREAD_WTD_PRI                      3
#define THREAD_WTDSOFT_PRI                  2


#define THREAD_COMMON_TASK_PRI				8


/****************************************************************************
* 	                                        Types
****************************************************************************/

/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/



#endif/*_SYSTEM_PRI_H*/


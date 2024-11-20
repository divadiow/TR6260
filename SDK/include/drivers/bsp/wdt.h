/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:    wdt.h
 * File Mark:    
 * Description:  watchdog timer
 * Others:        
 * Version:       V1.0
 * Author:        wangc
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
#include "soc_top_reg.h"

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

/*******************************************************************************
 * Function:     wdt_init
 * Description:  watch dog initialize
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
void wdt_init(void);

/*******************************************************************************
 * Function:     wdt_config
 * Description:  configure watch dog
 * Parameters: 
 *   Input:   intr_period -- interrupt timer period
 *               rst_period  -- reset interval
 *               intr_enalbe  --  enable or disable the watch dog interrupt
 *               rst_enalbe  --  enable or disable the watch dog reset
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
#define WTD_CTRL_INTEN_DIS		0
#define WTD_CTRL_INTEN_EN		1

#define WTD_CTRL_RSTEN_DIS		0
#define WTD_CTRL_RSTEN_EN		1

#define WTD_CTRL_INTTIME_0		0
#define WTD_CTRL_INTTIME_1		1
#define WTD_CTRL_INTTIME_2		2
#define WTD_CTRL_INTTIME_3		3
#define WTD_CTRL_INTTIME_4		4
#define WTD_CTRL_INTTIME_5		5
#define WTD_CTRL_INTTIME_6		6
#define WTD_CTRL_INTTIME_7		7
#define WTD_CTRL_INTTIME_8		8
#define WTD_CTRL_INTTIME_9		9
#define WTD_CTRL_INTTIME_10		10
#define WTD_CTRL_INTTIME_11		11
#define WTD_CTRL_INTTIME_12		12
#define WTD_CTRL_INTTIME_13		13
#define WTD_CTRL_INTTIME_14		14
#define WTD_CTRL_INTTIME_15		15

#define WTD_CTRL_RSTTIME_0		0
#define WTD_CTRL_RSTTIME_1		1
#define WTD_CTRL_RSTTIME_2		2
#define WTD_CTRL_RSTTIME_3		3
#define WTD_CTRL_RSTTIME_4		4
#define WTD_CTRL_RSTTIME_5		5
#define WTD_CTRL_RSTTIME_6		6
#define WTD_CTRL_RSTTIME_7		7
#define WDT_INTR_PERIOD_0		WTD_CTRL_INTTIME_0		// 1/512 sec
#define WDT_INTR_PERIOD_1		WTD_CTRL_INTTIME_1		// 1/128 sec
#define WDT_INTR_PERIOD_2		WTD_CTRL_INTTIME_2		// 1/32 sec
#define WDT_INTR_PERIOD_3		WTD_CTRL_INTTIME_3		// 1/16 sec
#define WDT_INTR_PERIOD_4		WTD_CTRL_INTTIME_4		// 1/8 sec
#define WDT_INTR_PERIOD_5		WTD_CTRL_INTTIME_5		// 1/4 sec
#define WDT_INTR_PERIOD_6		WTD_CTRL_INTTIME_6		// 1/2 sec
#define WDT_INTR_PERIOD_7		WTD_CTRL_INTTIME_7		// 1 sec
#define WDT_INTR_PERIOD_8		WTD_CTRL_INTTIME_8		// 4 sec
#define WDT_INTR_PERIOD_9		WTD_CTRL_INTTIME_9		// 16 sec
#define WDT_INTR_PERIOD_10	WTD_CTRL_INTTIME_10	// 64 sec
#define WDT_INTR_PERIOD_11	WTD_CTRL_INTTIME_11	// 4min 16 sec
#define WDT_INTR_PERIOD_12	WTD_CTRL_INTTIME_12	// 17min 4sec
#define WDT_INTR_PERIOD_13	WTD_CTRL_INTTIME_13	// 1hour 8min 16sec
#define WDT_INTR_PERIOD_14	WTD_CTRL_INTTIME_14	// 4hour 33min 4sec
#define WDT_INTR_PERIOD_15	WTD_CTRL_INTTIME_15	// 18hour 12min 16sec

#define WDT_RST_TIME_0			WTD_CTRL_RSTTIME_0		// 1/256 sec
#define WDT_RST_TIME_1			WTD_CTRL_RSTTIME_1		// 1/128 sec
#define WDT_RST_TIME_2			WTD_CTRL_RSTTIME_2		// 1/64 sec
#define WDT_RST_TIME_3			WTD_CTRL_RSTTIME_3		// 1/32 sec
#define WDT_RST_TIME_4			WTD_CTRL_RSTTIME_4		// 1/16 sec
#define WDT_RST_TIME_5			WTD_CTRL_RSTTIME_5		// 1/8 sec
#define WDT_RST_TIME_6			WTD_CTRL_RSTTIME_6		// 1/4 sec
#define WDT_RST_TIME_7			WTD_CTRL_RSTTIME_7		// 1/2 sec

#define WDT_INTR_ENABLE		WTD_CTRL_INTEN_EN
#define WDT_INTR_DISABLE		WTD_CTRL_INTEN_DIS

#define WDT_RST_ENABLE			WTD_CTRL_RSTEN_EN
#define WDT_RST_DISABLE		WTD_CTRL_RSTEN_DIS

void wdt_config(int intr_period, int rst_period, int intr_enalbe, int rst_enalbe);

/*******************************************************************************
 * Function:     wdt_isr_register
 * Description:  register the interrupt callback function
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
void wdt_isr_register(void (*func)(void));

/*******************************************************************************
 * Function:     wdt_restart
 * Description:  watchdog restart to avoid the watchdog interrupt/reset
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
void wdt_restart(void);

/*******************************************************************************
 * Function:     wdt_start
 * Description:  watchdog setup and enable
 * Parameters: 
 *   Input:  type -- WDT_RESET_CHIP_TYPE_RESYSTEM, reset from system
 *                         WDT_RESET_CHIP_TYPE_REBOOT, reset from boot
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
#define WDT_RESET_CHIP_TYPE_REBOOT		0
#define WDT_RESET_CHIP_TYPE_RESYSTEM		1

void wdt_start(int type);
void wdt_stop(void);

/*******************************************************************************
 * Function:     wdt_reset_chip
 * Description:  watchdog reset the CHIP, and clk is from 32K
 * Parameters: 
 *   Input:  type -- WDT_RESET_CHIP_TYPE_RESYSTEM, reset from system
 *                         WDT_RESET_CHIP_TYPE_REBOOT, reset from boot
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
void wdt_reset_chip(int type);
void Hal_WtdTask_Init(void);
void wtd_task_init();
void wtd_task(void *pvParameters) ;
void wtd_rst_send(void);
void wtd_resume(void);
int  wtd_gstatus(void);
void wtd_softtask_init();
void wtd_softtask(void *pvParameters) ;
void Hal_WtdSoftTask_Init(void);
void wtd_set_stop(void);











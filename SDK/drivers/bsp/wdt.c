/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:    wdt.c
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
#include "wdt.h"
#include "irq.h"
#include "system_common.h"
#include "system_priority.h"



/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define REG32(reg)			(  *( (volatile unsigned int *) (reg) ) )

#define WTD_WP_NUM		0x5aa5
#define WTD_RESET_NUM		0xcafe


#define WTD_CTRL_REG		SOC_WDT_BASE+0x10
#define WTD_RESTART_REG	SOC_WDT_BASE+0x14
#define WTD_ENABLE_REG		SOC_WDT_BASE+0x18
#define WTD_STATUS_REG	SOC_WDT_BASE+0x1C


#define WTD_CTRL_EN(x)				((x)<<0)
#define WTD_CTRL_CLKSEL(x)			((x)<<1)
#define WTD_CTRL_INTEN(x)			((x)<<2)
#define WTD_CTRL_RSTEN(x)			((x)<<3)
#define WTD_CTRL_INTTIME(x)		((x)<<4)
#define WTD_CTRL_RSTTIME(x)		((x)<<8)


#define WTD_CTRL_TIMER_DISABLE	0
#define WTD_CTRL_TIMER_ENABLE		1

#define WTD_CTRL_CLKSEL_EXTCLK			0
#define WTD_CTRL_CLKSEL_PCLK				1

/****************************************************************************
* 	                                           Local Types
****************************************************************************/

typedef  struct WDT_DEV_T
{
	char  active;
	unsigned int ctrlRegValue;
	void (*callback)(void);

} WDT_DEV;

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

static WDT_DEV sWdtDev;

/****************************************************************************
* 	                                          Global Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/

/*******************************************************************************
 * Function:     wdt_isr
 * Description:  watchdog interrupt service routine
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
static void wdt_isr(void)
{
	REG32(WTD_STATUS_REG) = 1;

	if(sWdtDev.callback)
		sWdtDev.callback();

	//REG32(WTD_ENABLE_REG) = WTD_WP_NUM;
	//REG32(WTD_RESTART_REG) = WTD_RESET_NUM;
}

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
void wdt_init(void)
{
	CLK_ENABLE(CLK_WCT);
	irq_isr_register(IRQ_VECTOR_WDT, wdt_isr);
}

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
void wdt_config(int intr_period, int rst_period, int intr_enalbe, int rst_enalbe)
{
	sWdtDev.ctrlRegValue = WTD_CTRL_CLKSEL(WTD_CTRL_CLKSEL_EXTCLK)
					|WTD_CTRL_EN(WTD_CTRL_TIMER_ENABLE)
					|WTD_CTRL_INTEN(intr_enalbe)
					|WTD_CTRL_RSTEN(rst_enalbe)
					|WTD_CTRL_INTTIME(intr_period)
					|WTD_CTRL_RSTTIME(rst_period);
}

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
void wdt_isr_register(void (*func)(void))
{
	sWdtDev.callback = func;
}

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
void wdt_restart(void)
{
	REG32(WTD_ENABLE_REG) = WTD_WP_NUM;
	REG32(WTD_RESTART_REG) = WTD_RESET_NUM;
}


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
void wdt_start(int type)
{	
	if(type)
		REG32(SOC_AON_WAKEUP_ADD) = WDT_RESET_CHIP_TYPE_RESYSTEM;
	else
		REG32(SOC_AON_WAKEUP_ADD) = WDT_RESET_CHIP_TYPE_REBOOT;

	REG32(WTD_STATUS_REG) = 1;
	irq_unmask(IRQ_VECTOR_WDT);

	REG32(WTD_ENABLE_REG) = WTD_WP_NUM;
	REG32(WTD_CTRL_REG) = sWdtDev.ctrlRegValue;

	if(sWdtDev.active)
	{
		wdt_restart();
	}
	else
	{
		sWdtDev.active = 1;
	}
}

/*******************************************************************************
 * Function:     wdt_stop
 * Description:  watchdog disalbe
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
void wdt_stop(void)
{
	irq_mask(IRQ_VECTOR_WDT);
	REG32(WTD_ENABLE_REG) = WTD_WP_NUM;
	REG32(WTD_CTRL_REG) = 0;
	REG32(WTD_STATUS_REG) = 1;
}

/*******************************************************************************
 * Function:     wdt_reset_chip
 * Description:  watchdog reset the CHIP, and clk is from 32K
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
void wdt_reset_chip(int type)
{
	CLK_ENABLE(CLK_WCT);
	GIE_DISABLE();
	wdt_config(WDT_INTR_PERIOD_0, WDT_RST_TIME_0, WDT_INTR_DISABLE, WDT_RST_ENABLE);	
	wdt_start(type);
	//cfg area-mux, and change the cache to 32K-ilm
	REG32(SOC_AON_ILM_CACHE_SWITCH) = 1;
	while (1);
}

#if 0
void wdt_callback_rst(void)
{
	system_schedule_work_queue_from_isr((sys_task_func)wdt_restart, NULL, NULL);
}

 void Drv_Wtd_Init(void)
{
	int wtdFlag;
	int wtd_intr_period;
	char flag[8] = {0};
	wdt_init();
	wdt_isr_register(wdt_callback_rst);
	if( 1 == ef_get_env_blob(NV_WTD_FLAG, flag, 2, NULL))
	{
		 wtdFlag = atoi(flag);
	}
	else
	{
		 wtdFlag = 0;
	}
	if( 1 == ef_get_env_blob(NV_WTD_INTRPERIOD, flag, 2, NULL))
	{
		 wtd_intr_period = atoi(flag);
		 wtd_intr_period = wtd_intr_period > 15 ? 15 : wtd_intr_period;
		 wtd_intr_period = wtd_intr_period < 0 ? 0 : wtd_intr_period;
	}
	else
	{
		 wtd_intr_period = WDT_INTR_PERIOD_7;
	}	
	if(1 == wtdFlag )
	{
		wdt_config(wtd_intr_period, WDT_RST_TIME_0, WDT_INTR_ENABLE, WDT_RST_ENABLE);
		wdt_start(WDT_RESET_CHIP_TYPE_RESYSTEM);
		system_printf("wdt_start\n");
	}
	else
	{
		system_printf("wdt_not_start\n");
	}
	
 }
#endif

#define        WTD_TASK_STACK_SIZE	    (512 / sizeof(StackType_t))
StackType_t	   s_wtd_task_stack[WTD_TASK_STACK_SIZE] = {0,};
StaticTask_t						   s_wtd_task_tcb = {0,};
TaskHandle_t						   g_wtd_task_handle;
static SemaphoreHandle_t               xBinarySemaphore_wtd;
int	   g_wtdFlag;
int    g_wtdPriFlag;
int    g_wtdRunFlag;
int    g_wtdPeriod;



 /*******************************************************************************
  * Function:	  Drv_WtdTask_Init
  * Description:  启动watchdog任务。
  1、根据nv参数决定是否启动wtd，以及intr时长；
  2、创建wtd_task，任务优先级为11，开512bit栈空间，创建二进制信号量；
  3、中断处理give isr信号量并强制切换上下文；
  4、wtd_task阻塞take信号量，喂狗；
  5、设置wtd，intr时长，rst时长；
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

  void Hal_WtdTask_Init(void)
 {
	 //int wtdFlag;
	 int ret;
	 int wtd_intr_period;
	 char flag[8] = {0};
	 
#if (!defined _USR_LMAC_TEST)&&(!_USER_LMAC_SDIO)
	 if( 1 == ef_get_env_blob(NV_WTD_FLAG, flag, 2, NULL))
	 {
		  g_wtdFlag = atoi(flag);
	 }
	 else
	 {
		  g_wtdFlag = 0;
	 }
	 ret = ef_get_env_blob(NV_WTD_INTRPERIOD, flag, 8, NULL);
	 if( 0 == ret)
	 {
	 	  wtd_intr_period = WDT_INTR_PERIOD_7;

	 }
	 else
	 {
	 	  flag[ret] = 0;
		  wtd_intr_period = atoi(flag);
		  wtd_intr_period = wtd_intr_period > WDT_INTR_PERIOD_15 ? WDT_INTR_PERIOD_15 : wtd_intr_period;
		  wtd_intr_period = wtd_intr_period < WDT_INTR_PERIOD_0 ? WDT_INTR_PERIOD_0 : wtd_intr_period; 
	 }
	 #else
	 g_wtdFlag = 0;
	 wtd_intr_period = WDT_INTR_PERIOD_7;
	 #endif
	 if(1 == g_wtdFlag )
	 {
	 	 wdt_init();
	 	 wtd_task_init();
		 wdt_isr_register(wtd_rst_send);
		 wdt_config(wtd_intr_period, WDT_RST_TIME_7, WDT_INTR_ENABLE, WDT_RST_ENABLE);
		 wdt_start(WDT_RESET_CHIP_TYPE_RESYSTEM);
		 //system_printf("wdt_start\n");
	 }
	 else
	 {
		 system_printf("wdt_not_start\n");
	 }
	 
	 
  }

void Hal_WtdSoftTask_Init(void)
{
	 int ret;
	 char flag[8] = {0};
	 g_wtdPriFlag = 0;
	 int wtd_intrPeriod;
	 #if (!defined _USR_LMAC_TEST) && (!defined _USER_LMAC_SDIO)
	 if( 1 == ef_get_env_blob(NV_WTD_FLAG, flag, 2, NULL))
	 {
		  g_wtdFlag = atoi(flag);
	 }
	 else
	 {
		  g_wtdFlag = 0;
	 }
	 ret = ef_get_env_blob(NV_WTD_INTRPERIOD, flag, 8, NULL);
	 if( 0 == ret)
	 {
	 	  g_wtdPeriod = 30;
		  wtd_intrPeriod = WDT_INTR_PERIOD_10;
		  g_wtdPeriod = g_wtdPeriod* 1000;

	 }
	 else
	 {
	 	  flag[ret] = 0;
		  g_wtdPeriod = atoi(flag);
		  g_wtdPeriod = g_wtdPeriod > 1000 ? 1000 : g_wtdPeriod;
		  g_wtdPeriod = g_wtdPeriod < 1 ? 1 : g_wtdPeriod; 
		  wtd_intrPeriod = -1;
		  int ii;
		  int intr_period[16] = {0,0,0,0,0,0,0,1,4,16,64,256,1024,4096,16384,65536};
		  for(ii = 7; ii < 15 ; ii ++)
		  {
		  		int j;
				j = ii + 1;
		  		if(intr_period[ii]<= g_wtdPeriod && intr_period[j] > g_wtdPeriod)
				{
					wtd_intrPeriod = j;
					break;
		  		}
		  }
		  wtd_intrPeriod = wtd_intrPeriod == -1 ? WDT_INTR_PERIOD_15 : wtd_intrPeriod;
		  g_wtdPeriod = g_wtdPeriod* 1000;
		  
		  
	 }
	 
	 #else
	 g_wtdFlag = 0;
	 #endif
	 if(1 == g_wtdFlag )
	 {
	 	 g_wtdRunFlag = 1;
		 CLK_ENABLE(CLK_WCT);
	 	 wtd_softtask_init();
		 wdt_config(wtd_intrPeriod, WDT_RST_TIME_0, WDT_INTR_ENABLE, WDT_RST_ENABLE);
		 wdt_start(WDT_RESET_CHIP_TYPE_RESYSTEM);
	 }
	 else
	 {
		 system_printf("wdt_not_start\n");
	 } 		
 }


 /*******************************************************************************
  * Function:	  wtd_task_init
  * Description:  创建wtd task
	创建wtd_task，任务优先级为11，开512bit栈空间，创建二进制信号量；

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
void wtd_task_init()
{
	g_wtd_task_handle = xTaskCreateStatic(
				 wtd_task,
				 "WatchDog Task",
				 WTD_TASK_STACK_SIZE,
				 NULL,
				 THREAD_WTD_PRI/*WDT_TASK_PRIORITY*/,
				 &s_wtd_task_stack[0],
				 &s_wtd_task_tcb);
 	xBinarySemaphore_wtd = xSemaphoreCreateBinary();
	if (g_wtd_task_handle)
		system_printf("[%s, %d] task creation succeed!(0x%x)\n", __func__, __LINE__, g_wtd_task_handle);	 
	else
		system_printf("[%s, %d] task creation failed!(0x%x)\n", __func__, __LINE__);
}


void wtd_softtask_init()
{
	g_wtd_task_handle = xTaskCreateStatic(
				 wtd_softtask,
				 "WatchDog SoftTask",
				 WTD_TASK_STACK_SIZE,
				 NULL,
				 THREAD_WTDSOFT_PRI,
				 &s_wtd_task_stack[0],
				 &s_wtd_task_tcb);
	if (g_wtd_task_handle)
		system_printf("[%s, %d] task creation succeed!(0x%x)\n", __func__, __LINE__, g_wtd_task_handle);	 
	else
		system_printf("[%s, %d] task creation failed!(0x%x)\n", __func__, __LINE__);
}


/*******************************************************************************
 * Function:	 wtd_task
 * Description:  wtd task处理函数
   wtd_task阻塞take信号量，喂狗；

 * Parameters: 
 *	 Input:
 *
 *	 Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/ 

void wtd_task(void *pvParameters) 
{
	while(1)
	{
		if (xSemaphoreTake(xBinarySemaphore_wtd,( TickType_t )portMAX_DELAY))
		{
			wdt_restart();
			if(g_wtdPriFlag == 1)
				system_printf("feed dog\n");
		}
	}
}

void wtd_softtask(void *pvParameters) 
{
	const portTickType xDelay = pdMS_TO_TICKS(g_wtdPeriod);  

	while(1)
	{
		vTaskDelay( xDelay ); 
		if(g_wtdRunFlag == 1)
		{
			wdt_stop();
			wdt_start(WDT_RESET_CHIP_TYPE_RESYSTEM);
			if(g_wtdPriFlag == 1)
				system_printf("RERUN WTD\n");
		}	
	}
}


/*******************************************************************************
 * Function:	 wtd_rst_send
 * Description:  发送信号量
   中断处理give isr信号量并强制切换上下文；

 * Parameters: 
 *	 Input:
 *
 *	 Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/ 

void wtd_rst_send(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE, ret;
	ret = xSemaphoreGiveFromISR(xBinarySemaphore_wtd, &xHigherPriorityTaskWoken);
	if(ret != pdTRUE) 
	{
		system_printf("wdt rst send failed!!\n");
	}    
}

/*******************************************************************************
 * Function:	 wtd_resume
 * Description:  当nv配置有效时，启动wdt

 * Parameters: 
 *	 Input:
 *
 *	 Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/ 

void wtd_resume(void)
{
	if(1 == g_wtdFlag)
	{
		g_wtdRunFlag = 1;
		wdt_start(WDT_RESET_CHIP_TYPE_RESYSTEM);
	}
}

/*******************************************************************************
 * Function:	 wtd_gstatus
 * Description:  接口，供其他模块获得nv配置wtd

 * Parameters: 
 *	 Input:
 *
 *	 Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/ 

int wtd_gstatus(void)
{
	return 1 == g_wtdFlag ? 1 : 0;
}

void wtd_set_stop(void)
{
	wdt_stop();
	g_wtdRunFlag = 0;
}
#if (!defined AMT) && (!defined SINGLE_BOARD_VER)
static int cmd_wdt_err(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("cmd_wdt_err\n");
	while(1);
	return CMD_RET_SUCCESS;
}

CMD(werr, cmd_wdt_err,  "watch dog err test",   "wdt");

static int cmd_wtd_stop(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("cmd_wdt_stop\n");
	wdt_stop();
	g_wtdRunFlag = 0;
	return CMD_RET_SUCCESS;
}

CMD(wstop,cmd_wtd_stop,"","");
static int cmd_wtd_resume(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("cmd_wdt_resume\n");
	wtd_resume();
	return CMD_RET_SUCCESS;
}

CMD(wresume,cmd_wtd_resume,"","");
static int cmd_wtd_reset(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("cmd_wdt_resume\n");
	wdt_reset_chip(WDT_RESET_CHIP_TYPE_RESYSTEM);
	return CMD_RET_SUCCESS;
}

CMD(wreset,cmd_wtd_reset,"","");

static int cmd_wtd_print(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret;
	char * key;
	if(argc != 2)
	{
		system_printf("FAIL\n");
		return CMD_RET_FAILURE;
	}
	key = argv[1];
	ret = atoi(key); 
	g_wtdPriFlag = ret == 1 ? 1 : 0;
	if(g_wtdPriFlag == 1)
		system_printf("Open wtd print!\n");
	else
		system_printf("Close wtd print!\n");
	
	return CMD_RET_SUCCESS;
}

CMD(wprint,cmd_wtd_print,"","");

#endif
#if 0
#include "system_common.h"

static int wtd_start(cmd_tbl_t *t, int argc, char *argv[])
{

    CLK_ENABLE(CLK_WCT);
    wdt_config(WDT_INTR_PERIOD_9, WDT_RST_TIME_0, WDT_INTR_DISABLE, WDT_RST_ENABLE); // 16s
    wdt_start(WDT_RESET_CHIP_TYPE_RESYSTEM);
	system_printf("wtd_start!\n");
	
	return CMD_RET_SUCCESS;
}

CMD(wstart,wtd_start,"","");


void wdt_callback_test_timer(void)
{
	wdt_restart();
	system_printf("wdt_callback_test_timer\n");
}

void wdt_callback_test_rst(void)
{
	system_schedule_work_queue_from_isr((sys_task_func)wdt_restart, NULL, NULL);
	system_printf("wdt_callback_test_rst\n");
}


static int cmd_wdt_start(cmd_tbl_t *t, int argc, char *argv[])
{
	wdt_init();
	wdt_isr_register(wdt_callback_test_timer);
	wdt_config(WDT_INTR_PERIOD_8, WDT_RST_TIME_7, WDT_INTR_ENABLE, WDT_RST_DISABLE);
	wdt_start(WDT_RESET_CHIP_TYPE_RESYSTEM);
	system_printf("cmd_wdt_start\n");
	return CMD_RET_SUCCESS;
}

CMD(wstart, cmd_wdt_start,  "watch dog start",   "wdt");


static int cmd_wdt_stop(cmd_tbl_t *t, int argc, char *argv[])
{
	wdt_stop();
	system_printf("cmd_wdt\n");
	return CMD_RET_SUCCESS;
}

CMD(wstop, cmd_wdt_stop,  "watch dog stop",   "wdt");


static int cmd_wdt_rst(cmd_tbl_t *t, int argc, char *argv[])
{
	wdt_init();
	wdt_isr_register(wdt_callback_test_rst);
	wdt_config(WDT_INTR_PERIOD_7, WDT_RST_TIME_7, WDT_INTR_ENABLE, WDT_RST_ENABLE);
	wdt_start(WDT_RESET_CHIP_TYPE_RESYSTEM);
	system_printf("cmd_wdt_start\n");
	return CMD_RET_SUCCESS;
}

CMD(wrst, cmd_wdt_rst,  "watch dog stop",   "wdt");


static int cmd_wdt_err(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("cmd_wdt_err\n");
	while(1);
	return CMD_RET_SUCCESS;
}

CMD(werr, cmd_wdt_err,  "watch dog err test",   "wdt");



#endif



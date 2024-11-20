/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:    hal_pwm.c
 * File Mark:    
 * Description:  pwm
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
 *     Date: 20190307
 *     Version:
 *     Author: liuyafeng
 *     Modification:add test code
  ********************************************************************************/

/****************************************************************************
* 	                                           Include files
****************************************************************************/
#include "system_common.h"
#include "soc_top_reg.h"
#include "pit.h"
#include "irq.h"
#include "drv_timer.h"

/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define REG32(reg)			(  *( (volatile unsigned int *) (reg) ) )

/****************************************************************************
* 	                                           Local Types
****************************************************************************/

typedef struct _H_TIMER
{
	unsigned int reload;
	void (* callback)(void * );
	void * user_data;
} H_TIMER;

static H_TIMER hal_timer;

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
 * Function:     hal_timer_isr
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
void hal_timer_isr(void)
{
	pit_int_status_handle(PIT_NUM_0, PIT_CHN_0, PIT_INT_STATUS_CLEAN);
	irq_status_clean(IRQ_VECTOR_PIT0);

	if(hal_timer.reload == 0)
	{
		hal_timer_stop();
	}

	hal_timer.callback(hal_timer.user_data);
}

/*******************************************************************************
 * Function:     hal_timer_init
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
int hal_timer_init(void)
{
	irq_status_clean(IRQ_VECTOR_PIT0);
	irq_unmask(IRQ_VECTOR_PIT0);
	irq_isr_register(IRQ_VECTOR_PIT0, (void *)hal_timer_isr);
	return 0;
}


/*******************************************************************************
 * Function:     hal_timer_config
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
int hal_timer_config(unsigned int us, unsigned int reload)
{
	unsigned int count;
	int ret, flags;
	
	if(hal_timer_stop() != 0)
	{
		return -1;
	}	

	flags = system_irq_save();
	if(pit_ch_ctrl(PIT_NUM_0, 0, 0, PIT_CHCLK_APB, PIT_CHMODE_32BIT_TM) != 0)
	{
	    	system_irq_restore(flags); 
		return -1;
	}

	count = us * 40;
	hal_timer.reload = reload;

	ret = pit_ch_reload_value(PIT_NUM_0, 0, count);
    	system_irq_restore(flags); 
	return ret;	
	
}

/*******************************************************************************
 * Function:     hal_timer_start
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
int hal_timer_start(void)
{
	int ret, flags;
	flags = system_irq_save();
	pit_int_enable(PIT_NUM_0, 0, 1);
	ret = pit_ch_mode_set(PIT_NUM_0, 0, PIT_CH_TM0_EN);
    	system_irq_restore(flags); 
	return ret;	
}

/*******************************************************************************
 * Function:     hal_timer_stop
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
int hal_timer_stop(void)
{
	int ret, flags;
	flags = system_irq_save();
	pit_int_enable(PIT_NUM_0, 0, 0);
	ret = pit_ch_mode_set(PIT_NUM_0, 0, PIT_CH_DISABLE);
    	system_irq_restore(flags); 
	return ret;
}


/*******************************************************************************
 * Function: timer_callback_register
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
 void hal_timer_callback_register(void (* user_timer_cb_fun)(void *), void *data)
{
	hal_timer.callback = user_timer_cb_fun;
	hal_timer.user_data = data;
}
 
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
 void hal_timer_callback_unregister(void)
 {
	hal_timer.callback = 0;
	hal_timer.user_data = 0;
 }

#if 0

#include "system_common.h"
void timer_callback(void *data)
{
	system_printf("timer_callback every 5 sec!\n");
}

static int timer_open(cmd_tbl_t *t, int argc, char *argv[])
{
	unsigned int value;

	value = atoi(argv[1]);
	system_printf("timer_open, reload: %d\n", value);

	hal_timer_init();
	hal_timer_config(5*1000000, value);
	hal_timer_callback_register(timer_callback, NULL);
	hal_timer_start();
	return CMD_RET_SUCCESS;
}

static int timer_close(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("timer_close\n");
	hal_timer_stop();
	hal_timer_callback_unregister();
	return CMD_RET_SUCCESS;
}

CMD(tmo,   timer_open,   "timer_open test",   "timer_open test");
CMD(tmc,   timer_close,   "timer_close test",   "timer_close test");


#endif

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
#include "soc_top_reg.h"
#include "soc_pin_mux.h"
#include "pit.h"
#include "drv_pwm.h"

/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define REG32(reg)			(  *( (volatile unsigned int *) (reg) ) )
#define PWM_CLK_BASE		40000000 //40Mhz

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
uint8_t led_status[5] = {0,0,0,0,0};
uint8_t tuya_led_psm_flag = 1;
/****************************************************************************
* 	                                          Global Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/

/*******************************************************************************
 * Function:     hal_pwm_init
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
int32_t pwm_init(uint32_t channel)
{
	if(channel >= PMW_CHANNEL_MAC)
	{
		return DRV_ERR_INVALID_PARAM;
	}	
	
	pwm_stop(channel);

#ifdef MPW
	//JUST USE  PWM_CTRL5 FOR PWM, TEMPORARY
	//REG32(SOC_PIN0_MUX_BASE) = (REG32(SOC_PIN0_MUX_BASE) & (~(7<<9))) |(4<<9);
	REG32(SOC_PIN0_MUX_BASE) = (REG32(SOC_PIN0_MUX_BASE) & (~(7<<0))) |(4<<0);
	REG32(SOC_PIN0_MUX_BASE) = (REG32(SOC_PIN0_MUX_BASE) & (~(7<<3))) |(4<<3);
#endif


	switch (channel) 
	{
#if defined (_USR_TR6260)
		case PMW_CHANNEL_0:
			PIN_FUNC_SET(IO_MUX0_GPIO20, FUNC_GPIO20_PWM_CTRL0);
			break;
			
		case PMW_CHANNEL_1:
			PIN_FUNC_SET(IO_MUX0_GPIO21, FUNC_GPIO21_PWM_CTRL1);
			break;

		case PMW_CHANNEL_2:
			PIN_FUNC_SET(IO_MUX0_GPIO22, FUNC_GPIO22_PWM_CTRL2);
			break;

		case PMW_CHANNEL_3:
			PIN_FUNC_SET(IO_MUX0_GPIO23, FUNC_GPIO23_PWM_CTRL3);
			break;

		case PMW_CHANNEL_4:
			PIN_FUNC_SET(IO_MUX0_GPIO24, FUNC_GPIO24_PWM_CTRL4);
			break;
#endif

#if defined (_USR_TR6260S1)
		case PMW_CHANNEL_0:
			PIN_FUNC_SET(IO_MUX0_GPIO20, FUNC_GPIO20_PWM_CTRL0);
			break;
	
		case PMW_CHANNEL_1:
			PIN_FUNC_SET(IO_MUX0_GPIO21, FUNC_GPIO21_PWM_CTRL1);
			break;

		case PMW_CHANNEL_2:
			PIN_FUNC_SET(IO_MUX0_GPIO22, FUNC_GPIO22_PWM_CTRL2);
			break;
#endif
#ifndef _USER_LMAC_SDIO
		case PMW_CHANNEL_5:
			PIN_FUNC_SET(IO_MUX0_GPIO13, FUNC_GPIO13_PWM_CTRL5);
			break;
#endif
		
	}

	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function:     hal_pwm_deinit
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
int32_t pwm_deinit(uint32_t channel)
{
	if(channel >= PMW_CHANNEL_MAC)
	{
		return DRV_ERR_INVALID_PARAM;
	}

	return DRV_SUCCESS;
}


/*******************************************************************************
 * Function:     hal_pwm_config
 * Description:  
 * Parameters: 
 *   Input:freq:<= APB bus, duty_ratio:0~1000
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
int32_t pwm_config(uint32_t channel, uint32_t freq, uint32_t duty_ratio)
{
	uint32_t tick_h, tick_l;
	uint32_t base, num, flags;
	int32_t rnt = 0;

	if(channel >= PMW_CHANNEL_MAC || duty_ratio <= 0 || duty_ratio >=1000)
	{
		return DRV_ERR_INVALID_PARAM;
	}
	else if(channel < PMW_CHANNEL_4)
	{
		base = PIT_NUM_0;
		num = channel;
	}
	else
	{
		base = PIT_NUM_1;
		num = channel -PMW_CHANNEL_4;
	}

	rnt = pwm_stop(channel);
	
	if(rnt != DRV_SUCCESS)
    {
		return DRV_ERROR;
	}	

	//tick_h = PWM_CLK_BASE/1000*duty_ratio/freq;
	//tick_h = (PWM_CLK_BASE+0.0)/1000*duty_ratio/freq + 0.5;
	//tick_l = PWM_CLK_BASE/1000*(1000 - duty_ratio)/freq;
	//tick_l = (PWM_CLK_BASE+0.0)/1000*(1000 - duty_ratio)/freq + 0.5;
	
	tick_h = PWM_CLK_BASE/1000*duty_ratio/freq;
	tick_l = PWM_CLK_BASE/freq - tick_h;
	
	if(tick_h>=1)
		tick_h = tick_h - 1;
	if(tick_l>=1)
		tick_l = tick_l - 1;
	
	flags = system_irq_save();
	rnt = pit_ch_ctrl(base, num, 0, PIT_CHCLK_APB, PIT_CHMODE_PWM);
    if(rnt != DRV_SUCCESS)
    {
    	system_irq_restore(flags); 
		return DRV_ERROR;
	}
	rnt = pit_ch_reload_value(base, num, (tick_h<<16)|tick_l);
	if(rnt != DRV_SUCCESS)
    {
    	system_irq_restore(flags); 
		return DRV_ERROR;
	}
		system_irq_restore(flags); 
	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function:     hal_pwm_start
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
int32_t pwm_start(uint32_t channel)
{
	uint32_t base, num, flags;
	int32_t rnt = 0;

	if(channel >= PMW_CHANNEL_MAC)
	{
		return DRV_ERR_INVALID_PARAM;
	}
	else if(channel < PMW_CHANNEL_4)
	{
		base = PIT_NUM_0;
		num = channel;
	}
	else
	{
		base = PIT_NUM_1;
		num = channel -PMW_CHANNEL_4;
	}
	flags = system_irq_save();

	rnt = pit_ch_mode_set(base, num, PIT_CH_TM3_PWM_EN);
	if (channel>=0 && channel<5)
	{
		led_status[channel] = 1;
	}
	tuya_led_psm_flag = 1;
 	system_irq_restore(flags); 
	if(rnt != DRV_SUCCESS)
    {
		return DRV_ERROR;
	}
	
	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function:     hal_pwm_start
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
int32_t pwm_stop(uint32_t channel)
{
	uint32_t base, num, flags;
	int32_t rnt = 0;

	if(channel >= PMW_CHANNEL_MAC)
	{
		return DRV_ERR_INVALID_PARAM;
	}
	else if(channel < PMW_CHANNEL_4)
	{
		base = PIT_NUM_0;
		num = channel;
	}
	else
	{
		base = PIT_NUM_1;
		num = channel -PMW_CHANNEL_4;
	}
	flags = system_irq_save();
	rnt = pit_ch_mode_set(base, num, PIT_CH_DISABLE);
	if (channel>=0 && channel<5)
	{
		led_status[channel] = 0;
	}
	tuya_led_psm_flag = (led_status[0] | led_status[1] | led_status[2] | led_status[3] | led_status[4]);
 	system_irq_restore(flags); 
	if(rnt != DRV_SUCCESS)
    {
		return DRV_ERROR;
	}
	return DRV_SUCCESS;
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
#ifdef _USE_TEST_CMD_PWM
static int cmd_pwm_config(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret = 0;
	if(argc != 4)
	{
		system_printf("The Number Of Argv Is Not 4");
		return DRV_ERR_INVALID_PARAM;
	}
	ret = pwm_init(atoi(argv[1]));
	if (ret == DRV_ERR_INVALID_PARAM)
	{
		system_printf("PWM Chanel Is 0-5");
		return DRV_ERR_INVALID_PARAM;
	}
	ret = pwm_config(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	if(ret != DRV_SUCCESS)
	{
		system_printf("Config Fail");
		return DRV_ERROR;
	}
	return DRV_SUCCESS;
	
}


SUBCMD(pwm,
    config,
    cmd_pwm_config,
    "pwm config",
    "pwm config <PWM Channel> <Freq> <duty_ratio>");

static int cmd_pwm_start(cmd_tbl_t *t, int argc, char *argv[])
{
	if(argc != 2)
	{
		system_printf("The Number Of Argv Is Not 2");
		return DRV_ERR_INVALID_PARAM;
	}

	pwm_start(atoi(argv[1]));
	
	return DRV_SUCCESS;
		
}
	
	
SUBCMD(pwm,
		start,
		cmd_pwm_start,
		"pwm start",
		"pwm start <PWM Channel>");

static int cmd_pwm_stop(cmd_tbl_t *t, int argc, char *argv[])
{
    if(argc != 2)
    {
    	system_printf("The Number Of Argv Is Not 2");
		return DRV_ERR_INVALID_PARAM;
    }
	pwm_stop(atoi(argv[1]));
	
	return DRV_SUCCESS;
		
}
	
	
SUBCMD(pwm,
		stop,
		cmd_pwm_stop,
		"pwm stop",
		"pwm stop <PWM Channel>");

#endif

#if 0
int initEyeLight()
{
	int ret = 0;
	ret = pwm_init(PMW_CHANNEL_4);	
	if (ret == DRV_ERR_INVALID_PARAM)
	{
		system_printf("PWM Chanel Is 0-5");
		return DRV_ERR_INVALID_PARAM;	
	}
	ret = pwm_config(PMW_CHANNEL_4, 38000, 500);
	if(ret != DRV_SUCCESS)
	{
		system_printf("Config Fail");	
		return DRV_ERROR;	
	}
	return DRV_SUCCESS;	
}


int setEyeLight(int isOpen)
{
	int ret = 0;    
	if (isOpen == 1) 
	{       
		ret = pwm_start(PMW_CHANNEL_4);   
	}
	else
	{       
		ret= pwm_stop(PMW_CHANNEL_4);   
	}    
	return ret;
}


static void led_task(void *pvParameters)
{
 	int open=0;
	int mode=0;
	int delay_time=1000;

	initEyeLight();

	while(1)
	{		
		mode = 1;
		switch(mode)	
		{		
			case 0:		
				open=!open;		
				setEyeLight(open);		
				delay_time = 1000;	
				break;		
			case 1:		
				open=!open;			
				setEyeLight(open);		
				delay_time = 2000;			
				break;		
			case 2:	
				break;		
			default:		
				break;	
		}		       
		vTaskDelay(delay_time/portTICK_PERIOD_MS);       
	}

	vTaskDelete(NULL);	

}


#define LED_SUPPLICANT_STACK_SIZE	512
#define LED_TASK_PRIORITY		1
StackType_t	   led_stack[LED_SUPPLICANT_STACK_SIZE/sizeof(StackType_t)];
StaticTask_t   led_tcb;
static int cmd_pwm_test(cmd_tbl_t *t, int argc, char *argv[])
{
	xTaskCreateStatic(			
		led_task,		
		"led_task",		
		LED_SUPPLICANT_STACK_SIZE/sizeof(StackType_t),		
		NULL,			
		LED_TASK_PRIORITY,		
		&led_stack[0],		
		&led_tcb);
	
	return DRV_SUCCESS;
}
	
	
SUBCMD(pwm,
		test,
		cmd_pwm_test,
		"pwm test",
		"pwm test");


#endif /* _USE_TEST_CMD */


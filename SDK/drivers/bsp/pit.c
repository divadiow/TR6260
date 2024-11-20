/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:  pit.c  
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangchao
 * Date:          2018-4-11
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
#include "pit.h"
#include "soc_top_reg.h"
#include "drv_timer.h"

/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define REG32(reg)			(  *( (volatile unsigned int *) (reg) ) )
	
/* PIT register (32-bit width) */
#define PIT_ID_REV_N			(pit_base + 0x00 ) /* (ro)  PIT ID and Revision Register */
#define PIT_CFG_N				(pit_base + 0x10 ) /* (ro)  PIT Configuration Register	 */
#define PIT_INT_EN_N			(pit_base + 0x14 ) /* (rw)  PIT Interrupt Enable Register*/
#define PIT_INT_ST_N			(pit_base + 0x18 ) /* (w1c) PIT Interrupt Status Register*/
#define PIT_CH_EN_N				(pit_base + 0x1C ) /* (rw)  PIT Channel Enable Register	 */

#ifdef MPW
/* _chn_ from 0 to 3*/
/* (rw) PIT Channel x Control Register (32-bit width) */
#define PIT_CHNx_CTL_N(_chn_)		( pit_base + 0x20 + ( (_chn_)* 0x10) )
/* (rw) PIT Channel x Reload Register (32-bit width)  */
#define PIT_CHNx_LOAD_N(_chn_)		( pit_base + 0x24 + ( (_chn_)* 0x10) )
/* (ro) PIT Channel x Counter Register (32-bit width) */
#define PIT_CHNx_COUNT_N(_chn_)		( pit_base + 0x28 + ( (_chn_)* 0x10) )
#endif

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

#ifdef MPW
int pit_ch_count_get(unsigned int pit_base, unsigned int num)
{
	if(num > 4)
		return -1;

	return REG32(PIT_CHNx_COUNT_N(num));
}


int pit_ch_reload_value(unsigned int pit_base, unsigned int num, unsigned int value)
{
	if(num > 4)
		return -1;

	REG32(PIT_CHNx_LOAD_N(num)) = value;
	return 0;
}

/* 
	pwmpark: ...........

	chClk == 0: channel clock source is external clock
	chClk == 1: channel clock source is apb clock

	chmod == 0: reserved
	chmod == 1: 32bit timer
	chmod == 2: 16bit timer
	chmod == 3: 8bit timer
	chmod == 4: pwm
	chmod == 5: reserved
	chmod == 6: mixed pwm/16bit-timer
	chmod == 7: mixed pwm/8bit-timer
*/
int pit_ch_ctrl(unsigned int pit_base, unsigned int num, unsigned int PWMPark, unsigned int chClk, unsigned int chMode)
{
	if(num > 4)
		return -1;

	if(PWMPark > 2)
		return -2;

	if(chClk > 2)
		return -3;

	if(chMode > 8)
		return -4;

	REG32(PIT_CHNx_CTL_N(num)) = (PWMPark<<4) | (chClk <<3) | (chMode);
	return 0;
}

/* ------------------------------------------------------------------------------------------------------------
	channel-mode		(32bit timer)	(16bit timer)	(8bit timer)	(PWM)	(mixed pwm/16bit-timer)	(mixed pwm/8bit-timers)
					32bit-timer0	16bit-timer0	8bit-timer0				16bit-timer0				8bit-timer0
								16bit-timer1	8bit-timer1										8bit-timer1
											8bit-timer2
											8bit-timer3
														16bit-pwm	8bit-pwm					8bit-pwm
    ----------------------------------------------------------------------------------------------------------
								
	enable == 0x1: timer0 enable
	enable == 0x2: timer1 enable
	enable == 0x4: timer2 enable
	enable == 0x8: other fun(timer3 pwm) enable

   ----------------------------------------------------------------------------------------------------------*/
int pit_ch_mode_enable(unsigned int pit_base, unsigned int num, unsigned int enable)
{
	if(num > 4)
		return -1;

	REG32(PIT_CH_EN_N) = REG32(PIT_CH_EN_N) | (0x1 << (4*(num)));
	return 0;
}

/* 
	action == 1: timer interrupt status clear
	action == 0: get the timer interrupt status
*/
int pit_int_status_handle(unsigned int pit_base, unsigned int num, unsigned int action)
{
	if(num > 4)
		return -1;

	if (action)
	{
		REG32(PIT_INT_ST_N) = (0x1 << (4*(num)));
		return 0;
	}
	else
		return !!(REG32(PIT_INT_ST_N)  & ~(0x1 << (4*(num))));
}


int pit_int_enable(unsigned int pit_base, unsigned int num, unsigned int enable)
{
	if(num > 4)
		return -1;

	if (enable)
		REG32(PIT_INT_EN_N) = REG32(PIT_INT_EN_N) | (0x1 << (4*(num)));
	else
		REG32(PIT_INT_EN_N) = REG32(PIT_INT_EN_N) & ~(0x1 << (4*(num)));

	return 0;
}

void pit_init(void)
{
	unsigned int pit_base = PIT_BASE0;

	REG32(PIT_INT_EN_N) = 0;		/* disable all timer interrupt */
	REG32(PIT_CH_EN_N)  = 0;		/* disable all timer */
	REG32(PIT_INT_ST_N) = -1;		/* clear pending events */
	REG32(PIT_CHNx_LOAD_N(0)) = 0;	/* clean channel 0 reload */
	REG32(PIT_CHNx_LOAD_N(1)) = 0;	/* clean channel 1 reload */
	REG32(PIT_CHNx_LOAD_N(2)) = 0;	/* clean channel 2 reload */
	REG32(PIT_CHNx_LOAD_N(3)) = 0;	/* clean channel 3 reload */

	pit_base = PIT_BASE1;

	REG32(PIT_INT_EN_N) = 0;		/* disable all timer interrupt */
	REG32(PIT_CH_EN_N)  = 0;		/* disable all timer */
	REG32(PIT_INT_ST_N) = -1;		/* clear pending events */
	REG32(PIT_CHNx_LOAD_N(0)) = 0;	/* clean channel 0 reload */
	REG32(PIT_CHNx_LOAD_N(1)) = 0;	/* clean channel 1 reload */
	REG32(PIT_CHNx_LOAD_N(2)) = 0;	/* clean channel 2 reload */

	REG32(PIT_CHNx_LOAD_N(3)) = 0xFFFFFFFF;
	REG32(PIT_CHNx_CTL_N(3)) =  0x9;
	REG32(PIT_CH_EN_N) = REG32(PIT_CH_EN_N) | (0x1 << 0xC);
}


void pit_delay(unsigned long delay)
{
	long tmo = delay;
	unsigned long now, last = pit_ch_count_get(PIT_BASE1, 3);

	while (tmo > 0)
	{
//		asm volatile("":::"memory");
		now = pit_ch_count_get(PIT_BASE1, 3);
		if (now > last) /* count down timer overflow */
			tmo -= 0xFFFFFFFF - now+ last;
		else
			tmo -= last - now;
		last = now;
	}
}

#endif

int pit_ch_mode_disable(unsigned int pit_base, unsigned int num, unsigned int enable)
{
	if(num > 4)
		return -1;

	REG32(PIT_CH_EN_N) = REG32(PIT_CH_EN_N) &(~(0x1 << (4*(num))));
	return 0;
}

/* ------------------------------------------------------------------------------------------------------------
	channel-mode		(32bit timer)	(16bit timer)	(8bit timer)	(PWM)	(mixed pwm/16bit-timer)	(mixed pwm/8bit-timers)
					32bit-timer0	16bit-timer0	8bit-timer0				16bit-timer0				8bit-timer0
								16bit-timer1	8bit-timer1										8bit-timer1
											8bit-timer2
											8bit-timer3
														16bit-pwm	8bit-pwm					8bit-pwm
    ----------------------------------------------------------------------------------------------------------
								
	enable == 0x1: timer0 enable
	enable == 0x2: timer1 enable
	enable == 0x4: timer2 enable
	enable == 0x8: other fun(timer3 pwm) enable

   ----------------------------------------------------------------------------------------------------------*/

int pit_ch_mode_set(unsigned int pit_base, unsigned int num, unsigned int enable)
{
	unsigned int val;
	if(num > 4)
		return -1;

	val = REG32(PIT_CH_EN_N);
	val &= ~(0xF << 4*num);
	val |= enable << 4*num;

	REG32(PIT_CH_EN_N) = val;
	return 0;
}

#if 0

/*******************************************************************************
 * Function: timer_init
 * Description: 
 * Parameters: 
 *   Input:req---->DRV_TIMER_LOAD_MODE
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
void hal_timer_init(uint32_t req)
{

}

/*******************************************************************************
 * Function: timer_set_arm
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
 void hal_timer_set_arm(uint32_t val)
{


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
 void hal_timer_callback_register(void (* user_timer_cb_fun)(void))
 {

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

 }
 #endif

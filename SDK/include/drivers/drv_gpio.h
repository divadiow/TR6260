/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: drv_gpio.h   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2018-12-20
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef _DRV_GPIO_H
#define _DRV_GPIO_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/
#include "system.h"

/****************************************************************************
* 	                                        Macros
****************************************************************************/


/****************************************************************************
* 	                                        Types
****************************************************************************/
typedef enum 
{
	DRV_GPIO_0,
	DRV_GPIO_1,
	DRV_GPIO_2,
	DRV_GPIO_3,
	DRV_GPIO_4,
	DRV_GPIO_5,
	DRV_GPIO_6,
	DRV_GPIO_7,
	DRV_GPIO_8,
	DRV_GPIO_9,
	DRV_GPIO_10,
	DRV_GPIO_11,
	DRV_GPIO_12,
	DRV_GPIO_13,/*don't use in gpio mod*/
	DRV_GPIO_14,
	DRV_GPIO_15,
	DRV_GPIO_16,/*don't use in gpio mod*/
	DRV_GPIO_17,/*don't use in gpio mod*/
	DRV_GPIO_18,
	DRV_GPIO_19,
	DRV_GPIO_20,
	DRV_GPIO_21,
	DRV_GPIO_22,
	DRV_GPIO_23,
	DRV_GPIO_24,
	
	DRV_GPIO_MAX
}DRV_GPIO_PIN_NAME;


typedef enum 
{
	DRV_GPIO_DIR_INPUT,
	DRV_GPIO_DIR_OUTPUT,

	DRV_GPIO_DIR_MAX
}DRV_GPIO_DIR;

typedef enum 
{
	DRV_GPIO_PULL_DIS,
	DRV_GPIO_PULL_EN,

	DRV_GPIO_PULL_MAX
}DRV_GPIO_PULL_ENABLE;

typedef enum 
{
	DRV_GPIO_PULL_TYPE_UP,
	DRV_GPIO_PULL_TYPE_DOWN,

	DRV_GPIO_PULL_TYPE_MAX
}DRV_GPIO_PULL_TYPE;

typedef enum 
{
	DRV_GPIO_PULL_UP_EN,
	DRV_GPIO_PULL_UP_DIS,

	DRV_GPIO_PULL_UP_MAX
}DRV_GPIO_PULL_UP_ENABLE;


typedef enum 
{
	DRV_GPIO_DE_BOUNCE_DIS,
	DRV_GPIO_DE_BOUNCE_EN,

	DRV_GPIO_DE_BOUNCE_MAX
}DRV_GPIO_DE_BOUNCE_ENABLE;

typedef enum 
{
	DRV_GPIO_LEVEL_LOW,
	DRV_GPIO_LEVEL_HIGH,

	DRV_GPIO_LEVEL_MAX
}DRV_GPIO_LEVEL;

typedef enum 
{
	DRV_GPIO_WAKEUP_DIS,
	DRV_GPIO_WAKEUP_EN,

	DRV_GPIO_WAKEUP_MAX
}DRV_GPIO_WAKEUP_ENABLE;

typedef enum 
{
	DRV_GPIO_PWR_PD,
	DRV_GPIO_PWR_AO,

	DRV_GPIO_PWR_MAX
}DRV_GPIO_PWR;

typedef enum 
{
	DRV_GPIO_TESTMOD_DIS,
	DRV_GPIO_TESTMOD_EN,

	DRV_GPIO_TESTMOD_MAX
}DRV_GPIO_TESTMOD;


typedef enum 
{
	DRV_GPIO_INT_DIS,
	DRV_GPIO_INT_EN,

	DRV_GPIO_INT_ENABLE_MAX
}DRV_GPIO_INT_ENABLE;


typedef enum 
{
	DRV_GPIO_INT_RESERVED_0,
	DRV_GPIO_INT_RESERVED_1,
	DRV_GPIO_INT_HIGH_LEVEL,
	DRV_GPIO_INT_LOW_LEVEL,
	DRV_GPIO_INT_RESERVED_4,
	DRV_GPIO_INT_NEG_EDGE,
	DRV_GPIO_INT_POS_EDGE,
	DRV_GPIO_INT_DUAL_EDGE,

	DRV_GPIO_INT_LEVEL_MAX
}DRV_GPIO_INT_LEVEL;

typedef struct 
{
	DRV_GPIO_PIN_NAME			 GPIO_Pin;			
	//DRV_GPIO_DIR 	 		 	 GPIO_Dir;
	DRV_GPIO_PULL_ENABLE	 	 GPIO_PullEn;
	DRV_GPIO_PULL_TYPE	 	     GPIO_PullType;
} DRV_GPIO_CONFIG;

typedef struct
{
	void (* callback)(void * data);
	void *data;
}DRV_GPIO_CALLBACK;

/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/
int32_t gpio_config(DRV_GPIO_CONFIG *pGpioConfig);
int32_t gpio_set_dir(uint32_t pin_num,uint32_t pin_dir);
int32_t gpio_get_dir(uint32_t pin_num, uint32_t * pin_dir);
int32_t gpio_get_write_value(uint32_t pin_num, uint32_t * pin_value);
int32_t gpio_write(uint32_t pin_num,uint32_t pin_value);
int32_t gpio_read(uint32_t pin_num,uint32_t *pin_value);
int32_t gpio_wakeup_config(uint32_t pin_num,uint32_t wakeup_en);
int32_t gpio_power_area_config(uint32_t pin_num,uint32_t pwr);
int32_t gpio16_write(uint32_t pin_value);
int32_t gpio17_write(uint32_t pin_value);
int32_t gpio17_read_config(uint32_t pull_up_en);
int32_t gpio17_read(uint32_t *pin_value);
int32_t gpio_irq_level(uint32_t pin_num, uint32_t mode);
int32_t gpio_irq_callback_register(uint32_t pin_num, void(* callback)(void * data), void * data);
int32_t gpio_irq_unmusk(uint32_t pin_num);
int32_t gpio_irq_musk(uint32_t pin_num);
void gpio_isr_init(void);
void TsPsmSaveInfoForSysDeepSleep(void);
void TsPsmRestoreForSysDeepSleep(void);
int32_t gpio_read_tuya(uint32_t pin_num);
void gpio_pin_set(uint32_t pin_num);
int32_t gpio_read_level(uint32_t pin_num);
#endif/*_DRV_GPIO_H*/


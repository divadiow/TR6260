/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: drv_adc.h   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2018-12-25
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef _DRV_ADC_H
#define _DRV_ADC_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/
#include "system.h"
#include "soc_top_reg.h"
#include "drv_efuse.h"

/****************************************************************************
* 	                                        Macros
****************************************************************************/


/****************************************************************************
* 	                                        Types
****************************************************************************/
typedef enum 
{
	DRV_ADC_INPUT_REF_VOLT = 0x0,
	DRV_ADC_INPUT_TEMP_SENSOR = 0x1,
	DRV_ADC_INPUT_VBAT_SENSOR = 0x2,
	DRV_ADC_INPUT_TOUT0 = 0x4,
	DRV_ADC_INPUT_TOUT1 = 0x8,
	DRV_ADC_INPUT_TOUT2 = 0x10,
	DRV_ADC_INPUT_TOUT3 = 0x20,

	DRV_ADC_INPUTL_MAX
}DRV_ADC_INPUT_SEL;

typedef enum 
{
	DRV_ADC_VREF_0_4 = 0x1,//0.4v
	DRV_ADC_VREF_0_45 = 0x2,//0.45v
	DRV_ADC_VREF_0_50 = 0x4,//0.5v
	DRV_ADC_VREF_0_55 = 0x8,//0.55v
	DRV_ADC_VREF_0_60 = 0x10,//0.60v
	DRV_ADC_VREF_0_65 = 0x20,//0.65v
	DRV_ADC_VREF_0_70 = 0x40,//0.70v
	DRV_ADC_VREF_0_75 = 0x80,//0.75v

	DRV_ADC_VREF_MAX
}DRV_ADC_VREF_SEL;


typedef enum 
{
	DRV_ADC_INPUT_VOL_DIV_6 = 0x0,// input_vol/6
	DRV_ADC_INPUT_VOL_DIV_5 = 0x1,//input_vol/5
	DRV_ADC_INPUT_VOL_DIV_4 = 0x2,//input_vol/4

	DRV_ADC_INPUT_VOL_DIV_MAX
}DRV_ADC_INPUT_VOL_DIV;

typedef enum 
{
	DRV_ADC_BB_CLK_DIS,
	DRV_ADC_BB_CLK_EN,

	DRV_ADC_BB_CLK_MAX
}DRV_ADC_BB_CLK_STATUS;

typedef enum 
{
	DRV_ADC_POLYFIT_CAL_ABOVE_1200,
	DRV_ADC_POLYFIT_CAL_BELOW_1200,

	DRV_ADC_POLYFIT_CAL_MAX
}DRV_ADC_POLYFIT_CAL_TYPE;

typedef enum 
{
	DRV_ADC_TEMP_LOW,
	DRV_ADC_TEMP_NORMAL,
	DRV_ADC_TEMP_HIGH

}DRV_ADC_TEMP_TYPE;


/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/
int16_t temperature_sensor_get(void);
int16_t vbat_sensor_get(uint32_t volt_div);
#if 1
int16_t tout_sensor_get(uint32_t tout_num,uint32_t volt_div,uint32_t volt);
#else
int16_t tout_sensor_get(uint32_t tout_num,uint32_t volt_div);
#endif

DRV_ADC_TEMP_TYPE hal_adc_get_temp_type();
void time_check_temp();

extern unsigned int temp_status;


#endif/*_DRV_ADC_H*/


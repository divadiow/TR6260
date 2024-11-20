/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: hal_nec_6260_ir.c(standalone)   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        
 * Date:          20200420
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#include "drv_ir.h"
#include "bsp/soc.h"
#include "bsp/soc_pin_mux.h"
#include "drv_gpio.h"
#include "system_common.h"

#define IR_KEY_1				0xff00
#define IR_KEY_2				0xfe01
#define IR_KEY_3				0xfd02
#define IR_KEY_4				0xfc03
#define IR_KEY_5				0xfb04
#define IR_KEY_6				0xfa05
#define IR_KEY_7				0xf906
#define IR_KEY_8				0xf807
#define IR_KEY_9				0xf708
#define IR_KEY_10				0xf609
#define IR_KEY_11				0xf50a
#define IR_KEY_12				0xf40b
#define IR_KEY_13				0xf30c
#define IR_KEY_14				0xf20d
#define IR_KEY_15				0xf10e
#define IR_KEY_16				0xf00f
#define IR_KEY_17				0xef10
#define IR_KEY_18				0xee11
#define IR_KEY_19				0xed12
#define IR_KEY_20				0xec13
#define IR_KEY_21				0xeb14
#define IR_KEY_22				0xea15
#define IR_KEY_23				0xe916
#define IR_KEY_24				0xe817



#define COSTOM_CODE_HBS838					0xef00
#define COSTOM_DATA_BITS_NUM				32


#define IR_LEADER_HIGH_LEVEL_MAX			13990   	//13.59ms
#define IR_LEADER_HIGH_LEVEL_MIN			13090   

#define IR_LEADER_END_LEVEL_MAX				11990   	//11.32079ms
#define IR_LEADER_END_LEVEL_MIN				11000		

#define IR_LEADER_EMPTY_LEVEL_MAX			105000   	//95.592.32079ms
#define IR_LEADER_EMPTY_LEVEL_MIN			85000		



#define IR_INFOR_HIGH_LEVEL_MAX				2500   		//2.25ms
#define IR_INFOR_HIGH_LEVEL_MIN				2000
#define IR_INFOR_LOW_LEVEL_MAX				1300		//1.125ms
#define IR_INFOR_LOW_LEVEL_MIN				1000

#define IR_STOP_HIGH_LEVEL_MAX				44000   	//41.71ms
#define IR_STOP_HIGH_LEVEL_MIN				40000   	

volatile unsigned int infor_value = 0;
unsigned int test_time0 = 0;
unsigned char receive_num = 0;
enum status
{
	infor_code = 1,
	end_code = 2,
	repeat_code = 3,
};
enum status receive_status;	


unsigned int receive_repeat = 0;

static SemaphoreHandle_t ir_data_process;
TaskHandle_t ir_process_handle;

unsigned int hal_read_timestamp(void)
{
	return IN32(0x9004c8);
}
void hal_ir_data_process(void *pvParameters)
{
	while(1)
	{
		xSemaphoreTake(ir_data_process, portMAX_DELAY);
	//	system_printf(" custom_value = 0x%x, data_value =0x%x \n",(infor_value & 0xffff),(infor_value>>16));
		if((infor_value & 0xffff) == COSTOM_CODE_HBS838)
		{
			switch(infor_value>>16)
			{
				case IR_KEY_1:
					system_printf("the key is 1-1， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_2:
					system_printf("the key is 1-2， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_3:
					system_printf("the key is 1-3， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_4:
					system_printf("the key is 1-4， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_5:
					system_printf("the key is 2-1， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_6:
					system_printf("the key is 2-2， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_7:
					system_printf("the key is 2-3， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_8:
					system_printf("the key is 2-4， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_9:
					system_printf("the key is 3-1， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_10:
					system_printf("the key is 3-2， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_11:
					system_printf("the key is 3-3， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_12:
					system_printf("the key is 3-4， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_13:
					system_printf("the key is 4-1， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_14:
					system_printf("the key is 4-2， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_15:
					system_printf("the key is 4-3， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_16:
					system_printf("the key is 4-4， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_17:
					system_printf("the key is 5-1， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_18:
					system_printf("the key is 5-2， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_19:
					system_printf("the key is 5-3， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_20:
					system_printf("the key is 5-4， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_21:
					system_printf("the key is 6-1， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_22:
					system_printf("the key is 6-2， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_23:
					system_printf("the key is 6-3， receive_repeat = %d \n",receive_repeat);
					break;
				
				case IR_KEY_24:
					system_printf("the key is 6-4， receive_repeat = %d \n",receive_repeat);
					break;

				default:
					system_printf("the key value is	error\n");
					break;
			}
		}
	}
}

void hal_ir_callback(void *data)
{
//	system_printf("entry ir callback ! \n");	
	unsigned int test_time1 = 0;

	test_time1 = test_time0;
	test_time0 = hal_read_timestamp();
	
	// leader code 	
	if((IR_LEADER_HIGH_LEVEL_MIN < (test_time0 - test_time1)) && ((test_time0 - test_time1) < IR_LEADER_HIGH_LEVEL_MAX))
	{
		receive_status = 1;
		infor_value = 0;
		receive_repeat = 0;
	}
	
	// custom & data code 
	if(receive_status == 1)
	{
		if((IR_INFOR_LOW_LEVEL_MIN < (test_time0 - test_time1)) && ((test_time0 - test_time1) < IR_INFOR_LOW_LEVEL_MAX))		//logic '0'	
		{
			infor_value = infor_value >> 1;
			receive_num++;
		}
		else if((IR_INFOR_HIGH_LEVEL_MIN < (test_time0 - test_time1)) && ((test_time0 - test_time1) < IR_INFOR_HIGH_LEVEL_MAX))//logic '1'
		{
			infor_value = (infor_value >> 1) | 0x80000000;
			receive_num++;
		}
		if(receive_num == COSTOM_DATA_BITS_NUM)
		{
			receive_num = 0;
			receive_status = 2;
		}
	}
	
	// end code 
	if(receive_status == 2)
	{
		if((IR_STOP_HIGH_LEVEL_MIN<(test_time0 - test_time1)) && ((test_time0 - test_time1)<IR_STOP_HIGH_LEVEL_MAX))
		{
			receive_status = 3;
		}
		
	}

	//repeat code
	if(receive_status == 3)
	{
		if((IR_LEADER_END_LEVEL_MIN < (test_time0 - test_time1)) && ((test_time0 - test_time1) < IR_LEADER_END_LEVEL_MAX))
		{
			receive_repeat++;
			xSemaphoreGive(ir_data_process);
		}	
	}	
	
}


void hal_ir_init(void)
{
	gpio_isr_init();
	PIN_FUNC_SET(IO_MUX0_GPIO4, FUNC_GPIO4_GPIO4);
	gpio_irq_level(DRV_GPIO_4,0x5);	//interrupt by negative edge
	gpio_irq_callback_register(DRV_GPIO_4, hal_ir_callback, NULL);
	gpio_irq_unmusk(DRV_GPIO_4);
	xTaskCreate(hal_ir_data_process,(const char *)"ir_data_process", 2048, NULL, 4, &ir_process_handle);
	ir_data_process = xSemaphoreCreateCounting(32, 0);
}


#if 0
//test CMD code 
void hal_ir_gpio_read()
{
	unsigned char ir_value[10] = {0};
	unsigned char * gpio_value = ir_value;

	gpio_read(DRV_GPIO_4,(uint32_t *)gpio_value);
	system_printf("gpio_value = 0x%x\n",*gpio_value);
}
static int ir_read_gpio_test(cmd_tbl_t *t, int argc, char *argv[])
{
	hal_ir_init();
    hal_ir_gpio_read();
    return CMD_RET_SUCCESS;
}
CMD(ir_read, ir_read_gpio_test, "ir_read", "ir_read");


static int ir_write_test(cmd_tbl_t *t, int argc, char *argv[])
{
	int i=0;
	hal_ir_init();
	hal_ir_data_printf();
    return CMD_RET_SUCCESS;
}


CMD(ir_write, ir_write_test, "ir_write", "ir_write");
#endif


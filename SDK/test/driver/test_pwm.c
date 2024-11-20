/******************************************************************************
*
 * Copyright by Transa Semi.
 *
 * File Name:    
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       V1.0
 * Author:        liangyu
 * Date:          2019-8-14
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  
*******************************************************************************
*/

#ifndef _FILENAME_H
#define _FILENAME_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/

#include "drv_pwm.h"
#include "system_common.h"



/****************************************************************************
* 	                                        Macros
****************************************************************************/


/******************************************************************************
*
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
 
*******************************************************************************
*/




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
	static int test_pwm_init(cmd_tbl_t *t, int argc, char *argv[])
	{
		int ret = 0;
		int channel;
		if(argc != 2)
		{
			system_printf("The Number Of Argv Is Not 2");
			return CMD_RET_FAILURE;
		}
		
		system_printf("test_pwm_init\n");
		channel = atoi(argv[1]);
		ret = pwm_init(channel);
		return ret;
	}

	SUBCMD(test,
    pwminit,
    test_pwm_init,
    "pwm init test",
    "test pwminit");

	static int test_pwm_deinit(cmd_tbl_t *t, int argc, char *argv[])
	{
		int ret = 0;
		int channel;
		if(argc != 2)
		{
			system_printf("The Number Of Argv Is Not 2");
			return CMD_RET_FAILURE;
		}
		
		system_printf("test_pwm_deinit\n");
		channel = atoi(argv[1]);
		ret = pwm_deinit(channel);
		return ret;
	}

	SUBCMD(test,
    pwmdeinit,
    test_pwm_deinit,
    "pwm deinit test",
    "test pwmdeinit");



	static int test_pwm_config(cmd_tbl_t *t, int argc, char *argv[])
	{
		int ret = 0;
		int channel,freq,duty_ratio;
		if(argc != 4)
		{
			system_printf("The Number Of Argv Is Not 4");
			return CMD_RET_FAILURE;
		}
		
		system_printf("test_pwm_config\n");
		channel = atoi(argv[1]);
		freq = atoi(argv[2]);
		duty_ratio = atoi(argv[3]);
		ret = pwm_config(channel,freq,duty_ratio);
		return ret;
	}

	SUBCMD(test,
    pwmconfig,
    test_pwm_config,
    "pwm config test",
    "test pwmconfig");

	static int test_pwm_start(cmd_tbl_t *t, int argc, char *argv[])
	{
		int ret = 0;
		int channel;
		if(argc != 2)
		{
			system_printf("The Number Of Argv Is Not 2");
			return CMD_RET_FAILURE;
		}
		
		system_printf("test_pwm_start\n");
		channel = atoi(argv[1]);
		ret = pwm_start(channel);
		return ret;
	}

	SUBCMD(test,
    pwmstart,
    test_pwm_start,
    "pwm start test",
    "test pwmstart");	

	static int test_pwm_stop(cmd_tbl_t *t, int argc, char *argv[])
	{
		int ret = 0;
		int channel;
		if(argc != 2)
		{
			system_printf("The Number Of Argv Is Not 2");
			return CMD_RET_FAILURE;
		}
		
		system_printf("test_pwm_stop\n");
		channel = atoi(argv[1]);
		ret = pwm_stop(channel);
		return ret;
	}

	SUBCMD(test,
    pwmstop,
    test_pwm_stop,
    "pwm stop test",
    "test pwmstop");	

	
#endif/*_FILENAME_H*/





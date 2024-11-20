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

#include "wdt.h"
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
	static int test_wdt_err(cmd_tbl_t *t, int argc, char *argv[])
	{
		system_printf("cmd_wdt_err\n");
		while(1);
		return CMD_RET_SUCCESS;
	}

	SUBCMD(test,
    werr,
    test_wdt_err,
    "watch dog err test",
    "t wtderr");
	
	static int test_wtd_stop(cmd_tbl_t *t, int argc, char *argv[])
	{
		system_printf("cmd_wdt_stop\n");
		wtd_set_stop();
		return CMD_RET_SUCCESS;
	}

	SUBCMD(test,
    wstop,
    test_wtd_stop,
    "watch dog stop",
    "t wstop");

	static int test_wtd_resume(cmd_tbl_t *t, int argc, char *argv[])
	{
		system_printf("cmd_wdt_resume\n");
		wtd_resume();
		return CMD_RET_SUCCESS;
	}

	SUBCMD(test,
    wresume,
    test_wtd_resume,
    "watch dog resume",
    "t wresume");

	
	static int test_wtd_reset(cmd_tbl_t *t, int argc, char *argv[])
	{
		int ret = 0;
		int mode;
		if(argc != 2)
		{
			system_printf("The Number Of Argv Is Not 2");
			return CMD_RET_FAILURE;
		}
		
		system_printf("cmd_wdt_resume\n");
		mode = atoi(argv[1]) == 1 ? WDT_RESET_CHIP_TYPE_RESYSTEM : WDT_RESET_CHIP_TYPE_REBOOT;
		
		wdt_reset_chip(mode);
		return CMD_RET_SUCCESS;
	}


	SUBCMD(test,
    wreset,
    test_wtd_reset,
    "watch dog reset",
    "t wreset");
	
#endif/*_FILENAME_H*/




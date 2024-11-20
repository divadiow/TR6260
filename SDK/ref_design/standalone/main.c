/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: main.c(standalone)   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2019-1-4
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
#include "system.h"
#include "standalone.h"
#include "wdt.h"
#include "drv_adc.h"

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
extern void hal_rtc_init(void);
extern void wifi_drv_init(void);
extern sys_err_t wifi_load_nv_cfg();
/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/

/*******************************************************************************
 * Function: Drv_init
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
int32_t Drv_Init(void)
{
	
	hal_rtc_init();
	return 0;
}

/*******************************************************************************
 * Function: Os_init
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
int32_t Os_Init(void)
{

	system_task_init();
	partion_init();
	easyflash_init();
#ifdef _USE_PSM
	TrPsmInit();
#endif
	return 0;
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

int32_t Os_Scheduler_Init(void)
{

	vTaskStartScheduler();
	
	return 0;
}

/*******************************************************************************
 * Function: Wifi_init
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
int32_t Wifi_Init(void)
{
	int option = TR_PSM_OPTION_RF_INIT;
	ef_get_env_blob(NV_PSM_OPTION, &option, 2, NULL);

	if(option == TR_PSM_OPTION_RF_INIT){
		wifi_drv_init();
		standalone_main();
		wifi_load_nv_cfg();
	}else{
		extern int nrf_rtc_cal(void);
		nrf_rtc_cal();
    }

	util_cli_freertos_init(util_cmd_run_command);
	
	
    return 0;
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
int32_t App_Init(void)
{
	time_check_temp();
	return 0;

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
int main()
{
	#if 0
	hal_rf_init();
	hal_phy_init();
	hal_lmac_init();
	hal_lmac_set_dl_callback(nrc_mac_rx);

	util_cli_freertos_init(util_cmd_run_command);

    umac_info_init(0, 0);	// STA
	umac_scan_init( nrc_scan_done );

	system_task_init();
	standalone_main();
	TrPsmInit();
	hal_rtc_init();

	system_default_setting(0);

	vTaskStartScheduler();
	#else
	//-------------------Drv init--------------------
	  Drv_Init();
    //-------------------OS init---------------------
	  Os_Init();
	//-------------------WIFI init-------------------
	  Wifi_Init();
	//-------------------APP init--------------------
	  App_Init();
	//-------------------WTD init-------------------
	  Hal_WtdSoftTask_Init();
	//-------------------Os scheduler init-----------
	  Os_Scheduler_Init();
	#endif
	return 0;
}

/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: hal_trim.c   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangchao
 * Date:          2019-6-28
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
#include "drv_spiflash.h"
#include "drv_trim.h"

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

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/

/*******************************************************************************
 * Function: hal_trim_read_txpower
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
int32_t hal_trim_read_txpower(int8_t *pTxPwrB,int8_t *pTxPwrG,int8_t *pTxPwrN)
{

	#ifdef _USR_TR6260_3 
	/*read calculation data from efuse*/
	#else
	/*read calculation data from OTP*/
	#endif

	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function: hal_trim_read_buck
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
int32_t hal_trim_read_buck(int8_t *pBuck)
{
	
	#ifdef _USR_TR6260_3 
		/*read calculation data from efuse*/
	#else
		/*read calculation data from OTP*/
	#endif
	
	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function: hal_trim_read_ical
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
int32_t hal_trim_read_ical(int8_t *pIcal)
{
		
	#ifdef _USR_TR6260_3 
			/*read calculation data from efuse*/
	#else
			/*read calculation data from OTP*/
	#endif
		
	return DRV_SUCCESS;
}


/*******************************************************************************
 * Function: hal_trim_read_rcal
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
int32_t hal_trim_read_rcal(int8_t *pRcal)
{
			
	#ifdef _USR_TR6260_3 
				/*read calculation data from efuse*/
	#else
				/*read calculation data from OTP*/
	#endif
			
	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function: hal_cal_param_get
 * Description: obtain calibration parameter
 * Parameters: 
 *   Input:	addr-- calibration parameter address
 *			length -- calibration parameter length
 *			
 *   Output:	buf -- the store buffer for calibration parameter
 *
 * Returns: 
 *                  return 0 on success, otherwise will be failed
 *
 * Others: 
 ********************************************************************************/
int  hal_cal_param_get(unsigned int addr, int length, char * buf)
{
	if((addr < CAL_PARAM_BASE) || (addr >= CAL_PARAM_BASE_MAX) || (buf == NULL))
	{
		//return DRV_ERR_INVALID_PARAM;
	}
	
	return spiFlash_OTP_Read(addr, 1, (unsigned char *)(buf));
}

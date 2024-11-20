/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:drv_trim.h    
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

#ifndef _DRV_TRIM_H
#define _DRV_TRIM_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
****************************************************************************/


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
int32_t hal_trim_read_txpower(int8_t *pTxPwrB,int8_t *pTxPwrG,int8_t *pTxPwrN);
int32_t hal_trim_read_buck(int8_t *pBuck);
int32_t hal_trim_read_ical(int8_t *pIcal);
int32_t hal_trim_read_ical(int8_t *pRcal);



#define CAL_PARAM_BASE				0x1000

#define CAL_PARAM_BASE_BGM_0V9		CAL_PARAM_BASE
#define CAL_PARAM_LEN_BGM_0V9			1

#define CAL_PARAM_BASE_RCAL_TRIM		(CAL_PARAM_BASE_BGM_0V9+CAL_PARAM_LEN_BGM_0V9)
#define CAL_PARAM_LEN_RCAL_TRIM		1

#define CAL_PARAM_BASE_ICAL_TRIM		(CAL_PARAM_BASE_RCAL_TRIM+CAL_PARAM_LEN_RCAL_TRIM)
#define CAL_PARAM_LEN_ICAL_TRIM		1

#define CAL_PARAM_BASE_MAIN_LDO		(CAL_PARAM_BASE_ICAL_TRIM+CAL_PARAM_LEN_ICAL_TRIM)
#define CAL_PARAM_LEN_MAIN_LDO		1

#define CAL_PARAM_BASE_BUCK_1V45		(CAL_PARAM_BASE_MAIN_LDO+CAL_PARAM_LEN_MAIN_LDO)
#define CAL_PARAM_LEN_BUCK_1V45		1

#define CAL_PARAM_BASE_MAX			(CAL_PARAM_BASE_BUCK_1V45 + CAL_PARAM_LEN_BUCK_1V45)

/*******************************************************************************
 * Function: hal_cal_param_get
 * Description: obtain calibration parameter
 * Parameters: 
 *   Input:	addr-- calibration parameter address,   e.g. CAL_PARAM_BASE_XXXXX
 *			length -- calibration parameter length,   e.g. CAL_PARAM_LEN_XXXXX
 *			
 *   Output:	buf -- the store buffer for calibration parameter
 *
 * Returns: 
 *                  return 0 on success, otherwise will be failed
 *
 * Others: 
 ********************************************************************************/
int  hal_cal_param_get(unsigned int addr, int length, char * buf);

#endif/*_DRV_TRIM_H*/


/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: amtNV.h   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        liangyu
 * Date:          2020-2-24
 * History 1: 
 *     Date: 2020-04-17
 *     Version:
 *     Author: wangxia
 *     Modification:  add aliyun info in AMT partition

 * History 2: 
  ********************************************************************************/

#ifndef AMTNV_H
#define AMTNV_H

//#include "nv_config.h"


/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
****************************************************************************/
/**************************************************************************** 
*
*							AMT  NV  PARTION
*
*	AMT_NV_XXX			--- the address of the NV parameter
*	AMT_NV_XXX_len		--- the length of the NV parameter
*	AMT_NV_XXX_OFFSET	--- the offset length from the next NV parameter
*	
****************************************************************************/


#define  MAC_OTP_ADDR_GD25Q80E_FLASH 		 0x1030



extern unsigned int amt_partion_base;
#define AMT_PARTION_SIZE					4096

#define AMT_PARTION_END						(amt_partion_base + AMT_PARTION_SIZE)

#define AMT_NV_SN							amt_partion_base
#define AMT_NV_SN_LEN						64
#define AMT_NV_SN_OFFSET					72
	
#define AMT_NV_MAC							(AMT_NV_SN + AMT_NV_SN_OFFSET)
#define AMT_NV_MAC_LEN						18
#define AMT_NV_MAC_OFFSET					24
	
#define AMT_NV_PIN_FLAG						(AMT_NV_MAC + AMT_NV_MAC_OFFSET)
#define AMT_NV_PIN_FLAG_LEN					2
#define AMT_NV_PIN_FLAG_OFFSET				8
	
#define AMT_NV_PSM_FLAG						(AMT_NV_PIN_FLAG + AMT_NV_PIN_FLAG_OFFSET)
#define AMT_NV_PSM_FLAG_LEN					2
#define AMT_NV_PSM_FLAG_OFFSET				8
	
#define AMT_NV_WIFI_FLAG					(AMT_NV_PSM_FLAG + AMT_NV_PSM_FLAG_OFFSET)
#define AMT_NV_WIFI_FLAG_LEN				2
#define AMT_NV_WIFI_FLAG_OFFSET				8
	
#define AMT_NV_UPDATE_FLAG					(AMT_NV_WIFI_FLAG + AMT_NV_WIFI_FLAG_OFFSET)
#define AMT_NV_UPDATE_FLAG_LEN				2
#define AMT_NV_UPDATE_FLAG_OFFSET			8
	
#define AMT_NV_SN_FLAG						(AMT_NV_UPDATE_FLAG + AMT_NV_UPDATE_FLAG_OFFSET)
#define AMT_NV_SN_FLAG_LEN					2
#define AMT_NV_SN_FLAG_OFFSET				8
	
#define AMT_NV_MAC_FLAG						(AMT_NV_SN_FLAG + AMT_NV_SN_FLAG_OFFSET)
#define AMT_NV_MAC_FLAG_LEN					2
#define AMT_NV_MAC_FLAG_OFFSET				8
	
#define AMT_NV_FREQOFFSET_FLAG				(AMT_NV_MAC_FLAG + AMT_NV_MAC_FLAG_OFFSET)
#define AMT_NV_FREQOFFSET_FLAG_LEN			2
#define AMT_NV_FREQOFFSET_FLAG_OFFSET		8
	
#define AMT_NV_FREQOFFSET					(AMT_NV_FREQOFFSET_FLAG + AMT_NV_FREQOFFSET_FLAG_OFFSET)
#define AMT_NV_FREQOFFSET_LEN				8
#define AMT_NV_FREQOFFSET_OFFSET			16
	
#define AMT_NV_TXPOWER_FLAG					(AMT_NV_FREQOFFSET + AMT_NV_FREQOFFSET_OFFSET)
#define AMT_NV_TXPOWER_FLAG_LEN				2
#define AMT_NV_TXPOWER_FLAG_OFFSET			8
	
#define AMT_NV_TXPOWER_HTCH1				(AMT_NV_TXPOWER_FLAG + AMT_NV_TXPOWER_FLAG_OFFSET)
#define AMT_NV_TXPOWER_HTCH1_LEN			8
#define AMT_NV_TXPOWER_HTCH1_OFFSET			16
	
#define AMT_NV_TXPOWER_HTCH6				(AMT_NV_TXPOWER_HTCH1 + AMT_NV_TXPOWER_HTCH1_OFFSET)
#define AMT_NV_TXPOWER_HTCH6_LEN			8
#define AMT_NV_TXPOWER_HTCH6_OFFSET			16
	
#define AMT_NV_TXPOWER_HTCH11				(AMT_NV_TXPOWER_HTCH6 + AMT_NV_TXPOWER_HTCH6_OFFSET)
#define AMT_NV_TXPOWER_HTCH11_LEN			8
#define AMT_NV_TXPOWER_HTCH11_OFFSET		16
	
#define AMT_NV_TXPOWER_HTCH3				(AMT_NV_TXPOWER_HTCH11 + AMT_NV_TXPOWER_HTCH11_OFFSET)
#define AMT_NV_TXPOWER_HTCH3_LEN			8
#define AMT_NV_TXPOWER_HTCH3_OFFSET			16
	
#define AMT_NV_TXPOWER_HTCH8				(AMT_NV_TXPOWER_HTCH3 + AMT_NV_TXPOWER_HTCH3_OFFSET)
#define AMT_NV_TXPOWER_HTCH8_LEN			8
#define AMT_NV_TXPOWER_HTCH8_OFFSET			16
	
#define AMT_NV_TXPOWER_NONHTCH1				(AMT_NV_TXPOWER_HTCH8 + AMT_NV_TXPOWER_HTCH8_OFFSET)
#define AMT_NV_TXPOWER_NONHTCH1_LEN			8
#define AMT_NV_TXPOWER_NONHTCH1_OFFSET		16
	
#define AMT_NV_TXPOWER_NONHTCH6				(AMT_NV_TXPOWER_NONHTCH1 + AMT_NV_TXPOWER_NONHTCH1_OFFSET)
#define AMT_NV_TXPOWER_NONHTCH6_LEN			8
#define AMT_NV_TXPOWER_NONHTCH6_OFFSET		16
	
#define AMT_NV_TXPOWER_NONHTCH11			(AMT_NV_TXPOWER_NONHTCH6 + AMT_NV_TXPOWER_NONHTCH6_OFFSET)
#define AMT_NV_TXPOWER_NONHTCH11_LEN		8
#define AMT_NV_TXPOWER_NONHTCH11_OFFSET		16
	
#define AMT_NV_TXPOWER_11BCH1				(AMT_NV_TXPOWER_NONHTCH11 + AMT_NV_TXPOWER_NONHTCH11_OFFSET)
#define AMT_NV_TXPOWER_11BCH1_LEN			8
#define AMT_NV_TXPOWER_11BCH1_OFFSET		16
	
#define AMT_NV_TXPOWER_11BCH6				(AMT_NV_TXPOWER_11BCH1 + AMT_NV_TXPOWER_11BCH1_OFFSET)
#define AMT_NV_TXPOWER_11BCH6_LEN			8
#define AMT_NV_TXPOWER_11BCH6_OFFSET		16
	
#define AMT_NV_TXPOWER_11BCH11				(AMT_NV_TXPOWER_11BCH6 + AMT_NV_TXPOWER_11BCH6_OFFSET)
#define AMT_NV_TXPOWER_11BCH11_LEN			8
#define AMT_NV_TXPOWER_11BCH11_OFFSET		16
	
#define AMT_NV_CALIBRATION					(AMT_NV_TXPOWER_11BCH11 + AMT_NV_TXPOWER_11BCH11_OFFSET)
#define AMT_NV_CALIBRATION_LEN				2
#define AMT_NV_CALIBRATION_OFFSET			8

#define AMT_NV_ALIYUN_PDKEY					(AMT_NV_CALIBRATION + AMT_NV_CALIBRATION_OFFSET)
#define AMT_NV_ALIYUN_PDKEY_LEN				32
#define AMT_NV_ALIYUN_PDKEY_OFFSET			40

#define AMT_NV_ALIYUN_PDSECRET				(AMT_NV_ALIYUN_PDKEY + AMT_NV_ALIYUN_PDKEY_OFFSET)
#define AMT_NV_ALIYUN_PDSECRET_LEN			32
#define AMT_NV_ALIYUN_PDSECRET_OFFSET		40

#define AMT_NV_ALIYUN_DEVNAME				(AMT_NV_ALIYUN_PDSECRET + AMT_NV_ALIYUN_PDSECRET_OFFSET)
#define AMT_NV_ALIYUN_DEVNAME_LEN			32
#define AMT_NV_ALIYUN_DEVNAME_OFFSET		40

#define AMT_NV_ALIYUN_DEVSECRET				(AMT_NV_ALIYUN_DEVNAME + AMT_NV_ALIYUN_DEVNAME_OFFSET)
#define AMT_NV_ALIYUN_DEVSECRET_LEN			32
#define AMT_NV_ALIYUN_DEVSECRET_OFFSET		40

#define AMT_NV_ALIYUN_FLAG					(AMT_NV_ALIYUN_DEVSECRET + AMT_NV_ALIYUN_DEVSECRET_OFFSET)
#define AMT_NV_ALIYUN_FLAG_LEN				2
#define AMT_NV_ALIYUN_FLAG_OFFSET			8

#define AMT_NV_TEST_FLAG					(AMT_NV_ALIYUN_FLAG + AMT_NV_ALIYUN_FLAG_OFFSET)
#define AMT_NV_TEST_FLAG_LEN				10
#define AMT_NV_TEST_FLAG_OFFSET				16
	
// continuous updating .....
	
#define AMT_NV_MAX							(amt_partion_base + 2048 -1)//(AMT_NV_MAC_FLAG + AMT_NV_MAC_FLAG_OFFSET)


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
#ifdef AMT
/*******************************************************************************
 * Function: amt_nv_write
 * Description: AMT NV parameter write interface
 * Parameters: 
 *   Input:	addr -- the specified address  of nv
 *			buf -- the buffer address of the update data to be written
 *			len -- the length of the update data to be written
 *			
 *   Output:
 *
 * Returns:    0  -- sucess
 *                -4 -- the input parameter is invalid
 *                -5 -- failed to malloc memory 
 * Others: 
 ********************************************************************************/
int  amt_nv_write(unsigned int addr, unsigned char * buf, unsigned int len);

/*******************************************************************************
 * Function: amt_nv_init
 * Description: AMT NV parameter initialization
 * Parameters: 
 *   Input:
 *			
 *   Output:
 *
 * Returns: 
 *
 * Others: 
 ********************************************************************************/
void amt_nv_init(void);
#endif

/*******************************************************************************
 * Function: amt_nv_read
 * Description: AMT NV parameter read interface
 * Parameters: 
 *   Input:	addr -- the specified address  of nv
 *			buf -- the buffer address  to read
 *			len -- the buffer length  to read
 *
 *   Output:
 *
 * Returns: 
 *
 * Others: 
 ********************************************************************************/
int  amt_nv_read(unsigned int addr, unsigned char * buf, unsigned int len);
#endif

#ifdef TUYA_SDK_ADPT
int amt_mac_write(unsigned char * MAC);
#endif


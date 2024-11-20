/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:  drv_spiflash.h  
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangchao
 * Date:          2019-1-4
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef	_DRV_SPIFLASH_H
#define _DRV_SPIFLASH_H

/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
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
int spiFlash_OTP_Read(int addr,int length,unsigned char *pdata);
void spiFlash_OTP_Lock(int LB);
int spiFlash_OTP_Se(unsigned int addr);
int hal_spifiash_OTPWrite(unsigned int addr, unsigned int len, unsigned char * buf);

int hal_spiflash_read(unsigned int addr, unsigned char * buf, unsigned int len);
int hal_spifiash_write(unsigned int addr, unsigned char * buf, unsigned int len);
int hal_spiflash_erase(unsigned int addr,  unsigned int len);
void hal_spiflash_init(void);
void hal_spiflash_exit(void);

#endif/*_DRV_SPIFLASH_H*/


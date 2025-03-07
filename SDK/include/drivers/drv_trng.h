/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: drv_trng.h   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2019-1-3
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef _HAL_TRNG_H
#define _HAL_TRNG_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/
#include "system.h"
#include "soc_top_reg.h"


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
uint32_t trng_read(void);

#endif/*_HAL_TRNG_H*/




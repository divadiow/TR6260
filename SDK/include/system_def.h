/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:    system_def.h
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       V1.0
 * Author:        lixiao
 * Date:          2018-12-21
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef _SYSTEM_DEF_H
#define _SYSTEM_DEF_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
****************************************************************************/
#define BIT0    (0x00000001L << 0)
#define BIT1    (0x00000001L << 1)
#define BIT2    (0x00000001L << 2)
#define BIT3    (0x00000001L << 3)
#define BIT4    (0x00000001L << 4)
#define BIT5    (0x00000001L << 5)
#define BIT6    (0x00000001L << 6)
#define BIT7    (0x00000001L << 7)
#define BIT8    (0x00000001L << 8)
#define BIT9    (0x00000001L << 9)
#define BIT10   (0x00000001L << 10)
#define BIT11   (0x00000001L << 11)
#define BIT12   (0x00000001L << 12)
#define BIT13   (0x00000001L << 13)
#define BIT14   (0x00000001L << 14)
#define BIT15   (0x00000001L << 15)
#define BIT16   (0x00000001L << 16)
#define BIT17   (0x00000001L << 17)
#define BIT18   (0x00000001L << 18)
#define BIT19   (0x00000001L << 19)
#define BIT20   (0x00000001L << 20)
#define BIT21   (0x00000001L << 21)
#define BIT22   (0x00000001L << 22)
#define BIT23   (0x00000001L << 23)
#define BIT24   (0x00000001L << 24)
#define BIT25   (0x00000001L << 25)
#define BIT26   (0x00000001L << 26)
#define BIT27   (0x00000001L << 27)
#define BIT28   (0x00000001L << 28)
#define BIT29   (0x00000001L << 29)
#define BIT30   (0x00000001L << 30)
#define BIT31   (0x00000001L << 31)
#define BITZero    (0x00000000L)
        
#define BIT(x) (1U << (x))

/****************************************************************************
* 	                                        Types
****************************************************************************/
// Type Definition
#include <stdint.h>
#include <stdbool.h> 
typedef signed char                 int8_t;
typedef unsigned char               uint8_t;
typedef signed short                int16_t;
typedef unsigned short              uint16_t;
typedef signed long long            int64_t;
typedef unsigned long long          uint64_t;

#ifndef bool
#define bool uint32_t
#endif
    
    
typedef int32_t sys_err_t;

/* Definitions for error constants. */

#define SYS_OK          	       0x0
#define SYS_ERR                    0x1

// for psm
#define SYS_PM_NOT_HANDLED         0x10
#define SYS_DVEICE_NOT_SUPPORT     0x11
#define SYS_SOC_SLEEP_MODE_ERR     0x12
#define SYS_SYS_SLEEP_MODE_ERR     0x13

// for common error
#define SYS_ERR_NO_MEM          	0x101
#define SYS_ERR_INVALID_ARG     	0x102
#define SYS_ERR_INVALID_STATE   	0x103
#define SYS_ERR_INVALID_SIZE    	0x104
#define SYS_ERR_NOT_FOUND       	0x105
#define SYS_ERR_NOT_SUPPORTED   	0x106
#define SYS_ERR_TIMEOUT         	0x107
#define SYS_ERR_INVALID_RESPONSE    0x108
#define SYS_ERR_INVALID_CRC     	0x109
#define SYS_ERR_INVALID_VERSION     0x10A
#define SYS_ERR_INVALID_MAC     	0x10B

// for wifi
#define SYS_ERR_WIFI_BASE       	0x3000 /*!< Starting number of WiFi error codes */
#define SYS_ERR_WIFI_MODE           0x3001 /*wifi operate mode error*/
#define SYS_ERR_WIFI_BUSY           0X3002 /*wifi busy*/

/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/



#endif/*_SYSTEM_DEF_H*/


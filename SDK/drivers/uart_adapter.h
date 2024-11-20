/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:  uart_adapter.h  
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        lixinghao/weifeiting
 * Date:          2020-3-19
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef _UART_ADAPTER_H
#define _UART_ADAPTER_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
****************************************************************************/
#define FRAME_HEAD 0xAA
#define FRAME_TAIL 0x55
#define FRAME_MIN_LEN 2
#define FRAME_HEAD_LEN 1
#define FRAME_LENGTH_FIELD_LEN 1
#define FRAME_TAIL_LEN 2
#define FRAME_CHECKSUM_LEN 1

/****************************************************************************
* 	                                        Types
****************************************************************************/
typedef struct _rx_frame_info_t
{
	unsigned int find_frame_head_invalid_data_len;
	unsigned char frame_len;
}frame_info;
/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/
unsigned char send_frame_to_mcu(const unsigned char * buf, unsigned char len);
unsigned char get_frame_from_mcu(unsigned char *buf);
void uart_open();

#endif/*_UART_ADAPTER_H*/


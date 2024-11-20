/*******************************************************************************
* Copyright by Transa Semi.
*
* File Name:uart_adapter.c    
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

/****************************************************************************
* 	                                           Include files
****************************************************************************/
#include "uart_adapter.h"
#include "drv_uart.h"
#include "system_common.h"
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
TaskHandle_t uart_read_handle;
uart_handle_t uart_tx_handle = NULL;
uart_handle_t uart_rx_handle = NULL;

/****************************************************************************
* 	                                          Global Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/
void filling_data_to_uart_buf(unsigned char *buf, unsigned char data, unsigned data_len)
{
	buf[0] = data;
	hal_uart_rec_end(data_len);
}

unsigned char filling_frame_head_to_uart_buf()
{
	unsigned int avail_len = hal_uart_get_txbuff_avail_len();
	if(0 == avail_len)
	{
		return 0;
	}
	
	unsigned char *sendBuf = (unsigned char *)hal_uart_get_tx_buff();
	filling_data_to_uart_buf(sendBuf, FRAME_HEAD, 1);
		
	return 1;
}

unsigned char filling_frame_len_to_uart_buf(unsigned char len)
{
	unsigned int avail_len = hal_uart_get_txbuff_avail_len();
	if(0 == avail_len)
	{
		return 0;
	}
	
	unsigned char *sendBuf = (unsigned char *)hal_uart_get_tx_buff();
	filling_data_to_uart_buf(sendBuf, len, 1);
	return 1;
}

unsigned char filling_user_data_to_uart_buf(const unsigned char * buf, unsigned char len)
{
   unsigned int i=0;
	for( i = 1; i < (len + 1); i++)
	{
		unsigned int avail_len = hal_uart_get_txbuff_avail_len();
		if(0 == avail_len)
		{
			return 0;
		}

		unsigned char *sendBuf = (unsigned char *)hal_uart_get_tx_buff();
		filling_data_to_uart_buf(sendBuf, buf[i], 1);
	}

	return 1;
}

unsigned char filling_checksum_to_uart_buf(const unsigned char * buf, unsigned char len)
{
	unsigned int avail_len = hal_uart_get_txbuff_avail_len();
	if(0 == avail_len)
	{
		return 0;
	}
	
	unsigned int checksum = 0;
	unsigned int i=0;
	for(i = 1; i < (len + 1); i++)
	{
		checksum += buf[i];
	}
	checksum += len;
	unsigned char *sendBuf = (unsigned char *)hal_uart_get_tx_buff();
	filling_data_to_uart_buf(sendBuf, (checksum & 0xFF), 1);

	return 1;
}

unsigned char filling_frame_tail_to_uart_buf()
{
	unsigned int avail_len = hal_uart_get_txbuff_avail_len();
	if(0 == avail_len)
	{
		return 0;
	}
	
	unsigned char *sendBuf = (unsigned char *)hal_uart_get_tx_buff();
	filling_data_to_uart_buf(sendBuf, FRAME_TAIL, 1);
	return 1;
}


unsigned char send_frame_to_mcu(const unsigned char * buf, unsigned char len)
{
	if(NULL == buf || len < FRAME_MIN_LEN)
	{
		system_printf("send_frame_to_mcu: buf or len is err\n");
		return 0;
	}
	
	unsigned char *sendBuf = (unsigned char *)hal_uart_get_tx_buff();
	
	if(0 == filling_frame_head_to_uart_buf())
	{
		system_printf("send_frame_to_mcu: filling_frame_head_to_uart_buf err\n");
		return 0;
	}

	if(0 == filling_frame_len_to_uart_buf(len))
	{
		system_printf("send_frame_to_mcu: filling_frame_len_to_uart_buf err\n");
		return 0;
	}

	if(0 == filling_user_data_to_uart_buf(buf, len))
	{
		system_printf("send_frame_to_mcu: filling_user_data_to_uart_buf err\n");
		return 0;
	}

	if(0 == filling_checksum_to_uart_buf(buf, len))
	{
		system_printf("send_frame_to_mcu: filling_checksum_to_uart_buf err\n");
		return 0;
	}

	if(0 == filling_frame_tail_to_uart_buf())
	{
		system_printf("send_frame_to_mcu: filling_frame_tail_to_uart_buf err\n");
		return 0;
	}

	hal_uart_write_data(uart_tx_handle);
	return 1;
}


unsigned char get_frame_len(unsigned char *buf)
{
	buf += FRAME_HEAD_LEN;
	return *buf;
}

unsigned char *get_full_frame_from_buf(frame_info *frame_info)
{
	unsigned int avail_len = hal_uart_get_rx_len();
	unsigned char *rcv_buf = (unsigned char *)hal_uart_get_rx_buff();
	unsigned char data_len = 0;
	unsigned char *rcv_buf_tail = NULL;
	unsigned char invalid_data_len = 0;

	while(0 != avail_len)
	{
		if(FRAME_HEAD == *rcv_buf)
		{
			data_len = get_frame_len(rcv_buf);
			rcv_buf_tail = rcv_buf + data_len + FRAME_HEAD_LEN + FRAME_LENGTH_FIELD_LEN + FRAME_CHECKSUM_LEN;
			
			if(FRAME_TAIL == *rcv_buf_tail)
			{
				frame_info->find_frame_head_invalid_data_len = invalid_data_len;
				frame_info->frame_len = data_len;
				return rcv_buf;
			}
			else
			{
				avail_len--;
				invalid_data_len++;
				rcv_buf++;
			}
		}
		else
		{
			avail_len--;
			invalid_data_len++;
			rcv_buf++;
		}
	}

	return NULL;
}

bool is_checksum_valid(unsigned char *buf, unsigned char data_len)
{
	unsigned int checksum = 0;
	unsigned char *data_len_buf = buf + FRAME_HEAD_LEN;
	int i;
	
	for(i = 0; i < data_len + FRAME_LENGTH_FIELD_LEN; i++)
	{
		checksum += data_len_buf[i];
	}
	checksum = checksum & 0xFF;

	unsigned char *rcv_buf_tail = buf + data_len + FRAME_HEAD_LEN + FRAME_LENGTH_FIELD_LEN;
	if(*rcv_buf_tail == checksum)
	{
		return pdTRUE;
	}
	return pdFALSE;
}

void copy_data_to_buf(unsigned char *buf, unsigned char *frame_head, unsigned char data_len)
{
	unsigned char *data_len_buf = frame_head + FRAME_HEAD_LEN;
	memcpy(buf, data_len_buf, data_len + FRAME_LENGTH_FIELD_LEN);
}

unsigned char get_frame_from_mcu(unsigned char *buf)
{
	frame_info frame_info;
	unsigned char *frame_head = get_full_frame_from_buf(&frame_info);
	if(NULL == frame_head)
	{
		system_printf("get_frame_from_mcu: get_full_frame_from_buf err\n");
		return 0;
	}

	if(!is_checksum_valid(frame_head, frame_info.frame_len))
	{
		system_printf("get_frame_from_mcu: is_checksum_valid err\n");
		hal_uart_get_rx_data_end(frame_info.find_frame_head_invalid_data_len + frame_info.frame_len + FRAME_HEAD_LEN + FRAME_LENGTH_FIELD_LEN + FRAME_TAIL_LEN);
		return 0;
	}

	copy_data_to_buf(buf, frame_head, frame_info.frame_len);
	hal_uart_get_rx_data_end(frame_info.find_frame_head_invalid_data_len + frame_info.frame_len + FRAME_HEAD_LEN + FRAME_LENGTH_FIELD_LEN + FRAME_TAIL_LEN);

	return 1;
}

void uart_open()
{
	if (uart_tx_handle == NULL)
	{  
			uart_tx_handle = hal_uart_open_data(0,UART_ID_1, UART_DATA_BIT_8, 9600, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
			system_printf("uart open success\n");
	}
}




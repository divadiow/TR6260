/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:   hal_aes.c 
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2018-12-21
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
#include "soc_top_reg.h"
#include "drv_aes.h"



/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define AES_CTRL				 SOC_AES_BASE
#define AES_RSV0				(SOC_AES_BASE+0x04)
#define AES_DATA				(SOC_AES_BASE+0x08)
#define AES_RSV1				(SOC_AES_BASE+0x0C)
#define AES_KEY0				(SOC_AES_BASE+0x10)
#define AES_KEY1				(SOC_AES_BASE+0x14)
#define AES_KEY2				(SOC_AES_BASE+0x18)
#define AES_KEY3				(SOC_AES_BASE+0x1C)


#define AES_KEY_LEN_MAX				128 //128 bit key
#define AES_INPUT_DATA_LEN_MAX		16 //16 byte data
#define AES_DATA_FIFO_WIDTH			4  //4byte
#define AES_DATA_FIFO_DEPTH			4 // 32bit width,4 depth

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
#if 0
int32_t aes_128_encrypt(uint8_t *input,uint32_t input_len,uint8_t *key,uint8_t key_len,uint8_t *output)
{
	uint32_t RegTmp = 0;
	uint8_t  DataTemp_8[16];
	uint32_t DataTemp_32[4];
	uint8_t index=0;

	if((input_len > AES_INPUT_DATA_LEN_MAX)||(key_len > AES_KEY_LEN_MAX))
	{
		return DRV_ERR_INVALID_PARAM;
	}

	
	memset(DataTemp_32,0x0,sizeof(uint32_t)*4);
	memset(DataTemp_8,0x0,sizeof(uint8_t)*16);
	
	//1.check AES  busy status
	while((IN32(AES_CTRL)&0x2) == 0x2)//bit1: 1--->input data need,AES idle;0--->input data ready,AES work
	{
		break;
	}

	#if 0
	//2.clear FIFO
	for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
	{
		OUT32(AES_DATA,0x0);
	}
	#endif

    //3.set encode in ctrl
    OUT32(AES_CTRL, (IN32(AES_CTRL)&0xE));//bit0:  1---->decrypt;0---->encrypt

	
	//4.input key
	for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
	{
		DataTemp_32[index] = (*(key+4*index+3)<<24)+(*(key+4*index+2)<<16)+(*(key+4*index+1)<<8)+*(key+4*index);
		OUT32((AES_KEY0+4*index), DataTemp_32[index]);
	}
	memset(DataTemp_32,0x0,sizeof(uint32_t)*4);
	
	//5.input data
	if(input_len == AES_INPUT_DATA_LEN_MAX)
	{
		for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
		{
			DataTemp_32[index] = (*(input+4*index+3)<<24)+(*(input+4*index+2)<<16)+(*(input+4*index+1)<<8)+*(input+4*index);
			OUT32(AES_DATA, DataTemp_32[index]);
		}
	}
	else if(input_len < AES_INPUT_DATA_LEN_MAX)
	{
		if((input_len%AES_DATA_FIFO_WIDTH) == 0)
		{
			for(index=0;index<(AES_INPUT_DATA_LEN_MAX/input_len);index++)
			{
				DataTemp_32[index] = (*(input+4*index+3)<<24)+(*(input+4*index+2)<<16)+(*(input+4*index+1)<<8)+*(input+4*index);
				OUT32(AES_DATA, DataTemp_32[index]);
			}
			for(index=0;index<(AES_DATA_FIFO_DEPTH-(AES_INPUT_DATA_LEN_MAX/input_len));index++)
			{
				OUT32(AES_DATA, 0x0);
			}
			
		}
		else if((input_len%AES_DATA_FIFO_WIDTH) > 0)
		{
			for(index=0;index<(input_len/AES_DATA_FIFO_WIDTH);index++)
			{
				DataTemp_32[index] = (*(input+4*index+3)<<24)+(*(input+4*index+2)<<16)+(*(input+4*index+1)<<8)+*(input+4*index);
				OUT32(AES_DATA, DataTemp_32[index]);
			}
			if(input_len%AES_DATA_FIFO_WIDTH < AES_DATA_FIFO_WIDTH)
			{

				DataTemp_32[index+1] = 
			
			}

		}
		else
		{
		}
	}

	//6.wait encrypy finish
	while((IN32(AES_CTRL)&0x4) == 0x4)//bit2: 1--->work finish;0--->working
	{
		break;
	}

	//7.read data
	for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
	{
		 OUT32(AES_DATA,DataTemp_32[index]);	
	}
	
}
#endif

/*******************************************************************************
 * Function: aes_128_encrypt
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
int32_t aes_128_encrypt(uint8_t *input,uint32_t input_len,const uint8_t *key,const uint8_t key_len,uint8_t *output)
{
	uint32_t RegTmp = 0;
	uint8_t  DataTemp_8[16];
	uint32_t DataTemp_32[4];
	uint8_t index=0;
	int32_t outputlen = 0;
	if((input_len > AES_INPUT_DATA_LEN_MAX)||(key_len != AES_KEY_LEN_MAX)||((input_len%AES_DATA_FIFO_WIDTH) != 0))
	{
		return DRV_ERR_INVALID_PARAM;
	}

	
	memset(DataTemp_32,0x0,sizeof(uint32_t)*4);
	memset(DataTemp_8,0x0,sizeof(uint8_t)*16);


	//0.clock enable
	CLK_ENABLE(CLK_AES);
	
	//1.check AES  busy status
	while((IN32(AES_CTRL)&0x2) == 0x2)//bit1: 1--->input data need,AES idle;0--->input data ready,AES work
	{
		break;
	}
    //3.set encode in ctrl
    OUT32(AES_CTRL, (IN32(AES_CTRL)&0xE));//bit0:  1---->decrypt;0---->encrypt

	//4.input key
	for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
	{
		DataTemp_32[index] = (*(key+4*index+3)<<24)+(*(key+4*index+2)<<16)+(*(key+4*index+1)<<8)+*(key+4*index);
		OUT32((AES_KEY0+4*index), DataTemp_32[index]);
	}
	memset(DataTemp_32,0x0,sizeof(uint32_t)*4);
	
	//5.input data,must be 4-byte alignment
	if(input_len == AES_INPUT_DATA_LEN_MAX)
	{
		for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
		{
			DataTemp_32[index] = (*(input+4*index+3)<<24)+(*(input+4*index+2)<<16)+(*(input+4*index+1)<<8)+*(input+4*index);
			OUT32(AES_DATA, DataTemp_32[index]);
		}
	}
	else if(input_len < AES_INPUT_DATA_LEN_MAX)
	{
		if((input_len%AES_DATA_FIFO_WIDTH) == 0)
		{
			for(index=0;index<=(AES_INPUT_DATA_LEN_MAX/input_len);index++)
			{
				DataTemp_32[index] = (*(input+4*index+3)<<24)+(*(input+4*index+2)<<16)+(*(input+4*index+1)<<8)+*(input+4*index);
				OUT32(AES_DATA, DataTemp_32[index]);
			}
			for(index=0;index<(AES_DATA_FIFO_DEPTH-(AES_INPUT_DATA_LEN_MAX/input_len));index++)
			{
				OUT32(AES_DATA, 0x0);
			}
			
		}
		#if 0
		else if((input_len%AES_DATA_FIFO_WIDTH) > 0)
		{
			for(index=0;index<(input_len/AES_DATA_FIFO_WIDTH);index++)
			{
				DataTemp_32[index] = (*(input+4*index+3)<<24)+(*(input+4*index+2)<<16)+(*(input+4*index+1)<<8)+*(input+4*index);
				OUT32(AES_DATA, DataTemp_32[index]);
			}
			if(input_len%AES_DATA_FIFO_WIDTH < AES_DATA_FIFO_WIDTH)
			{

				DataTemp_32[index+1] = 
			
			}

		}
		else
		{
		}
		#endif
	}
	//6.wait encrypy finish
	while(!(IN32(AES_CTRL)&0x4));//bit2: 1--->work finish;0--->working
		
	//7.read data
	for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
	{
		DataTemp_32[index] = IN32(AES_DATA);
		//  OUT32(AES_DATA,DataTemp_32[index]);	
	}
	
    for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
    {
		DataTemp_8[4*index]   = (DataTemp_32[index]&0xFF);
		DataTemp_8[4*index+1] = ((DataTemp_32[index]&0xFF00)>>8);
		DataTemp_8[4*index+2] = ((DataTemp_32[index]&0xFF0000)>>16);
		DataTemp_8[4*index+3] = ((DataTemp_32[index]&0xFF000000)>>24);
	}

    outputlen  = sizeof(uint8_t)*AES_DATA_FIFO_DEPTH*AES_DATA_FIFO_WIDTH;
    memcpy(output,DataTemp_8, outputlen);

    //8.clock disable
    CLK_DISABLE(CLK_AES);
    
	return outputlen;
	
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
int32_t aes_128_decrypt(uint8_t *input,uint32_t input_len,const uint8_t *key,const uint8_t key_len,uint8_t *output)
{
	uint32_t RegTmp = 0;
	uint8_t  DataTemp_8[16];
	uint32_t DataTemp_32[4];
	uint8_t index=0;
	int32_t  outputlen = 0;

	if((input_len != AES_INPUT_DATA_LEN_MAX)||(key_len != AES_KEY_LEN_MAX))
	{
		return DRV_ERR_INVALID_PARAM;
	}
	memset(DataTemp_32,0x0,sizeof(uint32_t)*4);
	memset(DataTemp_8,0x0,sizeof(uint8_t)*16);

	//0.clock enable
	CLK_ENABLE(CLK_AES);

	//1.check AES  busy status
	while((IN32(AES_CTRL)&0x2) == 0x2)//bit1: 1--->input data need,AES idle;0--->input data ready,AES work
	{
		break;
	}

    //2.set encode in ctrl
    OUT32(AES_CTRL, (IN32(AES_CTRL)|0x1));//bit0:  1---->decrypt;0---->encrypt

	//3.input key
	for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
	{
		DataTemp_32[index] = (*(key+4*index+3)<<24)+(*(key+4*index+2)<<16)+(*(key+4*index+1)<<8)+*(key+4*index);
		OUT32((AES_KEY0+4*index), DataTemp_32[index]);
	}
	memset(DataTemp_32,0x0,sizeof(uint32_t)*4);

	//5.input data,must be 128bit
	//if(input_len == AES_INPUT_DATA_LEN_MAX)
	//{
		for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
		{
			DataTemp_32[index] = (*(input+4*index+3)<<24)+(*(input+4*index+2)<<16)+(*(input+4*index+1)<<8)+*(input+4*index);
			OUT32(AES_DATA, DataTemp_32[index]);
		}
	//}

	//6.wait encrypy finish
	while(!(IN32(AES_CTRL)&0x4));//bit2: 1--->work finish;0--->working

	//7.read data
	for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
	{
		DataTemp_32[index] = IN32(AES_DATA);
		//  OUT32(AES_DATA,DataTemp_32[index]);	
	}
	
    for(index=0;index<AES_DATA_FIFO_DEPTH;index++)
    {
		DataTemp_8[4*index]   = (DataTemp_32[index]&0xFF);
		DataTemp_8[4*index+1] = ((DataTemp_32[index]&0xFF00)>>8);
		DataTemp_8[4*index+2] = ((DataTemp_32[index]&0xFF0000)>>16);
		DataTemp_8[4*index+3] = ((DataTemp_32[index]&0xFF000000)>>24);
	}

    outputlen  = sizeof(uint8_t)*AES_DATA_FIFO_DEPTH*AES_DATA_FIFO_WIDTH;
    memcpy(output,DataTemp_8, outputlen);

	//8.clock disable
    CLK_DISABLE(CLK_AES);

	return outputlen;
}


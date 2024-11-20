/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: amtnv.c   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        liangyu
 * Date:          2019-11-17
 * History 1:      
 *     Date: 2020-04-17
 *     Version:
 *     Author: wangxia
 *     Modification:  add aliyun info set&get
 * History 2: 
  ********************************************************************************/

/****************************************************************************
* 	                                           Include files
****************************************************************************/
#include "system.h"
#include "amtNV.h"
#include "drv_spiflash.h"
#include "nv_config.h"
#include <easyflash.h>


/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
unsigned int amt_partion_base = 0xFF000;
volatile unsigned char amt_partion_flag = 0;


#if (AMT_NV_MAX < (amt_partion_base+256))
#define AMT_NV_BLOCK_SIZE						256
#elif (AMT_NV_MAX < (amt_partion_base+512))
#define AMT_NV_BLOCK_SIZE						512
#elif (AMT_NV_MAX < (amt_partion_base+1024))
#define AMT_NV_BLOCK_SIZE						1024
#elif (AMT_NV_MAX < (amt_partion_base+2048))
#define AMT_NV_BLOCK_SIZE						2048
#else
#error the AMT NV size is too big!!!
#endif
	
#define AMT_NV_BLOCK_NUM						(AMT_PARTION_SIZE/AMT_NV_BLOCK_SIZE)
//#define AMT_NV_BLOCK_NUM			4	//just for debug
#define AMT_NV_SIZE								(AMT_NV_MAX - amt_partion_base)
	
#define  AMT_NV_MAGIC_VALUE						0x4E56	//NV
#define  AMT_NV_MAGIC_NONE						0xFFFF	//NV-none
	
#define  AMT_NV_STATE_NONE						0xFFFF
#define  AMT_NV_STATE_USED						0xFF00
#define  AMT_NV_STATE_DELE						0x0000

#define AMT_NV_HEAD_SIZE						sizeof(struct amt_nv_head_t)

#define AMT_NV_BACKUP_BASE						0xFE000
#define AMT_NV_BACKUP_BEGIN						AMT_NV_BACKUP_BASE
#define AMT_NV_BACKUP_COMP						(AMT_NV_BACKUP_BASE+4)
#define AMT_NV_BACKUP_ADDR						(AMT_NV_BACKUP_BASE+16)


/****************************************************************************
* 	                                           Local Types
****************************************************************************/
struct amt_nv_head_t
{
	unsigned short magic;
	unsigned short state;	
	unsigned char val_len;	
	unsigned char crc8;
	unsigned char data[0];
};

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
 * Function: amt_nv_crc8
 * Description: CRC-8
 * Parameters: 
 *   Input:	buf-- the buffer which is need calculate crc
 *			length -- the length of the buffer
 *			
 *   Output:	
 *
 * Returns: 
 *                  return the crc value
 *
 * Others: 
 ********************************************************************************/
static unsigned char amt_nv_crc8(unsigned char * buf, unsigned short length)
{
	unsigned char crc = 0, i = 0;

	while(length--)
	{
		crc ^= *buf++;
		for(i = 8; i > 0; i--)
		{
			if(crc & 0x80)
			{
				crc = (crc << 1) ^ 0x31;
			}
			else
			{
				crc <<= 1;
			}
		}
	}

	return crc;
}
//#define AMT
#if(!defined _USR_LMAC_TEST)&&(!_USER_LMAC_SDIO)
/*******************************************************************************
 * Function: amt_nv_search
 * Description: find  the valid ID and free id for the specified address nv
 * Parameters: 
 *   Input:	addr -- the specified address  of nv
 *			
 *   Output:	pHead -- nv head pointer
 *		       index_search -- the valid ID pointer 
 *		       index_alloc -- the free ID pointer 
 *
 * Returns: 
 *
 * Others: 
 ********************************************************************************/
static 
void amt_nv_search(unsigned int addr, struct amt_nv_head_t *pHead, int * index_search, int * index_alloc)
{
	unsigned short state;
	int i, search_id = AMT_NV_BLOCK_NUM;
	struct amt_nv_head_t head;	
	if(amt_partion_flag == 0)
	{
		partion_info_get(PARTION_NAME_NV_AMT, &amt_partion_base, NULL);
		amt_partion_flag = 1;
	}
	for(i = 0; i < AMT_NV_BLOCK_NUM; i++)
	{
		hal_spiflash_read(addr + i * AMT_NV_BLOCK_SIZE, (unsigned char *)(&head), AMT_NV_HEAD_SIZE);
		
		switch (head.magic)
		{
			case  AMT_NV_MAGIC_VALUE:
				if(head.state == AMT_NV_STATE_USED)
				{
					if(search_id == AMT_NV_BLOCK_NUM)
					{
						memcpy((unsigned char *)(pHead), (unsigned char *)(&head), AMT_NV_HEAD_SIZE);
						search_id = i;
					}
					else
					{
						state = AMT_NV_STATE_DELE;
						hal_spifiash_write(addr + i *AMT_NV_BLOCK_SIZE + 2 , (unsigned char *)(&state), 2);
					}
				}
				break;

			case AMT_NV_MAGIC_NONE:
				if(i == 0)
				{
					* index_search = AMT_NV_BLOCK_NUM;
					* index_alloc = 0;
					return;
				}
				else
				{
					if(search_id < AMT_NV_BLOCK_NUM)
					{
						* index_search = search_id;
						* index_alloc = i;
						return;
					}
					else
					{
						* index_search = AMT_NV_BLOCK_NUM;
						* index_alloc = i;
						return;
					}
				}

			default:
				break;
		}
	}

	* index_search = search_id;
	* index_alloc = AMT_NV_BLOCK_NUM;
}

/*******************************************************************************
 * Function: amt_nv_gc
 * Description: NV parameter garbage collection
 * Parameters: 
 *   Input:	addr -- the specified address  of nv
 *			buf -- the buffer address of the update data to be written
 *			len -- the length of the update data to be written
 *			
 *   Output:
 *
 * Returns: 
 *
 * Others: 
 ********************************************************************************/
static 
void amt_nv_gc(unsigned int addr, unsigned char * buf, unsigned int len)
{
	unsigned char * cache = (unsigned char *)0x253000;
	int offset = 0, index_search, index_alloc;
	struct amt_nv_head_t *pHead;
	if(amt_partion_flag == 0)
	{
		partion_info_get(PARTION_NAME_NV_AMT, &amt_partion_base, NULL);
		amt_partion_flag = 1;
	}

	hal_spiflash_read(amt_partion_base, cache, AMT_NV_SIZE);

	do
	{
		pHead = (struct amt_nv_head_t *)(cache + offset);
		if((pHead->magic == AMT_NV_MAGIC_VALUE) && (pHead->state == AMT_NV_STATE_DELE))
		{
			amt_nv_search(amt_partion_base + offset, pHead, &index_search, &index_alloc);
			if(index_search < AMT_NV_BLOCK_NUM)
			{
				hal_spiflash_read(amt_partion_base + offset + index_search * AMT_NV_BLOCK_SIZE + AMT_NV_HEAD_SIZE, 
					pHead->data, pHead->val_len);
			}
			else
			{
				memset(pHead, 0xFF, pHead->val_len + AMT_NV_HEAD_SIZE);
			}

			offset += WORD_ALIGN(AMT_NV_HEAD_SIZE + pHead->val_len);
		}
		else if((pHead->magic == AMT_NV_MAGIC_VALUE) && (pHead->state == AMT_NV_STATE_USED))
		{
			offset += WORD_ALIGN(AMT_NV_HEAD_SIZE + pHead->val_len);
		}
		else
		{
			offset+= 4;
		}

	} while(offset < AMT_NV_SIZE );

	if(buf)
	{
		memcpy(cache + addr - amt_partion_base, buf, len);
	}

	unsigned char state = 0;

	hal_spifiash_write(AMT_NV_BACKUP_BEGIN , (unsigned char *)(&state), 1);
	hal_spifiash_write(AMT_NV_BACKUP_ADDR, cache, AMT_NV_SIZE);
	hal_spifiash_write(AMT_NV_BACKUP_COMP, (unsigned char *)(&state), 1);

	hal_spiflash_erase(amt_partion_base, AMT_PARTION_SIZE);
	hal_spifiash_write(amt_partion_base, cache, AMT_NV_SIZE);
	hal_spiflash_erase(AMT_NV_BACKUP_BASE, AMT_PARTION_SIZE);

}

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
int  amt_nv_write(unsigned int addr, unsigned char * buf, unsigned int len)
{
	int index_search, index_alloc;
	unsigned short state;
	struct amt_nv_head_t head, *pHead;
	if(amt_partion_flag == 0)
	{
		partion_info_get(PARTION_NAME_NV_AMT, &amt_partion_base, NULL);
		amt_partion_flag = 1;
	}

	if((addr < amt_partion_base) || (addr > AMT_NV_MAX) ||(buf == NULL))
	{
		return DRV_ERR_INVALID_PARAM;
	}

	pHead = (struct amt_nv_head_t *)pvPortMalloc(len +AMT_NV_HEAD_SIZE);
	if(pHead == NULL)
	{
		return DRV_ERR_MEM_ALLOC;
	}

	pHead->magic = AMT_NV_MAGIC_VALUE;
	pHead->state = AMT_NV_STATE_USED;
	pHead->val_len = len;
	pHead->crc8 = amt_nv_crc8(buf, len);
	memcpy(pHead->data, buf, len);

	amt_nv_search(addr, (struct amt_nv_head_t *)(&head), &index_search, &index_alloc);
	if(index_alloc ==AMT_NV_BLOCK_NUM)
	{
		amt_nv_gc(addr, (unsigned char *)pHead, len + AMT_NV_HEAD_SIZE);
		//amt_nv_search(addr, (struct amt_nv_head_t *)(&head), &index_search, &index_alloc);
	}
	else
	{
		hal_spifiash_write(addr + index_alloc *AMT_NV_BLOCK_SIZE , (unsigned char *)pHead, len +AMT_NV_HEAD_SIZE);

		if(index_search < AMT_NV_BLOCK_NUM)
		{
			state = AMT_NV_STATE_DELE;
			hal_spifiash_write(addr + index_search *AMT_NV_BLOCK_SIZE + 2 , (unsigned char *)(&state), 2);
		}
	}
	vPortFree((void *)pHead);

	return DRV_SUCCESS;
}


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
void amt_nv_init(void)
{
	unsigned int backup_begin, backup_comp;
	unsigned char * cache = (unsigned char *)0x253000;
	if(amt_partion_flag == 0)
	{
		partion_info_get(PARTION_NAME_NV_AMT, &amt_partion_base, NULL);
		amt_partion_flag = 1;
	}

	hal_spiflash_read(AMT_NV_BACKUP_BEGIN, (unsigned char *)&backup_begin, 4);	
	hal_spiflash_read(AMT_NV_BACKUP_COMP, (unsigned char *)&backup_comp, 4);
	if((backup_comp == 0xFFFFFFFF) && (backup_begin == 0xFFFFFFFF))
	{		
		// do nothing...
	}
	else if((backup_comp == 0xFFFFFF00) && (backup_begin == 0xFFFFFF00))
	{
		hal_spiflash_read(AMT_NV_BACKUP_ADDR, cache, AMT_NV_SIZE);
		hal_spiflash_erase(amt_partion_base, AMT_PARTION_SIZE);
		hal_spifiash_write(amt_partion_base, cache, AMT_NV_SIZE);
		hal_spiflash_erase(AMT_NV_BACKUP_BASE, AMT_PARTION_SIZE);
	}
	else
	{
		hal_spiflash_erase(AMT_NV_BACKUP_BASE, AMT_PARTION_SIZE);
	}
}
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
int  amt_nv_read(unsigned int addr, unsigned char * buf, unsigned int len)
{
	int index_search;
	struct amt_nv_head_t head;
	if(amt_partion_flag == 0)
	{
		partion_info_get(PARTION_NAME_NV_AMT, &amt_partion_base, NULL);
		amt_partion_flag = 1;
	}

	if((addr < amt_partion_base) || (addr > AMT_NV_MAX) ||(buf == NULL))
	{
		return DRV_ERR_INVALID_PARAM;
	}

#if 1
	for(index_search = 0; index_search<AMT_NV_BLOCK_NUM; index_search++)
	{
		hal_spiflash_read(addr + index_search * AMT_NV_BLOCK_SIZE, (unsigned char *)(&head), AMT_NV_HEAD_SIZE);
		if((head.magic == AMT_NV_MAGIC_VALUE) && (head.state == AMT_NV_STATE_USED))
		{
			break;
		}		
	}
#else
	int index_alloc;
	amt_nv_search(addr, (struct amt_nv_head_t *)(&head), &index_search, &index_alloc);
#endif
	if(index_search == AMT_NV_BLOCK_NUM)
	{
		return DRV_ERROR;
	}

	if(head.val_len > len)
	{
		return DRV_ERROR;
	}

	hal_spiflash_read(addr + index_search * AMT_NV_BLOCK_SIZE + AMT_NV_HEAD_SIZE , buf, head.val_len);
	
	if( head.crc8 == amt_nv_crc8(buf, head.val_len))
	{
		return DRV_SUCCESS;
	}
	else
	{
		return DRV_ERROR;
	}
}


//******************************************************************************************************//
//int  amt_nv_write(unsigned int addr, unsigned char * buf, unsigned int len);
//int  amt_nv_read(unsigned int addr, unsigned char * buf, unsigned int len);

#ifndef SINGLE_BOARD_VER

static int amt_nv_set(cmd_tbl_t *t, int argc, char *argv[])
	{
		int ret;
		unsigned int addr,len;
		char * key, *value;
		if(amt_partion_flag == 0)
		{
			partion_info_get(PARTION_NAME_NV_AMT, &amt_partion_base, NULL);
			amt_partion_flag = 1;
		}
		if(argc != 3)
		{
			system_printf("FAIL\n");
			return CMD_RET_FAILURE;
		}
		key = argv[1];
		value = argv[2];
		if(strcmp(key, "SN")==0)
		{
			addr = amt_partion_base;
			len = AMT_NV_SN_LEN;
		}
		else if(strcmp(key, "MAC")==0)
		{
			addr = AMT_NV_MAC;
			len = AMT_NV_MAC_LEN;		
		}
		else if(strcmp(key, "PIN_FLAG")==0)
		{
			addr = AMT_NV_PIN_FLAG;
			len = AMT_NV_PIN_FLAG_LEN;		
		}
		#ifndef SINGLE_BOARD_VER
		else if(strcmp(key, "PSM_FLAG")==0)
		{
			addr = AMT_NV_PSM_FLAG;
			len = AMT_NV_PSM_FLAG_LEN;		
		}
		#endif
		else if(strcmp(key, "WIFI_FLAG")==0)
		{
			addr = AMT_NV_WIFI_FLAG;
			len = AMT_NV_WIFI_FLAG_LEN; 	
		}
		else if(strcmp(key, "UPDATE_FLAG")==0)
		{
			addr = AMT_NV_UPDATE_FLAG;
			len = AMT_NV_UPDATE_FLAG_LEN;		
		}	
		else if(strcmp(key, "SN_FLAG")==0)
		{
			addr = AMT_NV_SN_FLAG;
			len = AMT_NV_SN_FLAG_LEN;		
		}
		else if(strcmp(key, "MAC_FLAG")==0)
		{
			addr = AMT_NV_MAC_FLAG;
			len = AMT_NV_MAC_FLAG_LEN;		
		}
		#ifndef SINGLE_BOARD_VER
		else if(strcmp(key, "FREQOFFSET_FLAG")==0)
		{
			addr = AMT_NV_FREQOFFSET_FLAG;
			len = AMT_NV_FREQOFFSET_FLAG_LEN;		
		}	
		else if(strcmp(key, "FREQOFFSET")==0)
		{
			addr = AMT_NV_FREQOFFSET;
			len = AMT_NV_FREQOFFSET_LEN;		
		}	
		else if(strcmp(key, "TXPOWER_FLAG")==0)
		{
			addr = AMT_NV_TXPOWER_FLAG;
			len = AMT_NV_TXPOWER_FLAG_LEN;		
		}
		else if(strcmp(key, "TXPOWER_HTCH1")==0)
		{
			addr = AMT_NV_TXPOWER_HTCH1;
			len = AMT_NV_TXPOWER_HTCH1_LEN; 	
		}
		else if(strcmp(key, "TXPOWER_HTCH6")==0)
		{
			addr = AMT_NV_TXPOWER_HTCH6;
			len = AMT_NV_TXPOWER_HTCH6_LEN; 	
		}
		else if(strcmp(key, "TXPOWER_HTCH11")==0)
		{
			addr = AMT_NV_TXPOWER_HTCH11;
			len = AMT_NV_TXPOWER_HTCH11_LEN;		
		}
		else if(strcmp(key, "TXPOWER_HTCH3")==0)
		{
			addr = AMT_NV_TXPOWER_HTCH3;
			len = AMT_NV_TXPOWER_HTCH3_LEN; 	
		}
		else if(strcmp(key, "TXPOWER_HTCH8")==0)
		{
			addr = AMT_NV_TXPOWER_HTCH8;
			len = AMT_NV_TXPOWER_HTCH8_LEN; 	
		}
		else if(strcmp(key, "TXPOWER_NONHTCH1")==0)
		{
			addr = AMT_NV_TXPOWER_NONHTCH1;
			len = AMT_NV_TXPOWER_NONHTCH1_LEN;		
		}
		else if(strcmp(key, "TXPOWER_NONHTCH6")==0)
		{
			addr = AMT_NV_TXPOWER_NONHTCH6;
			len = AMT_NV_TXPOWER_NONHTCH6_LEN;		
		}
		else if(strcmp(key, "TXPOWER_NONHTCH11")==0)
		{
			addr = AMT_NV_TXPOWER_NONHTCH11;
			len = AMT_NV_TXPOWER_NONHTCH11_LEN; 	
		}
		else if(strcmp(key, "TXPOWER_11BCH1")==0)
		{
			addr = AMT_NV_TXPOWER_11BCH1;
			len = AMT_NV_TXPOWER_11BCH1_LEN;		
		}
		else if(strcmp(key, "TXPOWER_11BCH6")==0)
		{
			addr = AMT_NV_TXPOWER_11BCH6;
			len = AMT_NV_TXPOWER_11BCH6_LEN;		
		}
		else if(strcmp(key, "TXPOWER_11BCH11")==0)
		{
			addr = AMT_NV_TXPOWER_11BCH11;
			len = AMT_NV_TXPOWER_11BCH11_LEN;		
		}
		else if(strcmp(key, "CALIBRATION")==0)
		{
			addr = AMT_NV_CALIBRATION;
			len = AMT_NV_CALIBRATION_LEN;		
		}
		#endif
		//add by wangxia 20200417
		else if(strcmp(key, "PDK")==0)
		{
			addr = AMT_NV_ALIYUN_PDKEY;
			len = AMT_NV_ALIYUN_PDKEY_LEN;	
		}
		else if(strcmp(key, "PDS")==0)
		{
			addr = AMT_NV_ALIYUN_PDSECRET;
			len = AMT_NV_ALIYUN_PDSECRET_LEN;	
		}
		else if(strcmp(key, "DN")==0)
		{
			addr = AMT_NV_ALIYUN_DEVNAME;
			len = AMT_NV_ALIYUN_DEVNAME_LEN;	
		}
		else if(strcmp(key, "DS")==0)
		{
			addr = AMT_NV_ALIYUN_DEVSECRET;
			len = AMT_NV_ALIYUN_DEVSECRET_LEN;	
		}
		else if(strcmp(key, "ALY_FLAG")==0)
		{
			addr = AMT_NV_ALIYUN_FLAG;
			len = AMT_NV_ALIYUN_FLAG_LEN;	
		}
		else if(strcmp(key, "TEST_SIGN")==0)
		{		
			addr = AMT_NV_TEST_FLAG;
			len = AMT_NV_TEST_FLAG_LEN;
		}
		else
		{
			//system_printf("111FAIL\n");
			return CMD_RET_FAILURE;
		}
		ret = amt_nv_write(addr,(unsigned char*)value, len);
		if(ret!=0)
		{
			//system_printf("222FAIL\n", ret);
			return CMD_RET_FAILURE; 	
		}
		char readvalue[256];
		memset(readvalue, 0, 256);
		if(amt_nv_read(addr,(unsigned char*)readvalue,len)!=0)
		{
			//system_printf("333FAIL\n", ret);
			return CMD_RET_FAILURE; 	
		}
		if(strcmp(readvalue, value)!=0)
		{
			//system_printf("444FAIL\n", ret);
			return CMD_RET_FAILURE;
		}
		system_printf("SUCCESS AMT_NV SET key:[ %s ] value:[ %s ]\n",key,value);
		return CMD_RET_SUCCESS; 
	}
	
	CMD(nvsamt,
		amt_nv_set,
		"amt set",
		"amt nv set");

/*******************************************************************************
 * Function: amt_nv_get
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
int amt_nv_read_printf()
{
    int state = 0;
    char readvaluebuff[256];
    memset(readvaluebuff, 0, 256);
    
	switch(state) 
	{
	    case 0:
     	    if(amt_nv_read(AMT_NV_SN,(unsigned char*)readvaluebuff,AMT_NV_SN_LEN) !=0)
		        system_printf("\r\nno key--['SN']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read SN = %s", readvaluebuff);
		    state = 0x01;
		case 0x01:
			if(amt_nv_read(AMT_NV_MAC,(unsigned char*)readvaluebuff,AMT_NV_MAC_LEN) !=0)
		        system_printf("\r\nno key--['MAC']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read MAC = %s", readvaluebuff); 
		    state = 0x02;
		case 0x02:
			if(amt_nv_read(AMT_NV_PIN_FLAG,(unsigned char*)readvaluebuff,AMT_NV_PIN_FLAG) !=0)
		        system_printf("\r\nno key--['PIN_FLAG']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read PIN_FLAG = %s", readvaluebuff); 
		    state = 0x03;
		case 0x03:
			if(amt_nv_read(AMT_NV_PSM_FLAG,(unsigned char*)readvaluebuff,AMT_NV_PSM_FLAG_LEN) !=0)
		        system_printf("\r\nno key--['PSM_FLAG']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read PSM_FLAG = %s", readvaluebuff); 
		    state = 0x04;
		case 0x04:
			if(amt_nv_read(AMT_NV_WIFI_FLAG,(unsigned char*)readvaluebuff,AMT_NV_WIFI_FLAG_LEN) !=0)
		        system_printf("\r\nno key--['WIFI_FLAG']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read WIFI_FLAG = %s", readvaluebuff); 
		    state = 0x05;
		case 0x05:
			if(amt_nv_read(AMT_NV_UPDATE_FLAG,(unsigned char*)readvaluebuff,AMT_NV_UPDATE_FLAG_LEN) !=0)
		        system_printf("\r\nno key--['UPDATE_FLAG']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read UPDATE_FLAG = %s", readvaluebuff); 
		    state = 0x06;
		case 0x06:
			if(amt_nv_read(AMT_NV_SN_FLAG,(unsigned char*)readvaluebuff,AMT_NV_SN_FLAG_LEN) !=0)
		        system_printf("\r\nno key--['SN_FLAG']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read SN_FLAG = %s", readvaluebuff); 
		    state = 0x07;
		case 0x07:
			if(amt_nv_read(AMT_NV_MAC_FLAG,(unsigned char*)readvaluebuff,AMT_NV_MAC_FLAG_LEN) !=0)
		        system_printf("\r\nno key--['MAC_FLAG']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read MAC_FLAG = %s", readvaluebuff); 
		    state = 0x08;
		case 0x08:
			if(amt_nv_read(AMT_NV_FREQOFFSET_FLAG,(unsigned char*)readvaluebuff,AMT_NV_FREQOFFSET_FLAG_LEN) !=0)
		        system_printf("\r\nno key--['FREQOFFSET_FLAG']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read FREQOFFSET_FLAG = %s", readvaluebuff); 
		    state = 0x09;
		case 0x09:
			if(amt_nv_read(AMT_NV_FREQOFFSET,(unsigned char*)readvaluebuff,AMT_NV_FREQOFFSET_LEN) !=0)
		        system_printf("\r\nno key--['FREQOFFSET']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read FREQOFFSET = %s", readvaluebuff); 
		    state = 0x10;
		case 0x10:
			if(amt_nv_read(AMT_NV_TXPOWER_FLAG,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_FLAG_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_FLAG']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_FLAG = %s", readvaluebuff); 
		    state = 0x11;
		case 0x11:
			if(amt_nv_read(AMT_NV_TXPOWER_HTCH1,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_HTCH1_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_HTCH1']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_HTCH1 = %s", readvaluebuff); 
		    state = 0x12;
		case 0x12:
			if(amt_nv_read(AMT_NV_TXPOWER_HTCH6,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_HTCH6_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_HTCH6']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_HTCH6 = %s", readvaluebuff); 
		    state = 0x13;
		case 0x13:
			if(amt_nv_read(AMT_NV_TXPOWER_HTCH11,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_HTCH11_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_HTCH11']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_HTCH11 = %s", readvaluebuff); 
		    state = 0x14;
		case 0x14:
			if(amt_nv_read(AMT_NV_TXPOWER_HTCH3,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_HTCH3_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_HTCH3']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_HTCH3 = %s", readvaluebuff); 
		    state = 0x15;
		case 0x15:
			if(amt_nv_read(AMT_NV_TXPOWER_HTCH8,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_HTCH8_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_HTCH8']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_HTCH8 = %s", readvaluebuff); 
		    state = 0x16;
		case 0x16:
			if(amt_nv_read(AMT_NV_TXPOWER_NONHTCH1,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_NONHTCH1_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_NONHTCH1']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_NONHTCH1 = %s", readvaluebuff); 
		    state = 0x17;
		case 0x17:
			if(amt_nv_read(AMT_NV_TXPOWER_NONHTCH6,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_NONHTCH6_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_NONHTCH6']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_NONHTCH6 = %s", readvaluebuff); 
		    state = 0x18;
		case 0x18:
			if(amt_nv_read(AMT_NV_TXPOWER_NONHTCH11,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_NONHTCH11_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_NONHTCH11']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_NONHTCH11 = %s", readvaluebuff); 
		    state = 0x19;
		case 0x19:
			if(amt_nv_read(AMT_NV_TXPOWER_11BCH1,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_11BCH1_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_11BCH1']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_11BCH1 = %s", readvaluebuff); 
		    state = 0x20;
		case 0x20:
			if(amt_nv_read(AMT_NV_TXPOWER_11BCH6,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_11BCH6_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_11BCH6']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_11BCH6 = %s", readvaluebuff); 
		    state = 0x21;
		case 0x21:
			if(amt_nv_read(AMT_NV_TXPOWER_11BCH11,(unsigned char*)readvaluebuff,AMT_NV_TXPOWER_11BCH11_LEN) !=0)
		        system_printf("\r\nno key--['TXPOWER_11BCH11']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read TXPOWER_11BCH11 = %s", readvaluebuff); 
		    state = 0x22;
		case 0x22:
			if(amt_nv_read(AMT_NV_CALIBRATION,(unsigned char*)readvaluebuff,AMT_NV_CALIBRATION_LEN) !=0)
		        system_printf("\r\nno key--['CALIBRATION']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read CALIBRATION = %s", readvaluebuff); 
		    state = 0x23;
		case 0x23:
			if(amt_nv_read(AMT_NV_ALIYUN_PDKEY,(unsigned char*)readvaluebuff,AMT_NV_ALIYUN_PDKEY_LEN) !=0)
		        system_printf("\r\nno key--['PDK']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read PDK = %s", readvaluebuff); 
		    state = 0x24;
		case 0x24:
			if(amt_nv_read(AMT_NV_ALIYUN_PDSECRET,(unsigned char*)readvaluebuff,AMT_NV_ALIYUN_PDSECRET_LEN) !=0)
		        system_printf("\r\nno key--['PDS']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read PDS = %s", readvaluebuff); 
		    state = 0x25;
		case 0x25:
			if(amt_nv_read(AMT_NV_ALIYUN_DEVNAME,(unsigned char*)readvaluebuff,AMT_NV_ALIYUN_DEVNAME_LEN) !=0)
		        system_printf("\r\nno key--['DN']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read DN = %s", readvaluebuff); 
		    state = 0x26;
		case 0x26:
			if(amt_nv_read(AMT_NV_ALIYUN_DEVSECRET,(unsigned char*)readvaluebuff,AMT_NV_ALIYUN_DEVSECRET_LEN) !=0)
		        system_printf("\r\nno key--['DS']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read DS = %s", readvaluebuff); 
		    state = 0x27;
		case 0x27:
			if(amt_nv_read(AMT_NV_ALIYUN_FLAG,(unsigned char*)readvaluebuff,AMT_NV_ALIYUN_FLAG_LEN) !=0)
		        system_printf("\r\nno key--['ALY_FLAG']!");
	        else
		        system_printf("\r\nSUCCESS AMT_NV read ALY_FLAG = %s\r\n", readvaluebuff); 
		break;
	}
	return 0;
}

static int amt_nv_get(cmd_tbl_t *t, int argc, char *argv[])
{
		int ret;
		unsigned int addr,len;
		char * key;		
		if(amt_partion_flag == 0)
		{
			partion_info_get(PARTION_NAME_NV_AMT, &amt_partion_base, NULL);
			amt_partion_flag = 1;
			
		}
		
		if(argc != 2)
		{
			system_printf("FAIL\n");
			return CMD_RET_FAILURE;
		}
		key = argv[1];
		if(strcmp(key, "SN")==0)
		{
			addr = AMT_NV_SN;
			len = AMT_NV_SN_LEN;
		}
		else if(strcmp(key, "MAC")==0)
		{
			addr = AMT_NV_MAC;
			len = AMT_NV_MAC_LEN;
		}
		else if(strcmp(key, "PIN_FLAG")==0)
		{
			addr = AMT_NV_PIN_FLAG;
			len = AMT_NV_PIN_FLAG_LEN;		
		}
		#ifndef SINGLE_BOARD_VER
		else if(strcmp(key, "PSM_FLAG")==0)
		{
			addr = AMT_NV_PSM_FLAG;
			len = AMT_NV_PSM_FLAG_LEN;		
		}
		#endif
		else if(strcmp(key, "WIFI_FLAG")==0)
		{
			addr = AMT_NV_WIFI_FLAG;
			len = AMT_NV_WIFI_FLAG_LEN; 	
		}
		else if(strcmp(key, "UPDATE_FLAG")==0)
		{
			addr = AMT_NV_UPDATE_FLAG;
			len = AMT_NV_UPDATE_FLAG_LEN;		
		}	
		else if(strcmp(key, "SN_FLAG")==0)
		{
			addr = AMT_NV_SN_FLAG;
			len = AMT_NV_SN_FLAG_LEN;		
		}
		else if(strcmp(key, "MAC_FLAG")==0)
		{
			addr = AMT_NV_MAC_FLAG;
			len = AMT_NV_MAC_FLAG_LEN;		
		}
		#ifndef SINGLE_BOARD_VER
		else if(strcmp(key, "FREQOFFSET_FLAG")==0)
		{
			addr = AMT_NV_FREQOFFSET_FLAG;
			len = AMT_NV_FREQOFFSET_FLAG_LEN;		
		}	
		else if(strcmp(key, "FREQOFFSET")==0)
		{
			addr = AMT_NV_FREQOFFSET;
			len = AMT_NV_FREQOFFSET_LEN;		
		}	
		else if(strcmp(key, "TXPOWER_FLAG")==0)
		{
			addr = AMT_NV_TXPOWER_FLAG;
			len = AMT_NV_TXPOWER_FLAG_LEN;		
		}	
		else if(strcmp(key, "TXPOWER_HTCH1")==0)
		{
			addr = AMT_NV_TXPOWER_HTCH1;
			len = AMT_NV_TXPOWER_HTCH1_LEN; 	
		}	
		else if(strcmp(key, "TXPOWER_HTCH6")==0)
		{
			addr = AMT_NV_TXPOWER_HTCH6;
			len = AMT_NV_TXPOWER_HTCH6_LEN; 	
		}
		else if(strcmp(key, "TXPOWER_HTCH11")==0)
		{
			addr = AMT_NV_TXPOWER_HTCH11;
			len = AMT_NV_TXPOWER_HTCH11_LEN;		
		}
		else if(strcmp(key, "TXPOWER_HTCH3")==0)
		{
			addr = AMT_NV_TXPOWER_HTCH3;
			len = AMT_NV_TXPOWER_HTCH3_LEN; 	
		}
		else if(strcmp(key, "TXPOWER_HTCH8")==0)
		{
			addr = AMT_NV_TXPOWER_HTCH8;
			len = AMT_NV_TXPOWER_HTCH8_LEN; 	
		}
		else if(strcmp(key, "TXPOWER_NONHTCH1")==0)
		{
			addr = AMT_NV_TXPOWER_NONHTCH1;
			len = AMT_NV_TXPOWER_NONHTCH1_LEN;		
		}
		else if(strcmp(key, "TXPOWER_NONHTCH6")==0)
		{
			addr = AMT_NV_TXPOWER_NONHTCH6;
			len = AMT_NV_TXPOWER_NONHTCH6_LEN;		
		}
		else if(strcmp(key, "TXPOWER_NONHTCH11")==0)
		{
			addr = AMT_NV_TXPOWER_NONHTCH11;
			len = AMT_NV_TXPOWER_NONHTCH11_LEN; 	
		}
		else if(strcmp(key, "TXPOWER_11BCH1")==0)
		{
			addr = AMT_NV_TXPOWER_11BCH1;
			len = AMT_NV_TXPOWER_11BCH1_LEN;		
		}
		else if(strcmp(key, "TXPOWER_11BCH6")==0)
		{
			addr = AMT_NV_TXPOWER_11BCH6;
			len = AMT_NV_TXPOWER_11BCH6_LEN;		
		}
		else if(strcmp(key, "TXPOWER_11BCH11")==0)
		{
			addr = AMT_NV_TXPOWER_11BCH11;
			len = AMT_NV_TXPOWER_11BCH11_LEN;		
		}
		else if(strcmp(key, "CALIBRATION")==0)
		{
			addr = AMT_NV_CALIBRATION;
			len = AMT_NV_CALIBRATION_LEN;		
		}
		//add by wangxia 20200417
		else if(strcmp(key, "PDK")==0)
		{
			addr = AMT_NV_ALIYUN_PDKEY;
			len = AMT_NV_ALIYUN_PDKEY_LEN;	
		}
		else if(strcmp(key, "PDS")==0)
		{
			addr = AMT_NV_ALIYUN_PDSECRET;
			len = AMT_NV_ALIYUN_PDSECRET_LEN;	
		}
		else if(strcmp(key, "DN")==0)
		{
			addr = AMT_NV_ALIYUN_DEVNAME;
			len = AMT_NV_ALIYUN_DEVNAME_LEN;	
		}
		else if(strcmp(key, "DS")==0)
		{
			addr = AMT_NV_ALIYUN_DEVSECRET;
			len = AMT_NV_ALIYUN_DEVSECRET_LEN;	
		}
		else if(strcmp(key, "ALY_FLAG")==0)
		{
			addr = AMT_NV_ALIYUN_FLAG;
			len = AMT_NV_ALIYUN_FLAG_LEN;	
		}
		else if(strcmp(key, "TEST_SIGN")==0)
		{
			
			addr = AMT_NV_TEST_FLAG;
			len = AMT_NV_TEST_FLAG_LEN;	
		}
		else if(strcmp("all", key)==0)
		{
			#ifndef SINGLE_BOARD_VER
			amt_nv_read_printf();
			#endif
			return CMD_RET_SUCCESS;
		}
		#endif
		else
		{
			//system_printf("1111FAIL\n");
			return CMD_RET_FAILURE;
		}
		char readvalue[256];
		memset(readvalue, 0, 256);
		if(amt_nv_read(addr,(unsigned char*)readvalue,len)!=0)
		{
			//system_printf("2222FAIL\n", ret);
			return CMD_RET_FAILURE; 	
		}
		system_printf("SUCCESS AMT_NV read key:[ %s ] value:[ %s ]\n",key, readvalue);
		return CMD_RET_SUCCESS; 
}

	
CMD(nvgamt,
		amt_nv_get,
		"nv get amt",
		"get amt nv");



int amt_mac_write(unsigned char * MAC)
{
	int ret;
	unsigned int addr,len;
	
	addr = AMT_NV_MAC;
	len = AMT_NV_MAC_LEN;
	ret = amt_nv_write(addr, (unsigned char*)MAC, len);
	if(ret!=0)
	{
		system_printf("AMT MAC Write Fail\n", ret);
		return 1;	
	}
	char readvalue[256];
	memset(readvalue, 0, 256);
	if(amt_nv_read(addr,(unsigned char*)readvalue,len)!=0)
	{
		system_printf("AMT MAC Read Fail\n", ret);
		return 1;	
	}
	if(strcmp(readvalue, (char *)MAC)!=0)
	{
		system_printf("FAIL\n", ret);
		return 1;
	}
	//system_printf("SUCCESS\n", ret);
	return 0; 
}

static uint8_t hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int amt_mac_get(cmd_tbl_t *t, int argc, char *argv[])
{
	//0:amt
	//1:otp
	unsigned int type = strtoul(argv[1], NULL, 0);
	int i = 0;
	char tmp_mac[18] = {0};
	uint8_t MAC[6] = {0};
	#if 0
	//user_nv
	i = ef_get_env_blob(NV_WIFI_STA_MAC, tmp_mac, 18, NULL);
	if (!i || (hex2num(tmp_mac[1]) & 0x1)) 
	{		
	} 
	else 
	{
		for (i=0; i<6; i++)
		{
			MAC[i] = hex2num(tmp_mac[i*3]) * 0x10 + hex2num(tmp_mac[i*3+1]);			
		}
		system_printf("USER_NV MAC:%02x:%02x:%02x:%02x:%02x:%02x\n", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
		return CMD_RET_SUCCESS;
	}
	#endif
	if (type == 0)
	{
		//amt_nv
		i = amt_nv_read(AMT_NV_MAC, (unsigned char *)tmp_mac, 18);
		if ((0 != i) || (hex2num(tmp_mac[1]) & 0x1))
		{		
		} 
		else 
		{
			for (i=0; i<6; i++)
			{
				MAC[i] = hex2num(tmp_mac[i*3]) * 0x10 + hex2num(tmp_mac[i*3+1]);
			}
			system_printf("AMT_NV MAC:%02x:%02x:%02x:%02x:%02x:%02x\n", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
			return CMD_RET_SUCCESS;
		}
	}
	else if (type == 1)
	{
		//otp
		spiFlash_OTP_Read(0x1030, 6, (unsigned char *)MAC);
		if (MAC[0] & 0x1) 
		{		
		} 
		else 
		{				
			system_printf("OTP MAC:%02x:%02x:%02x:%02x:%02x:%02x\n", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
			return CMD_RET_SUCCESS;
		}
	}
	system_printf("NO MAC\n");
	
	return CMD_RET_FAILURE; 
}
	
CMD(get_mac,
	amt_mac_get,
	"get_mac",
	"get_mac");

static int amt_get_user_nv_version(cmd_tbl_t *t, int argc, char *argv[])
{
	char value[256];
	memset(value, 0, 256);
	ef_get_env_blob(NV_PRODUCT_VERSIONINNER, value, 256, NULL);
	system_printf("NV GET key: %s, value: %s\n", NV_PRODUCT_VERSIONINNER, value);

	return CMD_RET_SUCCESS; 
}

CMD(get_version,
	amt_get_user_nv_version,
	"get_version",
	"get_version");


#endif

//*****************************************************************************************************//

#if 0

unsigned char sn[48];
unsigned char mac[18] = "12:34:56:78:90:AB";
unsigned char flag3, flag4;
static int cmd_amtNVset(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret;
	int cmd;

	cmd = atoi(argv[1]);
	system_printf("amt_nv_write, cmd: %d\n", cmd);

	switch (cmd)
	{
		case 1:
			amt_nv_write(AMT_NV_SN, sn, 48);
			sn[0]++;
			break;

		case 2:
			amt_nv_write(AMT_NV_MAC, mac, 17);
			mac[0]++;
			break;

		case 3:
			amt_nv_write(AMT_NV_SN_FLAG, &flag3, 1);
			flag3++;
			break;

		case 4:
			amt_nv_write(AMT_NV_MAC_FLAG, &flag4, 1);
			flag4++;
			break;
	}

	return CMD_RET_SUCCESS;
}

CMD(anvs, cmd_amtNVset,  "",  "");

unsigned char rBuf[64];

static int cmd_amtNVinit(cmd_tbl_t *t, int argc, char *argv[])
{
	int i, cmd;

	cmd = atoi(argv[1]);

	for(i=0; i< 48; i++)
		sn[i] = i;

	flag3 = 1;
	flag4 =1;

	if(cmd == 1)
		hal_spiflash_erase(AMT_PARTION_BASE, AMT_PARTION_SIZE);
	
	amt_nv_init();


	system_printf("amt_nv_init\n");
	return CMD_RET_SUCCESS;
}

CMD(anvi, cmd_amtNVinit,  "amt nv init",  "anvi");

//unsigned char rBuf[64];

static int cmd_amtNVget(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret;
	int cmd;

	cmd = atoi(argv[1]);
	system_printf("amt_nv_read, cmd: %d\n", cmd);

	switch (cmd)
	{
		case 1:
			amt_nv_read(AMT_NV_SN, rBuf, 64);
			system_oprintf((char *)rBuf, 48);
			break;

		case 2:
			amt_nv_read(AMT_NV_MAC, rBuf, 64);
			system_oprintf((char *)rBuf, 17);
			break;

		case 3:
			amt_nv_read(AMT_NV_SN_FLAG, rBuf, 64);
			system_oprintf((char *)rBuf, 1);
			break;

		case 4:
			amt_nv_read(AMT_NV_MAC_FLAG, rBuf, 64);
			system_oprintf((char *)rBuf, 1);
			break;
	}

	return CMD_RET_SUCCESS;
}

CMD(anvg, cmd_amtNVget,  "",  "");


#endif

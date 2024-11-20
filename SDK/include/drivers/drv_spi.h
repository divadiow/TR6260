/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:   drv_spi.h 
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        liuyafeng
 * Date:          2019-4-23
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef _DRV_SPI_H
#define _DRV_SPI_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
****************************************************************************/
typedef enum{
	SPI_MASTER_FREQ_40M=0x00,
	SPI_MASTER_FREQ_20M,
	SPI_MASTER_FREQ_13M,
	SPI_MASTER_FREQ_10M,
	SPI_MASTER_FREQ_80M=0xFF
}spi_master_clk;

typedef struct {
	unsigned char cmd;				//flash SPI Command
	unsigned int transCtrl; 		//SPI Transfer Control
}spi_master_cmd;
	
	
typedef struct{
	unsigned int datamerge;
	unsigned int inten;
	unsigned int addr_len;
	unsigned int data_len;
	spi_master_clk master_clk;
	unsigned int spi_rdcmd;
	spi_master_cmd cmd_write;
	spi_master_cmd cmd_read;
}spi_master_dev;


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
void spi_master_init(spi_master_dev *spi_master_dev);
int spi_master_read(int addr,int length,unsigned int *buf);
int spi_master_write(int addr,int length,unsigned int *buf);



#endif/*_DRV_SPI_H*/


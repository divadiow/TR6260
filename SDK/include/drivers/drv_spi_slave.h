/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:    
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        liuyafeng
 * Date:          2019-5-21
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef	_DRV_SPISLAVE_H
#define _DRV_SPISLAVE_H

/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
****************************************************************************/

#define SPI_SLAVE_TX_BUF_SIZE (2048)
#define SPI_SLAVE_RX_BUF_SIZE (2048)



/****************************************************************************
* 	                                        Types
****************************************************************************/
typedef enum{
		SPI_MASTER_FREQ_20M=0,
		SPI_MASTER_FREQ_10M,
		SPI_MASTER_FREQ_5M=3,
		SPI_MASTER_FREQ_4M,
		SPI_MASTER_FREQ_2M=9,
		SPI_MASTER_FREQ_40M=0xFF
}spi_master_clk;

typedef enum{
	MSPI=0x0060A000,
	SPI0=0x00609000
}SPI;
			
typedef struct _spi_slave_dev_t
{	
	unsigned int	regbase;
	unsigned char spi_tx_slave_buf[SPI_SLAVE_TX_BUF_SIZE];
	int spi_tx_slave_buf_head;
	int spi_tx_slave_buf_tail;
	unsigned char spi_rx_slave_buf[SPI_SLAVE_RX_BUF_SIZE];
	int spi_rx_slave_buf_head;
	int spi_rx_slave_buf_tail;	
} spi_slave_buff_dev;


typedef struct{ 
		SPI spi_base;
		unsigned int inten;
		unsigned int addr_len;
		unsigned int data_len;
		spi_master_clk master_clk;
}spi_slave_dev;



/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/
void spi_slave_init(spi_slave_dev *spi_slave_dev);
void spi_slave_load(unsigned int vector);
int spi_slave_write(unsigned char *buf,unsigned int length);
int spi_slave_read(unsigned int *pdata,unsigned int length);


#endif


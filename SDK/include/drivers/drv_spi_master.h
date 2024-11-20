/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:    
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        liuyafeng
 * Date:          2019-4-11
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef	_DRV_SPIMASTER_H
#define _DRV_SPIMASTER_H

/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
****************************************************************************/
#define SPI_TX_BUF_SIZE (4096)
#define SPI_RX_BUF_SIZE (4096)

#define SPI_TRANS_SIZE (512)




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
		SPI_MASTER_SINGLE_MOSI=0,		
		SPI_MASTER_DUAL,
		SPI_MASTER_QUAD,
		SPI_MASTER_SINGLE_MOSI_MISO
}spi_master_trans_mode;		

typedef enum{
	MSPI=0x0060A000,
	SPI0=0x00609000
}SPI;
			
typedef struct {
		unsigned char cmd;				//flash SPI Command
		unsigned int transCtrl; 		//SPI Transfer Control
}spi_master_cmd;

typedef struct _spi_dev_t
{	
	unsigned int	regbase;
	unsigned char spi_tx_buf[SPI_TX_BUF_SIZE];
	int spi_tx_buf_head;
	int spi_tx_buf_tail;
	unsigned char spi_rx_buf[SPI_RX_BUF_SIZE];
	int spi_rx_buf_head;
	int spi_rx_buf_tail;	
} spi_buff_dev;

typedef struct _spi_dev_dma_t
{	
	unsigned int    regbase;
	unsigned int	buffer_tx_length;
	unsigned int 	spi_tx_buf_head;
	unsigned int 	spi_tx_buf_tail;
	unsigned int 	tx_dma_ch;
	void (* callback)(void * );
	void * user_data;
	
	unsigned int	buffer_rx_length;
	unsigned int 	spi_rx_buf_head;
	unsigned int	spi_rx_buf_tail;
	unsigned int 	rx_dma_ch;
} spi_buff_dev_dma;




typedef struct{ 
		SPI spi_base;
		unsigned int inten;
		unsigned int addr_len;
		unsigned int data_len;
		
		unsigned int spi_clk_pol;
		unsigned int spi_clk_pha;
		spi_master_trans_mode spi_trans_mode;
		
		spi_master_clk master_clk;
		spi_master_cmd cmd_write;
		spi_master_cmd cmd_read;
		spi_master_cmd cmd_staus;
		spi_master_cmd cmd_length_request;
		unsigned int dma_enable;
		unsigned int tx_dma_ch;
		unsigned int rx_dma_ch;
}spi_master_dev;

typedef void * spi_handle_t;


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
int spi_master_read(int length,unsigned int *buf);
int spi_master_write(int length,unsigned char *buf);
int hal_spi_register_recv_callback(spi_handle_t handle, void (* callback)(void *), void *data);

void spi_tx_isr();
void spi_rx_isr();


#endif/*_DRV_SPIFLASH_H*/


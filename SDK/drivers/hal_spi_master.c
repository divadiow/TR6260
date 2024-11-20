/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:hal_spi_master.c    
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        liuyafeng
 * Date:          2019-4-20
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
#include "system_common.h"
#include "drv_spi_master.h"
#include "soc_pin_mux.h"
#include "soc_top_reg.h"
#include "task.h"
#include "drv_dma.h"


/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
/* 0x20 - spi transfer control register*/
#define SPI_TRANSCTRL_RCNT(x)			(((x) & 0x1FF) << 0)
#define SPI_TRANSCTRL_WCNT(x)			(((x) & 0x1FF) << 12)
#define SPI_TRANSCTRL_DUALQUAD(x)		(((x) & 0x3) << 22)
#define SPI_TRANSCTRL_TRAMODE(x)		(((x) & 0xF) << 24)
	
#define SPI_TRANSCTRL_DUALQUAD_REGULAR	SPI_TRANSCTRL_DUALQUAD(0)
#define SPI_TRANSCTRL_DUALQUAD_DUAL		SPI_TRANSCTRL_DUALQUAD(1)
#define SPI_TRANSCTRL_DUALQUAD_QUAD		SPI_TRANSCTRL_DUALQUAD(2)
	
#define SPI_TRANSCTRL_TRAMODE_WRCON		SPI_TRANSCTRL_TRAMODE(0)	/* w/r at the same time */
#define SPI_TRANSCTRL_TRAMODE_WO		SPI_TRANSCTRL_TRAMODE(1)	/* write only		*/
#define SPI_TRANSCTRL_TRAMODE_RO		SPI_TRANSCTRL_TRAMODE(2)	/* read only		*/
#define SPI_TRANSCTRL_TRAMODE_WR		SPI_TRANSCTRL_TRAMODE(3)	/* write, read		*/
#define SPI_TRANSCTRL_TRAMODE_RW		SPI_TRANSCTRL_TRAMODE(4)	/* read, write		*/
#define SPI_TRANSCTRL_TRAMODE_WDR		SPI_TRANSCTRL_TRAMODE(5)	/* write, dummy, read	*/
#define SPI_TRANSCTRL_TRAMODE_RDW		SPI_TRANSCTRL_TRAMODE(6)	/* read, dummy, write	*/
#define SPI_TRANSCTRL_TRAMODE_NONE		SPI_TRANSCTRL_TRAMODE(7)	/* none data */
#define SPI_TRANSCTRL_TRAMODE_DW		SPI_TRANSCTRL_TRAMODE(8)	/* dummy, write	*/
#define SPI_TRANSCTRL_TRAMODE_DR		SPI_TRANSCTRL_TRAMODE(9)	/* dummy, read	*/
	
#define SPI_TRANSCTRL_CMD_EN			(1<<30)
#define SPI_TRANSCTRL_ADDR_EN			(1<<29)
#define SPI_TRANSCTRL_ADDR_FMT			(1<<28)
	
#define SPI_TRANSCTRL_TOKEN_EN			(1<<21)
	
#define SPI_TRANSCTRL_DUMMY_CNT_1		(0<<9)
#define SPI_TRANSCTRL_DUMMY_CNT_2		(1<<9)
#define SPI_TRANSCTRL_DUMMY_CNT_3		(2<<9)
	
	
	
/* 0x30 - spi control register */
#define SPI_CONTROL_SPIRST				BIT(0)
#define SPI_CONTROL_RXFIFORST			BIT(1)
#define SPI_CONTROL_TXFIFORST			BIT(2)
	
	
/* 0x34 - spi status register */
#define SPI_STATUS_BUSY					BIT(0)
#define SPI_STATUS_RXNUM				(0x1F << 8)
#define SPI_STATUS_RXENPTY				BIT(14)
#define SPI_STATUS_TXFULL				BIT(23)
	
#define SPI_PREPARE_BUS(X)			\
		do{unsigned int spi_status = 0; 			\
		do {								\
			spi_status = (X);	\
		} while(spi_status & SPI_STATUS_BUSY);}while(0)
	
	
#define SPI_CLEAR_FIFO(X)			X |= (SPI_CONTROL_RXFIFORST|SPI_CONTROL_TXFIFORST)
#define SPI_CLEAR_FIFOTX(X)			X |= SPI_CONTROL_TXFIFORST
#define SPI_CLEAR_FIFORX(X)			X |= SPI_CONTROL_RXFIFORST
	

#define dma_rx_length 512
#define DMAC_BASE         (0x00800000UL)



#define SPI_WAIT_RX_READY(X)		\
		do{unsigned int spi_status_r = 0;			\
		do {								\
			spi_status_r = (X); \
		} while(spi_status_r & SPI_STATUS_RXENPTY);}while(0)
	
#define SPI_WAIT_TX_READY(X)		\
		do{unsigned int spi_status_t = 0;			\
		do {								\
			spi_status_t = (X); \
		} while(spi_status_t & SPI_STATUS_TXFULL);}while(0)

/****************************************************************************
* 	                                           Local Types
****************************************************************************/
typedef struct _spi_regs {
	volatile unsigned int	edRer;		/* 0x00 		 - id and revision reg*/
	volatile unsigned int	rev1[3];		/* 0x04-0x0C - reserved reg */
	volatile unsigned int	transFmt;	/* 0x10 		 - spi transfer format reg */
	volatile unsigned int	directIO;	/* 0x14 		 - spi direct io control reg */
	volatile unsigned int	rev2[2];		/* 0x18-0x1C - reserved reg */
	volatile unsigned int	transCtrl;	/* 0x20 		 - spi transfer control reg */
	volatile unsigned int	cmd;		/* 0x24 		 - spi command reg */
	volatile unsigned int	addr;		/* 0x28 		 - spi address reg */
	volatile unsigned int	data;		/* 0x2C 		 - spi data reg */	
	volatile unsigned int	ctrl;			   /* 0x30			- spi control reg */
	volatile unsigned int	status; 	/* 0x34 		 - spi status reg */
	volatile unsigned int	intrEn; 	/* 0x38 		 - spi interrupt enable reg */
	volatile unsigned int	intrSt; 	/* 0x3C 		 - spi interrupt status reg */
	volatile unsigned int	timing; 	/* 0x40 		 - spi interface timing reg */
	volatile unsigned int	rev3[3];		/* 0x44-0x4C - reserved reg */
	volatile unsigned int	memCtrl;	/* 0x50 		 - spi memery access control reg */
	volatile unsigned int	rev4[3];		/* 0x54-0x5C - reserved reg */
	volatile unsigned int	stvSt;		/* 0x60 		 - spi slave status reg */
	volatile unsigned int	slvDataCnt; /* 0x64 		 - spi slave data count reg */
	volatile unsigned int	rev5[5];		/* 0x68-0x78  - spi status reg */
	volatile unsigned int	config; 	/* 0x7C 		 - configuration reg */
}spi_reg;

typedef struct _spi_dev {
	spi_reg * spiReg;
} spi_dev;


/****************************************************************************
* 	                                           Local Constants
****************************************************************************/
static spi_dev spiDev;
static spi_master_dev spi_master;
static spi_buff_dev * p_spi_dev;
static spi_buff_dev_dma * p_spi_dev_dma;



static unsigned char *spi_master_tx_buff = (unsigned char *)(0x252000);
static unsigned char *spi_master_rx_buff = (unsigned char *)(0x253000);

uint8_t tx_write_buff_index = 0;
uint8_t tx_read_buff_index = 0;
uint8_t rx_write_buff_index = 0;
uint8_t rx_read_buff_index = 0;



#define tx_buff_size (SPI_TX_BUF_SIZE/SPI_TRANS_SIZE)
#define rx_buff_size (SPI_RX_BUF_SIZE/SPI_TRANS_SIZE)

//1:full 0:empty
uint8_t flag_tx_buff[tx_buff_size] = {0};
uint8_t flag_rx_buff[rx_buff_size] = {0};
#define BUFF_FULL (1)
#define BUFF_EMPTY (0)



extern atcdmac100_dev  s_atcdmac_dev;

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
static void delay(volatile unsigned int data)
{
	volatile unsigned int indx;
	
	for (indx = 0; indx < data; indx++) {
	
	};
}


static SemaphoreHandle_t xCountingSemaphore_spi;

static SemaphoreHandle_t xCountingSemaphore_spi_end;

static void spi_master_write_spi(void *pvParameters)
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	spi_reg * pSpiReg = (spi_reg *)spi_master.spi_base;
	unsigned int data,i,left;
	
	while(1)
	{
		xSemaphoreTake(xCountingSemaphore_spi, portMAX_DELAY);
		
		while(p_spi_dev->spi_tx_buf_head!=p_spi_dev->spi_tx_buf_tail)
		{	
			//system_printf("spi send\n");
			
			int length = 0;
			if(p_spi_dev->spi_tx_buf_head>=p_spi_dev->spi_tx_buf_tail)
				length = p_spi_dev->spi_tx_buf_head - p_spi_dev->spi_tx_buf_tail;
			else
				length = p_spi_dev->spi_tx_buf_head + SPI_TX_BUF_SIZE - p_spi_dev->spi_tx_buf_tail + 1;
			
			if(length>=512)
				length = 512;

			SPI_PREPARE_BUS(pSpiReg->status);		
			//reset FIFO
			SPI_CLEAR_FIFO(pSpiReg->ctrl);
			pSpiReg->transCtrl = spi_master.cmd_write.transCtrl | SPI_TRANSCTRL_WCNT(length-1);
			pSpiReg->cmd = spi_master.cmd_write.cmd;
			
			while(length>0)
			{
				data = 0;	
				// data are usually be read 32bits once a time 
				left = min(length, 4);
				for (i = 0; i < left; i++) 
				{
					data |= p_spi_dev->spi_tx_buf[p_spi_dev->spi_tx_buf_tail] << (i * 8);
					p_spi_dev->spi_tx_buf_tail = (p_spi_dev->spi_tx_buf_tail+1)%(SPI_TX_BUF_SIZE+1);					
				}
				// wait till TXFULL is deasserted 
				SPI_WAIT_TX_READY(pSpiReg->status); 	
				pSpiReg->data = data;
				//system_printf("data=0x%x\n",data);
				length -=4;				
			}			
		}
	}	
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
}
static void spi_master_write_spi_dma(void *pvParameters)
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	spi_reg * pSpiReg = (spi_reg *)spi_master.spi_base;
	int data,i,left,length;
	int length_tail_to_end;
	while(1)
	{
		xSemaphoreTake(xCountingSemaphore_spi, portMAX_DELAY);	
		
		if(flag_tx_buff[tx_read_buff_index] == BUFF_FULL)
		{				
			SPI_PREPARE_BUS(pSpiReg->status);
			while((IN32(DMAC_BASE+0x34) & (1<<(16+p_spi_dev_dma->tx_dma_ch))) != 0);
			dmac_init_SPI1_TX();
			dmac_config_spi2_tx(p_spi_dev_dma->tx_dma_ch,SPI_TRANS_SIZE,(unsigned int *)((unsigned char *)spi_master_tx_buff+SPI_TRANS_SIZE*tx_read_buff_index),(unsigned int *)((unsigned char *)pSpiReg+0x2c));
										
			//reset FIFO
			SPI_CLEAR_FIFO(pSpiReg->ctrl);
			pSpiReg->transCtrl = spi_master.cmd_write.transCtrl | SPI_TRANSCTRL_WCNT(SPI_TRANS_SIZE-1);
			pSpiReg->cmd = spi_master.cmd_write.cmd;
	
			//SPI_WAIT_TX_READY(pSpiReg->status);
			dmac_start(p_spi_dev_dma->tx_dma_ch);				
		}
		#if 0
			int length = 0;
			if(p_spi_dev_dma->spi_tx_buf_head>=p_spi_dev_dma->spi_tx_buf_tail)
			{
				length = p_spi_dev_dma->spi_tx_buf_head - p_spi_dev_dma->spi_tx_buf_tail;
				if(length>=512)
					length = 512;

				SPI_PREPARE_BUS(pSpiReg->status);
				while((IN32(DMAC_BASE+0x34) & (1<<(16+p_spi_dev_dma->tx_dma_ch))) != 0);
				dmac_init_SPI1_TX();
				dmac_config_spi2_tx(p_spi_dev_dma->tx_dma_ch,length,(unsigned int *)((unsigned char *)spi_master_tx_buff+p_spi_dev_dma->spi_tx_buf_tail),(unsigned int *)((unsigned char *)pSpiReg+0x2c));
				
						
				//reset FIFO
				SPI_CLEAR_FIFO(pSpiReg->ctrl);
				pSpiReg->transCtrl = spi_master.cmd_write.transCtrl | SPI_TRANSCTRL_WCNT(length-1);
				pSpiReg->cmd = spi_master.cmd_write.cmd;
		
				//SPI_WAIT_TX_READY(pSpiReg->status);
				dmac_start(p_spi_dev_dma->tx_dma_ch);
				
				//while((IN32(DMAC_BASE+0x30) & (1<<(16 + p_spi_dev_dma->tx_dma_ch))) == 0);
				//OUT32(DMAC_BASE+0x30, 1<<(16 + p_spi_dev_dma->tx_dma_ch));

				xSemaphoreTake(xCountingSemaphore_spi_end, portMAX_DELAY);	
				
				p_spi_dev_dma->spi_tx_buf_tail += length;							
			}
			else
			{
				length = p_spi_dev_dma->spi_tx_buf_head + SPI_TX_BUF_SIZE - p_spi_dev_dma->spi_tx_buf_tail +1;
				if(length>=512)
					length = 512;

				length_tail_to_end = (p_spi_dev_dma->spi_tx_buf_head>0)?(SPI_TX_BUF_SIZE - p_spi_dev_dma->spi_tx_buf_tail + 1):(SPI_TX_BUF_SIZE - p_spi_dev_dma->spi_tx_buf_tail);
				
				if(length > length_tail_to_end)
				{
					SPI_PREPARE_BUS(pSpiReg->status);	
					while((IN32(DMAC_BASE+0x34) & (1<<(16+p_spi_dev_dma->tx_dma_ch))) != 0);
					dmac_init_SPI1_TX();
					dmac_config_spi2_tx(p_spi_dev_dma->tx_dma_ch,length_tail_to_end,(unsigned int *)((unsigned char *)spi_master_tx_buff+p_spi_dev_dma->spi_tx_buf_tail),(unsigned int *)((unsigned char *)pSpiReg+0x2c));

						
					//reset FIFO
					SPI_CLEAR_FIFO(pSpiReg->ctrl);
					pSpiReg->transCtrl = spi_master.cmd_write.transCtrl | SPI_TRANSCTRL_WCNT(length-1);
					pSpiReg->cmd = spi_master.cmd_write.cmd;
		
					//SPI_WAIT_TX_READY(pSpiReg->status);
					dmac_start(p_spi_dev_dma->tx_dma_ch);				
					//while((IN32(DMAC_BASE+0x30) & (1<<(16 + p_spi_dev_dma->tx_dma_ch))) == 0);	
					//OUT32(DMAC_BASE+0x30, 1<<(16 + p_spi_dev_dma->tx_dma_ch));
					xSemaphoreTake(xCountingSemaphore_spi_end, portMAX_DELAY);
					p_spi_dev_dma->spi_tx_buf_tail = 0;
					
					dmac_init_SPI1_TX();
					dmac_config_spi2_tx(p_spi_dev_dma->tx_dma_ch,length - length_tail_to_end,(unsigned int *)((unsigned char *)spi_master_tx_buff+p_spi_dev_dma->spi_tx_buf_tail),(unsigned int *)((unsigned char *)pSpiReg+0x2c));
					dmac_start(p_spi_dev_dma->tx_dma_ch);				
					//while((IN32(DMAC_BASE+0x30) & (1<<(16 + p_spi_dev_dma->tx_dma_ch))) == 0);			
					//OUT32(DMAC_BASE+0x30, 1<<(16 + p_spi_dev_dma->tx_dma_ch));
					xSemaphoreTake(xCountingSemaphore_spi_end, portMAX_DELAY);
					p_spi_dev_dma->spi_tx_buf_tail += (length - length_tail_to_end);
					
				}
				else
				{	
					SPI_PREPARE_BUS(pSpiReg->status);		
					while((IN32(DMAC_BASE+0x34) & (1<<(16+p_spi_dev_dma->tx_dma_ch))) != 0);
					dmac_init_SPI1_TX();
					dmac_config_spi2_tx(p_spi_dev_dma->tx_dma_ch,length,(unsigned int *)((unsigned char *)spi_master_tx_buff+p_spi_dev_dma->spi_tx_buf_tail),(unsigned int *)((unsigned char *)pSpiReg+0x2c));
				
					
					//reset FIFO
					SPI_CLEAR_FIFO(pSpiReg->ctrl);
					pSpiReg->transCtrl = spi_master.cmd_write.transCtrl | SPI_TRANSCTRL_WCNT(length-1);
					pSpiReg->cmd = spi_master.cmd_write.cmd;
		
					//SPI_WAIT_TX_READY(pSpiReg->status);
					dmac_start(p_spi_dev_dma->tx_dma_ch);
				
					//while((IN32(DMAC_BASE+0x30) & (1<<(16 + p_spi_dev_dma->tx_dma_ch))) == 0);		
					//OUT32(DMAC_BASE+0x30, 1<<(16 + p_spi_dev_dma->tx_dma_ch));

					xSemaphoreTake(xCountingSemaphore_spi_end, portMAX_DELAY);
					p_spi_dev_dma->spi_tx_buf_tail += length;						
				}				
			}	
		#endif
		
	}
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
}

int hal_spi_register_recv_callback(spi_handle_t handle, void (* callback)(void *), void *data)
{
	spi_buff_dev_dma * spi_dev_dma = (uart_handle_t)handle;
	unsigned int flag = system_irq_save();
	spi_dev_dma->callback = callback;
	spi_dev_dma->user_data = data;
	system_irq_restore(flag);
	return 0;
}

void spi_callback_test(void *handle)
{
	system_printf("spi_callback\n");
}


int spi_master_write(int length,unsigned char *buf)
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	spi_reg * pSpiReg = (spi_reg *)spi_master.spi_base;
	unsigned int data;

	int i,left;
	unsigned int dma_enable = spi_master.dma_enable;

	int buffer_idle_length = 0;
	int buffer_head_to_end = 0;
	//DMA MODE
	if (dma_enable == 1)
	{	
		//ASYN Begin
		if(flag_tx_buff[tx_write_buff_index] == BUFF_EMPTY)
		{
			memcpy(&spi_master_tx_buff[SPI_TRANS_SIZE*tx_write_buff_index],&buf[0],length);
			flag_tx_buff[tx_write_buff_index++] = BUFF_FULL;
			if (tx_write_buff_index == tx_buff_size)
			{
				tx_write_buff_index = 0;
			}
			if(((pSpiReg->status)*SPI_STATUS_BUSY == 0)&&(flag_tx_buff[tx_read_buff_index] == BUFF_FULL))
			{				
				dmac_init_SPI1_TX();
				dmac_config_spi2_tx(p_spi_dev_dma->tx_dma_ch,SPI_TRANS_SIZE,(unsigned int *)((unsigned char *)spi_master_tx_buff+SPI_TRANS_SIZE*tx_read_buff_index),(unsigned int *)((unsigned char *)pSpiReg+0x2c));
											
				//reset FIFO
				SPI_CLEAR_FIFO(pSpiReg->ctrl);
				pSpiReg->transCtrl = spi_master.cmd_write.transCtrl | SPI_TRANSCTRL_WCNT(SPI_TRANS_SIZE-1);
				pSpiReg->cmd = spi_master.cmd_write.cmd;
		
				//SPI_WAIT_TX_READY(pSpiReg->status);
				dmac_start(p_spi_dev_dma->tx_dma_ch);	
			}
			
		}
		else
		{
			system_printf("SPI TX Buffer Full! Write Fail!!!\n");
		}
		//ASYN End		
	}	
	
	//CPU MODE
	else
	{	
		p_spi_dev->spi_tx_buf_head = 0;
		p_spi_dev->spi_tx_buf_tail = 0;
		memcpy(&p_spi_dev->spi_tx_buf[p_spi_dev->spi_tx_buf_head],&buf[0],length);

		
		SPI_PREPARE_BUS(pSpiReg->status);		
		//reset FIFO
		SPI_CLEAR_FIFO(pSpiReg->ctrl);
		pSpiReg->transCtrl = spi_master.cmd_write.transCtrl | SPI_TRANSCTRL_WCNT(length-1);
		pSpiReg->cmd = spi_master.cmd_write.cmd;
			
		while(length>0)
		{
			data = 0;	
			// data are usually be read 32bits once a time 
			left = min(length, 4);
			for (i = 0; i < left; i++) 
			{
				data |= p_spi_dev->spi_tx_buf[p_spi_dev->spi_tx_buf_tail] << (i * 8);
				p_spi_dev->spi_tx_buf_tail = (p_spi_dev->spi_tx_buf_tail+1)%(SPI_TX_BUF_SIZE+1);					
			}
			// wait till TXFULL is deasserted 
			SPI_WAIT_TX_READY(pSpiReg->status); 	
			pSpiReg->data = data;
			//system_printf("data=0x%x\n",data);
			length -=4;				
		}	
	}
	
#ifdef _USE_PSM
	//xSemaphoreGive(xCountingSemaphore_spi);
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
	return 0;
}


void spi_master_init(spi_master_dev *spi_master_dev)
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	spi_master = *spi_master_dev;

	unsigned int value;
	#if 1
	xCountingSemaphore_spi = xSemaphoreCreateCounting(32, 0);
	xCountingSemaphore_spi_end = xSemaphoreCreateCounting(32, 0);

	value = IN32(SW_RESET) & 0xFFFFFFFB;
	OUT32(SW_RESET,value);
	OUT32(SW_RESET,0xFFFFFFFF);
	
	//disable clk
	OUT32(CLK_EN_BASE, IN32(CLK_EN_BASE) &(~(CLK_SPI1)));
	if(spi_master_dev->spi_base == MSPI)
	{
		//cfg pin
		value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<21));
		OUT32(SOC_PIN0_MUX_BASE, value |(3<<21));	/* MSPI GPIO7-DO MUX CFG */
	
		value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<24));
		OUT32(SOC_PIN0_MUX_BASE, value |(3<<24));	/* MSPI GPIO8-DI MUX CFG */

		value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<27));
		OUT32(SOC_PIN0_MUX_BASE, value |(3<<27));	/* MSPI GPIO9-HOLD MUX CFG */

		value = IN32(SOC_PIN1_MUX_BASE) & (~(7<<0));
		OUT32(SOC_PIN1_MUX_BASE, value |(3<<0));		/* MSPI GPIO10-WP MUX CFG */

		value = IN32(SOC_PIN1_MUX_BASE) & (~(7<<3));
		OUT32(SOC_PIN1_MUX_BASE, value |(3<<3));		/* MSPI GPIO11-CLK MUX CFG */
	
		value = IN32(SOC_PIN1_MUX_BASE) & (~(7<<6));
		OUT32(SOC_PIN1_MUX_BASE, value |(3<<6));		/* MSPI GPIO12-CS0 MUX CFG */
	}
	else if(spi_master_dev->spi_base == SPI0)
	{
		//cfg pin
		value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<0));
		OUT32(SOC_PIN0_MUX_BASE, value |(3<<0));	/* SPI GPIO0-CLK MUX CFG */
	
		value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<3));
		OUT32(SOC_PIN0_MUX_BASE, value |(3<<3));	/* SPI GPIO1-CS0 MUX CFG */

		value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<6));
		OUT32(SOC_PIN0_MUX_BASE, value |(3<<6));	/* SPI GPIO2-MOSI MUX CFG */

		value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<9));
		OUT32(SOC_PIN0_MUX_BASE, value |(3<<9));		/* SPI GPIO3-MISO MUX CFG */
		//
		//value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<15));
		//OUT32(SOC_PIN0_MUX_BASE, value |(4<<15));		/* MSPI GPIO5-HOLD MUX CFG */
		
		//value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<18));
		//OUT32(SOC_PIN0_MUX_BASE, value |(4<<18));		/* MSPI GPIO6-WP MUX CFG */
	}
	else
	{
		system_printf("spi_base error");
	}
		
	#endif
	//
	delay(1000000);
	//enable clk
	OUT32(CLK_EN_BASE, IN32(CLK_EN_BASE) |(CLK_SPI1|CLK_SPI_FLASH |CLK_SPI_FLASH_AHB));
	
	if(spi_master_dev->dma_enable == 1)
	{
		p_spi_dev_dma = pvPortMalloc(sizeof(spi_buff_dev_dma));
		
		p_spi_dev_dma->buffer_tx_length = SPI_TX_BUF_SIZE;
		p_spi_dev_dma->buffer_rx_length = SPI_RX_BUF_SIZE;
		p_spi_dev_dma->spi_tx_buf_head = 0;
		p_spi_dev_dma->spi_tx_buf_tail = 0;
		p_spi_dev_dma->spi_rx_buf_head = 0;
		p_spi_dev_dma->spi_rx_buf_tail = 0;
		p_spi_dev_dma->tx_dma_ch = spi_master_dev->tx_dma_ch;
		p_spi_dev_dma->rx_dma_ch = spi_master_dev->rx_dma_ch;
		
		if(spi_master_dev->spi_base == MSPI)
		{
			p_spi_dev_dma->regbase = MSPI;
		}
		else if(spi_master_dev->spi_base == SPI0)
		{
			p_spi_dev_dma->regbase = SPI0;
		}
		
		//enable DMA and set TXTHRES
		value = IN32(spi_master_dev->spi_base+0x30) | BIT3 | BIT4 | BIT16;
		OUT32(spi_master_dev->spi_base+0x30, value);

		//disable datamerge
		value = IN32(spi_master_dev->spi_base+0x10) & (~BIT7);
		OUT32(spi_master_dev->spi_base+0x10, value);	
		
		hal_spi_register_recv_callback(p_spi_dev_dma, spi_callback_test, (void *)p_spi_dev_dma);
		
		//if(OS_CPU_Vector_Table[IRQ_VECTOR_DMA] == NULL)
		dma_init_isr();
		
		//xTaskCreate(spi_master_write_spi_dma, (const char *)"spi_master_spi_write_dma", 2048, NULL, 3, NULL);
	}
	else
	{
		p_spi_dev = pvPortMalloc(sizeof(spi_buff_dev));
		memset(p_spi_dev->spi_tx_buf , 0 ,SPI_TX_BUF_SIZE);
		memset(p_spi_dev->spi_rx_buf , 0 ,SPI_RX_BUF_SIZE);
		p_spi_dev->spi_tx_buf_head = 0;
		p_spi_dev->spi_tx_buf_tail = 0;
		p_spi_dev->spi_rx_buf_head = 0;
		p_spi_dev->spi_rx_buf_tail = 0;
		if(spi_master_dev->spi_base == MSPI)
		{
			p_spi_dev->regbase = MSPI;
		}
		else if(spi_master_dev->spi_base == SPI0)
		{
			p_spi_dev->regbase = SPI0;
		}
		//xTaskCreate(spi_master_write_spi, (const char *)"spi_master_spi_write", 2048, NULL, 3, NULL);
	}

	
	//master mode
	value = IN32(spi_master_dev->spi_base+0x10) & 0xFFFFFFFB;
	//3 line
	if(spi_master_dev->spi_trans_mode == SPI_MASTER_SINGLE_MOSI)
	{	
		value |= 0x10;
	}
	//4 line
	else
	{	
		value &= (~BIT4);	
	}
	//clock POL
	if(spi_master_dev->spi_clk_pol == 0)
		value &= (~BIT1);
	else if(spi_master_dev->spi_clk_pol == 1)
		value |= BIT1;
	else
	{
		system_printf("SPI Clock Polarity Error\n");
		return;
	}
	//clock pha
	if(spi_master_dev->spi_clk_pha==0)
		value &= (~BIT0);
	else if(spi_master_dev->spi_clk_pha == 1)
		value |= BIT0;
	else
	{
		system_printf("SPI Clock Phase Error\n");
		return;
	}
	
	OUT32(spi_master_dev->spi_base+0x10, value); 

	if(spi_master_dev->spi_trans_mode == SPI_MASTER_DUAL)
	{
		spi_master.cmd_write.transCtrl |= SPI_TRANSCTRL_DUALQUAD_DUAL;
		spi_master.cmd_read.transCtrl |= SPI_TRANSCTRL_DUALQUAD_DUAL;
	}
	if(spi_master_dev->spi_trans_mode == SPI_MASTER_QUAD)
	{
		spi_master.cmd_write.transCtrl |= SPI_TRANSCTRL_DUALQUAD_QUAD;
		spi_master.cmd_read.transCtrl |= SPI_TRANSCTRL_DUALQUAD_QUAD;
	}
	
	//addr length
	value = IN32(spi_master_dev->spi_base+0x10) & 0xFFFCFFFF;
	OUT32(spi_master_dev->spi_base+0x10, value |((spi_master_dev->addr_len-1) << 16));	
	
	//data length
	value = IN32(spi_master_dev->spi_base+0x10) & 0xFFFFE0FF;
	OUT32(spi_master_dev->spi_base+0x10, value |((spi_master_dev->data_len-1) << 8));

	//cfg timing
	value = IN32(spi_master_dev->spi_base+0x40) & 0xFFFFFF00;
	OUT32(spi_master_dev->spi_base+0x40, value |(spi_master_dev->master_clk & 0xFF));

	//spi-int
	OUT32(spi_master_dev->spi_base+0x38, spi_master_dev->inten);
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
}
int spi_master_read_to_buffer_dma(unsigned int length)
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	spi_reg * pSpiReg = (spi_reg *)spi_master.spi_base;

	dmac_init_SPI1_RX();
	dmac_config_spi2_rx(spi_master.rx_dma_ch,length,(unsigned int *)((unsigned char *)pSpiReg+0x2c),(void *)&spi_master_rx_buff[p_spi_dev_dma->spi_rx_buf_head]);
	dmac_start(spi_master.rx_dma_ch);	
	
	SPI_PREPARE_BUS(pSpiReg->status);
	//reset FIFO
	SPI_CLEAR_FIFO(pSpiReg->ctrl);
	
	pSpiReg->transCtrl = spi_master.cmd_read.transCtrl | SPI_TRANSCTRL_RCNT(length-1);	
			
	pSpiReg->cmd = spi_master.cmd_read.cmd;	

	//while((pSpiReg->status & SPI_STATUS_RXENPTY) == 1);	
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
	return 0;
}

int spi_master_read_to_buffer(unsigned int length)
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	spi_reg * pSpiReg = (spi_reg *)spi_master.spi_base;

	SPI_PREPARE_BUS(pSpiReg->status);
	//reset FIFO
	SPI_CLEAR_FIFO(pSpiReg->ctrl);
	
	pSpiReg->transCtrl = spi_master.cmd_read.transCtrl | SPI_TRANSCTRL_RCNT(length-1);	
	
	unsigned int Rxcount = 0;
	unsigned int i;
	unsigned int *p_dst_buffer = (unsigned int *)(p_spi_dev->spi_rx_buf+p_spi_dev->spi_rx_buf_head);	
	
	pSpiReg->cmd = spi_master.cmd_read.cmd;

	unsigned int temp = pSpiReg->status;
	int rxnum = ((temp & 0x00001f00)>>8) ;
	while((temp & SPI_STATUS_BUSY) || rxnum>0 )
	{		
		for(i=0; i<rxnum; i++)
	    {
	        *p_dst_buffer++ = pSpiReg->data;
	    }	
		temp = pSpiReg->status;
	    rxnum = ((temp & 0x00001f00)>>8) ;
	}	
	temp = pSpiReg->status;
	rxnum = ((temp & 0x00001f00)>>8) ;
	for(i=0; i<rxnum; i++)
	{
		*p_dst_buffer++ = pSpiReg->data;
	}
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
	return 0;
}

int spi_master_read(int length,unsigned int *buf)
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	if(length>512 || length<=0)
	{
#ifdef _USE_PSM
		TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
		system_printf("Length Error\n");
		return -1;
	}

	if(spi_master.dma_enable == 1)
	{
		spi_master_read_to_buffer_dma(length);
		while(flag_rx_buff[rx_read_buff_index] != BUFF_FULL);		
		memcpy(buf,spi_master_rx_buff,length);	
		flag_rx_buff[rx_read_buff_index] = BUFF_EMPTY;		
	}
	else
	{
		spi_master_read_to_buffer(length);
		memcpy(buf,p_spi_dev->spi_rx_buf,length);		
	}
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
	return 0;
}

int spi_master_read_slave_staus()
{

	spi_reg * pSpiReg = (spi_reg *)spi_master.spi_base;
	SPI_PREPARE_BUS(pSpiReg->status);
	
	//reset FIFO
	SPI_CLEAR_FIFO(pSpiReg->ctrl);
	
	pSpiReg->transCtrl = spi_master.cmd_staus.transCtrl | SPI_TRANSCTRL_RCNT(3);
	//pSpiReg->transCtrl = spi_master.cmd_staus.transCtrl;
	pSpiReg->cmd = spi_master.cmd_staus.cmd;	
	
	SPI_WAIT_RX_READY(pSpiReg->status);
	
	return pSpiReg->data;
}
void spi_tx_isr()
{
	spi_reg * pSpiReg = (spi_reg *)spi_master.spi_base;

	flag_tx_buff[tx_read_buff_index++] = BUFF_EMPTY;
	if (tx_read_buff_index == tx_buff_size)
	{
		tx_read_buff_index = 0;
	}
	
	if (flag_tx_buff[tx_read_buff_index] == BUFF_FULL)
	{
		dmac_init_SPI1_TX();
		dmac_config_spi2_tx(p_spi_dev_dma->tx_dma_ch,SPI_TRANS_SIZE,(unsigned int *)((unsigned char *)spi_master_tx_buff+SPI_TRANS_SIZE*tx_read_buff_index),(unsigned int *)((unsigned char *)pSpiReg+0x2c));
											
		//reset FIFO
		SPI_CLEAR_FIFO(pSpiReg->ctrl);
		pSpiReg->transCtrl = spi_master.cmd_write.transCtrl | SPI_TRANSCTRL_WCNT(SPI_TRANS_SIZE-1);
		pSpiReg->cmd = spi_master.cmd_write.cmd;

		dmac_start(p_spi_dev_dma->tx_dma_ch);		
	}
	
}
void spi_rx_isr()
{
	flag_rx_buff[rx_write_buff_index] = BUFF_FULL;
}




//#ifdef _USE_SPI_HOST_TEST
static int spi_master_init_test(cmd_tbl_t *t, int argc, char *argv[])
{
	spi_master_dev  spi_master_dev = 
		{			
			//MSPI
			.spi_base = SPI0,
			//SPI
			//.spi_base = SPI0,
			.inten = 0x0,
			.addr_len = 3,
			.data_len = 8,
			.master_clk = SPI_MASTER_FREQ_5M,
			
			.spi_clk_pol = 0,
			.spi_clk_pha = 0,
			.spi_trans_mode=SPI_MASTER_SINGLE_MOSI,
			
			//.cmd_read = {0x0c,SPI_TRANSCTRL_CMD_EN | SPI_TRANSCTRL_DUMMY_CNT_2 | SPI_TRANSCTRL_DUALQUAD_DUAL | SPI_TRANSCTRL_TRAMODE_DR},
			//.cmd_read = {0x0B,SPI_TRANSCTRL_CMD_EN | SPI_TRANSCTRL_TRAMODE_DR},
			.cmd_read = {0x00, SPI_TRANSCTRL_TRAMODE_RO},
			//regular
			//.cmd_write = {0x51,SPI_TRANSCTRL_CMD_EN| SPI_TRANSCTRL_TRAMODE_DW},
			.cmd_write = {0x00,SPI_TRANSCTRL_TRAMODE_WO},
			//dual
			//.cmd_write = {0x52,SPI_TRANSCTRL_CMD_EN | SPI_TRANSCTRL_DUMMY_CNT_2 | SPI_TRANSCTRL_DUALQUAD_DUAL | SPI_TRANSCTRL_TRAMODE_DW},
			.cmd_staus = {0x05,SPI_TRANSCTRL_CMD_EN | SPI_TRANSCTRL_TRAMODE_DR},
			.cmd_length_request = {0xFA,SPI_TRANSCTRL_CMD_EN | SPI_TRANSCTRL_TRAMODE_DR},
			.dma_enable = 1,
			//.dma_enable = 0,
			.tx_dma_ch = 1,
			.rx_dma_ch = 0
		};	
	
	spi_master_init(&spi_master_dev);	
	
	return CMD_RET_SUCCESS;
}
	
	
CMD(spi_init,
	spi_master_init_test,
	"spi_init",
	"spi_init");

static int spi_master_write_test(cmd_tbl_t *t, int argc, char *argv[])
{
	
	unsigned int length = strtoul(argv[1], NULL, 0);

	//system_printf("length=%d\n",length);
	

	unsigned char sfTest[1024]={0};
	int i;
	
	for(i=0;i<1024;++i)
	{
		//sfTest[i] = i%(0x80);
		sfTest[i] = (unsigned char)(i%256);
		//system_printf("sfTest[%d]=%d\n",i,sfTest[i]);
	}
	
	//unsigned char sfTest[4]={0x00,0xff,0xfb,0x10};
	spi_master_write(length, (unsigned char*)sfTest);
	
	return CMD_RET_SUCCESS;
}
		
		
CMD(spi_write,
	spi_master_write_test,
	"spi_write",
	"spi_write length buf");
static int spi_test_(cmd_tbl_t *t, int argc, char *argv[])
{
	unsigned int times = strtoul(argv[1], NULL, 0);
	unsigned int de = strtoul(argv[2], NULL, 0);

	unsigned char sfTest[1024]={0};
	int i;
	for(i=0;i<1024;++i)
	{
		sfTest[i] = i%(256);
	}	

	for(i=0;i<times;++i)
	{
		spi_master_write(512, (unsigned char*)sfTest);
		vTaskDelay(pdMS_TO_TICKS(de));
	}
	
	return CMD_RET_SUCCESS;
}
		
		
CMD(spi_test,
	spi_test_,
	"spi_write",
	"spi_write length buf");


static int spi_master_read_test(cmd_tbl_t *t, int argc, char *argv[])
{
	//unsigned int addr = strtoul(argv[1], NULL, 0);
		
	unsigned int length = strtoul(argv[1], NULL, 0);
	//unsigned int length = 128;
	unsigned char sfTest1[4]={0};

	
	//spi_master_write(4, (unsigned char *) sfTest1);
	
	unsigned char sfTest[512]={0};
	spi_master_read(length, (unsigned int *) sfTest);
	
	int i=0;
	int pass = 1;
	for(i=0;i<length;++i)
	{
		if(sfTest[i] != i%256)
		{
			pass = 0;
			system_printf("sfTest[%d]=%d\n",i,sfTest[i]);
			system_printf("FAIL\n");
		}
	}
	if(pass==1)
		system_printf("PASS\n");

		
	return CMD_RET_SUCCESS;
}
						
CMD(spi_read,
	spi_master_read_test,
	"spi_read",
	"spi_read length buf");

static int spi_master_read_staus(cmd_tbl_t *t, int argc, char *argv[])
{	
	/*	
	char sfTest1[4]={0};
	sfTest1[0]=0x0;
	sfTest1[1]=0xFB;
	sfTest1[2]=0x0;
	sfTest1[3]=0x0;
	
	spi_master_write(4, (unsigned int *) sfTest1);
	*/
	unsigned int staus = spi_master_read_slave_staus();
	system_printf("slave staus:0x%x\n",staus);
	return CMD_RET_SUCCESS;
}
							
CMD(spi_staus,
	spi_master_read_staus,
	"spi_staus",
	"spi_staus");


static int spi_master_buffer_show(cmd_tbl_t *t, int argc, char *argv[])
{	
	system_printf("p_spi_dev->spi_tx_buf_head=%d\n",p_spi_dev->spi_tx_buf_head);
	system_printf("p_spi_dev->spi_tx_buf_tail=%d\n",p_spi_dev->spi_tx_buf_tail);
	system_printf("p_spi_dev_dma->spi_tx_buf_head=%d\n",p_spi_dev_dma->spi_tx_buf_head);
	system_printf("p_spi_dev_dma->spi_tx_buf_tail=%d\n",p_spi_dev_dma->spi_tx_buf_tail);
	return CMD_RET_SUCCESS;
}
								
CMD(spi_master_buffer,
	spi_master_buffer_show,
	"spi_master_buffer",
	"spi_master_buffer");


//#endif


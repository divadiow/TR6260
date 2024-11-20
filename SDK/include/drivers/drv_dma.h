/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: drv_dma.h   
  ********************************************************************************/

#ifndef _DRV_DMA_H
#define _DRV_DMA_H



typedef  struct channel_ctrl_t
{
	unsigned int ChnCtrl;
	unsigned int ChnSrcAddr;
	unsigned int ChnDstAddr;
	unsigned int ChnTranSize;
	unsigned int ChnLLPointer;
	
}channel_ctrl;


typedef  struct atcdmac100_reg_t
{
	unsigned int idRev;			/* 0x00 			ID and revision register		*/
	unsigned int reserved1[3];		/* 0x04~0x0C	reserved					*/
	unsigned int DMACfg;			/* 0x10 			DMAC configuration register	*/
	unsigned int reserved2[3];		/* 0x14~0x1C 	reserved					*/
	unsigned int DMACtrl;			/* 0x20 			DMAC control register		*/
	unsigned int reserved3[3];		/* 0x24~0x2C 	reserved					*/
	unsigned int IntStatus;		/* 0x30 			Interrupt status register		*/
	unsigned int ChEN;			/* 0x34 			Channel enable register		*/
	unsigned int reserved4[2];		/* 0x38~0x3C 	reserved					*/
	unsigned int ChAbort;			/* 0x40			Channel abort register		*/

	channel_ctrl ChnCtrl[4];
	
}atcdmac100_reg;


typedef struct atcdmac100_dev_t
{
	atcdmac100_reg * pdmaBase;
	unsigned int ChnCfg;

}atcdmac100_dev;
void dma_init(void);
void dmac_start(int chnum);
void dma_init_isr();

void dmac_start_s(int chnum);
	
void dmac_init_UART0_TX(void);
void dmac_init_UART0_RX(void);
void dmac_init_UART1_TX(void);
void dmac_init_UART1_RX(void);
void dmac_init_SPI2_TX(void);
void dmac_init_SPI2_RX(void);
void dmac_init_SPI1_TX(void);
void dmac_init_SPI1_RX(void);
void dmac_config_spi2_tx(int chnum, unsigned int size, void * srcAddr,void * dstAddr);
void dmac_config_spi2_rx(int chnum, unsigned int size, void * srcAddr,void * dstAddr);
void dmac_reset(void);
void dmac_config_uart_rx(int chnum, unsigned int size, void * srcAddr,void * dstAddr);
void dmac_config_uart_tx(int chnum, unsigned int size, void * srcAddr,void * dstAddr);
#endif
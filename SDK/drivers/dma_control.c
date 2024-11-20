// Copyright 2018-2019 Transa-Semi Technology Inc. and its subsidiaries.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "drv_dma.h"
#include "drv_uart.h"
#include "drv_spi_master.h"
#include "stdio.h"
#include "string.h"
#include "bsp/soc.h"
#include "bsp/soc_pin_mux.h"




#define DMAC_BASE         (0x00800000UL)
#define CHN_CTRL_EN(x)					(((x) & 0x1)<<0)
#define CHN_CTRL_INT_TC_MASK(x)		(((x) & 0x1)<<1)
#define CHN_CTRL_INT_ERR_MASK(x)		(((x) & 0x1)<<2)
#define CHN_CTRL_INT_ABT_MASK(x)		(((x) & 0x1)<<3)
#define CHN_CTRL_DST_REQ_SEL(x)		(((x) & 0xF)<<4)
#define CHN_CTRL_SRC_REQ_SEL(x)		(((x) & 0xF)<<8)
#define CHN_CTRL_DST_ADDR_CTRL(x)		(((x) & 0x3)<<12)
#define CHN_CTRL_SRC_ADDR_CTRL(x)		(((x) & 0x3)<<14)
#define CHN_CTRL_DST_MODE(x)			(((x) & 0x1)<<16)
#define CHN_CTRL_SRC_MODE(x)			(((x) & 0x1)<<17)
#define CHN_CTRL_DST_WIDTH(x)			(((x) & 0x3)<<18)
#define CHN_CTRL_SRC_WIDTH(x)			(((x) & 0x3)<<20)
#define CHN_CTRL_SRC_BURST_SIZE(x)		(((x) & 0x7)<<22)
#define CHN_CTRL_PRIORITY(x)			(((x) & 0x1)<<29)

#define CHN_ENABLE		1
#define CHN_DISABLE		0

#define CHN_INT_MASK		1
#define CHN_INT_UNMASK		0


#define CHN_HANDSHAKE_ID_UART1TX		0
#define CHN_HANDSHAKE_ID_UART1RX		1
#define CHN_HANDSHAKE_ID_UART2TX		2
#define CHN_HANDSHAKE_ID_UART2RX		3
#define CHN_HANDSHAKE_ID_I2C			4
#define CHN_HANDSHAKE_ID_SPI1TX		    5
#define CHN_HANDSHAKE_ID_SPI1RX		    6
#define CHN_HANDSHAKE_ID_SPI2TX		    7
#define CHN_HANDSHAKE_ID_SPI2RX		    8
#define CHN_HANDSHAKE_ID_I2STX		    9
#define CHN_HANDSHAKE_ID_I2SRX		    10
#define CHN_HANDSHAKE_ID_UART3TX        11
#define CHN_HANDSHAKE_ID_UART3RX	    12

#define CHN_ADDR_INC	0
#define CHN_ADDR_DEC	1
#define CHN_ADDR_FIX	2

#define CHN_SRC_MODE_NORMAL		0
#define CHN_SRC_MODE_HANDSHAKE	1

#define CHN_WIDTH_BYTE			0
#define CHN_WIDTH_HALF_WORLD	1
#define CHN_WIDTH_WORLD		2

#define CHN_BURST_SIZE_1_TRANS	0
#define CHN_BURST_SIZE_2_TRANS	1
#define CHN_BURST_SIZE_4_TRANS	2
#define CHN_BURST_SIZE_8_TRANS	3
#define CHN_BURST_SIZE_16_TRANS	4
#define CHN_BURST_SIZE_32_TRANS	5
#define CHN_BURST_SIZE_64_TRANS	6
#define CHN_BURST_SIZE_128_TRANS	7

#define CHN_PRIORITY_LOW	0
#define CHN_PRIORITY_HIGH	1



#define CHN_CTRL_VALUE_DEBUG		CHN_CTRL_EN(CHN_ENABLE)		\
								|CHN_CTRL_INT_TC_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ERR_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ABT_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_DST_ADDR_CTRL(CHN_ADDR_INC)	\
								|CHN_CTRL_SRC_ADDR_CTRL(CHN_ADDR_INC)	\
								|CHN_CTRL_DST_MODE(CHN_SRC_MODE_NORMAL) \
								|CHN_CTRL_DST_WIDTH(CHN_WIDTH_WORLD)	\
								|CHN_CTRL_SRC_WIDTH(CHN_WIDTH_WORLD)	\
								|CHN_CTRL_SRC_BURST_SIZE(CHN_BURST_SIZE_128_TRANS)	\
								|CHN_CTRL_PRIORITY(CHN_PRIORITY_HIGH)




#define CHN_CTRL_VALUE		CHN_CTRL_EN(CHN_ENABLE)		\
								|CHN_CTRL_INT_TC_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ERR_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ABT_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_DST_ADDR_CTRL(CHN_ADDR_INC)	\
								|CHN_CTRL_SRC_ADDR_CTRL(CHN_ADDR_INC)	\
								|CHN_CTRL_DST_MODE(CHN_SRC_MODE_NORMAL) \
								|CHN_CTRL_DST_WIDTH(CHN_WIDTH_BYTE)	\
								|CHN_CTRL_SRC_WIDTH(CHN_WIDTH_BYTE)	\
								|CHN_CTRL_SRC_BURST_SIZE(CHN_BURST_SIZE_1_TRANS)	\
								|CHN_CTRL_PRIORITY(CHN_PRIORITY_LOW)


#define CHN_CTRL_VALUE_UART0TX		CHN_CTRL_EN(CHN_ENABLE)		\
								|CHN_CTRL_INT_TC_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ERR_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ABT_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_DST_REQ_SEL(CHN_HANDSHAKE_ID_UART1TX) \
								|CHN_CTRL_DST_ADDR_CTRL(CHN_ADDR_FIX)	\
								|CHN_CTRL_SRC_ADDR_CTRL(CHN_ADDR_INC)	\
								|CHN_CTRL_DST_MODE(CHN_SRC_MODE_HANDSHAKE) \
								|CHN_CTRL_DST_WIDTH(CHN_WIDTH_BYTE)	\
								|CHN_CTRL_SRC_WIDTH(CHN_WIDTH_BYTE)	\
								|CHN_CTRL_SRC_BURST_SIZE(CHN_BURST_SIZE_1_TRANS)	\
								|CHN_CTRL_PRIORITY(CHN_PRIORITY_LOW)

#define CHN_CTRL_VALUE_UART0RX		CHN_CTRL_EN(CHN_ENABLE)		\
								|CHN_CTRL_INT_TC_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ERR_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ABT_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_SRC_REQ_SEL(CHN_HANDSHAKE_ID_UART1RX) \
								|CHN_CTRL_DST_ADDR_CTRL(CHN_ADDR_INC)	\
								|CHN_CTRL_SRC_ADDR_CTRL(CHN_ADDR_FIX)	\
								|CHN_CTRL_SRC_MODE(CHN_SRC_MODE_HANDSHAKE) \
								|CHN_CTRL_DST_WIDTH(CHN_WIDTH_BYTE)	\
								|CHN_CTRL_SRC_WIDTH(CHN_WIDTH_BYTE)	\
								|CHN_CTRL_SRC_BURST_SIZE(CHN_BURST_SIZE_1_TRANS)	\
								|CHN_CTRL_PRIORITY(CHN_PRIORITY_LOW)
								
#define CHN_CTRL_VALUE_UART1TX		CHN_CTRL_EN(CHN_ENABLE)		\
									|CHN_CTRL_INT_TC_MASK(CHN_INT_UNMASK) \
									|CHN_CTRL_INT_ERR_MASK(CHN_INT_UNMASK)	\
									|CHN_CTRL_INT_ABT_MASK(CHN_INT_UNMASK)	\
									|CHN_CTRL_DST_REQ_SEL(CHN_HANDSHAKE_ID_UART2TX) \
									|CHN_CTRL_DST_ADDR_CTRL(CHN_ADDR_FIX)	\
									|CHN_CTRL_SRC_ADDR_CTRL(CHN_ADDR_INC)	\
									|CHN_CTRL_DST_MODE(CHN_SRC_MODE_HANDSHAKE) \
									|CHN_CTRL_DST_WIDTH(CHN_WIDTH_BYTE) \
									|CHN_CTRL_SRC_WIDTH(CHN_WIDTH_BYTE) \
									|CHN_CTRL_SRC_BURST_SIZE(CHN_BURST_SIZE_1_TRANS)	\
									|CHN_CTRL_PRIORITY(CHN_PRIORITY_LOW)
	
#define CHN_CTRL_VALUE_UART1RX		CHN_CTRL_EN(CHN_ENABLE)		\
									|CHN_CTRL_INT_TC_MASK(CHN_INT_UNMASK)	\
									|CHN_CTRL_INT_ERR_MASK(CHN_INT_UNMASK)	\
									|CHN_CTRL_INT_ABT_MASK(CHN_INT_UNMASK)	\
									|CHN_CTRL_SRC_REQ_SEL(CHN_HANDSHAKE_ID_UART2RX) \
									|CHN_CTRL_DST_ADDR_CTRL(CHN_ADDR_INC)	\
									|CHN_CTRL_SRC_ADDR_CTRL(CHN_ADDR_FIX)	\
									|CHN_CTRL_SRC_MODE(CHN_SRC_MODE_HANDSHAKE) \
									|CHN_CTRL_DST_WIDTH(CHN_WIDTH_BYTE) \
									|CHN_CTRL_SRC_WIDTH(CHN_WIDTH_BYTE) \
									|CHN_CTRL_SRC_BURST_SIZE(CHN_BURST_SIZE_1_TRANS)	\
									|CHN_CTRL_PRIORITY(CHN_PRIORITY_LOW)




#define CHN_CTRL_VALUE_SPI2TX		CHN_CTRL_EN(CHN_ENABLE)		\
								|CHN_CTRL_INT_TC_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ERR_MASK(CHN_INT_MASK)	\
								|CHN_CTRL_INT_ABT_MASK(CHN_INT_MASK)	\
								|CHN_CTRL_DST_REQ_SEL(CHN_HANDSHAKE_ID_SPI2TX) \
								|CHN_CTRL_DST_ADDR_CTRL(CHN_ADDR_FIX)	\
								|CHN_CTRL_SRC_ADDR_CTRL(CHN_ADDR_INC)	\
								|CHN_CTRL_DST_MODE(CHN_SRC_MODE_HANDSHAKE) \
								|CHN_CTRL_DST_WIDTH(CHN_WIDTH_WORLD)	\
								|CHN_CTRL_SRC_WIDTH(CHN_WIDTH_WORLD)	\
								|CHN_CTRL_SRC_BURST_SIZE(CHN_BURST_SIZE_1_TRANS)	\
								|CHN_CTRL_PRIORITY(CHN_PRIORITY_LOW)

#define CHN_CTRL_VALUE_SPI2RX		CHN_CTRL_EN(CHN_ENABLE)		\
								|CHN_CTRL_INT_TC_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ERR_MASK(CHN_INT_MASK)	\
								|CHN_CTRL_INT_ABT_MASK(CHN_INT_MASK)	\
								|CHN_CTRL_SRC_REQ_SEL(CHN_HANDSHAKE_ID_SPI2RX) \
								|CHN_CTRL_DST_ADDR_CTRL(CHN_ADDR_INC)	\
								|CHN_CTRL_SRC_ADDR_CTRL(CHN_ADDR_FIX)	\
								|CHN_CTRL_SRC_MODE(CHN_SRC_MODE_HANDSHAKE) \
								|CHN_CTRL_DST_WIDTH(CHN_WIDTH_WORLD)	\
								|CHN_CTRL_SRC_WIDTH(CHN_WIDTH_WORLD)	\
								|CHN_CTRL_SRC_BURST_SIZE(CHN_BURST_SIZE_1_TRANS)	\
								|CHN_CTRL_PRIORITY(CHN_PRIORITY_LOW)
#define CHN_CTRL_VALUE_SPI1TX		CHN_CTRL_EN(CHN_ENABLE)		\
								|CHN_CTRL_INT_TC_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ERR_MASK(CHN_INT_MASK)	\
								|CHN_CTRL_INT_ABT_MASK(CHN_INT_MASK)	\
								|CHN_CTRL_DST_REQ_SEL(CHN_HANDSHAKE_ID_SPI1TX) \
								|CHN_CTRL_DST_ADDR_CTRL(CHN_ADDR_FIX)	\
								|CHN_CTRL_SRC_ADDR_CTRL(CHN_ADDR_INC)	\
								|CHN_CTRL_DST_MODE(CHN_SRC_MODE_HANDSHAKE) \
								|CHN_CTRL_DST_WIDTH(CHN_WIDTH_BYTE)	\
								|CHN_CTRL_SRC_WIDTH(CHN_WIDTH_BYTE)	\
								|CHN_CTRL_SRC_BURST_SIZE(CHN_BURST_SIZE_1_TRANS)	\
								|CHN_CTRL_PRIORITY(CHN_PRIORITY_LOW)

#define CHN_CTRL_VALUE_SPI1RX		CHN_CTRL_EN(CHN_ENABLE)		\
								|CHN_CTRL_INT_TC_MASK(CHN_INT_UNMASK)	\
								|CHN_CTRL_INT_ERR_MASK(CHN_INT_MASK)	\
								|CHN_CTRL_INT_ABT_MASK(CHN_INT_MASK)	\
								|CHN_CTRL_SRC_REQ_SEL(CHN_HANDSHAKE_ID_SPI1RX) \
								|CHN_CTRL_DST_ADDR_CTRL(CHN_ADDR_INC)	\
								|CHN_CTRL_SRC_ADDR_CTRL(CHN_ADDR_FIX)	\
								|CHN_CTRL_SRC_MODE(CHN_SRC_MODE_HANDSHAKE) \
								|CHN_CTRL_DST_WIDTH(CHN_WIDTH_BYTE)	\
								|CHN_CTRL_SRC_WIDTH(CHN_WIDTH_BYTE)	\
								|CHN_CTRL_SRC_BURST_SIZE(CHN_BURST_SIZE_1_TRANS)	\
								|CHN_CTRL_PRIORITY(CHN_PRIORITY_LOW)



#define INT_STATUS_ABORT(x)	(((x)>>8) & 0xFF)
#define INT_STATUS_ERROR(x)	(((x)>>0) & 0xFF)
#define INT_STATUS_TC(x)		(((x)>>16) & 0xFF)


atcdmac100_dev  s_atcdmac_dev = {0};




void dmac_config(int chnum, unsigned int size, unsigned int* srcAddr, unsigned int* dstAddr)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;
	channel_ctrl * pChannel = &(pDmaDev->pdmaBase->ChnCtrl[chnum]);


	OUT32(&pChannel->ChnLLPointer, 0);
	OUT32(&pChannel->ChnTranSize, size/4);
	OUT32(&pChannel->ChnSrcAddr, srcAddr);
	OUT32(&pChannel->ChnDstAddr, dstAddr);

}


void dmac_config_uart_tx(int chnum, unsigned int size, void * srcAddr,void * dstAddr)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;
	channel_ctrl * pChannel = &(pDmaDev->pdmaBase->ChnCtrl[chnum]);


	OUT32(&pChannel->ChnLLPointer, 0);
	OUT32(&pChannel->ChnTranSize, size);
	OUT32(&pChannel->ChnSrcAddr,  srcAddr);
	OUT32(&pChannel->ChnDstAddr, dstAddr);

}


void dmac_config_uart_rx(int chnum, unsigned int size, void * srcAddr,void * dstAddr)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;
	channel_ctrl * pChannel = &(pDmaDev->pdmaBase->ChnCtrl[chnum]);


	OUT32(&pChannel->ChnLLPointer, 0);
	OUT32(&pChannel->ChnTranSize, size);
	OUT32(&pChannel->ChnSrcAddr, srcAddr);
	OUT32(&pChannel->ChnDstAddr, dstAddr);

}

void dmac_config_spi2_tx(int chnum, unsigned int size, void * srcAddr,void * dstAddr)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;
	channel_ctrl * pChannel = &(pDmaDev->pdmaBase->ChnCtrl[chnum]);


	OUT32(&pChannel->ChnLLPointer, 0);
	OUT32(&pChannel->ChnTranSize, size);
	OUT32(&pChannel->ChnSrcAddr, srcAddr);
	OUT32(&pChannel->ChnDstAddr, dstAddr);

}


void dmac_config_spi2_rx(int chnum, unsigned int size, void * srcAddr,void * dstAddr)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;
	channel_ctrl * pChannel = &(pDmaDev->pdmaBase->ChnCtrl[chnum]);


	OUT32(&pChannel->ChnLLPointer, 0);
	OUT32(&pChannel->ChnTranSize, size);
	OUT32(&pChannel->ChnSrcAddr, srcAddr);
	OUT32(&pChannel->ChnDstAddr, dstAddr);

}

#define IN8(reg) 		(unsigned char)( (*(volatile unsigned long *)(reg)) & 0x000000FF)


/* UART register offsets (4~8-bit width) */
/* SD_LCR_DLAB == 0 */
#define UARTC_RBR_OFFSET                0x20 /* receiver biffer register */
#define UARTC_THR_OFFSET                0x20 /* transmitter holding register */
#define UARTC_IER_OFFSET                0x24 /* interrupt enable register */
#define UARTC_IIR_OFFSET                0x28 /* interrupt identification register */
#define UARTC_FCR_OFFSET                0x28 /* FIFO control register */
#define UARTC_LCR_OFFSET                0x2c /* line control regitser */
#define UARTC_MCR_OFFSET                0x30 /* modem control register */
#define UARTC_LSR_OFFSET                0x34 /* line status register */
#define UARTC_TST_OFFSET                0x34 /* testing register */
#define UARTC_MSR_OFFSET                0x38 /* modem status register */
#define UARTC_SPR_OFFSET                0x3c /* scratch pad register */

/* SD_LCR_DLAB == 0 */
#define UARTC_DLL_OFFSET                0x20 /* baudrate divisor latch LSB */
#define UARTC_DLM_OFFSET                0x24 /* baudrate divisor latch MSB */
#define UARTC_PSR_OFFSET                0x28 /* prescaler register */


#define UARTC_LSR_RDR                   0x1 /* Data Ready */
#define UARTC_LSR_THRE                  0x20 /* THR/FIFO Empty */
#define UARTC_LSR_TEMT                  0x40 /* THR/TSR Empty */

#define DMAC_CHN_CTRL(ch) 				(DMAC_BASE + 0x44 + 0x14*ch)


void dmac_start(int chnum)
{
#if 1
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;
	channel_ctrl * pChannel = &pDmaDev->pdmaBase->ChnCtrl[chnum];
    // system_printf("---------&pChannel->ChnCtrl---------%08x %08x\n",&pChannel->ChnCtrl,s_atcdmac_dev.ChnCfg);
    OUT32(&pChannel->ChnCtrl, s_atcdmac_dev.ChnCfg);
#else
	memcpy(ADDR_DES, ADDR_SRC, TEST_LENGTH);

	if(0==memcmp(ADDR_SRC, ADDR_DES, TEST_LENGTH))
	{
		//debug("dma OK!\n");
	
		serial_puts("dma OK!\n");
	}
	else
	{

			//debug("dma fail!\n");

		serial_puts("dma fail!\n");
	}

	dma_test_over = 1;

#endif	
}

void dmac_start_s(int chnum)
{

	OUT32(DMAC_CHN_CTRL(chnum), IN32(DMAC_CHN_CTRL(chnum)) | 0x1);

}
extern uint8_t uart_rx_dma_ch;
extern uint8_t uart_tx_dma_ch;

void dma_isr(unsigned int * int_status)
{
	atcdmac100_reg * pdmaBase = s_atcdmac_dev.pdmaBase;
	
    if((pdmaBase->IntStatus & (1<<(16+uart_rx_dma_ch))) != 0)
	{
		pdmaBase->IntStatus = (1<<(16+uart_rx_dma_ch));
		hal_uart_rx_dma_isr();		
    }
	else if((pdmaBase->IntStatus & (1<<(16+uart_tx_dma_ch))) != 0)
	{		
		pdmaBase->IntStatus = (1<<(16+uart_tx_dma_ch));
		hal_uart_tx_dma_isr();
	}

	else if((pdmaBase->IntStatus & 0x00010000) != 0)
	{
		pdmaBase->IntStatus = 0x00010000;
		spi_rx_isr();
	}
	else
	{
		//system_printf("dma isr pdmaBase->IntStatus=0x%x\n\a",pdmaBase->IntStatus);
		pdmaBase->IntStatus = 0x00FFFFFF;
	}
}



// void dmac_isr(unsigned int * int_status)
// {
// 	atcdmac100_reg * pdmaBase = s_atcdmac_dev.pdmaBase;

// 	*int_status = pdmaBase->IntStatus;

// 	if(*int_status)
// 		pdmaBase->IntStatus = *int_status;
// }

#if 1
void dma_init(void)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;

	pDmaDev->pdmaBase = (atcdmac100_reg * )DMAC_BASE;


	//debug("dmctrl reset: 0x%x\n", &pDmaDev->pdmaBase->DMACtrl);


	/* reset dma */
	//writel(0x1, pDmaDev->pdmaBase->DMACtrl);

	s_atcdmac_dev.ChnCfg = CHN_CTRL_VALUE ;
}

void dma_init_debug(void)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;

	pDmaDev->pdmaBase =(atcdmac100_reg * ) DMAC_BASE;


	//debug("dmctrl reset: 0x%x\n", &pDmaDev->pdmaBase->DMACtrl);


	/* reset dma */
	//writel(0x1, pDmaDev->pdmaBase->DMACtrl);

	s_atcdmac_dev.ChnCfg = CHN_CTRL_VALUE_DEBUG;
}

void dma_init_isr()
{
	irq_status_clean(IRQ_VECTOR_DMA);
    irq_isr_register(IRQ_VECTOR_DMA, (void *)dma_isr);
	irq_unmask(IRQ_VECTOR_DMA);
}


#endif
	
void dmac_init_UART0_TX(void)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;

	pDmaDev->pdmaBase = (atcdmac100_reg * )DMAC_BASE;
	
	s_atcdmac_dev.ChnCfg = CHN_CTRL_VALUE_UART0TX;
}

void dmac_init_UART0_RX(void)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;

	pDmaDev->pdmaBase =(atcdmac100_reg * ) DMAC_BASE;

	s_atcdmac_dev.ChnCfg = CHN_CTRL_VALUE_UART0RX;
}

void dmac_init_UART1_TX(void)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;

	pDmaDev->pdmaBase = (atcdmac100_reg * )DMAC_BASE;
	
	s_atcdmac_dev.ChnCfg = CHN_CTRL_VALUE_UART1TX;
}

void dmac_init_UART1_RX(void)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;

	pDmaDev->pdmaBase =(atcdmac100_reg * ) DMAC_BASE;

	s_atcdmac_dev.ChnCfg = CHN_CTRL_VALUE_UART1RX;
}


void dmac_reset(void)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;

	pDmaDev->pdmaBase =(atcdmac100_reg * ) DMAC_BASE;
	pDmaDev->pdmaBase->DMACtrl = 1;
}

void dmac_init_SPI2_TX(void)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;

	pDmaDev->pdmaBase = (atcdmac100_reg * )DMAC_BASE;
	
	s_atcdmac_dev.ChnCfg = CHN_CTRL_VALUE_SPI2TX;
}


void dmac_init_SPI2_RX(void)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;

	pDmaDev->pdmaBase = (atcdmac100_reg * )DMAC_BASE;

	s_atcdmac_dev.ChnCfg = CHN_CTRL_VALUE_SPI2RX;
}
void dmac_init_SPI1_TX(void)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;

	pDmaDev->pdmaBase = (atcdmac100_reg * )DMAC_BASE;
	
	s_atcdmac_dev.ChnCfg = CHN_CTRL_VALUE_SPI1TX;
}


void dmac_init_SPI1_RX(void)
{
	atcdmac100_dev * pDmaDev = &s_atcdmac_dev;

	pDmaDev->pdmaBase = (atcdmac100_reg * )DMAC_BASE;
	
	s_atcdmac_dev.ChnCfg = CHN_CTRL_VALUE_SPI1RX;
}


/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:hal_spi_salve.c    
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

/****************************************************************************
* 	                                           Include files
****************************************************************************/
#include "system_common.h"
#include "drv_spi_slave.h"
#include "soc_pin_mux.h"
#include "soc_top_reg.h"

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


#define SPI_STS_SLAVE_CMD_INT		0x20
#define SPI_INT_SLAVE_CMD_EN		0x20


#define SPI_PREPARE_BUS(X)			\
		do{unsigned int spi_status = 0; 			\
		do {								\
			spi_status = (X);	\
		} while(spi_status & SPI_STATUS_BUSY);}while(0)
	
	
#define SPI_CLEAR_FIFO(X)			X |= (SPI_CONTROL_RXFIFORST|SPI_CONTROL_TXFIFORST)
#define SPI_CLEAR_FIFOTX(X)			X |= SPI_CONTROL_TXFIFORST
#define SPI_CLEAR_FIFORX(X)			X |= SPI_CONTROL_RXFIFORST
	
	
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
static spi_slave_dev spi_slave;

static spi_slave_buff_dev * p_spi_slave_dev;

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

void spi_slave_write_spi()
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	//system_printf("spi_slave_write_spi\n");
	spi_reg * pSpiReg = (spi_reg *)spi_slave.spi_base;
	unsigned int data,i,left;
	while((pSpiReg->status & SPI_STATUS_TXFULL) == 0)
	{
		if(p_spi_slave_dev->spi_tx_slave_buf_head == 0 && p_spi_slave_dev->spi_tx_slave_buf_tail == 0)
		{
			unsigned int value = IN32(spi_slave.spi_base+0x38) & (~BIT3);
			OUT32(spi_slave.spi_base+0x38, value);
			break;
		}
		else
		{
			
			//system_printf("p_spi_dev->spi_tx_buf_head = %d\n",p_spi_dev->spi_tx_buf_head);
			//system_printf("p_spi_dev->spi_tx_buf_tail = %d\n",p_spi_dev->spi_tx_buf_tail);
				
			int length = 0;
			if(p_spi_slave_dev->spi_tx_slave_buf_head>=p_spi_slave_dev->spi_tx_slave_buf_tail)
				length = p_spi_slave_dev->spi_tx_slave_buf_head - p_spi_slave_dev->spi_tx_slave_buf_tail;
			else
				length = p_spi_slave_dev->spi_tx_slave_buf_head + SPI_SLAVE_TX_BUF_SIZE - p_spi_slave_dev->spi_tx_slave_buf_tail;
					
		
			while(length>0)
			{
				if((pSpiReg->status & SPI_STATUS_TXFULL) != 0)
					break;
				else
				{
					data = 0;
						
					// data are usually be read 32bits once a time 
					left = min(length, 4);
					for (i = 0; i < left; i++) 
					{
						data |= p_spi_slave_dev->spi_tx_slave_buf[p_spi_slave_dev->spi_tx_slave_buf_tail] << (i * 8);
						p_spi_slave_dev->spi_tx_slave_buf_tail++;
						if(p_spi_slave_dev->spi_tx_slave_buf_tail == SPI_SLAVE_TX_BUF_SIZE)
							p_spi_slave_dev->spi_tx_slave_buf_tail = 0;
					}
					// wait till TXFULL is deasserted 
					//SPI_WAIT_TX_READY(pSpiReg->status); 	
					pSpiReg->data = data;
					//system_printf("data=0x%x\n",data);
					if(p_spi_slave_dev->spi_tx_slave_buf_head>=p_spi_slave_dev->spi_tx_slave_buf_tail)
						length = p_spi_slave_dev->spi_tx_slave_buf_head - p_spi_slave_dev->spi_tx_slave_buf_tail;
					else
						length = p_spi_slave_dev->spi_tx_slave_buf_head + SPI_SLAVE_TX_BUF_SIZE - p_spi_slave_dev->spi_tx_slave_buf_tail;
				}
			}
			if(length == 0)
			{	
				p_spi_slave_dev->spi_tx_slave_buf_head = 0;
				p_spi_slave_dev->spi_tx_slave_buf_tail = 0;
				
				unsigned int value = IN32(spi_slave.spi_base+0x38) & (~BIT3);
				OUT32(spi_slave.spi_base+0x38, value);
			}						
		}
	}
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
}

void spi_slave_init(spi_slave_dev *spi_slave_dev)
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	spi_slave = *spi_slave_dev;
	
	unsigned int value;
	#if 1
	//xBinarySemaphore_spi_slave = xSemaphoreCreateCounting(32, 0);

	//diable clk
	OUT32(CLK_EN_BASE, IN32(CLK_EN_BASE) |(~(CLK_SPI1|CLK_SPI_FLASH |CLK_SPI_FLASH_AHB)));

	unsigned int temp;
	temp = IN32(spi_slave_dev->spi_base+0x30);
	OUT32(spi_slave_dev->spi_base+0x30, SPI_CONTROL_RXFIFORST);
	
	temp = IN32(spi_slave_dev->spi_base+0x30);
	while((temp & 0x2) != 0)
	{
		temp = IN32(spi_slave_dev->spi_base+0x30);
	}
	
	temp = IN32(spi_slave_dev->spi_base+0x3C);
	OUT32(spi_slave_dev->spi_base+0x3C, 0xFFFFFFFF);
	temp = IN32(spi_slave_dev->spi_base+0x3C);
	
	if(spi_slave_dev->spi_base == MSPI)
	{

		p_spi_slave_dev->regbase = MSPI;
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

		irq_isr_register(IRQ_VECTOR_SPIM, (void *)spi_slave_load);
		irq_status_clean(IRQ_VECTOR_SPIM);
		irq_unmask(IRQ_VECTOR_SPIM);
	}
	else if(spi_slave_dev->spi_base == SPI0)
	{
		p_spi_slave_dev->regbase = SPI0;
		//cfg pin
		value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<0));
		OUT32(SOC_PIN0_MUX_BASE, value |(3<<0));	/* SPI GPIO0-CLK MUX CFG */
	
		value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<3));
		OUT32(SOC_PIN0_MUX_BASE, value |(3<<3));	/* SPI GPIO1-CS0 MUX CFG */

		value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<6));
		OUT32(SOC_PIN0_MUX_BASE, value |(3<<6));	/* SPI GPIO2-MOSI MUX CFG */

		value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<9));
		OUT32(SOC_PIN0_MUX_BASE, value |(3<<9));		/* SPI GPIO3-MISO MUX CFG */

		//value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<15));
		//OUT32(SOC_PIN0_MUX_BASE, value |(4<<15));		/* MSPI GPIO5-HOLD MUX CFG */
		
		//value = IN32(SOC_PIN0_MUX_BASE) & (~(7<<18));
		//OUT32(SOC_PIN0_MUX_BASE, value |(4<<18));		/* MSPI GPIO6-WP MUX CFG */

		irq_isr_register(IRQ_VECTOR_SPI1, (void *)spi_slave_load);
		irq_status_clean(IRQ_VECTOR_SPI1);
		irq_unmask(IRQ_VECTOR_SPI1);

	}
	else
	{
		system_printf("spi_base error");
	}
	//system_printf("sleep begin\n");
	
	delay(1000000);
	//system_printf("sleep over\n");
	OUT32(CLK_EN_BASE, IN32(CLK_EN_BASE) |(CLK_SPI1|CLK_SPI_FLASH |CLK_SPI_FLASH_AHB));	
	#endif
	
	p_spi_slave_dev = pvPortMalloc(sizeof(spi_slave_buff_dev));

	p_spi_slave_dev->spi_rx_slave_buf_head = 0;
	p_spi_slave_dev->spi_rx_slave_buf_tail = 0;
	p_spi_slave_dev->spi_tx_slave_buf_head = 0;
	p_spi_slave_dev->spi_tx_slave_buf_tail = 0;
	memset(p_spi_slave_dev->spi_rx_slave_buf,0,SPI_SLAVE_RX_BUF_SIZE);
	memset(p_spi_slave_dev->spi_tx_slave_buf,0,SPI_SLAVE_TX_BUF_SIZE);

	

	//salve mode & 3 line
	value = IN32(spi_slave_dev->spi_base+0x10) | BIT2;	
	OUT32(spi_slave_dev->spi_base+0x10, value | 0x10);	

	//addr length
	value = IN32(spi_slave_dev->spi_base+0x10) & 0xFFFCFFFF;
	OUT32(spi_slave_dev->spi_base+0x10, value |((spi_slave_dev->addr_len-1) << 16));

	//data length
	value = IN32(spi_slave_dev->spi_base+0x10) & 0xFFFFE0FF;
	OUT32(spi_slave_dev->spi_base+0x10, value |((spi_slave_dev->data_len-1) << 8));

	//cfg timing
	value = IN32(spi_slave_dev->spi_base+0x40) & 0xFFFFFF00;
	OUT32(spi_slave_dev->spi_base+0x40, value |(spi_slave_dev->master_clk & 0xFF));

	value = IN32(spi_slave_dev->spi_base+0x30) | 0x70000;
	OUT32(spi_slave_dev->spi_base+0x30, value);

	//spi-int 0x20
	OUT32(spi_slave_dev->spi_base+0x38, spi_slave_dev->inten);
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
}

int spi_slave_write(unsigned char *buf,unsigned int length)
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	if(length>512 || length<0)
	{
		system_printf("Length Error\n");
#ifdef _USE_PSM
		TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
		return -1;
	}
	int i,length_head_to_end;	
	
	int length_tem = length+4-length%4;
	unsigned int value = 0;
	//system_printf("length_tem=%d\n",length_tem);
	
	int avil_lenth = SPI_SLAVE_TX_BUF_SIZE;
	if((p_spi_slave_dev->spi_tx_slave_buf_head == p_spi_slave_dev->spi_tx_slave_buf_tail) && (p_spi_slave_dev->spi_tx_slave_buf_head!=0))
	{
		system_printf("No Enough Buffer Space\n");
		value = IN32(spi_slave.spi_base+0x38) | BIT3;
		OUT32(spi_slave.spi_base+0x38, value);
#ifdef _USE_PSM
		TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
		return -1;
	}
	if(p_spi_slave_dev->spi_tx_slave_buf_head >= p_spi_slave_dev->spi_tx_slave_buf_tail)
	{	
		avil_lenth = SPI_SLAVE_TX_BUF_SIZE - (p_spi_slave_dev->spi_tx_slave_buf_head - p_spi_slave_dev->spi_tx_slave_buf_tail);
		if(length_tem > (avil_lenth))
		{	
			system_printf("No Enough Buffer Space\n");
			value = IN32(spi_slave.spi_base+0x38) | BIT3;
			OUT32(spi_slave.spi_base+0x38, value);
#ifdef _USE_PSM
			TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
			return -1;
		}	
		length_head_to_end = SPI_SLAVE_TX_BUF_SIZE - p_spi_slave_dev->spi_tx_slave_buf_head;
		if(length > length_head_to_end)
		{
			memcpy(&p_spi_slave_dev->spi_tx_slave_buf[p_spi_slave_dev->spi_tx_slave_buf_head],&buf[0],length_head_to_end);
			p_spi_slave_dev->spi_tx_slave_buf_head = 0;
			memcpy(&p_spi_slave_dev->spi_tx_slave_buf[p_spi_slave_dev->spi_tx_slave_buf_head],&buf[length_head_to_end],length - length_head_to_end);
			p_spi_slave_dev->spi_tx_slave_buf_head = length - length_head_to_end;
		}
		else
		{
			memcpy(&p_spi_slave_dev->spi_tx_slave_buf[p_spi_slave_dev->spi_tx_slave_buf_head],&buf[0],length);
			p_spi_slave_dev->spi_tx_slave_buf_head += length;
		}
	}
	else if(p_spi_slave_dev->spi_tx_slave_buf_head < p_spi_slave_dev->spi_tx_slave_buf_tail)
	{
		avil_lenth = p_spi_slave_dev->spi_tx_slave_buf_tail - p_spi_slave_dev->spi_tx_slave_buf_head;
		if(length_tem > avil_lenth)
		{	
			system_printf("No Enough Buffer Space\n");
			value = IN32(spi_slave.spi_base+0x38) | BIT3;
			OUT32(spi_slave.spi_base+0x38, value);
#ifdef _USE_PSM
			TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
			return -1;
		}
		memcpy(&p_spi_slave_dev->spi_tx_slave_buf[p_spi_slave_dev->spi_tx_slave_buf_head],&buf[0],length);
		p_spi_slave_dev->spi_tx_slave_buf_head += length;
	}
	for(i=0;i<(length_tem - length);++i)
	{
		if(p_spi_slave_dev->spi_tx_slave_buf_head == SPI_SLAVE_TX_BUF_SIZE)
			p_spi_slave_dev->spi_tx_slave_buf_head = 0;
		p_spi_slave_dev->spi_tx_slave_buf[p_spi_slave_dev->spi_tx_slave_buf_head++] = 0x0;		
	}
	
	value = IN32(spi_slave.spi_base+0x38) | BIT3;
	OUT32(spi_slave.spi_base+0x38, value);
	
	//xSemaphoreGive(xBinarySemaphore_spi_slave);
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
	return 0;

}

void spi_slave_read_to_buffer()
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	unsigned int temp,i;

	spi_reg * pSpiReg = (spi_reg *)spi_slave.spi_base;
	
	unsigned int *p_dst_buffer = (unsigned int *)(p_spi_slave_dev->spi_rx_slave_buf+p_spi_slave_dev->spi_rx_slave_buf_head);
	//p_dst_buffer = &(p_spi_slave_dev->spi_rx_slave_buf[p_spi_slave_dev->spi_rx_slave_buf_head]);
	temp = pSpiReg->status;
	int rxnum = ((temp & 0x00001f00)>>8);
	int data_rec = 0;
	while((temp & SPI_STATUS_BUSY) || rxnum>0 )
	{
		//system_printf("pSpiReg->status=0x%x\n",pSpiReg->status);
		for(i=0; i<rxnum; i++)
		{			
			*p_dst_buffer++ = pSpiReg->data;
			data_rec = 1;			
		}
		temp = pSpiReg->status;
		rxnum = ((temp & 0x00001f00)>>8) ;
		//system_printf("pSpiReg->status=0x%x\n",temp);
	}
	rxnum = ((pSpiReg->status) & 0x1F00) >> 8;
	if(rxnum>0)
		for(i=0; i<rxnum; i++)
		{			
			*p_dst_buffer++ = pSpiReg->data;
			data_rec = 1;			
		}
	int length = (pSpiReg->slvDataCnt & 0x1FF);
	if(length==0 && data_rec == 1)
		length = 512;
	p_spi_slave_dev->spi_rx_slave_buf_head += length;
	
	#if 0
	if(p_spi_slave_dev->spi_rx_slave_buf_head != 512 || p_spi_slave_dev->spi_rx_slave_buf_tail != 0)
	{
		system_printf("p_spi_slave_dev->spi_rx_slave_buf_head = %d\n",p_spi_slave_dev->spi_rx_slave_buf_head);
		system_printf("p_spi_slave_dev->spi_rx_slave_buf_tail = %d\n",p_spi_slave_dev->spi_rx_slave_buf_tail);
		system_printf("Fail\n");
		
	}
	
	int ok = 1;
	for(i=0;i<512;++i)
		if(p_spi_slave_dev->spi_rx_slave_buf[i]!=(i%256))
		{
			ok=0;
			system_printf("spi_rx_slave_buf[%d]=%d\n",i,p_spi_slave_dev->spi_rx_slave_buf[i]);
		}
	if(ok==1)
		system_printf("Pass\n");
	else
		system_printf("Fail\n");
	p_spi_slave_dev->spi_rx_slave_buf_head = 0;
	p_spi_slave_dev->spi_rx_slave_buf_tail = 0;
	#endif
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
}

int spi_slave_read(unsigned int *pdata,unsigned int length)
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif	
	int buffer_length = p_spi_slave_dev->spi_rx_slave_buf_head - p_spi_slave_dev->spi_rx_slave_buf_tail;

	if(length >= buffer_length)
	{
		memcpy(pdata,p_spi_slave_dev->spi_rx_slave_buf+p_spi_slave_dev->spi_rx_slave_buf_tail ,buffer_length);
		p_spi_slave_dev->spi_rx_slave_buf_head = 0;
		p_spi_slave_dev->spi_rx_slave_buf_tail = 0;
	}
	else
	{
		memcpy(pdata,p_spi_slave_dev->spi_rx_slave_buf+p_spi_slave_dev->spi_rx_slave_buf_tail ,length);
		p_spi_slave_dev->spi_rx_slave_buf_tail += length;
	}
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
	return length;
}

void spi_slave_load(unsigned int vector)
{
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_SPI);
#endif
	//system_printf("spi_slave_load\n");
	
	unsigned int temp,sts,i;

	spi_reg * pSpiReg = (spi_reg *)spi_slave.spi_base;
	//system_printf("pSpiReg->intrSt=0x%x\n",pSpiReg->intrSt);
	if((pSpiReg->intrSt & 0x8) != 0)
	{
		//system_printf("if((pSpiReg->intrSt & 0x8)");
		pSpiReg->intrSt = 0x8;
		irq_status_clean(vector);
		spi_slave_write_spi();
#ifdef _USE_PSM
		TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
		return;
	}
	
	pSpiReg->intrSt = SPI_STS_SLAVE_CMD_INT;

	//1.clear INT GPIO
	irq_status_clean(vector);
		
	temp = (pSpiReg->cmd & 0xff);

	switch (temp)
	{
				case 0x51:
				case 0x52:
				case 0x54:					
					spi_slave_read_to_buffer();					
					break;
				case 0x0B:
				case 0x0C:
				case 0x0E:					
					break;				
				case 0x05:
				case 0x15:
				case 0x25:							
					pSpiReg->data = 0x10000;
					break;					
				default:
					break;
					
	}
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_SPI);
#endif
}




//#ifdef _USE_SPI_HOST_TEST

static int spi_slave_init_test(cmd_tbl_t *t, int argc, char *argv[])
{
	spi_slave_dev  Spi_slave_dev = 
			{			
				//MSPI
				.spi_base = SPI0,
				//SPI
				//.spi_base = SPI0,
				.inten = 0x20,
				.addr_len = 3,
				.data_len = 8,
				.master_clk = SPI_MASTER_FREQ_10M
			};
	
	spi_slave_init(&Spi_slave_dev);	
	
	return CMD_RET_SUCCESS;
}
CMD(spi_init_slave,
	spi_slave_init_test,
	"spi_slave_init",
	"spi_slave_init");

static int spi_slave_read_test(cmd_tbl_t *t, int argc, char *argv[])
{
	
	unsigned int length = strtoul(argv[1], NULL, 0);
	unsigned char test[1024]={0};
	
	int i;
	spi_slave_read((unsigned int *)test,length);
	int ok = 1;
	for(i=0;i<length;++i)
	{
		
		//system_printf("test[%d]=0x%x\n",i,test[i]);
		if(((unsigned char)(test[i]))!=(unsigned char)(i%256))
		{
			ok = 0;
			system_printf("test[%d]=%d\n",i,test[i]);
		}
		
		//system_printf("test[%d]=0x%x\n",i,test[i]);	
	}
	if(ok==1)
		system_printf("PASS\n");
	else
		system_printf("FAIL\n");
	return CMD_RET_SUCCESS;
}
CMD(spi_read_slave,
	spi_slave_read_test,
	"spi_slave_read",
	"spi_slave_read");
static int spi_slave_write_test(cmd_tbl_t *t, int argc, char *argv[])
{
	
	unsigned char sfTest1[512]={0}; 
	int i;
	for(i=0;i<512;++i)
	{
		sfTest1[i] = i%(256);
	}
	
	unsigned int length = strtoul(argv[1], NULL, 0);
	spi_slave_write((unsigned char*)sfTest1, length);
	return CMD_RET_SUCCESS;
}
CMD(spi_write_slave,
	spi_slave_write_test,
	"spi_write_slave",
	"spi_write_slave");
static int rx_buffer_test(cmd_tbl_t *t, int argc, char *argv[])
{
	
	system_printf("p_spi_slave_dev->spi_tx_slave_buf_head=%d\n",p_spi_slave_dev->spi_tx_slave_buf_head);
	system_printf("p_spi_slave_dev->spi_tx_slave_buf_tail=%d\n",p_spi_slave_dev->spi_tx_slave_buf_tail);
	system_printf("p_spi_slave_dev->spi_rx_slave_buf_head=%d\n",p_spi_slave_dev->spi_rx_slave_buf_head);
	system_printf("p_spi_slave_dev->spi_rx_slave_buf_tail=%d\n",p_spi_slave_dev->spi_rx_slave_buf_tail);
	

	return CMD_RET_SUCCESS;
}
CMD(spi_slave_buffer,
	rx_buffer_test,
	"rx_buufer",
	"rx_buufer");



//#endif


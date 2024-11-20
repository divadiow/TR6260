

#include "system_common.h"
#include "drv_uart.h"
#ifndef _USR_LMAC_TEST
#include "drv_rtc.h"
#endif
#include "irq.h"
#include "util_cli_freertos.h"
#include "soc_top_reg.h"
#include "soc_pin_mux.h"
#define UARTC_FCR_OFFSET                0x28 /* FIFO control register */
#define UARTC_RBR_OFFSET                0x20 /* receiver biffer register */
#define MAX_RECV_LEN                    (16)//(502+10) 
#ifdef MPW

#define TX_FIFO_MAX (16)
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

unsigned char uart_get_intType(unsigned int regBase)
{
	unsigned char  type = UART_INT_NONE;
	int intType = IN32(regBase + UARTC_IIR_OFFSET) & 0xF;

	if(intType == 0x4)		
		type = UART_INT_RX_DONE;
	else if(intType == 0x2)
		type = UART_INT_TX_EMPTY;
	else if(intType == 0xC)
		type = UART_INT_TIMEOUT;
	else if(intType == 0x6)
		type = UART_INT_ERROR;

	return type;
}
/*
void uart_set_baudrate(unsigned int regBase, unsigned int baud, unsigned int cond)
{
	unsigned int baudrate;
	unsigned int value = IN32(regBase + UARTC_LCR_OFFSET);

	if(cond == 0)
		baudrate = 20000000/16/baud; 
	else if(cond == 1)
		baudrate = 40000000/16/baud; 
	else
		baudrate = 80000000/16/baud; 

	OUT32(regBase + UARTC_LCR_OFFSET, 0x80|value);	
	OUT32(regBase + UARTC_DLL_OFFSET, (baudrate >> 0) & 0xff);
	OUT32(regBase + UARTC_DLM_OFFSET, (baudrate >> 8) & 0xff);
	OUT32(regBase + UARTC_LCR_OFFSET, value & 0xFFFFFF7F);
}
*/

void uart_set_lineControl(unsigned int regBase, 
			unsigned int databits,	/* 0--5bits, 1--6bits, 2--7bits, 3--8bits */
			unsigned int parity,	/* 0--no parity, 1--odd parity, 2--even parity*/
			unsigned int stopbits,	/* 0--1bit stopbits, 1--the num of stopbits is based on the databits*/
			unsigned int bc		/* break control */)
{

	unsigned int value = IN32(regBase + UARTC_LCR_OFFSET);

	value &= 0xFFFFFF80;

	value |= (databits & 3) | ((stopbits & 1) << 2) | ((bc & 1) << 6);

	if(parity == 1)
		value |= 0x8;
	else if(parity == 2)
		value |= 0x18;
	else if(parity == 3)
		value |= 0x28;
	else if(parity == 4)
		value |= 0x38;

	OUT32(regBase + UARTC_LCR_OFFSET, value);
}


void uart_set_fifoControl(unsigned int regBase, 
						unsigned int tFifoRst, 
						unsigned int rFifoRst,
						unsigned int fifoEn)
{
	unsigned int value = (tFifoRst & 1)<<2 | (rFifoRst & 1)<<1 |(tFifoRst & 1)<<0;
	OUT32(regBase + UARTC_FCR_OFFSET, value);	
}


void uart_set_intEnable(unsigned int regBase, 
						unsigned int tx, 
						unsigned int rx)
{
	unsigned int value = (tx & 1)<<1 | (rx & 1)<<0;
	OUT32(regBase + UARTC_IER_OFFSET, value);	
}

void uart_data_write(unsigned int regBase, const unsigned char * buf, unsigned int len)
{
	int i = 0;
	int fifo_count = 0;

	while(len)
	{
		while(!(IN32(regBase + UARTC_LSR_OFFSET) & UARTC_LSR_THRE));
		fifo_count = TX_FIFO_MAX;
	
		while (--fifo_count > 0) {
#if 1
                  if(buf[i] == '\n'){
                       OUT32(regBase + UARTC_THR_OFFSET, '\r');
                       if(!fifo_count)
                       break;
                  }
#endif	
		    OUT32(regBase + UARTC_THR_OFFSET, (unsigned char)buf[i++]);
		    if(--len == 0)
                      break;
		}		
	}

	while(!(IN32(regBase + UARTC_LSR_OFFSET) & UARTC_LSR_TEMT));
}

int uart_data_tstc(unsigned int regBase)
{
	return IN8(regBase + UARTC_LSR_OFFSET) & UARTC_LSR_RDR;
}

unsigned char uart_data_getc(unsigned int regBase)
{
	return IN8(regBase + UARTC_RBR_OFFSET);
}

#else

#define UART_OSCR_OFFSET        0x14
#define UARTC_DLL_OFFSET                0x20 /* baudrate divisor latch LSB */
#define UARTC_DLM_OFFSET                0x24 /* baudrate divisor latch MSB */
#define UARTC_LCR_OFFSET                0x2c /* line control regitser */


#endif

#include "drv_dma.h"

typedef struct _uart_dev_t
{
	unsigned int vector;
	unsigned int	regbase;
	void (* callback)(void * );
	void * user_data;
	char uart_rx_buf[UART_RX_BUF_SIZE];
	int uart_rx_buf_head;
	int uart_rx_buf_tail;
} uart_dev;


static unsigned char *uart_dst_buff = (unsigned char *)(0x253000);
static uart_dev * p_uart_dev_1,* p_uart_dev_2;
extern atcdmac100_dev  s_atcdmac_dev;

static int in_intr = 0;
static int total_len = 0;
/*
void dma_isr(unsigned int * int_status)
{
	atcdmac100_reg * pdmaBase = s_atcdmac_dev.pdmaBase;
    if((pdmaBase->IntStatus & 0x00040000) != 0){
		dmac_init_UART_RX();
		//system_printf("%08x %08x %08x\n",pdmaBase->ChnCtrl[0].ChnDstAddr, pdmaBase->ChnCtrl[0].ChnSrcAddr,pdmaBase->ChnCtrl[0].ChnTranSize);
		pdmaBase->IntStatus = 0x00040000;
        // pdmaBase->DMACtrl = 1;



		// total_len+=4106;
		// system_printf("recv %02x %02x %02x\n",uart_dst_buff[0],uart_dst_buff[1],uart_dst_buff[2]);
        memcpy(&p_uart_dev_1->uart_rx_buf[p_uart_dev_1->uart_rx_buf_head],&uart_dst_buff[0],MAX_RECV_LEN);
		// uart_data_getc(UART1_BASE);
		// system_printf("recv last %02x\n",uart_data_getc(UART1_BASE));
		p_uart_dev_1->uart_rx_buf_head += MAX_RECV_LEN;
		dmac_config_uart1_rx(2,MAX_RECV_LEN,(unsigned int *)(UART1_BASE + UARTC_RBR_OFFSET),(void *)(uart_dst_buff));
		if(p_uart_dev_1->callback){
				p_uart_dev_1->callback(p_uart_dev_1);
		}
		
		// while(uart_data_tstc(UART1_BASE)){
		// 	 p_uart_dev_1->uart_rx_buf[p_uart_dev_1->uart_rx_buf_head++] = uart_data_getc(UART1_BASE);
		// 	 total_len++;
		// }
		dmac_start(2);
    }

}
*/

void dma_uart_init(void){
    dmac_init_UART_RX();
	dma_init_isr();
	/*
	irq_status_clean(IRQ_VECTOR_DMA);
    irq_isr_register(IRQ_VECTOR_DMA, (void *)dma_isr);
	irq_unmask(IRQ_VECTOR_DMA);
	*/
}

void hal_uart_dma_isr()
{
	dmac_init_UART_RX();	
	
	memcpy(&p_uart_dev_1->uart_rx_buf[p_uart_dev_1->uart_rx_buf_head],&uart_dst_buff[0],MAX_RECV_LEN);
	p_uart_dev_1->uart_rx_buf_head += MAX_RECV_LEN;
	dmac_config_uart1_rx(2,MAX_RECV_LEN,(unsigned int *)(UART1_BASE + UARTC_RBR_OFFSET),(void *)(uart_dst_buff));
	if(p_uart_dev_1->callback){
			p_uart_dev_1->callback(p_uart_dev_1);
	}
			
	dmac_start(2);
}




void uart_set_baudrate(unsigned int regBase, unsigned int baud, unsigned int cond)
{
	unsigned int baudrate;
	unsigned int osc = 8;
	unsigned int value = IN32(regBase + UARTC_LCR_OFFSET);

	if(cond == 0)
		baudrate = 20000000/osc/baud; 
	else if(cond == 1)
		baudrate = 40000000/osc/baud; 
	else
		baudrate = 80000000/osc/baud; 

	OUT32(regBase + UART_OSCR_OFFSET, osc);	
	OUT32(regBase + UARTC_LCR_OFFSET, 0x80|value);	
	OUT32(regBase + UARTC_DLL_OFFSET, (baudrate >> 0) & 0xff);
	OUT32(regBase + UARTC_DLM_OFFSET, (baudrate >> 8) & 0xff);
	OUT32(regBase + UARTC_LCR_OFFSET, value & 0xFFFFFF7F);
}

#ifndef _USR_LMAC_TEST
static unsigned int uart_psm_time = 0;
static unsigned char uart_psm_enable = 1;
static unsigned char uart_psm_delay = 3;

int hal_uart_allow_psm(void)
{
	int sec, ms;
	unsigned int elapsed_ttime, curren_time = rtc_get_32K_cnt();

	if(uart_psm_time == 0 || uart_psm_enable == 0)
	{
		return 1;
	}

	if(RTC_ALARM_GET_SEC(curren_time) < RTC_ALARM_GET_SEC(uart_psm_time) )
	{
		sec = 60 - RTC_ALARM_GET_SEC(uart_psm_time) + RTC_ALARM_GET_SEC(curren_time);
		if(RTC_ALARM_GET_32K(uart_psm_time) > RTC_ALARM_GET_32K(curren_time))
		{
			sec -= 1;
			ms = ALARM_CLK - RTC_ALARM_GET_32K(uart_psm_time) + RTC_ALARM_GET_32K(curren_time);
		}
		else
			ms = RTC_ALARM_GET_32K(curren_time) - RTC_ALARM_GET_32K(uart_psm_time);
	}else{
		sec = RTC_ALARM_GET_SEC(curren_time) - RTC_ALARM_GET_SEC(uart_psm_time);
		if(RTC_ALARM_GET_32K(uart_psm_time) > RTC_ALARM_GET_32K(curren_time))
		{
			sec -= 1;
			ms = ALARM_CLK - RTC_ALARM_GET_32K(uart_psm_time) + RTC_ALARM_GET_32K(curren_time);
		}
		else
			ms = RTC_ALARM_GET_32K(curren_time) - RTC_ALARM_GET_32K(uart_psm_time);
	}
	elapsed_ttime = sec * ALARM_CLK + ms;

	if(uart_psm_delay*ALARM_CLK > elapsed_ttime)
	{
		return 0;
	}
	else
	{
		uart_psm_time = 0;
		return 1;
	}
}
#endif


static void hal_uart_isr(int vector)
{
	portENTER_CRITICAL();
	// irq_mask(vector);

	if(vector == IRQ_VECTOR_UART0)
	{
		unsigned int regbase = UART0_BASE;
		if(uart_get_intType(regbase) == UART_INT_RX_DONE)
		{
			while(uart_data_tstc(regbase)) 
				util_cli_callback(uart_data_getc(regbase));	
		}
	}
	else
	{
		

		// uart_dev * p_uart_dev;
		// if(vector == IRQ_VECTOR_UART1)
		// 	p_uart_dev = p_uart_dev_1;
		// else if(vector == IRQ_VECTOR_UART2)
		// 	p_uart_dev = p_uart_dev_2;
		// else
		// 	return;

		
		// system_printf("rx %d\n",i++);
		// in_intr++;
		while(uart_data_tstc(p_uart_dev_1->regbase)){
			 uart_data_getc(p_uart_dev_1->regbase);
			 total_len++;
		}
		// system_printf("total_len %d\n",total_len);
		// if(uart_get_intType(p_uart_dev->regbase) == UART_INT_RX_DONE)
		// {			
		// 	while(uart_data_tstc(p_uart_dev->regbase))
		// 	{
		// 		if(((p_uart_dev->uart_rx_buf_head + 1)%UART_RX_BUF_SIZE) == p_uart_dev->uart_rx_buf_tail)
		// 			break;
			
		// 		p_uart_dev->uart_rx_buf[p_uart_dev->uart_rx_buf_head] = uart_data_getc(p_uart_dev->regbase);
		// 		// system_printf("rx[%d] %x ",p_uart_dev->uart_rx_buf_head,p_uart_dev->uart_rx_buf[p_uart_dev->uart_rx_buf_head]);
		// 		p_uart_dev->uart_rx_buf_head++;
		// 		p_uart_dev->uart_rx_buf_head = p_uart_dev->uart_rx_buf_head % UART_RX_BUF_SIZE;
		// 	}

		// 	if(p_uart_dev->callback)
		// 		p_uart_dev->callback(p_uart_dev);
		// }
	}
	// irq_unmask(vector);
	// irq_unmask(IRQ_VECTOR_MAC0);
	portEXIT_CRITICAL();
#ifndef _USR_LMAC_TEST
	uart_psm_time = rtc_get_32K_cnt();
#endif
}

static void hal_uart_isr1(int vector)
{
	portENTER_CRITICAL();
	irq_mask(IRQ_VECTOR_MAC0);
	irq_mask(vector);

	// {
		

		// uart_dev * p_uart_dev;
		// if(vector == IRQ_VECTOR_UART1)
		// 	p_uart_dev = p_uart_dev_1;
		// else if(vector == IRQ_VECTOR_UART2)
		// 	p_uart_dev = p_uart_dev_2;
		// else
		// 	return;

		
		// system_printf("rx %d\n",i++);
		in_intr++;
		// system_printf("rx %d\n",uart_data_tstc(p_uart_dev_1->regbase));
		int type = uart_get_intType(p_uart_dev_1->regbase);
		//system_printf("rx type %d\n",type);
		// if(type == UART_INT_RX_DONE){
		// 	while(uart_data_tstc(p_uart_dev_1->regbase)){
		// 		uart_data_getc(p_uart_dev_1->regbase);
		// 		total_len++;
		// 	}
		// } else if(type == UART_INT_TIMEOUT){
			// system_printf("type %d\n",uart_get_intType(p_uart_dev_1->regbase));
			// while(uart_data_tstc(p_uart_dev_1->regbase)){
			// 	uart_data_getc(p_uart_dev_1->regbase);
			// 	total_len++;
			// }
		// } else {
			//system_printf("rx %d\n",type);
		// }
		
		// system_printf("total_len %d\n",total_len);
		// if(uart_get_intType(p_uart_dev->regbase) == UART_INT_RX_DONE)
		// {			
		// 	while(uart_data_tstc(p_uart_dev->regbase))
		// 	{
		// 		if(((p_uart_dev->uart_rx_buf_head + 1)%UART_RX_BUF_SIZE) == p_uart_dev->uart_rx_buf_tail)
		// 			break;
			
		// 		p_uart_dev->uart_rx_buf[p_uart_dev->uart_rx_buf_head] = uart_data_getc(p_uart_dev->regbase);
		// 		// system_printf("rx[%d] %x ",p_uart_dev->uart_rx_buf_head,p_uart_dev->uart_rx_buf[p_uart_dev->uart_rx_buf_head]);
		// 		p_uart_dev->uart_rx_buf_head++;
		// 		p_uart_dev->uart_rx_buf_head = p_uart_dev->uart_rx_buf_head % UART_RX_BUF_SIZE;
		// 	}

		// 	if(p_uart_dev->callback)
		// 		p_uart_dev->callback(p_uart_dev);
		// }
	// }
	irq_unmask(vector);
	irq_unmask(IRQ_VECTOR_MAC0);
	portEXIT_CRITICAL();
#ifndef _USR_LMAC_TEST
	uart_psm_time = rtc_get_32K_cnt();
#endif
}



int hal_uart_register_recv_callback(uart_handle_t handle, void (* callback)(void *), void *data)
{
	uart_dev * p_uart_dev = (uart_handle_t)handle;
	unsigned int flag = system_irq_save();
	p_uart_dev->callback = callback;
	p_uart_dev->user_data = data;
	system_irq_restore(flag);
	return 0;
}

int hal_uart_get_recv_len(uart_handle_t handle)
{
	unsigned int flag, len;
	uart_dev * p_uart_dev = (uart_handle_t)handle;

	flag = system_irq_save();
	if(p_uart_dev->uart_rx_buf_tail > p_uart_dev->uart_rx_buf_head)
		len = p_uart_dev->uart_rx_buf_head + UART_RX_BUF_SIZE - p_uart_dev->uart_rx_buf_tail + 1;
	else
		len = p_uart_dev->uart_rx_buf_head - p_uart_dev->uart_rx_buf_tail;
	system_irq_restore(flag);

	return len;
}

void hal_uart_write(uart_handle_t handle, const unsigned char * buf, int len)
{
	uart_dev * p_uart_dev = (uart_handle_t)handle;
	unsigned int flag = system_irq_save();
	uart_data_write(p_uart_dev->regbase, buf, len);
	system_irq_restore(flag);
}


unsigned int hal_uart_read(uart_handle_t handle, unsigned char * buf, int len)
{
	unsigned int flag, len_temp;
	uart_dev * p_uart_dev = (uart_handle_t)handle;
	
	//system_printf("head %d tail %d\n", p_uart_dev->uart_rx_buf_head, p_uart_dev->uart_rx_buf_tail);
	flag = system_irq_save();
	if(p_uart_dev->uart_rx_buf_tail > p_uart_dev->uart_rx_buf_head)
	{
		len_temp = UART_RX_BUF_SIZE - p_uart_dev->uart_rx_buf_tail;
		if(len_temp >= len)
		{
			memcpy(buf, p_uart_dev->uart_rx_buf + p_uart_dev->uart_rx_buf_tail , len);
			p_uart_dev->uart_rx_buf_tail = (p_uart_dev->uart_rx_buf_tail + len)%UART_RX_BUF_SIZE;
			len_temp = len;
		}
		else
		{
			memcpy(buf, p_uart_dev->uart_rx_buf + p_uart_dev->uart_rx_buf_tail, len_temp);
			
			if((len - len_temp) < p_uart_dev->uart_rx_buf_head)
			{
				memcpy(buf+len_temp, p_uart_dev->uart_rx_buf, len - len_temp);
				p_uart_dev->uart_rx_buf_tail = len - len_temp;
				len_temp = len;
			}
			else
			{
				memcpy(buf+len_temp, p_uart_dev->uart_rx_buf, p_uart_dev->uart_rx_buf_head);
				p_uart_dev->uart_rx_buf_tail = p_uart_dev->uart_rx_buf_head;
				len_temp += p_uart_dev->uart_rx_buf_head;
			}
		}
	}
	else
	{
		len_temp = p_uart_dev->uart_rx_buf_head - p_uart_dev->uart_rx_buf_tail;
		if(len_temp > len)
		{
			memcpy(buf, p_uart_dev->uart_rx_buf + p_uart_dev->uart_rx_buf_tail, len);
			len_temp = len;
			p_uart_dev->uart_rx_buf_tail += len;
			if(p_uart_dev->uart_rx_buf_tail == p_uart_dev->uart_rx_buf_head) {
				p_uart_dev->uart_rx_buf_head = p_uart_dev->uart_rx_buf_tail = 0;
			}
		}
		else
		{
			memcpy(buf, p_uart_dev->uart_rx_buf + p_uart_dev->uart_rx_buf_tail, len_temp);
			p_uart_dev->uart_rx_buf_head = p_uart_dev->uart_rx_buf_tail = 0;
		}
	}
	system_irq_restore(flag);

	return len_temp;
}

void uart_set_fifoControl_inter(unsigned int dma, unsigned int regBase, 
						unsigned int tFifoRst, 
						unsigned int rFifoRst,
						unsigned int fifoEn)
{
	//add dma
	unsigned int value =  ((dma & 1)<<3)| ((tFifoRst & 1)<<2) | ((rFifoRst & 1)<<1) |((fifoEn & 1)<<0);
	OUT32(regBase + UARTC_FCR_OFFSET, value);	
	// system_printf("value %08x\n",value);
}

uart_handle_t hal_uart_open(unsigned int id, 
					unsigned int databits, 
					unsigned int baud, 
					unsigned int parity, 
					unsigned int stopbits, 
					unsigned int flow)
{
	unsigned int regbase;
	unsigned int vector;
	uart_dev * p_uart_dev = NULL;

	if(id == UART_ID_0)
	{
		regbase = UART0_BASE;
		vector = IRQ_VECTOR_UART0;

		CLK_ENABLE(CLK_UART0);
#ifdef MPW
		//CFG  MPW UART0 PIN MUX
		OUT32(SOC_PIN0_MUX_BASE, (IN32(SOC_PIN0_MUX_BASE) & (~(7<<15))) |(2<<15));
		OUT32(SOC_PIN0_MUX_BASE, (IN32(SOC_PIN0_MUX_BASE) & (~(7<<18))) |(2<<18));
#endif
	}
	else
	{
		if(id == UART_ID_1)
		{
		
		// while(1);
			OUT32(SW_RESET, 0xFFFFFFFF);
			regbase = UART1_BASE;
			vector = IRQ_VECTOR_UART1;
			CLK_ENABLE(CLK_UART1);
#ifdef MPW
			//CFG  MPW UART1 PIN MUX
			OUT32(SOC_PIN0_MUX_BASE, (IN32(SOC_PIN0_MUX_BASE) & (~(7<<6))) |(2<<6));
			OUT32(SOC_PIN0_MUX_BASE, (IN32(SOC_PIN0_MUX_BASE) & (~(7<<9))) |(2<<9));
#else
			PIN_FUNC_SET(IO_MUX0_GPIO2, FUNC_GPIO2_UART1_RXD);
			PIN_FUNC_SET(IO_MUX0_GPIO3, FUNC_GPIO3_UART1_TXD);
#endif
			p_uart_dev_1 = pvPortMalloc(sizeof(uart_dev));
			if(p_uart_dev_1)
			{	
				p_uart_dev = p_uart_dev_1;
			}
			else
			{
				return (uart_handle_t)-1;
			}
		}
		else if(id == UART_ID_2)
		{
			regbase = UART2_BASE;
			vector = IRQ_VECTOR_UART1;
			CLK_ENABLE(CLK_UART2);
			p_uart_dev_2 = pvPortMalloc(sizeof(uart_dev));
			if(p_uart_dev_2)
			{	
				p_uart_dev = p_uart_dev_2;
			}
			else
			{
				return (uart_handle_t)-1;
			}
		}
		else
		{
			return (uart_handle_t)-1;
		}

		p_uart_dev->vector = vector;
		p_uart_dev->regbase = regbase;
		memset(p_uart_dev->uart_rx_buf , 0 ,UART_RX_BUF_SIZE);
		p_uart_dev->uart_rx_buf_head = 0;
		p_uart_dev->uart_rx_buf_tail = 0;
	}

#ifdef FPGA
	uart_set_baudrate(regbase, baud, 1);
#else
	uart_set_baudrate(regbase, baud, 1);
#endif
	uart_set_lineControl(regbase, databits, parity, stopbits, 0);
	uart_set_fifoControl(regbase, 1, 1, 1);

	if(vector == IRQ_VECTOR_UART1){
		uart_set_fifoControl_inter(1, regbase, 1, 1, 1);
		dma_uart_init();
		//system_printf("(uart_data_tstc(UART0&1_BASE)---------------------%d----%d\n",uart_data_tstc(UART0_BASE),uart_data_tstc(UART1_BASE));
		dmac_config_uart1_rx(2,MAX_RECV_LEN,(unsigned int *)(UART1_BASE + UARTC_RBR_OFFSET),(void *)uart_dst_buff);
		
		dmac_start(2);
	}else {
		irq_isr_register(vector, (void *)hal_uart_isr);
		irq_status_clean(vector);
		irq_unmask(vector);
		uart_set_intEnable(regbase, UART_INT_DISABLE, UART_INT_ENABLE);
	}
	
	
	return (uart_handle_t)p_uart_dev;
}

void hal_uart_close(uart_handle_t handle)
{
	uart_dev * p_uart_dev = (uart_handle_t)handle;
	irq_unmask(p_uart_dev->vector);
	irq_status_clean(p_uart_dev->vector);
	irq_isr_register(p_uart_dev->vector, NULL);
	vPortFree(p_uart_dev);
	p_uart_dev = NULL;
}

void hal_uart_init(void)
{
#ifdef UART_WPA_SEPARATION
	hal_uart_open(UART_ID_0, UART_DATA_BIT_8, UART_BAUD_RATE_57600, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
	hal_uart_open(UART_ID_1, UART_DATA_BIT_8, UART_BAUD_RATE_57600, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
	system_register_printf_port(UART1_BASE, uart_data_write);
	system_register_wpa_port(UART0_BASE);
#else
	hal_uart_open(UART_ID_0, UART_DATA_BIT_8, UART_BAUD_RATE_115200, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
	system_register_printf_port(UART0_BASE, uart_data_write);
#endif
}

#ifndef _USR_LMAC_TEST
static int cmd_uart_psm(cmd_tbl_t *t, int argc, char *argv[])
{
	unsigned int value = atoi(argv[1]);

	if(value == 0)
	{
		uart_psm_enable = 0;
		uart_psm_delay = 0;
	}
	else
	{
		uart_psm_enable = 1;
		uart_psm_delay= (value > 0xff)?0xff:(unsigned char)value;
	}
	//system_printf("cmd_uart_psm, delay: %d\n", uart_psm_delay);

	return CMD_RET_SUCCESS;
}

CMD(uartpsm, cmd_uart_psm,  "uart delya psm time",  "uartpsm <time> (time in sec)");
#endif


void hal_uart1_reset(uart_handle_t handle)
{
	uart_set_fifoControl_inter(1, UART1_BASE, 1, 1, 1);
	if (p_uart_dev_1 == NULL)
	{
		return;
	}
	memset(p_uart_dev_1->uart_rx_buf, 0, UART_RX_BUF_SIZE);
	p_uart_dev_1->uart_rx_buf_head = 0;
	p_uart_dev_1->uart_rx_buf_tail = 0;
	dma_uart_init();
	dmac_config_uart1_rx(2, MAX_RECV_LEN, (unsigned int *)(UART1_BASE + UARTC_RBR_OFFSET), (void *)uart_dst_buff);
	dmac_start(2);
}


#if 0

uart_handle_t uart_handle;


void uart_callback(void *handle)
{
	system_printf("uart_callback, rece-len: %d\n", hal_uart_get_recv_len((uart_handle_t)uart_handle));
}


static int uart_open(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("uart_open\n");
	PIN_FUNC_SET(IO_MUX0_GPIO2,FUNC_GPIO2_UART1_RXD);
	PIN_FUNC_SET(IO_MUX0_GPIO3,FUNC_GPIO3_UART1_TXD);
	uart_handle = hal_uart_open(UART_ID_1, UART_DATA_BIT_8, UART_BAUD_RATE_115200, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
	hal_uart_register_recv_callback(uart_handle, uart_callback, (void *)uart_handle);
	return CMD_RET_SUCCESS;
}
extern int get_uart_have_data_len();
/*
void hal_uart1_reset(uart_handle_t handle)
{
	uart_set_fifoControl_inter(1,UART1_BASE, 1, 1, 1);
	if(p_uart_dev_1 == NULL){
		return;
	}
	memset(p_uart_dev_1->uart_rx_buf , 0 ,UART_RX_BUF_SIZE);
	p_uart_dev_1->uart_rx_buf_head = 0;
	p_uart_dev_1->uart_rx_buf_tail = 0;
	dmac_reset();
}
*/

static int uart_loop(cmd_tbl_t *t, int argc, char *argv[])
{
	int length;
		// while(uart_data_tstc(p_uart_dev_1->regbase)){
		// 	 uart_data_getc(p_uart_dev_1->regbase);
		// 	 total_len++;
		// }
		
	system_printf("int num %d uart_total %d remain %d\n",in_intr,total_len, get_uart_have_data_len());
// total_len = 0;
	// int i = 0;
	// // hal_uart_write(uart_handle,(const unsigned char *)"123456test\n",11);
	// // while(1){
		system_printf("%02x %02x %02x %02x %02x\n",p_uart_dev_1->uart_rx_buf[0],p_uart_dev_1->uart_rx_buf[4100],p_uart_dev_1->uart_rx_buf[4103],p_uart_dev_1->uart_rx_buf[4104],p_uart_dev_1->uart_rx_buf[4105]);
		// length = hal_uart_read(uart_handle, buff, 4106);
		// system_printf("length:%d  ",length);
		// if(length == 128){
		// 	continue;
		// }
	// 	for(i=0;i<length;i++)
	// 		system_printf("[%d]:%d\n",i,buff[i]);
		
	// // }
	
	return CMD_RET_SUCCESS;
}

static int share_value = 0;
static void test1_task(void *pvParameters){
	int i = 0;
portENTER_CRITICAL();
	for(i = 0;i< 70000000;i++){
		int j =1000000;
		while(j--);
		share_value = i;
	}
	system_printf("test1_task share_value %d\n",share_value);
portEXIT_CRITICAL();

vTaskDelete(NULL);
}

static void test2_task(void *pvParameters){
portENTER_CRITICAL();
system_printf("test2_task share_value %d\n",share_value);
while(share_value){
share_value--;
}
		
portEXIT_CRITICAL();
vTaskDelete(NULL);

}

static void test3_task(void *pvParameters){
portENTER_CRITICAL();
		while(share_value){
share_value-=5;
}
		system_printf("test3_task share_value %d\n",share_value);
portEXIT_CRITICAL();
vTaskDelete(NULL);

}
static int for_test(cmd_tbl_t *t, int argc, char *argv[]){
	xTaskCreate(test1_task, "main_task", 1024, NULL, 3, NULL);
	xTaskCreate(test2_task, "main_task", 1024, NULL, 3, NULL);
	xTaskCreate(test3_task, "main_task", 1024, NULL, 3, NULL);
	return CMD_RET_SUCCESS;
}



CMD(uartc,
    uart_open,
    "uart create test",
    "uart create test");

CMD(fortest,
    for_test,
    "for test",
    "for test");

CMD(uartlp,
    uart_loop,
    "uart create test",
    "uart create test");


#endif


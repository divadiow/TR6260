

#include "system_common.h"
#include "system.h"
#include "drv_uart.h"
#if ((!defined _USR_LMAC_TEST) && (!defined _USR_MIN_SYS) && (!defined SINGLE_BOARD_VER))
#include "drv_rtc.h"
#endif
#include "irq.h"
#include "util_cli_freertos.h"
#include "soc_top_reg.h"
#include "soc_pin_mux.h"
#include "drv_timer.h"
#include "drv_dma.h"


#ifdef CONFIG_AT_COMMAND
#include "at_def.h"
#include "at_customer_wrapper.h"
#endif

uart_config uart_dev_config[3]={{0}}; //uart0/1/2

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

static int at_get_stopbit(int databits,int stopbits)
{
	if(stopbits == 0){
		return 1;
	} else{
		if(databits == UART_DATA_BIT_5) 
           return 2;
        else 
           return 3;
	}
	return 1;
}

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

	REG_WRITE(regBase + UARTC_LCR_OFFSET, value);

	if(regBase == UART0_BASE){
		uart_dev_config[0].databits = databits+5;
		uart_dev_config[0].parity   = parity;
		uart_dev_config[0].stopbits = at_get_stopbit(databits,stopbits);
	} else if(regBase == UART1_BASE){
		uart_dev_config[1].databits = databits+5;
		uart_dev_config[1].parity   = parity;
		uart_dev_config[1].stopbits = at_get_stopbit(databits,stopbits);
	} else if(regBase == UART2_BASE){
		uart_dev_config[2].databits = databits+5;
		uart_dev_config[2].parity   = parity;
		uart_dev_config[2].stopbits = at_get_stopbit(databits,stopbits);
	}

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

void at_uart_data_write(unsigned int regBase, const unsigned char * buf, unsigned int len)
{
	int i = 0;
	int fifo_count = 0;

	while(len)
	{
		while(!(IN32(regBase + UARTC_LSR_OFFSET) & UARTC_LSR_THRE));
		fifo_count = TX_FIFO_MAX;

		while (--fifo_count > 0) {
		    OUT32(regBase + UARTC_THR_OFFSET, (unsigned char)buf[i++]);
		    if(--len == 0)
                break;
		}
	}
	while(!(IN32(regBase + UARTC_LSR_OFFSET) & UARTC_LSR_TEMT));
}

void uart_put_char(unsigned int regBase, const char buf)
{
    OUT32(regBase + UARTC_THR_OFFSET,  buf);
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

#define TX_FIFO_MAX (16)

#define UART_OSCR_OFFSET        0x14
#define UARTC_DLL_OFFSET                0x20 /* baudrate divisor latch LSB */
#define UARTC_DLM_OFFSET                0x24 /* baudrate divisor latch MSB */
#define UARTC_LCR_OFFSET                0x2c /* line control regitser */

#define UARTC_THR_OFFSET                0x20 /* transmitter holding register */
#define UARTC_LSR_OFFSET                0x34 /* line status register */
#define UARTC_LSR_TEMT                  0x40 /* THR/TSR Empty */
#define UARTC_LSR_THRE                  0x20 /* THR/FIFO Empty */

static int at_get_stopbit(int databits,int stopbits)
{
	if(stopbits == 0){
		return 1;
	} else{
		if(databits == UART_DATA_BIT_5) 
           return 2;
        else 
           return 3;
	}
	return 1;
}

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

	if(regBase == UART0_BASE){
		uart_dev_config[0].databits = databits+5;
		uart_dev_config[0].parity   = parity;
		uart_dev_config[0].stopbits = at_get_stopbit(databits,stopbits);
	} else if(regBase == UART1_BASE){
		uart_dev_config[1].databits = databits+5;
		uart_dev_config[1].parity   = parity;
		uart_dev_config[1].stopbits = at_get_stopbit(databits,stopbits);
	} else if(regBase == UART2_BASE){
		uart_dev_config[2].databits = databits+5;
		uart_dev_config[2].parity   = parity;
		uart_dev_config[2].stopbits = at_get_stopbit(databits,stopbits);
	}

}
void at_uart_data_write(unsigned int regBase, const unsigned char * buf, unsigned int len)
{
	int i = 0;
	int fifo_count = 0;

	while(len)
	{
		while(!(IN32(regBase + UARTC_LSR_OFFSET) & UARTC_LSR_THRE));
		fifo_count = TX_FIFO_MAX;

		while (--fifo_count > 0) {
		    OUT32(regBase + UARTC_THR_OFFSET, (unsigned char)buf[i++]);
		    if(--len == 0)
                break;
		}
	}
	while(!(IN32(regBase + UARTC_LSR_OFFSET) & UARTC_LSR_TEMT));
}

#endif



typedef struct _uart_dev_t
{
	unsigned int vector;
	unsigned int	regbase;
	void (* callback)(void * );
	void * user_data;
	char uart_rx_buf[UART_RX_BUF_SIZE];
	unsigned int uart_rx_buf_head;
	unsigned int uart_rx_buf_tail;
} uart_dev;

static uart_dev * p_uart_dev_1, * p_uart_dev_2;

int uart_get_config(unsigned int regBase, uart_config * config)
{
	if(regBase == UART0_BASE){
		memcpy(config, &uart_dev_config[0],sizeof(uart_config));
	} else if(regBase == UART1_BASE){
		memcpy(config, &uart_dev_config[1],sizeof(uart_config));
	} else if(regBase == UART2_BASE){
		memcpy(config, &uart_dev_config[2],sizeof(uart_config));
	} else {
		return -1;
	}
	return 0;
}

void uart_set_baudrate(unsigned int regBase, unsigned int baud, unsigned int cond)
{
	unsigned int baudrate;
	unsigned int osc = 8;
	unsigned int value = IN32(regBase + UARTC_LCR_OFFSET);

	if(regBase == UART0_BASE){
		uart_dev_config[0].buad_rate = baud;
	} else if(regBase == UART1_BASE){
		uart_dev_config[1].buad_rate = baud;
	} else if(regBase == UART2_BASE){
		uart_dev_config[2].buad_rate = baud;
	}

	if(cond == 0)
		baudrate = 20000000/osc/baud; 
	else if(cond == 1)
		baudrate = 40000000/osc/baud; 
	else
		baudrate = 80000000/osc/baud; 
	if(cond==1)
	{
		if(baud == 460800)
			baudrate = 11;
	}
	else if(cond==2)
	{
		if(baud == 921600)
			baudrate = 11;
	}
	#ifdef AMT
	if(baud == 921600)
	{
		baudrate = 11;
	}
	else if(baud == 2000000)
	{
		baudrate = 5;
	}
	else if(baud == 2500000)
		{
		baudrate = 4;
		}
	#endif

	OUT32(regBase + UART_OSCR_OFFSET, osc);	
	OUT32(regBase + UARTC_LCR_OFFSET, 0x80|value);	
	OUT32(regBase + UARTC_DLL_OFFSET, (baudrate >> 0) & 0xff);
	OUT32(regBase + UARTC_DLM_OFFSET, (baudrate >> 8) & 0xff);
	OUT32(regBase + UARTC_LCR_OFFSET, value & 0xFFFFFF7F);
}

#if ((!defined _USR_LMAC_TEST) && (!defined _USR_MIN_SYS) && (!defined SINGLE_BOARD_VER))
static unsigned int uart_psm_time = 0;
static unsigned char uart_psm_enable = 1;
static unsigned char uart_psm_delay = 3;
//unsigned int isr_count = 0;
int hal_uart_allow_psm(void)
{
//	int sec, ms;
	unsigned int elapsed_ttime, curren_time = rtc_get_32K_cnt();

	if(uart_psm_time == 0 || uart_psm_enable == 0)
	{
		return 1;
	}

	if(RTC_ALARM_GET_SEC(curren_time) < RTC_ALARM_GET_SEC(uart_psm_time) )
	{
		elapsed_ttime = 60 - RTC_ALARM_GET_SEC(uart_psm_time) + RTC_ALARM_GET_SEC(curren_time);
#if 0
		sec = 60 - RTC_ALARM_GET_SEC(uart_psm_time) + RTC_ALARM_GET_SEC(curren_time);
		if(RTC_ALARM_GET_32K(uart_psm_time) > RTC_ALARM_GET_32K(curren_time))
		{
			sec -= 1;
			ms = ALARM_CLK - RTC_ALARM_GET_32K(uart_psm_time) + RTC_ALARM_GET_32K(curren_time);
		}
		else
			ms = RTC_ALARM_GET_32K(curren_time) - RTC_ALARM_GET_32K(uart_psm_time);
#endif
	}else{
		elapsed_ttime = RTC_ALARM_GET_SEC(curren_time) - RTC_ALARM_GET_SEC(uart_psm_time);
#if 0
		sec = RTC_ALARM_GET_SEC(curren_time) - RTC_ALARM_GET_SEC(uart_psm_time);
		if(RTC_ALARM_GET_32K(uart_psm_time) > RTC_ALARM_GET_32K(curren_time))
		{
			sec -= 1;
			ms = ALARM_CLK - RTC_ALARM_GET_32K(uart_psm_time) + RTC_ALARM_GET_32K(curren_time);
		}
		else
			ms = RTC_ALARM_GET_32K(curren_time) - RTC_ALARM_GET_32K(uart_psm_time);
#endif
	}
	//elapsed_ttime = sec * ALARM_CLK + ms;

	if(uart_psm_delay > elapsed_ttime)
	{
		return 0;
	}
	else
	{
//		system_printf("[wangc]psm_time: %#x, cur_time: %#x, elapsed_time: %#x, isr_conut: %d\n", uart_psm_time,  curren_time, elapsed_ttime, isr_count);
		uart_psm_time = 0;
		return 1;
	}
}
#endif

#ifdef AMT
int global_amt_uboot_flag = 0;
extern void amt_util_cli_callback(char c);
extern int global_amt_uboot_flag;
#endif


static void hal_uart_isr(int vector)
{
	irq_status_clean(vector);
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_UART1);
	TrPsmSetDeviceActive(PSM_DEVICE_UART2);
#endif
	unsigned int regbase;

	if(vector == IRQ_VECTOR_UART0)
	{
		regbase = UART0_BASE;
	}
	else if(vector == IRQ_VECTOR_UART1)
	{
		regbase = UART1_BASE;
	}
	#ifdef _USR_TR6260S1
	else if(vector == IRQ_VECTOR_UART2)
	{
		regbase = UART2_BASE;
	}
	#endif
		if(uart_get_intType(regbase) == UART_INT_RX_DONE)
		{
			while(uart_data_tstc(regbase)) 
				#ifndef AMT
				util_cli_callback(uart_data_getc(regbase));	
				#else
				if(global_amt_uboot_flag)
					amt_util_cli_callback(uart_data_getc(regbase));
				else
					util_cli_callback(uart_data_getc(regbase));	
				#endif
		}

#if ((!defined _USR_LMAC_TEST) && (!defined _USR_MIN_SYS) && (!defined SINGLE_BOARD_VER))
		uart_psm_time = rtc_get_32K_cnt();
//	isr_count ++;
#endif
/*
	}
	else
	{
		uart_dev * p_uart_dev;
		if(vector == IRQ_VECTOR_UART1)
			p_uart_dev = p_uart_dev_1;
		else if(vector == IRQ_VECTOR_UART2)
			p_uart_dev = p_uart_dev_2;
		else {
#ifdef _USE_PSM
			TrPsmSetDeviceIdle(PSM_DEVICE_UART1);
			TrPsmSetDeviceIdle(PSM_DEVICE_UART2);
#endif
			return;
		}

		if(uart_get_intType(p_uart_dev->regbase) == UART_INT_RX_DONE)
		{			
			while(uart_data_tstc(p_uart_dev->regbase))
			{
				if(((p_uart_dev->uart_rx_buf_head + 1)%UART_RX_BUF_SIZE) == p_uart_dev->uart_rx_buf_tail){
					 if(p_uart_dev->callback)
						p_uart_dev->callback(p_uart_dev->user_data);
					break;
				}
			
				p_uart_dev->uart_rx_buf[p_uart_dev->uart_rx_buf_head++ ] = uart_data_getc(p_uart_dev->regbase);
				p_uart_dev->uart_rx_buf_head = p_uart_dev->uart_rx_buf_head % UART_RX_BUF_SIZE;
			}

			if(p_uart_dev->callback)
				p_uart_dev->callback(p_uart_dev->user_data);
		}
	}
*/
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_UART1);
	TrPsmSetDeviceIdle(PSM_DEVICE_UART2);
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
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_UART1);
	TrPsmSetDeviceActive(PSM_DEVICE_UART2);
#endif
	unsigned int flag, len;
	uart_dev * p_uart_dev = (uart_handle_t)handle;

	flag = system_irq_save();
	if(p_uart_dev->uart_rx_buf_tail > p_uart_dev->uart_rx_buf_head)
		len = p_uart_dev->uart_rx_buf_head + UART_RX_BUF_SIZE - p_uart_dev->uart_rx_buf_tail;
	else
		len = p_uart_dev->uart_rx_buf_head - p_uart_dev->uart_rx_buf_tail;
	system_irq_restore(flag);
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_UART1);
	TrPsmSetDeviceIdle(PSM_DEVICE_UART2);
#endif
	return len;
}

void hal_uart_write(uart_handle_t handle, const unsigned char * buf, int len)
{
	if (!handle) {
		return;
	}
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_UART1);
	TrPsmSetDeviceActive(PSM_DEVICE_UART2);
#endif
	uart_dev * p_uart_dev = (uart_handle_t)handle;
	unsigned int flag = system_irq_save();
	uart_data_write(p_uart_dev->regbase, buf, len);
	system_irq_restore(flag);
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_UART1);
	TrPsmSetDeviceIdle(PSM_DEVICE_UART2);
#endif
}


unsigned int hal_uart_read(uart_handle_t handle, unsigned char * buf, int len)
{
	if (!handle) {
		return 0;
	}
	unsigned int flag, len_temp;
	uart_dev * p_uart_dev = (uart_handle_t)handle;
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_UART1);
	TrPsmSetDeviceIdle(PSM_DEVICE_UART2);	
#endif	
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
				memcpy(buf, p_uart_dev->uart_rx_buf, len - len_temp);
				p_uart_dev->uart_rx_buf_tail = len - len_temp;
				len_temp = len;
			}
			else
			{
				memcpy(buf, p_uart_dev->uart_rx_buf, p_uart_dev->uart_rx_buf_head);
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
		}
		else
		{
			memcpy(buf, p_uart_dev->uart_rx_buf + p_uart_dev->uart_rx_buf_tail, len_temp);
			p_uart_dev->uart_rx_buf_tail = p_uart_dev->uart_rx_buf_head;
		}
	}
	system_irq_restore(flag);
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_UART1);
	TrPsmSetDeviceIdle(PSM_DEVICE_UART2);
#endif
	return len_temp;
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
		
		//CFG  MPW UART0 PIN MUX
		OUT32(SOC_PIN0_MUX_BASE, (IN32(SOC_PIN0_MUX_BASE) & (~(7<<15))) |(0<<15));
		OUT32(SOC_PIN0_MUX_BASE, (IN32(SOC_PIN0_MUX_BASE) & (~(7<<18))) |(0<<18));
	}
	else
	{
		if(id == UART_ID_1)
		{
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
		#ifdef _USR_TR6260S1
		else if(id == UART_ID_2)
		{
			regbase = UART2_BASE;
			vector = IRQ_VECTOR_UART2;
			CLK_ENABLE(CLK_UART2);
			PIN_FUNC_SET(IO_MUX0_GPIO20, FUNC_GPIO20_UART2_RXD);
			PIN_FUNC_SET(IO_MUX0_GPIO21, FUNC_GPIO21_UART2_TXD);
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
		#endif
		else
		{
			return (uart_handle_t)-1;
		}

		p_uart_dev->vector = vector;
		p_uart_dev->regbase = regbase;
		p_uart_dev->uart_rx_buf_head = 0;
		p_uart_dev->uart_rx_buf_tail = 0;
	}

#ifdef FPGA
	uart_set_baudrate(regbase, baud, 1);
#else
	#ifdef AMT
	uart_set_baudrate(regbase, baud, 1); //80MHz Core clock,160Mhz need 2
	#else
	uart_set_baudrate(regbase, baud, 1); //80MHz Core clock
	//uart_set_baudrate(regbase, baud, 1); //160MHz Core clock
	#endif
#endif
	uart_set_lineControl(regbase, databits, parity, stopbits, 0);
	uart_set_fifoControl(regbase, 1, 1, 1);

	irq_isr_register(vector, (void *)hal_uart_isr);
	irq_status_clean(vector);
	irq_unmask(vector);
	
	uart_set_intEnable(regbase, UART_INT_DISABLE, UART_INT_ENABLE);
	return (uart_handle_t)p_uart_dev;
}

void hal_uart_close(uart_handle_t handle)
{
	if (!handle) {
		return;
	}

	uart_dev * p_uart_dev = (uart_handle_t)handle;
	irq_mask(p_uart_dev->vector);
	irq_status_clean(p_uart_dev->vector);
	irq_isr_register(p_uart_dev->vector, NULL);
	vPortFree(p_uart_dev);
	p_uart_dev = NULL;
}

void hal_uart_dma_isr()
{
}


void hal_uart_init(void)
{
#ifdef UART_WPA_SEPARATION
	#ifdef AMT
	hal_uart_open(UART_ID_0, UART_DATA_BIT_8, 921600, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
	#else
	hal_uart_open(UART_ID_0, UART_DATA_BIT_8, UART_BAUD_RATE_115200, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
	#endif
	hal_uart_open(UART_ID_1, UART_DATA_BIT_8, UART_BAUD_RATE_57600, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
	system_register_printf_port(UART1_BASE, uart_data_write);
	system_register_wpa_port(UART0_BASE);
#else
	#ifdef AMT
	hal_uart_open(UART_ID_0, UART_DATA_BIT_8, 115200, UART_PARITY_NONE, UART_STOP_BIT_1, 0);//115200--ly-20191204
	//hal_uart_open(UART_ID_0, UART_DATA_BIT_8, 2000000, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
	#else
	hal_uart_open(UART_ID_0, UART_DATA_BIT_8, UART_BAUD_RATE_115200, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
	#endif
	system_register_printf_port(UART0_BASE, uart_data_write);
#endif
}
#ifndef SINGLE_BOARD_VER


uart_dev * p_uart_dev_data;
uint8_t dma_enable = 0;
#define UARTC_THR_OFFSET                0x20 /* transmitter holding register */

SemaphoreHandle_t xCountingSemaphore_uart;
#define SOCKET_SEND_MAX_LEN 3000
#define UARTC_RBR_OFFSET                0x20 /* receiver biffer register */
#define UARTC_FCR_OFFSET                0x28 /* FIFO control register */

#define uart_rx_buff_len 3072
#define uart_tx_buff_len 3072

uint8_t uart_rx_buf[uart_rx_buff_len];
uint8_t uart_tx_buf[uart_tx_buff_len];

unsigned char *uart_rx_buff_dma = (unsigned char *)(0x252000);
unsigned char *uart_tx_buff_dma = (unsigned char *)(0x253000);
uint8_t uart_rx_dma_ch = 1;
uint8_t uart_tx_dma_ch = 2;
uint32_t uart_rx_trans_len = 1024;

#ifdef HISENSE_LOCK
uint32_t timeout_rx = 5; //band:9600 ref modbus rtu Frame interval 3.5ms, 115200 暂定 5ms
#else
//ms
uint32_t timeout_rx = 150;
#endif

uint32_t uart_rx_buffer_head = 1;
uint32_t uart_rx_buffer_tail = 0;
uint32_t uart_tx_buffer_head = 1;
uint32_t uart_tx_buffer_tail = 0;

uint32_t socket_send_len;
uint32_t uart_tx_len;



void hal_uart_start_timer()
{
	hal_timer_config(timeout_rx * 1000, 0);
	hal_timer_start();

	uart_set_intEnable(p_uart_dev_data->regbase, UART_INT_DISABLE, UART_INT_ENABLE);	
}

void hal_uart_rx_buff_reset()
{	
	#ifndef HISENSE
	uart_set_intEnable(p_uart_dev_data->regbase, UART_INT_DISABLE, UART_INT_DISABLE);
	#endif
	uart_rx_buffer_head = 1;
	uart_rx_buffer_tail = 0;	
}

void hal_uart_tx_buff_reset()
{
	uart_tx_buffer_head = 1;
	uart_tx_buffer_tail = 0;
}
int hal_uart_is_tx_buff_empty()
{
	return (uart_tx_buffer_head-uart_tx_buffer_tail)==1?1:0;
}
int hal_uart_is_rx_buff_empty()
{
	return (uart_rx_buffer_head-uart_rx_buffer_tail)==1?1:0;
}



void hal_uart_rx_chan_stop()
{
	hal_timer_stop();
	OUT32(DMAC_BASE + DMAC_ABORT_REG, 1 << uart_rx_dma_ch);	
}
void hal_uart_tx_chan_stop()
{
	OUT32(DMAC_BASE + DMAC_ABORT_REG, 1 << uart_tx_dma_ch);
}




void hal_uart_dma_rx_config(uart_handle_t handle)
{
	uart_dev * p_uart_dev = (uart_handle_t)handle;
	uint32_t len = hal_uart_get_rxbuff_avail_len();
	if (len > 0)
	{
		if (p_uart_dev->vector == IRQ_VECTOR_UART0)
		{
			dmac_init_UART0_RX();
		}
		else if(p_uart_dev->vector == IRQ_VECTOR_UART1)
		{
			dmac_init_UART1_RX();
		}		
		dmac_config_uart_rx(uart_rx_dma_ch, len ,(unsigned int *)(p_uart_dev->regbase + UARTC_RBR_OFFSET),(void *)(uart_rx_buff_dma + uart_rx_buffer_head));
		dmac_start(uart_rx_dma_ch);			
	}	
	hal_timer_config(timeout_rx * 1000, 0);
	hal_timer_start();
}

int hal_uart_is_rx_dma_enable()
{
	return (IN32(DMAC_CHN_CTRL(uart_rx_dma_ch)) & 0x1);
}
int hal_uart_is_tx_dma_enable()
{
	return (IN32(DMAC_CHN_CTRL(uart_tx_dma_ch)) & 0x1);
}

void dma_uart_init(void){
	dma_init_isr();
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

void hal_uart_set_timeout_rx(unsigned           int time)
{
	timeout_rx = time;
}
int hal_uart_get_timeout_rx()
{
	return timeout_rx;
}

void hal_uart_set_trans_len_rx(unsigned            int len)
{
	uart_rx_trans_len = len;
}

void * hal_uart_get_rx_buff()
{
	return uart_rx_buff_dma + uart_rx_buffer_tail + 1;
}

void * hal_uart_get_tx_buff()
{
	return uart_tx_buff_dma + uart_tx_buffer_head;
}


unsigned int hal_uart_get_rx_len()
{
	portENTER_CRITICAL();
	uint32_t socket_lenth;
	if(uart_rx_buffer_head > uart_rx_buffer_tail)
	{
		socket_lenth = uart_rx_buffer_head - uart_rx_buffer_tail - 1; 		 
	}
	else
	{
		socket_lenth = uart_rx_buff_len - uart_rx_buffer_tail - 1;			  
	}
	socket_send_len = min(socket_lenth,SOCKET_SEND_MAX_LEN);
	portEXIT_CRITICAL();
	return socket_send_len;
}
void hal_uart_get_rx_data_end()
{
	#if ((defined HISENSE) || (defined HISENSE_LOCK))
	uart_rx_buffer_tail = 0;
	uart_rx_buffer_head = 1;
	#else
	uart_rx_buffer_tail += socket_send_len;
	if (uart_rx_buffer_tail == (uart_rx_buff_len - 1))
		uart_rx_buffer_tail = 0;
	#endif
}
unsigned int hal_uart_get_rxbuff_avail_len()
{
	uint32_t avil_lenth;
	if(uart_rx_buffer_head > uart_rx_buffer_tail)
	{
		avil_lenth = uart_rx_buff_len- uart_rx_buffer_head; 		 
	}
	else
	{
		avil_lenth = uart_rx_buffer_tail - uart_rx_buffer_head;			  
	}
	return min(avil_lenth,uart_rx_trans_len);
}

unsigned int hal_uart_get_txbuff_avail_len()
{
	uint32_t avil_lenth;
	if ((uart_tx_buffer_head == uart_tx_buff_len) && (uart_tx_buffer_tail != 0))
	{
		uart_tx_buffer_head = 1;
	}
	if(uart_tx_buffer_head > uart_tx_buffer_tail)
	{
		avil_lenth = uart_tx_buff_len- uart_tx_buffer_head; 		 
	}
	else
	{
		avil_lenth = uart_tx_buffer_tail - uart_tx_buffer_head;			  
	}
	return min(avil_lenth,uart_rx_trans_len);
}

void hal_uart_dma_tx_config()
{	
	if (uart_tx_buffer_head > uart_tx_buffer_tail)
	{
		uart_tx_len = uart_tx_buffer_head - uart_tx_buffer_tail - 1;
	}
	else
	{
		uart_tx_len = uart_tx_buff_len - uart_tx_buffer_tail - 1;
	}
	
	if (uart_tx_len > 0)
	{		
		if (p_uart_dev_data->vector == IRQ_VECTOR_UART0)
		{
			dmac_init_UART0_TX();
		}
		else if(p_uart_dev_data->vector == IRQ_VECTOR_UART1)
		{
			dmac_init_UART1_TX();
		}
		dmac_config_uart_tx(uart_tx_dma_ch,uart_tx_len,(void *)(uart_tx_buff_dma + uart_tx_buffer_tail+1),(unsigned int *)(p_uart_dev_data->regbase + UARTC_THR_OFFSET));	
		dmac_start(uart_tx_dma_ch); 
	}	
}

void hal_uart_socket_rec_end(unsigned int len)
{
	uart_tx_buffer_head += len;
	if ((uart_tx_buffer_head == uart_tx_buff_len) && (uart_tx_buffer_tail != 0))
	{
		uart_tx_buffer_head = 1;
	}
	if (dma_enable)
	{
		if(hal_uart_is_tx_dma_enable() == 0)
		{
			hal_uart_dma_tx_config();
		}
	}
	else
	{
		if (uart_tx_buffer_head > uart_tx_buffer_tail)
		{
			uart_tx_len = uart_tx_buffer_head - uart_tx_buffer_tail - 1;
		}
		else
		{
			uart_tx_len = uart_tx_buff_len - uart_tx_buffer_tail - 1;
		}
		
		hal_uart_write(p_uart_dev_data, uart_tx_buff_dma + uart_tx_buffer_tail + 1 , uart_tx_len);
		uart_tx_buffer_tail += uart_tx_len;
		if (uart_tx_buffer_tail == (uart_tx_buff_len-1))
		{
			uart_tx_buffer_tail = 0;
		}
	}

	
	
	
}

void hal_uart_rec_end(unsigned int len)
{
	uart_tx_buffer_head += len;
	if ((uart_tx_buffer_head == uart_tx_buff_len) && (uart_tx_buffer_tail != 0))
	{
		uart_tx_buffer_head = 1;
	}		
}

void hal_uart_write_data(uart_handle_t handle)
{	
	if (uart_tx_buffer_head > uart_tx_buffer_tail)
	{
		uart_tx_len = uart_tx_buffer_head - uart_tx_buffer_tail - 1;
	}
	else
	{
		uart_tx_len = uart_tx_buff_len - uart_tx_buffer_tail - 1;
	}
	hal_uart_write(handle, uart_tx_buff_dma + uart_tx_buffer_tail + 1 , uart_tx_len);
	uart_tx_buffer_tail += uart_tx_len;
	if (uart_tx_buffer_tail == (uart_tx_buff_len-1))
	{
		uart_tx_buffer_tail = 0;
	}	
}

void hal_uart_rx_dma_isr()
{
	uint32_t dma_len;
	
	hal_timer_stop();
	
	uart_rx_buffer_head = (IN32(DMAC_CHN_DST(uart_rx_dma_ch)) - (unsigned int)uart_rx_buff_dma);
	if(uart_rx_buffer_head == uart_rx_buff_len)
		uart_rx_buffer_head = 1;	
	
	hal_uart_dma_rx_config(p_uart_dev_data);
	
	xSemaphoreGive(xCountingSemaphore_uart);
}

void hal_uart_tx_dma_isr()
{
	uart_tx_buffer_tail += uart_tx_len;
	if (uart_tx_buffer_tail == (uart_tx_buff_len - 1))
	{
		uart_tx_buffer_tail = 0;
	}
	hal_uart_dma_tx_config();
}

static void hal_uart_isr_wifi(int vector)
{
	irq_status_clean(vector);
#ifdef _USE_PSM
	TrPsmSetDeviceActive(PSM_DEVICE_UART1);
	TrPsmSetDeviceActive(PSM_DEVICE_UART2);
#endif
	unsigned int regbase;

	if(vector == IRQ_VECTOR_UART0)
	{
		regbase = UART0_BASE;
	}
	else if(vector == IRQ_VECTOR_UART1)
	{
		regbase = UART1_BASE;
	}
	#ifdef _USR_TR6260S1
	else if(vector == IRQ_VECTOR_UART2)
	{
		regbase = UART2_BASE;
	}
	#endif
	if(uart_get_intType(regbase) == UART_INT_RX_DONE || uart_get_intType(regbase) == UART_INT_TIMEOUT)
	{
		while(uart_data_tstc(regbase)) 
		{
			uart_rx_buff_dma[uart_rx_buffer_head++] = uart_data_getc(regbase);
			if (uart_rx_buffer_head == uart_rx_buffer_tail)
			{
				system_printf("buffer full\n");
			}
			if (uart_rx_buffer_head == uart_rx_buff_len)
				uart_rx_buffer_head = 1;
			if (uart_rx_buffer_head == uart_rx_buffer_tail)
			{
				system_printf("buffer full\n");
			}
		}
#ifdef HISENSE_LOCK    // uart recv data，start timer
		hal_timer_stop();
        hal_timer_config(timeout_rx * 1000, 0);
		hal_timer_start();		
#endif
#ifdef TUYA_SDK_ADPT
        if(p_uart_dev_data->callback)
            p_uart_dev_data->callback(p_uart_dev_data->user_data);
#endif
#ifdef HISENSE
		if (uart_rx_buffer_head>2 && (uart_rx_buff_dma[uart_rx_buffer_head - 2] == 0xF4) && (uart_rx_buff_dma[uart_rx_buffer_head - 1] == 0xFB))
		{
			if(p_uart_dev_data->callback)
				p_uart_dev_data->callback(p_uart_dev_data->user_data);
		}
		#endif
	}
#ifdef _USE_PSM
	TrPsmSetDeviceIdle(PSM_DEVICE_UART1);
	TrPsmSetDeviceIdle(PSM_DEVICE_UART2);
#endif
}

void timer_callback_wifi(void *data)
{	
	if (dma_enable == 1)
	{
		if (hal_uart_is_rx_dma_enable() == 0)
		{
			hal_uart_dma_rx_config(p_uart_dev_data);
		}
		else
		{
			uart_rx_buffer_head = (IN32(DMAC_CHN_DST(uart_rx_dma_ch)) - (unsigned int)uart_rx_buff_dma);
			if(uart_rx_buffer_head == uart_rx_buff_len)
				uart_rx_buffer_head = 1;
				
			xSemaphoreGive(xCountingSemaphore_uart);
			hal_timer_start();	
		}	
	}
	else
	{		
#ifdef HISENSE_LOCK
        if (uart_rx_buffer_head >= 9)     //uart_rx_buffer_head start from 1 
        {
                if(p_uart_dev_data->callback){    //all data of one cmd have received
				    p_uart_dev_data->callback(p_uart_dev_data->user_data);
                }
		}
		else{
			uart_rx_buffer_head = 1;
			uart_rx_buffer_tail = 0;
		}
#endif
		xSemaphoreGive(xCountingSemaphore_uart);
#ifndef HISENSE_LOCK	
		hal_timer_start();    //不用再次启动，等接受到串口数据后再次启动，否则浪费系统资源
#endif	
	}
	
}

uart_handle_t hal_uart_open_data(
					unsigned int dma_flag,
					unsigned int id, 
					unsigned int databits, 
					unsigned int baud, 
					unsigned int parity, 
					unsigned int stopbits, 
					unsigned int flow)
{
	unsigned int regbase;
	unsigned int vector;
	uart_dev * p_uart_dev = NULL;
	xCountingSemaphore_uart = xSemaphoreCreateCounting(32, 0);
	uint32_t value;
	p_uart_dev_data = os_zalloc(sizeof(uart_dev));
	if(p_uart_dev_data)
	{	
		p_uart_dev = p_uart_dev_data;
	}
	
	

	if(id == UART_ID_0)
	{
		regbase = UART0_BASE;
		vector = IRQ_VECTOR_UART0;		
		
		//CFG  MPW UART0 PIN MUX
		OUT32(SOC_PIN0_MUX_BASE, (IN32(SOC_PIN0_MUX_BASE) & (~(7<<15))) |(0<<15));
		OUT32(SOC_PIN0_MUX_BASE, (IN32(SOC_PIN0_MUX_BASE) & (~(7<<18))) |(0<<18));
		
		value = IN32(SW_RESET) & 0xFFFFFFEF;
		OUT32(SW_RESET,value);
		OUT32(SW_RESET,0xFFFFFFFF);

		CLK_ENABLE(CLK_UART0);
	}
	else
	{
		if(id == UART_ID_1)
		{
			regbase = UART1_BASE;
			vector = IRQ_VECTOR_UART1;			
#ifdef MPW
			//CFG  MPW UART1 PIN MUX
			OUT32(SOC_PIN0_MUX_BASE, (IN32(SOC_PIN0_MUX_BASE) & (~(7<<6))) |(2<<6));
			OUT32(SOC_PIN0_MUX_BASE, (IN32(SOC_PIN0_MUX_BASE) & (~(7<<9))) |(2<<9));
#else
			PIN_FUNC_SET(IO_MUX0_GPIO2, FUNC_GPIO2_UART1_RXD);
			PIN_FUNC_SET(IO_MUX0_GPIO3, FUNC_GPIO3_UART1_TXD);
#endif
			value = IN32(SW_RESET) & 0xFFFFFFDF;
			OUT32(SW_RESET,value);
			OUT32(SW_RESET,0xFFFFFFFF);

			CLK_ENABLE(CLK_UART1);			
		}	
		#ifdef _USR_TR6260S1
		else if(id == UART_ID_2)
		{
			regbase = UART2_BASE;
			vector = IRQ_VECTOR_UART2;			
			
			PIN_FUNC_SET(IO_MUX0_GPIO20, FUNC_GPIO20_UART2_RXD);
			PIN_FUNC_SET(IO_MUX0_GPIO21, FUNC_GPIO21_UART2_TXD);
			value = IN32(SW_RESET) & 0xFFFBFFFF;
			OUT32(SW_RESET,value);
			OUT32(SW_RESET,0xFFFFFFFF);

			CLK_ENABLE(CLK_UART2);	
		}
		#endif
		else
		{
			return (uart_handle_t)-1;
		}
	}
	p_uart_dev->vector = vector;
	p_uart_dev->regbase = regbase;

	hal_uart_rx_buff_reset();
	hal_uart_tx_buff_reset();

#ifdef FPGA
	uart_set_baudrate(regbase, baud, 1);
#else
	#ifdef AMT
	uart_set_baudrate(regbase, baud, 2); //80MHz Core clock
	#else
	uart_set_baudrate(regbase, baud, 1); //80MHz Core clock
	//uart_set_baudrate(regbase, baud, 1); //160MHz Core clock
	#endif
#endif
	uart_set_lineControl(regbase, databits, parity, stopbits, 0);
	uart_set_fifoControl(regbase, 1, 1, 1);

	#ifndef HISENSE
	hal_timer_callback_register(timer_callback_wifi, NULL);
	hal_timer_init();
	#endif

	if (dma_flag)
	{
		dma_enable = 1;
		uart_set_fifoControl_inter(1, regbase, 1, 1, 1);		

		dma_uart_init();
		hal_uart_dma_rx_config(p_uart_dev);		
	}
	else
	{
		dma_enable = 0;

		#ifndef HISENSE
		hal_timer_config(timeout_rx * 1000, 0);
		hal_timer_start();
		#endif
		
		irq_isr_register(vector, (void *)hal_uart_isr_wifi);
		irq_status_clean(vector);
		irq_unmask(vector);		
		uart_set_intEnable(regbase, UART_INT_DISABLE, UART_INT_ENABLE);
	}	
	return (uart_handle_t)p_uart_dev;
}

static int cmd_uart_buffer(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("rx_head=%d\n",uart_rx_buffer_head);
	system_printf("rx_tail=%d\n",uart_rx_buffer_tail);
	system_printf("tx_head=%d\n",uart_tx_buffer_head);
	system_printf("tx_tail=%d\n",uart_tx_buffer_tail);
	return CMD_RET_SUCCESS;
}
					
CMD(buffer, cmd_uart_buffer,	"uart buffer",	"buffer");

#endif




#if ((!defined _USR_LMAC_TEST) && (!defined _USR_MIN_SYS) && (!defined AMT) && (!defined SINGLE_BOARD_VER))
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
	system_printf("cmd_uart_psm, delay: %d\n", uart_psm_delay);

	return CMD_RET_SUCCESS;
}

CMD(uartpsm, cmd_uart_psm,  "uart delya psm time",  "uartpsm <time> (time in sec)");
#endif

#if 0

uart_handle_t uart_handle;
unsigned char buff[128] = {0};

void uart_callback(void *data)
{
	//system_printf("uart_callback, rece-len: %d\n", hal_uart_get_recv_len((uart_handle_t)data));
}


static int uart_open(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("uart_open\n");
	uart_handle = hal_uart_open(UART_ID_1, UART_DATA_BIT_8, UART_BAUD_RATE_57600, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
	hal_uart_register_recv_callback(uart_handle, uart_callback, (void *)uart_handle);
	return CMD_RET_SUCCESS;
}


static int uart_loop(cmd_tbl_t *t, int argc, char *argv[])
{
	int length;
	system_printf("uart_loop\n");

	length = hal_uart_read(uart_handle, buff, 128);
	hal_uart_write(uart_handle, buff, length);
	return CMD_RET_SUCCESS;
}

CMD(uartc,
    uart_open,
    "uart create test",
    "uart create test");


CMD(uartlp,
    uart_loop,
    "uart create test",
    "uart create test");


#endif


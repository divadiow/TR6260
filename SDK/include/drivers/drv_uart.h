
#ifndef	HAL_UART_H
#define HAL_UART_H

#define UART_ID_0		0
#define UART_ID_1		1
#define UART_ID_2		2


#define UART0_BASE			(0x00602000) /* Device base address */
#define UART1_BASE			(0x00603000) /* Device base address */
#define UART2_BASE			(0x0060C000) /* Device base address */


#define UART_INT_NONE			0
#define UART_INT_ERROR			1
#define UART_INT_TIMEOUT		2
#define UART_INT_RX_DONE		3
#define UART_INT_TX_EMPTY		4
#define UART_INT_MAX			5


#define UART_BAUD_RATE_38400		38400
#define UART_BAUD_RATE_57600		57600
#define UART_BAUD_RATE_115200		115200

#define UART_DATA_BIT_5	0
#define UART_DATA_BIT_6	1
#define UART_DATA_BIT_7	2
#define UART_DATA_BIT_8	3

#define UART_PARITY_NONE	0
#define UART_PARITY_ODD	1
#define UART_PARITY_EVEN	2

#define UART_STOP_BIT_1		0
/*  The num of stop bit is based on the databits setting. 
  *      When databits = 0, stop bit is 1.5bits 
  *      When databits = 1,2,3, stop bit is 2bits
  */
#define UART_STOP_BIT_OTHER	1  


#define UART_INT_ENABLE	1
#define UART_INT_DISABLE	0

typedef struct uart_config_t {
	unsigned int buad_rate;
	unsigned int databits;
	unsigned int parity;
	unsigned int stopbits;
	unsigned int flow_contrl;
} uart_config;

unsigned char uart_get_intType(unsigned int regBase);
int uart_get_config(unsigned int regBase, uart_config * config);
void uart_set_baudrate(unsigned int regBase, unsigned int baud, unsigned int cond);
void uart_set_lineControl(unsigned int regBase, unsigned int databits, unsigned int parity, unsigned int stopbits, unsigned int bc);
void uart_set_fifoControl(unsigned int regBase, unsigned int tFifoRst, unsigned int rFifoRst,unsigned int fifoEn);
void uart_set_intEnable(unsigned int regBase, unsigned int tx, unsigned int rx);
void uart_data_write(unsigned int regBase, const unsigned char * buf, unsigned int len);
void at_uart_data_write(unsigned int regBase, const unsigned char * buf, unsigned int len);
void uart_put_char(unsigned int regBase, const char buf);
int uart_data_tstc(unsigned int regBase);
unsigned char uart_data_getc(unsigned int regBase);

typedef void * uart_handle_t;
#define UART_RX_BUF_SIZE 	128

int hal_uart_register_recv_callback(uart_handle_t handle, void (* callback)(void *), void *data);
int hal_uart_get_recv_len(uart_handle_t handle);
void hal_uart_write(uart_handle_t handle, const unsigned char * buf, int len);
unsigned int hal_uart_read(uart_handle_t handle, unsigned char * buf, int len);
void hal_uart_close(uart_handle_t handle);
uart_handle_t hal_uart_open(unsigned int id, 
								unsigned int databits, 
								unsigned int baud, 
								unsigned int parity, 
								unsigned int stopbits, 
								unsigned int flow);


int hal_uart_allow_psm(void);
void hal_uart_dma_isr();



#if 1
int hal_uart_is_rx_dma_enable();
int hal_uart_is_tx_dma_enable();
void hal_uart_dma_rx_config(uart_handle_t handle);

unsigned int hal_uart_get_rxbuff_avail_len();
unsigned int hal_uart_get_txbuff_avail_len();

unsigned int hal_uart_get_rx_len();
void * hal_uart_get_rx_buff();
void * hal_uart_get_tx_buff();
int hal_uart_get_timeout_rx();
void hal_uart_start_timer();

void hal_uart_set_timeout_rx(unsigned        int time);

void hal_uart_get_rx_data_end();
void hal_uart_socket_rec_end(unsigned int len);
void hal_uart_rx_buff_reset();
void hal_uart_tx_buff_reset();
void hal_uart_rx_chan_stop();
void hal_uart_tx_chan_stop();
int hal_uart_is_tx_buff_empty();
int hal_uart_is_rx_buff_empty();
void hal_uart_rec_end(unsigned int len);
void hal_uart_write_data(uart_handle_t handle);


void hal_uart_rx_dma_isr();
void hal_uart_tx_dma_isr();
uart_handle_t hal_uart_open_data(
					unsigned int dma_flag,
					unsigned int id, 
					unsigned int databits, 
					unsigned int baud, 
					unsigned int parity, 
					unsigned int stopbits, 
					unsigned int flow);

#define DMAC_BASE         (0x00800000UL)
#define DMAC_CHN_CTRL(ch) 				(DMAC_BASE + 0x44 + 0x14*ch)
#define DMAC_CHN_SRC(ch) 				(DMAC_BASE + 0x48 + 0x14*ch)
#define DMAC_CHN_DST(ch) 				(DMAC_BASE + 0x4c + 0x14*ch)
#define DMAC_CHN_SIZE(ch) 				(DMAC_BASE + 0x50 + 0x14*ch)
#define DMAC_ABORT_REG         0x40


#endif


#endif /* HAL_UART_H */

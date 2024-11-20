#ifndef	SYSTEM_COMMON_H
#define SYSTEM_COMMON_H


#include "build_ver.h"
#include "release_ver.h"
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "system_def.h"
#include "system_log.h"


// ------------------------------------------------------------------------
// System Common Begin
// ------------------------------------------------------------------------
//
//#define RELEASE
#ifdef XIP_FLASH
#define FLASH __attribute__((used,section(".xip")))
#else
#define FLASH
#endif

#ifdef CACHE_XIP
#define ATTR_NC		__attribute__((section(".sfc_code_nc")))
#define ATTR_C		__attribute__((section(".sfc_code_c")))
#else
#define ATTR_NC
#define ATTR_C
#endif

#define SET_FIELD(reg, field, variable, value)                                 \
    do {                                                                       \
        (variable) |= (uint32_t)(reg##_##field##_MASK & ((value) << reg##_##field##_SHIFT));    \
    } while (0)

#define MOD_FIELD(reg, field, variable, value)                                 \
    do {                                                                       \
        (variable) &= ~reg##_##field##_MASK;                                   \
        (variable) |= (uint32_t)(reg##_##field##_MASK & ((value) << reg##_##field##_SHIFT));    \
    } while (0)

#define CLEAR_FIELD(reg, field, variable)                                      \
    (variable) &= ~reg##_##field##_MASK;                                   \

#define GET_FIELD(reg, field, variable) \
    (((variable) & reg##_##field##_MASK) >> reg##_##field##_SHIFT)

#define BITSET(var, num)    (var) |= ((uint32_t)0x1 << (num))
#define BITCLR(var, num)    (var) &= ~((uint32_t)0x1 << (num))
#define BITTST(var, num)    ((var) & ((uint32_t)0x1 << (num))) ? 1 : 0
#define BITTST_64(var, num)    ((var) & ((uint64_t)0x1 << (num))) ? 1 : 0

#define ABS(x)      (((x) > 0) ? (x) : 0 - (x))
/// Macro to perform division by using bit shift
#define MOD(x,n) ((x) & ((n)-1))
/// substract y from x with non-negative result
#define SUB_NON_NEG(x, y)   (((x) > (y)) ? ((x) - (y)) : 0)

#define WORD_ALIGNED(x) (((uint32_t)(x) & 0x3) == 0)
#define WORD_ALIGN(x)   ((((uint32_t)(x) + 3) >> 2) << 2)

#define OFFSET_OF(t,m) ((unsigned long) &((t*)0)->m)

static const uint32_t Hz    = 1;
static const uint32_t KHz   = 1000 * 1;
static const uint32_t MHz   = 1000 * 1000;

static const uint32_t Byte  = 1;
static const uint32_t KByte = 1024 * 1;
static const uint32_t MByte = 1024 * 1024;

static const uint32_t uSec  = 1;
static const uint32_t mSec  = 1000 * 1;
static const uint32_t Sec   = 1000 * 1000;


#define REG32(reg)			(  *( (volatile unsigned int *) (reg) ) )
#define IN32(reg)				(  *( (volatile unsigned int *) (reg) ) )
#define OUT32(reg, data)		( (*( (volatile unsigned int *) (reg) ) ) = (unsigned int)(data) )

/********************* error define ********************/
typedef int32_t trs_err_t; 

#define TRS_OK 0
#define TRS_ERR -1
#define TRS_ERR_INVALID_ARG -1
#define TRS_ERR_NO_MEM -2
#define TRS_ERR_NOT_FOUND -3
#define TRS_ERR_OP_FLASH -4
#define TRS_ERR_TIMEOUT -5
/************************** end *************************/

int system_register_tick_callback(void (*func)(void));
int system_unregister_tick_callback(void (*func)(void));

int system_register_idle_callback(void (*func)(void));
int system_unregister_idle_callback(void (*func)(void));

int system_register_soft_reboot_callback(void (*func)(void));

int system_unregister_soft_reboot_callback(void (*func)(void));


void system_task_init();

#define SYS_TASK_QUEUE_LENGTH		32
#define SYS_TASK_STACK_SIZE         (4096 / sizeof(StackType_t))
#define SYS_TASK_PRIORITY           (configMAX_PRIORITIES - 5)

#define MIN(x, y)               (((x) < (y))? (x) : (y))
#define MAX(x, y)               (((x) > (y))? (x) : (y))
#define min(x, y)               (((x) < (y))? (x) : (y))
#define max(x, y)               (((x) > (y))? (x) : (y))

#define BITAND(A, B)            (((uint32_t)A)&((uint32_t)B))
#define BITOR(A, B)             (((uint32_t)A)|((uint32_t)B))
#define BITNOT(D)               (~D)
#define BITXOR(A, B)            (A^B)
#define BITS(D, M)              (D = BITOR(D,M))
#define BITC(D, M)              (D = BITAND(D,BITNOT(M)))
#define BITSC(D, M, C)          (D = BITOR(M,BITAND(D,BITNOT(C))))

#define READ_REG(offset)        (*(volatile uint32_t*)(offset))
#define WRITE_REG(offset,value) (*(volatile uint32_t*)(offset) = (uint32_t)(value));
#define WRITE_REG2(value,offset) (*(volatile uint32_t*)(offset) = (uint32_t)(value));
//#define barrier() __asm__ __volatile__("": : :"memory")
//#define READ_REG(a) ({unsigned __v = _READ_REG(a); barrier(); __v; })
//#define WRITE_REG(v, a) ({unsigned __v = v; barrier(); _WRITE_REG(a, v); })
//#define WRITE_REG2(v, a) ({unsigned __v = v; barrier(); _WRITE_REG(a, v); })

#define READ_FIELD(reg, field) \
	((*(volatile uint32_t*)reg & (reg##_##field##_MASK)) >> (reg##_##field##_SHIFT))

// Use this macro for registers where 0 has no effect, e.g., control registers
#define WRITE_FIELD(reg, field, value)                                       \
	*(volatile uint32_t*)reg |= (uint32_t)(value << reg##_##field##_SHIFT);  \

// Use this macro for registers where 0 does have an effect (for multi-bit field)
#define RMW_FIELD(reg, field, value)                                         \
	do {                                                                     \
		uint32_t temp = *(uint32_t*)reg;                                     \
		temp &= (uint32_t)(~reg##_##field##_MASK);                           \
		temp |= (uint32_t)(value << reg##_##field##_SHIFT);                  \
		*(uint32_t*)reg = temp;                                              \
	} while (0)

#define INDEXOF(base, item) ((((uint8_t*)item)-((uint8_t*)base))/sizeof(*item))


typedef union _UINT64 {
	struct {
		uint32_t low;
		uint32_t high;
	} u;
	uint64_t q;
} UINT64;

typedef union _INT64 {
	struct {
		uint32_t low;
		int32_t high;
	} u;
	int64_t q;
} INT64;

void show_assert(const char *x, const char *file, const char *func, unsigned int line);
void show_assert_v(const char *x, const char *file, const char *func, unsigned int line, int v);

#define ASSERT(x)       if ((x)) {} else {show_assert(#x, __FILE__, __FUNCTION__, __LINE__); }
#define ASSERT_V(x, v)  if ((x)) {} else {show_assert_v(#x, __FILE__, __FUNCTION__, __LINE__, v); }
#define ASSERT_VALUE_RANGE(s,e,v)    if (((uint32_t)v>=(uint32_t)&s)&&((uint32_t)v<(uint32_t)&e)) {} else {show_assert("Range"#s"<="#v"<"#e, __FILE__, __FUNCTION__, __LINE__);}
#define ASSERT_VALUE_RANGE_V(s,e,v,l)    if (((uint32_t)v>=(uint32_t)&s)&&((uint32_t)v<(uint32_t)&e)) {} else {show_assert_v("Range"#s"<="#v"<"#e, __FILE__, __FUNCTION__, __LINE__, l);}

uint16_t    mem_read_16(const uint8_t *ptr);
uint32_t    mem_read_32(const uint8_t *ptr);
void        mem_write_16(uint16_t u16, uint8_t *ptr);
void        mem_write_32(uint32_t u32, uint8_t *ptr);

uint32_t    swap_uint32(uint32_t val);
int32_t     swap_int32(int32_t val);
uint16_t    swap_uint16(uint16_t val);
int16_t     swap_int16(int16_t val);

struct _SYS_BUF;

// ------------------------------------------------------------------------
// System Common End
// ------------------------------------------------------------------------
// -- General Include --
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"
//#include "umac_bcn_monitor.h"
#include "timers.h"




#if 1


#include "util_trace.h"
#include "util_cli_freertos.h"
#include "util_cmd.h"

//typedef void    (*isr_callback_t)(int vector);
//void            system_isr_init();
//void            system_register_isr(int evid , isr_callback_t isr);
#define system_irq_save             portSET_INTERRUPT_MASK_FROM_ISR
#define system_irq_restore          portCLEAR_INTERRUPT_MASK_FROM_ISR
#define system_irq_enable			portENABLE_INTERRUPTS
#define system_irq_disable			portDISABLE_INTERRUPTS
//void            system_irq_unmask(int evid);
//void            system_irq_prio(int evid , int prio);
//void            system_constructor_init();

// -- Print System ---
#ifdef HISENSE_LOCK
#define PRINT_BUFFER_SIZE 1024
#elif ENABLE_EZIOT
#define PRINT_BUFFER_SIZE 1024
#else
#define PRINT_BUFFER_SIZE	256
#endif
void system_register_printf_port(unsigned int portID, void (* printf)(unsigned int, const unsigned char *, unsigned int));
void system_printf(const char *f, ...);
void system_vprintf(const char *f, va_list args);
void system_oprintf(const char *f, unsigned int len);

#ifdef UART_WPA_SEPARATION
void system_register_wpa_port(unsigned int portID);
void system_wpa_write(const char *f, ...);
#endif

// -- System Buffer --

// -- General Include --
//#include "FreeRTOS.h"
//#include "semphr.h"
//#include "bsp_hal.h"
//#include "hal.h"
//#include "nds32.h"

// -- UTIL Include --
#include "util_trace.h"
#include "util_cli_freertos.h"
#include "util_cmd.h"


//#include "nrc_system_8266.h"
// -- Hal Include --
#include "drv_uart.h"
#include "drv_pwm.h"
//#include "hal_lmac_ts8266.h"

#define LMAC_CONFIG_FREERTOS            1

#define LMAC_CONFIG_11N                 1
#define LMAC_CONFIG_11AH                0

#define LMAC_CONFIG_NAN                 0

#ifdef INCLUDE_STANDALONE

    #define LMAC_CONFIG_BUFFER_SIZE         512
    #define LMAC_CONFIG_DL_DESCRIPTOR       32// RX
    #define LMAC_CONFIG_POOL_0_NUM          32 // RX
    #define LMAC_CONFIG_POOL_1_NUM          32// TX
#else
    #define LMAC_CONFIG_BUFFER_SIZE         512
    #define LMAC_CONFIG_DL_DESCRIPTOR       32 // RX
    #define LMAC_CONFIG_POOL_0_NUM          32 // RX
#ifdef MPW
    #define LMAC_CONFIG_POOL_1_NUM          32 // TX
#else
    #define LMAC_CONFIG_POOL_1_NUM          64 // TX
#endif
#endif

#define LMAC_CONFIG_CREDIT_QM0          8
#define LMAC_CONFIG_CREDIT_QM1          8
#define LMAC_CONFIG_CREDIT_QM2          8
#define LMAC_CONFIG_CREDIT_QM3          8

#include "hal_lmac_register.h"
//#include "hal_lmac_register_ts8266.h"
#include "hal_lmac_common.h"


//#include "hal_sdio_ts8266.h"

#ifdef FPGA
#include "hal_phy_fpga.h"
#include "hal_rf_fpga.h"
#else

#ifdef MPW
#include "hal_phy_mpw.h"
#include "hal_rf_mpw.h"
#else
#include "hal_phy_tr6260.h"
#include "hal_rf_tr6260.h"
#endif

#endif


// -- UMAC Include --
#include "umac_scan.h"
#include "umac_beacon.h"
#include "umac_info.h"
#include "umac_probe_resp.h"
#include "umac_bcn_monitor.h"
#endif // #ifdef TS8266

// ------------------------------------------------------------------------
// System Common Begin
// ------------------------------------------------------------------------
//
typedef struct _sys_info {
	//README swki - 2018-0730
	//current_channel and current_channel
	//These must always be set together.
    uint16_t    current_channel;  			//actually, center frequency
    uint16_t    current_channel_number; 	//channel number (under 255)
} sys_info;

extern sys_info g_sys_info;

typedef int32_t (*sys_task_func )(void *);
typedef void    (*sys_task_func_cb)(int32_t , void*);

typedef struct _SYS_TASK {
    sys_task_func       func;
    void*               param;
    sys_task_func_cb    cb;
} SYS_TASK;

bool system_schedule_work_queue_from_isr( sys_task_func func , void* param , sys_task_func_cb cb );
bool system_schedule_work_queue         ( sys_task_func func , void* param , sys_task_func_cb cb );


// Any Wordsize of SYS_HDR
typedef struct _SYS_HDR {
    int         (*m_cb)(struct _SYS_BUF*);
    uint8_t     m_type;
    uint8_t     m_ref_count;
    uint8_t     m_credit_id     :4;
    uint8_t     m_hooked   		:1;
    uint8_t     m_prealloc_id   :2;
    uint8_t     m_interface_id  :1;
    uint8_t     pool_id      ;   // TODO Pool* m_pool

    uint16_t    m_payload_length;
    uint16_t    m_payload_offset;
    struct      _SYS_BUF*       m_next;
    struct      _SYS_BUF*       m_link;
} SYS_HDR;

// 2 Word HIF_HDR
typedef struct _HIF_HDR {
	uint8_t type;
	uint8_t subtype;
	uint8_t flags;
	int8_t vifindex;
	uint16_t len;
	uint16_t tlv_len;
} HIF_HDR;

// 1 Word FRAME_HDR
typedef struct _FRAME_HDR {
	union {
		struct _rx_flags {
			uint8_t error_mic:1;
			uint8_t iv_stripped:1;
			uint8_t reserved:6;
			uint8_t rssi;
		} rx;
		struct _tx_flags {
			uint8_t ac;
			uint8_t reserved;
		} tx;
	} flags;
	union {
		struct _rx_info {
			uint16_t frequency;
		} rx;

		struct _tx_info {
			uint8_t cipher;
			uint8_t tlv_len;
		} tx;
	} info;
} FRAME_HDR;

// add 20201102, liuyong, for telnet 
struct telnet_RingBuffer
{
	char *buffer;
	int  buffer_len;
	int  read_pos;
	int  write_pos;
};


#if 0
#if defined(NRC7291_SDK_DUAL_CM0) || defined(NRC7291_SDK_DUAL_CM3)
// Mailbox Header
typedef struct _MBX_HDR{
	uint8_t type;
	uint8_t length;
	uint16_t channel;
	uint8_t data[0];
} MBX_HDR;
#endif
#endif

// Common System Buffer Structure
/*sizeof(SYS_HDR),sizeof(LMAC_TXHDR),sizeof(HIF_HDR),sizeof(FRAME_HDR),
sizeof(PHY_TXVECTOR),sizeof(GenericMacHeader),sizeof(LMAC_RXHDR)
的值分别为 20 36 8 4 12 26 24*/
typedef struct _SYS_BUF {
	SYS_HDR 	                sys_hdr;
	union {
		struct {
			LMAC_TXHDR	        lmac_txhdr;
			union {
				struct {
					HIF_HDR     hif_hdr;
					FRAME_HDR   frame_hdr;
				};
				PHY_TXVECTOR    phy_txvector;
				uint8_t more_payload[0];
			};
			union {
				GenericMacHeader    tx_mac_header;
				uint8_t payload[0];
			};
		};
		struct {
			LMAC_RXHDR		 lmac_rxhdr;
			GenericMacHeader rx_mac_header;
		};
#if defined(NRC7291_SDK_DUAL_CM0) || defined(NRC7291_SDK_DUAL_CM3)
		struct {
			MBX_HDR		 mbx_hdr;
		};
#endif
	};
} SYS_BUF;


#if 0
#if defined(NRC7291_SDK_DUAL_CM0) || defined(NRC7291_SDK_DUAL_CM3)
#include "hal_mbx_nrc7291.h"
#endif

#define	CHECK_RTC_TIME(x) \
({ \
	x();\
});
#define	CHECK_RTC_TIME_V(x,v) \
({ \
	x(v);\
});
#define	CHECK_RTC_TIME_VV(x,v,vv) \
({ \
	x(v,vv);\
});

// -- Timer --
#define system_delay
#endif

void usdelay(unsigned int delay);
void msdelay(unsigned int delay);


#define SYS_HDR(buf)    buf->sys_hdr
#define SYS_HDR_SIZE    sizeof(SYS_HDR)

#define LMAC_TXHDR_SIZE sizeof(LMAC_TXHDR)

#define HIF_HDR(buf)    buf->hif_hdr
#define HIF_HDR_SIZE    sizeof(HIF_HDR)

#define FRAME_HDR(buf)  buf->frame_hdr
#define FRAME_HDR_SIZE  sizeof(FRAME_HDR_SIZE)

#define TX_MAC_HDR(buf) buf->tx_mac_header
#define RX_MAC_HDR(buf) buf->rx_mac_header

#define LMAC_RXHDR_SIZE sizeof(LMAC_RXHDR)

void print_hif_hdr(HIF_HDR * hif_hdr);
void print_hex(void *address, int count);
void print_sysbuf_hex(SYS_BUF *head);
void *system_get_idle_hook();
void *system_set_idle_hook(void *idlehook);
#ifdef HEAP_MEMORY_TRACE
#define os_malloc(size) os_malloc_withtrace(size,__FILE__, __LINE__)
#define os_zalloc(size) os_zalloc_withtrace(size,__FILE__, __LINE__)
#define os_calloc(nmemb,size) os_calloc_withtrace(nmemb,size,__FILE__, __LINE__)

void * os_malloc_withtrace(size_t size,char *filename, int line);
void * os_zalloc_withtrace(size_t size,char *filename, int line);
void * os_calloc_withtrace( size_t nmemb, size_t size,char*filename, int line );

void *os_malloc_hook(size_t size);

#else
void * os_malloc(size_t size);
void * os_zalloc(size_t size);
void * os_calloc(size_t nmemb, size_t size );

#define os_malloc_hook os_malloc
#endif
void   os_free(void *ptr);
void * os_realloc(void *ptr, size_t size);
char * os_strdup(const char *str);
unsigned char os_get_cpu_usage(void);


#define STARTUP_TYPE_POWERUP			0
#define STARTUP_TYPE_WDT_TIMEOUT		1
#define STARTUP_TYPE_WDT_RESET		2
#define STARTUP_TYPE_SOFTWARE		3
#define STARTUP_TYPE_WAKEUP			4
#define STARTUP_TYPE_OTA				5

void system_reset(int type);
void system_set_startup_type(int type);

#include "protocol.h"
#include "hal_lmac_ps_common.h"
#include "hal_lmac_downlink.h"
#include "hal_lmac_queue_manager.h"
#include "hal_lmac_test.h"
#include "drv_lmac.h"
#include "wifi_sniffer.h"
#include "system_modem_api.h"
#include "system_memory_manager.h"
#include "sntp_tr.h"

#if (!defined _USR_LMAC_TEST) && (!defined _USER_LMAC_SDIO)
#include "easyflash.h"
#endif
#include "nv_config.h"

#endif /* SYSTEM_COMMON_H */

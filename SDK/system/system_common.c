#include "system.h"
#include "pit.h"
#include "drv_rtc.h"
#include "system_priority.h"
#include "wdt.h"


#define SYS_TASK_QUEUE_ITEM_SIZE		sizeof (SYS_TASK)

sys_info                            g_sys_info;

static StaticQueue_t				g_sys_static_queue;
QueueHandle_t						g_sys_queue_handle;
uint8_t								g_sys_queue_storage[ SYS_TASK_QUEUE_LENGTH * SYS_TASK_QUEUE_ITEM_SIZE ];

StackType_t                         g_sys_task_stack[ SYS_TASK_STACK_SIZE];
StaticTask_t                        g_sys_task_tcb;
TaskHandle_t                        g_sys_task_handle;
static void (*g_sys_lmac_idle_hook)(void);

static void (*soft_reboot_callback)(void) = NULL;


void try_free_uart_rx_buffer(void)
{
	unsigned long flags;
	void* mem = NULL;
	
	extern char *g_dynbuffer_to_free;
	
	flags = system_irq_save();
	if(g_dynbuffer_to_free)
	{
		mem=g_dynbuffer_to_free;
		g_dynbuffer_to_free=NULL;
	}
	system_irq_restore(flags);
	vPortFree(mem);
}

void sys_task(void *pvParameters) 
{
    SYS_TASK task;
    int32_t task_result;
	extern char *g_dynbuffer_to_free;
	
	g_sys_queue_handle = xQueueCreateStatic ( SYS_TASK_QUEUE_LENGTH, SYS_TASK_QUEUE_ITEM_SIZE, g_sys_queue_storage, &g_sys_static_queue);
	if ( g_sys_queue_handle == 0) {
		system_printf ("Fail to create queue in SYS_TASK \n");
	} else {
        //system_printf ("g_sys_queue_handle(0x%x) = 0x%x\n" , &g_sys_queue_handle , g_sys_queue_handle );
    }
	for ( ;; )
	{
        if (xQueueReceive( g_sys_queue_handle, &task, ( TickType_t )portMAX_DELAY))
		{
            task_result = task.func(task.param);
            if(task.cb)
            {
                task.cb( task_result , task.param);
            }
			if(g_dynbuffer_to_free) try_free_uart_rx_buffer();
        }
	}
}

void system_task_init()
{
	g_sys_task_handle = xTaskCreateStatic(
				sys_task,
				"SYSTEM Task",
				SYS_TASK_STACK_SIZE,
				NULL,
				THREAD_SYSTEM_PRI/*SYS_TASK_PRIORITY*/,
				&g_sys_task_stack[0],
				&g_sys_task_tcb);

	if (g_sys_task_handle) 
		system_printf("[%s, %d] task creation succeed!(0x%x)\n", __func__, __LINE__, g_sys_task_handle);	
	else
		system_printf("[%s, %d] task creation failed!(0x%x)\n", __func__, __LINE__);
}

bool system_schedule_work_queue_from_isr( sys_task_func func , void* param , sys_task_func_cb cb )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, ret; 
	bool res = true;
	SYS_TASK task = {
         .func     = func ,
         .param    = param ,
         .cb       = cb
    };
    if(g_sys_queue_handle == 0)
    {
        system_printf("sys_task not initialized yet\n");
        return false;
    }
    ret = xQueueSendFromISR( g_sys_queue_handle , (void*)&task , &xHigherPriorityTaskWoken );
	if(ret != pdTRUE) {
		res = false;
		system_printf("system task enqeue failed!!(isr) : func:0x%08X, param:0x%08X, cb:0x%08X\n", 
				func, param, cb);
		return res;
	}
    portYIELD_FROM_ISR ( xHigherPriorityTaskWoken );
    return res;
}
bool system_schedule_work_queue(  sys_task_func func , void* param , sys_task_func_cb cb )
{
	SYS_TASK task = {
         .func     = func ,
         .param    = param ,
         .cb       = cb
    };
	bool res = true;
    if(g_sys_queue_handle == 0)
    {
        system_printf("sys_task not initialized yet\n");
        return false;
    }
	BaseType_t ret = xQueueSend(g_sys_queue_handle , (void*)&task , ( TickType_t )portMAX_DELAY );
	if(ret != pdTRUE){
		res = false;
		system_printf("system task enqeue failed!! : func:0x%08X, param:0x%08X, cb:0x%08X\n", 
				func, param, cb);
	}
	return res;
}
inline void show_assert(const char *x, const char *file, const char *func, unsigned int line) 
{
   system_printf("[%u] ASSERT(%s) at %s() in %s, %d\n", NOW, x, func, file, line);
}

inline void show_assert_v(const char *x, const char *file, const char *func, unsigned int line, int v) 
{
    system_printf("[%u] ASSERT(%s) at %s() in %s, %d  for %d\n", NOW, x, func, file, line, v);
}
 
inline uint16_t mem_read_16(const uint8_t *ptr) 
{
	return (((*ptr) << 8) | (*(ptr + 1))); 
}

inline uint32_t mem_read_32(const uint8_t *ptr) 
{
	return (((*ptr) << 24) | ((*(ptr + 1)) << 16) | ((*(ptr + 2)) << 8) | (*(ptr + 3)));
}

inline void mem_write_16(uint16_t u16, uint8_t *ptr) 
{
    *ptr = (u16 & 0xFF00) >> 8;
	*(ptr + 1) = u16 & 0x00FF; 
}

inline void mem_write_32(uint32_t u32, uint8_t *ptr) 
{
	*ptr     = (u32 & 0xFF000000) >> 24;
	*(ptr + 1) = (u32 & 0x00FF0000) >> 16;
	*(ptr + 2) = (u32 & 0x0000FF00) >> 8;
	*(ptr + 3) = u32 & 0x000000FF; 
}

inline uint32_t swap_uint32(uint32_t val) 
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

inline int32_t swap_int32(int32_t val) 
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | ((val >> 16) & 0xFFFF); 
}

inline uint16_t swap_uint16(uint16_t val)
{
	return (val << 8) | (val >> 8);
}

inline int16_t swap_int16(int16_t val) 
{
	return (val << 8) | ((val >> 8) & 0xff); 
}

void print_hif_hdr(HIF_HDR * hif_hdr)
{
	system_printf("HIF_HDR:%p\n", hif_hdr);
	system_printf("type:%d, subtype:%d\n",
			hif_hdr->type,
			hif_hdr->subtype);
	system_printf("flags:0x%02X, vifIndex:%d\n", 
			hif_hdr->flags,
			hif_hdr->vifindex);
	system_printf("len:%d, tlv_len:%d\n", 
			hif_hdr->len,
			hif_hdr->tlv_len);
}

void print_hex(void *address, int count)
{
	int data_size = count;
	uint8_t *p = (uint8_t*) address;
	int i;

	system_printf("\n-- 0x%08X data_size=%d\n", address, data_size);
	for (i = 0; i < data_size; i++) {
		if (0 == (i & 7))
			system_printf("\n%03d :", i);

		system_printf("%02X ", *p);
		p++;

	}
	system_printf("\n");
}

void print_sysbuf_hex(SYS_BUF *head)
{
	SYS_BUF *iter = head;
	while(iter) {
		print_hex((uint8_t*)iter, LMAC_CONFIG_BUFFER_SIZE);
		iter = SYS_BUF_LINK(iter);
	}
}


void *system_set_idle_hook(void *idlehook)
{
    return NULL;
}

typedef struct system_printf_dev_t
{
#ifdef UART_WPA_SEPARATION
	unsigned int wpa_portID;
#endif	
	unsigned int printf_portID;
	unsigned int printf_timeStamp;
	void (* printf_func)(unsigned int, const unsigned char *, unsigned int);
	char print_buffer[PRINT_BUFFER_SIZE+16];

}system_printf_dev;

static system_printf_dev g_sys_printf_dev;

void system_register_printf_port(unsigned int portID, void (* printf)(unsigned int, const unsigned char *, unsigned int))
{
	system_printf_dev * pPrintDev = &g_sys_printf_dev;
	pPrintDev->printf_portID = portID;
	pPrintDev->printf_func = printf;
	pPrintDev->printf_timeStamp = 1;
}

#ifdef UART_WPA_SEPARATION

void system_register_wpa_port(unsigned int portID)
{
	system_printf_dev * pPrintDev = &g_sys_printf_dev;
	pPrintDev->wpa_portID = portID;
}


void system_wpa_write(const char *f, ...)
{
	unsigned long flags;
	system_printf_dev * pPrintDev = &g_sys_printf_dev;
	va_list ap;
	va_start(ap,f);

	flags = system_irq_save();
	if(pPrintDev->printf_func)
	{
		int offset = 0;
		offset += vsnprintf( &pPrintDev->print_buffer[offset], PRINT_BUFFER_SIZE+1, f, ap);
		pPrintDev->printf_func(pPrintDev->wpa_portID, (const unsigned char *)pPrintDev->print_buffer, offset);
	}
	system_irq_restore(flags);
	va_end(ap);
}
#endif	
#ifdef TELNETD_ENABLE	
// add 20201102, liuyong, for telnet
extern int telnet_negotiate_success;
extern struct telnet_RingBuffer telnet_log_RingBuffer;
extern SemaphoreHandle_t telnet_waite_printf_Semaphore;
// end add 20201102, liuyong, for telnet
#endif
void system_printf(const char *f, ...)
{
	unsigned long flags;
	system_printf_dev * pPrintDev = &g_sys_printf_dev;
	int offset = 0;
	int result;
	
#if(!defined _USR_LMAC_TEST)&&(!defined SINGLE_BOARD_VER)
	int  rtcCnt = rtc_get_32K_cnt();
#endif

	va_list ap;
	va_start(ap,f);

	flags = system_irq_save();
#ifdef TELNETD_ENABLE	
	// add 20201102, liuyong, for telnet
	if(telnet_negotiate_success)
	{
		if(pPrintDev->printf_timeStamp)
		{
			offset = snprintf( &pPrintDev->print_buffer[0], PRINT_BUFFER_SIZE, "[%02d:%02d:%02d.%03d]", RTC_ALARM_GET_HOUR(rtcCnt), RTC_ALARM_GET_MIN(rtcCnt), RTC_ALARM_GET_SEC(rtcCnt), RTC_ALARM_GET_32K(rtcCnt) /33);
		}
		offset += vsnprintf( &pPrintDev->print_buffer[offset], PRINT_BUFFER_SIZE, f, ap);
		if(pPrintDev->print_buffer[offset-1] == 0x0a || pPrintDev->print_buffer[offset-2] == 0x0a)
		{
			pPrintDev->printf_timeStamp = 1;
		}
		else
		{
			pPrintDev->printf_timeStamp = 0;
		}
		extern int write_telnet_RingBuffer(struct telnet_RingBuffer* ptcmd, char *data, int len);
		result = write_telnet_RingBuffer(&telnet_log_RingBuffer, pPrintDev->print_buffer, offset);
		xSemaphoreGive(telnet_waite_printf_Semaphore);
		system_irq_restore(flags); 
		return;
	}
	// end add 20201102, liuyong, for telnet
#endif
	if(pPrintDev->printf_func)
	{
		#if(!defined _USR_LMAC_TEST)&&(!defined SINGLE_BOARD_VER)
		if(pPrintDev->printf_timeStamp)
			offset = snprintf( &pPrintDev->print_buffer[0], PRINT_BUFFER_SIZE, "[%02d:%02d:%02d.%03d]", RTC_ALARM_GET_HOUR(rtcCnt), RTC_ALARM_GET_MIN(rtcCnt), RTC_ALARM_GET_SEC(rtcCnt), RTC_ALARM_GET_32K(rtcCnt) /33);
		#endif

		offset += vsnprintf( &pPrintDev->print_buffer[offset], PRINT_BUFFER_SIZE+1, f, ap);

		pPrintDev->printf_func(pPrintDev->printf_portID, (const unsigned char *)pPrintDev->print_buffer, offset/*strlen(pPrintDev->print_buffer)*/);

#ifdef MPW
		if(pPrintDev->print_buffer[offset-1] == 0x0a)
#else
		if(pPrintDev->print_buffer[offset-2] == 0x0a)
#endif
			pPrintDev->printf_timeStamp = 1;
		else
			pPrintDev->printf_timeStamp = 0;
	}
	system_irq_restore(flags); 

	va_end(ap);
}

void system_vprintf(const char *f, va_list args)
{
	unsigned long flags;
	system_printf_dev * pPrintDev = &g_sys_printf_dev;
	int offset = 0;
	int result;
	
	flags = system_irq_save();
#ifdef TELNETD_ENABLE	
	// add 20201102, liuyong, for telnet
	if(telnet_negotiate_success)
	{
		if(pPrintDev->printf_timeStamp)
		{
			offset = snprintf( &pPrintDev->print_buffer[0], PRINT_BUFFER_SIZE, "[%08x]", rtc_get_32K_cnt());
		}
		offset += vsnprintf( &pPrintDev->print_buffer[offset], PRINT_BUFFER_SIZE, f, args);
		if(pPrintDev->print_buffer[offset-1] == 0x0a || pPrintDev->print_buffer[offset-2] == 0x0a)
		{
			pPrintDev->printf_timeStamp = 1;
		}
		else
		{
			pPrintDev->printf_timeStamp = 0;
		}
		extern int write_telnet_RingBuffer(struct telnet_RingBuffer* ptcmd, char *data, int len);
		result = write_telnet_RingBuffer(&telnet_log_RingBuffer, pPrintDev->print_buffer, offset);
		xSemaphoreGive(telnet_waite_printf_Semaphore);
		system_irq_restore(flags); 
		return;
	}
	// end add 20201102, liuyong, for telnet
#endif
	if(pPrintDev->printf_func)
	{
		#if(!defined _USR_LMAC_TEST)&&(!defined SINGLE_BOARD_VER)
		if(pPrintDev->printf_timeStamp)
			offset = snprintf( &pPrintDev->print_buffer[0], PRINT_BUFFER_SIZE-offset, "[%08x]", rtc_get_32K_cnt());
		#endif

		
		//offset = snprintf( &pPrintDev->print_buffer[0], PRINT_BUFFER_SIZE, "[%08d]", rtc_get_32K_cnt());
		offset += vsnprintf( &pPrintDev->print_buffer[offset], PRINT_BUFFER_SIZE+1, f, args);
		pPrintDev->printf_func(pPrintDev->printf_portID, (const unsigned char *)pPrintDev->print_buffer, offset/*strlen(pPrintDev->print_buffer)*/);

	#ifdef MPW
			if(pPrintDev->print_buffer[offset-1] == 0x0a)
	#else
			if(pPrintDev->print_buffer[offset-2] == 0x0a)
	#endif
			pPrintDev->printf_timeStamp = 1;
		else
			pPrintDev->printf_timeStamp = 0;
	}
	system_irq_restore(flags); 
}

void system_oprintf(const char *f, unsigned int len)
{
	unsigned long flags;
	system_printf_dev * pPrintDev = &g_sys_printf_dev;
	
	flags = system_irq_save();
	if(pPrintDev->printf_func)
	{
#ifdef UART_WPA_SEPARATION

		pPrintDev->printf_func(pPrintDev->wpa_portID, (const unsigned char *)f, len);
#else
		//pPrintDev->printf_func(pPrintDev->printf_portID, (const unsigned char *)f, strlen(f));
		pPrintDev->printf_func(pPrintDev->printf_portID, (const unsigned char *)f, len);
#endif
	}
	system_irq_restore(flags); 
}



void usdelay(unsigned int delay)
{
	unsigned long flags;
	flags = system_irq_save();
	pit_delay(delay * 80);
	system_irq_restore(flags); 
}

void msdelay(unsigned int delay)
{
	unsigned long flags;
	flags = system_irq_save();
	pit_delay(delay * 80000);
	system_irq_restore(flags); 
}
#ifdef HEAP_MEMORY_TRACE
void * os_malloc_withtrace(size_t size,char *filename, int line)
{
	return pvPortMalloc_WithTrace(size,filename,line);
}
void * os_zalloc_withtrace(size_t size,char *filename, int line)
{
	void *p = os_malloc_withtrace(size,filename,line);
	if (p)
		memset(p, 0, size);
	return p;
}
void * os_calloc_withtrace( size_t nmemb, size_t size,char*filename, int line )
{
    return pvPortCalloc_WithTrace(nmemb, size,filename,line);
}

void * os_malloc_hook(size_t size)

{
    return pvPortMalloc(size);
}

#else
void * os_malloc(size_t size)
{
    return pvPortMalloc(size);
}
void * os_zalloc(size_t size)
{
	void *p = os_malloc(size);
	if (p)
		memset(p, 0, size);
	return p;
}
void * os_calloc( size_t nmemb, size_t size )
{
    return pvPortCalloc(nmemb, size);
}
#endif
void os_free(void *ptr)
{
	vPortFree(ptr);
}

void * os_realloc(void *ptr, size_t size)
{
	return pvPortReMalloc(ptr, size);
}

char * os_strdup(const char *str)
{
    char *dup;
    
    size_t size = strlen(str);
    dup = os_zalloc(size + 1);
    if (dup) {
        memcpy(dup, str, size);
    }

    return dup;
}

#if (!defined _USR_LMAC_TEST) && (!defined _USER_LMAC_SDIO)

void system_set_startup_type(int type)
{
	int ret;
	char nv_str[4];

	ret = snprintf(nv_str, 4, "%d", type);
	ef_set_env_blob(NV_STARTUP_TYPE, nv_str, ret-1);
}


void system_reset(int type)
{
    void (*entry_point)(void);	

    system_set_startup_type(type);

    switch (type)
    {
        case STARTUP_TYPE_WDT_RESET:
        case STARTUP_TYPE_OTA:
            wdt_reset_chip(WDT_RESET_CHIP_TYPE_REBOOT);
            break;

        case STARTUP_TYPE_SOFTWARE:
            if (soft_reboot_callback != NULL)
            {
                soft_reboot_callback();
            }

            GIE_DISABLE();
            entry_point = (void (*)(void))(0x10000);
            (*entry_point)();
            break;
    }
}

#endif

#ifdef  OS_CALC_CPU_USAGE

static volatile unsigned int osCPU_IdleTickTotal=0;
static volatile unsigned int osCPU_IdleTickStart;
static volatile unsigned int osCPU_IdleTick;

void vApplicationTickHook(void)
{
	static unsigned int tick = 0;

	if(tick++ >= TICK_CALC_CPU_USAGE)
	{
		tick = 0;
		osCPU_IdleTick = osCPU_IdleTickTotal<TICK_CALC_CPU_USAGE ? osCPU_IdleTickTotal : TICK_CALC_CPU_USAGE;
		osCPU_IdleTickTotal = 0;
	}
}

void vPortSwitchIn(void)
{
	if(xTaskGetCurrentTaskHandle() == xTaskGetIdleTaskHandle())
	{
		osCPU_IdleTickStart = xTaskGetTickCountFromISR();
	}
}

void vPortSwitchOut(void)
{
	if(xTaskGetCurrentTaskHandle() == xTaskGetIdleTaskHandle())
	{
		osCPU_IdleTickTotal += xTaskGetTickCountFromISR()-osCPU_IdleTickStart;
	}
}

unsigned char os_get_cpu_usage(void)
{
	return 100-osCPU_IdleTick*100/TICK_CALC_CPU_USAGE;
}
#else

#if ( configUSE_TICK_HOOK != 0 )
static void (*tick_callback)(void) = NULL;
int system_register_tick_callback(void (*func)(void))
{
    int ret = TRS_OK;
    if (tick_callback == NULL){
        tick_callback = func;
    }
    else 
    {
        ret = TRS_ERR;
    }
    return ret;
}

int system_unregister_tick_callback(void (*func)(void))
{
    int ret = TRS_OK;
    
    if(tick_callback == func)
    {   
        tick_callback = NULL;
    }
    else
    {
        ret = TRS_ERR;
    }
    
    return ret;
}


void vApplicationTickHook(void)
{
    if(tick_callback){
        tick_callback();	
    }
}
#endif

#if ( configUSE_IDLE_HOOK != 0 )
static void (*idle_callback)(void) = NULL;
int system_register_idle_callback(void (*func)(void))
{
    int ret = TRS_OK;
    if (idle_callback == NULL){
        idle_callback = func;
    }
    else 
    {
        ret = TRS_ERR;
    }
    return ret;
}

int system_unregister_idle_callback(void (*func)(void))
{
    int ret = TRS_OK;
    if(idle_callback == func)
    {   
        idle_callback = NULL;
    }
    else
    {
        ret = TRS_ERR;
    }
    
    return ret;
}

void vApplicationIdleHook(void)
{
    if(idle_callback)
    {
        idle_callback();	
    }
}
#endif

int system_register_soft_reboot_callback(void (*func)(void))
{
    int ret = TRS_OK;
    if (soft_reboot_callback == NULL)
    {
        soft_reboot_callback = func;
    }
    else 
    {
        ret = TRS_ERR;
    }
    return ret;
}

int system_unregister_soft_reboot_callback(void (*func)(void))
{
    int ret = TRS_OK;
    if(soft_reboot_callback == func)
    {   
        soft_reboot_callback = NULL;
    }
    else
    {
        ret = TRS_ERR;
    }
    
    return ret;
}


#endif


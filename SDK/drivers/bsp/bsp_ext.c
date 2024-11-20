

//#include <string.h>
#include "FreeRTOS.h"
#include "soc.h"

#define TICK_HZ configTICK_RATE_HZ

extern unsigned char _heap_start[];
extern unsigned char _heap1_start[];
extern unsigned char __BUF_END[];

#ifdef MPW
HeapRegion_t sysHeap[] = 
{
	{_heap1_start, 0}, 
	{_heap_start, 64*1024},
/*	{_heap2_start, 24*1024}, */
	{0,0}
};
#else

#if 1
HeapRegion_t sysHeap[] = 
{
	{_heap1_start, 0}, 
	{__BUF_END, 0}, 
/*	{_heap2_start, 24*1024}, */
	{0,0}
};
#endif

#endif

static void bsp_init_tick(unsigned int period, unsigned int  vecId)
{
extern void vPreemptiveTick();

	/* System tick init 	 */
	void *tick_isr_fp = vPreemptiveTick;

	/*  Set timer  as system tick by default  */
	pit_ch_reload_value(PIT_NUM_1, PIT_CHN_2, period);
	pit_int_enable(PIT_NUM_1, PIT_CHN_2, 1);
	pit_ch_ctrl(PIT_NUM_1, PIT_CHN_2, 0, PIT_CHCLK_APB, PIT_CHMODE_32BIT_TM);

	/*    tick ISR init 	 */
	irq_status_clean(vecId);
	irq_unmask(vecId);			
	irq_isr_register(vecId, tick_isr_fp);

	/* start timer */
	pit_ch_mode_set(PIT_NUM_1, PIT_CHN_2, PIT_CH_TM0_EN);
}


static void bsp_init_heap(void)
{
#ifdef MPW
    sysHeap[0].xSizeInBytes = 0x00038000 - (unsigned int)_heap1_start;
	//memset(sysHeap[0].pucStartAddress, 0 , sysHeap[0].xSizeInBytes);
	//memset(sysHeap[1].pucStartAddress, 0 , sysHeap[1].xSizeInBytes);
#else
    sysHeap[0].xSizeInBytes = 0x00224000 - (unsigned int)_heap1_start;

#ifndef ALIYUN
    sysHeap[1].xSizeInBytes = 0x00252000 - (unsigned int)__BUF_END;
#else
    sysHeap[1].xSizeInBytes = 0x00254000 - (unsigned int)__BUF_END;
#endif
	
#endif

	vPortDefineHeapRegions(sysHeap);
}



void bsp_init_os(void)
{
	bsp_init_tick((APB_CLOCK/TICK_HZ), IRQ_VECTOR_PIT1);
	bsp_init_heap();
}


#ifndef IRQ_H
#define IRQ_H

/*****************************************************************************
 * IRQ Vector
 ****************************************************************************/

#define VECTOR_NUMINTRS		32

 
#define IRQ_VECTOR_MAC0		0
#define IRQ_VECTOR_UART2		1
#define IRQ_VECTOR_RES0		2
#define IRQ_VECTOR_SDIO		3
#define IRQ_VECTOR_PIT0			4
#define IRQ_VECTOR_PIT1			5
#define IRQ_VECTOR_WDT			6
#define IRQ_VECTOR_GPIO		7
#define IRQ_VECTOR_I2C			8
#define IRQ_VECTOR_SPI1			9
#define IRQ_VECTOR_SPIM		10
#define IRQ_VECTOR_PCU			11
#define IRQ_VECTOR_MAC1		12
#define IRQ_VECTOR_MAC2		13
#define IRQ_VECTOR_MAC3		14
#define IRQ_VECTOR_UART1		15
#define IRQ_VECTOR_DMA			16
#define IRQ_VECTOR_RTC			17
#define IRQ_VECTOR_UART0		18
#define IRQ_VECTOR_SWI			19
#define IRQ_VECTOR_PM			20
#define IRQ_VECTOR_BMC			21
#define IRQ_VECTOR_I2S			22

#define IN_ISR_FALSE 0
#define IN_ISR_TRUE 1

#ifndef __ASSEMBLER__

#include "nds32_intrinsic.h"

#define GIE_ENABLE()                    __nds32__gie_en()
#define GIE_DISABLE()                   __nds32__gie_dis()

static inline void GIE_SAVE(unsigned long *var)
{
	*var = __nds32__mfsr(NDS32_SR_PSW);
	GIE_DISABLE();
}

static inline void GIE_RESTORE(unsigned long var)
{
	if (var & 1)
		GIE_ENABLE();
}


static inline int GIE_STATUS(void)
{
	return __nds32__mfsr(NDS32_SR_PSW) & 1;
}

void irq_status_clean(int _irqs_);
void	irq_unmask(int _irqs_);
void irq_mask(int _irqs_);

inline void irq_isr_register(int vector, void *isr);
int in_isr(void);

#endif

#endif

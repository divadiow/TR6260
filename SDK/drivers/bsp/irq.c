



#include "nds32.h"
#include "irq.h"

void irq_status_clean(int _irqs_)
{
	if ( _irqs_ == IRQ_VECTOR_SWI )
	{
		SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);
	}
	else
	{
		/* PEND2 is W1C */
		SR_SETB32(NDS32_SR_INT_PEND2,_irqs_);
	}
}

void	irq_unmask(int _irqs_)
{
	SR_SETB32(NDS32_SR_INT_MASK2,_irqs_);
}


void irq_mask(int _irqs_)
{
	SR_CLRB32(NDS32_SR_INT_MASK2,_irqs_);
}

extern void *OS_CPU_Vector_Table[32];

inline void irq_isr_register(int vector, void *isr)
{
	OS_CPU_Vector_Table[vector] = isr;
}

int in_isr(void)
{
    if(0x00070000 == (__nds32__mfsr(NDS32_SR_PSW) & 0x00070000)) {
        return IN_ISR_FALSE;
    }

    return IN_ISR_TRUE;
}




#ifndef __NDS32_H__
#define __NDS32_H__


#ifndef __ASSEMBLER__
#include <nds32_intrinsic.h>
#endif
#include "nds32_defs.h"



#define SR_CLRB32(reg, bit)		\
{					\
	int mask = __nds32__mfsr(reg)& ~(1<<bit);\
	__nds32__mtsr(mask, reg);	 \
	__nds32__dsb();				 \
}

#define SR_SETB32(reg,bit)\
{\
	int mask = __nds32__mfsr(reg)|(1<<bit);\
	__nds32__mtsr(mask, reg);			\
	__nds32__dsb();						\
}

#endif
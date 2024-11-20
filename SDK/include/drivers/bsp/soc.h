

#include "irq.h"
#include "pit.h"

//#ifdef CLOCK_USED_160M

#ifdef AMT
#define APB_CLOCK       40000000//160Mhz need 80000000
#else
#define APB_CLOCK		40000000
#endif
//#endif


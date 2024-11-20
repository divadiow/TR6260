

#include "nds32.h"
#include "irq.h"
#include "pit.h"
#include "soc_top_reg.h"
#include "drv_spiflash.h"

#define REG32(reg)			(  *( (volatile unsigned int *) (reg) ) )

#define VECTOR_BASE 0x00010000
#define IVB_INIT                                        \
        ((VECTOR_BASE >> IVB_offIVBASE) << IVB_offIVBASE\
         | 0x1UL << IVB_offESZ                          \
         | 0x0UL << IVB_offEVIC)

#ifdef MPW

#define PSW_MSK                                         \
        (PSW_mskGIE | PSW_mskINTL | PSW_mskPOM | PSW_mskIFCON | PSW_mskCPL)
#define PSW_INIT                                        \
        (0x0UL << PSW_offGIE                            \
         | 0x0UL << PSW_offINTL                         \
         | 0x1UL << PSW_offPOM                          \
         | 0x0UL << PSW_offIFCON                        \
         | 0x7UL << PSW_offCPL)

#define IVB_MSK                                         \
        (IVB_mskEVIC | IVB_mskESZ | IVB_mskIVBASE)

void _nds32_init_cpu(unsigned int IVB)
{
	unsigned int reg;

	/* Set PSW GIE/INTL to 0, superuser & CPL to 7 */
	reg = (__nds32__mfsr(NDS32_SR_PSW) & ~PSW_MSK) | PSW_INIT;
	__nds32__mtsr(reg, NDS32_SR_PSW);
	__nds32__isb();

	/* Set vector size: 16 byte, base: VECTOR_BASE, mode: IVIC */
	reg = (__nds32__mfsr(NDS32_SR_IVB) & ~IVB_MSK) | IVB;
	__nds32__mtsr(reg, NDS32_SR_IVB);

	/*
	 * Check interrupt priority programmable (IVB.PROG_PRI_LVL)
	 * 0: Fixed priority, 1: Programmable priority
	 */
	__nds32__mtsr(0x0, NDS32_SR_INT_CTRL);  /* Set PPL2FIX_EN to 0 to enable Programmable Priority Level */

	/* Mask and clear hardware interrupts,  IVB.IVIC_VER >= 1 */
	__nds32__mtsr(0x0, NDS32_SR_INT_MASK2);
	__nds32__mtsr(-1, NDS32_SR_INT_PEND2);
}

#define EDLM_BASE			0x00200000
/* This must be a leaf function, no child function */
void _nds32_init_mem(void) __attribute__((naked, optimize("Os")));
void _nds32_init_mem(void)
{
	/* Enable DLM */
	__nds32__mtsr(EDLM_BASE | 0x1, NDS32_SR_DLMB);
	__nds32__dsb();
}


/* DCache enable */
#define CACHE_CTL_ICACHE_ON                      (0x1UL << CACHE_CTL_offIC_EN)

void  _nds32_enable_icache(int On)
{
	unsigned int reg = __nds32__mfsr(NDS32_SR_CACHE_CTL);

	if(On)
	{
		if(reg & CACHE_CTL_ICACHE_ON)
			return;

		unsigned long end;
		unsigned long cache_line = 8 << (((__nds32__mfsr(NDS32_SR_ICM_CFG) & ICM_CFG_mskISZ) >> ICM_CFG_offISZ) - 1);

		//cfg area-mux, and change the 32K-ilm to cache 0x24000~0x2bfff
		REG32(0x601210) = 0;

		/* Invalid ICache */
		end = (1 + ((__nds32__mfsr(NDS32_SR_ICM_CFG) & ICM_CFG_mskIWAY) >> ICM_CFG_offIWAY))  \
			* ( 64 << ((__nds32__mfsr(NDS32_SR_ICM_CFG) & ICM_CFG_mskISET) >> ICM_CFG_offISET) ) * cache_line;
		do 
		{
			end -= cache_line;
			__nds32__cctlidx_wbinval(NDS32_CCTL_L1I_IX_INVAL, end);
		} while (end > 0);

		__nds32__isb();

		reg |= CACHE_CTL_ICACHE_ON;
	}
	else
	{
		if(reg & CACHE_CTL_ICACHE_ON)
			reg &= ~CACHE_CTL_ICACHE_ON;
		else
			return;
	}

	/* enable i-cache */
	__nds32__mtsr(reg, NDS32_SR_CACHE_CTL);
}

/* DCache enable */
#define CACHE_CTL_DCACHE_ON                      (0x1UL << CACHE_CTL_offDC_EN)

void _nds32_enable_dcache(int On)
{
	unsigned int reg = __nds32__mfsr(NDS32_SR_CACHE_CTL);

	if(On)
	{
		if(reg & CACHE_CTL_DCACHE_ON)
			return;

		__nds32__cctl_l1d_invalall();
		__nds32__msync_store();
		__nds32__dsb();

		reg |= CACHE_CTL_DCACHE_ON;
	}
	else
	{
		if(reg & CACHE_CTL_DCACHE_ON)
			reg &= ~CACHE_CTL_DCACHE_ON;
		else
			return;
	}

	/* enable d-cache */
	__nds32__mtsr(reg, NDS32_SR_CACHE_CTL);
}

#else
void _nds32_init_cpu(unsigned int IVB);
extern void _nds32_init_mem(void);
extern void  _nds32_enable_icache(int On);
extern void _nds32_init_mmu(void);
extern void _nds32_enable_dcache(int On);
#endif



static void bsp_init_clock(void)
{
#ifndef FPGA
	REG32(CLK_EN_BASE) = 0xFFFFFFFF;
	REG32(CLK_DBB) = (REG32(CLK_DBB) & (~(1<<2))) |(1<<2);
#ifdef AMT
	REG32(CLK_MUX_BASE) = 0x11;//15
#else
	REG32(CLK_MUX_BASE) = 0x11;
#endif

#endif
}


static void bsp_init_cpu(void)
{
	_nds32_init_cpu(IVB_INIT);

/*
 * Interrupt priority :
 * PIT(IRQ #2): highest priority
 * Others: lowest priority
 */
#define PRI1_DEFAULT            0xFFFFFFCF
#define PRI2_DEFAULT            0xFFFFFFFF

	/* Set default Hardware interrupts priority */
	__nds32__mtsr(PRI1_DEFAULT, NDS32_SR_INT_PRI);
	__nds32__mtsr(PRI2_DEFAULT, NDS32_SR_INT_PRI2);

	/* Mask all HW interrupts except SWI */
	__nds32__mtsr(1 << IRQ_VECTOR_SWI, NDS32_SR_INT_MASK2);	
}


static void bsp_init_c(void)
{
#if  defined(MPW)

	/* do nothing now... */

#else
#define MEMCPY(des, src, n)     __builtin_memcpy ((des), (src), (n))
	extern char __rw_lma_start, __rw_lma_end, __rw_vma_start;
	unsigned int size = &__rw_lma_end - &__rw_lma_start;

	/* Copy data section from LMA to VMA */
	MEMCPY(&__rw_vma_start, &__rw_lma_start, size);
#endif
}

static void bsp_init_cache(void)
{

#define CACHE_NONE				0
#define CACHE_WRITEBACK		2
#define CACHE_WRITETHROUGH	3

#define MMU_CTL_MSK                                     \
        (MMU_CTL_mskD                                   \
         | MMU_CTL_mskNTC0                              \
         | MMU_CTL_mskNTC1                              \
         | MMU_CTL_mskNTC2                              \
         | MMU_CTL_mskNTC3                              \
         | MMU_CTL_mskTBALCK                            \
         | MMU_CTL_mskMPZIU                             \
         | MMU_CTL_mskNTM0                              \
         | MMU_CTL_mskNTM1                              \
         | MMU_CTL_mskNTM2                              \
         | MMU_CTL_mskNTM3)
/*
 * NTC2: CACHE_MODE, NTC0,NTC3: Non-cacheable
 */
#define MMU_CTL_INIT                                    \
        (0x0UL << MMU_CTL_offD                          \
         | (CACHE_NONE)<< MMU_CTL_offNTC0              \
         | (CACHE_WRITEBACK) << MMU_CTL_offNTC1                     \
         | (CACHE_NONE) << MMU_CTL_offNTC2           \
         | (CACHE_NONE) << MMU_CTL_offNTC3                     \
         | 0x0UL << MMU_CTL_offTBALCK                   \
         | 0x0UL << MMU_CTL_offMPZIU                    \
         | 0x0UL << MMU_CTL_offNTM0                     \
         | 0x0UL << MMU_CTL_offNTM1                     \
         | 0x2UL << MMU_CTL_offNTM2                     \
         | 0x3UL << MMU_CTL_offNTM3)

	unsigned int reg;

	/* MMU initialization: NTC0~NTC3, NTM0~NTM3 */
	reg = (__nds32__mfsr(NDS32_SR_MMU_CTL) & ~MMU_CTL_MSK) | MMU_CTL_INIT;
	reg |= 1<<23;  //unaligned access is allowed
	__nds32__mtsr(reg, NDS32_SR_MMU_CTL);
	__nds32__dsb();

	_nds32_enable_icache(ICACHE_ENABLE);
	_nds32_enable_dcache(DCACHE_ENABLE);

}

static void bsp_init_pit(void)
{
	pit_init();
}


/*
 * Setup system tick for OS requeried.
 */
static void bsp_init(void)
{
extern void bsp_init_os(void);

	bsp_init_clock();		/* cfg main clock */
	bsp_init_cpu();		/* Initialize CPU to a post-reset state.*/
	bsp_init_c();			/* initialize c-envirment */
	bsp_init_cache();
	bsp_init_pit();
	bsp_init_os();
}


/* NDS32 reset handler to reset all devices sequentially and call application  entry function. */
void reset(void)
{
extern int main(void);
extern void hal_uart_init(void);

	/* Setup the OS system required initialization */
	bsp_init();
#ifndef _USER_LMAC_SDIO
	hal_spiflash_init();
#endif
#ifndef TUYA_SDK_ADPT
	hal_uart_init();
#endif

	/* Application enrty function */
	main();

	/* Never go back here! */
	while(1);
}


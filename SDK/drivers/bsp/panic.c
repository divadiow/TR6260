
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "nds32_intrinsic.h"
#include "nds32_defs.h"
#include "drv_uart.h"
#include "FreeRTOS.h"
//#include "semphr.h"
#include "task.h"

#include <nds32_intrinsic.h>
#include <nds32.h>

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

#include <stdio.h>
#include <string.h>
#include "system_common.h"
#include "soc_top_reg.h"
#include "drv_uart.h"
#include "easyflash.h"
#include "wdt.h"
#include "drv_spiflash.h"

#ifdef TUYA_SDK_ADPT
#include "tuya_os_adapter.h"
#endif
#ifndef PRINT_BUFFER_SIZE
#define PRINT_BUFFER_SIZE    256
#endif
char print_buffer[PRINT_BUFFER_SIZE+16] = {0};

void panic_uart_data_write(const unsigned char * buf, unsigned int len)
{
    int i = 0;
    int fifo_count = 0;

    while(len)
    {
        while(!(IN32(UART0_BASE + 0x34) & 0x20));
        fifo_count = 16;

        while (--fifo_count > 0) {
                  if(buf[i] == '\n'){
                       OUT32(UART0_BASE + 0x20, '\r');
                       if(!fifo_count)
                       break;
                  }
            OUT32(UART0_BASE + 0x20, (unsigned char)buf[i++]);
            if(--len == 0)
                      break;
        }
    }

    while(!(IN32(UART0_BASE + 0x34) & 0x40));
}

int panic_printf_type=0;

void panic_printf(const char *f, ...)
{
    va_list ap;
    va_start(ap,f);
    int offset = 0;
	if(panic_printf_type)
	{
		system_vprintf(f, ap);
	}
	else
	{
	    offset += vsnprintf((char *)print_buffer, PRINT_BUFFER_SIZE+1, f, ap);
		if(print_buffer[offset-1]==0)
		{
			offset=offset-1;
		}
	    panic_uart_data_write((unsigned char *)print_buffer, offset);

	}
	va_end(ap);
}


static const uint32_t pattern = 0x12345678;

static __inline__ char arch_endianness(void)
{
	uint8_t *p = (uint8_t *) &pattern;

	if (p[0] == 0x12)
		return 'B'; /* big endian */
	else
		return 'L'; /* little endian */
}

static __inline__ int arch_is_big_endian(void)
{
	uint8_t *p = (uint8_t *) &pattern;

	return p[0] == 0x12;
}

static __inline__ uint32_t get_unaligned_le32(const void *ptr)
{
	const uint8_t *x = ptr;

	return (x[3] << 24 | x[2] << 16 | x[1] << 8 | x[0]);
}

static __inline__ uint16_t get_unaligned_le16(const void *ptr)
{
	const uint8_t *x = ptr;

	return (x[1] << 8 | x[0]);
}

static __inline__ void put_unaligned_le32(uint32_t val, void *ptr)
{
	uint8_t *x = ptr;

	x[0] = val & 0xff;
	x[1] = (val >> 8) & 0xff;
	x[2] = (val >> 16) & 0xff;
	x[3] = (val >> 24) & 0xff;
}

static __inline__ void put_unaligned_le16(uint16_t val, void *ptr)
{
	uint8_t *x = ptr;

	x[0] = val & 0xff;
	x[1] = (val >> 8) & 0xff;
}

static __inline__ uint32_t get_unaligned_be32(const void *ptr)
{
	const uint8_t *x = ptr;

	return (x[0] << 24 | x[1] << 16 | x[2] << 8 | x[3]);
}

static __inline__ uint16_t get_unaligned_be16(const void *ptr)
{
	const uint8_t *x = ptr;
	return (x[0] << 8 | x[1]);
}

static __inline__ void put_unaligned_be32(uint32_t val, void *ptr)
{
	uint8_t *x = ptr;

	x[3] = val & 0xff;
	x[2] = (val >> 8) & 0xff;
	x[1] = (val >> 16) & 0xff;
	x[0] = (val >> 24) & 0xff;
}

static __inline__ void put_unaligned_be16(uint16_t val, void *ptr)
{
	uint8_t *x = ptr;

	x[1] = val & 0xff;
	x[0] = (val >> 8) & 0xff;
}


#define ALIGNMENT		0x0 /* I/D */
#define RESERVED_INST		0x1
#define TRAP			0x2
#define ARITHMETIC		0x3
#define PRECISE_BUS_ERROR	0x4 /* I/D */
#define IMPRECISE_BUS_ERROR	0x5 /* I/D */
#define COPROCESSOR		0x6
#define PRIVILEGED_INST		0x7
#define RESERVED_VALUE		0x8
#define NONEXISTENT_MEM_ADDR	0x9 /* I/D */
#define MPZIU_CONTROL		0xA
#define NEXT_PRECISE_STACK_OVERFLOW 0xB

#define INST			0x10

#define nds32_is_data_exception(v) ((((v) >> 4) & 1) == 0)
#define nds32_is_instruction_exception(v) !nds32_is_data_exception(v)

static const char *nds32_exception_name[] __attribute__((unused)) = {
	"Alignment",
	"Undefined instruction",
	"Trap",
	"Arithmetic",
	"Precise bus error",
	"Imprecise bus error",
	"Coprocessor",
	"Privliged instruction",
	"Reserved value",
	"Invalid memory access",
	"MPZIU control",
	"Stack overflow"
};

struct pt_regs {
	uint32_t pc;
	uint32_t ipsw;
#ifdef __TARGET_ZOL_EXT
	uint32_t lc;
	uint32_t le;
	uint32_t lb;
#endif
#ifdef __TARGET_IFC_EXT
	uint32_t ifc_lp;
#endif
	union {
		uint32_t r[32]; /* r6-r31, r0-r5 */
		struct {
			uint32_t r0, r1, r2, r3, r4, r5;
			uint32_t r6, r7, r8, r9;
			uint32_t r10, r11, r12, r13;
			uint32_t r14, r15, r16, r17;
			uint32_t r18, r19, r20, r21;
			uint32_t r22, r23, r24, r25;
			union {
				struct {
					uint32_t r26, r27, r28, r29, r30, r31;
				};
				struct {
					uint32_t p0, p1, fp, gp, lp, sp;
				};
			};
		};
	};
};


static void show_regs(struct pt_regs *pt)
{
	int i;

	for (i = 0; i < 24; i += 4) {
		panic_printf("r%-2d: %08x  r%-2d: %08x  r%-2d: %08x  r%-2d: %08x\n",
		       i, pt->r[i],
		       i + 1,  pt->r[i+1],
		       i + 2,  pt->r[i+2],
		       i + 3,  pt->r[i+3]);
	}
	panic_printf("r24: %08x  r25: %08x  p0 : %08x  p1 : %08x\n",
	       pt->r24, pt->r26, pt->r26, pt->r27);
	panic_printf("fp : %08x  gp : %08x  lp : %08x  sp : %08x\n",
	       pt->fp, pt->gp, pt->lp, pt->sp);
	panic_printf("pc : %08x  psw: %08x\n", pt->pc, pt->ipsw);
}

static void show_task(TaskStatus_t *tsk)
{
	TaskHandle_t current __attribute__((unused));

	current = xTaskGetCurrentTaskHandle();
	panic_printf("Process: %s (pid=%d, threadinfo=%p, stack limit=%p)\n",
	       tsk->pcTaskName, tsk->xTaskNumber, current, tsk->pxStackBase);
}

static void show_mem(const char *prefix, const char *indent, void *addr, int sz,
		     int width, void *pivot)
{
	unsigned long bottom, top, start;
	int i;
	const char *fmt[] = { [1] = " %02x", [2] = " %04x", [4]= " %08lx", };

	bottom = (unsigned long) addr;
	top = bottom + sz;

	if (prefix)
		panic_printf("%s:\n", prefix);

	for (start = bottom & ~31; start < ((top + 31) & ~31);  start += 32) {
		unsigned long p;
		char line[3 * 32 + 32], *cursor = line, color = 'b';
		int bad = 0;

		memset(line, ' ', sizeof(line));
		line[sizeof(line) - 1] = '\0';

		i = 0;
		p = start;
		for (; i < 32 / width && p < top; i++, p += width) {
			unsigned val;
			if (p < bottom || p >= top) {
				cursor += (2 * width + 1);
				continue;
			}
			if (p < (unsigned) pivot && color != 'r') {
				cursor += sprintf(cursor, "\x1b[31m");
				color = 'r';
				bad++;
			} else if (p > (unsigned) pivot && color != 'b') {
				cursor += sprintf(cursor, "\x1b[0m");
				color = 'b';
			}

			val = ((width == 1) ? *((uint8_t *) p) :
			       (width == 2) ? *((uint16_t *) p) : *(uint32_t *) p);
			cursor += sprintf(cursor, fmt[width], val);
		}
		panic_printf("%s%s0x%08lx:%s\x1b[0m\n", indent,
		       (bad == 0) ? "\x1b[0m" : "\x1b[31m", start, line);
	}
}


#define BACKTRACE_MAX_STACK_SIZE 256

static inline void show_stack(TaskStatus_t *tsk, void *sp, void *fp)
{
	size_t size = fp - sp;

	if (size > BACKTRACE_MAX_STACK_SIZE) {
		unsigned p1 = (unsigned) sp + BACKTRACE_MAX_STACK_SIZE/2;
		unsigned p2 = (unsigned) fp - BACKTRACE_MAX_STACK_SIZE/2;

		p1 &= ~0xf;
		p2 = (p2 + 0xf) & ~0xf;

		show_mem(NULL, "    ", sp,  (void *) p1 - sp, 4,
			 tsk->pxStackBase);
		panic_printf("    ..........               .... skipping (%4d bytes) ....       \n",
		       p2 - p1);
		sp = (void *) p2;
	}
	show_mem(NULL, "    ", sp, (void *)fp - (void *) sp, 4, tsk->pxStackBase);
}

void nds32_panic(const char *fmt, ...)
{
	char buf[128];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	panic_printf("%s\n", buf);

	asm("trap 0");
}


// #include <hal/unaligned.h>
// #include <hal/bitops.h>

static int nds32_instr_is_32bit(void *pc)
{
	uint8_t msb = *(uint8_t *) pc;
	return ((msb & 0x80) == 0);
}

#define nds32_instr_is_16bit(pc) (!nds32_instr_is_32bit(pc))

static inline uint32_t nds32_fetch_instruction(void *pc)
{
	uint32_t instr;

	if (nds32_instr_is_32bit(pc))
		instr = get_unaligned_be32(pc);
	else {
		instr = get_unaligned_be16(pc);
		instr |= (1 << 31); /* make every instruction 32-bit */
	}
	return instr;
}

#define nds32_instr_len(instr) (((instr) >> 31) ? 2 : 4)

/**
 * bf_get_w() - get the bitfield
 * @v: value
 * @pos: start bit position of the field.
 * @width: bit width of the field
 */
#define bf_get_w(v, pos, width) (((v) >> (pos)) & ((1 << (width)) - 1))

/**
 * bf_get() - get the bitfield
 * @v: value
 * @end: end bit position of the field.
 * @start: start bit position of the field.
 */
#define bf_get(v, end, start) 	bf_get_w(v, (start), ((end) - (start) + 1))

/**
 * bf_mask() - create a bit filed mask
 * @end: end bit position
 * @start:  start bit position
 *
 * This macro returns a mask whose [end, start] bits are all 1, and
 * other bits are all 0.
 */
#define bf_mask(end, start)   (bf_get(-1, end, start) << (start))

/**
 * bf_zero() - zero bit fields
 * @v : value
 * @end: end poisition
 * @start: start position
 */
#define bf_zero(v, end, start) 	((v) & ~bf_mask(end, start))

/**
 * bf_extract() - get only specified fields
 * @v: value
 * @end: end position
 * @start: start position
 */
 #define bf_extract(v, end, start) ((v) & bf_mask(end, start))

/**
 * sign_extend() - do sign extension
 * @v: value to sign extend
 * @nbits: number of bits in v
 *
 */
#define sign_extend(v, nbits)  ((((int) (v)) << (32-(nbits))) >> (32-(nbits)))

#define CONFIG_BACKTRACE
#ifdef CONFIG_BACKTRACE

/* addi $fp, $sp, # */
static inline int nds32_instr_is_addi_fp_sp(uint32_t instr)
{
	return (bf_zero(instr, 14, 0) == 0x51cf8000);
}

static inline int nds32_instr_is_smw(uint32_t instr)
{
	uint32_t mask = bf_mask(24, 6) | bf_mask(4, 2);
	return ((instr & ~mask) == 0x3a000020);
}

static inline int nds32_instr_is_lmw(uint32_t instr)
{
	uint32_t mask = bf_mask(24, 6) | bf_mask(4, 2);
	return ((instr & ~mask) == 0x3a00000);
}

static inline int nds32_instr_is_push25(uint32_t instr)
{
	return (bf_zero(instr, 6, 0) == 0x8000fc00);
}

static inline int nds32_instr_is_pop25(uint32_t instr)
{
	return (bf_zero(instr, 6, 0) == 0x8000fc80);
}

/**
 * nds32_instr_is_prologue() - check function if instr is in prolog
 *
 * @instr: current instruction
 */
static int nds32_instr_is_prologue(uint32_t instr)
{
	if (nds32_instr_is_push25(instr))
	    return 1;
	if (nds32_instr_is_smw(instr) && bf_get(instr, 9, 6) != 0)
		return 1;
	if (instr == 0x80000000 || instr == 0x00000000) /* filler */
		return 1;
	return 0;
}

/**
 * nds32_instr_is_epilogue() - check function if instr is in eplilog
 *
 * @instr: current instruction
 */
static inline int nds32_instr_is_epilogue(uint32_t instr)
{
	if (nds32_instr_is_pop25(instr))
		return 1;
	if (nds32_instr_is_lmw(instr) && bf_get(instr, 9, 6) != 0)
		return 1;

	if (bf_zero(instr, 4, 0) == 0x8000dd80)
		return 1; /* RET5 */

	if (bf_zero(instr, 14, 0) == 0x4a000000
	    && bf_get(instr, 5, 5) == 1) /* ret */
		return 1;
	if (instr == 0x64000004) /* iret */
		return 1;

	if (instr == 0x80009200) /* nop16 */
		return 1;
	return 0;
}

/**
 * nds32_prev_pc() - compute the previous pc in reverse program order
 * @pc: current pc
 *
 * Returns:
 *  The previous pc (in the program order)
 *
 * Tracing variable-size instructions backward is not straightforward.
 *
 * 1 = 16-bit instruction (MSB 0), 0 = non-16-bit instruction (MSB 1)
 *     pc-2  pc-4   pc-6   pc-8
 *     1     1      X      X      : done - pc-2 is 16
 *     1     0      1      X      : done - pc-4 is 32
 *     1     0      0      1      : done - pc-2 is 16, pc-6 is 32
 *     1     0      0      0      : ...
 *
 *     0     1      X      X      : done - error
 *     0     0      X      X      : done - pc-4 is 32
 *
 * We keep looking ahead until encountered by "1". If the distance
 * from @pc-2 is multiple of 4, then the previous pc is @pc-2 (16-bit);
 * otherwise @pc-4 (32-bit)
  */
static void *nds32_prev_pc(void *pc)
{
	unsigned v = 0;
	void *la; /* look ahead */

	v |= nds32_instr_is_16bit(pc - 2);
	v = (v << 1) | nds32_instr_is_16bit(pc - 4);
	if (v == 1)
		return NULL;
	else if (v == 0)
		return pc - 4;
	else if (v == 3)
		return pc - 2;

	la = pc - 6;
	while (!nds32_instr_is_16bit(la)) {
		la -= 2;
	}
	return ((pc - 2 - la) & 3) ? pc - 2 : pc - 4;
}

static void show_pc(void *pc)
{
	int i = 10;

	panic_printf("PC:\n");
	while (i-- > 0)
		pc = nds32_prev_pc(pc);

	while (i++ < 10) {
		unsigned instr = nds32_fetch_instruction(pc);
		if (nds32_instr_len(instr) == 2)
			panic_printf("    %p:\t%04x\n", pc, (uint16_t) instr & 0xffff);
		else
			panic_printf("    %p:\t%08x\n", pc, instr);
		pc += nds32_instr_len(instr);
	}
}

struct nds32_unwind_info {
	uint32_t *fp;
	uint32_t *sp;
	uint32_t *pc;
	uint32_t *lp;
};

/**
 * struct nds32_irq_frame
 *
 * NDS32 FreeRTOS IRQ entry pushes registers differently
 * compared to when it does its context save/restore, hence
 * need to have a dedicated frame structure for it.
 */
struct nds32_irq_frame {
#if defined(__TARGET_IFC_EXT) && defined(__TARGET_ZOL_EXT)
	uint32_t dummy;
#endif
	void *pc;
	uint32_t ipsw;
#ifdef __TARGET_ZOL_EXT
	uint32_t lc;
	uint32_t le;
	uint32_t lb;
#endif
#ifdef __TARGET_IFC_EXT
	uint32_t ifc_lp;
#endif
	uint32_t r[25 + 6]; /* r6-r30, r0-r5 */
};

static inline int ts6260_pc_is_irq_entry(void *pc)
{
	extern char OS_Int_Vectors, OS_Int_Vectors_End;

	return (pc >= (void *) &OS_Int_Vectors &&
		pc < (void *) &OS_Int_Vectors_End);
}
#if ICACHE_ENABLE
extern char _start[], __etext[], __stext1[], __etext1[];
static inline int ts6260_pc_is_valid(void *pc)
{
	/* FIXME: ROM */
	return ((pc >= (void *) _start && pc < (void *) __etext) ||
		(pc >= (void *) __stext1 && pc < (void *) __etext1));
}
#else
extern char _start[], __etext[];
static inline int ts6260_pc_is_valid(void *pc)
{
	/* FIXME: ROM */
	return ((pc >= (void *) _start && pc < (void *) __etext));
}
#endif
#define sp_is_8byte_aligned(sp) ((((unsigned) (sp)) & 0x7) == 0)

/**
 * nds32_prepare_unwind() - prepare for unwinding
 *
 * @tsk: task whose stack is being unwound
 * @i: frame number
 * @f: [0]: current, [1]: caller
 *
 * Returns:
 *  -1: bad frame. Give up.
 *   0: preparation (including fixup) successful. Proceed unwind.
 *   1: Unwind done inside this function, skip unwind.
 */
static int nds32_prepare_unwind(TaskStatus_t *tsk, int i,
				struct nds32_unwind_info f[2])
{
	uint32_t fault = __nds32__mfsr(NDS32_SR_ITYPE) & 0xf;
	int ret = 0, mem = 1;

	if (!ts6260_pc_is_valid(f[0].pc)) {
		ret = -1;
		if (i == 0 &&
		    (nds32_is_instruction_exception(fault) ||
		     fault == RESERVED_INST)) {
			/*
			 * We unwind just by making f[1].pc = f[0].lp, yet
			 * maintain the sp and lp hoping that the exception
			 * took place as soon as the caller jumped into f[0].pc.
			 */
			f[1].pc = nds32_prev_pc(f[0].lp);
			f[1].lp = NULL;
			f[1].sp = f[0].sp;
			f[1].fp = f[0].fp;
			ret = 1;
		}
	} else if (ts6260_pc_is_irq_entry(f[0].pc)) {
		struct nds32_irq_frame *iframe = (void *) f[0].sp;

		/*
		 * f[0] is a frame established by OS_Trap_Int_Comm,
		 * which cannot be unwound normally. We adjust and
		 * unwind this frame here, and skip the normal unwind
		 * step.
		 *
		 *   |----------------------------|<-- f[0].fp, f[1].fp
		 *   |                            |
		 *   |     interrupted frame      | #(i+1): interrupted
		 *   |                            |         function
		 *   |                            |
		 *   |----------------------------|<-- f[0].fp (corrected)
		 *   | {$r0-$r6}                  |
		 *   | {$r15-$p1, $fp, $gp, $lp}  |
		 *   | $ifc_ip (optional)         | #i: OS_Trap_Int_Comm()
		 *   | $lc, $le, $lb (optional)   |
		 *   | $pc, $psw, dummy           |
		 *   |----------------------------|<-- f[0].sp (=iframe)
		 *   |                            |
		 *   |                            | #(i-1) irq_handler()
		 *
		 * iframe->pc: interrupted pc (=> f[1].pc)
		 * iframe->fp: frame pointer of interrupted function (=> f[1].fp)
		 */
		f[0].lp = iframe->pc;
		f[0].fp = (void *) f[0].sp + sizeof(*iframe);

		f[1].pc = f[0].lp;
		f[1].lp = NULL;
		f[1].sp = f[0].fp;
		f[1].fp = (void *) iframe->r[28 - 15]; /* fp */
		ret = 1;
	} else if (!sp_is_8byte_aligned(f[0].fp) ||
		   !sp_is_8byte_aligned(f[0].sp)) {
		panic_printf("fp(%p)/sp(%p) is not 8-byte aligned\n", f[0].fp, f[0].sp);
		mem = 0;
		ret = -1;
	} else if (f[0].fp < f[0].sp) {
		panic_printf("fp(%p) < sp(%p)\n", f[0].fp, f[0].sp);
		mem = 0;
		ret = -1;
	} else if ((void *) f[0].sp < (void *) tsk->pxStackBase) {
		/* Stack overflow */
	}

	panic_printf("#%-2d [<%p>] sp=%p, fp=%p\n",
	       i++, f[0].pc, f[0].sp, f[0].fp);
	if (mem)
		show_stack(tsk, f[0].sp, f[0].fp);
	return ret;
}

/**
 * nds32_unwind_frame() - unwind the current frame
 * @f: array of frames ([0]: current, [1]: caller)
 *
 * This function examines the current frame (function text and stack)
 * to find out where in the stack the caller's fp (f[1].fp) and
 * pc (f[0].lp) are saved which are necessary to unwind the caller's stack.
 *
 * Assumption is that f[0].pc, f[0].fp, and f[0].sp are correct.
 *
 * The function exhaustly scans backward from f[0].pc to the
 * start of the function, until it finds instructions that pushes
 * caller's fp and lp. Therefore, if either of f[0].pc and f[0].fp
 * is not reliable, do not call this function.
 *
 * Return value:
 * -1 = unwind fails
 *  0 = unwind succeeds
 *  1 = unwind succeeds, but requires post fixup.
 */
static int nds32_unwind_frame(struct nds32_unwind_info f[2])
{
	void *pc = f[0].pc;
	uint32_t instr = nds32_fetch_instruction(pc);

	memset(&f[1], 0, sizeof(f[1]));

	/*
	 * Scan backward the instructions in the current function
	 * until we find addi $fp, $sp, #num, which sets f[0].fp.
	 */
	while (!nds32_instr_is_prologue(instr)) {
		if (nds32_instr_is_addi_fp_sp(instr))
			goto addi_fp_found;
		pc = nds32_prev_pc(pc);
		instr = nds32_fetch_instruction(pc);
	}
	return -1;

 addi_fp_found:
	/*
	 * Find the smw or push25 instruction right ahead of the addi instruction
	 * found above, and figure out where $fp and $lp registers are located
	 * in the f[0].fp.
	 */
	pc = nds32_prev_pc(pc);
	instr = nds32_fetch_instruction(pc);

	if (nds32_instr_is_smw(instr)) {
		/*
		 * SMW - examine enable4 field (instr[9:6])
		 * ENABLE4: 3   2   1   0
		 *          fp  gp  lr  sp
		 */
		uint32_t enable4 = bf_get(instr, 9, 6);
		int i, offset = -1;

		for (i = 0; i < 4; i++) {
			if (enable4 & BIT(i)) {
				if (i == 1)
					f[0].lp = (void *) f[0].fp[offset];
				else if (i == 3)
					f[1].fp = (uint32_t * ) f[0].fp[offset];
				offset--;
			}
		}
	} else if (nds32_instr_is_push25(instr)) {
		/*
		 * PUSH25: pushes {$fp, $gp, $lr} at the top of the stack
		 */
		f[0].lp = (void *) f[0].fp[-1];
		f[1].fp = (void *) f[0].fp[-3];
	} else
		return -1;

	f[1].pc = nds32_prev_pc(f[0].lp); /* which was found here */
	f[1].sp = f[0].fp;
	if (f[1].pc && f[1].fp) {
		return 0;
	}
	return -1;
}

static int nds32_finalize_unwind(TaskStatus_t *tsk, int i,
				 struct nds32_unwind_info f[2])
{
	if (f[1].pc == NULL || f[1].fp == NULL)
		return -1;

	if ((((unsigned) f[0].fp) & 7) || (((unsigned) f[0].sp) & 7)) {
		panic_printf("fp(%p)/sp(%p) is not 8-byte aligned\n", f[0].fp, f[0].sp);
		return -1;
	} else if (f[0].fp < f[0].sp) {
		panic_printf("fp(%p) < sp(%p)\n", f[0].fp, f[0].sp);
		return -1;
	} else if (f[1].fp == f[0].fp) {
		/* Stack is not unwound */
		if (i == 0 && f[1].pc != f[0].pc)
			return 0; /* probably it was the leaf frame - let it go further */
		return -1;
	}

	return 0;
}

#define FPEND ((void *) 0x28282828)

/**
 * backtrace() - trace back the stack frames
 * @pt: register context
 *
 * Limitations:
 * - Unable to go beyond interrupt handler
 * - Unable to trace back library functions (built without -fno-omit-frame-pointer
 *
 * TODO:
 * - mark the overflowing frame if possible (sp < tsk->stackbase)
 */
void backtrace(TaskStatus_t *tsk, struct pt_regs *pt)
{
	struct nds32_unwind_info frame[2] = {
		[0] = {
			.pc = (void *) pt->pc,
			.sp = (void *) pt->sp,
			.lp = (void *) pt->lp,
			.fp = (void *) pt->fp,
		},
	};
	int i = 0, ret;

	panic_printf("Backtrace:\n");
	do {
		if ((ret = nds32_prepare_unwind(tsk, i, frame)) < 0)
			break;
		if (ret != 1)
			ret = nds32_unwind_frame(frame);
		if (ret < 0)
			ret = nds32_finalize_unwind(tsk, i, frame);
		if (ret < 0) {
			panic_printf("Backtrace stopped\n");
			break;
		}

		frame[0] = frame[1];
	} while (i++ < 10 && frame[0].fp != FPEND);

	show_pc((void *) pt->pc);
}
#else
void backtrace(TaskStatus_t *tsk, struct pt_regs *pt)
{
	return;
}
#endif

struct pt_regs *saved_pt;
uint32_t g_itype=0;
uint32_t g_faddr =0;
uint32_t g_inst =0;
uint32_t g_overflow_type=0;

void uart_dump_mem(const unsigned char * start, unsigned int len)
{
    int i = 0;
    int fifo_count = 0;
    while(len)
    {
        while(!(IN32(UART0_BASE + 0x34) & 0x20));
       	fifo_count = 16;
        while (--fifo_count > 0) {
            OUT32(UART0_BASE + 0x20, (unsigned char)start[i++]);
            if(--len == 0)
                 break;
        }
    }
    while(!(IN32(UART0_BASE + 0x34) & 0x40));
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-address"
void *return_addr[8];
void dump_stack()
{

        return_addr[0]=__builtin_return_address(0);
        return_addr[1]=__builtin_return_address(1);
        return_addr[2]=__builtin_return_address(2);
        return_addr[3]=__builtin_return_address(3);
        return_addr[4]=__builtin_return_address(4);
        return_addr[5]=__builtin_return_address(5);
        return_addr[6]=__builtin_return_address(6);
        return_addr[7]=__builtin_return_address(7);
		system_printf("dump stack:%x,%x,%x,%x,%x,%x,%x,%x\r\n\r\n",return_addr[0],return_addr[1],return_addr[2],return_addr[3],return_addr[4],return_addr[5],return_addr[6],return_addr[7]);	
}  
#pragma GCC diagnostic pop

#define CMD_LEN 32
void mem_dump()
{
	unsigned int regbase = UART0_BASE;
	unsigned int num=0;
	char cmd[CMD_LEN];

	while (1)
	{
		while(uart_data_tstc(regbase)) 
		{
			cmd[num]=uart_data_getc(regbase);	
			if(num==CMD_LEN-2)
			{
				num=num+1;
				cmd[num]='\r';
			}
			if(cmd[num]=='\r')
			{
				cmd[num]=0;
				if(strstr(cmd,"ramdump"))
				{
					//panic_printf("dump Reg XXX from 0x%x ,len 0x%x\n",0x0,0);

					panic_printf("dump ILM from 0x%x ,len 0x%x\n",0x10000,0x14000);
					//0x010000-0x023FFF 0x14000 80K
					//0x200000-0x223FFF	0x24000 144K
					//0x240000-0x254000 0x14000 80K
					uart_dump_mem((const unsigned char *)0x10000,0x14000);
					panic_printf("dump DLM from 0x%x ,len 0x%x\n",0x200000,0x24000);
					uart_dump_mem((const unsigned char *)0x200000,0x24000);
					panic_printf("dump IRAM from 0x%x ,len 0x%x\n",0x240000,0x14000);
					uart_dump_mem((const unsigned char *)0x240000,0x14000);
				}
				else if(strstr(cmd,"reboot"))
				{
					wdt_reset_chip(0);	
				}
				num=0;
			}
			else
				num++;
		}
	}
}
void cpu_exception_print(uint32_t itype, uint32_t faddr, uint32_t inst, struct pt_regs *regs)
{
	panic_printf("\n");
	panic_printf("------------------------- [cut here] -------------------------\n");
	switch (itype & 0xf) {
	case ALIGNMENT:
		panic_printf("Unaligned access (%c) at pc=[<%08x>] addr=[<%08x>]\n",
		       (inst)? 'I' : 'D', regs->pc, faddr);
		break;
	case NONEXISTENT_MEM_ADDR:
		panic_printf("Invalid memory access (%c) at pc=[<%08x>] target=[<%08x>]?\n",
		       (inst)? 'I' : 'D', regs->pc, faddr);
		break;
	case PRECISE_BUS_ERROR:
	case IMPRECISE_BUS_ERROR:
		panic_printf("%s bus error (%c) at pc=[<%08x>] addr=[<%08x>]\n",
		       ((itype & 0xf) == PRECISE_BUS_ERROR) ? "" : "Imprecise",
		       inst ? 'I' : 'D', regs->pc, faddr);
		break;
	case ARITHMETIC:
		panic_printf("%s at pc=[<%08x>]\n",
		       (bf_get(itype, 19, 16) == 1) ?
		       "Division by zero" : "Integer overflow",
		       regs->pc);
		break;
	default:
		panic_printf("%s at pc=[<%08x>]\n", nds32_exception_name[itype & 0xf],
		       regs->pc);
		break;
	}

}


extern HeapRegion_t sysHeap[] ;
unsigned int g_flashdump_enable = 0;
unsigned int g_flash_addr_forsave = 0;
unsigned int g_flash_len_forsave = 0;
#define FLASH_DUMPOP_WRITE 0
#define FLASH_DUMPOP_READ  1
static int flash_rw_operation(char*data,int datalen,int op_type)
{
	datalen=min(g_flash_len_forsave,datalen);
	if(datalen)
	{
		if(FLASH_DUMPOP_WRITE==op_type)
		{
			hal_spifiash_write(g_flash_addr_forsave, (unsigned char *)data,datalen);
		}
		else if(FLASH_DUMPOP_READ==op_type)
		{
			hal_spiflash_read(g_flash_addr_forsave, (unsigned char *)data,datalen);
		}
		g_flash_addr_forsave+=datalen;
		g_flash_len_forsave-=datalen;
	}
	return datalen;

}
void flash_dump(struct pt_regs *pt,TaskStatus_t *tsk)
{
	char taskname[32] = {0};
	uint32_t tasknamelen=strlen(tsk->pcTaskName);
	uint8_t reglen=sizeof(struct pt_regs);
	uint8_t u32len=sizeof(uint32_t);
	uint32_t * base_addr = ( uint32_t * ) tsk->pxStackBase;
	uint32_t stacklen=0;
	if(0==g_flashdump_enable)
	{
		return ;
	}
	partion_info_get(PARTION_NAME_DATA_OTA, &g_flash_addr_forsave, &g_flash_len_forsave); 

	if(g_flash_addr_forsave)
	{
		hal_spiflash_erase(g_flash_addr_forsave,g_flash_len_forsave);

		flash_rw_operation((char *)&tasknamelen,u32len,FLASH_DUMPOP_WRITE);
		tasknamelen = min(tasknamelen, sizeof(taskname) - 1);
		snprintf(taskname, sizeof(taskname), tsk->pcTaskName);
		flash_rw_operation(taskname, tasknamelen,  FLASH_DUMPOP_WRITE);
		flash_rw_operation((char *)pt, reglen, FLASH_DUMPOP_WRITE);
		flash_rw_operation((char *)&g_itype, u32len, FLASH_DUMPOP_WRITE);
		flash_rw_operation((char *)&g_faddr, u32len, FLASH_DUMPOP_WRITE);
		flash_rw_operation((char *)&g_inst, u32len, FLASH_DUMPOP_WRITE);
		flash_rw_operation((char *)&g_overflow_type, u32len, FLASH_DUMPOP_WRITE);
		flash_rw_operation((char *)0x200000, 0x24000, FLASH_DUMPOP_WRITE);
		flash_rw_operation((char *)sysHeap[1].pucStartAddress, sysHeap[1].xSizeInBytes, FLASH_DUMPOP_WRITE);
	}

}


void flash_dump_print()
{
	uint32_t tasknamelen=0;
	char taskname[32]={0};
	struct pt_regs regs={0};
	uint32_t itype_temp;
	uint32_t faddr_temp;
	uint32_t inst_temp;	
	uint32_t overflow;
	uint32_t * base_addr=0;
	uint32_t stacklen=0;
	uint8_t reglen=sizeof(struct pt_regs);
	uint8_t u32len=sizeof(uint32_t);
	partion_info_get(PARTION_NAME_DATA_OTA, &g_flash_addr_forsave, &g_flash_len_forsave); 

	flash_rw_operation((char *)&tasknamelen, u32len, FLASH_DUMPOP_READ);
	tasknamelen=min(tasknamelen,sizeof(taskname)-1);
	flash_rw_operation(taskname, tasknamelen, FLASH_DUMPOP_READ);
	flash_rw_operation((char *)&regs, reglen, FLASH_DUMPOP_READ);
	flash_rw_operation((char *)&itype_temp, u32len, FLASH_DUMPOP_READ);
	flash_rw_operation((char *)&faddr_temp, u32len, FLASH_DUMPOP_READ);
	flash_rw_operation((char *)&inst_temp, u32len, FLASH_DUMPOP_READ);
	flash_rw_operation((char *)&overflow, u32len, FLASH_DUMPOP_READ);

	/*read_dump_flash(&base_addr,u32len);
	
	read_dump_flash(&stacklen,u32len);*/
	panic_printf_type = 1;
	system_printf("\r\n-------------------------------------------------------------------------------\r\n");
	cpu_exception_print(itype_temp,faddr_temp,inst_temp,&regs);
	panic_printf("***Process: %s overflow type is %d***\n",taskname,overflow);
	show_regs(&regs);
	//panic_printf("stack base is 0x%x, stack len is 0x%x\n",base_addr, stacklen);
	
}



void nds32_die(struct pt_regs *pt)
{
	TaskStatus_t tsk;

	vTaskGetTaskInfo(NULL, &tsk, pdFALSE, eRunning);
	show_task(&tsk);
	show_regs(pt);
	//backtrace(&tsk, pt);
	/*show_mem("PC", "", (void *) pt->pc-32, 64, 2, NULL);*/
#ifdef TUYA_SDK_ADPT
    int rst_reason;
    rst_reason = TY_RST_FATAL_EXCEPTION;
    ef_set_env_blob(NV_STARTUP_TYPE, &rst_reason, sizeof(int));
#endif
	panic_printf("System halted...\n");
	saved_pt=pt;


	flash_dump(pt,&tsk);
	mem_dump();

}


void handle_general_exception(struct pt_regs *regs)
{
	g_itype = __nds32__mfsr(NDS32_SR_ITYPE);
	g_faddr  = __nds32__mfsr(NDS32_SR_EVA);
	g_inst  = nds32_is_instruction_exception(g_itype);

	// if (kernel_enter_panic() > 1) {
	// 	while (1);
	// }
	panic_printf_type = 0;
	cpu_exception_print(g_itype,g_faddr,g_inst,regs);


	nds32_die(regs);
#undef E
}

extern void rom_uart_set_lineControl(unsigned int regBase, 
			unsigned int databits,	/* 0--5bits, 1--6bits, 2--7bits, 3--8bits */
			unsigned int parity,	/* 0--no parity, 1--odd parity, 2--even parity*/
			unsigned int stopbits,	/* 0--1bit stopbits, 1--the num of stopbits is based on the databits*/
			unsigned int bc		/* break control */);
extern void rom_uart_set_fifoControl(unsigned int regBase, 
						unsigned int tFifoRst, 
						unsigned int rFifoRst,
						unsigned int fifoEn);

void __attribute__((naked)) OS_Trap_Machine_Error(void)
{

		asm("la	$r0, OS_Trap_General_Exception");
		asm("jr5	$r0");
	
}


void __attribute__((naked)) OS_Trap_General_Exception(void)
{
	/* r0 - r5 are stashed by the exception entry */
	CLK_DISABLE(CLK_UART0);
	CLK_ENABLE(CLK_UART0);

	//asm("addi	$r0, $sp, #4*6");
	//asm("push	$r0");
	//asm("pushm 	$r6, $r30");
#ifdef __TARGET_IFC_EXT
	asm("mfusr	$r0, $ifc_lp");
	asm("push	$r0");
#endif

#ifdef __TARGET_ZOL_EXT
	//asm("mfusr	$r0, $lb");
	//asm("mfusr	$r1, $le");
	//asm("mfusr	$r2, $lc");
	asm("pushm	$r0, $r2");
#endif
	asm("mfsr 	$r0, $ipc");
	asm("mfsr	$r1, $ipsw");
	asm("pushm	$r0, $r1");

	handle_general_exception((void *) __nds32__get_current_sp());

	asm("addi $sp, $sp ,#4*6");
	asm("popm $r1, $r31");
	asm("iret");
}

void nds32_heap_overflow(TaskHandle_t xTask, int type)
{
	uint32_t itype;
	int i;
	void *sp= (void*)(*(int*)xTask);
	struct pt_regs pt;
	
	struct nds32_irq_frame * oregs=NULL;

	itype = (__nds32__mfsr(NDS32_SR_ITYPE) & 0xf) | NEXT_PRECISE_STACK_OVERFLOW;
	__nds32__mtsr(itype, NDS32_SR_ITYPE);

	g_overflow_type=type;

	oregs=(struct nds32_irq_frame *)sp;
	pt.sp = (uint32_t)sp;
	pt.pc = (uint32_t)oregs->pc;
	pt.ipsw=oregs->ipsw;
#ifdef __TARGET_ZOL_EXT
	pt.lc=oregs->lc;
	pt.le=oregs->le;
	pt.lb=oregs->lb;
#endif
#ifdef __TARGET_IFC_EXT
	pt.ifc_lp=oregs->ifc_lp;
#endif
	for(i=0;i<31;i++)
	{
		pt.r[(i+6)%31]=oregs->r[i];
	}

	handle_general_exception(&pt);
}

static int nds32_register_panic(void)
{
	// panic = nds32_panic;
	return 0;
}
// __initcall__(arch, nds32_register_panic);


#include <stdlib.h>


static int do_trap(int argc, char *argv[])
{
// 	int ret;

// 	if (argc == 1)
// 		return 0;

// 	if (strcmp(argv[1], "branch") == 0) {
// 		asm("jr %0" :: "r" (0x1));
// 	} else if (strcmp(argv[1], "align") == 0) {
// 		__asm__ __volatile__("lwi %0, [%1]"
// 				     : "=r" (ret)
// 				     : "r" (0x1));
// 	} else if (strcmp(argv[1], "stack") == 0) {
// #if 0
// 		volatile char buf[1024 * 8];
// 		int i;

// 		for (i = sizeof(buf) - 1; i >= 0; i--) {
// 			buf[i] = i & 255;
// 		}
// 		while (1);
// #endif
// 	} else if (strcmp(argv[1], "0") == 0) {
// 		volatile int x = 128;

// 		x = x / atoi(argv[1]);
// 	} else if (strcmp(argv[1], "assert") == 0) {
// 		assert(0);
// 	}

	return 0;
}

ENTRY(_start)
SECTIONS
{
	. = 0x00010000;
	.text	:
	{
		KEEP(*(.nds32_init ))
		../lib/lib_a-w_log10.o (.text .text.*)
		../lib/lib_a-rand.o  (.text .text.*)
		../lib/lib_a-data-rand.o (.text .text.*)
		../lib/lib_a-reent_errno.o (.text .text.*)
		../lib/lib_a-andes_dpexlog.o (.text .text.*)
		../lib/lib_a-ctype_.o (.text .text.*)
		../lib/_div_sf.o (.text .text.*)
		../lib/_sf_to_df.o (.text .text.*)
		../lib/_umoddi3.o (.text .text.*)
		../lib/_si_to_sf.o (.text .text.*)
		../lib/lib_a-s_ceil.o (.text .text.*)
		../lib/lib_a-s_round.o (.text .text.*)
		../lib/lib_a-errno.o (.text .text.*)
		../lib/lib_a-strerror.o (.text .text.*)
		../lib/lib_a-impure.o (.text .text.*)
		../lib/lib_a-u_strerr.o (.text .text.*)
		../lib/lib_a-w_pow.o (.text .text.*)
		../lib/lib_a-andes_dpexexp.o (.text .text.*)
		../lib/_mul_sf.o (.text .text.*)
		../lib/_addsub_sf.o (.text .text.*)
		../lib/_df_to_sf.o (.text .text.*)
		../lib/lib_a-s_scalbn.o (.text .text.*)
		../lib/lib_a-sf_scalbn.o (.text .text.*)
		../lib/lib_a-s_copysign.o (.text .text.*)
		../lib/lib_a-abs.o (.text .text.*)
		../lib/lib_a-w_sqrt.o (.text .text.*)
		../lib/lib_a-andes_dpexsqrt.o (.text .text.*)
		
		../out/tr6260/sig/obj_sig_tr6260/start.S.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/pcu.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/pit.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_adc.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_efuse.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_uart.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_trim.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/heap_5.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/irq.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/util_cli_freertos.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/timers.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/wdt.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_gpio.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/bsp.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/bsp_ext.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/os_cpu_a.S.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/queue.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/tasks.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_spiflash.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_trng.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/port.c.o (.text .text.*)
		
		PROVIDE (__etext = .);
	}

	. = 0x00200000;
	.data 	: AT(ALIGN(LOADADDR (.text) + SIZEOF (.text), ALIGNOF(.data)))
	{
		__rw_lma_start = LOADADDR (.data);
		__rw_vma_start = ADDR(.data);
		*(.data .data.*)
		. = ALIGN(8);
		__rw_lma_end =  LOADADDR (.bss);
	}

	.bss 	: 
	{
		__bss_start = .;
		*(.bss .bss.* ) 
		*(COMMON )
		*(.scommon_w .scommon_w.* )
		. = ALIGN(8);
		_end = .;
		PROVIDE (_heap1_start = .);
	}


	. = 0x00240000;
	.buffer (NOLOAD) :
	{
		PROVIDE (__BUF_START = .);
		. += 0x200;					/* The reserved space for psm */
		KEEP(*(.buffer))
		PROVIDE (__BUF_END = .);
	}

	. = 0x40C04010;
	.xip 	:
	{
		__stext1 = .;
        *(.xip ) 
		*(.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_lmac_common.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_lmac_11n.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_lmac_debug.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_lmac_downlink.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_lmac_ps_common.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_lmac_ps_dummy.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_lmac_queue_manager.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_lmac_test.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_lmac_util.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_phy_tr6260.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/hal_rf_tr6260.c.o (.text .text.*)
		../out/tr6260/sig/obj_sig_tr6260/drv_lmac.c.o (.text .text.*)

		__etext1 = .;
	    *(.rodata .rodata.* ) 
		KEEP(*(SORT(.nrc_cmd* )))
    }

	PROVIDE (_stack = 0x00223ff8);
	PROVIDE (_stack_size = 0x80);

	.stab	0 : { *(.stab) }
	.stabstr	0 : { *(.stabstr) }
	.stab.excl	0 : { *(.stab.excl) }
	.stab.exclstr	0 : { *(.stab.exclstr) }
	.stab.index	0 : { *(.stab.index) }
	.stab.indexstr	0 : { *(.stab.indexstr) }
	.note.nds32	0 : { *(.note.nds32) *(.note.nds32.*) }
	.comment	0 : { *(.comment) }
	.debug	0 : { *(.debug) }
	.line	0 : { *(.line) }
	.debug_srcinfo	0 : { *(.debug_srcinfo) }
	.debug_sfnames	0 : { *(.debug_sfnames) }
	.debug_aranges	0 : { *(.debug_aranges) }
	.debug_pubnames	0 : { *(.debug_pubnames) }
	.debug_info	0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev	0 : { *(.debug_abbrev) }
	.debug_line	0 : { *(.debug_line) }
	.debug_frame	0 : { *(.debug_frame) }
	.debug_str	0 : { *(.debug_str) }
	.debug_loc	0 : { *(.debug_loc) }
	.debug_macinfo	0 : { *(.debug_macinfo) }
	.debug_weaknames	0 : { *(.debug_weaknames) }
	.debug_funcnames	0 : { *(.debug_funcnames) }
	.debug_typenames	0 : { *(.debug_typenames) }
	.debug_varnames	0 : { *(.debug_varnames) }
}

INCLUDE "../scripts/ld/nds_rom_common.ld"


ASSERT(((__rw_lma_end - 0x10000) < 0xC000) , "Error: No enough roon left in IRAM");
ASSERT(((_end + _stack_size) < _stack) , "Error: No enough room left in DRAM");
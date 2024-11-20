ENTRY(_start)
SECTIONS
{
	. = 0x00010000;
	.text	:
	{
		KEEP(*(.nds32_init ))
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/start.S.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/os_cpu_a.S.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/bsp.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/bsp_ext.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/irq.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/pit.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/wdt.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/pcu.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/panic.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_uart.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_spiflash.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_rtc.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_i2c.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_gpio.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_pwm.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_aes.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_efuse.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_adc.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_trng.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_timer.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_spi_master.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/hal_spi_slave.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/dma_control.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/portISR.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/port.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/tasks.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/queue.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/heap_5.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/timers.c.o (.text .text.*)
		../out/tr6260s1/standalone_tuya/obj_standalone_tr6260s1_tuya/event_groups.c.o (.text .text.*)
		../lib/libpsm.a (.text .text.*)
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

	. = 0x40C07010;
	.xip 	:
	{
		__stext1 = .;
        *(.xip ) 
		*(.text .text.*)
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

PROVIDE ( memcmp = 0x00001000 );
PROVIDE ( memcpy = 0x00001038 );
PROVIDE ( memmove = 0x00001064 );
PROVIDE ( memset = 0x00001114 );

PROVIDE ( strcpy = 0x00001144 );
PROVIDE ( strlcpy = 0x00001158 );
PROVIDE ( strcmp = 0x00001184 );
PROVIDE ( strncmp = 0x000011a0 );
PROVIDE ( strlen = 0x000011b8 );
PROVIDE ( strnlen = 0x000011c4 );
PROVIDE ( strstr = 0x000011d8 );
PROVIDE ( strchr = 0x0000120c );
PROVIDE ( strrchr = 0x0000121c );
PROVIDE ( strtok_r = 0x0000127e );
PROVIDE ( strtod = 0x00001288 );

PROVIDE ( skip_atoi = 0x000014ee );
PROVIDE ( _tolower = 0x000015ca );
PROVIDE ( strcasecmp = 0x000015de );
PROVIDE ( strtol = 0x0000160a );

PROVIDE ( vsnprintf = 0x00001e44 );
PROVIDE ( snprintf = 0x00001e4c );
PROVIDE ( vsprintf = 0x00001e68 );
PROVIDE ( sprintf = 0x00001e80 );

PROVIDE ( skip_spaces = 0x00001e96 );

PROVIDE ( simple_strtoull = 0x00001f30 );
PROVIDE ( simple_strtoul = 0x00001f54 );
PROVIDE ( strtoul = 0x00001f5a );
PROVIDE ( simple_strtoll = 0x00001f60 );

PROVIDE ( vsscanf = 0x00001f7e );
PROVIDE ( sscanf = 0x000022b2 );

PROVIDE ( _ctype = 0x000022e0 );

PROVIDE ( qsort = 0x00002458 );

PROVIDE ( atoi = 0x0000269c );
PROVIDE ( atof = 0x000026a4 );

PROVIDE ( nds_isspace = 0x000026ac );

PROVIDE ( __fixunsdfsi = 0x000026bc );
PROVIDE ( __fixdfsi = 0x00002708 );
PROVIDE ( __floatundidf = 0x00002758 );
PROVIDE ( __floatunsidf = 0x000027cc );
PROVIDE ( __ashldi3 = 0x00002804 );
PROVIDE ( __lshrdi3 = 0x0000282a );

PROVIDE ( __gtdf2 = 0x00002854 );
PROVIDE ( __gedf2 = 0x00002854 );
PROVIDE ( __eqdf2 = 0x00002858 );
PROVIDE ( __nedf2 = 0x00002858 );
PROVIDE ( __ltdf2 = 0x00002858 );
PROVIDE ( __cmpdf2 = 0x00002858 );
PROVIDE ( __ledf2 = 0x00002858 );

PROVIDE ( __floatsidf = 0x000028c0 );

PROVIDE ( __subdf3 = 0x0000290c );
PROVIDE ( __adddf3 = 0x00002910 );
PROVIDE ( __divdf3 = 0x00002aec );
PROVIDE ( __muldf3 = 0x00002db8 );
PROVIDE ( __udivdi3 = 0x00002f98 );
PROVIDE ( __udivmoddi4 = 0x00002ff4 );
PROVIDE ( __divdi3 = 0x0000314c );

PROVIDE ( _nds32_init_cpu = 0x0000318c );
PROVIDE ( _nds32_init_mem = 0x000031c4 );
PROVIDE ( _nds32_init_mmu = 0x000031d4 );
PROVIDE ( _nds32_enable_icache = 0x000031f8 );
PROVIDE ( _nds32_enable_dcache = 0x00003258 );

PROVIDE ( uart_get_intType = 0x00003282 );
/* PROVIDE ( uart_set_baudrate = 0x000032a8 ); */
PROVIDE ( uart_set_lineControl = 0x000032ee );
PROVIDE ( uart_set_fifoControl = 0x00003332 );
PROVIDE ( uart_set_intEnable = 0x00003344 );
PROVIDE ( uart_data_write = 0x00003352 );
PROVIDE ( uart_data_tstc = 0x0000337e );
PROVIDE ( uart_data_getc = 0x00003386 );

PROVIDE ( spi_bus_ready = 0x0000338e );
PROVIDE ( spi_fifo_reset = 0x000033aa );
PROVIDE ( spi_rx_ready = 0x000033d4);
PROVIDE ( spiFlash_rdid = 0x000033f2);
PROVIDE ( spiFlash_rdsr = 0x00003426 );
PROVIDE ( spiFlash_wren = 0x00003458 );
PROVIDE ( spiFlash_se = 0x00003482 );
PROVIDE ( spiFlash_init = 0x000034b2 );

PROVIDE ( pit_ch_count_get = 0x00003550 );
PROVIDE ( pit_ch_reload_value = 0x00003564 );
PROVIDE ( pit_ch_ctrl = 0x0000357a);
PROVIDE ( pit_ch_mode_enable = 0x000035b0);
PROVIDE ( pit_int_status_handle = 0x000035ca );
PROVIDE ( pit_int_enable = 0x000035f4 );
PROVIDE ( pit_init = 0x0000361c );
PROVIDE ( pit_delay = 0x0000366a );


PROVIDE ( ulPortSetInterruptMask = 0x00003692 );
PROVIDE ( vPortClearInterruptMask = 0x000036a0 );
PROVIDE ( vPortYield = 0x000036ae );
PROVIDE ( pxPortInitialiseStack_rom = 0x000036bc );

PROVIDE ( vListInitialise = 0x00003730 );
PROVIDE ( vListInitialiseItem = 0x00003744 );
PROVIDE ( vListInsertEnd = 0x0000374a );
PROVIDE ( vListInsert = 0x0000375c );
PROVIDE ( uxListRemove = 0x0000378c );

PROVIDE ( crc32 = 0x0000381e );

PROVIDE ( rijndaelKeySetupEnc = 0x00003c8c );
PROVIDE ( rcons = 0x00003f34 );
PROVIDE ( Td4s = 0x00003f40 );
PROVIDE ( Td0 = 0x00004040 );
PROVIDE ( Te0 = 0x00004440 );

PROVIDE ( rijndaelKeySetupDec = 0x00004840 );
PROVIDE ( rijndaelDecrypt = 0x000048f4 );
PROVIDE ( aes_decrypt = 0x00004c84 );

PROVIDE ( SHA1Transform = 0x00004c98 );
PROVIDE ( SHA1Init = 0x00005aa0 );
PROVIDE ( SHA1Update = 0x00005aba );
PROVIDE ( SHA1Final = 0x00005b38 );
PROVIDE ( sha1_vector = 0x00005bca );
PROVIDE ( hmac_sha1_vector = 0x00005bf6 );
PROVIDE ( hmac_sha1 = 0x00005c8e );
PROVIDE ( sha1_prf = 0x00005c96 );
PROVIDE ( pbkdf2_sha1 = 0x00005d18 );

PROVIDE ( rc4_skip = 0x00005ddc );

PROVIDE ( hmac_md5_vector = 0x00005e7a );
PROVIDE ( hmac_md5 = 0x00005f6e );
PROVIDE ( MD5Transform = 0x00005f86 );
PROVIDE ( MD5Init = 0x00006664 );
PROVIDE ( MD5Update = 0x00006696 );
PROVIDE ( MD5Final = 0x00006726 );
PROVIDE ( md5_vector = 0x0000679a );

PROVIDE ( ip4_addr_netmask_valid = 0x000067d4 );
/*PROVIDE ( ip4addr_aton = 0x00006802 ); */
PROVIDE ( ipaddr_addr = 0x0000691a );
PROVIDE ( ip4addr_ntoa_r = 0x0000692c );
PROVIDE ( ip_addr_broadcast = 0x000069a4 );
PROVIDE ( ip_addr_any = 0x000069a8 );

PROVIDE ( lwip_htons = 0x000069ac );
PROVIDE ( lwip_htonl = 0x000069b8 );

PROVIDE ( lwip_standard_chksum = 0x000069c2 );
PROVIDE ( inet_cksum_pseudo_base = 0x000069fa );
PROVIDE ( inet_chksum_pseudo = 0x00006a56 );
PROVIDE ( inet_chksum = 0x00006a7c );
PROVIDE ( inet_chksum_pbuf = 0x00006a86 );

PROVIDE ( __gf_exadd = 0x00006ae2 );
PROVIDE ( __gf_exdiv = 0x00006b86 );
PROVIDE ( __gf_exmul = 0x00006d60 );
PROVIDE ( __gf_sigmax = 0x00006e06 );

PROVIDE ( lwiperf_txbuf_const = 0x00006eb4 );


ASSERT(((__rw_lma_end - 0x10000) < 0xC000) , "Error: No enough roon left in IRAM");
ASSERT(((_end + _stack_size) < _stack) , "Error: No enough room left in DRAM");

/*
 * Copyright (c) 2016-2019 Newracom, Inc.
 *
 * Command-line interface
 *
 */

#include <stdarg.h>
#include <stdlib.h>
#include "system.h"
#include "build_ver.h"
#include "release_ver.h"
#include "util_cmd_lmac.h"
#include "util_cmd_lmac_indirect.h"
#if defined(STACK_LWIP)
#include "system_network.h"
#include "system_lwip.h"
#endif
#include "wdt.h"
#if (!defined _USR_LMAC_TEST) && (!defined _USER_LMAC_SDIO)
#include "amtNV.h"
#endif
#ifdef RELEASE
#undef system_printf
#define system_printf hal_uart_printf
#endif

// global variables
int g_cmd_optind = 1;
char *g_cmd_optarg;
char *g_result_buf;

int util_cmd_getopt_long(int argc, char *argv[], struct cmd_option *options, int *index)
{
	int i, j;
	char *key, *value;

	for (i = g_cmd_optind; i < argc; i++) {
		if ((value = strstr(argv[i], "=")) == NULL)
			continue;

		/* key=value */
		for (j = 0; options[j].key != NULL; j++) {
			if (strncmp(argv[i], options[j].key, strlen(options[j].key)) == 0) {
				*index = j;
				g_cmd_optind = i+1;
				g_cmd_optarg = value+1;
				return options[j].val;
			}
		}
	}

	return -1;
}

int isblank2(char c)
{
    return (( c == ' ' ) || ( c == '\t' ));
}

static int util_cmd_parse_line(char *s, char *argv[])
{
	int argc = 0;

	while (argc < NRC_MAX_ARGV) {
		while (isblank2(*s))
			s++;

		if (*s == '\0')
			goto out;

		argv[argc++] = s;

		while (*s && !isblank2(*s))
			s++;

		if (*s == '\0')
			goto out;

		*s++ = '\0';
	}
#ifdef UART_WPA_SEPARATION
	system_wpa_write("Command too long\n");
#else
	system_printf("Too many args\n");
#endif

 out:
	argv[argc] = NULL;
	return argc;
}

static struct nrc_cmd *find_cmd(char *cmd)
{
	struct nrc_cmd *t;

	for (t = ll_cmd_start(); t < ll_cmd_end(); t++) {
		if (t->flag == 0 && strcmp(cmd, t->name) == 0) {
#ifdef UART_WPA_SEPARATION
			system_printf("Cmd>>%s\n", cmd);
#endif			
			return t;
		}
	}
	return NULL;
}

static cmd_tbl_t *find_sub_cmd(cmd_tbl_t *parent, char *cmd)
{
	cmd_tbl_t *t;

	for (t = parent + 1; (t < ll_cmd_end() && (t->flag & 1)); t++) {
		if (!strcmp(t->name, cmd))
			return t;
	}
	return NULL;
}

static int cmd_process(int argc, char *argv[])
{
	cmd_tbl_t *p, *t;
	bool subcmd = false;
	int rc = 0;

	g_cmd_optind = 1;
	g_cmd_optarg = NULL;

	t = find_cmd(argv[0]);
	if (!t) {
#ifdef UART_WPA_SEPARATION
		system_wpa_write("Unknown command: %s\n", argv[0]);
#else
		system_printf("Unknown command: %s\n", argv[0]);
#endif
		goto ret_fail;
	} else if (t->handler == NULL ||
		    ((rc = t->handler(t, argc, argv)) == CMD_RET_UNHANDLED)) {

		if (argc < 2) {
#ifdef UART_WPA_SEPARATION
			system_wpa_write("%s needs sub-command\n", argv[0]);
#else
			system_printf("%s needs sub-command\n", argv[0]);
#endif
			goto ret_fail;
		}

		p = t;
		t = find_sub_cmd(p, argv[1]);
		if (t == NULL) {
#ifdef UART_WPA_SEPARATION
			system_wpa_write("%s is not a sub-command of %s\n", argv[1], argv[0]);
#else
			system_printf("%s is not a sub-command of %s\n", argv[1], argv[0]);
#endif
			goto ret_cmd_fail;
		} else if (t->handler == NULL) {
#ifdef UART_WPA_SEPARATION
			system_wpa_write("no hanlder for %s\n", argv[0]);
#else
			system_printf("no hanlder for %s\n", argv[0]);
#endif
		}
		argc--;
		argv++;

		rc = t->handler(t, argc, argv);
	}

	switch (rc) {
	case CMD_RET_SUCCESS:
#ifdef UART_WPA_SEPARATION
		system_wpa_write("OK\n");
#else
		system_printf("OK\n");
#endif
		break;
	case CMD_RET_FAILURE:
#ifdef UART_WPA_SEPARATION
		system_wpa_write("ERROR\n");
#else
		system_printf("ERROR\n");
#endif
		break;
	case CMD_RET_USAGE:
#ifdef UART_WPA_SEPARATION
		system_wpa_write("Usage: %s\n", t->usage);
#else		
		system_printf("Usage: %s\n", t->usage);
#endif
		break;
	default:
		break;
	}

	return rc;

ret_fail:
	return -1;

ret_cmd_fail:
	return CMD_RET_FAILURE;

}

void util_cmd_set_result_buf(char *result)
{
    g_result_buf = result;
}

int util_cmd_run_command(char *cmd)
{
	char *argv[NRC_MAX_ARGV] = {NULL, };
	int argc;
	int rc = 0;

	if (!cmd || *cmd == '\0')
		goto ret_fail;

	if (strlen(cmd) >= NRC_MAX_CMDLINE_SIZE) {
#ifdef UART_WPA_SEPARATION
		system_wpa_write("Command too long\n");
#else
		system_printf("Command too long\n");
#endif
		goto ret_fail;
	}
	argc = util_cmd_parse_line(cmd, argv);
	if (argc == 0)
		return -1;

	rc = cmd_process(argc, argv);

	return rc;

ret_fail:
	return -1;
}


int util_cmd_hex2num(char c)
{
        if (c >= '0' && c <= '9')
                return c - '0';

        if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;

        if (c >= 'A' && c <= 'F')
                return c - 'A' + 10;

	return -1;
}


int util_cmd_hex2byte(uint8_t *hex, uint8_t *byte)
{
        int a, b;

        a = util_cmd_hex2num(*hex++);
        if (a < 0 || a > 15)
                return -1;

        b = util_cmd_hex2num(*hex++);
        if (b < 0 || b > 15)
                return -1;

        *byte = (a << 4) | b;

	return 0;
}

int util_cmd_parse_hwaddr(char *string, uint8_t *addr)
{
	uint8_t *p = (uint8_t *)string;
    int i;

	for (i = 0; i < MAC_ADDR_LEN; i++) {

		if (util_cmd_hex2byte(p, &addr[i]) < 0)
			return -1;
		p += 2;
		if (i < MAC_ADDR_LEN - 1 && *p++ != ':')
			return -1;
	}
	return 0;
}

#ifndef SINGLE_BOARD_VER
static int cmd_help(cmd_tbl_t *h, int argc, char *argv[])
{
	cmd_tbl_t *t = NULL, *start, *end;
	int i = 0;

	if (argc == 1) {
		for (t = ll_cmd_start(); t < ll_cmd_end(); t++) {
			if (!t->desc || t->flag)
				continue;
			system_printf("%-10s - %s\n", t->name, t->desc);
		}

		goto ret_success;
	}

	// help cmd subcmd
	t = start = ll_cmd_start();
	end = ll_cmd_end();

	t = find_cmd(argv[1]);
	if (!t)
		return CMD_RET_FAILURE;

	if (argc == 2 && t->flag == 0) {
		system_printf("%-10s - %s\n", t->name, t->desc);
		if (t->usage)
			system_printf("Usage: %s\n", t->usage);

		for (t = t + 1; (t < end) && (t->flag & CMD_ATTR_SUB); t++) {
			if (!t->desc)
				continue;
			system_printf("\t%-20s - %s\n", t->name, t->desc);

		}
		goto ret_success;
	}

	if (argc == 3)
		t = find_sub_cmd(t, argv[2]);

	if (!t)
		goto ret_fail;

	if (t->usage) {
		system_printf("Usage: %s\n", t->usage);
	}

	if (t->desc) {
		system_printf("%s\n", t->desc);
	}

	if (t->flag & CMD_ATTR_EXPL) {
		t->handler(h, 0, 0);
	}

ret_success:
	return CMD_RET_SUCCESS;

ret_fail:
	return CMD_RET_FAILURE;
}

CMD(help,
    cmd_help,
    "display information about CLI commands",
    "help [command]");



static int cmd_write(cmd_tbl_t *t, int argc, char *argv[])
{
	uint32_t addr, val;

	if (argc != 3)
		return CMD_RET_USAGE;

	addr = strtoul(argv[1], NULL, 0);
	addr = WORD_ALIGN(addr);
	val = strtoul(argv[2], NULL, 0);

	WRITE_REG(addr, val);

	return CMD_RET_SUCCESS;
}

CMD(write,
    cmd_write,
    "write a 32-bit value to a memory location",
    "write <address> <data>");

static int cmd_read(cmd_tbl_t *t, int argc, char *argv[])
{
	uint32_t addr, size , i;

	addr = (uint32_t)strtoul(argv[1], NULL, 0);
	addr = WORD_ALIGN(addr);
	size = (atoi(argv[2]) < sizeof(uint32_t)) ? sizeof(uint32_t) : atoi(argv[2]);

	for (i = 0; i < size/sizeof(uint32_t); i++) {
		system_printf("%08x: %08x\n", addr, READ_REG(addr));
		addr += 4;
	}

	return CMD_RET_SUCCESS;
}

CMD(read,
    cmd_read,
    "read 32-bit value(s) from a memory location",
    "read <address> <size in byte>");

CMD(set,
    NULL,
    "set various MAC/PHY parameters",
    "set <parameters>");

CMD(show,
    NULL,
    "displays current configuration and setting",
    "show [information]");

// reset command
CMD(reset,
    NULL,
    "reset configuration and setting",
    "reset {option}");

CMD(phy,
    NULL,
    "configure phy",
    "phy {option}");

CMD(test,
    NULL,
    "test",
    "test {option}");

CMD(pwm,
	NULL,
	"test",
	"test {option}");


/*----------------------------------------------------------------------
 *
 * Orphan commands looking for adoption.
 *
 *----------------------------------------------------------------------*/

static int cmd_show_version(cmd_tbl_t *t, int argc, char *argv[])
{
	#if 0
	system_printf("Newracom Firmware Version : %02u.%02u(%s:%s) \n"
		     "Compiled on "__DATE__" at "__TIME__"\n"
		     ""COPYRIGHT(2017)"\n",
		     (VERSION_MAJOR), (VERSION_MINOR), (VERSION_BRANCH), (VERSION_BUILD));
	#else
	system_printf("TRS Firmware Version : TR6260_IoTV%u.%u.%uB%0u\n"
		     "Compiled on "__DATE__" at "__TIME__"\n"
		     ""COPYRIGHT(2017)"\n",
		     (VERSION_MAJOR), (VERSION_SUB),(VERSION_MINOR), (VERSION_B));
	
	#endif

	return CMD_RET_SUCCESS;
}

SUBCMD(show,
       version,
       cmd_show_version,
       "displays firmware version",
       "show version");

#if 0
static int cmd_show_memory(cmd_tbl_t *t, int argc, char *argv[])
{
	uint32_t *addr, len , i;
	uint32_t address;

	if (argc != 3)
		return CMD_RET_USAGE;

	address = WORD_ALIGN(strtoul(argv[1], NULL, 0));
	addr = (uint32_t *)address;
	len = atoi(argv[2]);
	system_printf("show_memory (address:%p, length:%d)\n", addr, len);

	for (i = 0; i < len; ++i) {
		system_printf("%02X ", addr[i]);
		if (i % 16 == 15) system_printf("\n");
	}
	system_printf("\n");
	return CMD_RET_SUCCESS;
}
SUBCMD(show,
       memory,
       cmd_show_memory,
       "display memory",
       "show memory <address> <# bytes>");

static void print_heap_block(void *addr, size_t size, uint32_t extra)
{
	system_printf("[Heap] addr=0x%x size=%d extra=0x%x\n", (size_t) addr,
		size, extra);
}

static int cmd_show_heap(cmd_tbl_t *t, int argc, char *argv[])
{
	//vPortGetBlockInfo(print_heap_block);

	return CMD_RET_SUCCESS;
}

SUBCMD(show,
       heap,
       cmd_show_heap,
       "display heap",
       "show heap");
#endif
#if defined(NRC6101)
// FIXME: Move to platform-dependent file
static int cmd_show_time(cmd_tbl_t *t, int argc, char *argv[])
{
	uint32_t x, y, z;

	x = readl(MAC_REG_MAC_TIMER_HIGH);
	y = readl(MAC_REG_MAC_TIMER_LOW);
	z = readl(MAC_REG_MAC_TIMER);

	system_printf("MAC time: 0x%08x:0x%08x, 0x%08x\n", x, y, z);

	return CMD_RET_SUCCESS;
}
SUBCMD(show,
       time,
       cmd_show_time,
       "display MAC timer",
       "show time");
#endif

#if 0 //defined(NRC6101) || defined(NRC7291)
// FIXME: later
static int cmd_reset_phy(cmd_tbl_t *t, int argc, char *argv[])
{
	uint32_t val;

	val = readl(PHY_REG_TOP_STATE_0);
	val &= ~PHY_REG_TOP_STATE_0_SW_RESET_MASK;
	val |= (1 << PHY_REG_TOP_STATE_0_SW_RESET_SHIFT);
	writel(1, PHY_REG_TOP_STATE_0);

	val = readl(PHY_REG_TOP_STATE_0);
	val &= ~PHY_REG_TOP_STATE_0_SW_RESET_RX_IDLE_MASK;
	val |= (1 << PHY_REG_TOP_STATE_0_SW_RESET_RX_IDLE_SHIFT);
	writel(1, PHY_REG_TOP_STATE_0);

	NRC_HMAC.initialize();

	return CMD_RET_SUCCESS;
}

SUBCMD(reset,
       phy,
       cmd_reset_phy,
       "reset PHY",
       "reset phy");


#if defined(NRC7291)
static int cmd_reset_all(cmd_tbl_t *t, int argc, char *argv[])
{
	int rc;

	NRC_HMAC.reset();
	rc = cmd_reset_phy(NULL, 0, NULL);

	phy_disable();
	if (SUBSYSTEM_DELIMITER) {
		arf_self_trx_cal();
	} else {
		nrf_self_trx_cal();
	}
	phy_enable();

	return CMD_RET_SUCCESS;
}

SUBCMD(reset,
       all,
       cmd_reset_all,
       "reset all",
       "reset all");


#endif
#endif



#if 0
// commands to remove

static int cmd_set_timer(cmd_tbl_t *t, int argc, char *argv[])
{
	uint32_t value, period;
	Timer *timer = new Timer;

	if (argc != 3)
		return CMD_RET_USAGE;

	value = atoi(argv[1]);
	period = atoi(argv[2]);

	timer->set_id(1);
	SET_TIMER(System::CLI, timer, value, period);

	MAC_LOG_INFO(TT_HAL, "[%u]set timer (%u us) period:%u us\n", NOW, value, period);

	return CMD_RET_SUCCESS;
}

SUBCMD(set,
       timer,
       cmd_set_timer,
       "set timer",
       "set timer <start value> <period>");

static int cmd_show_timer(cmd_tbl_t *t, int argc, char *argv[])
{
	Timer::show();

	return CMD_RET_SUCCESS;
}
SUBCMD(show,
       timer,
       cmd_show_timer,
       "display timer",
       "show timer");

static void dm_send_word(uint32_t value)
{
	uint8_t *p = (uint8_t *)&value;
    int i;
	for (i = 0; i < 4; i++) {
        serial_printf("%c",*p);
		p++;
	}
}

static int cmd_dm(cmd_tbl_t *t, int argc, char *argv[])
{
    int i;
	MAC_LOG_DECLARE();
	MAC_LOG_BEGIN();

	if (strcmp(argv[1], "enter") == 0) {
		NRC_CLI.echo(false);
		CONSOLE::echo(false);
	} else if (strcmp(argv[1], "leave") == 0) {
		NRC_CLI.echo(true);
		CONSOLE::echo(true);
	} else if (strcmp(argv[1], "start") == 0) {
		uint32_t interval = (uint32_t) atoi(argv[1]);
		TASK(statistic).set_tp_interval(interval);
		TASK(statistic).set_dm_enable(true);
		SEND_MESSAGE(System::STATISTIC, new Message(Message::STATISTIC));
	} else if (strcmp(argv[1], "stop") == 0) {
		TASK(statistic).set_dm_enable(false);
		SEND_MESSAGE(System::STATISTIC, new Message(Message::STATISTIC));
	} else if (strcmp(argv[1], "init") == 0) {
		TASK(statistic).init_address();
	} else if (strcmp(argv[1], "add") == 0) {
		uint32_t index = (uint32_t) atoi(argv[2]);
		uint32_t addr = (uint32_t) atoi(argv[3]);
		TASK(statistic).set_address(index, addr);
	} else if (strcmp(argv[1], "done") == 0) {
		uint32_t index = (uint32_t) atoi(argv[2]);
		TASK(statistic).set_address_end(index);
	} else if (strcmp(argv[1], "dump") == 0) {
		//MAC_LOG_INFO(TT_HAL, "address_end: 0x%08X\n", TASK(statistic).get_address_end());

		for (i = 0; i < TASK(statistic).get_address_end(); i++) {
			uint32_t value = READ_REG(TASK(statistic).get_address(i));
			MAC_LOG_INFO_CTX(TT_HAL, "%d, 0x%08X, 0x%08X\n",
				     i, TASK(statistic).get_address(i), value);
		}
	} else if (strcmp(argv[1], "ir") == 0) {
		uint32_t waddr = MAC_REG_BASE_START;
		waddr += (uint32_t) atoi(argv[1]);
		uint32_t param = (uint32_t) atoi(argv[2]);

		uint32_t raddr = MAC_REG_BASE_START;
		raddr += (uint32_t) atoi(argv[3]);
		WRITE_REG(waddr, param);

		uint32_t ret = READ_REG(raddr);
		if (TASK(statistic).get_dm_enable()) {
			dm_send_word(0xC0BEDA7F);
			dm_send_word(ret);
		} else {
			MAC_LOG_VB_CTX(TT_HAL, "result:%d\n", ret);
		}
	}

	MAC_LOG_END();
	return CMD_RET_SUCCESS;
}

// DM
CMD(dm,
    cmd_dm,
    "diagnostic monitor",
    "dm {start|stop|enter|leave|init|add|done|dump|ir}");

#endif

#if defined(INCLUDE_WPA_SUPP)
int wpa_cmd_receive(int vif_id, int argc, char *argv[]);

static int cmd_wpa(cmd_tbl_t *t, int argc, char *argv[])
{
	if (wpa_cmd_receive(0, argc, argv) == 0)
		return CMD_RET_SUCCESS;

	return CMD_RET_FAILURE;
}

static int cmd_wpb(cmd_tbl_t *t, int argc, char *argv[])
{
	if (wpa_cmd_receive(1, argc, argv) == 0)
		return CMD_RET_SUCCESS;

	return CMD_RET_FAILURE;
}

CMD(wpa,
	cmd_wpa,
	"wpa_supplicant command (vif 0)",
	"wpa {scan}");

CMD(wpb,
	cmd_wpb,
	"wpa_supplicant command(vif 1)",
	"wpa {scan}");

#endif
#if defined(STACK_LWIP)
static int cmd_ifconfig(cmd_tbl_t *t, int argc, char *argv[])
{
	int i = 0;
	char buf[64] = {0,};

	if(argc>1)
	{
		sprintf(buf, "%s", argv[1]);
		for (i = 2; i < argc; i++) {
			sprintf(buf, "%s %s", buf, argv[i]);
		}
	}
	wifi_ifconfig(buf);
	return CMD_RET_SUCCESS;
}

CMD(ifconfig,
    cmd_ifconfig,
    "ifconfig command",
    "ifconfig -h");

#ifdef LWIP_PING
static int cmd_ping(cmd_tbl_t *t, int argc, char *argv[])
{
	int i = 0;
	char buf[512] = {0,};

	sprintf(buf, "%s", argv[1]);
	for (i = 2; i < argc; i++) {
		sprintf(buf, "%s %s", buf, argv[i]);
	}
	ping_run(buf);
	return CMD_RET_SUCCESS;
}

CMD(ping,
    cmd_ping,
    "ping command",
    "ping [IP address]");
#endif /* LWIP_PING */

#ifdef LWIP_IPERF
static int cmd_iperf(cmd_tbl_t *t, int argc, char *argv[])
{
	int i = 0;
	char buf[512] = {0,};

	if (argc <= 1)
	        return CMD_RET_USAGE;

	sprintf(buf, "%s", argv[1]);
	for (i = 2; i < argc; i++) {
	        sprintf(buf, "%s %s", buf, argv[i]);
	}
	iperf_run(buf);
	return CMD_RET_SUCCESS;
}

CMD(iperf,
    cmd_iperf,
    "iperf command",
    "iperf [-s|-c host] [options]");
#endif /* LWIP_IPERF */

#endif /* STACK_LWIP */

static int cmd_reBoot(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("REBOOT!!\r\n\r\n");
	wdt_reset_chip(WDT_RESET_CHIP_TYPE_REBOOT);	
	return CMD_RET_SUCCESS;
}

CMD(reboot,
    cmd_reBoot,
    "reset chip to boot",
    "reboot");

static int cmd_reSystem(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("RESYSTEM!!\r\n\r\n");
	wdt_reset_chip(WDT_RESET_CHIP_TYPE_RESYSTEM);	
	return CMD_RET_SUCCESS;
}

CMD(resystem,
    cmd_reSystem,
    "reset chip to boot",
    "reboot");
int cmd_assert(cmd_tbl_t *t, int argc, char *argv[])
{
	configASSERT(0);
	return 0;
}
CMD(assert,
	cmd_assert,
	"active assert",
	"assert");
#endif
extern void flash_dump_print();
int cmd_print_flash(cmd_tbl_t *t, int argc, char*argv[])
{
	flash_dump_print();
	return CMD_RET_SUCCESS;
}

CMD(flashdump,
	cmd_print_flash,
	"flash_print",
	"flash");


#if ((!defined _USR_LMAC_TEST) && (!defined _USER_LMAC_SDIO)) && (!defined SINGLE_BOARD_VER)

#if 0
static int cmd_reset(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret;
	int type;

	if(argc != 2)
	{
		system_printf("cmd_su ARG NUM(: %d) IS ERROR!!\n", argc);
		return CMD_RET_FAILURE;
	}

	type = atoi(argv[1]);

	system_reset(type);
	system_printf("cmd_reset type: %d\n", type);
	return CMD_RET_SUCCESS;
}

CMD(rst, cmd_reset,  "reset chip",  "rst [type], type: 0-normal  1-wdt_timeout  2-wdt_rst  3-soft_rst  4-wakeup  5-ota_rst");
#endif

static int cmd_NVDel(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret;
	char * key, *value;

	if(argc != 2)
	{
		system_printf("NV SET ARG NUM(: %d) IS ERROR!!\n", argc);
		return CMD_RET_FAILURE;
	}

	key = argv[1];

	ret = ef_del_env(key);
	if(ret)
	{
		system_printf("NV DEL ERROR(%d)!!\n", ret);
		return CMD_RET_FAILURE;
	}

	system_printf("NV DEL key: %s\n", key);
	return CMD_RET_SUCCESS;
}

CMD(nvd, cmd_NVDel,  "delete nv",  "nvd <key>");

static int cmd_NVSet(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret;
	char * key, *value;
	//char val[256] = {0};

	if(argc != 3)
	{
		system_printf("NV SET ARG NUM(: %d) IS ERROR!!\n", argc);
		return CMD_RET_FAILURE;
	}

	key = argv[1];
#if 0
	value = argv[2];
	memcpy(val, value, strlen(value));

	ret = ef_set_env_blob(key, val, strlen(value));
#else
	value = argv[2];

	ret = ef_set_env_blob(key, value, strlen(value));
#endif
	if(ret)
	{
		system_printf("NV SET ERROR(%d)!!\n", ret);
		return CMD_RET_FAILURE;
	}

	system_printf("NV SET key: %s, value: %s\n", key, value);
	return CMD_RET_SUCCESS;
}

CMD(nvs, cmd_NVSet,  "setting nv value",  "nvs <key> <value>");

static int cmd_NVGet(cmd_tbl_t *t, int argc, char *argv[])
{
	char * key;
	char value[256];

	if(argc != 2)
	{
		system_printf("NV GET ARG NUM(: %d) IS ERROR!!\n", argc);
		return CMD_RET_FAILURE;
	}

	key = argv[1];

	if(strcmp("all", key)==0)
	{
		ef_print_env();
		return CMD_RET_SUCCESS;
	}

	memset(value, 0, 256);

	if(ef_get_env_blob(key, value, 256, NULL) == 0)
	{
		if((strcmp("SN", key)==0) && (amt_nv_read(AMT_NV_MAC, (unsigned char *)value, 256) == 0))
		{
			//system_printf("AMT-NV GET key: %s, value: %s\n", key, value);
			//return CMD_RET_SUCCESS;
		}
		else
		{
			system_printf("NV GET FAILED, NO SUCH NV!!\n");
			return CMD_RET_FAILURE;
		}
	}

	system_printf("NV GET key: %s, value: %s\n", key, value);
	return CMD_RET_SUCCESS;
}

CMD(nvg, cmd_NVGet,  "getting nv value",  "nvg <key>");


#if 0
static int cmd_BackupNVDel(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret;
	char * key, *value;

	if(argc != 2)
	{
		system_printf("BNV SET ARG NUM(: %d) IS ERROR!!\n", argc);
		return CMD_RET_FAILURE;
	}

	key = argv[1];

	ret = backup_del_env(key);
	if(ret)
	{
		if(ret != EF_WRITE_ERR)
			system_printf("BNV DEL ERROR(%d)!!\n", ret);
		return CMD_RET_FAILURE;
	}

	system_printf("BNV DEL key: %s\n", key);
	return CMD_RET_SUCCESS;
}

CMD(bnvd, cmd_BackupNVDel,  "delete backup nv",  "bnvd <key>");


static int cmd_BackupNVSet(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret;
	char * key, *value;
	//char val[256] = {0};

	if(argc != 3)
	{
		system_printf("BNV SET ARG NUM(: %d) IS ERROR!!\n", argc);
		return CMD_RET_FAILURE;
	}

	key = argv[1];
#if 0
	value = argv[2];
	memcpy(val, value, strlen(value));

	ret = ef_set_env_blob(key, val, strlen(value));
#else
	value = argv[2];

	ret = backup_set_env_blob(key, value, strlen(value));
#endif
	if(ret)
	{
	
		if(ret != EF_WRITE_ERR)
			system_printf("BNV SET ERROR(%d)!!\n", ret);
		return CMD_RET_FAILURE;
	}

	system_printf("BNV SET key: %s, value: %s\n", key, value);
	return CMD_RET_SUCCESS;
}

CMD(bnvs, cmd_BackupNVSet,  "setting backup nv value",  "bnvs <key> <value>");


static int cmd_BackupNVGet(cmd_tbl_t *t, int argc, char *argv[])
{
	char * key;
	char value[256];
	char ret = 0;

	if(argc != 2)
	{
		system_printf("BNV GET ARG NUM(: %d) IS ERROR!!\n", argc);
		return CMD_RET_FAILURE;
	}

	key = argv[1];

	if(strcmp("all", key)==0)
	{
		if(backup_get_env_blob(key, value, 256, NULL)== 0xffffffff)	
			return CMD_RET_FAILURE;
		backup_print_env();
		return CMD_RET_SUCCESS;
	}

	memset(value, 0, 256);

	if((ret = backup_get_env_blob(key, value, 256, NULL) )== 0)
	{
		system_printf("BNV GET FAILED, NO SUCH NV!!\n");
		return CMD_RET_FAILURE;
	}
	if(ret == 0xffffffff)
		return CMD_RET_FAILURE;

	system_printf("BNV GET key: %s, value: %s\n", key, value);
	return CMD_RET_SUCCESS;
}

CMD(bnvg, cmd_BackupNVGet,  "getting backup nv value",  "bnvg <key>");


static int cmd_PartionGet(cmd_tbl_t *t, int argc, char *argv[])
{
	char * key;
	unsigned int base, len;
	
	if(argc != 2)
	{
		system_printf("PARTION GET ARG NUM(: %d) IS ERROR!!\n", argc);
		return CMD_RET_FAILURE;
	}

	key = argv[1];

	if(strcmp("all", key)==0)
	{
		partion_print_all();
		return CMD_RET_SUCCESS;
	}

	partion_info_get(key, &base, &len);

	system_printf("PARTION key: %s, base: 0x%08x, len: %d\n", key, base, len);
	return CMD_RET_SUCCESS;
}

CMD(bpg, cmd_PartionGet,  "getting backup nv value",  "bpg <key>");

static int cmd_NVRecover(cmd_tbl_t *t, int argc, char *argv[])
{
	int len;
	char * key;
	char value[256];

	if(argc != 2)
	{
		system_printf("PARTION GET ARG NUM(: %d) IS ERROR!!\n", argc);
		return CMD_RET_FAILURE;
	}

	key = argv[1];
	
	if(backup_get_env_blob(key, value, 256, NULL) == 0xffffffff)		
		return CMD_RET_FAILURE;

	if(strcmp("all", key)==0)
	{
		if(backup_get_env_blob(key, value, 256, NULL) == 0xffffffff)		
			return CMD_RET_FAILURE;
		backup_recovery();
		return CMD_RET_SUCCESS;
	}

	if((len = backup_get_env_blob(key, value, 256, NULL)) == 0)
	{
		system_printf("BNV GET FAILED, NO SUCH NV!!\n");
		return CMD_RET_FAILURE;
	}

	if(ef_set_env_blob(key, value, len))
	{
		system_printf("NV SET ERROR!!\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

CMD(nvr, cmd_NVRecover,  "recorery backup nv value",  "nvr <key>");
#endif


#endif


#ifdef  OS_CALC_CPU_USAGE
static int show_usage_fun(cmd_tbl_t *t, int argc, char *argv[])
{
	system_printf("CPU Usage: %d%%\n", os_get_cpu_usage());
	return CMD_RET_SUCCESS;
}
SUBCMD(show,   usage, show_usage_fun, "", "");
#endif

#ifdef RELEASE
#undef system_printf
#define system_printf
#endif

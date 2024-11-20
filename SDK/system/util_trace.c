#include "system.h"
#include "util_cmd.h"

static const struct TraceLevel g_tlevel[TL_MAX] = {
	{ TL_VB,        "verbose"	,"vvv", "0"},
	{ TL_INFO,      "info"		, "vv", "1"},
	{ TL_ERR,       "error"		,  "v", "2"},
};

struct TraceMap g_trace_map[TT_MAX] = {
	{"qm",       TL_ERR },
	{"hif",   	 TL_INFO},
	{"wim",      TL_INFO},
	{"api",      TL_ERR},
	{"msg",      TL_INFO},
	{"rx",       TL_INFO},
	{"tx",       TL_INFO},
	{"dl",       TL_INFO},
	{"ul",       TL_INFO},
	{"phy",      TL_INFO},
	{"rf",       TL_INFO},
	{"umac",     TL_INFO},
	{"ps",       TL_ERR},
	{"halow",	 TL_INFO},
	{"wpas",	 TL_INFO},
};

uint8_t util_trace_set_log_level(int type_id, uint32_t level)
{
	unsigned long flags = system_irq_save();
	uint8_t old = g_trace_map[type_id].level;
	g_trace_map[type_id].level = level;
	system_irq_restore(flags);
	return old;
}

#if (!defined AMT) && (!defined SINGLE_BOARD_VER)
static int cmd_set_log_level(cmd_tbl_t *t, int argc, char *argv[])
{
	int tt = TT_MAX;
	int tl = TL_MAX;
	int i;

	if (argc != 3)
		return CMD_RET_USAGE;

	for (i = 0; i < ARRAY_SIZE(g_tlevel); i++) {
		if ((strcmp(argv[2], g_tlevel[i].string) == 0) ||
			(strcmp(argv[2], g_tlevel[i].alias1) == 0) ||
			(strcmp(argv[2], g_tlevel[i].alias2) == 0)){
			tl = g_tlevel[i].value;
			break;
		}
	}

	if (tl == TL_MAX) {
		system_printf("invalid log level %s\n", argv[2]);
		return CMD_RET_USAGE;
	}

	/* per module */
	for (i = 0; i < TT_MAX; i++) {
		if (strcmp(argv[1], g_trace_map[i].string) == 0) {
			g_trace_map[i].level = tl;
			return CMD_RET_SUCCESS;
		}
	}

	return CMD_RET_USAGE;
}

SUBCMD(set,
	   log,
	   cmd_set_log_level,
	   "set log level (type 'show log' to see the current log level)",
	   "set log <module> {verbose|info|error|assert|none}");

static int cmd_show_log(cmd_tbl_t *t, int argc, char *argv[])
{
	int i;
	system_printf("------------------------\n");
	system_printf(" Type\tLevel\n");
	system_printf("------------------------\n");

	for (i = 0; i < TT_MAX; i++) {
		system_printf(" %s\t%s\n", g_trace_map[i].string, g_tlevel[g_trace_map[i].level].string);
	}
	return CMD_RET_SUCCESS;
}
SUBCMD(show,
	   log,
	   cmd_show_log,
	   "display log level",
	   "show log");
#endif

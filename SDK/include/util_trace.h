#include "system.h"

#ifndef TRACE_H
#define TRACE_H

///=============================================================================
///
/// Trace level and type enumerations
///
///=============================================================================
enum TRACE_LEVEL {
    TL_VB		= 0,
    TL_INFO		= 1,
    TL_ERR		= 2,
    TL_MAX,
};

enum TRACE_TYPES {
    TT_QM = 0,
    TT_HIF,
    TT_WIM,
    TT_API,
    TT_MSG,
    TT_RX,
    TT_TX,
    TT_DL,
    TT_UL,
    TT_PHY,
    TT_RF,
    TT_UMAC,
    TT_PS,
    TT_HALOW,
    TT_WPAS,
    TT_DG,
    TT_MAX,             // must be last
};

///=============================================================================
///
/// Trace level and Map structure
///
///=============================================================================
struct TraceLevel {
	uint8_t    value;
	const char *string;
	const char *alias1;
	const char *alias2;
} __packed;


struct TraceMap {
	const char *string;
	uint8_t     level;
} __packed;

extern struct TraceMap g_trace_map[TT_MAX];
void show_log_level();
uint8_t util_trace_set_log_level(int type_id, uint32_t level);

#define xPRINT(x, format, ...)		system_printf(format, ##__VA_ARGS__)

#if !defined(RELEASE) && !defined(UCODE) && !defined(INCLUDE_STANDALONE) && !defined(SINGLE_BOARD_VER)
#define V(x, format, ...)		\
do {						\
	if(TL_VB >= g_trace_map[x].level)		{\
		xPRINT(x, format, ##__VA_ARGS__);	}\
} while (0)

#define I(x, format, ...) 		\
do {						\
	if (TL_INFO >= g_trace_map[x].level)	{\
		xPRINT(x, format, ##__VA_ARGS__);	}\
} while (0)

#define E(x, format, ...)		\
do {						\
	if (TL_ERR >= g_trace_map[x].level)		{\
		xPRINT(x, format, ##__VA_ARGS__);	}\
} while (0)

#define D(x, format, ...)		\
	do {						\
		if (TL_ERR >= g_trace_map[x].level) 	{\
			xPRINT(x, format, ##__VA_ARGS__);	}\
	} while (0)
	
#define VCOND(x)			\
do {						\
	if (TL_VB < g_trace_map[x].level)		{\
		return;								}\
} while (0)

#define ICOND(x)			\
do {						\
	if (TL_INFO < g_trace_map[x].level)		{\
		return;								}\
} while (0)

#define ECOND(x)			\
do {						\
	if (TL_ERR < g_trace_map[x].level)		{\
		return;								}\
} while (0)

#else //RELEASE
#define V(x, format, ...)	do{}while(0)
#define I(x, format, ...) 	do{}while(0)
#define E(x, format, ...)	do{}while(0)
#define VCOND(x)			do{}while(0)
#define ICOND(x)			do{}while(0)
#define ECOND(x)			do{}while(0)
#endif //!defined(RELEASE)
#endif    // TRACE_H

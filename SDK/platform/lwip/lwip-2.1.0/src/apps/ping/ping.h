#ifndef __PING_H__
#define __PING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/arch.h"
#include "lwip/ip_addr.h"

/** ping delay - in milliseconds */
#ifndef PING_DELAY_DEFAULT
#define PING_DELAY_UNIT     1000  /* ms */
#define PING_DELAY_DEFAULT     PING_DELAY_UNIT
#endif

#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE     32
#endif

#define PING_COUNT_DEFAULT     0xFFFF

enum ping_mode {
	PING_STATUS_STOP,
	PING_STATUS_START,
	PING_STATUS_MAX,
};

typedef struct _ping_parm ping_parm_t;
struct _ping_parm
{
	u32_t packet_size;
	u32_t interval;
	u32_t target_count;
	u32_t time;
	u32_t count;
	u32_t success;
	u32_t total_delay;
	u32_t time_delay;
	u16_t seq_num;
	u8_t vif_id;
	u8_t force_stop;
	xTaskHandle task_handle;
	ip4_addr_t addr;
	ping_parm_t* next;
};

sys_thread_t ping_init(void *arg);
void ping_deinit(void *arg);
ping_parm_t *ping_get_session(ip4_addr_t* addr);
void ping_list_display(void);
void ping_mutex_init(void);
int ping_once(ping_parm_t* ping_info);
#ifdef __cplusplus
}
#endif

#endif /* __PING_H__ */

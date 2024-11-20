#ifndef HAL_LMAC_UTIL_H
#define HAL_LMAC_UTIL_H

// Events for rate control update
enum {
	RC_NO_OP,
	RC_ACK_RX_SUCCESS,
	RC_ACK_RX_FAIL,
};

// The result of update
enum {
	RC_NONE,
	RC_NEXT,
	RC_PREV,
};

enum {
	RC_ATTR_20M = (1 << 0),
	RC_ATTR_40M = (1 << 1),
	RC_ATTR_RSV = (1 << 2),
	RC_ATTR_SGI = (1 << 3),
};

enum {
	RC_ATTR_1M    = (1 << 0),
	RC_ATTR_2M    = (1 << 1),
	RC_ATTR_4M    = (1 << 2),
	RC_ATTR_MCS10 = (1 << 3),
};

enum {
#if LMAC_CONFIG_11N == 1
	RC_11B_ENTRY_NUM = 4,
	RC_11AG_ENTRY_NUM = 8,
	RC_HT_LGI_ENTRY_NUM = 8,
	RC_HT_SGI_ENTRY_NUM = 8,
	RC_MAX_ENTRY_SIZE	= RC_11B_ENTRY_NUM + RC_11AG_ENTRY_NUM + RC_HT_LGI_ENTRY_NUM + RC_HT_SGI_ENTRY_NUM,

	RC_11B_BASE = 0,
	RC_11AG_BASE = 4,
	RC_HT_LGI_BASE = 12,
	RC_HT_SGI_BASE = 20,

#elif LMAC_CONFIG_11AH == 1
	RC_1M_ENTRY_NUM = 9,
	RC_2M_ENTRY_NUM = 8,
	RC_4M_ENTRY_NUM = 8,
	RC_MAX_ENTRY_SIZE = RC_1M_ENTRY_NUM + RC_2M_ENTRY_NUM + RC_4M_ENTRY_NUM,

	RC_1M_BASE = 0,
	RC_2M_BASE = 9,
	RC_4M_BASE = 17,
#endif
};

/* 	0: 11BGN mixed
	1: legacy 11b only
	2: legacy 11g only
	3: 11N only in 2.4G*/
typedef enum {
 		RC_FORMAT_11BGN_MIX_NV = 0,
		RC_FORMAT_11B_ONLY_NV,
		RC_FORMAT_11G_ONLY_NV,
		RC_FORMAT_11N_ONLY_NV,
	}RC_FORMAT_NV;

struct track_index{
	bool flag;
	uint8_t index : 7;
};

typedef struct _rate_entry {
	uint8_t order;
	uint8_t format : 4;
	uint8_t mcs    : 4;
	uint8_t attr;
} __attribute__((packed)) RateEntry;

typedef struct _rate_controller {
	uint8_t m_start_idx;
	uint8_t m_end_idx;
	struct track_index m_entry_track[RC_MAX_ENTRY_SIZE];

	uint8_t m_success_count : 4;
	uint8_t m_fail_count    : 4;
	uint8_t m_cursor        : 7;
    uint8_t m_probe         : 1;

} __attribute__((packed)) RC_CONTROL;

typedef struct {
    uint8_t mcs;
    uint16_t rate;
}rate_set_t;

void lmac_rc_initialize(); /// Initialize values
void lmac_rc_configure(uint8_t format, uint8_t attr, uint32_t mcs_bitmap, int vif_id);

/// Called by internal HAL
uint8_t lmac_rc_seq_success(uint8_t count, int vif_id); /// one sequence completed without any error
uint8_t lmac_rc_seq_fail(uint8_t count, int vif_id); /// sequence has flaw point
uint8_t lmac_rc_pre_update(uint8_t event, uint8_t count, int vif_id);
void    lmac_rc_post_update(uint8_t event, int vif_id);
void 	hal_lmac_rc_reset_start_index(int vif_id);

//const struct _rate_entry* lmac_rc_get_base(int vif_id);
const struct _rate_entry* lmac_rc_get_entry(int vif_id);
const struct _rate_entry* lmac_rc_get_max_entry(int vif_id);
#if 0
void lmac_rc_test();
#endif
void    lmac_rc_show_all(int vif_id);
void    lmac_rc_show_current_entry(int vif_id);
void    hal_lmac_set_rate_control(int vif_id, bool en);
bool    hal_lmac_get_rate_control(int vif_id);
uint8_t hal_lmac_get_max_rc_bandwidth(int vif_id);
void    hal_lmac_config_rate_control(uint8_t format, uint8_t attr, uint32_t mcs_bitmap, int vif_id);
void    hal_lmac_reconfig_rc_entry(uint8_t cur_width, uint8_t new_width, int vif_id);
void 	hal_lmac_rc_set_cursor_from_format(uint8_t format,int vif_id);
void lmac_rc_sync_cursor(uint8_t vif_id);
void lmac_rc_all_configure(int revert_flag);
struct _rate_entry *lmac_rc_get_valid_min(uint8_t vif_id);
int hal_mac_revert_rate_set(void);
int hal_mac_update_rate_set(rate_set_t *rate_set);

#if LMAC_CONFIG_11AH == 1
bool    hal_lmac_check_sf_macaddr(uint8_t *mac);
int util_str_to_bandwidth(const char *str);
int util_str_to_prim_ch_bandwidth(const char *str);
bool util_str_onoff(const char *str);
#endif
#endif

#ifndef __HAL_LMAC_PS_COMMON_H__
#define __HAL_LMAC_PS_COMMON_H__

#
typedef enum _ps_mode {
    PS_MODE_NO,
    PS_MODE_PSNONPOLL,
    PS_MODE_PSPOLL,
    PS_MODE_WMMPS,
    PS_MODE_NONTIM
} ps_mode;

typedef enum _ps_state {
    PS_STATE_IDLE,
    PS_STATE_ACTIVE,
    PS_STATE_ACTIVE_TO_DOZE,
    PS_STATE_DOZE,
    PS_STATE_DOZE_TO_ACTIVE
} ps_state;

typedef void (*ps_doze_cb)(void);
typedef void (*ps_wake_cb)(void);

#define MAX_CB 5

typedef struct _lmac_ps_info {
    ps_mode     mode;

    ps_state    state;
    ps_state    tx_state;
    ps_state    beacon_tim;

    ps_doze_cb  doze_cb[ MAX_CB ];
    uint8_t     doze_cb_num;

    ps_wake_cb  wake_cb[ MAX_CB ];
    uint8_t     wake_cb_num;

    uint8_t     beacon_alive;

    uint32_t    wait_tx_before_deepsleep_us;
    uint32_t    wait_rx_before_deepsleep_us;
    uint32_t    wakeup_after_deepsleep_us;
    uint32_t    last_tx_us;
    uint32_t    last_rx_us;
    uint32_t    deepsleep_start_us;
    uint32_t    last_tim_tsf;
    uint32_t    dtim_count;
    uint32_t    dtim_period;

    bool        non_tim;
    bool        keep_alive;

} lmac_ps_info;

void lmac_process_ps();

bool hal_lmac_ps_doze();
bool hal_lmac_ps_wake();

void hal_lmac_qos_data_send();
void hal_lmac_ps_init(uint32_t wait_tx , uint32_t wait_rx , uint32_t wakeup_after_deepsleep_us );
bool hal_lmac_ps_add_doze_cb( ps_doze_cb );
bool hal_lmac_ps_add_wake_cb( ps_wake_cb );
void hal_lmac_ps_set_listen_interval( uint32_t listen_interval_us );

// ps_xxx common function
void lmac_ps_active();
void lmac_ps_active_to_doze();
void lmac_ps_doze();
void lmac_ps_doze_to_active();
ps_mode lmac_ps_get_psmode();

extern lmac_ps_info g_lmac_ps_info;



#endif // __HAL_LMAC_PS_COMMON_H__

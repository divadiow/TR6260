#ifndef HAL_LMAC_TEST_H
#define HAL_LMAC_TEST_H
#include "hal_lmac_common.h"


typedef struct {
	uint32_t	m_packet_count;
	uint16_t	m_length;
	uint16_t	m_increase;
	uint16_t	m_interval;
	uint8_t		m_packet_type;
	uint8_t		m_tid;
    bool        m_infinite_en;
    bool        m_infinite_txop_en;
    void        (*m_dl_callback)(struct _SYS_BUF *);
    bool        m_doppler;

    bool        m_rx_gain_sweep;
    uint8_t     m_rx_gain_current;
    uint8_t     m_rx_gain_start;
    uint8_t     m_rx_gain_end;
    int8_t      m_rx_gain_step;
    uint32_t     m_rx_gain_interval;
    uint32_t    m_rx_gain_start_tsf;

    uint8_t     m_auto_rxgain;
    uint8_t     m_auto_rxgain_margin;
    uint32_t    m_auto_rxgain_interval;
    uint8_t     m_auto_rxgain_change;

    bool        m_ignore_sec;

#if LMAC_CONFIG_HWEMUL == 1
    uint32_t    m_hwemul;
    uint32_t    m_hwemul_sched;
#endif

} LMAC_TEST_CFG;
void hal_lmac_test_init();
void hal_lmac_test_tsf_handler();
#ifdef SINGLE_BOARD_VER
int hal_lmac_signal_board_conf(uint32_t ch,uint8_t format,uint8_t mcs);
#endif
#endif

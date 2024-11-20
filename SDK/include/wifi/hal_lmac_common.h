#ifndef HAL_LMAC_COMMON_H
#define HAL_LMAC_COMMON_H
#include "hal_lmac_util.h"
#include "hal_lmac_test.h"
#include "nrc-wim-types.h"
#include "protocol.h"
#include "protocol_11ah.h"
#include "hal_lmac_debug.h"
#if LMAC_CONFIG_11N == 1
#include "hal_lmac_11n.h"
#elif LMAC_CONFIG_11AH == 1
#include "hal_lmac_11ah.h"
#endif

#define MAC_REG_SIZE 	4
#define FCS_SIZE 		4
#define DELIMITER_SIZE  4
#define MGMT_HEADER_SIZE 24
#define CFO_WINDOW_SIZE 16

#define MSNOW   (NOW/1000)
#define MSNOW1   (NOW1/1000)

#define LMAC_BUF_TO_SYS_BUF(buf) ((SYS_BUF*)(((uint32_t)(buf)) - sizeof(SYS_HDR)))
#define SYS_BUF_TO_LMAC_TXBUF(buf) ((LMAC_TXBUF*)(&buf->lmac_txhdr))
#define SYS_BUF_TO_LMAC_RXBUF(buf) ((LMAC_RXBUF*)(&buf->lmac_rxhdr))

#define SYS_BUF_NEXT(buf)           buf->sys_hdr.m_next
#define SYS_BUF_LINK(buf)           buf->sys_hdr.m_link
#define SYS_BUF_OFFSET(buf)         buf->sys_hdr.m_payload_offset
#define SYS_BUF_LENGTH(buf)         buf->sys_hdr.m_payload_length
#define SYS_BUF_DATA_LENGTH(buf)    buf->sys_hdr.m_payload_length - sizeof(LMAC_TXHDR)

#define LMAC_TXBUF_NEXT(buf)        buf->lmac_txhdr.m_next
#define LMAC_TXBUF_LINK(buf)        buf->lmac_txhdr.m_link
#define LMAC_TXBUF_MACHDR(buf)      ((GenericMacHeader*)( buf->machdr ))

#define FRAG_FIRST  2
#define FRAG_MID    3
#define FRAG_LAST   1
#define FRAG_SINGLE 0

#define MAC_ADDR_LEN        6

#define RTS_ON  1
#define RTS_OFF 0
#define RTS_DEFAULT  2

enum {
	MODE_STA = 0,
	MODE_AP  = 1,
};

enum {
	    /// Ownership bit
	DESC_SW = 0,
	DESC_HW = 1,

	/// Length limit (based on the size of control.length)
	DESC_MAX_LENGTH = (1 << 11)
};

enum {
    MAX_KEY_ID = 4,
    MAX_KEY_LEN = 4,
    WEP_IV_LEN = 4,
    CIPHER_HEADER_LEN = 8,
    CIPHER_HEADER_LEN_WEP = 4,
    CIPHER_HEADER_LEN_WAPI = 18,
    MIC_LEN = 8,
    MIC_LEN_WAPI = 16,
    ICV_LEN	= 4,
};

enum CipherType {
    CIP_WEP40 = 0,
    CIP_WEP104 = 1,
    CIP_TKIP = 2,
    CIP_CCMP = 3,
    CIP_WAPI = 4,

    CIP_MAX = 5,
};

enum KeyType {
    KEY_PTK = 0,
    KEY_GTK = 1,
    KEY_MAX = 2,
};

enum KeyCmd {
    KEY_ADD = 0,
    KEY_DEL = 1,
    KEY_DELALL = 2
};


#define CIP_NONE		CIP_MAX
#define MAX_GTK			4

typedef enum  _LMacEvent {
    LME_DOWNLINK        = 0,
    LME_ULDONE          = 1,
    LME_TBTT            = 2,
    LME_CONC            = 3,
    LME_IDLE            = 4,
    LME_ACTIVE          = 5,
    LME_PSPOOL          = 6,
    LME_NULL            = 7,
    LME_MIC             = 8,
    LME_NAN_DW_START    = 9,
    LME_NAN_DW_END      = 10,
    LME_NAN_TBTT        = 11,
    LME_MAX,
} LMacEvent;


#define AC_TO_TID(_ac) (       \
	((_ac) == ACI_VO) ? 6 : \
	((_ac) == ACI_VI) ? 5 : \
	((_ac) == ACI_BK) ? 1 : \
	0)

#define TID_TO_AC(_tid) (      \
	((_tid) == 0 || (_tid) == 3) ? ACI_BE : \
	((_tid) < 3) ? ACI_BK : \
	((_tid) < 6) ? ACI_VI : \
	ACI_VO)

#ifdef DEBUG_MAC_STATS
enum {
	LOG_NONE 	= 0,
	LOG_DATA 	= 1 << 0, // 1
	LOG_MGMT 	= 1 << 1, // 2
	LOG_BEACON	= 1 << 2, // 4
	LOG_ALL		= LOG_DATA | LOG_MGMT | LOG_BEACON, //7
};

enum {
	DIR_TX 	= 0,
	DIR_RX
};

typedef	enum mac_stats_status{
	STATS_OK =0,
	STATS_NOK,
	MAX_STATS_STATUS
} MAC_STATS_STATUS;

typedef	enum mac_stats_mcs{
	STATS_MCS0 =0,
	STATS_MCS1,
	STATS_MCS2,
	STATS_MCS3,
	STATS_MCS4,
	STATS_MCS5,
	STATS_MCS6,
	STATS_MCS7,
	STATS_MCS10,
	STATS_MCS_MAX
} MAC_STATS_MCS;

typedef	enum mac_stats_type{
	STATS_TYPES_MGMT =0,
	STATS_TYPES_CTRL,
	STATS_TYPES_DATA,
	STATS_TYPES_BEACON,
	STATS_TYPES_MAX
} MAC_STATS_TYPE;

typedef enum {
	NV_LMAC_FORMAT_BGN_MIX = 0,/*legacy 11b/g mixed*/
	NV_LMAC_FORMAT_B_ONLY, /*legacy 11B only*/
	NV_LMAC_FORMAT_A_ONLY,/*legacy 11A only*/
	NV_LMAC_FORMAT_ABG_MIX, /*legacy 11a/b/g mixed*/
	NV_LMAC_FORMAT_G_ONLY, /*legacy 11G only*/
	NV_LMAC_FORMAT_ABGN_ONLY, /*11ABGN mixed*/
	NV_LMAC_FORMAT_N_ONLY,/* 11N only in 2.4G*/
	NV_LMAC_FORMA_GN_MIX,/* 11GN mixed*/
	NV_LMAC_FORMAT_AN_MIX,/*11AN mixed*/
	NV_LMAC_FORMAT_BGN_MIX,/* 11BGN mixed*/
	NV_LMAC_FORMAT_AGN_MIX, /*11AGN mixed */
	NV_LMAC_FORMAT_N_ONLY,/*11N only in 5G*/
	NV_LMAC_FORMAT_AANC_MIX, /*11A/AN/AC mixed 5G band only (Only 11AC chipset support)*/
	NV_LMAC_FORMAT_ANC_MIX, /*11 AN/AC mixed 5G band only (Only 11AC chipset support)*/
}NV_LMAC_WIRELESS_FORMAT;

typedef struct mac_stats_info {
	uint8_t print_flag;
	uint8_t last_mcs;
	uint32_t n_mpdu[STATS_TYPES_MAX][MAX_STATS_STATUS];
	uint32_t b_mpdu[STATS_TYPES_MAX][MAX_STATS_STATUS];
	uint32_t n_ac[MAX_AC][MAX_STATS_STATUS];
	uint32_t b_ac[MAX_AC][MAX_STATS_STATUS];
	uint32_t n_mcs[STATS_MCS_MAX][MAX_STATS_STATUS];
	uint32_t b_mcs[STATS_MCS_MAX][MAX_STATS_STATUS];
} MAC_STATS_INFO;
#endif

#if(LMAC_CONFIG_FREERTOS == 1)
    #ifndef SYSTEM_LMAC_TASK_STACK_SIZE
        #define LMAC_TASK_STACK_SIZE	    (6144 / sizeof(StackType_t))
        #define LMAC_TASK_PRIORITY		    (31)
    #else
        #define LMAC_TASK_STACK_SIZE	    SYSTEM_LMAC_TASK_STACK_SIZE
        #define LMAC_TASK_PRIORITY		    SYSTEM_LMAC_TASK_PRIORITY
    #endif
#endif

#define COMMON						g_lmac_info.m_common
#define LMAC_SNIFFER				g_lmac_info.m_lmac_sniffer_mode
#define LMAC_SNIFFER_BEACON			g_lmac_info.m_lmac_sniffer_mode_beacon

#define DEC_VAL_GET_SET(t, x) \
	t get_##x(uint8_t);\
	void set_##x(uint8_t, t);

#define DEF_VAL_GET_SET(t, x) \
	t get_##x(uint8_t vif_id ) { \
		return VIF(vif_id).x; \
	}; \
	void set_##x(uint8_t vif_id, t value) { \
        VIF(vif_id).x = value; \
	}

#define DEC_ARR_GET_SET(t, x) \
	t lmac_get_##x(uint8_t);\
	void lmac_set_##x(uint8_t, t);

#define DEF_ARR_GET_SET(t, x) \
	t lmac_get_##x(uint8_t vif_id ) { \
		return VIF(vif_id).x; \
	}; \
	void lmac_set_##x(uint8_t vif_id, t value) { \
        memcpy( VIF(vif_id).x , value , sizeof( VIF(vif_id).x ) );\
	}

typedef struct {
	uint8_t     device_mode;
	uint8_t		prim_ch_bw; //
	uint8_t     prim_ch_loc; //
	uint8_t		ch_bw; //
	uint8_t		mcs;
	uint8_t		rts;
	uint16_t	rts_threshold;
	uint8_t		retry_limit;
	uint8_t		short_gi;
    uint8_t     macaddr[MAC_ADDR_LEN];
    uint8_t     destaddr[MAC_ADDR_LEN];
    uint8_t     bssid[MAC_ADDR_LEN];
	uint8_t     country[2]; //
    uint8_t     format;
    uint8_t     promiscuous_mode;
    bool        rate_ctrl_en;

    uint16_t    aid;
    uint8_t     cipher_en;
    uint8_t		key_index;
	uint8_t     gtk_addr[MAX_GTK][MAC_ADDR_LEN];
    uint8_t		bssid_wepkey;
    uint8_t		cipher_type;
    uint8_t		group_cipher_type;
    uint16_t	beacon_interval;
    uint16_t	dtim_period;
    uint8_t		protection;
    uint8_t     cipher_disable_hwmic;
    uint32_t    frequency;
    uint32_t    mac80211_frequency;
    uint32_t    basic_rate;
	bool        scanning;

    struct _sn { uint16_t sn : 12; } tx_sn[ACI_MAX];
#if LMAC_CONFIG_11AH == 1
    uint8_t     scrambler;
    uint8_t		n_sts;
    uint8_t		coding;
    uint8_t		smoothing;
    uint8_t		preamble_type;
    bool		stbc;
    bool		uplink_ind;
    bool        bypass_mgmt;
    uint16_t	color;
    uint8_t		phy_txgain;
    uint8_t		phy_rxgain;
    bool        pv1_en;
    uint64_t    pn;
#endif

#if LMAC_CONFIG_11N == 1
    uint8_t     erp_protection;
    uint8_t     short_slot;
    uint8_t     short_preamble;
    uint16_t    ht_info;
    uint16_t    ht_cap;
#endif
} VIF;

#define DEC_COMMON_GET_SET(t, x) \
	t get_common_##x();\
	void set_common_##x(t);

#define DEF_COMMON_GET_SET(t, x) \
	t get_common_##x() { \
		return COMMON.x; \
	}; \
	void set_common_##x(t value) { \
        COMMON.x = value; \
	}

typedef struct {
    bool        cfo_cal_en;
    bool        cfo_apply_en;
    double      cfo_avg;
    double      cfo_window[16];
    uint8_t     cfo_count;
} COMMON_VAR;

DEC_VAL_GET_SET(bool           ,   rate_ctrl_en);
DEC_VAL_GET_SET(uint8_t        ,   prim_ch_bw);
DEC_VAL_GET_SET(uint8_t        ,   prim_ch_loc);
DEC_VAL_GET_SET(uint32_t       ,   frequency);
DEC_VAL_GET_SET(uint32_t       ,   mac80211_frequency);
DEC_VAL_GET_SET(uint8_t        ,   device_mode);
DEC_VAL_GET_SET(uint16_t       ,   aid);
DEC_VAL_GET_SET(bool           ,   cipher_en);
DEC_VAL_GET_SET(uint8_t        ,   key_index);
DEC_VAL_GET_SET(enum CipherType,   cipher_type);
DEC_VAL_GET_SET(enum CipherType,   group_cipher_type);
DEC_VAL_GET_SET(uint8_t        ,   cipher_disable_hwmic);
DEC_VAL_GET_SET(uint8_t        ,   promiscuous_mode);
DEC_VAL_GET_SET(uint8_t        ,   ch_bw);
DEC_VAL_GET_SET(uint32_t       ,   basic_rate);

DEC_VAL_GET_SET(bool           ,   scanning);
#if LMAC_CONFIG_11N == 1
DEC_VAL_GET_SET(uint8_t        ,   erp_protection);
DEC_VAL_GET_SET(uint8_t        ,   short_slot);
DEC_VAL_GET_SET(uint8_t        ,   short_preamble);
DEC_VAL_GET_SET(uint16_t       ,   ht_info);
DEC_VAL_GET_SET(uint16_t       ,   ht_cap);
#endif
#if LMAC_CONFIG_11AH == 1
DEC_VAL_GET_SET(uint8_t        ,   preamble_type);
DEC_VAL_GET_SET(bool           ,   bypass_mgmt);
DEC_VAL_GET_SET(uint8_t        ,   phy_txgain);
DEC_VAL_GET_SET(uint8_t        ,   phy_rxgain);
DEC_VAL_GET_SET(bool           ,   pv1_en);
DEC_VAL_GET_SET(uint64_t       ,   pn);
#endif
DEC_VAL_GET_SET(uint8_t        ,   format);
DEC_VAL_GET_SET(uint8_t        ,   mcs);
DEC_VAL_GET_SET(uint8_t        ,   short_gi);

DEC_ARR_GET_SET(uint8_t*       ,   macaddr);
DEC_ARR_GET_SET(uint8_t*       ,   destaddr);
DEC_ARR_GET_SET(uint8_t*       ,   bssid);
DEC_ARR_GET_SET(uint8_t*       ,   country);

DEC_COMMON_GET_SET(bool        ,   cfo_cal_en);
DEC_COMMON_GET_SET(bool        ,   cfo_apply_en);
DEC_COMMON_GET_SET(double      ,   cfo_avg);
DEC_COMMON_GET_SET(uint8_t     ,   cfo_count);

uint8_t* get_gtk_addr(uint8_t vif_id, uint8_t key_index);
void set_gtk_addr(uint8_t vif_id, uint8_t key_index,  uint8_t *addr);

typedef struct {
	union {
		void* 		address;
		struct {
			uint8_t         vif_id;
			uint8_t         sub_event;
		};

	};
    uint32_t    tim_tsf;
	LMacEvent	event;

} LMAC_SIGNAL;
#define LMAC_SINGNAL_RCD_MAX (100)
typedef struct{
uint8_t tail;
uint8_t head;
int16_t m_rssi_rcd[LMAC_SINGNAL_RCD_MAX];
int16_t m_snr_rcd[LMAC_SINGNAL_RCD_MAX];
}signal_rcd;

typedef enum{
	WINAC_IRQ = 0,
	TXDONE_IRQ,
	MPDU_CNT,
	PSDU_CNT
}HMAC_STATS;

typedef struct lmac_info {
    uint8_t         m_irq_vector;
	COMMON_VAR      m_common;
	VIF				m_vif[VIF_MAX];
    /// stats
    uint8_t         m_txpower_index_per_mcs[9];

	uint32_t		m_irq_winac_bitmap;
	uint32_t		m_irq_txdone_bitmap;
    uint32_t        m_irq_txdone_count;
	uint32_t		m_discard_count;
	uint32_t        m_total_rx_byte;
    int32_t         m_total_beacon_rssi;
    int32_t         m_total_beacon_cnt;
    uint32_t        m_total_length;
    uint16_t        m_total_bw;
    int16_t         m_total_mcs;
    uint16_t        m_total_gi;
    uint16_t        m_total_doppler;
    uint32_t        m_total_symbol;
    uint16_t        m_rssi_zero_cnt;
    uint16_t        m_info_error_cnt;    
    uint16_t        m_vector_cross_cnt;
    uint16_t        m_vector_fix_cnt;

    uint16_t        m_wa_tx_result_reset;
    uint16_t        m_wa_tx_result_fail;
    uint16_t        m_wa_tx_result_success;

	bool			m_lmac_sniffer_mode;
	bool			m_lmac_sniffer_mode_beacon;
#if LMAC_CONFIG_11AH == 1
    uint8_t         m_aifsn_offset;  
#endif
	uint8_t			m_rssi_isbase_mac;
	uint8_t     	m_rssi_base_address[6];
	signal_rcd 		m_sig_rcd;
	uint8_t 		m_sig_open;
	uint32_t 		m_irq_soft_rxdone_count;
	uint32_t 		m_irq_soft_txdone_count;
	uint32_t		m_lmactask_count;
	uint32_t 		m_to_upper_cnt;
	uint32_t		m_dl_beacon_count;
	uint32_t		m_dl_probe_req_count;
	uint32_t		m_dl_probe_rsp_count;
	uint32_t		m_dl_auth_count;
	uint32_t		m_dl_asso_count;
	uint32_t 		m_dl_data_count;
} LMAC_INFO;

extern LMAC_INFO           g_lmac_info;
extern LMAC_TEST_CFG       g_lmac_test_config;
extern QueueHandle_t       xLmacTaskQueue;

#define VIF(x) g_lmac_info.m_vif[x]

#if (LMAC_CONFIG_FREERTOS == 1)
extern      QueueHandle_t               g_lmac_queue_handle;
extern      TaskHandle_t	            g_lmac_test_task_handle;
extern      TaskHandle_t                g_lmac_task_handle;
#endif

// HAL Function
struct      LMacCipher; // declare

void        hal_lmac_init                   (void);
void        hal_lmac_start                  (void);
void        hal_lmac_idle_hook              (void);
void        hal_lmac_set_dl_callback        (void (*dl_callback)(struct _SYS_BUF*) );
void        hal_lmac_set_tbtt_callback      (void (*tbtt_callback)(int8_t) );
void        hal_lmac_mask_tbtt_irq          (void);
void        hal_lmac_unmask_tbtt_irq        (void);
void        hal_lmac_set_beacon_interval    (uint16_t beacon_interval);
uint16_t 	hal_lmac_get_beacon_interval();

void        hal_lmac_set_credit_callback ( void (*credit_callback)(uint8_t ac, uint8_t value, bool inc) );
void        hal_lmac_isr                  (int vector);
void        hal_lmac_uplink_request_sysbuf(struct _SYS_BUF* buf, struct frame_tx_info *ti);
int8_t      hal_lmac_get_vif_id           (GenericMacHeader* gmh);

void        hal_lmac_set_mac_address      (int vif_id , uint8_t* addr);
uint8_t*    hal_lmac_get_mac_address      (int vif_id);
void        hal_lmac_set_enable_mac_addr  (int vif_id, bool enable);
bool        hal_lmac_get_enable_mac_addr  (int vif_id);
void        hal_lmac_set_bssid            (int vif_id, uint8_t *bssid);
uint8_t*    hal_lmac_get_bssid            (int vif_id);
void        hal_lmac_set_enable_bssid     (int vif_id, bool enable);
bool        hal_lmac_get_enable_bssid     (int vif_id);
void        hal_lmac_mode_bssid           (int vif_id, bool ap_mode_en);
bool        hal_lmac_set_aid              (int vif_id, uint16_t aid);
int         hal_lmac_get_aid              (int vif_id);
bool        hal_lmac_set_mode             (int vif_id, uint8_t mode);
bool        hal_lmac_is_ap                (int vif_id);
bool        hal_lmac_is_sta               (int vif_id);
void        hal_lmac_set_cca_ignore       (bool ignore);
void        hal_lmac_set_promiscuous_mode (int vif_id, uint8_t);
void        hal_lmac_set_promiscuous_filter(void *arg);
void        hal_lmac_set_promiscuous_enable(int vif_id, uint8_t enable);
bool        hal_lmac_get_bypass_mgmt_mode (int vif_id);
void		hal_lmac_set_bypass_mgmt_mode (int vif_id, bool);
bool        hal_lmac_set_erp_param        (int vif_id, uint8_t, uint8_t, uint8_t);
int         hal_lmac_get_cipher_icv_length(int type);
bool        hal_lmac_sec_set_enable_key   (int vif_id, bool enable);
bool        hal_lmac_sec_get_enable_key   (int vif_id);
bool        hal_lmac_sec_add_key          (int vif_id, struct LMacCipher *lmc, bool dummy);
bool        hal_lmac_sec_del_key          (int vif_id, struct LMacCipher *lmc);
bool        hal_lmac_sec_del_key_all      (int vif_id);
bool        hal_lmac_set_basic_rate       (int vif_id, uint32_t basic_rate_set);
uint32_t    hal_lmac_get_tsf_timer_high   (int vif_id);
uint32_t    hal_lmac_get_tsf_timer_low    (int vif_id);
void        hal_lmac_set_tsf_timer        (int vif_id, uint32_t ts_hi, uint32_t ts_lo);
void        hal_lmac_set_response_ind     (int ac, uint8_t response_ind);
uint8_t     hal_lmac_get_response_ind     (int ac);
void        hal_lmac_set_max_agg_num(int ac, int num);
uint8_t     hal_lmac_get_max_agg_num(int ac);
void        hal_lmac_set_aggregation      (int ac, bool aggregation);
bool        hal_lmac_get_aggregation      (int ac);
void        hal_lmac_reset_aggregation(void);
bool        hal_lmac_set_ht_operation_info(int vif_id, uint16_t ht_info);
bool		hal_lmac_set_ht_capability(int vif_id, uint16_t ht_cap);
bool 		hal_lmac_set_format(int vif_id, uint8_t format);
bool 		hal_lmac_set_short_gi(int vif_id, uint8_t short_gi);
void        hal_lmac_enable_1m            (bool enable, bool enable_dup);
bool        hal_lmac_get_scanning(int vif_id);
void        hal_lmac_set_scanning(int vif_id, bool en);
void 		hal_lmac_set_rts(int vif_id, uint8_t mode, uint16_t threshold);
int8_t 		hal_lmac_get_ap_vif_id ();
int8_t 		hal_lmac_get_sta_vif_id ();
void        hal_lmac_set_pv1              (int vif_id, bool en);
bool        hal_lmac_get_pv1              (int vif_id);
void        hal_lmac_set_pn               (int vif_id, uint64_t pn);
uint64_t    hal_lmac_get_pn               (int vif_id);

void        hal_lmac_common_set_cfo_cal_en       (bool en);
bool        hal_lmac_common_get_cfo_cal_en       ();

void        lmac_uplink_request (LMAC_TXBUF *buf, struct frame_tx_info *ti);

void        lmac_contention_result_isr  ();
void        lmac_tx_done_isr            ();
void        lmac_rx_done_isr            ();
void        lmac_tsf_isr				(uint8_t );
void        lmac_tbtt_isr               ();

bool        lmac_qm_transit             (uint8_t, uint16_t);
bool        lmac_process_tx_report      (uint8_t);	/// Processing tx result from hardware module

void        lmac_free_txbuffer(LMAC_TXBUF* buffer, uint8_t ac);

/// internal utility
bool        lmac_check_fragmented_frame(GenericMacHeader* gmh);
bool        lmac_send_null_frame        ();
void        lmac_rx_check_more_data     (struct _SYS_BUF*);
void		lmac_rx_check_beacon_tim(struct _SYS_BUF* buffer, uint32_t tim_tsf);
void        lmac_check_legacy_tim       (uint8_t *ie);
void        lmac_qm_show_statistics     ();
void        lmac_dl_show_statistics     ();
void        lmac_qm_initialize          ();
void        lmac_qm_configure           ();
void        lmac_qm_schedule            (uint8_t, uint8_t);
void        lmac_rx_dl_initialize       ();
bool 	    lmac_rx_post_process(struct _SYS_BUF* buffer, uint32_t tim_tsf);

uint32_t    lmac_bytes_to_bufend(struct _SYS_BUF* sys_buf, uint8_t *ptr);

void        discard                     (struct _SYS_BUF* buffer);
uint16_t    bd_set_datalen              (LMAC_TXBUF *, uint16_t, bool);
int         set_buffer_descriptor       (LMAC_TXBUF *);
int         set_tx_vector               (LMAC_TXBUF *, uint8_t, bool, struct frame_tx_info*);
uint32_t    get_valid_dl_desc_num       ();
void        fill_descriptor_all         (struct _SYS_BUF*);
void        fill_descriptor             (struct _SYS_BUF*);

void 		lmac_print_sys_buf(struct _SYS_BUF *buffer);
void 		lmac_set_sniffer_mode(bool v);
void		lmac_set_sniffer_mode_beacon_display(bool v);

#if LMAC_CONFIG_11AH == 1
bool s1g_check_bw (uint8_t bw, uint8_t format);
bool s1g_check_preamble (uint8_t preamble_type);
bool lmac_check_s1g_ndp_frame(LMAC_TXBUF *buf);
uint16_t get_sig_length(TXVECTOR* vector);
bool get_sig_aggregation(TXVECTOR* vector);
uint8_t get_sig_short_gi(TXVECTOR* vector);
uint8_t get_sig_mcs(TXVECTOR* vector);
void set_sig_length(TXVECTOR* vector, uint16_t len);
void set_sig_aggregation(TXVECTOR* vector, bool agg);
void set_sig_short_gi(TXVECTOR* vector, uint8_t gi);
void set_sig_mcs(TXVECTOR* vector, uint8_t mcs);
void set_sig_doppler(TXVECTOR* vector , uint8_t doppler);
#endif
uint32_t hal_lmac_get_freq(int vif_id);
void lmac_update_stats(uint8_t dir, void *hdr, uint8_t mcs, uint8_t status);
extern int  hal_cal_param_get(unsigned int addr, int length, char * buf);
int8_t get_txpower_from_nv(uint32_t chl);
bool lmac_set_channel_width(int vif_id, uint8_t chan_width, uint8_t prim_loc);
void hal_lmac_set_retry_limit(int32_t vif_id,uint32_t retry_limit);
uint8_t *macstr2int(char *mac, uint8_t *mac_int);
#endif

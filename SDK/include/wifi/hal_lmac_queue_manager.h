#ifndef HAL_LMAC_QUEUE_MANAGER_H_
#define HAL_LMAC_QUEUE_MANAGER_H_
#define	QM(x)   	                (m_qm[x])
#define	EDCA(x)                     (m_qm[x])->m_edcaparam
#define QUEUE(x, y)                 (m_qm[x])->m_queue[y]

#define DEC_EDCA_GET_SET(t, x) \
	t get_##x(uint8_t);\
	void set_##x(uint8_t, t);

#define DEF_EDCA_GET_SET(t, x) \
	t get_##x(uint8_t aci ) { \
		return EDCA(aci).m_##x; \
	}; \
	void set_##x(uint8_t aci, t value) { \
        EDCA(aci).m_##x = value; \
	}

DEC_EDCA_GET_SET(uint8_t    ,   acid);
DEC_EDCA_GET_SET(uint8_t    ,   acm);
DEC_EDCA_GET_SET(uint8_t    ,   aifsn);
DEC_EDCA_GET_SET(uint32_t   ,   cw_min);
DEC_EDCA_GET_SET(uint32_t   ,   cw_max);
DEC_EDCA_GET_SET(uint32_t   ,   txop_max);

typedef enum txmode {
	SIFS,
	BACKOFF,
	MAX_MODE
} TXMODE;

typedef enum qmstate {
	WAIT_FRAME,
	WAIT_CHANNEL,
	WAIT_ACK,
	CHANNEL_READY,
	STATE_MAX,
} QMSTATE;

typedef enum qtype {
	    RE_TX,          /// Retransmission Queue
	    NEW_TX,         /// MPDU queue from UMAC
	    PENDING,        /// Pending Queue
		MAX_Q,
} QTYPE;

/// statistics for monitoring QM
typedef enum qm_statistic {
		Q_RE_TX,
		Q_NEW_TX,
		Q_PENDING,
	    TOTAL_QUEUED,
	    TOTAL_BYTE,
	    REMAIN_BYTE,
	    ISSUED_TX,
	    RETX_QUEUED,
	    SENT_MPDU,
	    SENT_AMPDU,
	    SENT_PSDU,
	    TX_SUCCESS,
	    TX_FAIL,
	    N_BACKOFF_ISSUED,
	    N_SIFS_ISSUED,
	    N_MPDU_FREE,
	    N_DISCARD,
	    N_UL_REQUEST,
	    N_ACK_RECEIVED,
	    MAX_STATS,
} STATISTIC;


struct Queue {
    	LMAC_TXBUF *m_tail;
    	uint16_t    m_depth;
    	uint16_t    m_max_depth;

		uint8_t     n_sched;
		uint32_t    n_bytes;
};

struct q_edcaparam {
	/// EDCA QoS parameters for uplink tx
	uint8_t  m_acid;
	uint8_t  m_acm;
	uint8_t  m_aifsn;                       /// AIFSN
	uint16_t m_cw_min;                      /// Minimum CW
	uint16_t m_cw_max;                      /// Maximum CW
	uint16_t m_cw;                          /// current CW
	uint32_t m_txop_max;                    /// Maximum TXOP limit
	uint32_t m_txop_limit;
};

typedef struct _Sched_Info {
		uint8_t  valid      : 1;
		uint8_t  aggregated : 1;
		uint8_t  mpdu_cnt   : 6;
		uint16_t ssn        : 16;
#if     LMAC_CONFIG_11N == 1
		uint8_t mode;
		uint32_t queue;
		uint8_t n_max_mpdu;
		uint8_t n_mpdu;
		uint32_t* ul_max_table;

		uint32_t psdu_len;
		uint32_t tot_duration;

        LMAC_TXBD* bd;
        TXVECTOR*  vector;
#endif
} Sched_Info;

typedef struct queuemanager {

	/// Internal queues
	struct Queue m_queue[MAX_Q];
	/// State
	uint8_t m_state;
	/// Schedule& Pending Info
	Sched_Info sched_info, pend_info;
	/// Flags
	bool m_pend_info_copied;
	bool m_sched_failed;
	bool m_delayed_clear;

	/// Schedule list
	LMAC_TXBUF *m_sched_head;
	LMAC_TXBUF *m_sched_list;
	uint16_t m_sched_list_depth;
	/// Free list
	LMAC_TXBUF *m_free_head;
	LMAC_TXBUF *m_free_list;
	uint16_t m_free_list_depth;

	uint16_t m_to_be_clear_depth;
	uint16_t m_current_pending_depth;

	uint16_t m_fail_mpdu_cnt;

	uint32_t m_addr_bo_command;
	uint32_t m_addr_txop_command;

	uint8_t m_max_credit; // credit

	uint16_t m_sn;                          /// Next sequence number
	uint8_t m_max_aggregated_num;

	struct q_edcaparam m_edcaparam;

	uint8_t  m_response_ind;
	bool	 m_aggregation;

	uint8_t  m_priority;                    /// priority
	uint8_t  m_vif_id;

	uint8_t  m_max_tx_count;
	/// Statistics
    uint32_t m_ac_transmitted;
    uint16_t m_n_state[STATE_MAX];
	uint16_t m_stats[MAX_STATS];
	uint32_t m_transmitted_data;
    uint32_t m_hw_tx_bytes;
#ifndef RELEASE
    uint16_t m_n_aggregation[STAT_AGG_MAX];
#endif
	uint32_t timestamp;
} QUEUEMANAGER;
extern QUEUEMANAGER	*m_qm[MAX_AC];
void enqueue_lifo(LMAC_TXBUF* item, uint8_t ac, uint8_t qtype);
LMAC_TXBUF* queue_peek(uint8_t ac, uint8_t qtype);
bool queue_find(LMAC_TXBUF *p, uint8_t ac, uint8_t qtype);
void enqueue_fifo(LMAC_TXBUF* item, uint8_t ac, uint8_t qtype);
LMAC_TXBUF* queue_dequeue(uint8_t ac, uint8_t qtype);
LMAC_TXBUF* queue_bulk_dequeue(uint16_t depth, uint8_t ac, uint8_t qtype);
bool queue_remove(LMAC_TXBUF *item, uint8_t ac, uint8_t qtype);
void queue_mqfree(uint8_t ac, uint8_t qtype);
void queue_show(uint8_t ac, uint8_t qtype);
bool queue_append(LMAC_TXBUF *append_tail, uint16_t depth, bool pos, uint8_t ac, uint8_t qtype);

void lmac_qm_clear_sched(uint8_t ac);
void lmac_qm_flush_pending_queue(uint8_t ac, bool mode);
void lmac_qm_staggered_free(uint8_t ac);
bool lmac_qm_append_pend_queue(uint8_t ac);
void lmac_qm_restore_queue(uint8_t ac);

void lmac_qm_initialize();
void lmac_qm_configure();
void lmac_qm_show_statistics();

void lmac_qm_set_segment_duration(uint8_t ac, uint16_t duration);

#if	LMAC_CONFIG_11N == 1
LMAC_TXBUF* peek_tx_queue(uint32_t* queue, uint8_t ac);
bool schedule_one_mpdu(LMAC_TXBUF *buffer, uint8_t ac, uint8_t mode);
LMAC_TXBUF* hw_build_ppdu(uint8_t mode, uint8_t ac);
static uint32_t mk_tx_command(uint32_t oft, uint32_t aifsn, uint32_t cw);
static void hw_set_NAN( LMAC_TXBUF *bd, uint32_t addr);
void lmac_bd_print(LMAC_TXBUF* buf,uint8_t type);
void lmac_vector_print(LMAC_TXBUF* buf , uint8_t type);
#endif


void lmac_qm_schedule(uint8_t ac, uint8_t mode);
bool lmac_qm_transit(uint8_t ac, uint16_t state);
bool lmac_process_tx_report(uint8_t ac);


void hal_lmac_qm_init();

#endif /* HAL_LMAC_QUEUE_MANAGER_H_ */

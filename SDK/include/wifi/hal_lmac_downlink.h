#ifndef HAL_LMAC_DOWNLINK_H_
#define HAL_LMAC_DOWNLINK_H_

#define SN_WINDOW_SIZE 4
typedef struct _snmanager {
	uint8_t origin_addr[6];
	uint8_t tid : 4;
	uint16_t win_end : 12;
	uint16_t win_start : 12;
	uint8_t win_bitmap : SN_WINDOW_SIZE;
} __attribute__((packed)) SNMANAGER;

struct lmac_rx_h_data {
	int8_t vif_id;
    struct _SYS_BUF *buffer;
	struct _LMAC_RXBUF *vector;
	GenericMacHeader *mh;
};

typedef	enum dl_statistic{
	    DL_GOOD,
	    DL_ERR_CRC,
	    DL_ERR_MATCH,
	    DL_ERR_LEN,
	    DL_ERR_KEY,
	    DL_ERR_MIC,
	    DL_ERR_DESC,
	    DL_ERR_NOIND,
	    DL_UNDER_RUN,
	    DL_OVER_RUN,                       // this is for statistics only
	    DL_BUFFER_FULL,
	    MAX_DL_STATS                    // this is just a marker
} DL_STATISTIC;

typedef struct desc_ring {
	uint32_t    m_start;            ///< address of first descriptor
	uint32_t    m_end;              ///< address of last descriptor
	uint32_t    m_current;          ///< address of current descriptor
	uint32_t    m_current_desc;		///< address of current descriptor for including further information
	uint32_t    m_size;             ///< Register size
	uint32_t    m_base;             ///< Base address of buffer

	struct      _SYS_BUF *m_tail;   ///< Tail of unfinished dl/rx list
	uint32_t    m_head;             ///< First fragment of tx list
	uint32_t	m_fill;				///< Fill the Buffer to tail
	uint32_t	m_fill_desc;		///< Fill the Buffer to tail for including further information

	uint8_t     m_type;                             ///< Type of Descriptor ring
	uint8_t 	m_stats[MAX_DL_STATS];              ///< Descriptor error counts
	bool        m_valid[LMAC_CONFIG_DL_DESCRIPTOR]; ///< Prevent wrap over of ring

} DESC_RING;

SYS_BUF* rx_read_dl_desc();
void nrc7291_rxvect_rotate_war(struct lmac_rx_h_data *rx);
void lmac_get_rxsignal_info(void);
uint32_t* get_sysbuff_addr(uint32_t desc);
bool lmac_check_queue_empty();

#endif /* HAL_LMAC_DOWNLINK_H_ */

#ifndef HAL_LMAC_11AH_H
#define HAL_LMAC_11AH_H

#define MAX_PPDU_NUMBER             16
#define MAX_AGG_SCHED_NUMBER        16
#define STAT_AGG_MAX                16

#define NOW     (*(volatile uint32_t*)(MAC_REG_TSF_TIME_RO_LOWER))
#define NOW1     (*(volatile uint32_t*)(MAC_REG_TSF_TIME_RO_LOWER)) // FIXME
#define TSF     (*(volatile uint32_t*)(MAC_REG_TSF_TIME_RO_LOWER))
#define TSF1    (*(volatile uint32_t*)(MAC_REG_TSF_TIME_RO_LOWER))  // FIXME

#define SW_OWNED(desc)          (!((desc) & MAC_REG_DL_DESC_CONTROL_0_OWNER_MASK))
#define HW_OWNED(desc)          ((desc) & MAC_REG_DL_DESC_CONTROL_0_OWNER_MASK)
#define FIRST_FRAGMENT(desc)    (!((desc) & (1 << MAC_REG_DL_DESC_CONTROL_0_FRAGMENT_SHIFT)))
#define LAST_FRAGMENT(desc)     (!((desc) & (1 << (MAC_REG_DL_DESC_CONTROL_0_FRAGMENT_SHIFT + 1))))
// macro for address calculation
#define BUFFER_ADDRESS(base, offset)    ((base) + ((offset) << 2))
#define OFFSET_ADDRESS(base, addr)      ((((addr) - (base)) >> 2) & MAC_REG_DL_DESC_BUFFER_0_ADDR_OFFSET_MASK)


// 802.11ah specific enumeration
enum {
    MAX_SEQUENCE_NUMBER = 1 << 12,
};
enum {
    BW_1M = 0,
    BW_2M,
    BW_4M,
    BW_MAX,

    BW_8M = 3,
    BW_16M,
};

enum {
    PR_CH_WIDTH_2M = 0,
    PR_CH_WIDTH_1M = 1,
};

enum {
    OP_CH_WIDTH_1M  = 0,
    OP_CH_WIDTH_2M  = 1,
    OP_CH_WIDTH_4M  = 3,
    OP_CH_WIDTH_8M  = 7,
    OP_CH_WIDTH_16M = 15,
};

enum {
    PER_SYMBOL  = 40,
    PER_SYMBOL_DATA_NORMAL_GI = 40,
    PER_SYMBOL_DATA_SHORT_GI = 36,
    SIFS_DURATION        = 160,
    SLOT_DURATION		 = 52,
    L_LTF         = 80,
    L_STF         = 80,
    L_SIG         = 40,
    NUM_SYM_1M			= 14,
    NUM_SYM_2M			=  6,
    NDP_CMAC_1M			= NUM_SYM_1M * PER_SYMBOL,	//	S1G_1M format, 560usec
    NDP_CMAC_2M 		= NUM_SYM_2M * PER_SYMBOL,	//	S1G_SHORT or S1G_LONG format, 240usec
    MAX_DURATION		= 27840,			//  Maximum data unit durations (usec unit, S1G PPDU duration)
    S1G_1M_HEADER       = L_STF * 2 + L_LTF * 2 + L_SIG * 6,
    S1G_SHORT_HEADER    = L_STF + L_LTF + L_SIG * 2,
};

enum {
    SIG_SIZE            = 8,

    NORMAL_HEADER_SIZE	= 4,
    PV1_HEADER_SIZE		= 2,

    SERVICE_BIT         = 8,
    TAIL_BIT            = 6,
    SERVICE_TAIL_BIT    = SERVICE_BIT + TAIL_BIT,

    MPDU_LEN_THRESHOLD  = 511,
    MAX_MPDU_LEN        = (1 << 11) - 1,
    MAX_S1G_PSDU_LEN     = 65535,

    MAX_MCS = 11,
};


enum {
    CODING_BCC = 0,
    CODING_LDPC = 1,
};

enum {
	    SERVICE_SCRAMBLER_TAIL 	= 4,

};

enum {
	    QUEUE_REAR,
	    QUEUE_FRONT
};

///=============================================================================
///
/// Duration Calculation
///
///=============================================================================

/*
MCS table
----------------------------------------------------------------
MCS index   Modulation  Code rate   N_DBPS(1M)	N_DBPS(2M)	N_DBPS(4M)
----------------------------------------------------------------
    0           BPSK        1/2         12          26			54
    1           QPSK        1/2         24          52			108
    2           QPSK        3/4         36          78			162
    3           16QAM       1/2         48          104			216
    4           16QAM       3/4         72          156			324
    5           64QAM       2/3         96          208			432
    6           64QAM       3/4         108         234			486
    7           64QAM       5/6         120         260			540
    8           256QAM      3/4         144         312			648
    9           256QAM      5/6         160						720
    10          BPSK        1/4         6
-----------------------------------------------------------
*/

static const uint16_t bits_per_symbol[BW_MAX][MAX_MCS] = {
	{12,  24,  36,  48,  72,  96,  108, 120, 144, 160, 6},
	{26,  52,  78,  104, 156, 208, 234, 260, 312, 0,   0},
	{54,  108, 162, 216, 324, 432, 486, 540, 648, 720, 0}
};

// max_ul_bytes = (max_symbol(511) * bps - SERVICE_TAIL_BIT) / 8
static const uint16_t max_ul_bytes[BW_MAX][MAX_MCS] = {
	{ 764, 1531,  2297,  3064,  4597,  6130,  6896,  7663,  9196,  10218, 509},
	{1659, 3319,  4980,  6641,  9962,  13284, 14945, 16605, 19927, 0,     0  },
	{3447, 6896,  10346, 13795, 20693, 27592, 31041, 34490, 41389, 45988, 0  }
};

uint16_t compute_duration(uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint32_t, uint8_t);
uint16_t compute_symbol(uint16_t, uint8_t, uint8_t);

enum {
	S1G = 0,
	S1G_DUP_1M,
	S1G_DUP_2M
};

enum {
	S1G_SHORT = 0,
	S1G_LONG
};

enum {
    RID_NO_RESPONSE = 0,
    RID_NDP_RESPONSE,
    RID_NORMAL_RESPONSE,
    RID_LONG_RESPONSE,
    RID_MAX
};

#define VIF_MAX 2
typedef enum {
    ACI_BK = 0,
    ACI_BE,
    ACI_VI,
    ACI_VO,
    ACI_BEACON,
    ACI_GP,
    ACI_MAX
} ACI;
typedef enum {
    VIF0_BK = 0,
    VIF0_BE,
    VIF0_VI,
    VIF0_VO,
    VIF0_BEACON,
    VIF0_GP,
    MAX_AC
} VIFAC;

static const uint8_t AIFSN[MAX_AC] = { 7,  3,  2,  2,  1,  3};
static const uint8_t CWMIN[MAX_AC] = { 4,  4,  3,  2,  0,  4};
static const uint8_t CWMAX[MAX_AC] = {10, 10,  4,  3,  0,  10};

// [TODO] confirm the value of AC_GP and BEACON
// the unit of this value is microsecond.
// TXOP Limit complies with 802.11ah D10.0 Specifiacation
//static const uint16_t TXOPLIMIT[MAX_AC] = { 0, 0, 0, 0, 0, 0 };
//static const uint16_t TXOPLIMIT[MAX_AC] = { 2528, 2528, 4096, 2080, 2528, 2528};
static const uint16_t TXOPLIMIT[MAX_AC] = { 32736, 0, 32736, 32736, 0, 0};
//static const uint16_t TXOPLIMIT[MAX_AC] = { 0, 0, 3008, 1504, 2528, 2528};


typedef struct _TXVECTOR{
    // Exist only at first fragment (segment == 2'b01)
    // Vector
	uint32_t format         : 2;        // 0=S1G, 1=S1G_DUP_1M, 2=S1G_DUP_2M
	uint32_t bandwidth      : 2;        // 0=1M, 1=2M, 2=4M, 3=8M
	uint32_t preamble_type  : 1;        // 0=S1G_SHORT, 1=S1G_LONG
	uint32_t scrambler      : 7;
	uint32_t scrambler_exist: 1;
	uint32_t tx_pwr_level   : 8;
	uint32_t reserved0      : 11;

    // SIG0 & SIG1
	union {
		SigS1G1M 	s1g_1m;
		SigS1GShort 	s1g_short;
		SigS1GLong   s1g_long;
		uint32_t word[2];
	};

} TXVECTOR , PHY_TXVECTOR;

typedef struct _LMAC_TXHDR {
	// Total byte size = 36 (9 Words)
    // Word 0
    uint8_t     pool_id;
    uint8_t     vif_id;
    uint8_t     temp0[2];

    // Word 1
    uint32_t    temp1;
    // Word 2
    struct _LMAC_TXBUF  *m_next;
    // Word 3
    struct _LMAC_TXBUF  *m_link;
    union {
        struct {
            // Word 4 : BD info 0 (Valid only at first Buffer)
        	uint32_t    reserved1                    : 6;
        	uint32_t    ack_policy                   : 2;        // 00: Normal Ack, 01: No Ack
        	uint32_t    cipher_type                  : 3;        // 0:wep40, 1:wep128, 2:tkip, 3:ccmp, 4:wapi
        	uint32_t    rts_request                  : 1;
        	uint32_t    cts_self_request             : 1;
        	uint32_t    cf_end_request               : 1;
        	uint32_t    key_location                 : 4;        // security key location
        	uint32_t    timestamp_update             : 1;        // flag for updating timestamp in the frame
        	uint32_t    timestamp_position           : 6;        // in byte, timestamp position in frame
        	uint32_t	bssid_idx                    : 2;
        	uint32_t	mac_idx                      : 2;
        	uint32_t    reserved2                    : 3;

            // Word 5 : BD info 1
        	uint32_t    rts_duration                 : 16;
        	uint32_t    reserved3                    : 16;

            // Word 6 : BD info 2
        	uint32_t    psdu_length                  : 20;       /////
        	uint32_t    bcn_compatible_pos           : 5;
        	uint32_t    bcn_compatible_update        : 1;
        	uint32_t    tetra_partial_tsf_update     : 1;
        	uint32_t    penta_partial_tsf_update     : 1;
        	uint32_t    reserved4                    : 4;

            // Word 7 : BD info 3
	        uint32_t    mpdu_length         : 14;
	        uint32_t    ampdu_segment       : 2;        // 00:single, 10:first, 11:middle, 01:last
	        uint32_t    mac_header_length   : 11;
	        uint32_t    ampdu_spacing       : 5;

            // Word 8 : BD info 4
	        uint32_t    reserved5	        : 8;    // data start position from BD start (in bytes)
	        uint32_t    data_length         : 11;   // data length
	        uint32_t    segment             : 2;    // MPDU segmentation
	        uint32_t    reserved6           : 3;
	        uint32_t    tid                 : 4;	// LMAC FW use only, Assigned Queue Manager ID
	        uint32_t    tx_count            : 4;    // LMAC FW use only, check whether it exceeds retry limit
        };
        uint32_t    bd[5];
    };
} LMAC_TXHDR;

typedef struct _LMAC_TXBD {
    union {
        struct {
            // BD info #0
        	uint32_t    reserved1                    : 6;
        	uint32_t    ack_policy                   : 2;        // 00: Normal Ack, 01: No Ack
        	uint32_t    cipher_type                  : 3;        // 0:wep40, 1:wep128, 2:tkip, 3:ccmp, 4:wapi
        	uint32_t    rts_request                  : 1;
        	uint32_t    cts_self_request             : 1;
        	uint32_t    cf_end_request               : 1;
        	uint32_t    key_location                 : 4;        // security key location
        	uint32_t    timestamp_update             : 1;        // flag for updating timestamp in the frame
        	uint32_t    timestamp_position           : 6;        // in byte, timestamp position in frame
        	uint32_t	bssid_idx                    : 2;
        	uint32_t	mac_idx                      : 2;
        	uint32_t    reserved2                    : 3;

            // BD info #1
        	uint32_t    rts_duration                 : 16;
        	uint32_t    reserved3                    : 16;

            // BD info #2
        	uint32_t    psdu_length                  : 20;       /////
        	uint32_t    bcn_compatible_pos           : 5;
        	uint32_t    bcn_compatible_update        : 1;
        	uint32_t    tetra_partial_tsf_update     : 1;
        	uint32_t    penta_partial_tsf_update     : 1;
        	uint32_t    reserved4                    : 4;

            // BD info #3
	        uint32_t    mpdu_length         : 14;
	        uint32_t    ampdu_segment       : 2;        // 00:single, 10:first, 11:middle, 01:last
	        uint32_t    mac_header_length   : 11;
	        uint32_t    ampdu_spacing       : 5;

            // BD info #4
	        uint32_t    reserved5	        : 8;    // data start position from BD start (in bytes)
	        uint32_t    data_length         : 11;   // data length
	        uint32_t    segment             : 2;    // MPDU segmentation
	        uint32_t    reserved6           : 3;
	        uint32_t    tid                 : 4;	// LMAC FW use only, Assigned Queue Manager ID
	        uint32_t    tx_count            : 4;    // LMAC FW use only, check whether it exceeds retry limit
        };
        struct {
            uint32_t    bd[5];
        };
    };

    union {
    	struct {
    		// Vector
    		uint32_t format         : 2;        // 0=S1G, 1=S1G_DUP_1M, 2=S1G_DUP_2M
    		uint32_t bandwidth      : 2;        // 0=1M, 1=2M, 2=4M, 3=8M
    		uint32_t preamble_type  : 1;        // 0=S1G_SHORT, 1=S1G_LONG
    		uint32_t scrambler      : 7;
    		uint32_t scrambler_exist: 1;
    		uint32_t tx_pwr_level   : 8;
    		uint32_t reserved0      : 11;

    		// SIG0 & SIG1
    		union {
    			SigS1G1M 	s1g_1m;
    			SigS1GShort 	s1g_short;
    			SigS1GLong   s1g_long;
    			uint32_t word[2];
    		};

            GenericMacHeader  machdr[0];

    	};
      uint32_t          vector[3];
      uint8_t           payload[0];
    };
} LMAC_TXBD;

typedef struct _LMAC_TXBUF {
    //Word 0
    uint8_t     temp0[3];
    uint8_t     vif_id;

    //Word 1
    uint32_t    temp1;

    //Word 2
    struct _LMAC_TXBUF  *m_next;

    //Word 3
    struct _LMAC_TXBUF  *m_link;

    union {
        struct {
            // Word 4 : BD info 0 (Valid only at first Buffer)
        	uint32_t    reserved1                    : 6;
        	uint32_t    ack_policy                   : 2;        // 00: Normal Ack, 01: No Ack
        	uint32_t    cipher_type                  : 3;        // 0:wep40, 1:wep128, 2:tkip, 3:ccmp, 4:wapi
        	uint32_t    rts_request                  : 1;
        	uint32_t    cts_self_request             : 1;
        	uint32_t    cf_end_request               : 1;
        	uint32_t    key_location                 : 4;        // security key location
        	uint32_t    timestamp_update             : 1;        // flag for updating timestamp in the frame
        	uint32_t    timestamp_position           : 6;        // in byte, timestamp position in frame
        	uint32_t	bssid_idx                    : 2;
        	uint32_t	mac_idx                      : 2;
        	uint32_t    reserved2                    : 3;

            // Word 5 : BD info 1
        	uint32_t    rts_duration                 : 16;
        	uint32_t    reserved3                    : 16;

            // Word 6 : BD info 2
        	uint32_t    psdu_length                  : 20;       /////
        	uint32_t    bcn_compatible_pos           : 5;
        	uint32_t    bcn_compatible_update        : 1;
        	uint32_t    tetra_partial_tsf_update     : 1;
        	uint32_t    penta_partial_tsf_update     : 1;
        	uint32_t    reserved4                    : 4;

            // Word 7 : BD info 3
	        uint32_t    mpdu_length         : 14;
	        uint32_t    ampdu_segment       : 2;        // 00:single, 10:first, 11:middle, 01:last
	        uint32_t    mac_header_length   : 11;
	        uint32_t    ampdu_spacing       : 5;

            // Word 8 : BD info 4
	        uint32_t    reserved5	        : 8;    // data start position from BD start (in bytes)
	        uint32_t    data_length         : 11;   // data length
	        uint32_t    segment             : 2;    // MPDU segmentation
	        uint32_t    reserved6           : 3;
	        uint32_t    tid                 : 4;	// LMAC FW use only, Assigned Queue Manager ID
	        uint32_t    tx_count            : 4;    // LMAC FW use only, check whether it exceeds retry limit
        };
        uint32_t    bd[5];
    };

    union {
    	struct {
    		// Vector
    		uint32_t format         : 2;        // 0=S1G, 1=S1G_DUP_1M, 2=S1G_DUP_2M
    		uint32_t bandwidth      : 2;        // 0=1M, 1=2M, 2=4M, 3=8M
    		uint32_t preamble_type  : 1;        // 0=S1G_SHORT, 1=S1G_LONG
    		uint32_t scrambler      : 7;
    		uint32_t scrambler_exist: 1;
    		uint32_t tx_pwr_level   : 8;
    		uint32_t reserved0      : 11;

    		// SIG0 & SIG1
    		union {
    			SigS1G1M 	s1g_1m;
    			SigS1GShort 	s1g_short;
    			SigS1GLong   s1g_long;
    			uint32_t word[2];
    		};

            GenericMacHeader  machdr[0];
    	};
      uint32_t          vector[3];
      uint8_t           payload[0];
    };
} LMAC_TXBUF;

/// 11ah RXINFO
typedef struct _RXINFO {
        // RXINFO #1
        uint32_t mpdu_length    	: 14;
        uint32_t center_freq      	: 18;
        // RXINFO #2
      	uint32_t format       		: 2;
       	uint32_t preamble_type		: 1;
       	uint32_t bandwidth        	: 2;
       	uint32_t scrambler_or_crc 	: 7;
       	uint32_t rcpi             	: 8;
        uint32_t obss_frame     : 1;
        uint32_t ndp_ind        : 1;
        uint32_t long_2m_ind    : 1;
        uint32_t aggregation    : 1;
        uint32_t error_mic      : 1;
        uint32_t error_key      : 1;
        uint32_t protection     : 1;
        uint32_t error_length   : 1;
        uint32_t error_match    : 1;
        uint32_t error_crc      : 1;
        uint32_t okay           : 1;
        uint32_t error_seq      : 1;
        // RXINFO #3
        uint32_t timestamp;
        // RXINFO #4
        uint32_t fcs;
} RXINFO;

typedef struct _RXINFO_LMAC {
        // RXINFO #1
        uint32_t mpdu_length    	: 14;
        uint32_t center_freq      	: 18;
        // RXINFO #2
       	uint32_t reserved	    	: 18;
		uint32_t error_all_delim	: 1;
		uint32_t eof_ind			: 1;
        uint32_t obss_frame     : 1;
        uint32_t ndp_ind        : 1;
        uint32_t long_2m_ind    : 1;
        uint32_t aggregation    : 1;
        uint32_t error_mic      : 1;
        uint32_t error_key      : 1;
        uint32_t protection     : 1;
        uint32_t error_length   : 1;
        uint32_t error_match    : 1;
        uint32_t error_crc      : 1;
        uint32_t okay           : 1;
        uint32_t error_seq      : 1;
        // RXINFO #3
        uint32_t timestamp;
        // RXINFO #4
        uint32_t fcs;
} RXINFO_LMAC;


typedef struct _LMAC_RXBUF {
    union {
        struct {
        	// RxSIG
        	union {
        		SigS1G1M     s1g_1m;
        		SigS1GShort  s1g_short;
        		SigS1GLong   s1g_long;
        		uint32_t word[2];
        	};

        	// RXVECTOR #1
        	uint32_t format         : 2;
        	uint32_t preamble_type  : 1;
        	uint32_t reserved1      : 1;
        	uint32_t bandwidth      : 2;
        	uint32_t reserved2      : 1;
        	uint32_t obss_indication: 1;
        	uint32_t length    : 20;
        	uint32_t reserved3      : 4;

        	// RXVECTOR #2
        	uint32_t scrambler_or_crc   : 7;
        	uint32_t snr          		: 9;
        	uint32_t reserved4          : 16;

        	// RXVECTOR #3
        	uint32_t rcpi           : 8;
        	uint32_t rssi           : 8;
        	uint32_t cfo_estimate   : 13;
        	uint32_t reserved5      : 3;

        	// RXINFO 4 word
			RXINFO	rxinfo;

        };
        uint32_t            rx_vector[9];
    };
} LMAC_RXBUF , LMAC_RXHDR;

uint16_t compute_ndp_partial_aid(uint16_t aid, uint8_t *bssid);
uint16_t compute_symbol(uint16_t psdu_bytes, uint8_t bw, uint8_t mcs);
uint16_t compute_duration(uint16_t psdu_bytes, uint8_t bw, uint8_t mcs, uint8_t n_frame, uint8_t n_sifs, uint8_t short_gi, uint32_t pre_symbol_len, uint8_t n_mpdu);
void nrc7292_rxinfo_back_war(struct _SYS_BUF *current, int8_t *rssi);
void lmac_s1g_hook_cb(int32_t res, void *param);

#endif //HAL_LMAC_11AH_H

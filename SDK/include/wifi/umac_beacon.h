#ifndef UMAC_BEACON_H
#define UMAC_BEACON_H

#pragma pack(push, 1)

typedef struct { //__attribute__((packed)) {
    // Word 0 : MAC Header Word 0
    // 16 bit Frame Control
	uint16_t    version        : 2;    // default 0
	uint16_t    type           : 2;    // 0: Management , 1: Control,  2: Data,  3: reserved
	uint16_t    subtype        : 4;
	uint16_t    to_ds          : 1;    // to AP
	uint16_t    from_ds        : 1;    // from AP
	uint16_t    more_frag      : 1;
	uint16_t    retry          : 1;
	uint16_t    pwr_mgt        : 1;
	uint16_t    more_data      : 1;
	uint16_t    protect        : 1;
	uint16_t    order          : 1;
    // 16 bit Duration
	uint16_t	duration_id;

    // Word 1 ~ Word 5: MAC Header Word 1 ~ MAC Header Word 5(2byte)
	uint8_t     address1[6];
	uint8_t     address2[6];
	uint8_t     address3[6];

	// Word 5 : MAC Header Word 5(2byte)
	uint16_t    fragment_number    : 4;
	uint16_t    sequence_number    : 12;

    // Word 6 ~ Word 7
    uint64_t    timestamp;

    // Word 8
    uint16_t    beacon_interval;
   	uint16_t    capa_info;

    // SSID IE Word 9 
    uint8_t 	element[0];

} legacy_beacon_format;

typedef struct { //__attribute__((packed)) {
	uint8_t	next_tbtt_lo;
	uint16_t next_tbtt_hi;	
	uint32_t comp_ssid;
} S1GBeaconExtendedHeader;

#pragma pack(pop)

typedef struct {
#if LMAC_CONFIG_11N == 1
    struct _SYS_BUF*    beacon;
    uint16_t            beacon_len;
#elif LMAC_CONFIG_11AH == 1
	bool				short_beacon_enable;
	uint16_t			short_beacon_ratio;
	uint32_t			beacon_count;
	uint32_t			tsf_timer;
	bool				need_short_beacon_update;
	bool				security;
	uint8_t				sa[MAC_ADDR_LEN];
	int 				vif_id;
#endif
    uint8_t             dtim_period;
    uint8_t             dtim_count;
    uint16_t            beacon_interval;
    uint16_t            dtim_interval;
	bool				start;
	uint8_t				hidden_ssid;
} beacon_info;

void umac_beacon_init(int8_t vif_id);
void umac_beacon_start(int8_t vif_id);
void umac_beacon_stop();

void umac_beacon_update(int8_t vif_id, uint8_t* b, uint16_t len);
void umac_beacon_update_tim(uint16_t aid, bool flag);
void umac_beacon_update_dtim_period(int8_t vif_id, uint8_t period );
void umac_beacon_set_beacon_interval(int8_t vif_id, uint16_t interval);
void umac_beacon_set_ssid(int8_t vif_id, uint8_t *ssid , uint8_t ssid_len);
void umac_beacon_tbtt_cb(int8_t vif_id);

#if LMAC_CONFIG_11AH == 1
void umac_beacon_set_short_beacon_interval(int8_t vif_id, uint16_t interval);
void umac_beacon_set_bss_bw(uint8_t ch_bw, uint8_t prim_ch_bw);
#endif

#endif

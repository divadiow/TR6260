#ifndef UMAC_INFO_H
#define UMAC_INFO_H

#include "system.h"
#include "umac_ieee80211_types.h"

#define MAX_VIF			2

#define APINFO				g_umac_apinfo
#define DEC_ARR_APINFO_GET_SET(t, x) \
	t get_umac_apinfo_##x(int8_t);\
	void set_umac_apinfo_##x(int8_t, t, int8_t);

#define DEF_ARR_APINFO_GET_SET(t, x) \
	t get_umac_apinfo_##x(int8_t vif_id) { \
		return APINFO[vif_id].x; \
	}; \
	void set_umac_apinfo_##x(int8_t vif_id, t value, int8_t len) { \
        memcpy( APINFO[vif_id].x, value , len );\
	}

#define DEC_VAL_APINFO_GET_SET(t, x) \
	t get_umac_apinfo_##x(int8_t);\
	void set_umac_apinfo_##x(int8_t, t);

#define DEF_VAL_APINFO_GET_SET(t, x) \
	t get_umac_apinfo_##x(int8_t vif_id) { \
		return APINFO[vif_id].x; \
	}; \
	void set_umac_apinfo_##x(int8_t vif_id, t value) { \
        APINFO[vif_id].x = value; \
	}

typedef struct {
	// General
	uint8_t						bssid[6];
	uint8_t						ssid[32];			// key
	uint8_t						ssid_len;			// key_length

	uint8_t						security;
	uint16_t					beacon_interval;

#if LMAC_CONFIG_11AH == 1
	uint16_t					short_beacon_interval;
	// S1G Info
	uint32_t					comp_ssid;
	uint32_t					change_seq_num;
	// Peer Info
   	bool						s1g_long_support; 
	bool        				pv1_frame_support;
    bool        				nontim_support;
    bool        				twtoption_activated;
    bool        				ampdu_support;
    bool        				ndp_pspoll_support;
    bool        				shortgi_1mhz_support;
    bool        				shortgi_2mhz_support;
    bool        				shortgi_4mhz_support;
    bool        				maximum_mpdu_length;
    uint8_t     				maximum_ampdu_length_exp: 3;
    uint8_t     				minimum_mpdu_start_spacing: 5;
	uint8_t             		rx_s1gmcs_map;
	uint8_t						color;
	uint8_t						traveling_pilot_support;

#endif
} __attribute__((packed)) umac_apinfo;

#define DEC_ARR_STAINFO_GET_SET(t, x) \
	t get_umac_stainfo_##x(umac_stainfo*);\
	void set_umac_stainfo_##x(umac_stainfo*, t);

#define DEF_ARR_STAINFO_GET_SET(t, x) \
	t get_umac_stainfo_##x(umac_stainfo* sta_info) { \
		if (!sta_info) \
			return (t)0; \
		return sta_info->x; \
	}; \
	void set_umac_stainfo_##x(umac_stainfo* sta_info, t value) { \
		if (sta_info) \
        	memcpy( sta_info->x , value , sizeof( sta_info->x ) );\
	}

#define DEC_VAL_STAINFO_GET_SET(t, x) \
	t get_umac_stainfo_##x(umac_stainfo*);\
	void set_umac_stainfo_##x(umac_stainfo*, t);

#define DEF_VAL_STAINFO_GET_SET(t, x) \
	t get_umac_stainfo_##x(umac_stainfo* sta_info) { \
		if (!sta_info) \
			return (t)0; \
		return sta_info->x; \
	}; \
	void set_umac_stainfo_##x(umac_stainfo* sta_info, t value) { \
		if (sta_info) \
        	sta_info->x = value; \
	}

typedef struct _umac_stainfo {
	uint8_t						maddr[6];
	uint16_t					aid;			// key
	uint32_t					listen_interval;
#if LMAC_CONFIG_11AH == 1
	///uint32_t					partial_aid; // TODO: need to be used
	bool						bss_max_idle_period_idle_option;
	uint16_t					bss_max_idle_period;
	// Peer Info
   	bool						s1g_long_support; 
	bool                		pv1_frame_support;
	bool                		nontim_support;
	bool                		twtoption_activated;
	bool                		ampdu_support;
	bool                		ndp_pspoll_support;
	bool                		shortgi_1mhz_support;
	bool                		shortgi_2mhz_support;
	bool                		shortgi_4mhz_support;
	bool                		maximum_mpdu_length;
	uint8_t             		maximum_ampdu_length_exp: 3;
	uint8_t             		minimum_mpdu_start_spacing: 5;
	uint8_t             		rx_s1gmcs_map;
	uint8_t						traveling_pilot_support;

#endif
	struct _umac_stainfo		*next_info;

} __attribute__((packed)) umac_stainfo;

//////////////////////
// Public Functions //
//////////////////////

umac_stainfo*		alloc_umac_stainfo_peer_sta(int8_t vif_id, uint8_t* addr);
bool				dealloc_umac_stainfo_peer_sta(int8_t vif_id, uint8_t* addr);
bool				add_umac_stainfo_peer_sta(int8_t vif_id, uint16_t aid, uint8_t* addr);
bool				del_umac_stainfo_peer_sta(int8_t vif_id, uint16_t aid);
umac_stainfo*		get_umac_stainfo_by_macaddr(int8_t vif_id, uint8_t *addr);
umac_stainfo*		get_umac_stainfo_by_vifid(int8_t vif_id);

void				umac_info_init(int8_t, MAC_STA_TYPE);
void				umac_info_clean(int8_t);

DEC_ARR_APINFO_GET_SET(uint8_t*,			bssid);
DEC_ARR_APINFO_GET_SET(uint8_t*,			ssid);

DEC_VAL_APINFO_GET_SET(uint8_t, 			ssid_len);
DEC_VAL_APINFO_GET_SET(uint8_t, 			security);
DEC_VAL_APINFO_GET_SET(uint16_t,			beacon_interval);

DEC_ARR_STAINFO_GET_SET(uint8_t*,			maddr);
DEC_VAL_STAINFO_GET_SET(uint16_t,			aid);
DEC_VAL_STAINFO_GET_SET(uint32_t,			listen_interval);

#if LMAC_CONFIG_11AH == 1
DEC_VAL_APINFO_GET_SET(uint16_t,			short_beacon_interval);
DEC_VAL_APINFO_GET_SET(uint32_t, 			comp_ssid);
DEC_VAL_APINFO_GET_SET(uint32_t, 			change_seq_num);
DEC_VAL_APINFO_GET_SET(bool,				s1g_long_support);
DEC_VAL_APINFO_GET_SET(bool,				pv1_frame_support);
DEC_VAL_APINFO_GET_SET(bool,        		nontim_support);
DEC_VAL_APINFO_GET_SET(bool,        		twtoption_activated);
DEC_VAL_APINFO_GET_SET(bool,        		ampdu_support);
DEC_VAL_APINFO_GET_SET(bool,        		ndp_pspoll_support);
DEC_VAL_APINFO_GET_SET(bool,        		shortgi_1mhz_support);
DEC_VAL_APINFO_GET_SET(bool,        		shortgi_2mhz_support);
DEC_VAL_APINFO_GET_SET(bool,        		shortgi_4mhz_support);
DEC_VAL_APINFO_GET_SET(bool,        		maximum_mpdu_length);
DEC_VAL_APINFO_GET_SET(uint8_t,     		maximum_ampdu_length_exp);
DEC_VAL_APINFO_GET_SET(uint8_t,     		minimum_mpdu_start_spacing);
DEC_VAL_APINFO_GET_SET(uint8_t,     		rx_s1gmcs_map);
DEC_VAL_APINFO_GET_SET(uint8_t,     		color);
DEC_VAL_APINFO_GET_SET(uint8_t,        		traveling_pilot_support);


DEC_VAL_STAINFO_GET_SET(bool,				bss_max_idle_period_idle_option);
DEC_VAL_STAINFO_GET_SET(uint16_t,			bss_max_idle_period);
DEC_VAL_STAINFO_GET_SET(bool,				s1g_long_support);
DEC_VAL_STAINFO_GET_SET(bool,				pv1_frame_support);
DEC_VAL_STAINFO_GET_SET(bool,        		nontim_support);
DEC_VAL_STAINFO_GET_SET(bool,        		twtoption_activated);
DEC_VAL_STAINFO_GET_SET(bool,        		ampdu_support);
DEC_VAL_STAINFO_GET_SET(bool,        		ndp_pspoll_support);
DEC_VAL_STAINFO_GET_SET(bool,        		shortgi_1mhz_support);
DEC_VAL_STAINFO_GET_SET(bool,        		shortgi_2mhz_support);
DEC_VAL_STAINFO_GET_SET(bool,        		shortgi_4mhz_support);
DEC_VAL_STAINFO_GET_SET(bool,        		maximum_mpdu_length);
DEC_VAL_STAINFO_GET_SET(uint8_t,     		maximum_ampdu_length_exp);
DEC_VAL_STAINFO_GET_SET(uint8_t,     		minimum_mpdu_start_spacing);
DEC_VAL_STAINFO_GET_SET(uint8_t,     		rx_s1gmcs_map);
DEC_VAL_STAINFO_GET_SET(uint8_t,       		traveling_pilot_support);
#endif




#endif

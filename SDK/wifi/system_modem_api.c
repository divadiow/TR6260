#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "system.h"
#ifndef _USER_LMAC_SDIO
#include "driver_wifi.h"
#endif
#include "util_modem.h"

const static uint16_t ch_freq_table[] =
{
	2412, 2417, 2422, 2427, 2432,
	2437, 2442, 2447, 2452, 2457,
	2462, 2467, 2472, 2484, 
};

wifi_country_t g_country_info = {"CN", 1, 13};


/**
 *	Convert ACI that sent from Host to HW Queue AC index
 *	The 11N HW Queue has 11 HW Queue AC (BK,BE,VI,VO,BCN,CONC, BK,BE,VI,VO,BCN)
 *	but, 11AH HW Queue has only 6 HW Queue. (BK, BE, VI, VO, BCN, GP)
 *	so, Should use aci_to_hwac function for converting between host and hwac
 *
 */
static int aci_to_hwac(int aci)
{
#if LMAC_CONFIG_11N == 1
	return aci;
#elif LMAC_CONFIG_11AH == 1
	//return aci > 5 ? aci - 6: aci;
	return (aci > ACI_GP) ? aci - ACI_MAX : aci;
#endif
}

uint32_t system_modem_api_get_dl_hif_length(struct _SYS_BUF *packet)
{
#if LMAC_CONFIG_11N == 1
	uint32_t len = packet->lmac_rxhdr.rxinfo.mpdu_length;
#else
	uint32_t len = packet->lmac_rxhdr.rxinfo.mpdu_length;
#endif
	len += sizeof(LMAC_RXHDR) - sizeof(struct hif_hdr);

	return len;
	//return packet->lmac_rxhdr.length + sizeof(struct frame_hdr);
}

uint32_t system_modem_api_get_tx_space()
{
	//uint32_t size = RXVECTOR_SIZE + RXINFO_SIZE;
#if LMAC_CONFIG_11N == 1
	uint32_t size = 12 + 12;
#elif LMAC_CONFIG_11AH == 1
    uint32_t size = 20 + 16;
#endif
	return size;
}

uint32_t system_modem_api_get_rx_space()
{
	//uint32_t size = TXVECTOR_SIZE;
	uint32_t size = 12;
	return size;
}

void system_modem_api_get_capabilities(struct wim_cap_param* param)
{
	param->cap = WIM_SYSTEM_CAP_USF;
#if 0
	if (COMMON_VIF()->is_sec_hw()) {
		param.cap |= WIM_SYSTEM_CAP_HWSEC;

		if (COMMON_VIF()->is_sec_offline()) {
			param.cap |= WIM_SYSTEM_CAP_HWSEC_OFFL;
		}
	}
#endif
	param->cap |= WIM_SYSTEM_CAP_HWSEC;


#if defined(NRC7291_SDK_DUAL_CM3)
	//TODO: Have to consider dual-cpu case to access the function in stainfo by mailbox?
	uint16_t lintval = 100;
#else
	int vif_id = -1;
	uint16_t lintval;
	if (system_modem_api_is_sta(0)) vif_id = 0;
	else if (system_modem_api_is_sta(1)) vif_id = 1;
	if (vif_id < 0) {
		lintval = 100;
 	    E(TT_API, "[%s] Couldn't find a vif_id of a STA lintval: %u \n",
					__func__, lintval);
	} else {
		umac_stainfo* sta_info = get_umac_stainfo_by_vifid(vif_id);
		lintval = get_umac_stainfo_listen_interval(sta_info);
	}
#endif
	param->listen_interval = lintval;

	param->bss_max_idle = 100;
	param->cap |= WIM_SYSTEM_CAP_CHANNEL_2G;
#if LMAC_CONFIG_11AH == 1
	param->cap |= WIM_SYSTEM_CAP_CHANNEL_5G;
#endif
	param->cap |= WIM_SYSTEM_CAP_MULTI_VIF;
}

void system_modem_api_set_mac_address(int vif_id, uint8_t *mac_addr)
{
	hal_lmac_set_mac_address(vif_id, mac_addr);
}

uint8_t* system_modem_api_get_mac_address(int vif_id)
{
	return hal_lmac_get_mac_address(vif_id);
}

void system_modem_api_enable_mac_address(int vif_id, bool enable)
{
	hal_lmac_set_enable_mac_addr(vif_id, enable);
}

void system_modem_api_set_bssid(int vif_id, uint8_t *bssid)
{
	hal_lmac_set_bssid(vif_id, bssid);
	set_umac_apinfo_bssid(vif_id, bssid, 6);
}

uint8_t *system_modem_api_get_bssid(int vif_id)
{
	return hal_lmac_get_bssid(vif_id);
}

void system_modem_api_set_ssid(int vif_id, uint8_t *ssid , uint8_t ssid_len)
{
#if defined(INCLUDE_UMAC_BEACON)
    umac_beacon_set_ssid(vif_id, ssid , ssid_len);
#endif
}

void system_modem_api_set_beacon_interval(int vif_id, uint16_t beacon_interval)
{
#if defined(INCLUDE_UMAC_BEACON)
    umac_beacon_set_beacon_interval(vif_id, beacon_interval);
 #endif
}
void system_modem_api_set_short_beacon_interval(int vif_id, uint16_t short_beacon_interval)
{
#if defined(INCLUDE_UMAC_BEACON)
#if LMAC_CONFIG_11AH == 1
    umac_beacon_set_short_beacon_interval(vif_id, short_beacon_interval);
#endif
#endif
}
void system_modem_api_beacon_start(int vif_id)
{
#if defined(INCLUDE_UMAC_BEACON)
    umac_beacon_start(vif_id);
#endif
}

void system_modem_api_update_beacon(int vif_id, uint8_t* beacon , uint16_t len)
{
#if defined(INCLUDE_UMAC_BEACON)
    umac_beacon_update(vif_id, beacon , len);
#endif
}

void system_modem_api_enable_bssid(int vif_id, bool enable)
{
	hal_lmac_set_enable_bssid(vif_id, enable);
}

void system_modem_api_mode_bssid(int vif_id, bool ap_mode_en)
{
	hal_lmac_mode_bssid(vif_id, ap_mode_en);
}

void system_modem_api_set_edca_param(struct EdcaParam* edca)
{
	int hwac = aci_to_hwac(edca->aci);
	set_aifsn(hwac, edca->aifsn);
	set_cw_min(hwac, edca->cw_min+1);
	set_cw_max(hwac, edca->cw_max+1);
	set_txop_max(hwac, (edca->txop_limit) << 5);
}

struct EdcaParam* system_modem_api_get_edca_param(uint8_t acid)
{
	struct EdcaParam* edca = NULL;
	edca->aci 		    = get_acid(acid);
	edca->acm			= get_acm(acid);
	edca->aifsn 		= get_aifsn(acid);
	edca->cw_min 		= get_cw_min(acid);
	edca->cw_max 		= get_cw_max(acid);
	edca->txop_limit	= get_txop_max(acid) >> 5;
	return edca;
}

bool system_modem_api_set_aid(int vif_id, uint16_t aid)
{
	return hal_lmac_set_aid(vif_id, aid);
}

int system_modem_api_get_aid(int vif_id)
{
	return hal_lmac_get_aid(vif_id);
}

bool system_modem_api_set_mode(int vif_id, uint8_t mode)
{
	return hal_lmac_set_mode(vif_id, mode);
}

bool system_modem_api_is_ap(int vif_id)
{
	return hal_lmac_is_ap(vif_id);
}

bool system_modem_api_is_sta(int vif_id)
{
	return hal_lmac_is_sta(vif_id);
}

void system_modem_api_set_cca_ignore(bool ignore)
{
	hal_lmac_set_cca_ignore(ignore);
}

void system_modem_api_set_promiscuous_filter(wifi_promiscuous_filter_t *filter)
{
    hal_lmac_set_promiscuous_filter(filter);
}

void system_modem_api_set_promiscuous_enable(int vif_id, bool enable)
{
    hal_lmac_set_promiscuous_enable(vif_id, enable);
}

bool system_modem_api_get_promiscuous_mode(int vif_id)
{
	return get_promiscuous_mode(vif_id);
}

/// ERP configuration
bool system_modem_api_set_erp_param(int vif_id, struct wim_erp_param* p)
{
	bool ret = false;
#if LMAC_CONFIG_11N == 1
	ret = hal_lmac_set_erp_param(vif_id,p->use_11b_protection, p->use_short_slot, p->use_short_preamble);
#endif
    return ret;
}

int system_modem_api_get_cipher_icv_length(int type)
{
	return hal_lmac_get_cipher_icv_length(type);
}

bool system_modem_api_sec_set_enable_key(int vif_id, bool enable)
{
	return hal_lmac_sec_set_enable_key(vif_id, enable);
}

bool system_modem_api_sec_get_enable_key   (int vif_id)
{
	return hal_lmac_sec_get_enable_key(vif_id);
}

bool system_modem_api_add_key(int vif_id, struct LMacCipher *lmc, bool dummy)
{
	return hal_lmac_sec_add_key(vif_id, lmc, dummy);
}

bool system_modem_api_del_key(int vif_id, struct LMacCipher *lmc)
{
	return hal_lmac_sec_del_key(vif_id, lmc);
}

void system_modem_api_del_key_all(int vif_id)
{
	hal_lmac_sec_del_key_all(vif_id);
}

bool system_modem_api_set_basic_rate(int vif_id, uint32_t basic_rate_set)
{
	return hal_lmac_set_basic_rate(vif_id, basic_rate_set);
}

uint32_t system_modem_api_get_tsf_timer_high(int vif_id)
{
	return drv_lmac_get_tsf_hi(vif_id);
}

uint32_t system_modem_api_get_tsf_timer_low(int vif_id)
{
	return drv_lmac_get_tsf_lo(vif_id);
}

void system_modem_api_set_tsf_timer(int vif_id, uint32_t ts_hi, uint32_t ts_lo)
{
	hal_lmac_set_tsf_timer(vif_id, ts_hi, ts_lo);
}
void system_modem_api_set_response_ind(int ac, uint8_t response_ind)
{
	hal_lmac_set_response_ind(ac, response_ind);
}

uint8_t system_modem_api_get_response_ind(int ac)
{
	return hal_lmac_get_response_ind(ac);
}

void system_modem_api_set_aggregation(int ac, bool aggregation)
{
	hal_lmac_set_aggregation(ac, aggregation);
}

bool system_modem_api_get_aggregation(int ac)
{
	return hal_lmac_get_aggregation(ac);
}

bool system_modem_api_set_ht_operation_info(int vif_id, uint16_t ht_info)
{
	return hal_lmac_set_ht_operation_info(vif_id, ht_info);
}

void system_modem_api_set_short_gi(int vif_id, uint8_t short_gi)
{
	hal_lmac_set_short_gi(vif_id, short_gi);
}

bool system_modem_api_set_ht_capability(int vif_id, uint16_t ht_cap)
{
	return hal_lmac_set_ht_capability(vif_id, ht_cap);
}

void system_modem_api_set_mcs(int vif_id, uint8_t mcs)
{
	set_mcs(vif_id,mcs);
}

#if LMAC_CONFIG_11AH == 1
void system_modem_api_set_channel_width_s1goper(int vif_id, uint8_t prim_ch_width, uint8_t prim_loc, uint8_t prim_ch_number)
{
	if (SUBSYSTEM_DELIMITER) {
		// FPGA setting
		return;
	}

	uint8_t chan_width  = umac_s1g_config_get_operchwidth();
	uint8_t chan_number = umac_s1g_config_get_centerfreq();

	// s1g
	umac_s1g_config_set_primchwidth(prim_ch_width);
	umac_s1g_config_set_primchlocation(prim_loc);
	umac_s1g_config_set_primchnum(prim_ch_number);

	// lmac setting
	if (prim_ch_width == PR_CH_WIDTH_2M)
		set_prim_ch_bw(vif_id, 1);
	else
		set_prim_ch_bw(vif_id, 0);

	set_prim_ch_loc(vif_id, prim_loc);

	//
	switch (chan_width) {
		case OP_CH_WIDTH_1M:
			phy_set_primary_1m_loc(0);
			break;

		case OP_CH_WIDTH_2M:
			if (prim_ch_width == PR_CH_WIDTH_1M) {
				phy_set_primary_1m_loc(prim_loc);
			} else {
				phy_set_primary_1m_loc(0);
			}
			break;

		case OP_CH_WIDTH_4M:
			if (prim_ch_width == PR_CH_WIDTH_1M) {
				if (prim_ch_number < chan_number) {
					if (chan_number - prim_ch_number == 3)
						phy_set_primary_1m_loc(0);
					else
						phy_set_primary_1m_loc(1);
				} else {
					if (prim_ch_number - chan_number == 1)
						phy_set_primary_1m_loc(2);
					else
						phy_set_primary_1m_loc(3);
				}
			} else {
				uint8_t offset = 0;
				if (prim_ch_number > chan_number) {
					offset = 2;
				}
				phy_set_primary_1m_loc(offset + prim_loc);
			}
			break;
	}

#if defined(INCLUDE_UMAC_BEACON)
	umac_beacon_set_bss_bw(chan_width, prim_ch_width);
#endif

}
#endif

bool system_modem_api_set_channel_width(int vif_id, uint8_t chan_width, uint8_t prim_loc)
{
	// how to separate 11n/11ah? need external delimiter?
	bool res = true;

    if (chan_width >= BW_MAX) chan_width = BW_MAX - 1;
#if LMAC_CONFIG_11N == 1

#if defined(NRF40)
	nrf_op_bw_control(chan_width);
	phy_op_primary(vif_id, prim_loc);
#endif
    phy_op_primary(vif_id, prim_loc);
	set_ch_bw(vif_id, chan_width);
	set_prim_ch_loc(vif_id, prim_loc);

#elif LMAC_CONFIG_11AH == 1

	if (SUBSYSTEM_DELIMITER) {
		// FPGA setting
		return true;
	}

	// Reconfigure rc-map
	if (get_rate_ctrl_en(vif_id))
		hal_lmac_reconfig_rc_entry(get_ch_bw(vif_id), chan_width, vif_id);

	umac_s1g_config_set_primchlocation(prim_loc);
	set_prim_ch_loc(vif_id, prim_loc);

	switch (chan_width) {
		case BW_1M:
			set_ch_bw(vif_id, BW_1M);
			umac_s1g_config_set_operchwidth(OP_CH_WIDTH_1M);
			phy_op_bw(2);
#ifdef NRC7292_LMACTEST_FPGA_AVIA
            arf_filter_bw(2);
#else
			nrf_filter_bw(2);
#endif
			phy_set_primary_1m_loc(prim_loc);
			res = true;
			break;

		case BW_2M:
			set_ch_bw(vif_id, BW_2M);
			umac_s1g_config_set_operchwidth(OP_CH_WIDTH_2M);
			phy_op_bw(2);
#ifdef NRC7292_LMACTEST_FPGA_AVIA
            arf_filter_bw(2);
#else
			nrf_filter_bw(2);
#endif

			res = true;
			break;

		case BW_4M:
			set_ch_bw(vif_id, BW_4M);
			umac_s1g_config_set_operchwidth(OP_CH_WIDTH_4M);
			phy_op_bw(4);
#ifdef NRC7292_LMACTEST_FPGA_AVIA
            arf_filter_bw(4);
#else
			nrf_filter_bw(4);
#endif
			res = true;
			break;
	}

#endif

	return res;
}

bool system_modem_api_scan_channel(int vif_id, uint32_t frequency)
{
	bool res = false;
	uint32_t ch_freq = frequency;

    g_sys_info.current_channel = ch_freq;
    g_sys_info.current_channel_number = system_modem_api_mac80211_frequency_to_channel(ch_freq);
    E(TT_API, "[%s] %s: %d\n",
			__func__, (ch_freq > 165) ? "freq":"ch# (%d)", ch_freq, g_sys_info.current_channel_number);
#if LMAC_CONFIG_11N == 1
	set_frequency(vif_id, ch_freq);
	res = nrf_test_channel(ch_freq);
#elif LMAC_CONFIG_11AH == 1
	if (SUBSYSTEM_DELIMITER){
		// FPGA mode
        return true;
    }

	int8_t offset = 0;
	int8_t primary_loc = 0;
	uint32_t converted_ch = 0;

	if (ch_freq > 5000) {
		converted_ch = (uint32_t) GetS1GFreq(ch_freq, &offset ,&primary_loc); // 5ghz -> s1g band
	} else if (ch_freq < 166){
		// ch_freq is channel number
		ch_freq = (uint32_t) GetNonS1GFreqFromNonS1GChNum(ch_freq);
		converted_ch = (uint32_t) GetS1GFreq(ch_freq, &offset ,&primary_loc);
	}

	// save frequency in mac80211 block
	set_mac80211_frequency(vif_id, ch_freq);
	res = nrf_channel(converted_ch * 100000);
	if (res) {
		uint32_t bw = ((get_ch_bw(vif_id) == BW_4M) ? 4 : 2);
		phy_set_cfo2sfo_factor((converted_ch / 10), bw);
	}
#endif
	return res;
}

uint16_t system_modem_api_get_frequency(SYS_BUF *packet)
{
	uint32_t current_channel = packet->lmac_rxhdr.rxinfo.center_freq;
	uint16_t frequency = 0;

	frequency = system_modem_api_channel_to_mac80211_frequency(current_channel);

//	ASSERT(frequency);
	return frequency;
}

uint16_t system_modem_api_get_current_channel_number()
{
	return g_sys_info.current_channel_number;
}

uint32_t system_modem_api_get_snr(SYS_BUF *packet)
{
	return packet->lmac_rxhdr.snr;
}

uint32_t system_modem_api_get_current_snr(int loc)
{
	uint32_t snr, signal, noise;

	system_modem_api_read_signal_noise(loc, &signal, &noise);
	snr = util_modem_compute_snr(signal, noise);

	return snr;
}
extern uint32_t util_modem_compute_snr_i(uint32_t signal, uint32_t noise);
uint32_t system_modem_api_get_current_snr_i(int loc)
{
	uint32_t snr, signal, noise;

	system_modem_api_read_signal_noise(loc, &signal, &noise);
	snr = util_modem_compute_snr_i(signal, noise);

	return snr;
}

//return 1 -- param error
uint32_t system_modem_api_set_country_info(wifi_country_t *info)
{
    if (!info || info->nchan <= 0 || info->schan <= 0)
        return 1;
    if ((info->schan + info->nchan - 1) > 14)
        return 1;

    g_country_info = *info;
    g_country_info.cc[sizeof(g_country_info.cc) - 1] = '\0';
    
    return 0;
}

void system_modem_api_get_country_info(wifi_country_t *info)
{
    if (!info)
        return;
    *info = g_country_info;

    return;
}

void system_api_get_supported_channels(const uint16_t **chs, int *n_ch)
{
	if (!n_ch || !chs)
		return;

	*chs = ch_freq_table + g_country_info.schan - 1;
	*n_ch = g_country_info.nchan;
}

/* convert mac80211 frequency to it's actual channel number */
uint8_t system_modem_api_mac80211_frequency_to_channel(uint32_t frequency)
{
	int i;
	uint8_t channel = 0;
    
	for (i=0; i<ARRAY_SIZE(ch_freq_table); i++) {
		if (ch_freq_table[i] == frequency) {
			channel = i + 1;
			break;
		}
	}

    //check channel valid by country code.
    if (channel < g_country_info.schan || channel > (g_country_info.schan + g_country_info.nchan - 1))
        channel = 0;

    if (channel <= 0) {
        system_printf("error: freq:%d, channel:%d, cn:%2s, %d|%d\n", frequency, channel, g_country_info.cc, g_country_info.schan,
        g_country_info.nchan);
    }

	return channel;
}

/* convert actual channel number to mac80211 frequency */
uint32_t system_modem_api_channel_to_mac80211_frequency(uint8_t channel)
{
	uint32_t frequency = 0;

	ASSERT((channel-1) < ARRAY_SIZE(ch_freq_table));
    //check channel valid by country code.
    if (channel < g_country_info.schan || channel > (g_country_info.schan + g_country_info.nchan - 1)) {
        frequency = 0;
        E(TT_API, "freq:%d, channel:%d, cn:%2s, %d|%d\n", frequency, channel, g_country_info.cc, g_country_info.schan,
            g_country_info.nchan);
    } else {
    	frequency = ch_freq_table[channel-1];
    }

	return frequency;
}

int system_modem_api_get_rssi(SYS_BUF *packet)
{
    return packet->lmac_rxhdr.rssi;
}

bool system_modem_api_set_channel(int vif_id, uint32_t frequency)
{
	bool res = false;
	uint32_t ch_freq = frequency;
	//system_printf("[%s] %s: %d\n" , __func__, (ch_freq > 165) ? "freq":"ch#", ch_freq);
	if (g_sys_info.current_channel == ch_freq)
		return true;

    g_sys_info.current_channel = ch_freq;
#if LMAC_CONFIG_11N == 1
	set_frequency(vif_id, ch_freq);
	res = nrf_test_channel(ch_freq);
    g_sys_info.current_channel_number = system_modem_api_mac80211_frequency_to_channel(ch_freq);
	set_mac80211_frequency(vif_id, ch_freq);
#elif LMAC_CONFIG_11AH == 1

	int8_t offset = 0;
	int8_t primary_loc = 0;
	uint32_t converted_ch = 0;
	uint16_t confirm_channel = 0;
	if (ch_freq > 5000) {
		converted_ch = (uint32_t) GetS1GFreq(ch_freq, &offset ,&primary_loc); // 5ghz -> s1g band
		confirm_channel = ch_freq;
	} else if (ch_freq < 166){
		// ch_freq is channel number
		ch_freq = (uint32_t) GetNonS1GFreqFromNonS1GChNum(ch_freq);
		converted_ch = (uint32_t) GetS1GFreq(ch_freq, &offset ,&primary_loc);
		confirm_channel = ch_freq;
	}

	// save frequency in mac80211 block
	set_mac80211_frequency(vif_id, ch_freq);

	if (!converted_ch) {
		uint16_t default_channel = GetNonS1GDefaultFreq();
		converted_ch = (uint32_t) GetS1GFreq(default_channel, &offset, &primary_loc);
		confirm_channel = default_channel;
		g_sys_info.current_channel = default_channel;
		g_sys_info.current_channel_number = system_modem_api_mac80211_frequency_to_channel(default_channel);

		E(TT_MSG, "Unsupported channel. Change default channel (%d->%d), converted:%d\n",
						ch_freq, default_channel, converted_ch);
	} else {
		/*
		 * README swki - 2018-0730
		 * To avoid the ASSERT in system_modem_api_mac80211_frequency_to_channel()
		 */
		g_sys_info.current_channel_number = system_modem_api_mac80211_frequency_to_channel(ch_freq);
	}

	uint8_t ch_id = APGetS1GChID(confirm_channel);
	umac_s1g_config_set_primchnum(ch_id);
	umac_s1g_config_set_centerfreq(ch_id);
	uint8_t oper_class = APGetOperClass(ch_id);
	umac_s1g_config_set_operclass(oper_class);

	uint8_t cca_type = GetS1GCCAType((const uint16_t)converted_ch);

	// According to channel info from each of country code, cca_type is configured
	if ((!cca_type) || (cca_type > 2)) {
		E(TT_MSG, "[%s] CCA fail : discard CMD\n", __func__);
	} else {
		// FIXME: wrap with hal_phy function
		WRITE_REG(PHY_REG_RXDFE_CTRL_CCA_LVL_TYPE, cca_type - 1);
	}

	//if no matching or same as current, just discard it
	if (!converted_ch ||
	    (converted_ch == hal_lmac_get_freq(vif_id)
	     && hal_lmac_is_ap(vif_id))) {

		V(TT_MSG, "[%s] %s Channel : discard CMD\n",
				__func__, (!converted_ch)?"Fail to set":"The same");
		return true;
	} else {
		const CHANNEL_MAPPING_TABLE *item = get_s1g_channel_item_by_nons1g_freq(ch_freq);
		if (item == NULL) {
			item = GetS1GDefaulTable();
			E(TT_MSG, "Load Default table s1g:%d, nons1g:%d, spacing:%d\n", item->s1g_freq, item->nons1g_freq, item->chan_spacing);
		}

		system_modem_api_set_channel_width(vif_id, item->chan_spacing, primary_loc);

		// offset for 1MHz align
		uint32_t freq = converted_ch + offset;
    	if (SUBSYSTEM_DELIMITER) {
            res = true;
        }
        else {
    		res = nrf_channel(freq * 100000);
        }

		// phy setting
		if (res) {
			uint32_t bw = ((get_ch_bw(vif_id) == BW_4M) ? 4 : 2);
			phy_set_cfo2sfo_factor((freq / 10), bw); // phy setting
		}

		set_frequency(vif_id, converted_ch);	// mac setting
		phy_set_primary_1m_loc(primary_loc);	//set 1M prim location here for mgmt frame

		if (hal_lmac_is_ap(vif_id)) {
			bool enable = umac_s1g_config_get_dup1mhz_support();
			bool enable_dup = umac_s1g_config_get_1mctrlresppreamble_support();
			hal_lmac_enable_1m(enable, enable_dup); // configures MAC function of 1MHz duplication
		}
		E(TT_MSG, "[%s (5g_freq:%d s1g_freq:%d , cca_type:%d, offset:%2d, loc:%d, spacing:%d) \n",
				__func__, frequency, converted_ch, cca_type, offset, primary_loc,item->chan_spacing);

	}

#endif

	return res;
}

void system_modem_api_set_rate_control(int vif_id, bool enable)
{
	// enable rate control
	hal_lmac_set_rate_control(vif_id, enable);

#if LMAC_CONFIG_11AH == 1
	// update rate entry
	uint8_t ch_bw = get_ch_bw(vif_id);
	if (ch_bw == BW_4M) {
		lmac_rc_configure(S1G, RC_ATTR_4M, 0xff, vif_id);
		lmac_rc_configure(S1G, RC_ATTR_2M, 0xff, vif_id);
	} else if (ch_bw == BW_2M) {
		lmac_rc_configure(S1G, RC_ATTR_2M, 0xff, vif_id);
	}
#endif
}

void system_modem_api_set_txgain(uint32_t txgain)
{
#if LMAC_CONFIG_11AH == 1
	int vif_id = 0;
	if (txgain > 30 || txgain == 0) {
		system_printf("invalid tx gain index\n");
		system_printf(" -- tx gain index has a value 1~30 (1dB step)\n");
	}
	set_phy_txgain(vif_id, txgain);
	phy_nrf_txgain_control(txgain);
#endif
}

#if LMAC_CONFIG_11AH == 1
#if defined (NRC7291)
#define SYS_MAX_RX_GAIN_VALUE	103
#else
#define SYS_MAX_RX_GAIN_VALUE	120
#endif
#endif

void system_modem_api_set_rxgain(uint32_t rxgain)
{
#if LMAC_CONFIG_11AH == 1
	int vif_id = 0;
	if (rxgain > SYS_MAX_RX_GAIN_VALUE) {
		system_printf("invalid rx gain\n");
		system_printf(" -- rx gain has value 0~%d (1 dB step)\n",SYS_MAX_RX_GAIN_VALUE);
	}
	set_phy_rxgain(vif_id, rxgain);
	phy_nrf_rxgain_control(rxgain);
#endif
}

void system_modem_api_set_tx_suppress_dur(uint32_t value)
{
#if LMAC_CONFIG_11AH == 1
	WRITE_REG(MAC_REG_TXSUPPRESS_COMMAND, 7);
	WRITE_REG(MAC_REG_TXSUPPRESS_DURATION, value);
#endif
}

void system_modem_api_set_tx_suppress_cmd(uint32_t value)
{
#if LMAC_CONFIG_11AH == 1
	WRITE_REG(MAC_REG_TXSUPPRESS_COMMAND, value)
#endif
}

//TODO swki - move to system_api.c
uint32_t system_api_get_version(void)
{
//	return  SI_11N 		= 1,
#if LMAC_CONFIG_11N == 1
	return WIM_SYSTEM_VER_11N;
#elif LMAC_CONFIG_11AH == 1
	return WIM_SYSTEM_VER_11AH_1ST;
#endif
	return 0;
}

uint32_t system_api_get_align(void)
{
	return 0;
}

uint32_t system_api_get_buffer_length(void)
{
	//return LMAC_CONFIG_BUFFER_SIZE - sizeof(SYS_BUF) - sizeof(LMAC_TXHDR);
	return 456;
}

void system_api_set_promiscuous_mode(bool enable)
{
	hal_lmac_set_promiscuous_mode(0, enable);
}



#ifndef NON_SFLASH
#if LMAC_CONFIG_11AH == 1
void system_api_get_rf_cal(sf_info_t *m_sfi, uint32_t address, uint8_t *buffer, size_t size)
{
#if defined(CACHE_XIP)
	memcpy(buffer, (uint8_t*)(SFC_MEM_BASE_ADDR + address), size);
#else
	nrc_sf_read(m_sfi, address, buffer, size);
#endif
}

void system_api_set_rf_cal(sf_info_t *m_sfi, uint32_t address, uint8_t *buffer, size_t size)
{
#if defined(CACHE_XIP)
	system_printf("can't store cal info during XIP mode yet.\n");
#else
	nrc_sf_erase(m_sfi, address, size);
	nrc_sf_write(m_sfi, address, buffer, size);
#endif
}

void system_api_clear_rf_cal(sf_info_t *m_sfi, uint32_t address, size_t size)
{
#if defined(CACHE_XIP)
	system_printf("can't clear cal info during XIP mode yet.\n");
#else
	nrc_sf_erase(m_sfi, address, size);
#endif
}
#endif
#endif

void system_modem_api_update_probe_resp(uint8_t* probe, uint16_t len)
{
	umac_presp_offl_update(probe, len);
}

void system_default_setting(int vif_id)
{
    system_modem_api_set_channel(vif_id, 2412);
}

/**
 * system_modem_api_read_signal_noise
 * desc: read signal and noise register for snr calculation
 * arg: int loc:  0:primary, 1:secondary 
 * 		uint32_t *signal: signal level
 * 		uint32_t *noise: noise level
 */
void system_modem_api_read_signal_noise(int loc, uint32_t *signal, uint32_t *noise)
{
	uint32_t reg = 0;
#define READ_REG_FIELD(n, r, f)\
    reg = READ_REG(r);\
    n   = GET_FIELD(r, f, reg);\

#if LMAC_CONFIG_11AH == 1
	switch(loc) {
		case 0:
			READ_REG_FIELD(*signal, PHY_REG_RXDFE_CTRL_RDFE_MON_LOG_22, CHNL_MON_02);
			READ_REG_FIELD(*noise, PHY_REG_RXDFE_CTRL_RDFE_MON_LOG_26, CHNL_MON_06);
			break;
		case 1:
			READ_REG_FIELD(*signal, PHY_REG_RXDFE_CTRL_RDFE_MON_LOG_23, CHNL_MON_03);
			READ_REG_FIELD(*noise, PHY_REG_RXDFE_CTRL_RDFE_MON_LOG_27, CHNL_MON_07);
			break;
		default:
			*signal = 0;
			*noise = 0;
			E(TT_DL, "%s Invalid:%d\n", __func__, loc);
			break;
	}
#elif LMAC_CONFIG_11N == 1
	switch(loc) {
		case 0:
			READ_REG_FIELD(*signal, PHY_REG_RXDFE_CTRL_MON_LOG_28, CHNL_MON);
			READ_REG_FIELD(*noise, PHY_REG_RXDFE_CTRL_MON_LOG_29, CHNL_MON);
			break;
		case 1:
			READ_REG_FIELD(*signal, PHY_REG_RXDFE_CTRL_MON_LOG_30, CHNL_MON);
			READ_REG_FIELD(*noise, PHY_REG_RXDFE_CTRL_MON_LOG_31, CHNL_MON);
			break;
		default:
			*signal = 0;
			*noise = 0;
			E(TT_DL, "%s Invalid:%d\n", __func__, loc);
			break;
	}
#endif
#undef READ_REG_FIELD	
}

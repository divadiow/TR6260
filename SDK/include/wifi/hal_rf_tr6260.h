#ifndef __NRC_RF_H__
#define __NRC_RF_H__

#define PMU_BASE	0x00601000
#define PCU_BASE	0x00601040
#define RF_BASE		0x00601C00

#define RF_REG_SIZE 			(0x4)
#define RF_LDO_ADDA 				(RF_BASE + 0x0)//RF_REG_SIZE*0)
#define RF_LDO_RESERVE				(RF_BASE + 0x4)//RF_REG_SIZE*1)
#define RF_LDO_DYNAMIC				(RF_BASE + 0x8)//RF_REG_SIZE*2)
#define RF_LDO_PLL					(RF_BASE + 0xc)//RF_REG_SIZE*3)
#define RF_CTRL_RX_REG4				(RF_BASE + 0x10)//RF_REG_SIZE*4)
#define RF_CTRL_RX_REG5				(RF_BASE + 0x14)//RF_REG_SIZE*5)
#define RF_CTRL_RX_REG6				(RF_BASE + 0x18)//RF_REG_SIZE*6)
#define RF_RX_DYNAMIC				(RF_BASE + 0x1c)//RF_REG_SIZE*7)
#define RF_RX_SPARTBITS				(RF_BASE + 0x20)//RF_REG_SIZE*8)
#define RF_AGC_CONTROL				(RF_BASE + 0x24)//RF_REG_SIZE*9)	
#define RF_IQ_ADC_DYNAMIC			(RF_BASE + 0x28)//RF_REG_SIZE*10)
#define RF_IQ_ADC_DCOC_CTRL			(RF_BASE + 0x2c)//RF_REG_SIZE*11)
#define RF_IQ_ADC_CTRL_REG12		(RF_BASE + 0x30)//RF_REG_SIZE*12)
#define RF_IQ_ADC_CTRL_REG13		(RF_BASE + 0x34)//RF_REG_SIZE*13)
#define RF_IQ_ADC_OTA_CTRL			(RF_BASE + 0x38)//RF_REG_SIZE*14)
#define RF_IQ_ADC_QUANTIZER			(RF_BASE + 0x3c)//RF_REG_SIZE*15)
#define RF_IQ_ADC_SPARTBITS			(RF_BASE + 0x40)//RF_REG_SIZE*16)
#define RF_TX_SPARTBITS				(RF_BASE + 0x44)//RF_REG_SIZE*17)
#define RF_TX_PA_DA_CTRL			(RF_BASE + 0x48)//RF_REG_SIZE*18)
#define RF_TX_STATIC_CTRL			(RF_BASE + 0x4c)//RF_REG_SIZE*19)
#define RF_TX_DYNAMIC0_TX_CTRL		(RF_BASE + 0x50)//RF_REG_SIZE*20)
#define RF_TX_DYNAMIC1_TX_CTRL		(RF_BASE + 0x54)//RF_REG_SIZE*21)
#define RF_TX_DYNAMIC2_TX_CTRL		(RF_BASE + 0x58)//RF_REG_SIZE*22)
#define RF_TX_DYNAMIC3_TX_CTRL		(RF_BASE + 0x5c)//RF_REG_SIZE*23)
#define RF_TX_DYNAMIC0_RX_CTRL		(RF_BASE + 0x60)//RF_REG_SIZE*24)
#define RF_TX_DYNAMIC1_RX_CTRL		(RF_BASE + 0x64)//RF_REG_SIZE*25)
#define RF_TX_DYNAMIC2_RX_CTRL		(RF_BASE + 0x68)//RF_REG_SIZE*26)
#define RF_TX_DYNAMIC3_RX_CTRL		(RF_BASE + 0x6c)//RF_REG_SIZE*27)
#define RF_APC_CTRL_REG28			(RF_BASE + 0x70)//RF_REG_SIZE*28)
#define RF_APC_CTRL_REG29			(RF_BASE + 0x74)//RF_REG_SIZE*29)
#define RF_APC_CTRL_REG30			(RF_BASE + 0x78)//RF_REG_SIZE*30)
#define RF_PGA_ADC0					(RF_BASE + 0x7c)//RF_REG_SIZE*31)
#define RF_PGA_ADC1					(RF_BASE + 0x80)//RF_REG_SIZE*32)
#define RF_PGA_ADC2					(RF_BASE + 0x84)//RF_REG_SIZE*33)
#define RF_PGA_ADC3					(RF_BASE + 0x88)//RF_REG_SIZE*34)
#define RF_PLL_CTRL0				(RF_BASE + 0x8c)//RF_REG_SIZE*35)
#define RF_PLL_CTRL1				(RF_BASE + 0x90)//RF_REG_SIZE*36)
#define RF_PLL_CTRL2				(RF_BASE + 0x94)//RF_REG_SIZE*37)
#define RF_PLL_CTRL3				(RF_BASE + 0x98)//RF_REG_SIZE*38)
#define RF_PLL_CTRL4				(RF_BASE + 0x9c)//RF_REG_SIZE*39)
#define RF_PLL_SPARTBIT12			(RF_BASE + 0xA0)//RF_REG_SIZE*40)
#define RF_PLL_SPARTBIT34			(RF_BASE + 0xa4)//RF_REG_SIZE*41)
#define RF_PLL_CTRL5				(RF_BASE + 0xa8)//RF_REG_SIZE*42)
#define RF_BB_CLK_GEN				(RF_BASE + 0xac)//RF_REG_SIZE*43)
#define RF_BB_CLK_GEN_DYNAMIC		(RF_BASE + 0xb0)//RF_REG_SIZE*44)
#define RF_TEST_MUX					(RF_BASE + 0xb4)//RF_REG_SIZE*45)
#define RF_ATST_ALL_TEST			(RF_BASE + 0xb8)//RF_REG_SIZE*46)
#define RF_ALL_TEST					(RF_BASE + 0xbc)//RF_REG_SIZE*47)
#define RF_CHANNEL_SWITCH			(RF_BASE + 0xc0)//RF_REG_SIZE*48)
#define RF_DFE_RX_CTRL				(RF_BASE + 0xc4)//RF_REG_SIZE*49)
#define RF_DFE_TX_CTRL				(RF_BASE + 0xc8)//RF_REG_SIZE*50)
#define RF_DFE_TX_REG51				(RF_BASE + 0xcc)//RF_REG_SIZE*51)
#define RF_DFE_TX_REG52				(RF_BASE + 0xd0)//RF_REG_SIZE*52)
#define RF_APC_RAM_RD_SWITCH		(RF_BASE + 0xd4)//RF_REG_SIZE*53)
#define RF_SDM_CW1					(RF_BASE + 0xd8)//RF_REG_SIZE*54)
#define RF_SDM_CW2					(RF_BASE + 0xdc)//RF_REG_SIZE*55)
#define RF_SDM_CW3					(RF_BASE + 0xe0)//RF_REG_SIZE*56)
#define RF_SDM_CW4					(RF_BASE + 0xe4)//RF_REG_SIZE*57)
#define RF_SDM_CW5					(RF_BASE + 0xe8)//RF_REG_SIZE*58)
#define RF_SDM_CW6					(RF_BASE + 0xec)//RF_REG_SIZE*59)
#define RF_FCAL_CW1					(RF_BASE + 0xf0)//RF_REG_SIZE*60)
#define RF_FCAL_CW2					(RF_BASE + 0xf4)//RF_REG_SIZE*61)
#define RF_FCAL_CW3					(RF_BASE + 0xf8)//RF_REG_SIZE*62)
#define RF_FCAL_CW4					(RF_BASE + 0xfc)//RF_REG_SIZE*63)
#define RF_FCAL_CW56				(RF_BASE + 0x100)//RF_REG_SIZE*64)
#define RF_FCAL_CW7					(RF_BASE + 0x104)//RF_REG_SIZE*65)
#define RF_HPF_PAR					(RF_BASE + 0x108)//RF_REG_SIZE*66)
#define RF_RF_TO_DIG				(RF_BASE + 0x10c)//RF_REG_SIZE*67)
#define RF_CTRL_REG68				(RF_BASE + 0x110)//RF_REG_SIZE*68)
#define RF_CTRL_REG69				(RF_BASE + 0x114)//RF_REG_SIZE*69)
#define RF_CTRL_REG70				(RF_BASE + 0x118)//RF_REG_SIZE*70)
#define RF_CTRL_REG71				(RF_BASE + 0x11c)//RF_REG_SIZE*71)
#define RF_CTRL_REG72				(RF_BASE + 0x120)//RF_REG_SIZE*72)
#define RF_CTRL_REG73				(RF_BASE + 0x124)//RF_REG_SIZE*73)
#define RF_CTRL_REG74				(RF_BASE + 0x128)//RF_REG_SIZE*74)
#define RF_CTRL_REG75				(RF_BASE + 0x12c)//RF_REG_SIZE*75)
#define RF_CTRL_REG76				(RF_BASE + 0x130)//RF_REG_SIZE*76)
#define RF_PLL_BIAS_CTRL			(RF_BASE + 0x134)//RF_REG_SIZE*77)
#define RF_ADC_DC_INDEX0_3			(RF_BASE + 0x138)//RF_REG_SIZE*78)
#define RF_ADC_DC_INDEX4_7			(RF_BASE + 0x13c)//RF_REG_SIZE*79)
#define RF_ADC_DC_INDEX8_10			(RF_BASE + 0x140)//RF_REG_SIZE*80)
#define RF_ADC_DC_COMP_VAL			(RF_BASE + 0x144)//RF_REG_SIZE*81)
#define RF_ADC_DC_COMP_TAB01		(RF_BASE + 0x148)//RF_REG_SIZE*82)
#define RF_ADC_DC_COMP_TAB23		(RF_BASE + 0x14c)//RF_REG_SIZE*83)
#define RF_ADC_DC_COMP_TAB45		(RF_BASE + 0x150)//RF_REG_SIZE*84)
#define RF_ADC_DC_COMP_TAB67		(RF_BASE + 0x154)//RF_REG_SIZE*85)
#define RF_ADC_DC_COMP_TAB89		(RF_BASE + 0x158)//RF_REG_SIZE*86)
#define RF_DIG_RX_LNA_FAST			(RF_BASE + 0x15c)//RF_REG_SIZE*87)
#define RF_TX_CTRL_REG88			(RF_BASE + 0x160)//RF_REG_SIZE*88)

#define DRIVER_VERSION	0x15
#define UPDATE_DATE		0x20180627
	
#define DUMP_BASE       0x00A06500
#define TRIGGER_SEL      (DUMP_BASE + 0x08)
#define DUMP_CTRL        (DUMP_BASE + 0x0C)
#define TRIGGER_VALID    (DUMP_BASE + 0x10)
#define DUMP_CLK_SEL     (DUMP_BASE + 0x14)
	
#define Debug_Mode_en   	0x0060B100
#define Debug_Mode_sel  	0x0060B104
#define Iram_Maxmium_addr 	0x0060B108
#define PHY_Dump_Count 		0x0060B10C
#define PHY_Stop_addr 		0x0060B110

struct nrf_test_channel_reg {
	uint8_t channel;
	uint16_t frequency;
	uint32_t first[6];
	uint32_t tx_resmp_ratio1;
	uint32_t tx_resmp_ratio0;
	uint32_t tx_resmp_rstnumm1;
	uint32_t rx_resmp_ratiom1;
	uint32_t rx_resmp_ratiom0; // Edit By DK.Lee
	uint32_t rx_resmp_rstnumm1;
};
enum NV_txpower_ch
{
	TXPOWER_HT_CH1 = 0,
	TXPOWER_HT_CH6,
	TXPOWER_HT_CH11,
	TXPOWER_HT_40M_CH3,
	TXPOWER_HT_40M_CH8,
	TXPOWER_NONHT_CH1,
	TXPOWER_NONHT_CH6,
	TXPOWER_NONHT_CH11,
	TXPOWER_11B_CH1,
	TXPOWER_11B_CH6,
	TXPOWER_11B_CH11,
	TXPOWER_CH_END,
};

void hal_rf_init();
void initialize_nrf();
void nrf_reg_write(uint32_t addr,uint32_t value);
uint32_t nrf_reg_read(uint32_t addr);
bool nrf_test_channel(uint32_t ch_freq);
void nrf_reg_init();
void nrf_radio_mode_spi(uint32_t);
void nrf_radio_mode_pin(uint32_t);
void nrf_test_pll_cal(uint32_t);
void nrf_power_on();
void nrf_power_off();
void nrf_show_channel();
void nrf_spi_test();
void nrf_test_rccal_reg(uint32_t *);
void nrf_loiq_cal(uint32_t, uint32_t, uint32_t, uint32_t);
void cal_param_search(uint32_t, uint32_t, uint32_t, uint32_t);
void cal_process_ind(uint32_t);
void nrf_pllcal_all();
void nrf_pllcal_results_mem(uint32_t, uint32_t, uint32_t, uint32_t);
void nrf_tx_bbgain_control(uint32_t tx_bq1_gain_val, uint32_t tx_bq2_gain_val);
void nrf_tx_gain_control(uint32_t tx_mixgain);
void nrf_spi_tx_gain_control(uint32_t tx_gain);
void nrf_cfo_cal(int32_t cfo_ppm, uint32_t *cfo_reg);
void nrf_cfo_set(uint32_t cfo_reg);
void nrf_test_tssi();
void nrf_trxgain_ctl_auto();
void nrf_trxgain_ctl_spi();
void nrf_mode_rxhp_ctl_auto(uint32_t hpf0,uint32_t hpf1,uint32_t hpf3);
void nrf_setting_for_trxtest();
void nrf_loiqcal_loopback(uint32_t cal_mode);
void phy_singen_tx();
void phy_singen_rx();
uint8_t get_iq_resolution();
void nrf_rtx_cal(void);
void nrf_txcal_path_on(void);
void nrf_txcal_path_off(void);
void nrf_rtx_cal_delay(uint32_t delay);
void nrf_txcal_val_restore(void);
void nrf_txcal_val_reset(void);
void nrf_txcal_val_manual(int16_t txiq_0, int16_t txiq_1, int16_t txlo_i, int16_t txlo_q);
void nrf_rx_gain_control_manual(uint16_t lna_gain,uint16_t tia_gain,uint16_t adc_gain,uint16_t dig_gain);
void phy_dump_rx(uint32_t trigger, uint32_t clock);
void nrf_bandwidth_set();
void nrf_dfe_reset_control();
int nrf_rtc_cal(void);
int rf_rx_gain();
void nrf_phy_reset(void);
void nrf_txgain_adjust(uint32_t format,int vif_id);
uint32_t cturn_cal();
void hal_rf_rx_only_init();
struct nrf_test_channel_reg* nrf_test_find(uint32_t ch_freq,uint32_t bw);
uint32_t nrf_freq_cal(uint8_t freq_index);
extern int32_t nrf_get_txpower_regval(uint32_t format,int vif_id);
uint32_t nrf_pll_cal(uint32_t freq_in,uint32_t fcalcfg,uint32_t fcaldly);
void nrf_r_cal(uint32_t itrim,uint32_t rtrim);
void nrf_ccal();
void nrf_dcoc_cal(void);
void Dynamic_trx_test();
void Dynamic_rx_test(uint32_t lna_gain1,uint32_t tia_gain1,uint32_t adc_gain1,uint32_t dig_gain1,uint32_t lna_gain2,uint32_t tia_gain2,uint32_t adc_gain2,uint32_t dig_gain2);
void Dynamic_hpf_test();
int util_ate_wifi_tx_sintone_send(uint32_t single_tone);
int util_ate_wifi_tx_sintone_stop();
#endif //__NRC_RF_H__

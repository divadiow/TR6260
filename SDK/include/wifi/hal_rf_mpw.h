#ifndef __NRC_RF_H__
#define __NRC_RF_H__

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
void nrf_txgain_adjust(uint32_t format);//add by wangxia 20190223 for compile

#endif //__NRC_RF_H__

#ifndef __NRC_PHY_H__
#define __NRC_PHY_H__

#define NRC6111

void hal_phy_init();

void mdelay();
void soft_spi_ch_set();
void dac_setting();
void initialize_mdac();
void initialize_phy_mrf();
void initialize_phy();
void enable_romgen(uint32_t, uint32_t);
void disable_romgen();
void phy_rx_gain_control(uint32_t);
void phy_set_cfo2sfo_factor(uint32_t, bool print);
void phy_op_bw_control(uint32_t bw);
void phy_status_counter_for_n();
void phy_status_counter_rst();
void agc_stf_rssi_measure(uint32_t);
void phy_op_primary(int vif_id, uint32_t primary);
void state_log();
void phy_reg_setting_for_nrf_rxtest();
void phy_reg_setting_for_nrf_txtest();
void phy_reg_setting_for_nrf_trxtest();
void ni_characterize(uint32_t wanted_ni_power);
void phy_reg_setting_for_nrf_board_dep(uint32_t nrf_bd_num);
void phy_rfloiq_cal(uint32_t cal_mode, uint32_t cal_range, uint32_t cal_step, uint32_t cal_search);
void phy_loiqcal_settings(uint32_t phy_cal_mode);
void phy_rfloiq_calparam_search(uint32_t cal_mode, uint32_t cal_range, uint32_t cal_step, uint32_t cal_search);
uint32_t phy_adc_data_read(uint8_t adc_path, uint8_t loop_count);
void phy_rtx_cal_debug_delay(uint32_t debug_delay);

enum {
    ///========================================================
    ///
    ///     Register Base Address
    ///
    ///========================================================

    PHY_REG_BASE_START      =  0x00a00000,
    PHY_REG_SIZE            =  4,

    ///========================================================
    ///
    ///     PHY Register definitions
    ///
    ///========================================================

    ///========================================================
    ///     PHY Register partition
    ///========================================================
    // 1. TX DFE
    PHY_REG_TX_DFE          = PHY_REG_BASE_START + 0x0000,
    // 2. RF DFE INT
    PHY_REG_RF_AFE_INT      = PHY_REG_BASE_START + 0x3000,
    // 3. RX DFE
    PHY_REG_RX_DFE          = PHY_REG_BASE_START + 0x4000,
    // 4. TOP
    PHY_REG_TOP             = PHY_REG_BASE_START + 0x6000,
    // 5. 11B
    PHY_REG_11B             = PHY_REG_BASE_START + 0x7000,

    ///========================================================
    ///     PHY Register descriptions
    ///========================================================

    // 1. TX DFE
    PHY_REG_TX_DFE_TOP                   = PHY_REG_TX_DFE     + 0x2000,
    PHY_REG_TX_DFE_LPF                   = PHY_REG_TX_DFE     + 0x2200,
    PHY_REG_TX_DFE_CAL                   = PHY_REG_TX_DFE     + 0x2400,
    PHY_REG_TX_DFE_CFR                   = PHY_REG_TX_DFE     + 0x2800,
    PHY_REG_MON_CTRL                     = PHY_REG_TX_DFE     + 0x2C00,

    // 2. RF DFE INT
    // No sub-partition

    // 3. RX DFE
    PHY_REG_RX_DFE_CTRL                  = PHY_REG_RX_DFE     + 0x0000,
    PHY_REG_RX_DFE_AGC                   = PHY_REG_RX_DFE     + 0x0400,
    PHY_REG_RX_DFE_LPF                   = PHY_REG_RX_DFE     + 0x0800,
    PHY_REG_RX_DFE_SYNC_CFO              = PHY_REG_RX_DFE     + 0x0C00,
    PHY_REG_RX_DFE_TRK                   = PHY_REG_RX_DFE     + 0x1000,
    PHY_REG_RX_DFE_CS                    = PHY_REG_RX_DFE     + 0x1400,
    PHY_REG_RX_DFE_SNREST                = PHY_REG_RX_DFE     + 0x1800,
    PHY_REG_RX_DFE_IQDC                  = PHY_REG_RX_DFE     + 0x1C00,

    // 4. TOP,     5.11B
    // No sub-partition


    ///========================================================
    ///========================================================
    ///========================================================
    ///========================================================
    ///========================================================
    // 1. TX DFE
    PHY_REG_TX_DFE_ROMGEN_ONOFF                                        = PHY_REG_TX_DFE        + PHY_REG_SIZE * 28 ,
    PHY_REG_TX_DFE_ROMGEN_FRAME_INTERVAL                               = PHY_REG_TX_DFE        + PHY_REG_SIZE * 29 ,
    PHY_REG_TX_DFE_ROMGEN_LEGACY_HT                                    = PHY_REG_TX_DFE        + PHY_REG_SIZE * 30 ,
    PHY_REG_TX_DFE_ROMGEN_MCS                                          = PHY_REG_TX_DFE        + PHY_REG_SIZE * 31 ,
    PHY_REG_TX_DFE_ROMGEN_CHBW                                         = PHY_REG_TX_DFE        + PHY_REG_SIZE * 32 ,

    PHY_REG_TX_DFE_SPI_SRC_SEL                                         = PHY_REG_TX_DFE        + PHY_REG_SIZE * 34 ,

    PHY_REG_TX_DFE_DAC_OFFSET_BIN                                      = PHY_REG_TX_DFE        + PHY_REG_SIZE * 40 ,
    PHY_REG_TX_DFE_ADC_OFFSET_BIN                                      = PHY_REG_TX_DFE        + PHY_REG_SIZE * 44 ,

    PHY_REG_TX_DFE_TOP_TXRESMP_ON                                      = PHY_REG_TX_DFE_TOP    + PHY_REG_SIZE * 1  ,
    PHY_REG_TX_DFE_TOP_TXRESMP0_RATIOM0                                = PHY_REG_TX_DFE_TOP    + PHY_REG_SIZE * 2  ,
    PHY_REG_TX_DFE_TOP_TXRESMP0_RSTNUMM1                               = PHY_REG_TX_DFE_TOP    + PHY_REG_SIZE * 3  ,
    PHY_REG_TX_DFE_TOP_TXRESMP_EN_FOR_LOIQCAL                          = PHY_REG_TX_DFE_TOP    + PHY_REG_SIZE * 6  ,
    PHY_REG_TX_DFE_TOP_TXRESMP0_RATIOM1                                = PHY_REG_TX_DFE_TOP    + PHY_REG_SIZE * 7  ,
    PHY_REG_TX_DFE_TOP_TXRESMP1_RATIOM0                                = PHY_REG_TX_DFE_TOP    + PHY_REG_SIZE * 8  ,
    PHY_REG_TX_DFE_TOP_TXRESMP1_RSTNUMM1                               = PHY_REG_TX_DFE_TOP    + PHY_REG_SIZE * 9  ,
    PHY_REG_TX_DFE_TOP_TXRESMP1_RATIOM1                                = PHY_REG_TX_DFE_TOP    + PHY_REG_SIZE * 10 ,
    PHY_REG_TX_DFE_CAL_TXIQ_PARA_0                                     = PHY_REG_TX_DFE_CAL    + PHY_REG_SIZE * 0  ,
    PHY_REG_TX_DFE_CAL_TXIQ_PARA_1                                     = PHY_REG_TX_DFE_CAL    + PHY_REG_SIZE * 1  ,
    PHY_REG_TX_DFE_CAL_TXLO_PARA_I                                     = PHY_REG_TX_DFE_CAL    + PHY_REG_SIZE * 2  ,
    PHY_REG_TX_DFE_CAL_TXLO_PARA_Q                                     = PHY_REG_TX_DFE_CAL    + PHY_REG_SIZE * 3  ,

    PHY_REG_MON_CTRL_ENA                                               = PHY_REG_MON_CTRL      + PHY_REG_SIZE * 0  ,
    PHY_REG_MON_CTRL_CLR                                               = PHY_REG_MON_CTRL      + PHY_REG_SIZE * 1  ,
    PHY_REG_MON_CTRL_CFG0                                              = PHY_REG_MON_CTRL      + PHY_REG_SIZE * 2  ,
    PHY_REG_MON_CTRL_CFG1                                              = PHY_REG_MON_CTRL      + PHY_REG_SIZE * 3  ,
    PHY_REG_MON_CTRL_RPT                                               = PHY_REG_MON_CTRL      + PHY_REG_SIZE * 4  ,

    PHY_REG_MON_CTRL_TIME_BASE                                         = PHY_REG_MON_CTRL      + PHY_REG_SIZE * 64 ,
    PHY_REG_MON_CTRL_STATE_BASE                                        = PHY_REG_MON_CTRL      + PHY_REG_SIZE * 128,

    // 2. RF DFE INT
    PHY_REG_RF_AFE_INT_SPI_CONFIG                                      = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 1  ,
    PHY_REG_RF_AFE_INT_SPI_HW_SELF_MODE                                = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 2  ,
    PHY_REG_RF_AFE_INT_RF_SPI_SW_TX_DATA	                           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 6  ,

	PHY_REG_ENTRX_MANUAL_MODE										   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 7 ,
	PHY_REG_ENTRX_MANUAL_CFG										   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 8 ,
    PHY_REG_RF_AFE_INT_RF_SPI_RD_DATA	                               = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 10 ,
	PHY_REG_TRXGAIN_MANUAL_MODE 									   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 11 ,

    PHY_REG_RF_AFE_INT_ADC_CTL                 	                       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 12 ,
    PHY_REG_RF_AFE_INT_ADC_CTL_SEL          	                       = PHY_REG_RF_AFE_INT_ADC_CTL                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_SEL_SHIFT                               = 0                                         ,
    PHY_REG_RF_AFE_INT_ADC_CTL_SEL_MASK                                = 0x00000001                                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_INT_ENA          	                   = PHY_REG_RF_AFE_INT_ADC_CTL                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_INT_ENA_SHIFT                           = 1                                         ,
    PHY_REG_RF_AFE_INT_ADC_CTL_INT_ENA_MASK                            = 0x00000002                                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_INT_IQSWAP          	                   = PHY_REG_RF_AFE_INT_ADC_CTL                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_INT_IQSWAP_SHIFT                        = 2                                         ,
    PHY_REG_RF_AFE_INT_ADC_CTL_INT_IQSWAP_MASK                         = 0x00000004                                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_INT_OFFSET_BIN          	               = PHY_REG_RF_AFE_INT_ADC_CTL                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_INT_OFFSET_BIN_SHIFT                    = 3                                         ,
    PHY_REG_RF_AFE_INT_ADC_CTL_INT_OFFSET_BIN_MASK                     = 0x00000008                                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_EXT_ENA          	                   = PHY_REG_RF_AFE_INT_ADC_CTL                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_EXT_ENA_SHIFT                           = 4                                         ,
    PHY_REG_RF_AFE_INT_ADC_CTL_EXT_ENA_MASK                            = 0x00000010                                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_EXT_IQSWAP          	                   = PHY_REG_RF_AFE_INT_ADC_CTL                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_EXT_IQSWAP_SHIFT                        = 5                                         ,
    PHY_REG_RF_AFE_INT_ADC_CTL_EXT_IQSWAP_MASK                         = 0x00000020                                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_EXT_OFFSET_BIN          	               = PHY_REG_RF_AFE_INT_ADC_CTL                ,
    PHY_REG_RF_AFE_INT_ADC_CTL_EXT_OFFSET_BIN_SHIFT                    = 6                                         ,
    PHY_REG_RF_AFE_INT_ADC_CTL_EXT_OFFSET_BIN_MASK                     = 0x00000040                                ,

    PHY_REG_RF_AFE_INT_DAC_CTL                 	                       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 13 ,
    PHY_REG_RF_AFE_INT_DAC_CTL_SEL          	                       = PHY_REG_RF_AFE_INT_DAC_CTL                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_SEL_SHIFT                               = 0                                         ,
    PHY_REG_RF_AFE_INT_DAC_CTL_SEL_MASK                                = 0x00000001                                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_INT_ENA          	                   = PHY_REG_RF_AFE_INT_DAC_CTL                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_INT_ENA_SHIFT                           = 1                                         ,
    PHY_REG_RF_AFE_INT_DAC_CTL_INT_ENA_MASK                            = 0x00000002                                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_INT_IQSWAP          	                   = PHY_REG_RF_AFE_INT_DAC_CTL                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_INT_IQSWAP_SHIFT                        = 2                                         ,
    PHY_REG_RF_AFE_INT_DAC_CTL_INT_IQSWAP_MASK                         = 0x00000004                                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_INT_OFFSET_BIN          	               = PHY_REG_RF_AFE_INT_DAC_CTL                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_INT_OFFSET_BIN_SHIFT                    = 3                                         ,
    PHY_REG_RF_AFE_INT_DAC_CTL_INT_OFFSET_BIN_MASK                     = 0x00000008                                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_EXT_ENA          	                   = PHY_REG_RF_AFE_INT_DAC_CTL                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_EXT_ENA_SHIFT                           = 4                                         ,
    PHY_REG_RF_AFE_INT_DAC_CTL_EXT_ENA_MASK                            = 0x00000010                                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_EXT_IQSWAP          	                   = PHY_REG_RF_AFE_INT_DAC_CTL                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_EXT_IQSWAP_SHIFT                        = 5                                         ,
    PHY_REG_RF_AFE_INT_DAC_CTL_EXT_IQSWAP_MASK                         = 0x00000020                                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_EXT_OFFSET_BIN          	               = PHY_REG_RF_AFE_INT_DAC_CTL                ,
    PHY_REG_RF_AFE_INT_DAC_CTL_EXT_OFFSET_BIN_SHIFT                    = 6                                         ,
    PHY_REG_RF_AFE_INT_DAC_CTL_EXT_OFFSET_BIN_MASK                     = 0x00000040                                ,
    PHY_REG_RF_AFE_INT_RF_LNA_TXGAIN_CTL          	                   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 15 ,
    PHY_REG_RF_AFE_INT_RF_TIA_TXGAIN_CTL          	                   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 16 ,
    PHY_REG_RF_AFE_INT_RF_BQ1_TXGAIN_CTL          	                   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 17 ,
    PHY_REG_RF_AFE_INT_RF_BQ2_TXGAIN_CTL          	                   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 18 ,
    PHY_REG_RF_AFE_INT_NRF_PLLCAL_BSEARCH_TRIG            	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 20 ,
    PHY_REG_RF_AFE_INT_NRF_PLLCAL_CBCLK_TRN_TRIG            	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 21 ,


    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG            	                   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 22 ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_TXEN                              = PHY_REG_RF_AFE_INT_RF_CTL_CONFIG          ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_TXEN_SHIFT                        = 0                                         ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_TXEN_MASK                         = 0x00000001                                ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_RXEN                              = PHY_REG_RF_AFE_INT_RF_CTL_CONFIG          ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_RXEN_SHIFT                        = 1                                         ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_RXEN_MASK                         = 0x00000002                                ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_PAON                              = PHY_REG_RF_AFE_INT_RF_CTL_CONFIG          ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_PAON_SHIFT                        = 2                                         ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_PAON_MASK                         = 0x00000004                                ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_ANTSEL                            = PHY_REG_RF_AFE_INT_RF_CTL_CONFIG          ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_ANTSEL_SHIFT                      = 3                                         ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_ANTSEL_MASK                       = 0x00000008                                ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_ANTSEL_B                          = PHY_REG_RF_AFE_INT_RF_CTL_CONFIG          ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_ANTSEL_B_SHIFT                    = 4                                         ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_ANTSEL_B_MASK                     = 0x00000010                                ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_SHDN_N                            = PHY_REG_RF_AFE_INT_RF_CTL_CONFIG          ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_SHDN_N_SHIFT                      = 5                                         ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_SHDN_N_MASK                       = 0x00000020                                ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_MANUAL_MODE                       = PHY_REG_RF_AFE_INT_RF_CTL_CONFIG          ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_MANUAL_MODE_SHIFT                 = 6                                         ,
    PHY_REG_RF_AFE_INT_RF_CTL_CONFIG_MANUAL_MODE_MASK                  = 0x00000040                                ,


    PHY_REG_RF_AFE_INT_RF_TXEN_PRE_DLY             	                   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 23 ,

    PHY_REG_RF_AFE_INT_RF_SHDN                                         = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 25 ,
    PHY_REG_RF_AFE_INT_RF_SEL                    	                   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 172,
    PHY_REG_RF_AFE_INT_RF_TXGAIN_CTL                                   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 179,
    PHY_REG_RF_AFE_INT_NRF_RADIO_MODE             	                   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 28 ,
    PHY_REG_RF_AFE_INT_NRF_PLLCAL_EN              	                   = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 29 ,
    PHY_REG_RF_AFE_INT_NRF_PLLCAL_FINE_CBCODE              	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 32 ,
    PHY_REG_RF_AFE_INT_NRF_PLLCAL_COARSE_CBCODE              	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 33 ,
    PHY_REG_RF_AFE_INT_NRF_PLLCAL_PROCESS_DONE              	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 34 ,
    PHY_REG_RF_AFE_INT_NRF_PLLCAL_SUCCESS_DONE              	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 35 ,
    PHY_REG_RF_AFE_INT_NRF_RCCAL_EN                         	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 36 ,
    PHY_REG_RF_AFE_INT_NRF_RCCAL_DONE                       	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 38 ,
    PHY_REG_RF_AFE_INT_NRF_RCCAL_CNTOUT                      	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 39 ,

    PHY_REG_RF_AFE_INT_NRFMON_TSEN_EN                      	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 47 ,
    PHY_REG_RF_AFE_INT_NRFMON_VBAT_EN                      	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 48 ,
    PHY_REG_RF_AFE_INT_NRFMON_TSSI_EN                      	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 49 ,
    PHY_REG_RF_AFE_INT_NRFMON_POWER                        	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 53 ,
    PHY_REG_RF_AFE_INT_NRFMON_TSSI_DONE                      	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 56 ,


    PHY_REG_RF_AFE_INT_RF_LOIQCAL_CAL_MODE                   	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 57 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_TONE_FREQ                 	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 58 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_EN                      	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 59 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_LOOPBACK_DLY             	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 60 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_LENGTH                   	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 61 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_STG_TEST                 	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 62 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_ERROR                  	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 64 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_VAL                   	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 65 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_OUT_I                  	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 66 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_OUT_Q                  	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 67 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_TXIQ_PARA_0             	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 68 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_TXIQ_PARA_1             	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 69 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_TXLO_PARA_I             	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 70 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_TXLO_PARA_Q             	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 71 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_RXIQ_PARA_0             	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 72 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_RXIQ_PARA_1             	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 73 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_TX_DGAIN             	               = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 81 , // 0x0144 Add By Tommykim(2017.01.02)
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_MIS_TXCAL_TEST             	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 82 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_MIS_RXCAL_TEST             	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 83 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_MIS_TXIQ_PARA_0             	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 84 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_MIS_TXIQ_PARA_1             	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 85 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_MIS_TXLO_PARA_I             	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 86 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_MIS_TXLO_PARA_Q             	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 87 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_MIS_RXIQ_PARA_0             	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 88 ,
    PHY_REG_RF_AFE_INT_RF_LOIQCAL_MIS_RXIQ_PARA_1             	       = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 89 ,





    PHY_REG_RF_AFE_INT_CAL_PROCESS_MON                   	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 90 ,
//#ifdef NRC6111
    PHY_REG_RF_AFE_INT_CAL_EN                                          = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 92 , // 0x0170
    PHY_REG_RF_AFE_INT_NRF_RADIO_MODE_MANUAL                           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 95 , // 0x017C
//#endif

    PHY_REG_RF_AFE_INT_INTADC_DC_OFFSET_I                  	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 128,
    PHY_REG_RF_AFE_INT_INTADC_DC_OFFSET_Q                  	           = PHY_REG_RF_AFE_INT    + PHY_REG_SIZE * 129,

    // 3. RX DFE
    // CTRL
    PHY_REG_RX_DFE_CTRL_REFPWR_RCPI                                    = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 7  ,
    PHY_REG_RX_DFE_CTRL_STR_PKT_ON                                     = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 4  ,
    PHY_REG_RX_DFE_CTRL_RXRESMP1_RSTNUMM1                              = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 6  ,
    PHY_REG_RX_DFE_CTRL_RXRESMP1_RATIOM1                               = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 10 ,
    PHY_REG_RX_DFE_CTRL_RXRESMP1_RATIOM0                               = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 11 ,
    PHY_REG_RX_DFE_CTRL_RXRESMP0_RATIOM1                               = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 12 ,
    PHY_REG_RX_DFE_CTRL_11B_OFF                                        = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 15 ,
    PHY_REG_RX_DFE_CTRL_RXRESMP_ON                                     = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 16 ,
    PHY_REG_RX_DFE_CTRL_RXRESMP0_RATIOM0                               = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 17 ,

    PHY_REG_RX_DFE_CTRL_RXRESMP0_RSTNUMM1                              = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 18 ,
    PHY_REG_RX_DFE_CTRL_RXRESMP_EN_FOR_LOIQCAL                         = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 19 ,


    PHY_REG_RXDFE_CTRL_MON_CFG                                         = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 44 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_00                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 45 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_01                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 46 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_02                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 47 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_03                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 48 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_04                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 49 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_05                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 50 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_06                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 51 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_07                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 52 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_08                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 53 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_09                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 54 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_10                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 55 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_11                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 56 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_12                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 57 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_13                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 58 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_14                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 59 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_15                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 60 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_16                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 61 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_17                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 62 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_18                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 63 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_19                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 64 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_20                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 65 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_24                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 70 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_25                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 71 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_27                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 72 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_28                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 73 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_29                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 74 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_30                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 75 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_31                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 76 ,
	/*begin add by wbh 20190416*/
	PHY_REG_RXDFE_CTRL_MON_LOG_28_CHNL_MON							   = PHY_REG_RXDFE_CTRL_MON_LOG_28,
	PHY_REG_RXDFE_CTRL_MON_LOG_28_CHNL_MON_SHIFT					   = 0,
	PHY_REG_RXDFE_CTRL_MON_LOG_28_CHNL_MON_MASK 					   = 0xffffffff,
	//PHY_REG_RXDFE_CTRL_MON_LOG_29									   = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 74 ,
	PHY_REG_RXDFE_CTRL_MON_LOG_29_CHNL_MON							   = PHY_REG_RXDFE_CTRL_MON_LOG_29,
	PHY_REG_RXDFE_CTRL_MON_LOG_29_CHNL_MON_SHIFT					   = 0,
	PHY_REG_RXDFE_CTRL_MON_LOG_29_CHNL_MON_MASK 					   = 0xffffffff,
	//PHY_REG_RXDFE_CTRL_MON_LOG_30									   = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 75 ,
	PHY_REG_RXDFE_CTRL_MON_LOG_30_CHNL_MON							   = PHY_REG_RXDFE_CTRL_MON_LOG_30,
	PHY_REG_RXDFE_CTRL_MON_LOG_30_CHNL_MON_SHIFT					   = 0,
	PHY_REG_RXDFE_CTRL_MON_LOG_30_CHNL_MON_MASK 					   = 0xffffffff,
	//PHY_REG_RXDFE_CTRL_MON_LOG_31									   = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 76 ,
	PHY_REG_RXDFE_CTRL_MON_LOG_31_CHNL_MON							   = PHY_REG_RXDFE_CTRL_MON_LOG_31,
	PHY_REG_RXDFE_CTRL_MON_LOG_31_CHNL_MON_SHIFT					   = 0,
	PHY_REG_RXDFE_CTRL_MON_LOG_31_CHNL_MON_MASK 					   = 0xffffffff,
	/*end add by wbh 20190416*/


	PHY_REG_RXDFE_CTRL_MON_LOG_32                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 77 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_33                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 78 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_34                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 79 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_35                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 80 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_36                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 81 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_37                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 82 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_38                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 83 ,
    PHY_REG_RXDFE_CTRL_MON_LOG_39                                      = PHY_REG_RX_DFE_CTRL   + PHY_REG_SIZE * 84 ,

    //AGC
    PHY_REG_RX_DFE_AGC_MODE_SEL                                        = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 0  ,
    PHY_REG_RX_DFE_AGC_TIME_OP                                         = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 1  ,
    PHY_REG_RX_DFE_AGC_LOCK_DIFF                                       = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 2  ,
    PHY_REG_RX_DFE_AGC_G_CTRL_NO                                       = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 3  ,
    PHY_REG_RX_DFE_AGC_WAIT_PRD                                        = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 5  ,
    PHY_REG_RX_DFE_AGC_TIA_INIT_GAIN                                   = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 9  ,
    PHY_REG_RX_DFE_AGC_LNA_GAIN_M                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 12 ,
    PHY_REG_RX_DFE_AGC_LNA_GAIN_S                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 13 ,
    PHY_REG_RX_DFE_AGC_REF_LOG                                         = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 14 ,
    PHY_REG_RX_DFE_AGC_GAINL                                           = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 15 ,
    PHY_REG_RX_DFE_AGC_MANUAL_MODE                                     = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 44 ,
    PHY_REG_RX_DFE_AGC_MANUAL_RXHP                                     = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 45 ,
    PHY_REG_RX_DFE_AGC_NRC_MANUAL_LNA_GAIN                             = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 46 ,
    PHY_REG_RX_DFE_AGC_NRC_MANUAL_TIA_GAIN                             = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 47 ,
    PHY_REG_RX_DFE_AGC_NRC_MANUAL_BQ1_GAIN                             = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 48 ,
    PHY_REG_RX_DFE_AGC_NRC_MANUAL_BQ2_GAIN                             = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 49 ,
    PHY_REG_RX_DFE_AGC_MANUAL_DIGITAL_GAIN                             = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 50 ,

    PHY_REG_RX_DFE_AGC_FINE_AGC_BYPASS                                 = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 52 ,
    PHY_REG_RX_DFE_AGC_DIG_AMP_BYPASS                                  = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 53 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP00                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 54 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP01                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 55 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP02                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 56 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP03                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 57 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP04                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 58 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP05                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 59 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP06                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 60 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP07                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 61 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP08                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 62 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP09                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 63 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP10                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 64 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP11                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 65 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP12                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 66 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP13                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 67 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP14                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 68 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP15                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 69 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP16                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 70 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP17                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 71 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP18                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 72 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP19                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 73 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP20                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 74 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP21                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 75 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP22                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 76 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP23                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 77 ,
    PHY_REG_RX_DFE_AGC_GAIN_MAP24                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 78 ,

    PHY_REG_RX_DFE_AGC_GAIN_HOLD_EN                                    = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 79 ,
    PHY_REG_RX_DFE_AGC_GAIN_HOLD_REF                                   = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 80 ,
    PHY_REG_RX_DFE_AGC_GAIN_HOLD_THR                                   = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 81 ,
    PHY_REG_RX_DFE_AGC_SAT_LIMIT_CNT                                   = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 84 ,
    PHY_REG_RX_DFE_AGC_RXHP_SEL                                        = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 85 ,
    PHY_REG_RX_DFE_AGC_WAIT_CON_EN                                     = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 88 ,
    PHY_REG_RX_DFE_AGC_WAIT_PRD_L                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 89 ,
    PHY_REG_RX_DFE_AGC_SAT_END_EN                                      = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 90 ,
    PHY_REG_RX_DFE_AGC_L_UP_CNT                                        = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 92 ,
    PHY_REG_RX_DFE_AGC_TIA_GAIN0                                       = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 94 ,
    PHY_REG_RX_DFE_AGC_TIA_GAIN1                                       = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 95 ,
    PHY_REG_RX_DFE_AGC_TIA_GAIN2                                       = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 96 ,
    PHY_REG_RX_DFE_AGC_BQ1_GAIN0                                       = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 97 ,
    PHY_REG_RX_DFE_AGC_BQ1_GAIN1                                       = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 98 ,
    PHY_REG_RX_DFE_AGC_BQ1_GAIN2                                       = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 99 ,
    PHY_REG_RX_DFE_AGC_BQ2_GAIN0                                       = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 100,
    PHY_REG_RX_DFE_AGC_BQ2_GAIN1                                       = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 101,
    PHY_REG_RX_DFE_AGC_BQ2_GAIN2                                       = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 102,
    PHY_REG_RX_DFE_AGC_NRF_RSSI_TEST_EN                                = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 197,
    PHY_REG_RX_DFE_AGC_NRF_RSSI_TEST_DONE                              = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 198,
    PHY_REG_RX_DFE_AGC_NRF_RSSI0_RES                                   = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 199,
    PHY_REG_RX_DFE_AGC_NRF_RSSI1_RES                                   = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 200,
    PHY_REG_RX_DFE_AGC_NRF_RSSI2_RES                                   = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 201 ,
    PHY_REG_RX_DFE_AGC_NRF_RSSI3_RES                                   = PHY_REG_RX_DFE_AGC    + PHY_REG_SIZE * 202 ,


    // CS
    PHY_REG_RX_DFE_CS_ENABLE                                           = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 0  ,
    PHY_REG_RX_DFE_CS_RXEN_SEL                                         = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 1  ,
    PHY_REG_RX_DFE_CS_RXEN_DLY                                         = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 2  ,
    PHY_REG_RX_DFE_CS_CHK_DLY_EN                                       = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 3  ,
    PHY_REG_RX_DFE_CS_CHK_DLY_CNT                                      = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 4  ,
    PHY_REG_RX_DFE_CS_CHK_DLY_CNT_RIFS                                 = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 5  ,
    PHY_REG_RX_DFE_CS_SAT_THR                                          = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 8  ,
    PHY_REG_RX_DFE_CS_SAT_DET_CNT                                      = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 9  ,
    PHY_REG_RX_DFE_CS_SAT_WIN_SIZE                                     = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 11 ,
    PHY_REG_RX_DFE_CS_SAT_AGC_WAIT_EN                                  = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 12 ,
    PHY_REG_RX_DFE_CS_XCR_PWR_WGT                                      = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 16 ,
    PHY_REG_RX_DFE_CS_XCR_PWR_THR                                      = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 17 ,
    PHY_REG_RX_DFE_CS_XCR_RSSI_THR                                     = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 18 ,
    PHY_REG_RX_DFE_CS_PRE_DET_EN                                       = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 24 ,
    PHY_REG_RX_DFE_CS_PRE_DET_FORCE                                    = PHY_REG_RX_DFE_CS     + PHY_REG_SIZE * 25 ,
    PHY_REG_RX_DFE_SYNC_RX_FSYNC                                       = PHY_REG_RX_DFE_SYNC_CFO + PHY_REG_SIZE * 2  ,
    PHY_REG_RX_DFE_SYNC_RX_CFO                                         = PHY_REG_RX_DFE_SYNC_CFO + PHY_REG_SIZE * 3  ,

    // IQDC
    PHY_REG_RX_DFE_IQDC_HPF_ON                                         = PHY_REG_RX_DFE_IQDC   + PHY_REG_SIZE * 0  ,
    PHY_REG_RX_DFE_IQDC_HPF_WHILE_AGC                                  = PHY_REG_RX_DFE_IQDC   + PHY_REG_SIZE * 1  ,
    PHY_REG_RX_DFE_IQDC_HPF_WGT0                                       = PHY_REG_RX_DFE_IQDC   + PHY_REG_SIZE * 2  ,
    PHY_REG_RX_DFE_IQDC_HPF_WGT1                                       = PHY_REG_RX_DFE_IQDC   + PHY_REG_SIZE * 3  ,
    PHY_REG_RX_DFE_IQDC_HPF_WGT2                                       = PHY_REG_RX_DFE_IQDC   + PHY_REG_SIZE * 4  ,

    PHY_REG_RX_DFE_IQDC_RXIQ_PARA_0                                    = PHY_REG_RX_DFE_IQDC   + PHY_REG_SIZE * 6  ,
    PHY_REG_RX_DFE_IQDC_RXIQ_PARA_1                                    = PHY_REG_RX_DFE_IQDC   + PHY_REG_SIZE * 7  ,

    // TRK
    PHY_REG_RXDFE_TRK_CFO2SFO_FACTOR                                   = PHY_REG_RX_DFE_TRK    + PHY_REG_SIZE * 0  ,

    // 4. TOP
    PHY_REG_TOP_SW_RST                                                 = PHY_REG_TOP           + PHY_REG_SIZE * 4  ,
    PHY_REG_TOP_OPMODE                                                 = PHY_REG_TOP           + PHY_REG_SIZE * 8  ,
//-------------------------------------------------------------------------------------------------------------------------------,
    PHY_REG_TOP_PRICH                                                  = PHY_REG_TOP           + PHY_REG_SIZE * 9  ,
//-------------------------------------------------------------------------------------------------------------------------------,
		PHY_REG_TOP_PRICH_SELECT_0                                	   = PHY_REG_TOP_PRICH,
		PHY_REG_TOP_PRICH_SELECT_0_SHIFT                           	   = 0,
		PHY_REG_TOP_PRICH_SELECT_0_MASK          		               = 0x00000001,

		PHY_REG_TOP_PRICH_SELECT_1                                	   = PHY_REG_TOP_PRICH,
		PHY_REG_TOP_PRICH_SELECT_1_SHIFT                           	   = 1,
		PHY_REG_TOP_PRICH_SELECT_1_MASK 		                       = 0x00000002,
//-------------------------------------------------------------------------------------------------------------------------------,
    PHY_REG_TOP_RND_SCR_SEED                                           = PHY_REG_TOP           + PHY_REG_SIZE * 10 ,
//-------------------------------------------------------------------------------------------------------------------------------,
    PHY_REG_TOP_TXSMPLRATE                                             = PHY_REG_TOP           + PHY_REG_SIZE * 11 ,

    //PHY_REG_TOP_MRF_BAND_SEL                                           = PHY_REG_TOP           + PHY_REG_SIZE * 82 ,

    PHY_REG_TOP_PSCM_PHY_VER                                           = PHY_REG_TOP           + PHY_REG_SIZE * 84 ,
    PHY_REG_TOP_PSCM_MAC_VER                                           = PHY_REG_TOP           + PHY_REG_SIZE * 85 ,
    PHY_REG_TOP_PSCM_SOC_VER                                           = PHY_REG_TOP           + PHY_REG_SIZE * 86 ,

    PHY_REG_TOP_PHY_TOP_STATE                                          = PHY_REG_TOP           + PHY_REG_SIZE * 128,

    PHY_REG_TOP_RXDBE_STATE                                            = PHY_REG_TOP           + PHY_REG_SIZE * 320,

    // 5. 11B
    //PHY_REG_11B_00                                                     = PHY_REG_11B           + PHY_REG_SIZE * 0  ,
    PHY_REG_11B_CFO2SFO_FACTOR                                         = PHY_REG_11B           + PHY_REG_SIZE * 0  ,
    PHY_REG_11B_CS_PWR_SEL_THR                                         = PHY_REG_11B           + PHY_REG_SIZE * 14 ,
    PHY_REG_11B_TARGET_PWR                                             = PHY_REG_11B           + PHY_REG_SIZE * 17 ,
    PHY_REG_11B_CS_RSSI_LIMIT                                          = PHY_REG_11B           + PHY_REG_SIZE * 18 ,



};

#endif //__NRC_PHY_H__

#ifndef HAL_LMAC_DEBUG_H
#define HAL_LMAC_DEBUG_H
#ifndef HAL_LMAC_COMM_STATIS
//#define HAL_LMAC_COMM_STATIS
#endif


//#define HAL_VER_HTOL
typedef struct {
uint32_t reg_addr;
uint32_t reg_val;
}RESTORE_REG_ST;
#ifdef HAL_LMAC_COMM_STATIS
typedef struct{
	uint32_t type :4;
	uint32_t subtype :5;
	uint32_t comm_direction :2;
	uint32_t comm_valid_data_stream :2;
	uint32_t comm_time_point :19;
}COMM_STATISTICS_ATTR_ST;

#define COMM_STATISTICS_MAX (1000)
typedef struct{
COMM_STATISTICS_ATTR_ST comm_statistics_attr[COMM_STATISTICS_MAX];
uint16_t rcd_head_index;
uint8_t comm_statis_enable;
uint32_t comm_statis_last_rtc ;
}COMM_STATISTICS_ST;

enum{
	LMAC_COMM_DEFALT = 0,
	LMAC_COMM_DOWNLINK,
	LMAC_COMM_UPLINK,
	LMAC_COMM_MAX,
};
#endif
void hal_lmac_report();
void lmac_check_txrx_timer_cb(TimerHandle_t xTimer);
int32_t lmac_check_txrx_timer_init(uint32_t interval);
int reset_mac(uint32_t *rxbuff);
void hal_lmac_reset_all();
//void hal_lmac_comm_statistics(SYS_BUF *buffer,uint8_t direction_mode);
#endif //HAL_LMAC_DEBUG_H

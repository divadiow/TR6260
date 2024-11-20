#ifndef _INDIRECT_REG_H_
#define _INDIRECT_REG_H_

#ifndef RELEASE
#if LMAC_CONFIG_11N == 1
#define N_CATEGORY     5
enum {
    //MAX_TX = 112,
    MAX_TX = 88,
    MAX_RX = 45,
    MAX_DMA = 26,
    MAX_SEC = 12,
    MAX_IRQ = 16,
    CNT_MAX = 115
};
#elif LMAC_CONFIG_11AH == 1
#define N_CATEGORY     5
enum {
    MAX_TX = 96,
    MAX_RX = 53,
    MAX_SEC = 7,
    MAX_DMA = 26,
    MAX_IRQ = 27,
    CNT_MAX = 100 
};
#endif //#if LMAC_CONFIG_11N == 1

typedef struct {
	uint16_t    offset;
	uint16_t    operand;
	uint8_t     start_bit;
	uint8_t     end_bit;
	char        notation;
	const char  *description;
	const char  *unit;
} RegDetail_t;


typedef struct {
	const char  *category;
	uint32_t    addr;
	uint32_t    data_addr;
	uint16_t    list_count;
	RegDetail_t reg_list[CNT_MAX];
} IndirectReg_t;
extern void show_indirect_reg(const char *cat);
uint32_t hal_mac_is_need_reset(uint8_t txrx_mode/*0: rx 1:tx*/);
#endif //#ifndef RELEASE
#endif //#ifndef _INDIRECT_REG_H_

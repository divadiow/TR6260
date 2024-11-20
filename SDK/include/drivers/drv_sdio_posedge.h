#include <nds32_intrinsic.h>
//#include <stdarg.h>
//#include <stdio.h>

#include "system_common.h"


/* SDIO device controller */
#define SDIO_BASE_ADDR       (0x00B00000)
#define SDIO_CONTROL         (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x00)))
#define SDIO_COMMAND         (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x04)))
#define SDIO_ARGUMENT1       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x08)))
#define SDIO_BLOCK           (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x0C)))
#define SDIO_DMA1_ADDR       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x10)))
#define SDIO_DMA1_CTL        (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x14)))
#define SDIO_DMA2_ADDR       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x18)))
#define SDIO_DMA2_CTL        (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x1C)))
#define SDIO_ERASE_START     (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x20)))
#define SDIO_ERASE_END       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x24)))
#define SDIO_PASSWD_LEN      (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x28)))
#define SDIO_SECURE_BC       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x2C)))
#define SDIO_RESERVED0       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x30)))
#define SDIO_RESERVED1       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x34)))
#define SDIO_RESERVED2       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x38)))
#define SDIO_INT1_STATUS     (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x3C)))
#define SDIO_INT1_STATUS_EN  (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x40)))
#define SDIO_INT1_EN         (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x44)))
#define SDIO_CARD_ADDR       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x48)))
#define SDIO_CARD_DATA       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x4C)))
#define SDIO_IOREADY         (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x50)))
#define SDIO_FUN1_CTL        (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x54)))
#define SDIO_FUN2_CTL        (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x58)))
#define SDIO_CCCR            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x5C)))
#define SDIO_FBR1            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x60)))
#define SDIO_FBR2            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x64)))
#define SDIO_FBR3            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x68)))
#define SDIO_FBR4            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x6C)))
#define SDIO_FBR5            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x70)))
#define SDIO_FBR6            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x74)))
#define SDIO_FBR7            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x78)))
#define SDIO_FBR8            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x7C)))
#define SDIO_CARD_SIZE       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x80)))
#define SDIO_CARD_OCR        (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x84)))
#define SDIO_CONTROL2        (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x88)))
#define SDIO_FUN3            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x90)))
#define SDIO_FUN4            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x94)))
#define SDIO_FUN5            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x98)))
#define SDIO_INT2_STATUS     (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x9C)))
#define SDIO_INT2_STATUS_EN  (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xA0)))
#define SDIO_INT2_EN         (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xA4)))
#define SDIO_PASSWD_127_96   (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xA8)))
#define SDIO_PASSWD_95_64    (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xAC)))
#define SDIO_PASSWD_63_32    (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xB0)))
#define SDIO_PASSWD_31_0     (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xB4)))
#define SDIO_ADMA_ERR        (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xB8)))
#define SDIO_RCA             (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xBC)))
#define SDIO_DBG0            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xC0)))
#define SDIO_DBG1            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xC4)))
#define SDIO_DBG2            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xC8)))
#define SDIO_DBG3            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xCC)))
#define SDIO_DBG4            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xD0)))
#define SDIO_DBG5            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xD4)))
#define SDIO_DBG6            (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xD8)))
#define SDIO_AHB_BURST       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xDC)))
#define SDIO_ARGUMENT2       (*((volatile unsigned int *)(SDIO_BASE_ADDR + 0xE0)))


#define SDIO_INFO_REG		(*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x1C0)))
#define SDIO_INFO_EN		(*((volatile unsigned int *)(SDIO_BASE_ADDR + 0x1C4)))



#define IS_FUNCX_ABORT(X)				((X) & BIT29)
#define IS_PROGRAM_START(X)			((X) & BIT25)
#define IS_FUNC1_RESET(X)				((X) & BIT21)
#define IS_SOFT_RESET(X)				((X) & BIT12)
#define IS_RX_START(X)					((X) & BIT3)
#define IS_TX_START(X)					((X) & BIT4)
#define IS_TRANSFER_COMPLETE(X)		((X) & BIT0)

#define IS_ADMA_ERROR()					((SDIO_INT2_STATUS) & BIT6)




#define SDIO_BLOCK_SIZE			(SDIO_ARGUMENT1 & 0x1FF)//((SDIO_COMMAND >> 1) & 0xFFF)
#define SDIO_BLOCK_COUNT		(SDIO_BLOCK) //(SDIO_ARGUMENT1 & 0x1FF)

#define SDIO_TRANSFER_SIZE		(SDIO_BLOCK_SIZE * SDIO_BLOCK_COUNT)
#define SDIO_TRANSFER_ADDR	((SDIO_ARGUMENT1>>9) & 0x1FFFF)


#define SDIO_INT_CLR(X)			(SDIO_INT1_STATUS = X)  
#define SDIO_FUN1_IND(X)		(SDIO_FUN1_CTL = X)

#define SDIO_ADMA_EN()			(SDIO_CONTROL2	   |= BIT2)

#define SDIO_ADMA_ENABLE		1
#define SDIO_ADMA_BLK			0

#define SINGLE_MASK_MAP (1)

//#define SDIO_512_BASE			(0xC0BE00)
#if 1
//#define SDIO_MSG_BASE				(0xC0BE20)
#define SDIO_MSG_BASE				(0x00253FF0)
//#define SDIO_MSG_BASE				(0xC0FFF0)
//#define SDIO_MSG_BASE				(0xC00014)


#define SDIO_DES_BASE_INFO		(0x00253F20)
#define SDIO_DES_BASE_RX		(0x00253F30)
#define SDIO_DES_BASE_TX		(0x00253F60)

#else
#define SDIO_MSG_BASE				(0xC0BFF0)
//#define SDIO_MSG_BASE				(0xC0FFF0)
//#define SDIO_MSG_BASE				(0xC00014)


#define SDIO_DES_BASE_INFO				(0xC0BE20)
#define SDIO_DES_BASE_RX				(0xC0BE30)
#define SDIO_DES_BASE_TX				(0xC0BE60)
#endif

#define SDIO_MSG_CREDIT_VIF0	(*((volatile uint32_t *)(SDIO_MSG_BASE + 0x00)))
#define SDIO_MSG_CREDIT_VIF1	(*((volatile uint32_t *)(SDIO_MSG_BASE + 0x04)))
#define SDIO_MSG_RX				(*((volatile uint32_t *)(SDIO_MSG_BASE + 0x08)))
#define SDIO_MSG_TX				(*((volatile uint32_t *)(SDIO_MSG_BASE + 0x0C)))


#if SDIO_ADMA_BLK
#define SDIO_MSG_LEN		512//0x10
#else
#define SDIO_MSG_LEN		0x1//0x10//512//0x10
#endif

#define SDIO_MSG_GET_CREDIT_VIF0	SDIO_MSG_CREDIT_VIF0
#define SDIO_MSG_GET_CREDIT_VIF1	SDIO_MSG_CREDIT_VIF1
#define SDIO_MSG_GET_RX()			SDIO_MSG_RX
#define SDIO_MSG_GET_TX			SDIO_MSG_TX

#define SDIO_MSG_SET_CREDIT_VIF0(X)	(SDIO_MSG_CREDIT_VIF0 = X)
#define SDIO_MSG_SET_CREDIT_VIF1(X)	(SDIO_MSG_CREDIT_VIF1 = X)
#define SDIO_MSG_SET_RX(X)				(SDIO_MSG_RX = X)
#define SDIO_MSG_SET_TX(X, N)			((*((volatile uint32_t *)(SDIO_MSG_BASE + 0x0C + N * 4))) = X)

#define  SDIO_IDLE    0
#define  SDIO_INIT    1
#define  SDIO_WR      2
#define  SDIO_RD      3
#define  SDIO_RX_DAT  4
#define  SDIO_TX_DAT  5
#define  SDIO_TX_INFO	6

#define SDIO_RD_IDLE	0
#define SDIO_RD_INFO	1
#define SDIO_RD_DATA	2

#define OCR 0x00ff8000
#define LRST 0
#define SMID_RST_N 1
#define SD_VER_SEL 2
#define SD_MMC_VER_SEL 3
#define MMC_VER_SEL 5
#define CMD_ACCEPT  6

#define SDIO_PARA_CONFIG0       (*((volatile unsigned int *)0x6018018))
#define SDIO_PARA_CONFIG1       (*((volatile unsigned int *)0x601801c))


#define SDIO_ISR_STATUS1_ABORT (BIT29)

#define SDIO_ADMA_ATTR_VALID (BIT0)
#define SDIO_ADMA_ATTR_END   (BIT1)
#define SDIO_ADMA_ATTR_INT   (BIT2)
#define SDIO_ADMA_ATTR_ACT_NOP (0 << 4)
#define SDIO_ADMA_ATTR_ACT_TRS (2 << 4)
#define SDIO_ADMA_ATTR_ACT_LINK (3 << 4)





struct adma_descriptor {
	unsigned short attri;
	unsigned short len;
	unsigned int addr;
}__packed;

void drv_sdio_posedge_reset();
void drv_sdio_posedge_set(uint32_t address);
void drv_sdio_posedge_start(uint32_t address);
void drv_sdio_posedge_ind(uint32_t szie);
uint32_t drv_sdio_get_int1_status();
uint32_t drv_sdio_get_int2_status();
void drv_sdio_posedge_write_done();
volatile uint32_t drv_sdio_posedge_get_func1_control();


#include <nds32_intrinsic.h>
#include "drv_sdio_posedge.h"

void drv_sdio_posedge_reset()
{
	SDIO_PARA_CONFIG0 = OCR;
	SDIO_PARA_CONFIG1 = (1>>LRST|
		                1>>SMID_RST_N|
		                0>>SD_VER_SEL|
		                2>>SD_MMC_VER_SEL|
		                0>>MMC_VER_SEL|
		                0>>CMD_ACCEPT);

	SDIO_CARD_OCR		= 0x11FF8000;
//	SDIO_CCCR			= 0x23F3F343;
	SDIO_CCCR			= 0x23FFF343;
	
	SDIO_INT1_STATUS_EN = 0xFFFFFFFF;
	SDIO_INT1_STATUS	= 0xFFFFFFFF;
	SDIO_INT1_EN		= 0xFFFFFFFF;

	SDIO_INT2_STATUS_EN = 0xFFFFFFFF;
	SDIO_INT2_STATUS	= 0xFFFFFFFF;
	SDIO_INT2_EN		= 0xFFFFFFFF;
	
	SDIO_IOREADY		= BIT1;  // function 1 is ready
	SDIO_CONTROL	&= 0xFF1FFFFF;
	SDIO_CONTROL	   |= BIT2;  // card is ready to operate
//	SDIO_CONTROL2	   |= BIT2;  // adma-enalbe
//		PRINT("sdio reset\n");
}

void drv_sdio_posedge_set(uint32_t address)
{
	SDIO_DMA1_ADDR = address;
	SDIO_DMA1_CTL  = 1;
}

void drv_sdio_posedge_start(uint32_t address)
{
	SDIO_DMA1_ADDR = address;
	SDIO_DMA1_CTL  = 0xF;
}

void drv_sdio_posedge_ind(uint32_t szie)
{
	SDIO_FUN1_CTL = szie;
}

uint32_t drv_sdio_get_int1_status()
{
	return SDIO_INT1_STATUS;
}

uint32_t drv_sdio_get_int2_status()
{
	return SDIO_INT2_STATUS;
}

void drv_sdio_posedge_write_done()
{
    SDIO_CONTROL |= 1;    // write done
}

volatile uint32_t drv_sdio_posedge_get_func1_control()
{
    return SDIO_FUN1_CTL;
}

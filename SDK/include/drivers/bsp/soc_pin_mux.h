/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:soc_pin_mux.h    
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2018-12-19
 * History 1:      
 *     Date: 2019-3-6
 *     Version:
 *     Author: wangxia
 *     Modification: add TR6260S/TR6260_3 pin_mux define 
 * History 2: 
  ********************************************************************************/

#ifndef _SOC_PIN_MUX_H
#define _SOC_PIN_MUX_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/
#include "soc_top_reg.h"
#include "system_common.h"

/****************************************************************************
* 	                                        Macros
****************************************************************************/


/****************************************************************************
* 	                                        Types
****************************************************************************/

/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/

#if (defined _USR_TR6260 || defined MPW || defined _USER_LMAC_SDIO)
//------------------------IO_MUX_0-----------------------------------
//GPIO0
#define IO_MUX0_GPIO0_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO0_BITS			7 
#define IO_MUX0_GPIO0_OFFSET		0 //bit2:bit0
#define FUNC_GPIO0_TCK				0
#define FUNC_GPIO0_GPIO0			1
#define FUNC_UART0_DTR				2
#define FUNC_GPIO0_SPI0_CLK			3
#define FUNC_GPIO0_PWM_CTRL0		4
#define FUNC_GPIO0_UART0_RXD		5
#define FUNC_GPIO0_I2S_TXSCK		6
#define FUNC_GPIO0_I2C_SCL			7

//GPIO1
#define IO_MUX0_GPIO1_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO1_BITS			7 
#define IO_MUX0_GPIO1_OFFSET		3 //bit5:bit3
#define FUNC_GPIO1_TMS				0
#define FUNC_GPIO1_GPIO1			1
#define FUNC_GPIO1_UART0_DSR		2
#define FUNC_GPIO1_SPI0_CS0			3
#define FUNC_GPIO1_PWM_CTRL1		4
#define FUNC_GPIO1_BT_ACTIVE		5
#define FUNC_GPIO1_I2S_RXD			6
#define FUNC_GPIO1_I2C_SDA			7

//GPIO2
#define IO_MUX0_GPIO2_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO2_BITS			7 
#define IO_MUX0_GPIO2_OFFSET		6 //bit8:bit6
#define FUNC_GPIO2_TDO				0
#define FUNC_GPIO2_GPIO2			1
#define FUNC_GPIO2_UART1_RXD		2
#define FUNC_GPIO2_SPI0_MOSI		3
#define FUNC_GPIO2_PWM_CTRL2		4
#define FUNC_GPIO2_BT_PRI			5
#define FUNC_GPIO2_I2S_RXWS			6
#define FUNC_GPIO2_UART2_RXD		7

//GPIO3
#define IO_MUX0_GPIO3_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO3_BITS			7 
#define IO_MUX0_GPIO3_OFFSET		9 //bit11:bit9
#define FUNC_GPIO3_TDI				0
#define FUNC_GPIO3_GPIO3			1
#define FUNC_GPIO3_UART1_TXD		2
#define FUNC_GPIO3_SPI0_MIS0		3
#define FUNC_GPIO3_PWM_CTRL3		4
#define FUNC_GPIO3_W_ACTIVE			5
#define FUNC_GPIO3_I2S_RXSCK		6
#define FUNC_GPIO3_UART2_TXD		7

//GPIO4
#define IO_MUX0_GPIO4_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO4_BITS			7 
#define IO_MUX0_GPIO4_OFFSET		12//bit14:bit12
#define FUNC_GPIO4_TRST				0
#define FUNC_GPIO4_GPIO4			1
#define FUNC_GPIO4_GPIO4_SPI0_CLK	2
#define FUNC_GPIO4_SPI0_CS1			3
#define FUNC_GPIO4_PWM_CTRL4		4
#define FUNC_GPIO4_W_PRI			5
#define FUNC_GPIO4_I2S_MCLK			6

//GPIO5
#define IO_MUX0_GPIO5_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO5_BITS			7 
#define IO_MUX0_GPIO5_OFFSET		15//bit17:bit15
#define FUNC_GPIO5_UART0_RXD		0
#define FUNC_GPIO5_GPIO5			1
#define FUNC_GPIO5_SPI0_CS0			2
#define FUNC_GPIO5_UART1_CTS		3
#define FUNC_GPIO5_SPI0_HOLD		4
#define FUNC_GPIO5_40M_CLK_OUT		5
#define FUNC_GPIO5_PCU_DEBUG0		8


//GPIO6
#define IO_MUX0_GPIO6_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO6_BITS			7 
#define IO_MUX0_GPIO6_OFFSET		18//bit20:bit18
#define FUNC_GPIO6_UART0_TXD		0
#define FUNC_GPIO6_GPIO6			1
#define FUNC_GPIO6_SPI0_MOSI		2
#define FUNC_GPIO6_MSPI_CS1			3
#define FUNC_GPIO6_SPI0_WP			4
#define FUNC_GPIO6_COLD_RESET		5
#define FUNC_GPIO6_PCU_DEBUG1		8


//GPIO7
#define IO_MUX0_GPIO7_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO7_BITS			7 
#define IO_MUX0_GPIO7_OFFSET		21//bit23:bit21
#define FUNC_GPIO7_SD_DATA0			0
#define FUNC_GPIO7_GPIO7			1
#define FUNC_GPIO7_UART0_CTS		2
#define FUNC_GPIO7_MSPI_MOSI		3
#define FUNC_GPIO7_UART0_TXD		4
#define FUNC_GPIO7_PCU_DEBUG2		8

//GPIO8
#define IO_MUX0_GPIO8_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO8_BITS			7 
#define IO_MUX0_GPIO8_OFFSET		24//bit26:bit24
#define FUNC_GPIO8_SD_DATA1			0
#define FUNC_GPIO8					1
#define FUNC_GPIO8_UART0_RTS		2
#define FUNC_GPIO8_MSPI_MISO		3
#define FUNC_GPIO8_SPI0_CS1			4
#define FUNC_GPIO8_PCU_DEBUG3		8

//GPIO9
#define IO_MUX0_GPIO9_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO9_BITS			7 
#define IO_MUX0_GPIO9_OFFSET		27//bit29:bit27
#define FUNC_GPIO9_SD_DATA2			0
#define FUNC_GPIO9_GPIO9			1
#define FUNC_GPIO9_UART1_DSR		2
#define FUNC_GPIO9_MSPI_WP			3
#define FUNC_GPIO9_I2C_SCL			4
#define FUNC_GPIO9_UART1_RXD		5
#define FUNC_GPIO9_PCU_DEBUG4		8

//GPIO24
#define IO_MUX0_GPIO24_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO24_BITS			3 
#define IO_MUX0_GPIO24_OFFSET		30//bit31:bit30
#define FUNC_GPIO24_GPIO24			0
#define FUNC_GPIO24_I2S_MCLK		1
#define FUNC_GPIO24_I2S_RXSCK		2
#define FUNC_GPIO24_PWM_CTRL4		3



//------------------------IO_MUX_1-----------------------------------
//GPIO10
#define IO_MUX0_GPIO10_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO10_BITS			7 
#define IO_MUX0_GPIO10_OFFSET		0//bit2:bit0
#define FUNC_GPIO10_SD_DATA3		0
#define FUNC_GPIO10					1
#define FUNC_GPIO10_UART1_DTR		2
#define FUNC_GPIO10_MSPI_HOLD		3
#define FUNC_GPIO10_I2C_SDA			4
#define FUNC_GPIO10_UART1_TXD		5
#define FUNC_GPIO10_PCU_DEBUG5	    8

//GPIO11
#define IO_MUX0_GPIO11_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO11_BITS			7 
#define IO_MUX0_GPIO11_OFFSET		3//bit5:bit3
#define FUNC_GPIO11_SD_CLK			0
#define FUNC_GPIO11_GPIO11			1
#define FUNC_GPIO11_UART1_RTS		2
#define FUNC_GPIO11_MSPI_CLK		3
#define FUNC_GPIO11_UART1_RXD		4
#define FUNC_GPIO11_PCU_DEBUG6	    8

//GPIO12
#define IO_MUX0_GPIO12_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO12_BITS			7 
#define IO_MUX0_GPIO12_OFFSET		6//bit8:bit6
#define FUNC_GPIO12_SD_CMD			0
#define FUNC_GPIO12_GPIO12			1
#define FUNC_GPIO12_UART1_CTS		2
#define FUNC_GPIO12_MSPI_CS0		3
#define FUNC_GPIO12_UART1_TXD		4
#define FUNC_GPIO12_32K_CLK_IN		5
#define FUNC_GPIO12_PCU_DEBUG7		8

//GPIO13
#define IO_MUX0_GPIO13_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO13_BITS			7 
#define IO_MUX0_GPIO13_OFFSET		9//bit11:bit9
#define FUNC_GPIO13_WAKE_UP			0
#define FUNC_GPIO13_GPIO13			1
#define FUNC_GPIO13_I2S_TXD			2
#define FUNC_GPIO13_SPI0_MISO		3
#define FUNC_GPIO13_PWM_CTRL5		4
#define FUNC_GPIO13_32K_CLK_OUT		5
#define FUNC_GPIO13_PHY_ENTRX		6

//GPIO14
#define IO_MUX0_GPIO14_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO14_BITS			7 
#define IO_MUX0_GPIO14_OFFSET		12//bit14:bit12
#define FUNC_GPIO14_BOOTMODE0		0
#define FUNC_GPIO14_GPIO14			1
#define FUNC_GPIO14_1_TOUT2			2
#define FUNC_GPIO14_2_TOUT2			3
#define FUNC_GPIO14_PWM_CTRL3		4
#define FUNC_GPIO14_ATST_A			5
#define FUNC_GPIO14_I2S_TXD			6
#define FUNC_GPIO14_UART2_RXD		7

//GPIO15
#define IO_MUX0_GPIO15_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO15_BITS			7 
#define IO_MUX0_GPIO15_OFFSET		15//bit17:bit15
#define FUNC_GPIO15_BOOTMODE1		0
#define FUNC_GPIO15_GPIO15			1
#define FUNC_GPIO15_1_TOUT3			2
#define FUNC_GPIO15_2_TOUT3			3
#define FUNC_GPIO15_PWM_CTRL5		4
#define FUNC_GPIO15_ATST_B			5
#define FUNC_GPIO15_I2S_TXWS		6
#define FUNC_GPIO15_UART2_TXD		7


//GPIO18
#define IO_MUX0_GPIO18_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO18_BITS			1 
#define IO_MUX0_GPIO18_OFFSET		18//bit18
#define FUNC_GPIO18_GPIO18			0
#define FUNC_GPIO18_TOUT1			2

//GPIO19
#define IO_MUX0_GPIO19_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO19_BITS			1 
#define IO_MUX0_GPIO19_OFFSET		19//bit19
#define FUNC_GPIO19_GPIO19			0
#define FUNC_GPIO19_TOUT0			2

//GPIO20
#define IO_MUX0_GPIO20_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO20_BITS			7 
#define IO_MUX0_GPIO20_OFFSET		20//bit22:bit20
#define FUNC_GPIO20_GPIO20			0
#define FUNC_GPIO20_UART0_RXD		1
#define FUNC_GPIO20_I2S_TXWS		2
#define FUNC_GPIO20_PWM_CTRL0		3
#define FUNC_GPIO20_BT_ACTIVE		4
#define FUNC_GPIO20_UART2_RXD		5

//GPIO21
#define IO_MUX0_GPIO21_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO21_BITS			7 
#define IO_MUX0_GPIO21_OFFSET		23//bit25:bit23
#define FUNC_GPIO21_GPIO21			0
#define FUNC_GPIO21_UART0_TXD		1
#define FUNC_GPIO21_I2S_TXSCK		2
#define FUNC_GPIO21_PWM_CTRL1		3
#define FUNC_GPIO21_BT_PRIO			4
#define FUNC_GPIO21_UART2_TXD		5

//GPIO22
#define IO_MUX0_GPIO22_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO22_BITS			7 
#define IO_MUX0_GPIO22_OFFSET		26//bit28:bit26
#define FUNC_GPIO22_GPIO22			0
#define FUNC_GPIO22_UART1_RXD		1
#define FUNC_GPIO22_I2S_RXD			2
#define FUNC_GPIO22_PWM_CTRL2		3
#define FUNC_GPIO22_W_ACTIVE		4
#define FUNC_GPIO22_UART2_RXD		5

//GPIO23
#define IO_MUX0_GPIO23_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO23_BITS			7 
#define IO_MUX0_GPIO23_OFFSET		29//bit31:bit29
#define FUNC_GPIO23_GPIO23			0
#define FUNC_GPIO23_UART1_TXD		1
#define FUNC_GPIO23_I2S_RXWS		2
#define FUNC_GPIO23_PWM_CTRL3		3
#define FUNC_GPIO23_W_PRIO			4
#define FUNC_GPIO23_UART2_TXD		5

#endif/*TR6260*/


/*-----------------------------------------------TR6260S-------------------------------------------------*/

#ifdef _USR_TR6260S1
//------------------------IO_MUX_0-----------------------------------
//GPIO0
#define IO_MUX0_GPIO0_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO0_BITS			7 
#define IO_MUX0_GPIO0_OFFSET		0 //bit2:bit0
#define FUNC_GPIO0_TCK				0
#define FUNC_GPIO0_GPIO0			1
#define FUNC_UART0_DTR				2
#define FUNC_GPIO0_SPI0_CLK			3
#define FUNC_GPIO0_PWM_CTRL0		4
#define FUNC_GPIO0_UART0_RXD		5
#define FUNC_GPIO0_I2S_TXSCK		6
#define FUNC_GPIO0_I2C_SCL			7

//GPIO1
#define IO_MUX0_GPIO1_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO1_BITS			7 
#define IO_MUX0_GPIO1_OFFSET		3 //bit5:bit3
#define FUNC_GPIO1_TMS				0
#define FUNC_GPIO1_GPIO1			1
#define FUNC_GPIO1_UART0_DSR		2
#define FUNC_GPIO1_SPI0_CS0			3
#define FUNC_GPIO1_PWM_CTRL1		4
#define FUNC_GPIO1_BT_ACTIVE		5
#define FUNC_GPIO1_I2S_RXD			6
#define FUNC_GPIO1_I2C_SDA			7

//GPIO2
#define IO_MUX0_GPIO2_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO2_BITS			7 
#define IO_MUX0_GPIO2_OFFSET		6 //bit8:bit6
#define FUNC_GPIO2_TDO				0
#define FUNC_GPIO2_GPIO2			1
#define FUNC_GPIO2_UART1_RXD		2
#define FUNC_GPIO2_SPI0_MOSI		3
#define FUNC_GPIO2_PWM_CTRL2		4
#define FUNC_GPIO2_BT_PRI			5
#define FUNC_GPIO2_I2S_RXWS			6
#define FUNC_GPIO2_UART2_RXD		7

//GPIO3
#define IO_MUX0_GPIO3_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO3_BITS			7 
#define IO_MUX0_GPIO3_OFFSET		9 //bit11:bit9
#define FUNC_GPIO3_TDI				0
#define FUNC_GPIO3_GPIO3			1
#define FUNC_GPIO3_UART1_TXD		2
#define FUNC_GPIO3_SPI0_MIS0		3
#define FUNC_GPIO3_PWM_CTRL3		4
#define FUNC_GPIO3_W_ACTIVE			5
#define FUNC_GPIO3_I2S_RXSCK		6
#define FUNC_GPIO3_UART2_TXD		7

//GPIO4
#define IO_MUX0_GPIO4_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO4_BITS			7 
#define IO_MUX0_GPIO4_OFFSET		12//bit14:bit12
#define FUNC_GPIO4_TRST				0
#define FUNC_GPIO4_GPIO4			1
#define FUNC_GPIO4_GPIO4_SPI0_CLK	2
#define FUNC_GPIO4_SPI0_CS1			3
#define FUNC_GPIO4_PWM_CTRL4		4
#define FUNC_GPIO4_W_PRI			5
#define FUNC_GPIO4_I2S_MCLK			6

//GPIO5
#define IO_MUX0_GPIO5_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO5_BITS			7 
#define IO_MUX0_GPIO5_OFFSET		15//bit17:bit15
#define FUNC_GPIO5_UART0_RXD		0
#define FUNC_GPIO5_GPIO5			1
#define FUNC_GPIO5_SPI0_CS0			2
#define FUNC_GPIO5_UART1_CTS		3
#define FUNC_GPIO5_SPI0_HOLD		4
#define FUNC_GPIO5_40M_CLK_OUT		5
#define FUNC_GPIO5_PCU_DEBUG0		8


//GPIO6
#define IO_MUX0_GPIO6_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO6_BITS			7 
#define IO_MUX0_GPIO6_OFFSET		18//bit20:bit18
#define FUNC_GPIO6_UART0_TXD		0
#define FUNC_GPIO6_GPIO6			1
#define FUNC_GPIO6_SPI0_MOSI		2
#define FUNC_GPIO6_MSPI_CS1			3
#define FUNC_GPIO6_SPI0_WP			4
#define FUNC_GPIO6_COLD_RESET		5
#define FUNC_GPIO6_PCU_DEBUG1		8

//GPIO13
#define IO_MUX0_GPIO13_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO13_BITS			7 
#define IO_MUX0_GPIO13_OFFSET		9//bit11:bit9
#define FUNC_GPIO13_WAKE_UP			0
#define FUNC_GPIO13_GPIO13			1
#define FUNC_GPIO13_I2S_TXD			2
#define FUNC_GPIO13_SPI0_MISO		3
#define FUNC_GPIO13_PWM_CTRL5		4
#define FUNC_GPIO13_32K_CLK_OUT		5
#define FUNC_GPIO13_PHY_ENTRX		6

//GPIO14
#define IO_MUX0_GPIO14_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO14_BITS			7 
#define IO_MUX0_GPIO14_OFFSET		12//bit14:bit12
#define FUNC_GPIO14_BOOTMODE0		0
#define FUNC_GPIO14_GPIO14			1
#define FUNC_GPIO14_1_TOUT2			2
#define FUNC_GPIO14_2_TOUT2			3
#define FUNC_GPIO14_PWM_CTRL3		4
#define FUNC_GPIO14_ATST_A			5
#define FUNC_GPIO14_I2S_TXD			6
#define FUNC_GPIO14_UART2_RXD		7

//GPIO15
#define IO_MUX0_GPIO15_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO15_BITS			7 
#define IO_MUX0_GPIO15_OFFSET		15//bit17:bit15
#define FUNC_GPIO15_BOOTMODE1		0
#define FUNC_GPIO15_GPIO15			1
#define FUNC_GPIO15_1_TOUT3			2
#define FUNC_GPIO15_2_TOUT3			3
#define FUNC_GPIO15_PWM_CTRL5		4
#define FUNC_GPIO15_ATST_B			5
#define FUNC_GPIO15_I2S_TXWS		6
#define FUNC_GPIO15_UART2_TXD		7

//GPIO20
#define IO_MUX0_GPIO20_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO20_BITS			7 
#define IO_MUX0_GPIO20_OFFSET		20//bit22:bit20
#define FUNC_GPIO20_GPIO20			0
#define FUNC_GPIO20_UART0_RXD		1
#define FUNC_GPIO20_I2S_TXWS		2
#define FUNC_GPIO20_PWM_CTRL0		3
#define FUNC_GPIO20_BT_ACTIVE		4
#define FUNC_GPIO20_UART2_RXD		5

//GPIO21
#define IO_MUX0_GPIO21_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO21_BITS			7 
#define IO_MUX0_GPIO21_OFFSET		23//bit25:bit23
#define FUNC_GPIO21_GPIO21			0
#define FUNC_GPIO21_UART0_TXD		1
#define FUNC_GPIO21_I2S_TXSCK		2
#define FUNC_GPIO21_PWM_CTRL1		3
#define FUNC_GPIO21_BT_PRIO			4
#define FUNC_GPIO21_UART2_TXD		5

//GPIO22
#define IO_MUX0_GPIO22_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO22_BITS			7 
#define IO_MUX0_GPIO22_OFFSET		26//bit28:bit26
#define FUNC_GPIO22_GPIO22			0
#define FUNC_GPIO22_UART1_RXD		1
#define FUNC_GPIO22_I2S_RXD			2
#define FUNC_GPIO22_PWM_CTRL2		3
#define FUNC_GPIO22_W_ACTIVE		4
#define FUNC_GPIO22_UART2_RXD		5

#endif/*TR6260S1*/

/*-----------------------------------------------TR6260_3-------------------------------------------------*/
#ifdef _USR_TR6260_3
//------------------------IO_MUX_0-----------------------------------
//GPIO0
#define IO_MUX0_GPIO0_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO0_BITS			7 
#define IO_MUX0_GPIO0_OFFSET		0 //bit2:bit0
#define FUNC_GPIO0_TCK				0
#define FUNC_GPIO0_GPIO0			1
#define FUNC_UART0_DTR				2
#define FUNC_GPIO0_SPI0_CLK			3
#define FUNC_GPIO0_PWM_CTRL0		4
#define FUNC_GPIO0_UART0_RXD		5
#define FUNC_GPIO0_I2S_TXSCK		6
#define FUNC_GPIO0_I2C_SCL			7

//GPIO1
#define IO_MUX0_GPIO1_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO1_BITS			7 
#define IO_MUX0_GPIO1_OFFSET		3 //bit5:bit3
#define FUNC_GPIO1_TMS				0
#define FUNC_GPIO1_GPIO1			1
#define FUNC_GPIO1_UART0_DSR		2
#define FUNC_GPIO1_SPI0_CS0			3
#define FUNC_GPIO1_PWM_CTRL1		4
#define FUNC_GPIO1_BT_ACTIVE		5
#define FUNC_GPIO1_I2S_RXD			6
#define FUNC_GPIO1_I2C_SDA			7

//GPIO2
#define IO_MUX0_GPIO2_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO2_BITS			7 
#define IO_MUX0_GPIO2_OFFSET		6 //bit8:bit6
#define FUNC_GPIO2_TDO				0
#define FUNC_GPIO2_GPIO2			1
#define FUNC_GPIO2_UART1_RXD		2
#define FUNC_GPIO2_SPI0_MOSI		3
#define FUNC_GPIO2_PWM_CTRL2		4
#define FUNC_GPIO2_BT_PRI			5
#define FUNC_GPIO2_I2S_RXWS			6
#define FUNC_GPIO2_UART2_RXD		7

//GPIO3
#define IO_MUX0_GPIO3_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO3_BITS			7 
#define IO_MUX0_GPIO3_OFFSET		9 //bit11:bit9
#define FUNC_GPIO3_TDI				0
#define FUNC_GPIO3_GPIO3			1
#define FUNC_GPIO3_UART1_TXD		2
#define FUNC_GPIO3_SPI0_MIS0		3
#define FUNC_GPIO3_PWM_CTRL3		4
#define FUNC_GPIO3_W_ACTIVE			5
#define FUNC_GPIO3_I2S_RXSCK		6
#define FUNC_GPIO3_UART2_TXD		7

//GPIO4
#define IO_MUX0_GPIO4_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO4_BITS			7 
#define IO_MUX0_GPIO4_OFFSET		12//bit14:bit12
#define FUNC_GPIO4_TRST				0
#define FUNC_GPIO4_GPIO4			1
#define FUNC_GPIO4_GPIO4_SPI0_CLK	2
#define FUNC_GPIO4_SPI0_CS1			3
#define FUNC_GPIO4_PWM_CTRL4		4
#define FUNC_GPIO4_W_PRI			5
#define FUNC_GPIO4_I2S_MCLK			6

//GPIO5
#define IO_MUX0_GPIO5_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO5_BITS			7 
#define IO_MUX0_GPIO5_OFFSET		15//bit17:bit15
#define FUNC_GPIO5_UART0_RXD		0
#define FUNC_GPIO5_GPIO5			1
#define FUNC_GPIO5_SPI0_CS0			2
#define FUNC_GPIO5_UART1_CTS		3
#define FUNC_GPIO5_SPI0_HOLD		4
#define FUNC_GPIO5_40M_CLK_OUT		5
#define FUNC_GPIO5_PCU_DEBUG0		8


//GPIO6
#define IO_MUX0_GPIO6_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO6_BITS			7 
#define IO_MUX0_GPIO6_OFFSET		18//bit20:bit18
#define FUNC_GPIO6_UART0_TXD		0
#define FUNC_GPIO6_GPIO6			1
#define FUNC_GPIO6_SPI0_MOSI		2
#define FUNC_GPIO6_MSPI_CS1			3
#define FUNC_GPIO6_SPI0_WP			4
#define FUNC_GPIO6_COLD_RESET		5
#define FUNC_GPIO6_PCU_DEBUG1		8


//GPIO7
#define IO_MUX0_GPIO7_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO7_BITS			7 
#define IO_MUX0_GPIO7_OFFSET		21//bit23:bit21
#define FUNC_GPIO7_SD_DATA0			0
#define FUNC_GPIO7_GPIO7			1
#define FUNC_GPIO7_UART0_CTS		2
#define FUNC_GPIO7_MSPI_MOSI		3
#define FUNC_GPIO7_UART0_TXD		4
#define FUNC_GPIO7_PCU_DEBUG2		8

//GPIO8
#define IO_MUX0_GPIO8_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO8_BITS			7 
#define IO_MUX0_GPIO8_OFFSET		24//bit26:bit24
#define FUNC_GPIO8_SD_DATA1			0
#define FUNC_GPIO8					1
#define FUNC_GPIO8_UART0_RTS		2
#define FUNC_GPIO8_MSPI_MISO		3
#define FUNC_GPIO8_SPI0_CS1			4
#define FUNC_GPIO8_PCU_DEBUG3		8

//GPIO9
#define IO_MUX0_GPIO9_REG			SOC_PIN0_MUX_BASE
#define IO_MUX0_GPIO9_BITS			7 
#define IO_MUX0_GPIO9_OFFSET		27//bit29:bit27
#define FUNC_GPIO9_SD_DATA2			0
#define FUNC_GPIO9_GPIO9			1
#define FUNC_GPIO9_UART1_DSR		2
#define FUNC_GPIO9_MSPI_WP			3
#define FUNC_GPIO9_I2C_SCL			4
#define FUNC_GPIO9_UART1_RXD		5
#define FUNC_GPIO9_PCU_DEBUG4		8

//------------------------IO_MUX_1-----------------------------------
//GPIO10
#define IO_MUX0_GPIO10_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO10_BITS			7 
#define IO_MUX0_GPIO10_OFFSET		0//bit2:bit0
#define FUNC_GPIO10_SD_DATA3		0
#define FUNC_GPIO10					1
#define FUNC_GPIO10_UART1_DTR		2
#define FUNC_GPIO10_MSPI_HOLD		3
#define FUNC_GPIO10_I2C_SDA			4
#define FUNC_GPIO10_UART1_TXD		5
#define FUNC_GPIO10_PCU_DEBUG5	    8

//GPIO11
#define IO_MUX0_GPIO11_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO11_BITS			7 
#define IO_MUX0_GPIO11_OFFSET		3//bit5:bit3
#define FUNC_GPIO11_SD_CLK			0
#define FUNC_GPIO11_GPIO11			1
#define FUNC_GPIO11_UART1_RTS		2
#define FUNC_GPIO11_MSPI_CLK		3
#define FUNC_GPIO11_UART1_RXD		4
#define FUNC_GPIO11_PCU_DEBUG6	    8

//GPIO12
#define IO_MUX0_GPIO12_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO12_BITS			7 
#define IO_MUX0_GPIO12_OFFSET		6//bit8:bit6
#define FUNC_GPIO12_SD_CMD			0
#define FUNC_GPIO12_GPIO12			1
#define FUNC_GPIO12_UART1_CTS		2
#define FUNC_GPIO12_MSPI_CS0		3
#define FUNC_GPIO12_UART1_TXD		4
#define FUNC_GPIO12_32K_CLK_IN		5
#define FUNC_GPIO12_PCU_DEBUG7		8

//GPIO13
#define IO_MUX0_GPIO13_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO13_BITS			7 
#define IO_MUX0_GPIO13_OFFSET		9//bit11:bit9
#define FUNC_GPIO13_WAKE_UP			0
#define FUNC_GPIO13_GPIO13			1
#define FUNC_GPIO13_I2S_TXD			2
#define FUNC_GPIO13_SPI0_MISO		3
#define FUNC_GPIO13_PWM_CTRL5		4
#define FUNC_GPIO13_32K_CLK_OUT		5
#define FUNC_GPIO13_PHY_ENTRX		6

//GPIO14
#define IO_MUX0_GPIO14_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO14_BITS			7 
#define IO_MUX0_GPIO14_OFFSET		12//bit14:bit12
#define FUNC_GPIO14_BOOTMODE0		0
#define FUNC_GPIO14_GPIO14			1
#define FUNC_GPIO14_1_TOUT2			2
#define FUNC_GPIO14_2_TOUT2			3
#define FUNC_GPIO14_PWM_CTRL3		4
#define FUNC_GPIO14_ATST_A			5
#define FUNC_GPIO14_I2S_TXD			6
#define FUNC_GPIO14_UART2_RXD		7

//GPIO15
#define IO_MUX0_GPIO15_REG			SOC_PIN1_MUX_BASE
#define IO_MUX0_GPIO15_BITS			7 
#define IO_MUX0_GPIO15_OFFSET		15//bit17:bit15
#define FUNC_GPIO15_BOOTMODE1		0
#define FUNC_GPIO15_GPIO15			1
#define FUNC_GPIO15_1_TOUT3			2
#define FUNC_GPIO15_2_TOUT3			3
#define FUNC_GPIO15_PWM_CTRL5		4
#define FUNC_GPIO15_ATST_B			5
#define FUNC_GPIO15_I2S_TXWS		6
#define FUNC_GPIO15_UART2_TXD		7

#endif/*TR6260_3*/
#define PIN_MUX_SET(reg,bits,offset,fun) OUT32(reg, (IN32(reg) & (~(bits<<offset))) |(fun<<offset))
#define PIN_FUNC_SET(PIN_NAME,PIN_FUNC) \
		PIN_MUX_SET(PIN_NAME##_REG, PIN_NAME##_BITS, PIN_NAME##_OFFSET, PIN_FUNC)

#endif/*_SOC_PIN_MUX_H*/



/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:soc_top_reg.h    
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangchao
 * Date:          2018-8-19
 * History 1:      
 *     Date: 2018-12-19
 *     Version:
 *     Author: wangxia
 *     Modification: add mod top reg 
 * History 2: 
  ********************************************************************************/

/****************************************************************************
* 	                                           Include files
****************************************************************************/


/****************************************************************************
* 	                                           Local Macros
****************************************************************************/

/****************************************************************************
* 	                                           Local Types
****************************************************************************/

/****************************************************************************
* 	                                           Local Constants
****************************************************************************/

/****************************************************************************
* 	                                           Local Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Global Constants
****************************************************************************/

/****************************************************************************
* 	                                          Global Variables
****************************************************************************/

/****************************************************************************
* 	                                          Global Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/

//CLOCK
#define CLK_MUX_BASE		(0x00601804)
#define CLK_EN_BASE			(0x0060180C)
#define SW_RESET			(0x00601810)
#define CLK_DBB		        (0x00601CAC)

#define CLK_AHB_DAMC		(1<<0)
#define CLK_SDIO			(1<<1)
#define CLK_SPI1			(1<<2)
#define CLK_SPI_FLASH		(1<<3)
#define CLK_UART0			(1<<4)
#define CLK_UART1			(1<<5)
#define CLK_I2C				(1<<6)
#define CLK_GPIO			(1<<7)
#define CLK_PIT0			(1<<8)
#define CLK_PIT1			(1<<9)
#define CLK_WCT				(1<<10)
#define CLK_DOT				(1<<11)
#define CLK_BUF_MEM			(1<<12)
#define CLK_RFC				(1<<13)
#define CLK_TRNG			(1<<14)
#define CLK_SPI_FLASH_AHB	(1<<15)
#define CLK_DUMP			(1<<16)
#define CLK_EFUSE			(1<<17)
#define CLK_AES				(1<<18)
#define CLK_CSPI			(1<<19)
#define CLK_PHY_TX			(1<<20)
#define CLK_PHY_RX			(1<<21)
#define CLK_I2S_AHB			(1<<22)
#define CLK_I2S_CLK			(1<<23)
#define CLK_UART2			(1<<24)

#define CLK_ENABLE(x)	      \
				do{ unsigned long flags;  flags = system_irq_save();((*((volatile unsigned int *)CLK_EN_BASE)) = (unsigned int)((*((volatile unsigned int *)CLK_EN_BASE))|(x))); system_irq_restore(flags); }while(0)
#define CLK_DISABLE(x)	      \
				do{ unsigned long flags;  flags = system_irq_save();((*((volatile unsigned int *)CLK_EN_BASE)) = (unsigned int)((*((volatile unsigned int *)CLK_EN_BASE))&(~(x)))); system_irq_restore(flags); }while(0)

// mode Base
#define	SOC_PMU_REG_BASE			0x00601000
#define SOC_PCU_REG_BASE			0x00601040
#define SOC_CLK_MUX_REG_BASE		0x00601804
#define SOC_MOD_CLK_EN_REG_BASE		0x0060180C
#if 0
#define SOC_WAKEUP_ADD_BASE			0x00601204 /*default:0  
												0:wakeup from 0x0 address
											    1:wakeup from 64KB address*/
#endif

//PMU
#define SOC_PMU_CPU_PWR_CTRL			SOC_PMU_REG_BASE+0x10

//PCU
#define SOC_PCU_LIGHT_SLEEP_CTRL		SOC_PCU_REG_BASE
#define SOC_PCU_DEEP_SLEEP_CTRL			SOC_PCU_REG_BASE+0x4
#define SOC_PCU_CTRL11					SOC_PCU_REG_BASE+0x70

//IO_MUX
#define SOC_PIN0_MUX_BASE				0x00601824
#define SOC_PIN1_MUX_BASE				0x00601828

//AON_SMU
#define SOC_AON_SMU_BASE				0x00601200
#define SOC_AON_CHIP_ID					SOC_AON_SMU_BASE
#define SOC_AON_WAKEUP_ADD				SOC_AON_SMU_BASE+0x4/*default:0  
													  			0:wakeup from 0x0 address
											          			1:wakeup from 64KB address*/
#define SOC_AON_32K_SEL					SOC_AON_SMU_BASE+0x8/*0:analog 1:pad*/
#define SOC_AON_LIGHT_CONF				SOC_AON_SMU_BASE+0xC
#define SOC_AON_ILM_CACHE_SWITCH		SOC_AON_SMU_BASE+0x10
#define SOC_AON_TEST_MOD				SOC_AON_SMU_BASE+0x14
#define SOC_AON_GPIO13_MUX				SOC_AON_SMU_BASE+0x18
#define SOC_AON_GPIO16_MUX				SOC_AON_SMU_BASE+0x1C
#define SOC_AON_GPIO17_MUX				SOC_AON_SMU_BASE+0x20
#define SOC_AON_GPIO_MOD_SWITCH			SOC_AON_SMU_BASE+0x24/*switch GPIO between PD and AO
															   0:PD area
															   1:AO area*/
//periphs
#define SOC_APB_BASE				    0x00600000
#define SOC_APB1_BASE				    0x00601000
#define SOC_UART0_BASE					0x00602000
#define SOC_UART1_BASE					0x00603000
#define SOC_UART2_BASE					0x0060C000
#define SOC_PIT0_BASE				    0x00604000
#define SOC_PIT1_BASE				    0x00605000
#define SOC_WDT_BASE				    0x00606000
#define SOC_GPIO_BASE				    0x00607000
#define SOC_I2C_BASE				    0x00608000
#define SOC_SPI1_BASE				    0x00609000
//#define SOC_SPI_FLASH_BASE				0x0060A000
#define SOC_APB11_BASE				    0x0060B000
#define SOC_PROCESS_SENSOR_BASE			0x0060B000
#define SOC_TRNG_BASE					0x0060B040
#define SOC_EFUSE_BASE					0x0060B200
#define SOC_AES_BASE					0x0060B300
#define SOC_ANALOG_BASE					0x00601C00
#define SPI2_BASE						0x0060A000 /* Device base address */



/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:  system.h  
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2018-11-30
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef _SYSTEM_H
#define _SYSTEM_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/
#include "system_common.h"
#include "system_os_api.h"


/****************************************************************************
* 	                                        Macros
****************************************************************************/
//#define _USE_MPW   1

#define SYSTEM_CLI_TASK_STACK_SIZE  (1024 / sizeof(StackType_t))
#define SYSTEM_CLI_TASK_PRIORITY	( configMAX_PRIORITIES - 10 )

#define SYSTEM_LMAC_TASK_STACK_SIZE (2048/ sizeof(StackType_t))
#define SYSTEM_LMAC_TASK_PRIORITY ( configMAX_PRIORITIES - 1)

#define SYSTEM_EVENT_LOOP_PRIORITY   (configMAX_PRIORITIES - 9)


//#define	LIGHT_TEST
/*-----------------------Chip Reg Base-------------------------------*/
// mode Base
#if 0
#define	SOC_PMU_REG_BASE			0x00601000
#define SOC_PCU_REG_BASE			0x00601040
#define SOC_SENSOR_REG_BASE			0x0060b000
#define SOC_TRNG_REG_BASE			0x0060b011
#define SOC_EFUSE_REG_BASE			0x0060b200
#define SOC_CLK_MUX_REG_BASE		0x00601804
#define SOC_MOD_CLK_EN_REG_BASE		0x0060180C
#define SOC_WAKEUP_ADD_BASE			0x00601204 /*default:0  
												0:wakeup from 0x0 address
											    1:wakeup from 64KB address*/
#endif

//PMU
#define SOC_PMU_CPU_PWR_CTRL			SOC_PMU_REG_BASE+0x10
#define	SOC_PMU_CPU_PARA_BAKUP_ADD		0x240000 /*defult:024000*/

//PCU
#define SOC_PCU_LIGHT_SLEEP_CTRL		SOC_PCU_REG_BASE
#define SOC_PCU_DEEP_SLEEP_CTRL			SOC_PCU_REG_BASE+0x4
#define SOC_PCU_CTRL11					SOC_PCU_REG_BASE+0x70
#define SOC_PCU_CTRL10					SOC_PMU_REG_BASE+0x60

//RF register fullmask
#define RF_CTRL_TO_DIG_ISO				SOC_ANALOG_BASE+0x10C

/*-----------------------Chip Reg Base-------------------------------*/

#define NRC_WPA_NUM_INTERFACES			(2)
#define NRC_WPA_INTERFACE_NAME_0		("wlan0")
#define NRC_WPA_INTERFACE_NAME_1		("wlan1")

//PSM OPTION
#define TR_PSM_OPTION_RF_INIT   0
#define TR_PSM_OPTION_RF_NOT_INIT   1

/****************************************************************************
* 	                                        Types
****************************************************************************/
typedef enum
{
		/* 0 ~ -29: commom */
		DRV_SUCCESS 									= 0,		/* successed */
		DRV_ERROR										 = -1,		/* failed */
		DRV_ERR_INVALID_IOCTL_CMD = -2, 	 /* no this control command branch */
		DRV_ERR_NOT_SUPPORTED		   = -3,	  /* this function hasn't been supported */
		DRV_ERR_INVALID_PARAM		   = -4,	  /* the input parameter is invalid */
		DRV_ERR_MEM_ALLOC				   = -5,	  /* failed to malloc memory */
		DRV_ERR_HISR_CREATE_FAIL	 = -6,		/* failed to create hisr */
		DRV_ERR_TIMEOUT 						= -7,	   /* timeout for a block waitting operation */
		DRV_ERR_BUSY								 = -8,		/* busy now to do the request operation */
		DRV_ERR_NOT_OPENED				   = -9,	  /* the device to operate hasn't been opened yet */
		DRV_ERR_OPEN_TIMES					= -10,	  /* try to open a device which has been opened already */
		DRV_ERR_NOT_STARTED 			  = -11,	/* the device to operate hasn't been started yet */
		DRV_ERR_START_TIMES 			   = -12,	 /* try to open a device which has been opened already */
		/* reserved */
	
		/* -30 ~ -39: for dal */
		DRV_ERR_DEV_OVERFLOW	 = -30, 	  /* no free entry to install this device. please change ZDRV_MAX_DEV_NUM in dal_api.h */
		DRV_ERR_DEV_TABLE				= -31,		 /* the device table has been destroyed */
		DRV_ERR_FD_OVERFLOW 	   = -32,		/* no free entry to open this device. pleas change ZDRV_MAX_DEV_FILE_NUM in dal_api.h */
		DRV_ERR_FD_TABLE				  = -33,	  /* the file descriptor table has been destroyed */
		DRV_ERR_INSTALLED_TIMES = -34,		/* try to install a device which hasn been installed yet */
		DRV_ERR_NO_THIS_DEVICE	  = -35,	  /* try to open a device which hasn't been installed yet */
		/* reserved */
		
		/*-40 ~ -59: for sio */
		DRV_ERR_NO_CHANNEL					 = -40, 		/*the used sio no channel*/
		DRV_ERR_CHAN_CREATE_FAIL	  = -41,		   /*the  sio creat channel fail*/
		DRV_ERR_DEV_STATE_WRONG 	 = -42, 		 /*the	sio state error*/
		DRV_ERR_CHAN_DELETE_FAIL	  = -43,		  /*the  sio delete channel fail*/
		DRV_ERR_DEV_READ						 = -44, 	 /*the	sio read data error*/
		DRV_ERR_CHAN_SEM_USED			 = -45, 		/*the  sio semp has been used r*/
		DRV_ERR_CHAN_DELETED			   = -46,		/*the  sio channel has been deleted */
		DRV_ERR_DEV_CLOSED					  = -47,		/*the  sio has been closed */
		DRV_ERR_DEV_OPT_NULL				= -48,		 /*the	sio device ptr is null*/
		DRV_ERR_INSTALL_DRIVER_FAIL = -49,			/*the  sio install	faill*/
		DRV_ERR_BUFFER_NOT_ENOUGH = -50,			/*the  sio data buffer not enough*/
		/* reserved */
	
		
		/* -60 ~ 69: for mux */
		DRV_ERR_MUX_INVALID_DLCI		  = -60,	  /* the dlci is invalid */
		DRV_ERR_MUX_BUSY							 = -61, 	 /* busy now, so the required operation has been rejected */
		DRV_ERR_MUX_NOT_READY				= -62,		/* the mux or dlci is not ready to do this required operation */
		DRV_ERR_MUX_FLOW_CONTROLED = -63,	  /* this dlc is flow-controled by the opposite station, so can't sent data any more */
		DRV_ERR_MUX_PN_REJECTED 		   = -64,	  /* the parameter for this dlc establishment is rejected by the opposite station */
		DRV_ERR_MUX_BUF_IS_FULL 			 = -65, 	 /* the data buffer of this dlc is full, so can't write any more */
		DRV_ERR_MUX_BUF_IS_EMPTY		  = -66,	  /* the data buffer of this dlc is empty, so no data to transfer */
		DRV_ERR_MUX_FRAME_INVALID	   = -67,	   /* the frame data is invalid */
		DRV_ERR_MUX_FRAME_UNCOMPLETE	= -68,		/* the frame data is uncomplete */
	
		DRV_ERROR_EMPTY = -90,
		DRV_ERROR_FULL = -91,
		DRV_ERROR_NODEV = -92,
		DRV_ERROR_SUSPEND = -93,
		DRV_ERROR_AGAIN = -94,
		DRV_ERROR_ABORT = -95,
		DRV_ERROR_NOCONNECT = -96,
	 
		/*-100~-104 for spi*/
		DRV_ERR_NOCOMPLETE				= -100,
	
		/*-105~-109 for gpio*/
		DRV_ERR_NOT_WRITE				= -105,
	
		/*-110~-119 for pmic */ 	 
		DRV_ERR_CLIENT_NBOVERFLOW		= -110, /*!< The requested operation could not becompleted because there are too many PMIC client requests */
		DRV_ERR_SPI_READ				= -111, 		
		DRV_ERR_SPI_WRITE				= -112,    
		DRV_ERR_EVENT_NOT_SUBSCRIBED	= -113, /*!< Event occur and not subscribed 	  */
		DRV_ERR_EVENT_CALL_BACK 		= -114, /*!< Error - bad call back				  */   
		DRV_ERR_UNSUBSCRIBE 			= -115, /*!< Error in un-subscribe event		  */
	
		/*-120~-129 for sd */ 
		DRV_ERR_INTR_TIMEOUT			= -120,
		DRV_ERR_INTR_ERROR				= -121,
		DRV_ERR_CARDSTATE_ERROR 		= -122,
		DRV_ERR_CARD_DISCONNECT 		= -123,
		DRV_ERR_WRITE_PROTECT			= -124,
		DRV_ERR_PWD_ERR 				= -125,
		DRV_ERR_LOCKCARD_ERR			= -126,
		DRV_ERR_FORCEERASE_ERR			= -127,
		DRV_ERR_RESPONSE_ERR			= -128,
		DRV_ERR_HLE_ERROR			   = -129,
		DRV_ERR_EIO 					= -130, /*IO Error*/
		DRV_ERR_ERANGE					= -131, /* Math result not representable */
		DRV_ERR_EINPROGRESS 			= -132, /* Operation now in progress */
		DRV_ERR_ENODEV					= -133, /*no such device*/
		DRV_ERR_BADMSG					= -134, /*not a date message*/
		DRV_ERR_ENOENT					= -135, /* No such file or directory */ 
		DRV_ERR_ILSEQ					= -136, /* Illegal byte sequence */
	
		/*-137~-140 for i2c */ 
		DRV_ERR_ADDR_TRANSFER			= -137,
		DRV_ERR_DATA_TRANSFER			= -138,
		DRV_ERR_AGAIN					= -139,
		DRV_ERR_NOACK					= -140,
	
	
	
		/*-141~-156 for usb */ 
		DRV_ERR_NOT_READY					= -141,
		DRV_ERR_STATUS_BUSY 			= -142,
		DRV_ERR_STALL						= -143,
		DRV_ERR_END 							= -144,
		DRV_ERR_USB_BUF_STATE		= -145,
		DRV_ERR_USB_BUF_FULL			= -146,
		DRV_ERR_USB_BUF_EMPTY		  = -147,
		DRV_ERR_USB_QMI_UNDERRUN  = -148,
		DRV_ERR_USB_QMI_OVERRUN 	= -149,
		DRV_ERR_USB_QMI_READ0		  = -150,
		DRV_ERR_USB_UNCONNECTED    = -151,
		DRV_ERR_QMI_HEADER_ERR		 = -152,
		DRV_ERR_QMI_CTL_ERR 	  = -153,
		DRV_ERR_QMI_WDS_ERR 	  = -154,
		DRV_ERR_QMI_DMS_ERR 	  = -155,
		DRV_ERR_QMI_NAS_ERR 	  = -156,
	
		/*-160~-180 for nand */
		DRV_ERR_LEN_ADDRESS 			= -160, 	/* 2�����¦�??���䨪?�� */
		DRV_ERR_COMMAND 				= -161, 	/* ?����???�䨪?�� */
		DRV_ERR_PE_ERROR				= -162, 	/* ��?D��䨪?�� */
		DRV_ERR_NAND_BUSY				= -163, 	/* D????| */
		DRV_ERR_PROTECTED				= -164, 	/* ����?��??���� */
		DRV_ERR_BANK_ERROR				= -165, 	/* BANK�䨪?�� */
		DRV_ERR_UNKNOWN 				= -166, 	/* UNKNOWN�䨪?�� */
		DRV_ERR_LENGTH					= -167,
	
		/*-150~-154 for backlight */
		DRV_ERR_INTERNAL_PERIOD 		= -180,
		DRV_ERR_INTERNAL_FREQ			= -181,

		DRV_ERR_MAX		
}TR_DRVS_RETURN_TYPE;



typedef enum
{
		PSM_DISABLE,
		PSM_ENABLE,
	
		PSM_MODE_MAX
		
}TR_PSM_MODE;

typedef enum
{
		RTC_CAL_DIS,
		RTC_CAL_EN,
	
		RTC_CAL_MAX
		
}TR_PSM_RTC_CAL;
	
	
typedef enum
{
		SYS_AMT_MODE,
		SYS_NORMAL_MODE,
	
		SYS_MODE_MAX
		
}TR_AMT_MODE;
	
typedef enum
{
		SYS_WIFI_IDLE,
		SYS_WIFI_ACTIVE,
	
		SYS_WIFI_MAX
		
}TR_WIFI_WORK_STATUS;

typedef enum
{
		SYS_WIFI_STA,
		SYS_WIFI_AP,
	
		SYS_WIFI_MODE
		
}TR_WIFI_STATUS;

typedef enum
{
		SYS_BEACON_LOST,
		SYS_BEACON_ALIVE,
		SYS_BEACON_ALIVE_NOT_SLEEP,
	
		SYS_BEACON_MAX
		
}TR_WIFI_BEACON_STATUS;
	
typedef enum
{
		SOC_SLEEP_IDLE,
		SOC_SLEEP_LIGHT,
		SOC_SLEEP_DEEP1,
		SOC_SLEEP_DEEP2,
	
		SOC_SLEEP_MAX
		
}TR_SOC_SLEEP_MODE;
	
typedef enum
{
		PSM_DEEP_SLEEP_DISABLE,
		PSM_DEEP_SLEEP_ENABLE,
	
		PSM_DEEP_SLEEP_MAX
		
}TR_PSM_DEEP_SLEEP_MODE;
	
typedef enum
{
		PSM_WAKEUP_RTC,
		PSM_WAKEUP_KEY,
		PSM_WAKEUP_POWER,
		PSM_WAKEUP_MAX
}TR_PSM_WAKEUP_WAY;
	
	
typedef enum
{
		PSM_DEVICE_GPIO,
		PSM_DEVICE_I2C,
		PSM_DEVICE_SPI,
		PSM_DEVICE_ADC,
		PSM_DEVICE_SDIO,
		PSM_DEVICE_DMA,
		PSM_DEVICE_RTC,
		PSM_DEVICE_I2S,
		PSM_DEVICE_CODEC,
		PSM_DEVICE_WATCHDOG,
		PSM_DEVICE_WIFI_STA,
		PSM_DEVICE_WIFI_AP,
		PSM_DEVICE_UART1,
		PSM_DEVICE_UART2,
		PSM_DEVICE_PWM,
		
		PSM_MAX_DEVICE
}TR_PSM_DEVICE;
	
	
typedef enum 
{
		SYS_PM_STATE_IDLE,
		SYS_PM_STATE_MODEM_SLEEP,
		SYS_PM_STATE_LIGHT_SLEEP,	
		SYS_PM_STATE_DEEP_SLEEP,
	
		SYS_PM_STATE_MAX
}TR_SYS_PSM_STATE;
	

typedef  struct  _T_RF_REG{
	int rf_1c;
	int rf_28;
	int rf_50;
	int rf_5c;
	int rf_60;
	int rf_a8;
	int rf_ac;
	int rf_f0;
}T_RF_REG;

typedef enum
{
	SLEEP_MODE_LIGHT,
	SLEEP_MODE_MODEM,

	SLEEP_MODE_MAX
}SLEEP_MODE_STATE;

typedef struct	
{
		TR_SYS_PSM_STATE				SysPsmState;
		TR_PSM_DEEP_SLEEP_MODE			PsmDeepSleepEnableFlag;
		int32_t							expectSleepTime;//DTIM,Beacon-->ms
		uint32_t							startSleepTime;//tick cnt?
		uint32_t							deepSleep1Period;// using by user to set deep sleep period
		uint32_t							amtMode;// AMT mode or Normal mode
		uint32_t							deviceStatus;
		uint32_t							irqsStatus;
   		uint32_t   						waitBeacon;
		int 								psmFlag;
		SLEEP_MODE_STATE                    sleepmode;

		uint32_t    						nextBeaconTime;
		uint32_t    						lastAlarmSet;
		uint32_t    						beaconStatus;
		int		 						sleepWithBeaconCnt;
		int		 						sleepWithoutBeaconCnt;
}TR_PSM_CONTEXT;

typedef struct
{
	uint32_t	ch;
	uint32_t   rf_ctrim;
	uint32_t   rf_rx;
	uint32_t	 rf_adc_30;
	uint32_t   rf_adc_34;
	uint32_t   rf_adc_spartbits;
	uint32_t   rf_dcoc;
	uint32_t   rf_ctunValue;
	uint32_t	rf_adda_ldo;
	
}TR_PSM_WIFI;


typedef enum 
{
	DRV_MOD_CLK_DISABLE,
	DRV_MOD_CLK_ENABLE,
	DRV_MOD_CLK_ALL_DISABLE,
	DRV_MOD_CLK_ALL_ENABLE,

	DRV_MOD_CLK_MAX
}TR_DRV_MOD_CLK_CTRL;


/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/
extern SemaphoreHandle_t xPsmTaskSem;
extern SemaphoreHandle_t xLmacPsmTaskSem;
extern TR_PSM_CONTEXT psmCtx;
extern int32_t  timCnt;

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/
//function 
void TrPsmInit(void);
int32_t TrPsmPcuInit(void);
void  TrPsmTestSocSleep(uint32_t socsleepmode);
void system_default_setting(int vif_id); 
uint32_t TrPsmModClkCtrl(uint32_t mode_clk,uint32_t clk_ctrl);
int32_t	TrPsmSetDeviceIdle(uint32_t      device);
int32_t  TrPsmSetDeviceActive(uint32_t  device);
void TrPsmWifistore(void);
void TrPsmDeepSleep(void);
void   TrPsmModemSleepRestore();
uint32_t TrPsmSetSysDeepSleep(uint32_t deepsleep, uint32_t  sleepperiod);
void TrPsmIrqMask();
void TrPsmPitStop(TR_PSM_CONTEXT *ctx);
void TrPsmEnterSocSleep(uint32_t socsleepmode);
#endif


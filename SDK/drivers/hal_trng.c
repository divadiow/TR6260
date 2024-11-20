/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:  hal_trng.c  
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2019-1-3
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

/****************************************************************************
* 	                                           Include files
****************************************************************************/
#include "drv_trng.h"


/****************************************************************************
* 	                                           Local Macros
****************************************************************************/

#define TRNG_CLOCK_GATING_EN			SOC_TRNG_BASE
#define	TRNG_TRNG_VAL					SOC_TRNG_BASE+0x4
#define	TRNG_PRNG_VAL					SOC_TRNG_BASE+0x8
#define	TRNG_TRNG_PROCESS_VAL			SOC_TRNG_BASE+0xC

#define	PROCESS_SENSOR_START			SOC_PROCESS_SENSOR_BASE
#define	PROCESS_SENSOR_DELAY_CNT		SOC_PROCESS_SENSOR_BASE+0x4
#define	PROCESS_SENSOR_CNTR0			SOC_PROCESS_SENSOR_BASE+0x8
#define	PROCESS_SENSOR_CNTR1			SOC_PROCESS_SENSOR_BASE+0xC
#define	PROCESS_SENSOR_CNTR2			SOC_PROCESS_SENSOR_BASE+0x10
#define	PROCESS_SENSOR_CNTR3			SOC_PROCESS_SENSOR_BASE+0x14
#define PROCESS_SENSOR_DELAY_CNT_MAX	0x7FF // 0~10 bit


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

static void delay(volatile unsigned int data)
{
	volatile unsigned int indx;

	for (indx = 0; indx < data; indx++) {

	};
}


/*******************************************************************************
 * Function: trng_read
 * Description: 
 * Parameters: 
 *   Input:
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
uint32_t trng_read(void)
{
	uint32_t tmpdata = 0;
	
    //1.enable clk
	CLK_ENABLE(CLK_TRNG);
	OUT32(TRNG_CLOCK_GATING_EN, 0x1);//enable gating clk

	//2.read data once,and abandon
	IN32(TRNG_TRNG_VAL);

	//3.read data
	tmpdata = IN32(TRNG_TRNG_VAL);

	usdelay(2);
	//4.disabel clk
	OUT32(TRNG_CLOCK_GATING_EN, 0x0);//enable gating clk
	CLK_DISABLE(CLK_TRNG);

	return tmpdata;
}

/*******************************************************************************
 * Function: hal_prng_read
 * Description: 
 * Parameters: 
 *   Input:
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
uint32_t hal_prng_read(void)
{
	uint32_t tmpdata = 0;
	
    //1.enable clk
	CLK_ENABLE(CLK_TRNG);
	OUT32(TRNG_CLOCK_GATING_EN, 0x1);//enable gating clk

	//2.read data once,and abandon
	IN32(TRNG_PRNG_VAL);

	//3.read data
	tmpdata = IN32(TRNG_PRNG_VAL);

	//4.disabel clk
	CLK_DISABLE(CLK_TRNG);
	OUT32(TRNG_CLOCK_GATING_EN, 0x0);//enable gating clk

	return tmpdata;
}


/*******************************************************************************
 * Function: 
 * Description: 
 * Parameters: 
 *   Input:
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
uint32_t hal_trng_process_read(void)
{
	uint32_t tmpdata = 0;
	
    //1.enable clk
	CLK_ENABLE(CLK_TRNG);
	OUT32(TRNG_CLOCK_GATING_EN, 0x1);//enable gating clk

	//2.read data once,and abandon
	IN32(TRNG_TRNG_PROCESS_VAL);

	//3.read data
	tmpdata = IN32(TRNG_TRNG_PROCESS_VAL);

	//4.disabel clk
	CLK_DISABLE(CLK_TRNG);
	OUT32(TRNG_CLOCK_GATING_EN, 0x0);//enable gating clk

	return tmpdata;

}

/*******************************************************************************
 * Function: 
 * Description: 
 * Parameters: 
 *   Input:
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
 uint32_t hal_process_sensor_read(uint32_t DelayTimeUs,uint32_t *pProcessCnt)
{
	uint32_t tmpProcessCnt[4];

	memset(tmpProcessCnt,0x0,4*sizeof(uint32_t));

	
	if(DelayTimeUs > PROCESS_SENSOR_DELAY_CNT_MAX || pProcessCnt == NULL)
	{
		return DRV_ERR_INVALID_PARAM;
	}

	
	//1.enable clk
	CLK_ENABLE(CLK_TRNG);
	OUT32(TRNG_CLOCK_GATING_EN, 0x1);//enable gating clk
	
    //2.set delaytimer
	if(DelayTimeUs != 0)
	{
		OUT32(PROCESS_SENSOR_DELAY_CNT, DelayTimeUs);
	}

	//3.start process sensor
	#if 0
	while(IN32(PROCESS_SENSOR_START) != 0)
	{

	}
	#endif
	OUT32(PROCESS_SENSOR_START, 0x1);

	//4.delay
	delay(PROCESS_SENSOR_DELAY_CNT_MAX);

	//5.read data
	tmpProcessCnt[0] = IN32(PROCESS_SENSOR_CNTR0);
	tmpProcessCnt[1] = IN32(PROCESS_SENSOR_CNTR1);
	tmpProcessCnt[2] = IN32(PROCESS_SENSOR_CNTR2);
	tmpProcessCnt[3] = IN32(PROCESS_SENSOR_CNTR3);
	
	//6.cpy data
	memcpy(pProcessCnt,tmpProcessCnt,4*sizeof(uint32_t));

	return DRV_SUCCESS;

}

//----------------------------------------------------------------------------------------
#if (!defined AMT) && (!defined SINGLE_BOARD_VER)
#ifdef _USE_TEST_CMD_TRNG
static int cmd_read_trng(cmd_tbl_t *t, int argc, char *argv[])
{
	uint32_t  randNum; 
	randNum = trng_read();
	system_printf("random digit:%x\n", randNum);

	return CMD_RET_SUCCESS;
}

SUBCMD(show,
    trng,
    cmd_read_trng,
    "Enable Cpu Power Save Mode",
    "read read_trng");

static int cmd_read_process(cmd_tbl_t *t, int argc, char *argv[])
{
		uint32_t  randNum[4]; 
		uint32_t  index = 0;
		uint32_t  delay_cnt;

		memset(randNum,0xaa,4);

		if (argc != 2)
		return CMD_RET_USAGE;
		
		delay_cnt = atoi(argv[1]);
		
		
		hal_process_sensor_read(delay_cnt,randNum);
		for(index = 0;index<4;index++)
		{
			system_printf("random digit[%d]:0x%x\n", index,randNum[index]);
		}
	
		return CMD_RET_SUCCESS;
}
	
SUBCMD(show,
		process,
		cmd_read_process,
		"Enable Cpu Power Save Mode",
		"read read_trng");
#endif
#endif



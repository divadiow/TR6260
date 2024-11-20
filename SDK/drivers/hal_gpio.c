/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: hal_gpio.c   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2018-12-20
 * History 1:      
 *     Date: 2018-12-25
 *     Version:
 *     Author: yanhaijian
 *     Modification:  add gpio isr function
 * History 2: 
 *     Date: 2019-1-21
 *     Version:
 *     Author: wangxia
 *     Modification:  modify funciton according debug
 * History 3: 
 *     Date: 2019-3-4
 *     Version:
 *     Author: liuyafeng
 *     Modification:  modify gpio isr callback function & add gpio isr test code
  ********************************************************************************/

/****************************************************************************
* 	                                           Include files
****************************************************************************/
#include "soc_top_reg.h"
#include "drv_gpio.h"
#include "soc_pin_mux.h"


/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define GPIO_MOD_ID	   					SOC_GPIO_BASE
#define GPIO_CONF						SOC_GPIO_BASE+0x10
#define GPIO_CHNNL_DATA_IN	  			SOC_GPIO_BASE+0x20
#define GPIO_CHNNL_DATA_OUT  			SOC_GPIO_BASE+0x24
#define	GPIO_CHNNL_DIR					SOC_GPIO_BASE+0x28
#define GPIO_CHNNL_DATA_OUT_CLEAR		SOC_GPIO_BASE+0x2C
#define GPIO_CHNNL_DATA_OUT_SET			SOC_GPIO_BASE+0x30
#define GPIO_PULL_EN					SOC_GPIO_BASE+0x40
#define GPIO_PULL_TYPE					SOC_GPIO_BASE+0x44
#define GPIO_INT_EN						SOC_GPIO_BASE+0x50
#define GPIO_INT_MODE_7_0				SOC_GPIO_BASE+0x54
#define GPIO_INT_MODE_15_8				SOC_GPIO_BASE+0x58
#define GPIO_INT_MODE_23_16				SOC_GPIO_BASE+0x5C
#define GPIO_INT_MODE_31_24				SOC_GPIO_BASE+0x60
#define GPIO_INT_STATUS					SOC_GPIO_BASE+0x64
#define GPIO_DEBOUNCE_EN				SOC_GPIO_BASE+0x70
#define GPIO_DEBOUNCE_CTRL				SOC_GPIO_BASE+0x74

#define GPIO_CHNx_INT_BASE(_chn_)		(SOC_GPIO_BASE + 0x54 + ( (_chn_/8)* 0x4))
#define GPIO_INT_ENABLE					0x1

/****************************************************************************
* 	                                           Local Types
****************************************************************************/
typedef union 
{
	uint32_t gpio_int_mode;
	struct
	{
		uint32_t interrupt_mode_chnnl_0:3;
		uint32_t reserver3:1;
		uint32_t interrupt_mode_chnnl_1:3;
		uint32_t reserver7:1;
		uint32_t interrupt_mode_chnnl_2:3;
		uint32_t reserver11:1;
		uint32_t interrupt_mode_chnnl_3:3;
		uint32_t reserver15:1;
		uint32_t interrupt_mode_chnnl_4:3;
		uint32_t reserver19:1;
		uint32_t interrupt_mode_chnnl_5:3;
		uint32_t reserver23:1;
		uint32_t interrupt_mode_chnnl_6:3;
		uint32_t reserver27:1;
		uint32_t interrupt_mode_chnnl_7:3;
		uint32_t reserver31:1;
	};
}DRV_GPIO_INT_MODE;

/****************************************************************************
* 	                                           Local Constants
****************************************************************************/

/****************************************************************************
* 	                                           Local Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Global Constants
****************************************************************************/
DRV_GPIO_CALLBACK   gpio_callback[DRV_GPIO_MAX];
/****************************************************************************
* 	                                          Global Variables
****************************************************************************/

/****************************************************************************
* 	                                          Global Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/
#define PIN_MUX0 0x601824
#define PIN_MUX1 0x601828

void gpio_pin_set(uint32_t pin_num)
{
	if (pin_num>=DRV_GPIO_0 && pin_num<=DRV_GPIO_9)
	{
		OUT32(PIN_MUX0, (IN32(PIN_MUX0) & (~(7<<(pin_num*3)))) |  (1<<(pin_num*3)));
	}
	else if(pin_num>=DRV_GPIO_10 && pin_num<=DRV_GPIO_15)
	{
		OUT32(PIN_MUX1, (IN32(PIN_MUX1) & (~(7<<((pin_num-10)*3)))) |  (1<<((pin_num-10)*3)));
	}
	else if(pin_num == DRV_GPIO_16 || pin_num == DRV_GPIO_17 || pin_num>=DRV_GPIO_MAX)
	{
		system_printf("pin_num %d Unvaild GPIO Port\n",pin_num);
		return;
	}
	else if(pin_num == DRV_GPIO_18 || pin_num == DRV_GPIO_19)
	{
		OUT32(PIN_MUX1, (IN32(PIN_MUX1) & (~(1<<pin_num))));
	}
	else if(pin_num >= DRV_GPIO_20 && pin_num <= DRV_GPIO_23)
	{
		OUT32(PIN_MUX1, (IN32(PIN_MUX1) & (~(7<<(20 + (pin_num-20)*3)))));
	}
	else if(pin_num == DRV_GPIO_24)
	{

		OUT32(PIN_MUX0, (IN32(PIN_MUX0) & (~(3<<30))));
	}
}

/*******************************************************************************
 * Function: gpio_config
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
int32_t gpio_config(DRV_GPIO_CONFIG *pGpioConfig)
{
	//uint32_t gpio_pin_tmp = pGpioConfig->GPIO_Pin;
	DRV_GPIO_CONFIG *pGpiocfgTemp = pGpioConfig;
	uint32_t RegTmp = 0;
	uint32_t flag = 0;
	
	if((pGpioConfig == NULL)||(pGpiocfgTemp->GPIO_Pin >= DRV_GPIO_MAX) \
		||(pGpiocfgTemp->GPIO_Pin == DRV_GPIO_16)\
		||(pGpiocfgTemp->GPIO_Pin == DRV_GPIO_17)\
		||(pGpiocfgTemp->GPIO_PullEn >= DRV_GPIO_PULL_MAX))
	{
		return DRV_ERR_INVALID_PARAM;
	}
   /*lock irq*/
   flag = system_irq_save();
      //---------1.set pull en------------
	  RegTmp = IN32(GPIO_PULL_EN);
	  if(pGpiocfgTemp->GPIO_PullEn == DRV_GPIO_PULL_DIS) //set corresponding bit to 0
	  {
		 RegTmp &=~(1<<(pGpiocfgTemp->GPIO_Pin));
		 OUT32(GPIO_PULL_EN,RegTmp);
	  }
	  else if(pGpiocfgTemp->GPIO_PullEn == DRV_GPIO_PULL_EN)
	  {
		RegTmp |=1<<(pGpiocfgTemp->GPIO_Pin);
		OUT32(GPIO_PULL_EN,RegTmp);

		//--------2.set pull type----------
	 	RegTmp = IN32(GPIO_PULL_TYPE);
		if(pGpiocfgTemp->GPIO_PullType == DRV_GPIO_PULL_TYPE_UP) //set corresponding bit to 0
		{
		 	RegTmp &=~(1<<(pGpiocfgTemp->GPIO_Pin));
		 	OUT32(GPIO_PULL_TYPE,RegTmp);
	  	}
		else if(pGpiocfgTemp->GPIO_PullType == DRV_GPIO_PULL_TYPE_DOWN)//set corresponding bit to 1
		{
			RegTmp |=1<<(pGpiocfgTemp->GPIO_Pin);
			OUT32(GPIO_PULL_TYPE,RegTmp);
		}
		else
		{
			//other status
		}
	  }
	  else
	 {
		//other status
	 }

	 #if 0
	 //-------------3.set dir----------
	 RegTmp = IN32(GPIO_CHNNL_DIR);
	 if(pGpiocfgTemp->GPIO_Dir == DRV_GPIO_DIR_INPUT) //set corresponding bit to 0
	 {
		RegTmp &=~(1<<(pGpiocfgTemp->GPIO_Pin));
		OUT32(GPIO_PULL_EN,RegTmp);
	 }
	 else if(pGpiocfgTemp->GPIO_Dir == DRV_GPIO_DIR_OUTPUT)//set corresponding bit to 1
	 {
		RegTmp |=1<<(pGpiocfgTemp->GPIO_Pin);
		OUT32(GPIO_PULL_EN,RegTmp);
	 }
	 else
	 {
		//other status
	 }
	 #endif

    //unlock irq 
	  system_irq_restore(flag);
	 
	 return DRV_SUCCESS;
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
int32_t gpio_set_dir(uint32_t pin_num,uint32_t pin_dir)
{
  
 	uint32_t RegTmp = 0;
	uint32_t flag = 0;
 
 	if((pin_dir >= DRV_GPIO_DIR_MAX)||(pin_num >= DRV_GPIO_MAX) \
	 ||(pin_num == DRV_GPIO_16)\
	 ||(pin_num == DRV_GPIO_17))
 	{
	 	return DRV_ERR_INVALID_PARAM;
 	}
	 
    /*lock irq*/
    flag = system_irq_save();

 	RegTmp = IN32(GPIO_CHNNL_DIR);
	if(pin_dir == DRV_GPIO_DIR_INPUT) //set corresponding bit to 0
 	{
		RegTmp &=~(1<<(pin_num));
		OUT32(GPIO_CHNNL_DIR,RegTmp);
 	}
 	else if(pin_dir == DRV_GPIO_DIR_OUTPUT)//set corresponding bit to 1
 	{
		RegTmp |=1<<(pin_num);
		OUT32(GPIO_CHNNL_DIR,RegTmp);
 	}
 	else
 	{
		//other status
 	}

	//unlock irq 
  	system_irq_restore(flag);
	return DRV_SUCCESS;
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
int32_t gpio_get_dir(uint32_t pin_num,uint32_t *pin_dir)
{
  
 	uint32_t RegTmp = 0;
	uint32_t flag = 0;
	uint32_t PinDir = 0;
 
 	if((pin_num >= DRV_GPIO_MAX) \
	 ||(pin_num == DRV_GPIO_16)\
	 ||(pin_num == DRV_GPIO_17))
 	{
	 	return DRV_ERR_INVALID_PARAM;
 	}
	 
   /*lock irq*/
	flag = system_irq_save();
	RegTmp = IN32(GPIO_CHNNL_DIR);
	
	if((RegTmp >> pin_num) & 0x1)
		PinDir = DRV_GPIO_DIR_OUTPUT;
	else
		PinDir = DRV_GPIO_DIR_INPUT;

	memcpy(pin_dir,&PinDir,sizeof(uint32_t));

	//unlock irq 
  	system_irq_restore(flag);
	
	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function: gpio_write
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
int32_t gpio_read_level(uint32_t pin_num)
{
   	uint32_t regtmp = 0;
	if((pin_num >= DRV_GPIO_MAX) \
		||(pin_num == DRV_GPIO_16)\
		||(pin_num == DRV_GPIO_17))
	{
		return DRV_ERR_INVALID_PARAM;
	}
	regtmp = IN32(GPIO_CHNNL_DATA_OUT);
    if (regtmp & (1 << pin_num)) {
        return 1;
    } else {
        return 0;
    }
}



/*******************************************************************************
 * Function: gpio_write
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
int32_t gpio_write(uint32_t pin_num,uint32_t pin_value)
{
   	uint32_t RegTmp = 0;
	uint32_t flag = 0;
	
	if((pin_num >= DRV_GPIO_MAX) \
		||(pin_num == DRV_GPIO_16)\
		||(pin_num == DRV_GPIO_17) \
		||(pin_value >= DRV_GPIO_LEVEL_MAX))
	{
		return DRV_ERR_INVALID_PARAM;
	}
		
    /*lock irq*/
   	 flag = system_irq_save();
		
	if(pin_value == DRV_GPIO_LEVEL_LOW) //gpio Data Out clear reg
	{

		#if 0
		RegTmp = IN32(GPIO_CHNNL_DATA_OUT_CLEAR);
		RegTmp |=(1<<pin_num);
		OUT32(GPIO_CHNNL_DATA_OUT_CLEAR, RegTmp);
		#else
		RegTmp = IN32(GPIO_CHNNL_DATA_OUT);
		RegTmp &=~(1<<pin_num);
		OUT32(GPIO_CHNNL_DATA_OUT, RegTmp);
		#endif

	}
	else if(pin_value == DRV_GPIO_LEVEL_HIGH) //gpio Data Out set reg
	{

		#if 0
		RegTmp = IN32(GPIO_CHNNL_DATA_OUT_SET);
		RegTmp |=(1<<pin_num);
		OUT32(GPIO_CHNNL_DATA_OUT, RegTmp);
		#else
		RegTmp = IN32(GPIO_CHNNL_DATA_OUT);
		RegTmp |=(1<<pin_num);
		OUT32(GPIO_CHNNL_DATA_OUT, RegTmp);
		#endif
	}
	else
	{
		//other status
    }

	 //unlock irq 
	  system_irq_restore(flag);
	
	 return DRV_SUCCESS;
}

/*******************************************************************************
 * Function: gpio_write
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
int32_t gpio_get_write_value(uint32_t pin_num,uint32_t *pin_value)
{
   	uint32_t RegTmp = 0;
	uint32_t flag = 0;
	
	if((pin_num >= DRV_GPIO_MAX) \
		||(pin_num == DRV_GPIO_16)\
		||(pin_num == DRV_GPIO_17))
	{
		return DRV_ERR_INVALID_PARAM;
	}
		
    /*lock irq*/
   	 flag = system_irq_save();

	RegTmp = IN32(GPIO_CHNNL_DATA_OUT);
	RegTmp = (RegTmp >> pin_num) & 0x1;
	
	memcpy(pin_value,&RegTmp,sizeof(uint32_t));

	/*unlock irq*/
	system_irq_restore(flag);
	
	return DRV_SUCCESS;
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
int32_t gpio_read(uint32_t pin_num,uint32_t *pin_value)
{
   	uint32_t RegTmp = 0;
	uint32_t flag = 0;
	
	if((pin_num >= DRV_GPIO_MAX) \
		||(pin_num == DRV_GPIO_16)\
		||(pin_num == DRV_GPIO_17)||(pin_value == NULL))
	{
		return DRV_ERR_INVALID_PARAM;
	}

    /*lock irq*/
   	flag = system_irq_save();
		
	RegTmp = IN32(GPIO_CHNNL_DATA_IN);

	RegTmp = (RegTmp >> pin_num) & 0x1;

	memcpy(pin_value,&RegTmp,sizeof(uint32_t));

	//unlock irq 
	system_irq_restore(flag);
	
	return DRV_SUCCESS;
}

int32_t gpio_read_tuya(uint32_t pin_num)
{
   	uint32_t RegTmp = 0;
	uint32_t flag = 0;
	
	if((pin_num >= DRV_GPIO_MAX) \
		||(pin_num == DRV_GPIO_16)\
		||(pin_num == DRV_GPIO_17))
	{
		return DRV_ERR_INVALID_PARAM;
	}

    /*lock irq*/
   	flag = system_irq_save();
		
	RegTmp = IN32(GPIO_CHNNL_DATA_IN);

	RegTmp = (RegTmp >> pin_num) & 0x1;

	//unlock irq 
	system_irq_restore(flag);
	
	return RegTmp;
}

/*******************************************************************************
 * Function: gpio_wakeup_config
 * Description: 
 * Parameters: 
 *   Input:DRV_GPIO_13,DRV_GPIO_17
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
int32_t gpio_wakeup_config(uint32_t pin_num,uint32_t wakeup_en)
{
	uint32_t RegTmp = 0;
	uint32_t flag = 0;
	
	if((pin_num == DRV_GPIO_13) || (pin_num == DRV_GPIO_17) || (wakeup_en <= DRV_GPIO_WAKEUP_MAX))
	{
	     /*lock irq*/
	   	flag = system_irq_save();
		
		if(pin_num == DRV_GPIO_13)
		{
			if(wakeup_en == DRV_GPIO_WAKEUP_EN)
			{
				// OUT32(SOC_AON_GPIO13_MUX, 11);//set GPIO13 to wakeup

				OUT32(SOC_AON_GPIO13_MUX, IN32(SOC_AON_GPIO13_MUX) & ~(0x1 << 1));
				OUT32(SOC_AON_GPIO13_MUX, IN32(SOC_AON_GPIO13_MUX) | 0x1 );
				OUT32(SOC_AON_GPIO17_MUX, IN32(SOC_AON_GPIO17_MUX) & ~0x1 );
			}
			else if(wakeup_en == DRV_GPIO_WAKEUP_DIS)
			{
				OUT32(SOC_AON_GPIO13_MUX, 00); //set GPIO13 to gpio
			}
		}
		else if(pin_num == DRV_GPIO_17)
		{
			
			if(wakeup_en == DRV_GPIO_WAKEUP_EN)
			{
				// OUT32(SOC_AON_GPIO13_MUX, 0x01);//set GPIO17 to wakeup

				OUT32(SOC_AON_GPIO13_MUX, IN32(SOC_AON_GPIO13_MUX) & ~(0x1));
				OUT32(SOC_AON_GPIO13_MUX, IN32(SOC_AON_GPIO13_MUX) | 0x1 << 1);
				OUT32(SOC_AON_GPIO17_MUX, IN32(SOC_AON_GPIO17_MUX) | 0x1 );
			}
			else if(wakeup_en == DRV_GPIO_WAKEUP_DIS)
			{
				RegTmp = IN32(SOC_AON_GPIO13_MUX);
				RegTmp &=0x01; //set bit0 to 0;
				OUT32(SOC_AON_GPIO13_MUX, RegTmp); //set GPIO17 to gpio

				OUT32(SOC_AON_GPIO17_MUX, IN32(SOC_AON_GPIO17_MUX) &  ~(0x1));
			}
		}
		else
		{
			//other status
		}

		//unlock irq 
		system_irq_restore(flag);
		return DRV_SUCCESS;
	}
	else
	{
		return DRV_ERR_INVALID_PARAM;
	}
}

/*******************************************************************************
 * Function: gpio_power_area_config
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
int32_t gpio_power_area_config(uint32_t pin_num,uint32_t pwr)
{
	uint32_t Rtn = 0;
	uint32_t flag = 0;
	uint32_t RegTmp = 0;

   if(pin_num > DRV_GPIO_MAX)
   {
		return DRV_ERR_INVALID_PARAM;
   }
   
   /*lock irq*/
   	flag = system_irq_save();
   
	switch (pwr)
	{
		case  DRV_GPIO_PWR_PD:
			if(pin_num == DRV_GPIO_16)
			{
				OUT32(SOC_AON_GPIO16_MUX, 0x0);
			    Rtn = DRV_SUCCESS;
			}
			else if(pin_num == DRV_GPIO_17)
			{
				OUT32(SOC_AON_GPIO17_MUX, 0x0);
				Rtn = DRV_SUCCESS;
			}
			else if(pin_num == DRV_GPIO_MAX)
			{
				OUT32(SOC_AON_GPIO_MOD_SWITCH, 0x0);
				Rtn = DRV_SUCCESS;
			}
			else
			{			
				RegTmp = IN32(SOC_AON_GPIO_MOD_SWITCH);
				RegTmp &= ~(1<<pin_num);
				OUT32(SOC_AON_GPIO_MOD_SWITCH, RegTmp);
				Rtn = DRV_SUCCESS;
			}
			break;
			
		case DRV_GPIO_PWR_AO:
			if(pin_num == DRV_GPIO_16)
			{
				OUT32(SOC_AON_GPIO16_MUX, 0x1);
				Rtn = DRV_SUCCESS;
			}
			else if(pin_num == DRV_GPIO_17)
			{
				OUT32(SOC_AON_GPIO17_MUX, 0x1);
				Rtn = DRV_SUCCESS;
			}
			else if(pin_num == DRV_GPIO_MAX)
			{
				OUT32(SOC_AON_GPIO_MOD_SWITCH, 0x1);
				Rtn = DRV_SUCCESS;
			}
			else
			{
				RegTmp = IN32(SOC_AON_GPIO_MOD_SWITCH);
				RegTmp |= (1<<pin_num);
				OUT32(SOC_AON_GPIO_MOD_SWITCH, RegTmp);
				Rtn = DRV_SUCCESS;
			}
			break;
		default:
			break;
	}

    //unlock irq 
	  system_irq_restore(flag);
	
	return Rtn;
}

/*******************************************************************************
 * Function: gpio16_write
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
int32_t gpio16_write(uint32_t pin_value)
{
	uint32_t RegTmp = 0;
	uint32_t flag = 0;

   
    #if 0
	if((pull_up > DRV_GPIO_PULL_UP_MAX)||(pin_value >= DRV_GPIO_LEVEL_MAX))
	{
		return DRV_ERR_INVALID_PARAM;
	}
    // pull up enable
	if(pull_up == DRV_GPIO_PULL_UP_EN)
	{
		RegTmp |= BIT3; // pull up enable,set bit3 to 1
	}
	else if(pull_up == DRV_GPIO_PULL_UP_DIS)
	{
		RegTmp &=~BIT3;// pull up disable,set bit3 to 0
	}
    #endif
	
	/*lock irq*/
   	flag = system_irq_save();

	//disable test mode
	OUT32(SOC_AON_TEST_MOD,DRV_GPIO_TESTMOD_DIS);

#if 1

	pin_value = !!pin_value;

	OUT32(GPIO_CHNNL_DIR, IN32(GPIO_CHNNL_DIR) | (1<<16));

	RegTmp = IN32(GPIO_CHNNL_DATA_OUT) & (~(1<<16));
	
	OUT32(GPIO_CHNNL_DATA_OUT,  RegTmp | (pin_value<<16));

#else
	RegTmp = IN32(SOC_AON_LIGHT_CONF);
		
   //set data to gpio
    RegTmp &=(0xFFFFFFFF&(pin_value<<2)); // value is bit2

	//en output
	RegTmp |=0x10; //enable outup is bit1
	OUT32(SOC_AON_LIGHT_CONF, RegTmp); 
#endif
	//unlock irq 
	system_irq_restore(flag);
	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function: gpio17_write
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
int32_t gpio17_write(uint32_t pin_value)
{
	uint32_t RegTmp = 0;
	uint32_t flag = 0;
	

	/*lock irq*/
   	flag = system_irq_save();

	//disable wakeup function
	gpio_wakeup_config(DRV_GPIO_17, DRV_GPIO_WAKEUP_DIS);

#if 1
	pin_value = !!pin_value;

	OUT32(GPIO_CHNNL_DIR, IN32(GPIO_CHNNL_DIR) | (1<<17));

	RegTmp =IN32(GPIO_CHNNL_DATA_OUT) & ~(1<<17);
	OUT32(GPIO_CHNNL_DATA_OUT,  RegTmp| pin_value<<17);

#else
	RegTmp = IN32(SOC_AON_LIGHT_CONF);

	//set data to gpio
    RegTmp &=(0xFFFFFFFF&(pin_value<<6)); // value is bit6

	//set dir to output
	RegTmp |= BIT5; //output enable ,set bit5 to 1

	OUT32(SOC_AON_LIGHT_CONF, RegTmp);
#endif

	//unlock irq 
	system_irq_restore(flag);
	
	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function: gpio17_read
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
int32_t gpio17_read_config(uint32_t pull_up_en)
{

	uint32_t RegTmp = 0;
	uint32_t flag = 0;
	
	if(pull_up_en >= DRV_GPIO_PULL_UP_MAX)
	{
		return DRV_ERR_INVALID_PARAM;
	}

	/*lock irq*/
   	flag = system_irq_save();

	//disable wakeup function
	gpio_wakeup_config(DRV_GPIO_17, DRV_GPIO_WAKEUP_DIS);

#if 0
	RegTmp = IN32(SOC_AON_LIGHT_CONF);
	
	// pull up enable
	if(pull_up_en == DRV_GPIO_PULL_UP_EN)
	{
		RegTmp |= BIT7; // pull up enable,set bit3 to 1
	}
	else if(pull_up_en == DRV_GPIO_PULL_UP_DIS)
	{
		RegTmp &=~BIT7;// pull up disable,set bit3 to 0
	}
	else
	{
	}
	
	//set dir to input
    RegTmp |= BIT4; //input enable ,set bit4 to 1
	OUT32(SOC_AON_LIGHT_CONF, RegTmp); 
#else

 	RegTmp = IN32(GPIO_CHNNL_DIR);
	RegTmp &=~(1<<(DRV_GPIO_17));
	OUT32(GPIO_CHNNL_DIR,RegTmp);

#endif

	//unlock irq 
	system_irq_restore(flag);
	
	return DRV_SUCCESS;
}


/*******************************************************************************
 * Function: gpio17_read
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
int32_t gpio17_read(uint32_t *pin_value)
{
	uint32_t RegTmp = 0;
	uint32_t flag = 0;

	/*lock irq*/
   	flag = system_irq_save();
	
	RegTmp = IN32(SOC_AON_LIGHT_CONF);
	
	//read data from gpio
	RegTmp = IN32(GPIO_CHNNL_DATA_IN);
	RegTmp = (RegTmp >> DRV_GPIO_17) & 0x1;
	memcpy(pin_value,&RegTmp,sizeof(uint32_t));

	//unlock irq 
	system_irq_restore(flag);
	
	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function: gpio_set_irq_en
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
 int32_t hal_gpio_isr(int32_t vector)
{
    uint32_t gpio_int_status = 0;
	uint32_t int_tmp = 0;
    uint32_t i = 0;

    //1.clear INT GPIO
    irq_status_clean(vector);

    //2.clear chnnl int
    gpio_int_status = IN32(GPIO_INT_STATUS);
    OUT32(GPIO_INT_STATUS, gpio_int_status);
	
    for(i = 0; i < DRV_GPIO_MAX; i++)
    {
		int_tmp = (gpio_int_status&(1<<i));
		if((gpio_callback[i].callback != NULL)&&(int_tmp != 0))
		{
			gpio_callback[i].callback(gpio_callback[i].data);
		}
    }

    return 0;
}

/*******************************************************************************
 * Function: gpio_init
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
void gpio_isr_init(void)
{
	irq_isr_register(IRQ_VECTOR_GPIO, (void *)hal_gpio_isr);
	irq_status_clean(IRQ_VECTOR_GPIO);
	irq_unmask(IRQ_VECTOR_GPIO);
}

/*******************************************************************************
 * Function: gpio_irq_level
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
int32_t  gpio_irq_level(uint32_t pin_num, uint32_t mode)
{
	DRV_GPIO_INT_MODE	int_mode;
	int32_t	int_region = 0;

	if((pin_num >= DRV_GPIO_MAX) \
		||(pin_num == DRV_GPIO_16)\
		||(pin_num == DRV_GPIO_17) \
		|| (mode > DRV_GPIO_INT_LEVEL_MAX))
	{
		return DRV_ERR_INVALID_PARAM;
	}

	#if 0
	int_mode = IN32(GPIO_CHNx_INT_BASE(pin_num));

	int_mode &= ~(0x7 << (pin_num % 8));

	OUT32(GPIO_CHNx_INT_BASE(pin_num), int_mode | (mode << (pin_num % 8)));
	#endif

	//1.get pin's region & read region
	int_mode.gpio_int_mode = IN32(GPIO_CHNx_INT_BASE(pin_num));

	//2.
	int_region = (pin_num%8);
	
	switch(int_region)
	{
		case 0:
		int_mode.interrupt_mode_chnnl_0 = mode;
			break;

		case 1:
		int_mode.interrupt_mode_chnnl_1 = mode;
			break;

		case 2:
		int_mode.interrupt_mode_chnnl_2 = mode;
			break;

		case 3:
		int_mode.interrupt_mode_chnnl_3 = mode;
			break;

		case 4:
		int_mode.interrupt_mode_chnnl_4 = mode;
			break;

		case 5:
		int_mode.interrupt_mode_chnnl_5 = mode;
			break;

		case 6:
		int_mode.interrupt_mode_chnnl_6 = mode;
			break;

		case 7:
		int_mode.interrupt_mode_chnnl_7 = mode;
			break;

		default:
			break;
	}

	OUT32(GPIO_CHNx_INT_BASE(pin_num), int_mode.gpio_int_mode);
	
	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function: gpio_irq_callback
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
int32_t  gpio_irq_callback_register(uint32_t pin_num, void (* callback)(void * data),  void  *data)
{
	if((pin_num >= DRV_GPIO_MAX) \
		||(pin_num == DRV_GPIO_16)\
		||(pin_num == DRV_GPIO_17))
	{
		return DRV_ERR_INVALID_PARAM;
	}
	

	gpio_callback[pin_num].callback = callback;
	gpio_callback[pin_num].data = data;

	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function: gpio_set_irq_en
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
 int32_t  gpio_irq_unmusk(uint32_t pin_num)
{
    uint32_t  irqEn = 0;
	if((pin_num >= DRV_GPIO_MAX) \
		||(pin_num == DRV_GPIO_16)\
		||(pin_num == DRV_GPIO_17) )
	{
		return DRV_ERR_INVALID_PARAM;
	}

	irqEn = IN32(GPIO_INT_EN);
	OUT32(GPIO_INT_EN, irqEn|(GPIO_INT_ENABLE << pin_num));

	return DRV_SUCCESS;
}

/*******************************************************************************
 * Function: gpio_set_irq_en
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
 int32_t  gpio_irq_musk(uint32_t pin_num)
{
    int  irqEn = 0;
    if((pin_num >= DRV_GPIO_MAX) \
	||(pin_num == DRV_GPIO_16)\
	||(pin_num == DRV_GPIO_17) )
    {
		return DRV_ERR_INVALID_PARAM;
	}

    irqEn = IN32(GPIO_INT_EN);
    irqEn &= ~(GPIO_INT_ENABLE << pin_num);
    OUT32(GPIO_INT_EN, irqEn);

    return DRV_SUCCESS;
}

int gpio_pull_en = 0;
int pin_mux1 = 0;
int pin_mux2 = 0;
int test_mode = 0x0;
int wakeup = 0;
 void TsPsmSaveInfoForSysDeepSleep(void)
 {
#if 0
	gpio_pull_en = IN32(0x607040);
	pin_mux1 = IN32(0x601824);
	pin_mux2 = IN32(0x601828);

	PIN_FUNC_SET(IO_MUX0_GPIO0, FUNC_GPIO0_GPIO0);
	PIN_FUNC_SET(IO_MUX0_GPIO1, FUNC_GPIO1_GPIO1);
	PIN_FUNC_SET(IO_MUX0_GPIO2, FUNC_GPIO2_GPIO2);
	PIN_FUNC_SET(IO_MUX0_GPIO3, FUNC_GPIO3_GPIO3);
	PIN_FUNC_SET(IO_MUX0_GPIO4, FUNC_GPIO4_GPIO4);
	PIN_FUNC_SET(IO_MUX0_GPIO5, FUNC_GPIO5_GPIO5);
	PIN_FUNC_SET(IO_MUX0_GPIO6, FUNC_GPIO6_GPIO6);
#ifndef _USR_TR6260S1
	PIN_FUNC_SET(IO_MUX0_GPIO7, FUNC_GPIO7_GPIO7);
#endif
	//PIN_FUNC_SET(IO_MUX0_GPIO8, FUNC_GPIO8_GPIO8);
#ifndef _USR_TR6260S1
	PIN_FUNC_SET(IO_MUX0_GPIO9, FUNC_GPIO9_GPIO9);
#endif
	//PIN_FUNC_SET(IO_MUX0_GPIO10, FUNC_GPIO10_GPIO10);
#ifndef _USR_TR6260S1
	PIN_FUNC_SET(IO_MUX0_GPIO11, FUNC_GPIO11_GPIO11);
	PIN_FUNC_SET(IO_MUX0_GPIO12, FUNC_GPIO12_GPIO12);
#endif
	//PIN_FUNC_SET(IO_MUX0_GPIO13, FUNC_GPIO13_GPIO13);
	wakeup = IN32(0x601218);
	OUT32(0x601218, 0x2);
	PIN_FUNC_SET(IO_MUX0_GPIO14, FUNC_GPIO14_GPIO14);
	PIN_FUNC_SET(IO_MUX0_GPIO15, FUNC_GPIO15_GPIO15);
	//PIN_FUNC_SET(IO_MUX0_GPIO16, FUNC_GPIO16_GPIO16);
	test_mode = IN32(0x601214);
	OUT32(0x601214, 0x0);
	OUT32(0x60121c, 0x0);
	//PIN_FUNC_SET(IO_MUX0_GPIO17, FUNC_GPIO17_GPIO17);
	//OUT32(0x601220, 0x0);
#ifndef _USR_TR6260S1
	PIN_FUNC_SET(IO_MUX0_GPIO18, FUNC_GPIO18_GPIO18);
	PIN_FUNC_SET(IO_MUX0_GPIO19, FUNC_GPIO19_GPIO19);
#endif
	PIN_FUNC_SET(IO_MUX0_GPIO20, FUNC_GPIO20_GPIO20);
	PIN_FUNC_SET(IO_MUX0_GPIO21, FUNC_GPIO21_GPIO21);
	PIN_FUNC_SET(IO_MUX0_GPIO22, FUNC_GPIO22_GPIO22);
#ifndef _USR_TR6260S1
	PIN_FUNC_SET(IO_MUX0_GPIO23, FUNC_GPIO23_GPIO23);
	PIN_FUNC_SET(IO_MUX0_GPIO24, FUNC_GPIO24_GPIO24);
#endif
	//direction set  0:input 1:output
	OUT32(0x607028, 0x0);

	//pull en: 0 : disable, 1;enbale
	//OUT32(0x607040, ~(0x32000));
	//OUT32(0x607040, 0xffffffff);

	//0: pull up, 1:pull down
	//OUT32(0x607044, ~(0x20));
	//OUT32(0x607044, 0x30044);
	OUT32(0x607044, 0x3006c);
#endif
 	//gpiomode
	//OUT32(0x601224, 0x1);
	//add gpio config above

 }

 void TsPsmRestoreForSysDeepSleep(void)
 {
#if 0
	//same as TsPsmSaveInfoForSysDeepSleep
	OUT32(0x607040, gpio_pull_en);
	OUT32(0x601824, pin_mux1);
	OUT32(0x601828, pin_mux2);
	OUT32(0x601218, wakeup);
	OUT32(0x601214, test_mode);
#endif
	//add gpio above
 	//quit gpiomode
	OUT32(0x601224, 0x0);

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
#if 0
static int g17_read(cmd_tbl_t *t, int argc, char *argv[])
{
	uint32_t value;
	gpio17_read_config(DRV_GPIO_PULL_UP_DIS);
	gpio17_read(&value);
	system_printf("g17_read, val: %d\n", value);
	return CMD_RET_SUCCESS;
}

CMD(g17,   g17_read,   "g17_read test",   "g17_read test");
#endif

#ifdef _USE_TEST_CMD_GPIO
static void hal_gpio_chnnl_int_isr(void *data)
{
	system_printf("gpio_chnnl_int_isr \n");

}
static int cmd_gpio(cmd_tbl_t *t, int argc, char *argv[])
{
	DRV_GPIO_CONFIG  tGpioCfg;
	uint32_t gpio_temp = 0;

	if (strcmp(argv[1], "24on") == 0) 
	{
	    //0.set GPIO11 to gpio mode
		PIN_FUNC_SET(IO_MUX0_GPIO24,FUNC_GPIO24_GPIO24);
		
		//2.set value
		gpio_write(DRV_GPIO_24,DRV_GPIO_LEVEL_HIGH);

		//3.set dir
		gpio_set_dir(DRV_GPIO_24,DRV_GPIO_DIR_OUTPUT);
		
		system_printf("GPIO24 is output and set to 1\n");
		return CMD_RET_SUCCESS;
	} 
	else if (strcmp(argv[1], "24off") == 0) 
	{
		//0.set GPIO11 to gpio mode
		PIN_FUNC_SET(IO_MUX0_GPIO24,FUNC_GPIO24_GPIO24);
		
		//2.set value
		gpio_write(DRV_GPIO_24,DRV_GPIO_LEVEL_LOW);

		//3.set dir
		gpio_set_dir(DRV_GPIO_24,DRV_GPIO_DIR_OUTPUT);
	    system_printf("GPIO24 is output and set to 0\n");
		return CMD_RET_SUCCESS;
	}
	 else if (strcmp(argv[1], "12on") == 0) 
	{
		 //0.set GPIO11 to gpio mode
		  PIN_FUNC_SET(IO_MUX0_GPIO12,FUNC_GPIO12_GPIO12);
		 
		  //2.set value
		  gpio_write(DRV_GPIO_12,DRV_GPIO_LEVEL_HIGH);
		 
		  //3.set dir
		  gpio_set_dir(DRV_GPIO_12,DRV_GPIO_DIR_OUTPUT);
				 
		  system_printf("GPIO12 is output and set to 1\n");
		  return CMD_RET_SUCCESS;
	} 
	else if (strcmp(argv[1], "12off") == 0) 
	{
		  //0.set GPIO11 to gpio mode
		  PIN_FUNC_SET(IO_MUX0_GPIO12,FUNC_GPIO12_GPIO12);
				 		 
		  //2.set value
		  gpio_write(DRV_GPIO_12,DRV_GPIO_LEVEL_LOW);
		 
		  //3.set dir
		  gpio_set_dir(DRV_GPIO_12,DRV_GPIO_DIR_OUTPUT);
		  
		  system_printf("GPIO12 is output and set to 0\n");
		  return CMD_RET_SUCCESS;
	}
	else if (strcmp(argv[1], "13on") == 0) 
	{
	    //0.set gpio13 to gpio mode
	    gpio_wakeup_config(DRV_GPIO_13,DRV_GPIO_WAKEUP_DIS);

		//1.set GPIO13 to gpio mode
		 PIN_FUNC_SET(IO_MUX0_GPIO13,FUNC_GPIO13_GPIO13);
				 		 
		//2.set value
		 gpio_write(DRV_GPIO_13,DRV_GPIO_LEVEL_HIGH);
		 
		//3.set dir
		 gpio_set_dir(DRV_GPIO_13,DRV_GPIO_DIR_OUTPUT);
		
		system_printf("GPIO13 is output and set to 1\n");
		return CMD_RET_SUCCESS;
	} 
	else if (strcmp(argv[1], "13off") == 0) 
	{
	    //0.set gpio13 to gpio mode
	    gpio_wakeup_config(DRV_GPIO_13,DRV_GPIO_WAKEUP_DIS);

		//1.set GPIO13 to gpio mode
		 PIN_FUNC_SET(IO_MUX0_GPIO13,FUNC_GPIO13_GPIO13);
				 		 
		//2.set value
		 gpio_write(DRV_GPIO_13,DRV_GPIO_LEVEL_LOW);
		 
		//3.set dir
		 gpio_set_dir(DRV_GPIO_13,DRV_GPIO_DIR_OUTPUT);
		
		system_printf("GPIO13 is output and set to 0\n");

		return CMD_RET_SUCCESS;
	}
	else if (strcmp(argv[1], "14on") == 0) 
	{
		//0.set to gpio mode
		PIN_FUNC_SET(IO_MUX0_GPIO14,FUNC_GPIO14_GPIO14);
		
		//1.set value
		gpio_write(DRV_GPIO_14,DRV_GPIO_LEVEL_HIGH);

		//2.set dir
		gpio_set_dir(DRV_GPIO_14,DRV_GPIO_DIR_OUTPUT);
	    system_printf("GPIO14 is output and set to 1\n");
		return CMD_RET_SUCCESS;
	}
	else if (strcmp(argv[1], "18on") == 0) 
	{
		//0.set GPIO18 to gpio mode
		PIN_FUNC_SET(IO_MUX0_GPIO18,FUNC_GPIO18_GPIO18);
		
		//1.set value
		gpio_write(DRV_GPIO_18,DRV_GPIO_LEVEL_HIGH);

		//2.set dir
		gpio_set_dir(DRV_GPIO_18,DRV_GPIO_DIR_OUTPUT);

		system_printf("GPIO18 is output and set to 1\n");

		//3.read value
		gpio_get_write_value(DRV_GPIO_18,&gpio_temp);
		system_printf("GPIO18 is output and read from reg is  %d\n",gpio_temp);
		gpio_get_dir(DRV_GPIO_18, &gpio_temp);
		system_printf("GPIO18 is output and read from reg is  %d\n",gpio_temp);
	    
		return CMD_RET_SUCCESS;
	}
	else if (strcmp(argv[1], "18off") == 0) 
	{
		//0.set GPIO18 to gpio mode
		PIN_FUNC_SET(IO_MUX0_GPIO18,FUNC_GPIO18_GPIO18);
		
		//1.set value
		gpio_write(DRV_GPIO_18,DRV_GPIO_LEVEL_LOW);

		//2.set dir
		gpio_set_dir(DRV_GPIO_18,DRV_GPIO_DIR_OUTPUT);
	    system_printf("GPIO18 is output and set to 0\n");

		//3.read value
		gpio_get_write_value(DRV_GPIO_18,&gpio_temp);
		
	    system_printf("GPIO18 is output read from reg is  %d\n",gpio_temp);
		
		return CMD_RET_SUCCESS;
	}
	else if (strcmp(argv[1], "19on") == 0) 
	{
		//0.set GPIO19 to gpio mode
		PIN_FUNC_SET(IO_MUX0_GPIO19,FUNC_GPIO19_GPIO19);
		
		//1.set value
		gpio_write(DRV_GPIO_19,DRV_GPIO_LEVEL_HIGH);

		//2.set dir
		gpio_set_dir(DRV_GPIO_19,DRV_GPIO_DIR_OUTPUT);
	    system_printf("GPIO19 is output and set to 1\n");
		return CMD_RET_SUCCESS;
	}
	else if (strcmp(argv[1], "19off") == 0) 
	{
		//0.set GPIO11 to gpio mode
		PIN_FUNC_SET(IO_MUX0_GPIO19,FUNC_GPIO19_GPIO19);
		
		//1.set value
		gpio_write(DRV_GPIO_19,DRV_GPIO_LEVEL_LOW);

		//2.set dir
		gpio_set_dir(DRV_GPIO_19,DRV_GPIO_DIR_OUTPUT);
	    system_printf("GPIO19 is output and set to 0\n");
		return CMD_RET_SUCCESS;
	}
	else if (strcmp(argv[1], "high") == 0) 
	{

		//1.register int
		gpio_isr_init();
		
		//2.set GPIO11 to gpio mode
		//PIN_FUNC_SET(IO_MUX0_GPIO19,FUNC_GPIO19_GPIO19);
		PIN_FUNC_SET(IO_MUX0_GPIO23,FUNC_GPIO23_GPIO23);
		
		//3.set dir
		//gpio_set_dir(DRV_GPIO_19,DRV_GPIO_DIR_INPUT);
		gpio_set_dir(DRV_GPIO_23,DRV_GPIO_DIR_INPUT);

		//4.set int status
		//gpio_irq_level(DRV_GPIO_19,DRV_GPIO_INT_HIGH_LEVEL);,
		gpio_irq_level(DRV_GPIO_23,DRV_GPIO_INT_HIGH_LEVEL);

		//5.register callback function
		//gpio_irq_callback_register(DRV_GPIO_19, hal_gpio_chnnl_int_isr, NULL);
		gpio_irq_callback_register(DRV_GPIO_23, hal_gpio_chnnl_int_isr, NULL);

		//6.enable chnnl INT
		gpio_irq_unmusk(DRV_GPIO_23);
		
	    system_printf("GPIO23 is input and set to 0\n");
		return CMD_RET_SUCCESS;
	}
	else if (strcmp(argv[1], "low") == 0) 
	{

		//1.register int
		gpio_isr_init();
		
		//2.set GPIO11 to gpio mode
		//PIN_FUNC_SET(IO_MUX0_GPIO19,FUNC_GPIO19_GPIO19);
		PIN_FUNC_SET(IO_MUX0_GPIO23,FUNC_GPIO23_GPIO23);
		
		//3.set dir
		//gpio_set_dir(DRV_GPIO_19,DRV_GPIO_DIR_INPUT);
		gpio_set_dir(DRV_GPIO_23,DRV_GPIO_DIR_INPUT);

		//4.set int status
		//gpio_irq_level(DRV_GPIO_19,DRV_GPIO_INT_HIGH_LEVEL);,
		gpio_irq_level(DRV_GPIO_23,DRV_GPIO_INT_LOW_LEVEL);

		//5.register callback function
		//gpio_irq_callback_register(DRV_GPIO_19, hal_gpio_chnnl_int_isr, NULL);
		gpio_irq_callback_register(DRV_GPIO_23, hal_gpio_chnnl_int_isr, NULL);

		//6.enable chnnl INT
		gpio_irq_unmusk(DRV_GPIO_23);
		
	    system_printf("GPIO23 is output and set to 0\n");
		return CMD_RET_SUCCESS;
	}
	else if (strcmp(argv[1], "pos_edge") == 0) 
	{

		//1.register int
		gpio_isr_init();
		
		//2.set GPIO11 to gpio mode
		//PIN_FUNC_SET(IO_MUX0_GPIO19,FUNC_GPIO19_GPIO19);
		PIN_FUNC_SET(IO_MUX0_GPIO23,FUNC_GPIO23_GPIO23);
		
		//3.set dir
		//gpio_set_dir(DRV_GPIO_19,DRV_GPIO_DIR_INPUT);
		gpio_set_dir(DRV_GPIO_23,DRV_GPIO_DIR_INPUT);

		//4.set int status
		//gpio_irq_level(DRV_GPIO_19,DRV_GPIO_INT_HIGH_LEVEL);,
		gpio_irq_level(DRV_GPIO_23,DRV_GPIO_INT_POS_EDGE);

		//5.register callback function
		//gpio_irq_callback_register(DRV_GPIO_19, hal_gpio_chnnl_int_isr, NULL);
		gpio_irq_callback_register(DRV_GPIO_23, hal_gpio_chnnl_int_isr, NULL);

		//6.enable chnnl INT
		gpio_irq_unmusk(DRV_GPIO_23);
		
	    system_printf("GPIO23 is output and set to 0\n");
		return CMD_RET_SUCCESS;
	}
	else if (strcmp(argv[1], "neg_edge") == 0) 
	{

		//1.register int
		gpio_isr_init();
		
		//2.set GPIO11 to gpio mode
		//PIN_FUNC_SET(IO_MUX0_GPIO19,FUNC_GPIO19_GPIO19);
		PIN_FUNC_SET(IO_MUX0_GPIO23,FUNC_GPIO23_GPIO23);
		
		//3.set dir
		//gpio_set_dir(DRV_GPIO_19,DRV_GPIO_DIR_INPUT);
		gpio_set_dir(DRV_GPIO_23,DRV_GPIO_DIR_INPUT);

		//4.set int status
		//gpio_irq_level(DRV_GPIO_19,DRV_GPIO_INT_HIGH_LEVEL);,
		gpio_irq_level(DRV_GPIO_23,DRV_GPIO_INT_NEG_EDGE);

		//5.register callback function
		//gpio_irq_callback_register(DRV_GPIO_19, hal_gpio_chnnl_int_isr, NULL);
		gpio_irq_callback_register(DRV_GPIO_23, hal_gpio_chnnl_int_isr, NULL);

		//6.enable chnnl INT
		gpio_irq_unmusk(DRV_GPIO_23);
		
	    system_printf("GPIO23 is output and set to 0\n");
		return CMD_RET_SUCCESS;
	}
	else if (strcmp(argv[1], "dual_edge") == 0) 
	{

		//1.register int
		gpio_isr_init();
		
		//2.set GPIO11 to gpio mode
		//PIN_FUNC_SET(IO_MUX0_GPIO19,FUNC_GPIO19_GPIO19);
		PIN_FUNC_SET(IO_MUX0_GPIO23,FUNC_GPIO23_GPIO23);
		
		//3.set dir
		//gpio_set_dir(DRV_GPIO_19,DRV_GPIO_DIR_INPUT);
		gpio_set_dir(DRV_GPIO_23,DRV_GPIO_DIR_INPUT);

		//4.set int status
		//gpio_irq_level(DRV_GPIO_19,DRV_GPIO_INT_HIGH_LEVEL);,
		gpio_irq_level(DRV_GPIO_23,DRV_GPIO_INT_DUAL_EDGE);

		//5.register callback function
		//gpio_irq_callback_register(DRV_GPIO_19, hal_gpio_chnnl_int_isr, NULL);
		gpio_irq_callback_register(DRV_GPIO_23, hal_gpio_chnnl_int_isr, NULL);

		//6.enable chnnl INT
		gpio_irq_unmusk(DRV_GPIO_23);
		
	    system_printf("GPIO23 is output and set to 0\n");
		return CMD_RET_SUCCESS;
	}
    else
    {
		system_printf("INVALID CMD!!!!\n");
		return CMD_RET_FAILURE;
    }
}


SUBCMD(set,
    gpio,
    cmd_gpio,
    "GPIO output test",
    "GPIO ISR test");
#if 0
#include "drv_rtc.h"

static int cmd_gpio_mode_test(cmd_tbl_t *t, int argc, char *argv[])
{
	unsigned long flags = system_irq_save();
	OUT32(0x601224, 0x1);

	int rtcCnt = IN32(RTC_32K_CNT_REG);
	while(1)
	{
		//2.set value
		gpio_write(DRV_GPIO_24,DRV_GPIO_LEVEL_LOW);

		//3.set dir
		gpio_set_dir(DRV_GPIO_24,DRV_GPIO_DIR_OUTPUT);

		gpio_wakeup_config(DRV_GPIO_17, DRV_GPIO_WAKEUP_DIS);

		gpio17_write(1);

		while(1)
		{
			if(IN32(RTC_32K_CNT_REG) - rtcCnt > 0x1ffff)
			{
				rtcCnt  =  IN32(RTC_32K_CNT_REG);
				break;
			}
		}

		//2.set value
		gpio_write(DRV_GPIO_24,DRV_GPIO_LEVEL_HIGH);

		//3.set dir
		gpio_set_dir(DRV_GPIO_24,DRV_GPIO_DIR_OUTPUT);

		gpio_wakeup_config(DRV_GPIO_17, DRV_GPIO_WAKEUP_DIS);

		gpio17_write(0);

		while(1)
		{
			if(IN32(RTC_32K_CNT_REG) - rtcCnt > 0x1ffff)
			{
				rtcCnt  =  IN32(RTC_32K_CNT_REG);
				break;
			}
		}
	}

	OUT32(0x601224, 0x0);
	system_irq_restore(flags);
	return CMD_RET_SUCCESS;
}

SUBCMD(set,
    gpiomod,
    cmd_gpio_mode_test,
    "GPIO output test",
    "GPIO ISR test");
#endif
#endif /* _USE_TEST_CMD */


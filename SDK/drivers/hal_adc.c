/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:  hal_adc.c  
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2018-12-25
 * History 1:      
 *     Date: 2020--3-16
 *     Version:
 *     Author: wangxia
 *     Modification: modify vbat sensor function
 * History 2: 
  ********************************************************************************/

/****************************************************************************
* 	                                           Include files
****************************************************************************/
#include "drv_adc.h"
#include "soc_pin_mux.h"
#include "drv_spiflash.h"


/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define SOC_ADC_PGA_0					SOC_ANALOG_BASE+0x7C
#define SOC_ADC_PGA_1					SOC_ANALOG_BASE+0x80
#define SOC_ADC_PGA_2					SOC_ANALOG_BASE+0x84
#define SOC_ADC_PGA_3					SOC_ANALOG_BASE+0x88
#define SOC_BB_CLK_GEN					SOC_ANALOG_BASE+0xAC

#define SOC_ADC_CONFIG					0x0060182C //ADC digital
#define SOC_ADC_OUT						0x00601830 //ADC digital

#define ADC_INPUT_VOLT_MIN				1200 // mV
#define ADC_INPUT_VOLT_MAX				3300 // mV

#define ADC_VDD_VOLT					1200 // mV
#define ADC_VREF_VOLT					ADC_VDD_VOLT
#define ADC_CNT_MAX						10
#define ADC_INPUT_VOLT_DIV_1			1
#define ADC_INPUT_VOLT_DIV_4			4
#define ADC_INPUT_VOLT_DIV_5			5
#define ADC_INPUT_VOLT_DIV_6			6

#define ADC_DIV_4_PARAM_OFFSET			463//463mV
#define ADC_DIV_4_PARAM_SLOPE			808
#define ADC_DIV_5_PARAM_OFFSET			588//588mV
#define ADC_DIV_5_PARAM_SLOPE			805
#define ADC_DIV_6_PARAM_OFFSET			0
#define ADC_DIV_6_PARAM_SLOPE			1
#define ADC_VOLT_MIN_PARAM_OFFSET		0
#define ADC_VOLT_MIN_PARAM_SLOPE		1

#define ADC_TEMP_LOW					-10
#define ADC_TEMP_NORMAL_TEM				80
#define ADC_TEMP_HIGH					125
#define ADC_TEMP_SENSOR_TO_TEMP			1.6 /*40/25*/

TimerHandle_t	  hal_temp_Timer = NULL;
StaticTimer_t Timer_temp;
#define   Time_to_check_Temp     pdMS_TO_TICKS(60000)
unsigned int temp_status=1;

/****************************************************************************
* 	                                           Local Types
****************************************************************************/

typedef union 
{
		uint32_t adc_pga0;
		struct 
		{
			uint32_t dig_input_signal_muxB_en:1;
			uint32_t dig_input_signal_muxA_en:1;
			uint32_t dig_gpio4_res_divider_bypass:1;
			uint32_t dig_gpio4_volt_select:2;
			uint32_t dig_gpio4_enable:1;
			uint32_t dig_gpio3_res_divider_bypass:1;
			uint32_t dig_gpio3_volt_select:2;
			uint32_t dig_gpio3_enable:1;
			uint32_t dig_gpio2_res_divider_bypass:1;
			uint32_t dig_gpio2_volt_select:2;
			uint32_t dig_gpio2_enable:1;
			uint32_t dig_gpio1_res_divider_bypass:1;
			uint32_t dig_gpio1_volt_select:2;
			uint32_t dig_gpio1_enable:1;
			uint32_t dig_temp_sensor_res_ctrl2:4;
			uint32_t dig_temp_sensor_res_ctrl1:4;
			uint32_t dig_temp_sensor_en:1;
			uint32_t dig_vbat_select:2;
			uint32_t dig_vbat_enable:1;
			uint32_t reserver30_31:2;
		}dig_adc_pga0;
}DRV_ADC_PGA_0;
	
typedef union 
{
		uint32_t adc_pga1;
		struct
		{
			uint32_t dig_input_signal_mux_vrefA_sel:8;
			uint32_t dig_input_signal_mux_vrefB_sel_en:1;
			uint32_t dig_input_signal_mux_vrefA_sel_en:1;
			uint32_t dig_input_signal_muxB_ctrl:8;
			uint32_t dig_input_signal_muxA_ctrl:8;
			uint32_t reserver26_31:6;
		}dig_adc_pga1;
}DRV_ADC_PGA_1;
	
	
typedef union
{
		uint32_t adc_pga2;
		struct
		{
			uint32_t dig_pga_ibias2_ctrl:3;
			uint32_t dig_pga_ibias1_ctrl:3;
			uint32_t dig_pga_gain_sw:1;
			uint32_t dig_pga_bypass:1;
			uint32_t dig_pga_enable:1;
			uint32_t dig_input_driver_ibias2_ctrl:3;
			uint32_t dig_input_driver_ibias1_ctrl:3;
			uint32_t dig_input_driver_enable:1;
			uint32_t dig_input_signal_mux_vrefB_sel:8;
			uint32_t reserver24_31:8;
		}dig_adc_pga2;;
}DRV_ADC_PGA_2;
	
	
typedef union
{
		uint32_t adc_pga3;
		struct
		{
			uint32_t dig_diff_amp_bias_ctrl2:3;
			uint32_t dig_diff_amp_bias_ctrl1:3;
			uint32_t dig_sd_adc_vcm_sel:6;
			uint32_t dig_sd_adc_enable:1;
			uint32_t dig_pga_vcm_sel:11;
			uint32_t reserver30_31:2;
		}dig_adc_pga3;
}DRV_ADC_PGA_3;
	
	
typedef union
{
		uint32_t adc_clk_gen;
		struct
		{
			uint32_t dig_iqdac_clk_gen_sel:1;
			uint32_t dig_iqadc_clk_gen_sel:1;
			uint32_t dig_dbb_clk_gen_en:1;
			uint32_t dig_aux_adc_clk_gen_en:1;
			uint32_t reserver4_31:28;
		}dig_adc_clk_gen;
}DRV_ADC_BB_CLK_GEN;

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

#define POLYFIT_POINT_NUM 4
#define POINT_NUM 1
double polyfit_var [ 2 ] = {0.0,0.0};
uint8_t is_polyfit_above_1200 = 0;
uint8_t is_polyfit_below_1200 = 0;

double polyfit_x [ POLYFIT_POINT_NUM ] = {1000,1700,2400,3100};
double polyfit_y [ POLYFIT_POINT_NUM ] = {0,0,0,0};


#define POLYFIT_ADC_VOLT_ADDR_GD25Q80E_FLASH 0x1020
#define GET_TEMP_ADC_VOLT_ADDR_GD25Q80E_FLASH 0x1028
#define GET_CHIP_TYPE 0x1007

#define POLYFIT_ADC_VOLT_ADDR_EFUSE 		 0x15


#define GET_FLASH_ADC_VOLT_LEN				8
#define POLYFIT_ADC_VOLT_LEN				 8
#define CHIP_TYPE_ADDR_GD25Q80E_FLASH		 0x1007
#define CHIP_TYPE_ADDR_EFUSE		 		 4
#define CHIP_TYPE_LEN						 1


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

static int16_t sum(int16_t *arr,uint32_t cnt)  
{
   int16_t sum=0;
   uint32_t index =0;

   for(index=0;index<cnt;index++)
   {
       sum=sum+(*(arr+index));

   }

   return sum;
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
static int16_t hal_adc_get_oricode(int16_t NegCode)
{
	int16_t tempdata1 = 0;
	int16_t tempdata2 = 0;

	
	tempdata1 = NegCode >> 11;
	#if 0
	if(tempdata1 == 0)
	{
		return NegCode;
	}

    tempdata2 = ((~(NegCode&(~BIT11)))|(tempdata1<<1))+1;//set bit11 to 0
	#else
	if(tempdata1 == 0)
	{
		tempdata2 = (NegCode | BIT11);
	}
	else if(tempdata1 == 1)
	{
		tempdata2 = (NegCode&(~BIT11));
	}
	#endif

	return tempdata2;
}
/*******************************************************************************
 * Function: hal_adc_init
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
int32_t hal_adc_init(void)
{

	
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
 int32_t hal_adc_inputsel(void)
{
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
 //polyfit begin
double b_xnrm2(int n, const double x[8], int ix0)
{
  double y;
  double scale;
  int kend;
  int k;
  double absxk;
  double t;
  y = 0.0;
  if (!(n < 1)) {
    if (n == 1) {
      y = fabs(x[ix0 - 1]);
    } else {
      scale = 3.3121686421112381E-170;
      kend = (ix0 + n) - 1;
      for (k = ix0; k <= kend; k++) {
        absxk = fabs(x[k - 1]);
        if (absxk > scale) {
          t = scale / absxk;
          y = 1.0 + y * t * t;
          scale = absxk;
        } else {
          t = absxk / scale;
          y += t * t;
        }
      }

      y = scale * sqrt(y);
    }
  }

  return y;
}

/*
 * Arguments    : const double x[8]
 *                int ix0
 * Return Type  : double
 */


double xnrm2(const double x[8], int ix0)
{
  double y;
  double scale;
  int k;
  double absxk;
  double t;
  y = 0.0;
  scale = 3.3121686421112381E-170;
  for (k = ix0; k <= ix0 + 3; k++) {
    absxk = fabs(x[k - 1]);
    if (absxk > scale) {
      t = scale / absxk;
      y = 1.0 + y * t * t;
      scale = absxk;
    } else {
      t = absxk / scale;
      y += t * t;
    }
  }

  return scale * sqrt(y);
}

int ixamax(int n, const double x[2], int ix0)
{
  int idxmax;
  int ix;
  double smax;
  int k;
  double s;
  if (n < 1) {
    idxmax = 0;
  } else {
    idxmax = 1;
    if (n > 1) {
      ix = ix0 - 1;
      smax = fabs(x[ix0 - 1]);
      for (k = 2; k <= n; k++) {
        ix++;
        s = fabs(x[ix]);
        if (s > smax) {
          idxmax = k;
          smax = s;
        }
      }
    }
  }

  return idxmax;
}
bool rtIsNaN(double value)
{
  return (value!=value)? 1U:0U;
}

double rt_hypotd_snf(double u0, double u1)
{
  double y;
  double a;
  double b;
  a = fabs(u0);
  b = fabs(u1);
  if (a < b) {
    a /= b;
    y = b * sqrt(a * a + 1.0);
  } else if (a > b) {
    b /= a;
    y = a * sqrt(b * b + 1.0);
  } else if (rtIsNaN(b)) {
    y = b;
  } else {
    y = a * 1.4142135623730951;
  }

  return y;
}

void xgeqp3(double A[8], double tau[2], int jpvt[2])
{
	  int k;
	  int iy;
	  int i;
	  double work[2];
	  int i_i;
	  double temp;
	  int pvt;
	  double vn1[2];
	  double vn2[2];
	  int ix;
	  double atmp;
	  double temp2;
	  int i0;
	  int lastv;
	  int lastc;
	  int exitg1;
	  k = 1;
	  for (iy = 0; iy < 2; iy++) {
		jpvt[iy] = 1 + iy;
		work[iy] = 0.0;
		temp = xnrm2(A, k);
		vn2[iy] = temp;
		k += 4;
		vn1[iy] = temp;
	  }
	
	  for (i = 0; i < 2; i++) {
		i_i = i + (i << 2);
		pvt = (i + ixamax(2 - i, vn1, i + 1)) - 1;
		if (pvt + 1 != i + 1) {
		  ix = pvt << 2;
		  iy = i << 2;
		  for (k = 0; k < 4; k++) {
			temp = A[ix];
			A[ix] = A[iy];
			A[iy] = temp;
			ix++;
			iy++;
		  }
	
		  iy = jpvt[pvt];
		  jpvt[pvt] = jpvt[i];
		  jpvt[i] = iy;
		  vn1[pvt] = vn1[i];
		  vn2[pvt] = vn2[i];
		}
	
		atmp = A[i_i];
		temp2 = 0.0;
		temp = b_xnrm2(3 - i, A, i_i + 2);
		if (temp != 0.0) {
		  temp = rt_hypotd_snf(A[i_i], temp);
		  if (A[i_i] >= 0.0) {
			temp = -temp;
		  }
	
		  if (fabs(temp) < 1.0020841800044864E-292) {
			iy = 0;
			i0 = (i_i - i) + 4;
			do {
			  iy++;
			  for (k = i_i + 1; k < i0; k++) {
				A[k] *= 9.9792015476736E+291;
			  }
	
			  temp *= 9.9792015476736E+291;
			  atmp *= 9.9792015476736E+291;
			} while (!(fabs(temp) >= 1.0020841800044864E-292));
	
			temp = rt_hypotd_snf(atmp, b_xnrm2(3 - i, A, i_i + 2));
			if (atmp >= 0.0) {
			  temp = -temp;
			}
	
			temp2 = (temp - atmp) / temp;
			atmp = 1.0 / (atmp - temp);
			i0 = (i_i - i) + 4;
			for (k = i_i + 1; k < i0; k++) {
			  A[k] *= atmp;
			}
	
			for (k = 1; k <= iy; k++) {
			  temp *= 1.0020841800044864E-292;
			}
	
			atmp = temp;
		  } else {
			temp2 = (temp - A[i_i]) / temp;
			atmp = 1.0 / (A[i_i] - temp);
			i0 = (i_i - i) + 4;
			for (k = i_i + 1; k < i0; k++) {
			  A[k] *= atmp;
			}
	
			atmp = temp;
		  }
		}
	
		tau[i] = temp2;
		A[i_i] = atmp;
		if (i + 1 < 2) {
		  atmp = A[i_i];
		  A[i_i] = 1.0;
		  if (tau[0] != 0.0) {
			lastv = 4;
			iy = i_i + 3;
			while ((lastv > 0) && (A[iy] == 0.0)) {
			  lastv--;
			  iy--;
			}
	
			lastc = 1;
			iy = 5;
			do {
			  exitg1 = 0;
			  if (iy <= lastv + 4) {
				if (A[iy - 1] != 0.0) {
				  exitg1 = 1;
				} else {
				  iy++;
				}
			  } else {
				lastc = 0;
				exitg1 = 1;
			  }
			} while (exitg1 == 0);
		  } else {
			lastv = 0;
			lastc = 0;
		  }
	
		  if (lastv > 0) {
			if (lastc != 0) {
			  ix = i_i;
			  temp = 0.0;
			  for (iy = 5; iy <= lastv + 4; iy++) {
				temp += A[iy - 1] * A[ix];
				ix++;
			  }
	
			  work[0] = temp;
			}
	
			if (!(-tau[0] == 0.0)) {
			  pvt = 4;
			  k = 0;
			  iy = 1;
			  while (iy <= lastc) {
				if (work[k] != 0.0) {
				  temp = work[k] * -tau[0];
				  ix = i_i;
				  i0 = lastv + pvt;
				  for (iy = pvt; iy < i0; iy++) {
					A[iy] += A[ix] * temp;
					ix++;
				  }
				}
	
				k++;
				pvt += 4;
				iy = 2;
			  }
			}
		  }
	
		  A[i_i] = atmp;
		}
	
		iy = i + 2;
		while (iy < 3) {
		  if (vn1[1] != 0.0) {
			temp = fabs(A[4 + i]) / vn1[1];
			temp = 1.0 - temp * temp;
			if (temp < 0.0) {
			  temp = 0.0;
			}
	
			temp2 = vn1[1] / vn2[1];
			temp2 = temp * (temp2 * temp2);
			if (temp2 <= 1.4901161193847656E-8) {
			  vn1[1] = b_xnrm2(3 - i, A, i + 6);
			  vn2[1] = vn1[1];
			} else {
			  vn1[1] *= sqrt(temp);
			}
		  }
	
		  iy = 3;
		}
	  }
}


void polyfit(const double x[4], const double y[4], double p[2])
{
  int k;
  double V[8];
  double tau[2];
  int jpvt[2];
  double B[4];
  int i;
  double wj;
  for (k = 0; k < 4; k++) {
    V[4 + k] = 1.0;
    V[k] = x[k];
    B[k] = y[k];
  }

  xgeqp3(V, tau, jpvt);
  for (k = 0; k < 2; k++) {
    p[k] = 0.0;
    if (tau[k] != 0.0) {
      wj = B[k];
      for (i = k + 1; i + 1 < 5; i++) {
        wj += V[i + (k << 2)] * B[i];
      }

      wj *= tau[k];
      if (wj != 0.0) {
        B[k] -= wj;
        for (i = k + 1; i + 1 < 5; i++) {
          B[i] -= V[i + (k << 2)] * wj;
        }
      }
    }
  }

  for (i = 0; i < 2; i++) {
    p[jpvt[i] - 1] = B[i];
  }

  for (k = 1; k >= 0; k--) {
    p[jpvt[k] - 1] /= V[k + (k << 2)];
    i = 1;
    while (i <= k) {
      p[jpvt[0] - 1] -= p[jpvt[1] - 1] * V[4];
      i = 2;
    }
  }
}
//type 0: above 1.2V
//type 1: below 1.2V

void polyfit_cal(DRV_ADC_POLYFIT_CAL_TYPE polyfit_cal_type)
{

	unsigned int read_flag=0;
	if (polyfit_cal_type >= DRV_ADC_POLYFIT_CAL_MAX)
	{
		system_printf("DRV_ERR_INVALID_PARAM !!! polyfit_cal_type=%d\n",polyfit_cal_type);
		return ;
	}

	if (is_polyfit_above_1200 == 0 && polyfit_cal_type == DRV_ADC_POLYFIT_CAL_ABOVE_1200)
	{		
		uint8_t length = POLYFIT_ADC_VOLT_LEN;
		unsigned char tem[8]={0};
		uint8_t chip_type = hal_efuse_read(CHIP_TYPE_ADDR_EFUSE) & 0xF;
		if (chip_type<0x01 || (chip_type>0x7 && chip_type!=0x0A && chip_type!=0x0D)) //chip_type in efuse invalid
		{
			read_flag=spiFlash_OTP_Read(GET_CHIP_TYPE, CHIP_TYPE_LEN, (unsigned char *)&chip_type);
			if (chip_type<0x01 || (chip_type>0x7 && chip_type!=0x0A && chip_type!=0x0D))//chip_type in otp invalid
			{
				system_printf("chip_type in otp invalid\n");
				return;
			}
		}
		if((chip_type>0 && chip_type<0x8 )|| chip_type==0x0A|| chip_type==0x0D)
		{
			if(chip_type==0x02||chip_type==0x04||chip_type==0x06||chip_type==0x0A)//chip_type is sip
			{	
				read_flag=spiFlash_OTP_Read(POLYFIT_ADC_VOLT_ADDR_GD25Q80E_FLASH, length, (unsigned char *)tem);

				if(read_flag==0)//support flash otp
				{
					if(tem[0]==255 && tem[1]==255) 
					{
						system_printf("No Volt Points In Flash\n");
						return ;
					}
					else
					{
						uint16_t * volt_read = (uint16_t *)tem;
						int i;
						for(i=0;i<POLYFIT_POINT_NUM;++i)
						{
							polyfit_y[i] = volt_read[i];
						}
						polyfit(polyfit_x, polyfit_y, polyfit_var);		 

						is_polyfit_above_1200 = 1;					 
					}
				}
				else
				{	
					system_printf("Not support Flash\n");
					return ;
				}

			}
			else
			{	
				if(((hal_efuse_read(5) & 0xFFFF00)>>8)==0) 
				{
					system_printf("No Volt Points In efuse\n");
					return ;
				}
				else
				{
					polyfit_y[0] = (hal_efuse_read(5) & 0xFFFF00)>>8;
					polyfit_y[1] = (hal_efuse_read(5) >> 24) | ((hal_efuse_read(6) & 0xFF)<<8);
					polyfit_y[2] = (hal_efuse_read(6) & 0xFFFF00)>>8;
					polyfit_y[3] = (hal_efuse_read(6) >> 24) | ((hal_efuse_read(7) & 0xFF)<<8);
					polyfit(polyfit_x, polyfit_y, polyfit_var);		
					is_polyfit_above_1200 = 1;	
				}
			}
		}
	}
	else if(is_polyfit_below_1200 == 0 && polyfit_cal_type == DRV_ADC_POLYFIT_CAL_BELOW_1200)
	{
		
	}

}

/*******************************************************************************
 * Function: get_flash_efuse_volt_data
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

unsigned int get_flash_read_volt_data()
{
	uint16_t volt_point_y= 0;
	unsigned int read_flag=0;
	uint8_t length = 2;
	unsigned char tem[2]={0};
	uint8_t chip_type = hal_efuse_read(CHIP_TYPE_ADDR_EFUSE) & 0xF;
	
	if (chip_type<0x01 || (chip_type>0x7 && chip_type!=0x0A && chip_type!=0x0D)) //chip_type in efuse invalid
	{
		read_flag=spiFlash_OTP_Read(GET_CHIP_TYPE, CHIP_TYPE_LEN, (unsigned char *)&chip_type);
		if (chip_type<0x01 || (chip_type>0x7 && chip_type!=0x0A && chip_type!=0x0D))//chip_type in otp invalid
		{
			system_printf("chip_type in otp invalid\n");
			return 0;
		}
	}
	if((chip_type>0 &&chip_type<0x8 )|| chip_type==0x0A || chip_type==0x0D)
	{
		if(chip_type==0x02||chip_type==0x04||chip_type==0x06||chip_type==0x0A)//chip_type is sip
		{

			read_flag=spiFlash_OTP_Read(GET_TEMP_ADC_VOLT_ADDR_GD25Q80E_FLASH, length, (unsigned char *)tem);
			if(read_flag==0)//support flash otp
			{
				if(tem[0]==255 && tem[1]==255) 
				{
					system_printf("No Volt Points In Flash\n");
					return 0;
				}
				else
				{
					volt_point_y = (tem[1] << 8)|tem[0];
				}
			}
			else
			{	
				system_printf("Not support Flash\n");
				return 0;
			}
		}
		else
		{
			if(((hal_efuse_read(7) & 0xFFFF00)>>8)==0) 
			{
				system_printf("temp No Volt Points In efuse\n");
				return 0;
			}
			else
			{
				volt_point_y=(hal_efuse_read(7) & 0xFFFF00)>>8;
			}	
		}
	}
	return volt_point_y;
}



 int32_t hal_adc_clk_en(uint32_t clk_en)
 {
	uint32_t Rtn = 0;
	uint32_t flag = 0;
	uint32_t RegTmp = 0;

	if(clk_en >= DRV_ADC_BB_CLK_MAX)
	{
		return DRV_ERR_INVALID_PARAM;
	}

	RegTmp = IN32(SOC_BB_CLK_GEN);
	
    switch (clk_en)
	{
		case  DRV_ADC_BB_CLK_DIS:
			RegTmp &=~BIT3;// bit3:dig_aux_adc_clk_gen_en
			OUT32(SOC_BB_CLK_GEN, RegTmp);
			Rtn = DRV_SUCCESS;
			break;
			
		case DRV_ADC_BB_CLK_EN:
			RegTmp |= BIT3;// bit3:dig_aux_adc_clk_gen_en
			OUT32(SOC_BB_CLK_GEN, RegTmp);
			Rtn = DRV_SUCCESS;
			break;
			
		default:
			Rtn = DRV_ERR_INVALID_PARAM;
			break;
    }
	return Rtn;
 }

/*******************************************************************************
 * Function: temperature_sensor_read
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
 int16_t temperature_sensor_get(void)
 {
	uint32_t Rtn = 0;
	uint32_t flag = 0;
	uint32_t RegTmp = 0;
	int16_t  AdcOut = 0;
	int16_t  AdcOutTmp[ADC_CNT_MAX];
	uint32_t index = 0;
	
	DRV_ADC_PGA_0 tPga0;
	DRV_ADC_PGA_1 tPga1;
	DRV_ADC_PGA_2 tPga2;
	DRV_ADC_PGA_3 tPga3;

	memset(AdcOutTmp,0,10*sizeof(int16_t));
	
	tPga0.adc_pga0 = IN32(SOC_ADC_PGA_0);
	tPga1.adc_pga1 = IN32(SOC_ADC_PGA_1);
	tPga2.adc_pga2 = IN32(SOC_ADC_PGA_2);
	tPga3.adc_pga3 = IN32(SOC_ADC_PGA_3);
	
	//lock irq
   	flag = system_irq_save();

	//1.-----------------anolog config mux_A/mux_B--------------------------
	//1.1 temperature sensor en
	tPga0.dig_adc_pga0.dig_temp_sensor_en = 1;//enable temp input insignal
	tPga0.dig_adc_pga0.dig_input_signal_muxA_en = 1;
	tPga0.dig_adc_pga0.dig_input_signal_muxB_en = 1;
	OUT32(SOC_ADC_PGA_0, tPga0.adc_pga0);

	//1.2 mux_A,mux_B
	tPga1.dig_adc_pga1.dig_input_signal_muxA_ctrl = DRV_ADC_INPUT_TEMP_SENSOR;
	tPga1.dig_adc_pga1.dig_input_signal_muxB_ctrl = DRV_ADC_INPUT_REF_VOLT;
	tPga1.dig_adc_pga1.dig_input_signal_mux_vrefB_sel_en = 1;
	OUT32(SOC_ADC_PGA_1, tPga1.adc_pga1);

	//1.3 mux_B ref sel
	tPga2.dig_adc_pga2.dig_input_signal_mux_vrefB_sel = DRV_ADC_VREF_0_60;
	//2.----------------anolog Drv & ADC-----------------------------------
	//2.1 input enable
	tPga2.dig_adc_pga2.dig_input_driver_enable = 1; 
	//2.2 PGA by pass
	tPga2.dig_adc_pga2.dig_pga_bypass = 1;
	OUT32(SOC_ADC_PGA_2, tPga2.adc_pga2);

	//2.3 ADC enable
	tPga3.dig_adc_pga3.dig_sd_adc_enable = 1;
	OUT32(SOC_ADC_PGA_3, tPga3.adc_pga3);

	//3.-----------------open analog clk--------------------------------------
	hal_adc_clk_en(DRV_ADC_BB_CLK_EN);

	//4.-----------------digital----------------------------------------------
	//4.1 enable digital ADC
	RegTmp = IN32(SOC_ADC_CONFIG);
	RegTmp |= BIT4;//bit4:aux_adc_en
	OUT32(SOC_ADC_CONFIG, RegTmp);

	/*delay 100us*/
	delay(0xFFFF);
	//system_printf("-----------------ADC config complited!!!!---------------\n");

	//4.2 read Volt
	//AdcOut = IN32(SOC_ADC_OUT);
	#if 0
	AdcOut = hal_adc_get_oricode((int16_t)(IN32(SOC_ADC_OUT)));
	AdcOut = ((ADC_VREF_VOLT/2)-AdcOut*ADC_VREF_VOLT/4096)*2+(ADC_VREF_VOLT/2);
	#else
	for(index=0;index<ADC_CNT_MAX;index++)
	{
		AdcOutTmp[index] = hal_adc_get_oricode((int16_t)(IN32(SOC_ADC_OUT)));
		AdcOutTmp[index] = ((ADC_VREF_VOLT/2)-AdcOutTmp[index]*ADC_VREF_VOLT/4096)*2+(ADC_VREF_VOLT/2);
	}
	AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX); // mV
	#endif


	//5.disable clk before finish
    //hal_adc_clk_en(DRV_ADC_BB_CLK_DIS);
	
   //unlock irq 
	system_irq_restore(flag);

	return AdcOut;
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
int16_t vbat_sensor_get(uint32_t volt_div)
{
	uint32_t Rtn = 0;
	uint32_t flag = 0;
	uint32_t RegTmp = 0;
	int16_t  AdcOut = 0;
	int16_t  AdcOutTmp[ADC_CNT_MAX];
	uint32_t index = 0;
	DRV_ADC_PGA_0 tPga0 = {0};
	DRV_ADC_PGA_1 tPga1 = {0};
	DRV_ADC_PGA_2 tPga2 = {0};
	DRV_ADC_PGA_3 tPga3 = {0};

	if(volt_div >= DRV_ADC_INPUT_VOL_DIV_MAX)
	{
		return -199;//????
	}

	memset(AdcOutTmp,0,10*sizeof(int16_t));
	
	tPga0.adc_pga0 = IN32(SOC_ADC_PGA_0);
	tPga1.adc_pga1 = IN32(SOC_ADC_PGA_1);
	tPga2.adc_pga2 = IN32(SOC_ADC_PGA_2);
	tPga3.adc_pga3 = IN32(SOC_ADC_PGA_3);

	//lock irq
   	flag = system_irq_save();

	//1.-----------------anolog config mux_A/mux_B--------------------------
	//1.1 vbat sensor en
	tPga0.dig_adc_pga0.dig_vbat_enable = 1;//enable vbat input insignal
	tPga0.dig_adc_pga0.dig_vbat_select = volt_div;
	tPga0.dig_adc_pga0.dig_input_signal_muxA_en = 1;
	tPga0.dig_adc_pga0.dig_input_signal_muxB_en = 1;
	OUT32(SOC_ADC_PGA_0, tPga0.adc_pga0);

	//1.2 mux_A,mux_B
	tPga1.dig_adc_pga1.dig_input_signal_muxA_ctrl = DRV_ADC_INPUT_VBAT_SENSOR;
	tPga1.dig_adc_pga1.dig_input_signal_muxB_ctrl = DRV_ADC_INPUT_REF_VOLT;
	tPga1.dig_adc_pga1.dig_input_signal_mux_vrefB_sel_en = 1;
	OUT32(SOC_ADC_PGA_1, tPga1.adc_pga1);

	//1.3 mux_B ref sel
	tPga2.dig_adc_pga2.dig_input_signal_mux_vrefB_sel = DRV_ADC_VREF_0_60;
	//2.----------------anolog Drv & ADC-----------------------------------
	//2.1 input enable
	tPga2.dig_adc_pga2.dig_input_driver_enable = 1; 
	//2.2 PGA by pass
	tPga2.dig_adc_pga2.dig_pga_bypass = 1;
	OUT32(SOC_ADC_PGA_2, tPga2.adc_pga2);

	//2.3 ADC enable
	tPga3.dig_adc_pga3.dig_sd_adc_enable = 1;
	OUT32(SOC_ADC_PGA_3, tPga3.adc_pga3);

	//3.-----------------open analog clk--------------------------------------
	hal_adc_clk_en(DRV_ADC_BB_CLK_EN);

	//4.-----------------digital----------------------------------------------
	//4.1 enable digital ADC
	RegTmp = IN32(SOC_ADC_CONFIG);
	RegTmp |= BIT4;//bit4:aux_adc_en
	OUT32(SOC_ADC_CONFIG, RegTmp);

	/*delay 100us*/
	delay(0xFFFF);

	//4.2 read Volt--------??????????
	for(index=0;index<ADC_CNT_MAX;index++)
	{
		AdcOutTmp[index] = hal_adc_get_oricode((int16_t)(IN32(SOC_ADC_OUT)));
		AdcOutTmp[index] = ((ADC_VREF_VOLT/2)-AdcOutTmp[index]*ADC_VREF_VOLT/4096)*2+(ADC_VREF_VOLT/2);
	}

#if 0
    #if 0
    if(volt_div == DRV_ADC_INPUT_VOL_DIV_1)
    {
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_1; // mV
    }
    #endif
	if(volt_div == DRV_ADC_INPUT_VOL_DIV_4)
	{
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_4;
	}
	else if(volt_div == DRV_ADC_INPUT_VOL_DIV_5)
	{
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_5;
	}
	else if(volt_div == DRV_ADC_INPUT_VOL_DIV_6)
	{
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_6;
	}
	else
	{

	}
#endif

	
#if 0
    if(volt_div == DRV_ADC_INPUT_VOL_DIV_1)
    {
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_1; // mV
    }
	else if(volt_div == DRV_ADC_INPUT_VOL_DIV_4)
	{
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_4;
	}
	else if(volt_div == DRV_ADC_INPUT_VOL_DIV_5)
	{
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_5;
	}
	else if(volt_div == DRV_ADC_INPUT_VOL_DIV_6)
	{
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_6;
	}
	else
	{

	}
#else
 	   if(volt_div == DRV_ADC_INPUT_VOL_DIV_4)
		{
			AdcOut = (((sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_4)-ADC_DIV_4_PARAM_OFFSET)*1000/ADC_DIV_4_PARAM_SLOPE;
		}
		else if(volt_div == DRV_ADC_INPUT_VOL_DIV_5)
		{
			AdcOut = (((sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_5)-ADC_DIV_5_PARAM_OFFSET)*1000/ADC_DIV_5_PARAM_SLOPE;
		}
		else if(volt_div == DRV_ADC_INPUT_VOL_DIV_6)
		{
			AdcOut = (((sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_6)-ADC_DIV_6_PARAM_OFFSET)*1000/ADC_DIV_6_PARAM_SLOPE;
		}
		else
		{

		}
#endif

	//5.disable clk before finish
    //hal_adc_clk_en(DRV_ADC_BB_CLK_DIS);
	if(AdcOut <= ADC_INPUT_VOLT_MIN)
	{	
		/*
		if (!is_polyfit_below_1200)
		{
			polyfit_cal(DRV_ADC_POLYFIT_CAL_BELOW_1200);			
		}
		AdcOut = (AdcOut-polyfit_var[1])/polyfit_var[0];
		*/
	}
	else
	{
		if (!is_polyfit_above_1200)
		{
			polyfit_cal(DRV_ADC_POLYFIT_CAL_ABOVE_1200);			
		}
		AdcOut = (AdcOut-polyfit_var[1])/polyfit_var[0];
	}
   	//unlock irq 
	system_irq_restore(flag);

	return AdcOut;
}

/*******************************************************************************
 * Function: adc_get_temp_type
 * Description: 
 * Parameters: 
 *   Input: 
 *
 *   Output: tempature status
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
DRV_ADC_TEMP_TYPE hal_adc_get_temp_type()
{	
	uint16_t volt_25=0;
	int16_t temp_to_volt_now = temperature_sensor_get(); 
	//system_printf("temp_to_volt_now=%d\n",temp_to_volt_now);
	if(temp_to_volt_now <= ADC_INPUT_VOLT_MIN)
	{
		volt_25=get_flash_read_volt_data();
	//	system_printf("volt_25=%d\n",volt_25);
	}
	else
	{
		return DRV_ADC_TEMP_HIGH;
	}
	
	if((volt_25>=100)&&(volt_25<=1500))
	{
		if(temp_to_volt_now<(volt_25-(ADC_TEMP_SENSOR_TO_TEMP*(25-ADC_TEMP_LOW))))
		{
			return DRV_ADC_TEMP_LOW;
		}
		else if((temp_to_volt_now>=(volt_25-(ADC_TEMP_SENSOR_TO_TEMP*(25-ADC_TEMP_LOW))))&&(temp_to_volt_now<=(volt_25-(ADC_TEMP_SENSOR_TO_TEMP*(25-ADC_TEMP_NORMAL_TEM)))))
		{
			return DRV_ADC_TEMP_NORMAL;
		}
		else 
		{		
			system_printf("--Currently at High Temperature Status--\n");
			return DRV_ADC_TEMP_HIGH;
		}
	}
	else
	{
		xTimerStop(hal_temp_Timer, 100);
		return DRV_ADC_TEMP_NORMAL;
	}
}

static void hal_check_temp_status(TimerHandle_t xTimer)
{	
	temp_status=hal_adc_get_temp_type();
	
}

void time_check_temp()
{
	hal_temp_Timer = xTimerCreateStatic("xTemp_sensor",
						Time_to_check_Temp,
						pdTRUE,
						( void * ) 0,
						hal_check_temp_status,
						&Timer_temp);
	xTimerStart(hal_temp_Timer, 100);
}

/*******************************************************************************
 * Function: 
 * Description: 
 * Parameters: 
 *   Input: tout_num:DRV_ADC_INPUT_SEL
 *          volt_div:DRV_ADC_INPUT_VOL_DIV
 *          volt:
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
 #if 1
 int16_t tout_sensor_get(uint32_t tout_num,uint32_t volt_div,uint32_t volt)
 #else
 int16_t tout_sensor_get(uint32_t tout_num,uint32_t volt_div)
 #endif
{
	if(volt <= ADC_INPUT_VOLT_MIN)
	{
		//polyfit_cal(DRV_ADC_POLYFIT_CAL_BELOW_1200);
	}
	else
	{
		polyfit_cal(DRV_ADC_POLYFIT_CAL_ABOVE_1200);
	}
	uint32_t Rtn = 0;
	uint32_t flag = 0;
	uint32_t RegTmp = 0;
	int16_t  AdcOut = 0;
	int16_t  AdcOutTmp[ADC_CNT_MAX];
	uint32_t index = 0;

	DRV_ADC_PGA_0 tPga0 = {0};
	DRV_ADC_PGA_1 tPga1 = {0};
	DRV_ADC_PGA_2 tPga2 = {0};
	DRV_ADC_PGA_3 tPga3 = {0};

	if(volt_div >= DRV_ADC_INPUT_VOL_DIV_MAX \
	   || tout_num == DRV_ADC_INPUT_VBAT_SENSOR \
	   || tout_num == DRV_ADC_INPUT_TEMP_SENSOR \
	   || tout_num == DRV_ADC_INPUT_REF_VOLT \
	   || tout_num >= DRV_ADC_INPUTL_MAX)
	{
		return -199;//????
	}

	memset(AdcOutTmp,0,10*sizeof(int16_t));

	tPga0.adc_pga0 = IN32(SOC_ADC_PGA_0);
	tPga1.adc_pga1 = IN32(SOC_ADC_PGA_1);
	tPga2.adc_pga2 = IN32(SOC_ADC_PGA_2);
	tPga3.adc_pga3 = IN32(SOC_ADC_PGA_3);

	

	//lock irq
   	flag = system_irq_save();

	//1.-----------------anolog config mux_A/mux_B--------------------------
	switch (tout_num)
	{
		#ifdef _USR_TR6260
		case  DRV_ADC_INPUT_TOUT0:
			//1.1 vbat sensor en
			tPga0.dig_adc_pga0.dig_gpio1_enable = 1;//
			#if 0
			if(volt_div == DRV_ADC_INPUT_VOL_DIV_1)
			#else
			if(volt <= ADC_INPUT_VOLT_MIN)
			#endif
			{
				tPga0.dig_adc_pga0.dig_gpio1_res_divider_bypass = 1; 
			}
			else
			{
				tPga0.dig_adc_pga0.dig_gpio1_res_divider_bypass = 0; 
			}
			tPga0.dig_adc_pga0.dig_gpio1_volt_select = volt_div;
			tPga0.dig_adc_pga0.dig_input_signal_muxA_en = 1;
			tPga0.dig_adc_pga0.dig_input_signal_muxB_en = 1;
			OUT32(SOC_ADC_PGA_0, tPga0.adc_pga0);
			
			//1.2 mux_A,mux_B
			tPga1.dig_adc_pga1.dig_input_signal_muxA_ctrl = DRV_ADC_INPUT_TOUT0;
			tPga1.dig_adc_pga1.dig_input_signal_muxB_ctrl = DRV_ADC_INPUT_REF_VOLT;
			tPga1.dig_adc_pga1.dig_input_signal_mux_vrefB_sel_en = 1;
			OUT32(SOC_ADC_PGA_1, tPga1.adc_pga1);			
			break;
			
		case  DRV_ADC_INPUT_TOUT1:			
			//1.1 vbat sensor en
			tPga0.dig_adc_pga0.dig_gpio2_enable = 1;//enable TOUT1 input insignal
			#if 0
			if(volt_div == DRV_ADC_INPUT_VOL_DIV_1)
			#else
			if(volt <= ADC_INPUT_VOLT_MIN)
			#endif
			{
				tPga0.dig_adc_pga0.dig_gpio2_res_divider_bypass = 1; 
			}
			else
			{
				tPga0.dig_adc_pga0.dig_gpio2_res_divider_bypass = 0; 
			}

			tPga0.dig_adc_pga0.dig_gpio2_volt_select = volt_div;
			tPga0.dig_adc_pga0.dig_input_signal_muxA_en = 1;
			tPga0.dig_adc_pga0.dig_input_signal_muxB_en = 1;
			OUT32(SOC_ADC_PGA_0, tPga0.adc_pga0);
				
			//1.2 mux_A,mux_B
			tPga1.dig_adc_pga1.dig_input_signal_muxA_ctrl = DRV_ADC_INPUT_TOUT1;
			tPga1.dig_adc_pga1.dig_input_signal_muxB_ctrl = DRV_ADC_INPUT_REF_VOLT;
			tPga1.dig_adc_pga1.dig_input_signal_mux_vrefB_sel_en = 1;
			OUT32(SOC_ADC_PGA_1, tPga1.adc_pga1);		
			break;
		#endif

		case  DRV_ADC_INPUT_TOUT2:
			//1.1 vbat sensor en
			tPga0.dig_adc_pga0.dig_gpio3_enable = 1;//enable TOUT2 input insignal
			#if 0
			if(volt_div == DRV_ADC_INPUT_VOL_DIV_1)
			#else
			if(volt <= ADC_INPUT_VOLT_MIN)
			#endif
			{
				tPga0.dig_adc_pga0.dig_gpio3_res_divider_bypass = 1; 
			}
			else
			{
				tPga0.dig_adc_pga0.dig_gpio3_res_divider_bypass = 0; 
			}

			tPga0.dig_adc_pga0.dig_gpio3_volt_select = volt_div;
			tPga0.dig_adc_pga0.dig_input_signal_muxA_en = 1;
			tPga0.dig_adc_pga0.dig_input_signal_muxB_en = 1;
			OUT32(SOC_ADC_PGA_0, tPga0.adc_pga0);
					
			//1.2 mux_A,mux_B
			tPga1.dig_adc_pga1.dig_input_signal_muxA_ctrl = DRV_ADC_INPUT_TOUT2;
			tPga1.dig_adc_pga1.dig_input_signal_muxB_ctrl = DRV_ADC_INPUT_REF_VOLT;
			tPga1.dig_adc_pga1.dig_input_signal_mux_vrefB_sel_en = 1;
			OUT32(SOC_ADC_PGA_1, tPga1.adc_pga1);		
			break;
			
		case  DRV_ADC_INPUT_TOUT3:
			//1.1 vbat sensor en
			tPga0.dig_adc_pga0.dig_gpio4_enable = 1;//enable TOUT3 input insignal
			#if 0
			if(volt_div == DRV_ADC_INPUT_VOL_DIV_1)
			#else
			if(volt <= ADC_INPUT_VOLT_MIN)
			#endif
			{
				tPga0.dig_adc_pga0.dig_gpio4_res_divider_bypass = 1; 
			}
			else
			{
				tPga0.dig_adc_pga0.dig_gpio4_res_divider_bypass = 0; 
			}

			tPga0.dig_adc_pga0.dig_gpio4_volt_select = volt_div;
			tPga0.dig_adc_pga0.dig_input_signal_muxA_en = 1;
			tPga0.dig_adc_pga0.dig_input_signal_muxB_en = 1;
			OUT32(SOC_ADC_PGA_0, tPga0.adc_pga0);
						
			//1.2 mux_A,mux_B
			tPga1.dig_adc_pga1.dig_input_signal_muxA_ctrl = DRV_ADC_INPUT_TOUT3;
			tPga1.dig_adc_pga1.dig_input_signal_muxB_ctrl = DRV_ADC_INPUT_REF_VOLT;
			tPga1.dig_adc_pga1.dig_input_signal_mux_vrefB_sel_en = 1;
			OUT32(SOC_ADC_PGA_1, tPga1.adc_pga1);		
			break;
		default:
			break;
    }

	//1.3 mux_B ref sel
	tPga2.dig_adc_pga2.dig_input_signal_mux_vrefB_sel = DRV_ADC_VREF_0_60;
	//2.----------------anolog Drv & ADC-----------------------------------
	//2.1 input enable
	tPga2.dig_adc_pga2.dig_input_driver_enable = 1; 
	//2.2 PGA by pass
	tPga2.dig_adc_pga2.dig_pga_bypass = 1;
	OUT32(SOC_ADC_PGA_2, tPga2.adc_pga2);

	//2.3 ADC enable
	tPga3.dig_adc_pga3.dig_sd_adc_enable = 1;
	OUT32(SOC_ADC_PGA_3, tPga3.adc_pga3);

	//3.-----------------open analog clk--------------------------------------
	hal_adc_clk_en(DRV_ADC_BB_CLK_EN);

	//4.-----------------digital----------------------------------------------
	//4.1 enable digital ADC
	RegTmp = IN32(SOC_ADC_CONFIG);
	RegTmp |= BIT4;//bit4:aux_adc_en
	OUT32(SOC_ADC_CONFIG, RegTmp);

	/*delay 100us*/
	delay(0xFFFF);

	//4.2 read Volt--------??????????
	//AdcOut = IN32(SOC_ADC_OUT);
	#if 0
	AdcOut = hal_adc_get_oricode((int16_t)(IN32(SOC_ADC_OUT)));
	AdcOut = ((ADC_VREF_VOLT/2)-AdcOut*ADC_VREF_VOLT/4096)*2+(ADC_VREF_VOLT/2);
	#else
	for(index=0;index<ADC_CNT_MAX;index++)
	{
		AdcOutTmp[index] = hal_adc_get_oricode((int16_t)(IN32(SOC_ADC_OUT)));
		AdcOutTmp[index] = ((ADC_VREF_VOLT/2)-AdcOutTmp[index]*ADC_VREF_VOLT/4096)*2+(ADC_VREF_VOLT/2);
	}

    #if 0
    if(volt_div == DRV_ADC_INPUT_VOL_DIV_1)
    {
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_1; // mV
    }
	else if(volt_div == DRV_ADC_INPUT_VOL_DIV_4)
	{
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_4;
	}
	else if(volt_div == DRV_ADC_INPUT_VOL_DIV_5)
	{
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_5;
	}
	else if(volt_div == DRV_ADC_INPUT_VOL_DIV_6)
	{
		AdcOut = (sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_6;
	}
	else
	{

	}
	#else
	if(volt <= ADC_INPUT_VOLT_MIN)
	{
		AdcOut = ((sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)-ADC_VOLT_MIN_PARAM_OFFSET)/ADC_VOLT_MIN_PARAM_SLOPE;
	}
	else
	{
		if(volt_div == DRV_ADC_INPUT_VOL_DIV_4)
		{
			AdcOut = (((sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_4)-ADC_DIV_4_PARAM_OFFSET)*1000/ADC_DIV_4_PARAM_SLOPE;
		}
		else if(volt_div == DRV_ADC_INPUT_VOL_DIV_5)
		{
			AdcOut = (((sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_5)-ADC_DIV_5_PARAM_OFFSET)*1000/ADC_DIV_5_PARAM_SLOPE;
		}
		else if(volt_div == DRV_ADC_INPUT_VOL_DIV_6)
		{
			AdcOut = (((sum(AdcOutTmp,ADC_CNT_MAX)/ADC_CNT_MAX)*ADC_INPUT_VOLT_DIV_6)-ADC_DIV_6_PARAM_OFFSET)*1000/ADC_DIV_6_PARAM_SLOPE;
		}
		else
		{

		}
	}
    #endif

	#endif

	//5.disable clk before finish
    //hal_adc_clk_en(DRV_ADC_BB_CLK_DIS);
	
   //unlock irq 
	
	if(volt <= ADC_INPUT_VOLT_MIN && is_polyfit_below_1200)
	{		
			
	}
	else if(volt > ADC_INPUT_VOLT_MIN && is_polyfit_above_1200)
	{
		AdcOut = (AdcOut-polyfit_var[1])/polyfit_var[0];
	}
	system_irq_restore(flag);

	return AdcOut;
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
#ifdef _USE_TEST_CMD_ADC
static int16_t test_adc_tout(uint32_t tout_num)
{
	int16_t rnt = 0;
	
	rnt = tout_sensor_get(tout_num,DRV_ADC_INPUT_VOL_DIV_4,ADC_INPUT_VOLT_MAX);

	return rnt;
}

 static int cmd_adc(cmd_tbl_t *t, int argc, char *argv[])
 {
	 uint16_t adc_temp = 0;
	 
	 if (strcmp(argv[1], "temp") == 0) 
	 {

		 adc_temp = temperature_sensor_get();
		 system_printf("Temp Sensor is %d mV\n",adc_temp);
		 
		 return CMD_RET_SUCCESS;
	 } 
	 else if (strcmp(argv[1], "vbat") == 0) 
	 {
         adc_temp = vbat_sensor_get(DRV_ADC_INPUT_VOL_DIV_4);
		 system_printf("ADC Sensor is %d mV\n",adc_temp);
		 
		 return CMD_RET_SUCCESS;
	 }
	 else if (strcmp(argv[1], "tout0") == 0) 
	 {
	 	//set GPIO19 to ADC function
	    PIN_FUNC_SET(IO_MUX0_GPIO19, FUNC_GPIO19_TOUT0);
		
         if(strcmp(argv[2], "0") == 0)
         {
		 	adc_temp = tout_sensor_get(DRV_ADC_INPUT_TOUT0,DRV_ADC_INPUT_VOL_DIV_4,ADC_INPUT_VOLT_MIN);
		 	system_printf("TOUT0 is %d mV\n",adc_temp);
         }
		 else if(strcmp(argv[2], "1") == 0)
		 {
			adc_temp = tout_sensor_get(DRV_ADC_INPUT_TOUT0,DRV_ADC_INPUT_VOL_DIV_4,ADC_INPUT_VOLT_MAX);
		 	system_printf("TOUT0 is %d mV\n",adc_temp);
		 }
		 else
		 {
		 }
		 return CMD_RET_SUCCESS;
	 }
	 else if (strcmp(argv[1], "tout1") == 0) 
	 {
	 	//set GPIO18 to ADC function
	    PIN_FUNC_SET(IO_MUX0_GPIO18, FUNC_GPIO18_TOUT1);
		
         if(strcmp(argv[2], "0") == 0)
         {
		 	adc_temp = tout_sensor_get(DRV_ADC_INPUT_TOUT1,DRV_ADC_INPUT_VOL_DIV_4,ADC_INPUT_VOLT_MIN);
		 	system_printf("TOUT1 is %d mV\n",adc_temp);
         }
		 else if(strcmp(argv[2], "1") == 0)
		 {
			adc_temp = tout_sensor_get(DRV_ADC_INPUT_TOUT1,DRV_ADC_INPUT_VOL_DIV_4,ADC_INPUT_VOLT_MAX);
		 	system_printf("TOUT1 is %d mV\n",adc_temp);
		 }
		 else
		 {
		 }
		 
		 return CMD_RET_SUCCESS;
	 }
	 else if (strcmp(argv[1], "tout2") == 0) 
	 {
	 	//set GPIO18 to ADC function
	    PIN_FUNC_SET(IO_MUX0_GPIO14, FUNC_GPIO14_1_TOUT2);
		
         if(strcmp(argv[2], "0") == 0)
         {
		 	adc_temp = tout_sensor_get(DRV_ADC_INPUT_TOUT2,DRV_ADC_INPUT_VOL_DIV_4,ADC_INPUT_VOLT_MIN);
		 	system_printf("TOUT2 is %d mV\n",adc_temp);
         }
		 else if(strcmp(argv[2], "1") == 0)
		 {
			adc_temp = tout_sensor_get(DRV_ADC_INPUT_TOUT2,DRV_ADC_INPUT_VOL_DIV_4,ADC_INPUT_VOLT_MAX);
		 	system_printf("TOUT2 is %d mV\n",adc_temp);
		 }
		 else
		 {
		 }
		 
		 return CMD_RET_SUCCESS;
	 }
	 else if (strcmp(argv[1], "tout3") == 0) 
	 {
	 	//set GPIO18 to ADC function
	    PIN_FUNC_SET(IO_MUX0_GPIO15, FUNC_GPIO15_1_TOUT3);
		
         if(strcmp(argv[2], "0") == 0)
         {
		 	adc_temp = tout_sensor_get(DRV_ADC_INPUT_TOUT3,DRV_ADC_INPUT_VOL_DIV_4,ADC_INPUT_VOLT_MIN);
		 	system_printf("TOUT3 is %d mV\n",adc_temp);
         }
		 else if(strcmp(argv[2], "1") == 0)
		 {
			adc_temp = tout_sensor_get(DRV_ADC_INPUT_TOUT3,DRV_ADC_INPUT_VOL_DIV_4,ADC_INPUT_VOLT_MAX);
		 	system_printf("TOUT3 is %d mV\n",adc_temp);
		 }
		 else
		 {
		 }
		 
		 return CMD_RET_SUCCESS;
	 }
	 else
	 {
		 system_printf("INVALID CMD!!!!\n");
		 return CMD_RET_FAILURE;
	 }
 }
 
  SUBCMD(set,
	 adc,
	 cmd_adc,
	 "ADC Temp test",
	 "ADC Vbat test");


#endif /* _USE_TEST_CMD */




/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: drv_rtc.h   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        yanhaijian
 * Date:          2019-3-14
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef _DRV_RTC_H
#define _DRV_RTC_H

/****************************************************************************
* 	                                        Include files
****************************************************************************/
#include <stdarg.h>
#include "system_common.h"
#include "soc_top_reg.h"

/****************************************************************************
* 	                                        Macros
****************************************************************************/
#define RTC_BASE			0x601100
	
#define RTC_32K_CNT_REG			RTC_BASE+0x04
#define RTC_CNT_REG					RTC_BASE+0x10
#define RTC_ALARM_REG				RTC_BASE+0x14
#define RTC_CTRL_REG				RTC_BASE+0x18
#define RTC_STATUS_REG				RTC_BASE+0x1C
	
#define RTC_CNT_SEC(x)				((x)<<0)
#define RTC_CNT_MIN(x)				((x)<<6)
#define RTC_CNT_HOUR(x)				((x)<<12)
#define RTC_CNT_DAY(x)				((x)<<17)
	
#define RTC_CNT_GET_SEC(x)			(((x)>>0) & 0x3F)
#define RTC_CNT_GET_MIN(x)			(((x)>>6) & 0x3F)
#define RTC_CNT_GET_HOUR(x)			(((x)>>12) & 0x1F)
#define RTC_CNT_GET_DAY(x)			(((x)>>17) & 0x7FFF)
	
#define RTC_ALARM_32K(x)			((x)<<0)
#define RTC_ALARM_SEC(x)			((x)<<15)
#define RTC_ALARM_MIN(x)			((x)<<21)
#define RTC_ALARM_HOUR(x)			((x)<<27)
	
#define RTC_ALARM_GET_32K(x)		((x) & 0x7fff)
#define RTC_ALARM_GET_SEC(x)		(((x)& 0x1f8000) >> 15 )
#define RTC_ALARM_GET_MIN(x)		(((x) & 0x7e00000) >> 21)
#define RTC_ALARM_GET_HOUR(x)		(((x) & 0xf8000000)>> 27)
	
#define RTC_ALARM_SET_SEC(x)		((x) << 15)
	
#define RTC_CTRL_EN(x)				((x)<<0)
#define RTC_CTRL_WAKEUP(x)			((x)<<1)
#define RTC_CTRL_INT(x)				((x)<<2)
#define RTC_CTRL_DAY(x)				((x)<<3)
#define RTC_CTRL_HOUR(x)			((x)<<4)
#define RTC_CTRL_MIN(x)				((x)<<5)
#define RTC_CTRL_SEC(x)				((x)<<6)
#define RTC_CTRL_HSEC(x)			((x)<<7)
	
	
#define RTC_STATUS_GET_INT(x)		(((x)>>2)&1)
#define RTC_STATUS_GET_DAY(x)		(((x)>>3)&1)
#define RTC_STATUS_GET_HOUR(x)		(((x)>>4)&1)
#define RTC_STATUS_GET_MIN(x)		(((x)>>5)&1)
#define RTC_STATUS_GET_SEC(x)		(((x)>>6)&1)
#define RTC_STATUS_GET_HSEC(x)		(((x)>>7)&1)
#define RTC_STATUS_GET_WD(x)		(((x)>>14)&1)
	
	
#define RTC_GET_WD_BIT()			(RTC_STATUS_GET_WD(IN32(RTC_STATUS_REG)))
//#define RTC_GET_WD_BIT()			((inw(RTC_STATUS_REG)>>14)&1)
#define RTC_READY_TO_WR()			while(RTC_GET_WD_BIT() == 0)
	
#define   ALARM_CLK					32768
#define  HOUR_TO_MS(X)  			((X) * 1000 * 60 * 60)
#define  MIN_TO_MS(X)    			((X) * 1000 * 60)
#define  SEC_TO_MS(X)    			((X) * 1000)
#define  clk32K_TO_MS(X)    		((((X) & 0x7fff) * 1000) / ALARM_CLK)
	
#define	MS_TO_ALARM_CNT(X)			(((X) * ALARM_CLK) / 1000)


/****************************************************************************
* 	                                        Types
****************************************************************************/
struct	alarm_config{
		uint16_t	millisecond;
		uint8_t second;
		uint8_t minute;
		uint8_t hour;
};
	
typedef struct
{
		int32_t	  ms;
		int8_t    tm_sec; /* 秒 – 取值区间为[0,59] */
		int8_t    tm_min; /* 分 - 取值区间为[0,59] */
		int8_t    tm_hour; /* 时 - 取值区间为[0,23] */
}rtc_time;
	
typedef struct 
{
		uint8_t    tm_mday; /* 一个月中的日期 - 取值区间为[1,31] */
		uint8_t    tm_mon; /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */
		uint16_t  tm_year; /* 年份，其值等于实际年份减去1900 */
		uint8_t    tm_wday; /* 星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推 */
}date;
	
struct tm {
		uint8_t tm_sec; /* 秒 – 取值区间为[0,59] */
		uint8_t tm_min; /* 分 - 取值区间为[0,59] */
		uint8_t tm_hour; /* 时 - 取值区间为[0,23] */
		uint8_t tm_mday; /* 一个月中的日期 - 取值区间为[1,31] */
		uint8_t tm_mon; /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */
		uint16_t tm_year; /* 年份，其值等于实际年份减去1900 */
		uint8_t tm_wday; /* 星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推 */
};

typedef struct{
    rtc_time   rtc_time;
    uint8_t    rtcNeedUpdate;
}net_time;  

/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/
void rtc_wakeup_set(int sec_time);
rtc_time  rtc_read_time(void);
void rtc_set_time(struct tm sysTime);
uint32_t rtc_set_alarm(uint32_t value, int8_t  setFlag);
int32_t rtc_get_alarm(void);
//void ts8266_rtc_resume(void );
void rtc_stop_alarm(void);
uint32_t rtc_get_time_of_day(void);
struct tm  rtc_get_system_time(void);
unsigned int rtc_get_32K_cnt(void);
int  rtc_get_interval_cnt(int  pre_rtc, int now_rtc);
int hal_rtc_allow_psm(void);
void hal_rtc_init(void);
uint32_t rtc_get_time_tuya(void);
void rtc_set_time_tuya(uint32_t time);

typedef unsigned long rtc_base_t;
// typedef unsigned long rtc_time_t;
typedef uint64_t rtc_date_t;

#define RTC_TIMER_NAME_MAX_LEN 10		   // 定时器名字最大长度
#define RTC_TIMER_DAY_TO_TICK 24 * 60 * 60 // 一天的计数值
#define RTC_TIMER_HOUR_TO_TICK 60 * 60	 // 一小时的计数值
#define RTC_TIMER_MIN_TO_TICK 60		   // 一分钟的计数值

enum rtc_timer_flag
{
	RTC_TIMER_FLAG_ONE_SHOT,
	RTC_TIMER_FLAG_PERIODIC,
	RTC_TIMER_FLAG_MAX
};

enum rtc_timer_command
{
	RTC_TIMER_CTRL_SET_ONE_SHOT,  // 修改定时器为单次
	RTC_TIMER_CTRL_SET_PERIODIC,  // 修改定时器为周期性
	RTC_TIMER_CTRL_SET_NAME,	  // 设置定时器名字
	RTC_TIMER_CTRL_SET_PARAMETER, // 设置定时器回调函数的参数
	RTC_TIMER_CTRL_MAX
};

enum rtc_timer_type
{
	RTC_TIMER_TYPE_COUNT_DOWN, // 倒计时：时分秒天
	// RTC_TIMER_TYPE_TARGET_YEAR,  // 目标是年：一年一个周期
	RTC_TIMER_TYPE_TARGET_DAY,   // 目标是天：一天一个周期
	RTC_TIMER_TYPE_TARGET_MONTH, // 目标是月：一个月一个周期
	// RTC_TIMER_TYPE_TARGET_WEEK, // 一周一个周期
	// RTC_TIMER_TYPE_TARGET_HOUR, 	// 目标是小时：一小时一个周期
	// RTC_TIMER_TYPE_TARGET_MIN,		// 目标是分钟：一分钟一个周期
	// RTC_TIMER_TYPE_TARGET_SEC,		// 目标是秒
	RTC_TIMER_TYPE_MAX
};

enum rtc_timer_status
{
	RTC_TIMER_STATUS_INACTIVE, // 刚create，没有start
	// RTC_TIMER_STATUS_ACTIVE,
	RTC_TIMER_STATUS_READY,   // start之后就是ready状态
	RTC_TIMER_STATUS_RUNNING, // 该定时器正在running
	RTC_TIMER_STATUS_STOP,	// stop
	RTC_TIMER_STATUS_SUSPEND, // 倒计时定时器挂起
	RTC_TIMER_STATUS_MAX
};

#pragma  pack(push)  //保存对齐状态
#pragma  pack(1)
typedef struct rtc_timer_s
{
	char name[RTC_TIMER_NAME_MAX_LEN]; // 定时器名字
	// rtc_base_t countvalue;
	// rtc_base_t reloadvalue;
	// rtc_base_t enterIntvalue;

	// bool IsStarted;
	// bool IsNext2Expire;
	void (*timeout)(void *parameter);
	void *parameter;
	enum rtc_timer_type type;
	enum rtc_timer_flag flag;
	enum rtc_timer_status status;

	rtc_date_t set_date_remain;// 倒计时的长度
	rtc_date_t set_date;
	rtc_date_t aim_date; // bit14-bit0 : ms
						 // bit20-bit15: sec
						 // bit26-bit21: min
						 // bit31-bit27: hour
						 // bit32-bit36: day
						 // bit37-bit40: month
						 // bit40-...  : year

	// struct rtc_timer_s *next;

	struct rtc_timer_s *next;
    struct rtc_timer_s *prev;
} rtc_timer_t;
#pragma  pack(pop)

// 中断
void rtc_timer_isr(void);

// 新建date变量
rtc_date_t rtc_timer_date_create(rtc_date_t year, rtc_date_t month, rtc_date_t day,
								 rtc_date_t hour, rtc_date_t min, rtc_date_t sec, rtc_date_t ms);

/**
 * @brief    新建定时器
 *
 * @param name: 定时器的名字
 * @param timeout: 回调函数
 * @param parameter:回调函数传参
 * @param date: 定时器日期
 * @param type: 定时器类型
 * @param flag: 定时器周期
 * @return    : 返回定时器句柄
 */
rtc_timer_t *rtc_timer_create(const char *name,
							  void (*timeout)(void *parameter),
							  void *parameter,
							  rtc_date_t date,
							  enum rtc_timer_type type,
							  enum rtc_timer_flag flag);

/**
 * @brief    开始定时器
 *
 * @param timer: 定时器的句柄
 */
bool rtc_timer_start(rtc_timer_t *timer);

/**
 * @brief    关闭定时器
 *
 * @param timer: 定时器的句柄
 */
void rtc_timer_stop(rtc_timer_t *timer);

/**
 * @brief    挂起定时器
 *
 * @param timer: 定时器的句柄
 */
void rtc_timer_suspend(rtc_timer_t *timer);

/**
 * @brief    唤醒定时器
 *
 * @param timer: 定时器的句柄
 */
void rtc_timer_resume(rtc_timer_t *timer);

/**
 * @brief    删除定时器
 *
 * @param timer: 定时器的句柄
 */
void rtc_timer_delete(rtc_timer_t *timer);

// 修改系统时间前的准备工作
void rtc_timer_systime_change_pre(void);

// 修改系统时间后的处理工作
void rtc_timer_systime_change_after(void);

// 保存定时器链表
void rtc_timer_list_nv_save(void);

// 读取定时器链表
void rtc_timer_list_nv_read(void);


#endif/*_DRV_RTC_H*/



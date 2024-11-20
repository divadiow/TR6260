#include <nds32_intrinsic.h>
#include "system.h"
#include "drv_rtc.h"
#include "system_common.h"
#include <stdarg.h>
#include <stdio.h>
#include "easyflash.h"

#define LEAP_YAER_SEC 		(366*24*3600)
#define NOLEAP_YAER_SEC 	(365*24*3600)

date sys_global_time = {0};
const char monthDays[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const char *week[7] = {"Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat"};
const uint8_t febofleapYear = 29;
bool alarmSetFlag;
net_time g_net_time;
bool leapyear(uint16_t year)
{
	if (year % 100)
		if ((year % 4) == 0)
			return true;
		else
			return false;
	else if ((year % 400) == 0)
		return true;
	else
		return false;
}

uint32_t rtc_get_time_of_day(void)
{
	rtc_time tm;
	uint32_t nPass4year;
	uint8_t nYearremaining, month;
	uint32_t days = 0, sec = 0;

	tm = rtc_read_time();

	//取过去多少个四年，每四年有1461*24小时
	unsigned int no_leap_year_num = 0;
	unsigned int leap_year_num = 0;
	if (sys_global_time.tm_year - 1970 <= 2)
	{
		no_leap_year_num = sys_global_time.tm_year - 1970;
		leap_year_num = 0;
	}
	else
	{
		no_leap_year_num = 2;
		nPass4year = (sys_global_time.tm_year - 2 - 1970)/4;
		no_leap_year_num += (3*nPass4year);
		leap_year_num += nPass4year;

		if ((sys_global_time.tm_year - 2 - 1970)%4>=1)
		{
			leap_year_num++;
			no_leap_year_num += ((sys_global_time.tm_year - 2 - 1970)%4 - 1);
		}		
	}

	//计算 距1970 经过年的总天数
	days = no_leap_year_num*365 + leap_year_num*366;

	//计算1月1日经过的天数
	for (month = 0; month < sys_global_time.tm_mon; month++)
	{
		if (leapyear(sys_global_time.tm_year) && month == 2)
			days += febofleapYear;
		else
			days += monthDays[month];
	}
	days += (sys_global_time.tm_mday-1);

	sec = days * 24 * 3600 + tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;

	return sec;
}

struct tm rtc_get_system_time(void)
{
	struct tm systemTime;
	rtc_time rtcTime;

	systemTime.tm_year = sys_global_time.tm_year;
	systemTime.tm_mon = sys_global_time.tm_mon;
	systemTime.tm_mday = sys_global_time.tm_mday;
	systemTime.tm_wday = sys_global_time.tm_wday;

	rtcTime = rtc_read_time();
	systemTime.tm_hour = rtcTime.tm_hour;
	systemTime.tm_min = rtcTime.tm_min;
	systemTime.tm_sec = rtcTime.tm_sec;

	return systemTime;
}

void update_days()
{
	uint8_t daysofmonth;

	sys_global_time.tm_mday += 1;
	if (!(leapyear(sys_global_time.tm_year) && (sys_global_time.tm_mon == 2)))
	{
		daysofmonth = monthDays[sys_global_time.tm_mon];
	}
	else
	{
		daysofmonth = febofleapYear;
	}

	if (sys_global_time.tm_mday > daysofmonth)
	{
		sys_global_time.tm_mday = 1;
		sys_global_time.tm_mon++;

		if (sys_global_time.tm_mon > 12)
		{
			sys_global_time.tm_mon = 1;
			sys_global_time.tm_year++;
		}
	}
}

static unsigned int rtc_psm_time = 0;
static unsigned char rtc_psm_delay = 1;
int hal_rtc_allow_psm(void)
{
//	int sec, ms;
	unsigned int elapsed_ttime, curren_time = rtc_get_32K_cnt();

	if(rtc_psm_time == 0)
	{
		return 1;
	}

	if(RTC_ALARM_GET_SEC(curren_time) < RTC_ALARM_GET_SEC(rtc_psm_time) )
	{
		elapsed_ttime = 60 - RTC_ALARM_GET_SEC(rtc_psm_time) + RTC_ALARM_GET_SEC(curren_time);
	}else{
		elapsed_ttime = RTC_ALARM_GET_SEC(curren_time) - RTC_ALARM_GET_SEC(rtc_psm_time);
	}

	if(rtc_psm_delay > elapsed_ttime)
	{
		return 0;
	}
	else
	{
		rtc_psm_time = 0;
		return 1;
	}
}

unsigned int rtc_get_32K_cnt(void)
{
	int rtcMsCnt = IN32(RTC_32K_CNT_REG);
	while(1)
	{
		if((rtcMsCnt & 0x7fff) != 0)
                   break;

		rtcMsCnt = IN32(RTC_32K_CNT_REG);
	}
	return rtcMsCnt;
}

int rtc_get_interval_cnt(int pre_rtc, int now_rtc)
{
	int hour, min, sec, cnt;

	if (RTC_ALARM_GET_HOUR(pre_rtc) > RTC_ALARM_GET_HOUR(now_rtc))
		hour = 24 - RTC_ALARM_GET_HOUR(pre_rtc) + RTC_ALARM_GET_HOUR(now_rtc);
	else
		hour = RTC_ALARM_GET_HOUR(now_rtc) - RTC_ALARM_GET_HOUR(pre_rtc);

	min = hour * 60 + RTC_ALARM_GET_MIN(now_rtc) - RTC_ALARM_GET_MIN(pre_rtc);

	sec = min * 60 + RTC_ALARM_GET_SEC(now_rtc) - RTC_ALARM_GET_SEC(pre_rtc);

	cnt = sec * 32768 + RTC_ALARM_GET_32K(now_rtc) - RTC_ALARM_GET_32K(pre_rtc);

	return cnt;
}

rtc_time rtc_read_time(void)
{
	unsigned int rtcCtrl = 0;
	unsigned int value = 0;
	uint16_t rtcMsCnt = 0;
	rtc_time rtcTime;
#if 0	
	rtcCtrl = IN32(RTC_CTRL_REG);
	if((rtcCtrl & RTC_CTRL_EN(1)) == 0)
		OUT32(RTC_CTRL_REG,  rtcCtrl | RTC_CTRL_EN(1));
#endif
	value = IN32(RTC_CNT_REG);

	rtcTime.tm_sec = RTC_CNT_GET_SEC(value);
	rtcTime.tm_min = RTC_CNT_GET_MIN(value);
	rtcTime.tm_hour = RTC_CNT_GET_HOUR(value);

	rtcMsCnt = rtc_get_32K_cnt();

	rtcTime.ms = clk32K_TO_MS(rtcMsCnt & 0x7fff);

	return rtcTime;
}

void rtc_set_time_tuya(uint32_t time)
{
	rtc_timer_systime_change_pre();

	unsigned int  rtclValue = 0;
	unsigned int year=0,mon=0,mday=0,wday=0,hour= 0, min=0, sec=0;
	wday = (time%(3600*24*7))/(3600*24);

	unsigned int leap_year_num = 0;
	unsigned int no_leap_year_num = 0;
	if (time <= 2*NOLEAP_YAER_SEC)
	{
		leap_year_num = 0;
		no_leap_year_num = time/NOLEAP_YAER_SEC;
		time -= (no_leap_year_num*NOLEAP_YAER_SEC);
	}
	else
	{	
		no_leap_year_num = 2;
		time -= 2*NOLEAP_YAER_SEC;
		unsigned int four_years_num = 0;
		four_years_num = time/(3*NOLEAP_YAER_SEC + LEAP_YAER_SEC);
		no_leap_year_num += 3*four_years_num;
		time -= (four_years_num * 3 * NOLEAP_YAER_SEC);
		leap_year_num =+ four_years_num;
		time -= (four_years_num * LEAP_YAER_SEC);

		if (time > LEAP_YAER_SEC)
		{
			leap_year_num++;
			time -= LEAP_YAER_SEC;
			no_leap_year_num += (time/NOLEAP_YAER_SEC);
			time -= (time/NOLEAP_YAER_SEC)*NOLEAP_YAER_SEC;
		}
		
	}	
	year = no_leap_year_num + leap_year_num;
	mday = time/(24*3600) + 1;
	time = time%(24*3600);
	for(mon=0;mon<12;++mon)
	{		
		if (leapyear(year+1970) && (mon ==2))
		{
			if (mday <= 29)
				break;
			mday -= 29;
		}
		else
		{
			if (mday <= monthDays[mon])
				break;
			mday -= monthDays[mon];
		}
		
	}

	hour = time/3600;
	min = (time%3600)/60;
	sec = time%3600%60;
	g_net_time.rtc_time.tm_hour = hour;
	g_net_time.rtc_time.tm_min = min;
	g_net_time.rtc_time.tm_sec = sec;

	sys_global_time.tm_year = (1970+year);
	sys_global_time.tm_mon  = mon;
	sys_global_time.tm_mday = mday;	
	sys_global_time.tm_wday = (wday + 4) % 7;

 	unsigned long flags = system_irq_save();
	RTC_READY_TO_WR();
	OUT32(RTC_CNT_REG, RTC_CNT_HOUR(g_net_time.rtc_time.tm_hour) | RTC_CNT_MIN(g_net_time.rtc_time.tm_min) | RTC_CNT_SEC(g_net_time.rtc_time.tm_sec));
 
	rtc_psm_time = rtc_get_32K_cnt();
	system_irq_restore(flags);

	rtc_timer_systime_change_after();
}
#if(!(defined _USR_LMAC_TEST || defined AMT))
static int rtc_test_func(cmd_tbl_t *t, int argc, char *argv[])
{
	unsigned int rtc_time = strtoul(argv[1], NULL, 0);

#if defined(TUYA_SDK_ADPT) || defined(HISENSE_LOCK)   
	rtc_set_time_tuya(rtc_time);
#else
	extern int set_system_time(unsigned int newtime);
	set_system_time(rtc_time);
#endif
	return CMD_RET_SUCCESS;
}	
		
CMD(rtc_test,
	rtc_test_func,
	"rtc_test",
	"rtc_test rtc_time");
#endif
uint32_t rtc_get_time_tuya(void)
{
	uint32_t rtc_reg = 0,value = 0;
	rtc_reg = IN32(RTC_CNT_REG);
	value += (RTC_CNT_GET_SEC(value) + RTC_CNT_GET_SEC(value) * 60 + RTC_CNT_GET_HOUR(value) * 3600);
	return value;
}

void rtc_set_time(struct tm sysTime)
{
	rtc_timer_systime_change_pre();

	unsigned int  rtclValue = 0;
	int32_t hour, min,sec;

	sys_global_time.tm_year = sysTime.tm_year;
	sys_global_time.tm_mon  = sysTime.tm_mon;
	sys_global_time.tm_mday = sysTime.tm_mday;
	sys_global_time.tm_wday = sysTime.tm_wday;

	g_net_time.rtc_time.tm_hour = sysTime.tm_hour;
	g_net_time.rtc_time.tm_min = sysTime.tm_min;
	g_net_time.rtc_time.tm_sec = sysTime.tm_sec;

 	unsigned long flags = system_irq_save();
 
	RTC_READY_TO_WR();
	OUT32(RTC_CNT_REG, RTC_CNT_HOUR(g_net_time.rtc_time.tm_hour) | RTC_CNT_MIN(g_net_time.rtc_time.tm_min) | RTC_CNT_SEC(g_net_time.rtc_time.tm_sec));
 
	rtc_psm_time = rtc_get_32K_cnt();
	system_irq_restore(flags);
	
	rtc_timer_systime_change_after();
 }

int rtc_32k_cnt;
uint32_t rtc_set_alarm(uint32_t value, int8_t setFlag)
{

	uint32_t alarm_32k = 0;
	uint32_t alarm_sec = 0;
	uint32_t alarm_min = 0;
	uint32_t alarm_hour = 0;
	uint32_t alarm_ms = 0;
	uint32_t rtc_config = 0;
	uint32_t rtc_cnt = 0;

	//bit0 : enable  rtc，bit1:enable  alarm wakeup int
	alarmSetFlag = true;

	//read hour min sec from rtc_cnt_reg  and read ms from rtc_32k_cnt
	rtc_32k_cnt = rtc_get_32K_cnt();

#if 1
	//set 32k alarm count
	alarm_32k = value + RTC_ALARM_GET_32K(rtc_32k_cnt);
	alarm_sec = alarm_32k / ALARM_CLK;
	alarm_32k = alarm_32k % ALARM_CLK;

	//alarm  sec
	alarm_sec += RTC_ALARM_GET_SEC(rtc_32k_cnt);
	alarm_min = alarm_sec / 60;
	alarm_sec = alarm_sec % 60;

	//alarm min
	alarm_min += RTC_ALARM_GET_MIN(rtc_32k_cnt);
	alarm_hour = alarm_min / 60;
	alarm_min = alarm_min % 60;

	//alarm hour
	alarm_hour = (RTC_ALARM_GET_HOUR(rtc_32k_cnt) + alarm_hour) % 24;

	rtc_cnt = RTC_ALARM_HOUR(alarm_hour) |
			  RTC_ALARM_MIN(alarm_min) |
			  RTC_ALARM_SEC(alarm_sec) |
			  RTC_ALARM_32K(alarm_32k);
#else
	rtc_cnt = rtc_32k_cnt + MS_TO_ALARM_CNT(alarm_ms);
	rtc_cnt += RTC_ALARM_SET_SEC(alarm_sec);
#endif

	//system_printf("%d:%d:%d.%d, rtc_cnt:%x ,values:%d\n", alarm_hour, alarm_min, alarm_sec, alarm_32k, rtc_cnt, value);
#if 1
	if ((rtc_cnt & 0x7fff) == 0x0)
	{
		rtc_cnt += 1;
	}
#endif

	if (!setFlag)
		return rtc_cnt;

	RTC_READY_TO_WR();
	OUT32(RTC_ALARM_REG, rtc_cnt);

	rtc_config = IN32(RTC_CTRL_REG);
	RTC_READY_TO_WR();
	OUT32(RTC_CTRL_REG, rtc_config | RTC_CTRL_WAKEUP(1));

	return rtc_cnt;
}

void rtc_stop_alarm(void)
{
	uint32_t rtc_config = 0;

	rtc_config = IN32(RTC_CTRL_REG);

	rtc_config &= ~(RTC_CTRL_WAKEUP(1));

	RTC_READY_TO_WR();
	OUT32(RTC_CTRL_REG, rtc_config);
}

//计算alarm 定时器剩余时间
int32_t alarmReg = 0, currReg = 0, currReg1 = 0;
uint32_t pcu_int_status;
int32_t rtc_get_alarm(void)
{
	int32_t alarmReg = 0, currReg = 0;
	int32_t   ms = 0;

	RTC_READY_TO_WR();
	alarmReg = IN32(RTC_ALARM_REG);
	currReg 	= rtc_get_32K_cnt();

	ms = rtc_get_interval_cnt(alarmReg,  currReg);
	
	return   ms;
}

void hal_rtc_isr(int vector)
{
	uint32_t rtcstatus;
	rtc_time rtcTime;

	irq_status_clean(vector);

	rtcstatus = IN32(RTC_STATUS_REG);
	OUT32(RTC_STATUS_REG, rtcstatus);

	if (RTC_STATUS_GET_DAY(rtcstatus))
	{
		update_days();

		rtcTime = rtc_read_time();
		system_printf("***************************************************\n");
		system_printf("%d:%d:%d  %d:%d:%d\n", sys_global_time.tm_year, sys_global_time.tm_mon, sys_global_time.tm_mday, rtcTime.tm_hour, rtcTime.tm_min, rtcTime.tm_sec);
		system_printf("***************************************************\n");
	}
}

void hal_rtc_init(void)
{
	uint32_t rtclValue;

	sys_global_time.tm_year = 1970;
	sys_global_time.tm_mon = 01;
	sys_global_time.tm_mday = 01;
	sys_global_time.tm_wday = 4;//1970/01/01 Thursday

	irq_isr_register(IRQ_VECTOR_RTC, (void *)hal_rtc_isr);
	irq_unmask(IRQ_VECTOR_RTC);

	rtclValue = RTC_CTRL_EN(1) | RTC_CTRL_WAKEUP(0) | RTC_CTRL_INT(0) | RTC_CTRL_DAY(1) | RTC_CTRL_HOUR(0) | RTC_CTRL_MIN(0) | RTC_CTRL_SEC(0) | RTC_CTRL_HSEC(0);
	OUT32(RTC_CTRL_REG, rtclValue);
}

#ifdef _USE_RTC_TEST_CMD_SET_TIME
static int cmd_set_time(cmd_tbl_t *t, int argc, char *argv[])
{
	struct tm sysTime;
	rtc_time tmpTime;

	sysTime.tm_hour = atoi(argv[1]);
	sysTime.tm_min = atoi(argv[2]);
	sysTime.tm_sec = atoi(argv[3]);

	sysTime.tm_year = atoi(argv[4]);
	sysTime.tm_mon = atoi(argv[5]);
	sysTime.tm_mday = atoi(argv[6]);

	tmpTime = rtc_read_time();
	system_printf("before set rtc:%d:%d:%d  %d:%d:%d\n", sys_global_time.tm_year, sys_global_time.tm_mon, sys_global_time.tm_mday, tmpTime.tm_hour, tmpTime.tm_min, tmpTime.tm_sec);

	rtc_set_time(sysTime);

	tmpTime = rtc_read_time();
	system_printf("after set rtc:%d:%d:%d  %d:%d:%d\n", sys_global_time.tm_year, sys_global_time.tm_mon, sys_global_time.tm_mday, tmpTime.tm_hour, tmpTime.tm_min, tmpTime.tm_sec);

	return CMD_RET_SUCCESS;
}

SUBCMD(set,
	   sys_time,
	   cmd_set_time,
	   "set system time",
	   "set sys_time [HH MM SS YYYY mm DD]");
#endif


static int cmd_read_time(cmd_tbl_t *t, int argc, char *argv[])
{
	rtc_time tmpTime = rtc_read_time();
	system_printf("CurrentTime: %s %d-%02d-%02d %02d:%02d:%02d\n",
				  week[sys_global_time.tm_wday],
				  sys_global_time.tm_year,
				  sys_global_time.tm_mon,
				  sys_global_time.tm_mday,
				  tmpTime.tm_hour,
				  tmpTime.tm_min,
				  tmpTime.tm_sec);

	return CMD_RET_SUCCESS;
}

SUBCMD(set,
	   rtcread,
	   cmd_read_time,
	   "set system time",
	   "set rtcread");


/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************************/
//
//                                    RTC函数
//
/*****************************************************************************************************/
const int8_t monthday_leapyear[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const int8_t monthday_noryear[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define RTC_DATE_PACKAGE(year, month, day, hour, min, sec, cnt) \
	((year) << 41 | (month) << 37 | (day) << 32 | (hour) << 27 | (min) << 21 | (sec) << 15 | (cnt))
#define RTC_DATE_UNPACKAGE_YEAR(date) ((date) >> 41)
#define RTC_DATE_UNPACKAGE_MONTH(date) (((date) >> 37) & 0xf)
#define RTC_DATE_UNPACKAGE_DAY(date) (((date) >> 32) & 0x1f)
#define RTC_DATE_UNPACKAGE_HOUR(date) (((date) >> 27) & 0x1f)
#define RTC_DATE_UNPACKAGE_MIN(date) (((date) >> 21) & 0x3f)
#define RTC_DATE_UNPACKAGE_SEC(date) (((date) >> 15) & 0x3f)
#define RTC_DATE_UNPACKAGE_MS(date) ((date)&0x7fff)
#define RTC_DATE_COMPARE(date1, date2) ((date1 == date2) ? 0 : ((date1 > date2) ? 1 : -1))
#define RTC_INTEGER(x) ((int)(x + 0.5))

/*利用基姆拉尔森计算日期公式  w=(d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)*/
const char *rtc_get_weekday(int year, int month, int day)
{
	int weekday = -1;
	if (1 == month || 2 == month)
	{
		month += 12;
		year--;
	}
	weekday = (day + 1 + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7;

	if (weekday >= 0 && weekday <= 6)
	{
		return week[weekday];
	}

	return NULL;
}
#if 0
static int cmd_yeartoweek(cmd_tbl_t *t, int argc, char *argv[])
{
	const char *week = rtc_get_weekday(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));

	system_printf("WeekDay : %s\n", week);

	return CMD_RET_SUCCESS;
}
SUBCMD(set, yeartoweek, cmd_yeartoweek, "set yeartoweek", "set yeartoweek");
#endif
// 输入年份，返回今年一共多少天
static rtc_base_t rtc_year_to_day(rtc_base_t year)
{
	if (leapyear(year))
	{
		return 366;
	}
	else
	{
		return 365;
	}
}
// 这一年的某个月有多少天
static rtc_base_t rtc_monthday_num(rtc_base_t year, rtc_base_t month)
{
	if (leapyear(year))
	{
		return monthday_leapyear[month];
	}
	else
	{
		return monthday_noryear[month];
	}
}
// 年月日差了多少天
// tag - now
static rtc_base_t rtc_days_interval(rtc_base_t yearnow, rtc_base_t monthnow, rtc_base_t daynow,
									rtc_base_t yeartag, rtc_base_t monthtag, rtc_base_t daytag)
{
	rtc_base_t passdays_now = 0, passdays_tag = 0, num = 0;

	for (num = 1; num < monthnow; num++)
	{
		passdays_now += rtc_monthday_num(yearnow, num);
	}
	passdays_now += daynow; // 现在时刻在一年中过去了几天

	for (num = 1; num < monthtag; num++)
	{
		passdays_tag += rtc_monthday_num(yeartag, num);
	}
	passdays_tag += daytag; // 目标时刻在一年中过去了几天

	if (yearnow == yeartag)
	{
		return passdays_tag - passdays_now;
	}
	else
	{
		passdays_now = rtc_year_to_day(yearnow) - passdays_now + passdays_tag;

		for (num = yearnow + 1; num < yeartag; num++)
		{
			passdays_now += rtc_year_to_day(num);
		}

		return passdays_now;
	}
}
#if 0
static int cmd_rtc_days_interval(cmd_tbl_t *t, int argc, char *argv[])
{
	rtc_base_t year1 = atoi(argv[1]);
	rtc_base_t month1 = atoi(argv[2]);
	rtc_base_t day1 = atoi(argv[3]);
	rtc_base_t year2 = atoi(argv[4]);
	rtc_base_t month2 = atoi(argv[5]);
	rtc_base_t day2 = atoi(argv[6]);

	rtc_base_t daypassed = rtc_days_interval(year1,month1,day1,year2,month2,day2);

	system_printf("daypassed:%d\r\n",daypassed);

	return CMD_RET_SUCCESS;
}
SUBCMD(set, rtcdaysinterval, cmd_rtc_days_interval, "set rtcdaysinterval", "set rtcdaysinterval");
#endif

const rtc_base_t year2days[5] = {0, 366, 731, 1096, 1461};
const rtc_base_t leapMonthDays[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};
const rtc_base_t notleapMonthDays[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

// 天数转化为年月日
// 0年1月一日为一天
// 0年最后一天为366天
static void rtc_days_to_yeardays(rtc_base_t days, rtc_base_t *year, rtc_base_t *month, rtc_base_t *day)
{
	if (days == 0)
	{
		return;
	}

	rtc_base_t year_fac = days / 1461;

	// rtc_base_t days_remain = days - year_fac * 1461;
	rtc_base_t days_remain = days % 1461;

	rtc_base_t years = year_fac * 4;

	uint8_t i = 0;

	if (days_remain == 0) // 一年的最后一天
	{
		*year = years - 1;
		*month = 12;
		*day = 31;
	}
	else if (days_remain <= 366)
	{
		*year = years;

		for (i = 1; i < 13; i++)
		{
			if (leapMonthDays[i - 1] < days_remain && days_remain <= leapMonthDays[i])
			{
				*month = i;
				*day = days_remain - leapMonthDays[i - 1];
			}
		}
	}
	else if (days_remain <= 731)
	{
		*year = years + 1;
		days_remain -= 366;
		for (i = 1; i < 13; i++)
		{
			if (notleapMonthDays[i - 1] < days_remain && days_remain <= notleapMonthDays[i])
			{
				*month = i;
				*day = days_remain - notleapMonthDays[i - 1];
			}
		}
	}
	else if (days_remain <= 1096)
	{
		*year = years + 2;
		days_remain -= 731;
		for (i = 1; i < 13; i++)
		{
			if (notleapMonthDays[i - 1] < days_remain && days_remain <= notleapMonthDays[i])
			{
				*month = i;
				*day = days_remain - notleapMonthDays[i - 1];
			}
		}
	}
	else
	{
		*year = years + 3;
		days_remain -= 1096;
		for (i = 1; i < 13; i++)
		{
			if (notleapMonthDays[i - 1] < days_remain && days_remain <= notleapMonthDays[i])
			{
				*month = i;
				*day = days_remain - notleapMonthDays[i - 1];
			}
		}
	}
}
#if 0
static int cmd_rtc_days_to_yeardays(cmd_tbl_t *t, int argc, char *argv[])
{
	rtc_base_t year,month,day;

	rtc_days_to_yeardays(atoi(argv[1]),&year,&month,&day);

	system_printf("day:%ld ---> %ld-%ld-%ld\r\n",atoi(argv[1]),year,month,day);

	return CMD_RET_SUCCESS;
}
SUBCMD(set, rtcdaystoyeardays, cmd_rtc_days_to_yeardays, "set rtcdaystoyeardays", "set rtcdaystoyeardays");
#endif

// 年月日转为天数
// 0年1月1日为一天，2月2日为两天
static rtc_base_t rtc_yeardays_to_days(rtc_base_t year, rtc_base_t month, rtc_base_t day)
{
	rtc_base_t yearpassed = year / 4;

	rtc_base_t yearremain = year % 4;

	rtc_base_t days = yearpassed * 1461;

	rtc_base_t i;

	switch (yearremain)
	{
	case 1:
		days += 366;
		break;
	case 2:
		days += 731;
		break;
	case 3:
		days += 1096;
		break;
	default:
		break;
	}

	for (i = 1; i < month; i++) // 现在时刻在一年中过去了几天
	{
		days += rtc_monthday_num(year, i);
	}
	days += day;

	return days;
}
#if 0
static int cmd_rtc_yeardays_to_days(cmd_tbl_t *t, int argc, char *argv[])
{
	rtc_base_t days = rtc_yeardays_to_days(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));

	system_printf("%ld-%ld-%ld ---> days:%ld\r\n", atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), days);

	return CMD_RET_SUCCESS;
}
SUBCMD(set, rtcyeardaystodays, cmd_rtc_yeardays_to_days, "set cmd_rtc_yeardays_to_days", "set cmd_rtc_yeardays_to_days");
#endif
// 两个时间相加
// datenow + datetag
static rtc_date_t rtc_date_add(rtc_date_t datenow, rtc_date_t datetag)
{
	rtc_base_t yearnow = RTC_DATE_UNPACKAGE_YEAR(datenow);
	rtc_base_t monthnow = RTC_DATE_UNPACKAGE_MONTH(datenow);
	rtc_base_t daynow = RTC_DATE_UNPACKAGE_DAY(datenow);
	rtc_base_t hournow = RTC_DATE_UNPACKAGE_HOUR(datenow);
	rtc_base_t minnow = RTC_DATE_UNPACKAGE_MIN(datenow);
	rtc_base_t secnow = RTC_DATE_UNPACKAGE_SEC(datenow);
	rtc_base_t msnow = RTC_DATE_UNPACKAGE_MS(datenow);

	rtc_base_t yeartag = RTC_DATE_UNPACKAGE_YEAR(datetag);
	rtc_base_t monthtag = RTC_DATE_UNPACKAGE_MONTH(datetag);
	rtc_base_t daytag = RTC_DATE_UNPACKAGE_DAY(datetag);
	rtc_base_t hourtag = RTC_DATE_UNPACKAGE_HOUR(datetag);
	rtc_base_t mintag = RTC_DATE_UNPACKAGE_MIN(datetag);
	rtc_base_t sectag = RTC_DATE_UNPACKAGE_SEC(datetag);
	rtc_base_t mstag = RTC_DATE_UNPACKAGE_MS(datetag);

	rtc_base_t daytag_exclude_today = rtc_yeardays_to_days(yeartag, monthtag, daytag);
	rtc_base_t daynow_exclude_today = rtc_yeardays_to_days(yearnow, monthnow, daynow);

	rtc_base_t day = daytag_exclude_today + daynow_exclude_today;
	rtc_base_t hour = hournow + hourtag;
	rtc_base_t mini = minnow + mintag;
	rtc_base_t sec = secnow + sectag;
	rtc_base_t ms = msnow + mstag;

	sec += ms / 1000;
	ms = ms % 1000;

	mini += sec / 60;
	sec = sec % 60;

	hour += mini / 60;
	mini = mini % 60;

	day += hour / 24;
	hour = hour % 24;

	rtc_days_to_yeardays(day, &yeartag, &monthtag, &daytag);

	rtc_date_t date = RTC_DATE_PACKAGE((rtc_date_t)yeartag, (rtc_date_t)monthtag, (rtc_date_t)daytag,
									   (rtc_date_t)hour, (rtc_date_t)mini, (rtc_date_t)sec, 0);

	return date;
}
#if 0
static int cmd_rtc_date_add(cmd_tbl_t *t, int argc, char *argv[])
{
	rtc_base_t yearnow = atoi(argv[1]);
	rtc_base_t monthnow = atoi(argv[2]);
	rtc_base_t daynow = atoi(argv[3]);
	rtc_base_t hournow = atoi(argv[4]);
	rtc_base_t minnow = atoi(argv[5]);
	rtc_base_t secnow = atoi(argv[6]);

	rtc_base_t yeartag = atoi(argv[7]);
	rtc_base_t monthtag = atoi(argv[8]);
	rtc_base_t daytag = atoi(argv[9]);
	rtc_base_t hourtag = atoi(argv[10]);
	rtc_base_t mintag = atoi(argv[11]);
	rtc_base_t sectag = atoi(argv[12]);

	rtc_base_t daytag_exclude_today = rtc_yeardays_to_days(yeartag,monthtag,daytag) ;
	rtc_base_t daynow_exclude_today = rtc_yeardays_to_days(yearnow,monthnow,daynow) ;

	rtc_base_t day = daytag_exclude_today + daynow_exclude_today;
	rtc_base_t hour = hournow + hourtag;
	rtc_base_t mini = minnow + mintag;
	rtc_base_t sec = secnow + sectag;

	mini += sec/60;
	sec = sec%60;

	hour += mini/60;
	mini = mini % 60;

	day += hour/24;
	hour = hour % 24;

	rtc_days_to_yeardays(day,&yeartag,&monthtag,&daytag);

	system_printf("%ld-%ld-%ld  %ld:%ld:%ld\r\n",yeartag,monthtag,daytag,hour,mini,sec);

	
	return CMD_RET_SUCCESS;
}
SUBCMD(set, rtcdateadd, cmd_rtc_date_add, "set rtcdateadd", "set rtcdateadd");
#endif
// 年月日时分秒差了多少秒
// tag - now
static rtc_base_t rtc_seconds_interval(rtc_date_t datenow, rtc_date_t datetag)
{
	rtc_base_t yearnow = RTC_DATE_UNPACKAGE_YEAR(datenow);
	rtc_base_t monthnow = RTC_DATE_UNPACKAGE_MONTH(datenow);
	rtc_base_t daynow = RTC_DATE_UNPACKAGE_DAY(datenow);
	rtc_base_t hournow = RTC_DATE_UNPACKAGE_HOUR(datenow);
	rtc_base_t minnow = RTC_DATE_UNPACKAGE_MIN(datenow);
	rtc_base_t secnow = RTC_DATE_UNPACKAGE_SEC(datenow);

	rtc_base_t yeartag = RTC_DATE_UNPACKAGE_YEAR(datetag);
	rtc_base_t monthtag = RTC_DATE_UNPACKAGE_MONTH(datetag);
	rtc_base_t daytag = RTC_DATE_UNPACKAGE_DAY(datetag);
	rtc_base_t hourtag = RTC_DATE_UNPACKAGE_HOUR(datetag);
	rtc_base_t mintag = RTC_DATE_UNPACKAGE_MIN(datetag);
	rtc_base_t sectag = RTC_DATE_UNPACKAGE_SEC(datetag);
	rtc_base_t secinterval = 0;

	rtc_base_t day_interval = rtc_days_interval(yearnow, monthnow, daynow, yeartag, monthtag, daytag);

	rtc_base_t secpassnow = hournow * 60 * 60 + minnow * 60 + secnow;
	rtc_base_t secpasstag = hourtag * 60 * 60 + mintag * 60 + sectag;

	if (day_interval == 0)
	{
		secinterval = secpasstag - secpassnow;
	}
	else
	{
		secinterval = 24 * 60 * 60 - secpassnow + secpasstag;

		secinterval += (day_interval - 1) * 24 * 60 * 60;
	}

	return secinterval;
}
#if 0
static int cmd_rtc_seconds_interval(cmd_tbl_t *t, int argc, char *argv[])
{
	rtc_base_t yearnow = atoi(argv[1]);
	rtc_base_t monthnow = atoi(argv[2]);
	rtc_base_t daynow = atoi(argv[3]);
	rtc_base_t hournow = atoi(argv[4]);
	rtc_base_t minnow = atoi(argv[5]);
	rtc_base_t secnow = atoi(argv[6]);
	rtc_base_t msnow = atoi(argv[7]);

	rtc_base_t yeartag = atoi(argv[8]);
	rtc_base_t monthtag = atoi(argv[9]);
	rtc_base_t daytag = atoi(argv[10]);
	rtc_base_t hourtag = atoi(argv[11]);
	rtc_base_t mintag = atoi(argv[12]);
	rtc_base_t sectag = atoi(argv[13]);
	rtc_base_t mstag = atoi(argv[14]);
	rtc_date_t datenow = RTC_DATE_PACKAGE((rtc_date_t)(yearnow), (rtc_date_t)(monthnow), (rtc_date_t)(daynow),
										  (rtc_date_t)(hournow), (rtc_date_t)(minnow), (rtc_date_t)(secnow),
										  (rtc_date_t)(msnow));
	rtc_date_t datetag = RTC_DATE_PACKAGE((rtc_date_t)(yeartag), (rtc_date_t)(monthtag), (rtc_date_t)(daytag),
										  (rtc_date_t)(hourtag), (rtc_date_t)(mintag), (rtc_date_t)(sectag),
										  (rtc_date_t)(mstag));
	rtc_base_t interval = rtc_seconds_interval(datenow, datetag);

	system_printf("interval seconds:%ld\r\n", interval); // 理论应该相差1450450059秒

	return CMD_RET_SUCCESS;
}
SUBCMD(set, rtcsecondsinterval, cmd_rtc_seconds_interval, "set yeartoweek", "set yeartoweek");
#endif

// 年月日时分秒差了多少毫秒
// tag - now
static rtc_date_t rtc_ms_interval(rtc_date_t datenow, rtc_date_t datetag)
{
	rtc_date_t yearnow = RTC_DATE_UNPACKAGE_YEAR(datenow);
	rtc_date_t monthnow = RTC_DATE_UNPACKAGE_MONTH(datenow);
	rtc_date_t daynow = RTC_DATE_UNPACKAGE_DAY(datenow);
	rtc_date_t hournow = RTC_DATE_UNPACKAGE_HOUR(datenow);
	rtc_date_t minnow = RTC_DATE_UNPACKAGE_MIN(datenow);
	rtc_date_t secnow = RTC_DATE_UNPACKAGE_SEC(datenow);
	rtc_date_t msnow = RTC_DATE_UNPACKAGE_MS(datenow);

	rtc_date_t yeartag = RTC_DATE_UNPACKAGE_YEAR(datetag);
	rtc_date_t monthtag = RTC_DATE_UNPACKAGE_MONTH(datetag);
	rtc_date_t daytag = RTC_DATE_UNPACKAGE_DAY(datetag);
	rtc_date_t hourtag = RTC_DATE_UNPACKAGE_HOUR(datetag);
	rtc_date_t mintag = RTC_DATE_UNPACKAGE_MIN(datetag);
	rtc_date_t sectag = RTC_DATE_UNPACKAGE_SEC(datetag);
	rtc_date_t mstag = RTC_DATE_UNPACKAGE_MS(datetag);
	rtc_date_t msinterval = 0;

	rtc_date_t day_interval = rtc_days_interval(yearnow, monthnow, daynow, yeartag, monthtag, daytag);

	rtc_date_t mspassnow = (hournow * 60 * 60 + minnow * 60 + secnow) * 1000 + msnow;
	rtc_date_t mspasstag = (hourtag * 60 * 60 + mintag * 60 + sectag) * 1000 + mstag;

	if (day_interval == 0)
	{
		msinterval = mspasstag - mspassnow;
	}
	else
	{
		msinterval = 24 * 60 * 60 * 1000 - mspassnow + mspasstag;

		msinterval += (day_interval - 1) * 24 * 60 * 60 * 1000;
	}

	return msinterval;
}
#if 0
static int cmd_rtc_ms_interval(cmd_tbl_t *t, int argc, char *argv[])
{
	rtc_base_t yearnow = atoi(argv[1]);
	rtc_base_t monthnow = atoi(argv[2]);
	rtc_base_t daynow = atoi(argv[3]);
	rtc_base_t hournow = atoi(argv[4]);
	rtc_base_t minnow = atoi(argv[5]);
	rtc_base_t secnow = atoi(argv[6]);
	rtc_base_t msnow = atoi(argv[7]);

	rtc_base_t yeartag = atoi(argv[8]);
	rtc_base_t monthtag = atoi(argv[9]);
	rtc_base_t daytag = atoi(argv[10]);
	rtc_base_t hourtag = atoi(argv[11]);
	rtc_base_t mintag = atoi(argv[12]);
	rtc_base_t sectag = atoi(argv[13]);
	rtc_base_t mstag = atoi(argv[14]);
	rtc_date_t datenow = RTC_DATE_PACKAGE((rtc_date_t)(yearnow), (rtc_date_t)(monthnow), (rtc_date_t)(daynow),
										  (rtc_date_t)(hournow), (rtc_date_t)(minnow), (rtc_date_t)(secnow),
										  (rtc_date_t)(msnow));
	rtc_date_t datetag = RTC_DATE_PACKAGE((rtc_date_t)(yeartag), (rtc_date_t)(monthtag), (rtc_date_t)(daytag),
										  (rtc_date_t)(hourtag), (rtc_date_t)(mintag), (rtc_date_t)(sectag),
										  (rtc_date_t)(mstag));
	rtc_date_t interval = rtc_ms_interval(datenow, datetag);

	system_printf("interval Mseconds:%lld\r\n", interval); //

	return CMD_RET_SUCCESS;
}
SUBCMD(set, rtcmsinterval, cmd_rtc_ms_interval, "set cmd_rtc_ms_interval", "set cmd_rtc_ms_interval");
#endif

// 获取当前的日期和时间
static rtc_date_t rtc_get_now_date(void)
{
	rtc_time nowtime = rtc_read_time();
	rtc_date_t year = sys_global_time.tm_year;
	rtc_date_t month = sys_global_time.tm_mon;
	rtc_date_t day = sys_global_time.tm_mday;
	rtc_date_t hour = nowtime.tm_hour;
	rtc_date_t min = nowtime.tm_min;
	rtc_date_t sec = nowtime.tm_sec;
	rtc_date_t ms = nowtime.ms;

	rtc_date_t ret = RTC_DATE_PACKAGE(year, month, day, hour, min, sec, ms);

	return ret;
}
static rtc_date_t rtc_get_now_date_each(uint8_t sel)
{
	rtc_time nowtime = rtc_read_time();
	rtc_date_t year = sys_global_time.tm_year;
	rtc_date_t month = sys_global_time.tm_mon;
	rtc_date_t day = sys_global_time.tm_mday;
	rtc_date_t hour = nowtime.tm_hour;
	rtc_date_t min = nowtime.tm_min;
	rtc_date_t sec = nowtime.tm_sec;
	rtc_date_t ms = nowtime.ms;

	switch (sel)
	{
	case 1:
		return year;
		break;
	case 2:
		return month;
		break;

	case 3:
		return day;
		break;

	case 4:
		return hour;
		break;

	case 5:
		return min;
		break;

	case 6:
		return sec;
		break;

	case 7:
		return ms;
		break;
	default:
		break;
	}
	return 0;
}

// 根据周期性方式预测下一个的时间点：年月日
// 一天的周期
// 当前日期加一天的日期
static void rtc_next_time_day_period(rtc_base_t yearnow, rtc_base_t monthnow, rtc_base_t daynow,
									 rtc_base_t *yeartag, rtc_base_t *monthtag, rtc_base_t *daytag)
{
	if (daynow == rtc_monthday_num(yearnow, monthnow)) // 月末
	{
		if (monthnow == 12) //一年的最后一天
		{
			*yeartag = yearnow + 1;
			*monthtag = 1;
			*daytag = 1;
		}
		else // 一个月的最后一天
		{
			*yeartag = yearnow;
			*monthtag = monthnow + 1;
			*daytag = 1;
		}
	}
	else // 不是月末
	{
		*yeartag = yearnow;
		*monthtag = monthnow;
		*daytag = daynow + 1;
	}
}
// 一个月的周期
// 正常只允许最大日期为28号
static bool rtc_next_time_month_period(rtc_base_t yearnow, rtc_base_t monthnow, rtc_base_t daynow,
									   rtc_base_t *yeartag, rtc_base_t *monthtag, rtc_base_t *daytag)
{
	if (daynow > 28)
	{
		return false;
	}
	else
	{
		if (monthnow == 12)
		{
			*yeartag = yearnow + 1;
			*monthtag = 1;
			*daytag = daynow;
		}
		else
		{
			*yeartag = yearnow;
			*monthtag = monthnow + 1;
			*daytag = daynow;
		}
		return true;
	}
}
// 一星期的周期
static void rtc_next_time_week_period(rtc_base_t yearnow, rtc_base_t monthnow, rtc_base_t daynow,
									  rtc_base_t *yeartag, rtc_base_t *monthtag, rtc_base_t *daytag)
{
}
/****************************************************************************
* 	                                        链表
****************************************************************************/

static __inline void rt_list_init(rtc_timer_t *l)
{
	l->next = l->prev = l;
}

static __inline void rt_list_insert_after(rtc_timer_t *l, rtc_timer_t *n)
{
	l->next->prev = n;
	n->next = l->next;

	l->next = n;
	n->prev = l;
}

static __inline void rt_list_insert_before(rtc_timer_t *l, rtc_timer_t *n)
{
	l->prev->next = n;
	n->prev = l->prev;

	l->prev = n;
	n->next = l;
}

static __inline void rt_list_remove(rtc_timer_t *n)
{
	n->next->prev = n->prev;
	n->prev->next = n->next;

	n->next = n->prev = n;
}

static __inline int rt_list_isempty(const rtc_timer_t *l)
{
	return l->next == l;
}

static __inline unsigned int rt_list_len(const rtc_timer_t *l)
{
	if (l == NULL)
	{
		return 0;
	}

	unsigned int len = 1;
	const rtc_timer_t *p = l;
	while (p->next != l)
	{
		p = p->next;
		len++;
	}

	return len;
}
/*****************************************************************************************************/
//
//                                     RTC Timer函数
//
/*****************************************************************************************************/

rtc_timer_t *TimerListHead = NULL;

#define rtc_timer_excute_callback(_callback_, context) \
	do                                                 \
	{                                                  \
		if (_callback_ == NULL)                        \
		{                                              \
			while (1)                                  \
				;                                      \
		}                                              \
		else                                           \
		{                                              \
			_callback_(context);                       \
		}                                              \
	} while (0);

static void rtc_timer_date_show(rtc_date_t date)
{
#if 1
	rtc_base_t A = RTC_DATE_UNPACKAGE_YEAR(date);
	rtc_base_t B = RTC_DATE_UNPACKAGE_MONTH(date);
	rtc_base_t C = RTC_DATE_UNPACKAGE_DAY(date);
	rtc_base_t D = RTC_DATE_UNPACKAGE_HOUR(date);
	rtc_base_t E = RTC_DATE_UNPACKAGE_MIN(date);
	rtc_base_t F = RTC_DATE_UNPACKAGE_SEC(date);
	rtc_base_t G = RTC_DATE_UNPACKAGE_MS(date);
	system_printf("  ||%4d-%2d-%2d %2d:%2d:%2d:%4d||  ", A, B, C, D, E, F, G);
#endif
}
static void rtc_timer_status_show(rtc_timer_t *timer)
{
#if 1
	switch (timer->status)
	{
	case RTC_TIMER_STATUS_INACTIVE:
		system_printf(" | inactive | ");
		break;
	case RTC_TIMER_STATUS_READY:
		system_printf(" | ready    | ");
		break;
	case RTC_TIMER_STATUS_RUNNING:
		system_printf(" | running  | ");
		break;
	case RTC_TIMER_STATUS_STOP:
		system_printf(" | stop     | ");
		break;
	default:
		break;
	}
#endif
}

static void rtc_timer_type_flag_show(rtc_timer_t *timer)
{
#if 1
	switch (timer->type)
	{
	case RTC_TIMER_TYPE_COUNT_DOWN:
		system_printf(" | count down | ");
		break;
	case RTC_TIMER_TYPE_TARGET_DAY:
		system_printf(" | target day | ");
		break;
	case RTC_TIMER_TYPE_TARGET_MONTH:
		system_printf(" | target mon | ");
		break;
	default:
		break;
	}
	switch (timer->flag)
	{
	case RTC_TIMER_FLAG_ONE_SHOT:
		system_printf(" | one shot | ");
		break;
	case RTC_TIMER_FLAG_PERIODIC:
		system_printf(" | periodic | ");
		break;
	default:
		break;
	}
#endif
}
static void rtc_timer_list_show(void)
{
	int len = rt_list_len(TimerListHead);
	int i = 0;
	rtc_timer_t *cur = TimerListHead;

	system_printf("===========timer list lengh  =  %d  ===============\r\n", len);

	while (len--)
	{
		system_printf("[%s]:", cur->name);
		rtc_timer_date_show(cur->set_date);
		rtc_timer_date_show(cur->aim_date);
		rtc_timer_status_show(cur);
		rtc_timer_type_flag_show(cur);
		system_printf("\r\n");
		cur = cur->next;
	}
}

// 设置alarm时间
// 一般在修改了表头定时器时才设置
static void rtc_timer_set_alarm(rtc_timer_t *timer)
{
	if (TimerListHead == NULL)
	{
		rtc_stop_alarm();
	}
	if (timer == NULL)
	{
		return;
	}

	timer->status = RTC_TIMER_STATUS_RUNNING;

	rtc_base_t rtc_cnt;
	rtc_base_t rtc_config = 0;
	rtc_date_t date = timer->aim_date;

	rtc_base_t alarm_hour = RTC_DATE_UNPACKAGE_HOUR(date);
	rtc_base_t alarm_min = RTC_DATE_UNPACKAGE_MIN(date);
	rtc_base_t alarm_sec = RTC_DATE_UNPACKAGE_SEC(date);
	rtc_base_t alarm_ms = RTC_DATE_UNPACKAGE_MS(date);

	rtc_base_t alarm_32k = RTC_INTEGER(alarm_ms * 32.768);

	// system_printf("alarm set:%d %d %d %d\r\n", alarm_hour, alarm_min, alarm_sec, alarm_ms, alarm_32k);

	rtc_cnt = RTC_ALARM_HOUR(alarm_hour) |
			  RTC_ALARM_MIN(alarm_min) |
			  RTC_ALARM_SEC(alarm_sec) |
			  RTC_ALARM_32K(alarm_32k);
	if ((rtc_cnt & 0x7fff) == 0x0)
	{
		rtc_cnt += 1;
	}

	RTC_READY_TO_WR();
	OUT32(RTC_ALARM_REG, rtc_cnt);

	rtc_config = IN32(RTC_CTRL_REG);
	RTC_READY_TO_WR();
	OUT32(RTC_CTRL_REG, rtc_config | RTC_CTRL_WAKEUP(1));
}

// 定时器顺序插入
static void rtc_timer_list_insert(rtc_timer_t *timer)
{
	if (timer == NULL)
	{
		return;
	}

	rtc_timer_t *cur;
	timer->status = RTC_TIMER_STATUS_READY;

	if (TimerListHead == NULL)
	{
		timer->status = RTC_TIMER_STATUS_RUNNING;
		TimerListHead = timer;
	}
	// 只有一个定时器
	else if (TimerListHead == TimerListHead->next)
	{
		rt_list_insert_before(TimerListHead, timer); // 插入表头前面

		// timer 比表头小
		if (RTC_DATE_COMPARE(TimerListHead->aim_date, timer->aim_date) >= 0)
		{
			TimerListHead->status = RTC_TIMER_STATUS_READY;
			TimerListHead = timer;
			timer->status = RTC_TIMER_STATUS_RUNNING;
		}
	}
	else
	{
		// timer 比表头小，也就是最小
		if (RTC_DATE_COMPARE(TimerListHead->aim_date, timer->aim_date) >= 0)
		{
			rt_list_insert_before(TimerListHead, timer); // 插入表头前面
			TimerListHead->status = RTC_TIMER_STATUS_READY;
			TimerListHead = timer;
			timer->status = RTC_TIMER_STATUS_RUNNING;
		}
		// timer 最大
		else if (RTC_DATE_COMPARE(TimerListHead->prev->aim_date, timer->aim_date) <= 0)
		{
			rt_list_insert_before(TimerListHead, timer); // 插入表头前面
		}
		else
		{
			cur = TimerListHead->next;

			while (cur != TimerListHead)
			{
				if (RTC_DATE_COMPARE(cur->aim_date, timer->aim_date) >= 0)
				{
					rt_list_insert_before(cur, timer);
					break;
				}
				cur = cur->next;
			}
		}
	}
}

// 表头移除链表
static void rtc_timer_listhead_remove(void)
{
	if (TimerListHead == NULL)
	{
		return;
	}
	rtc_timer_t *tmp = TimerListHead->next;

	// 只有一个定时器
	if (TimerListHead->next == TimerListHead)
	{
		TimerListHead = NULL;
	}
	// 不止一个定时器
	else
	{
		rt_list_remove(TimerListHead);
		TimerListHead = tmp;
	}
}

// 倒计时调用该函数的话直接就会重新计时
// 闹钟调用该函数，超时的话则更新目标时间
static void rtc_timer_aimdate_forecast(rtc_timer_t *timer)
{
	// if(RTC_DATE_COMPARE(rtc_get_now_date(),timer->aim_date) <= 0)	// 超时判断
	// {
	// 	return;		// 当前时间并未超过目标时间，不更新
	// }

	// rtc_date_t date = timer->aim_date; // 更新时间时：目标时间 == 当前时间
	rtc_date_t yearnow = RTC_DATE_UNPACKAGE_YEAR(timer->aim_date);
	rtc_date_t monthnow = RTC_DATE_UNPACKAGE_MONTH(timer->aim_date);
	rtc_date_t daynow = RTC_DATE_UNPACKAGE_DAY(timer->aim_date);
	rtc_date_t hournow = RTC_DATE_UNPACKAGE_HOUR(timer->aim_date);
	rtc_date_t minnow = RTC_DATE_UNPACKAGE_MIN(timer->aim_date);
	rtc_date_t secnow = RTC_DATE_UNPACKAGE_SEC(timer->aim_date);
	rtc_date_t msnow = RTC_DATE_UNPACKAGE_MS(timer->aim_date);

	// 倒计时：重新计数
	if (timer->type == RTC_TIMER_TYPE_COUNT_DOWN)
	{
		timer->aim_date = rtc_date_add(rtc_get_now_date(), timer->set_date);
	}
	// 一天的周期性（非周期性）闹钟
	else if (timer->type == RTC_TIMER_TYPE_TARGET_DAY)
	{
		// 超过预订的时间
		if (RTC_DATE_COMPARE(rtc_get_now_date(), timer->aim_date) >= 0)
		{
			// 今天的闹钟时间
			yearnow = rtc_get_now_date_each(1);
			monthnow = rtc_get_now_date_each(2);
			daynow = rtc_get_now_date_each(3);
			rtc_date_t today_aim = RTC_DATE_PACKAGE(yearnow, monthnow, daynow, hournow, minnow, secnow, msnow);

			// 今天闹钟也已经过去
			if (RTC_DATE_COMPARE(rtc_get_now_date(), today_aim) >= 0)
			{
				// 预测下一次时间
				rtc_base_t yearnew, monthnew, daynew;
				rtc_next_time_day_period(yearnow, monthnow, daynow, &yearnew, &monthnew, &daynew);
				timer->set_date = RTC_DATE_PACKAGE((rtc_date_t)yearnew, (rtc_date_t)monthnew, (rtc_date_t)daynew,
												   hournow, minnow, secnow, msnow);
				timer->aim_date = timer->set_date;
			}
			// 今天闹钟未过去，直接把今天闹钟时间当做目标时间
			else
			{
				timer->set_date = today_aim;
				timer->aim_date = timer->set_date;
			}
		}
	}
	// 一个月的周期性（非周期性）闹钟
	else if (timer->type == RTC_TIMER_TYPE_TARGET_MONTH)
	{
		// 超过预订的时间
		if (RTC_DATE_COMPARE(rtc_get_now_date(), timer->aim_date) >= 0)
		{
			// 本月的闹钟时间
			yearnow = rtc_get_now_date_each(1);
			monthnow = rtc_get_now_date_each(2);
			rtc_date_t current_month_aim = RTC_DATE_PACKAGE(yearnow, monthnow, hournow, daynow, minnow, secnow, msnow);

			// 本月的闹钟也超过去了
			if (RTC_DATE_COMPARE(rtc_get_now_date(), current_month_aim) >= 0)
			{
				// 预测下一个时间
				rtc_base_t yearnew, monthnew, daynew;
				rtc_next_time_month_period(yearnow, monthnow, daynow, &yearnew, &monthnew, &daynew);
				timer->set_date = RTC_DATE_PACKAGE((rtc_date_t)yearnew, (rtc_date_t)monthnew, (rtc_date_t)daynew,
												   hournow, minnow, secnow, msnow);
				timer->aim_date = timer->set_date;
			}
			// 本月的闹钟还未过去
			else
			{
				timer->set_date = current_month_aim;
				timer->aim_date = timer->set_date;
			}
		}
	}
	else
	{
		/* code */
	}
}

// 初始化目标时间
// 创建定时器时调用
// 倒计时：重新倒计时
// 闹钟：超时的话则重新预测下一次超时时间
static bool rtc_timer_aimdate_init(rtc_timer_t *timer)
{
	if (timer == NULL)
	{
		system_printf("***************************timer is null\r\n");
		return false;
	}

	// 倒计时
	if (timer->type == RTC_TIMER_TYPE_COUNT_DOWN)
	{
		// timer->countdownvalue = rtc_ms_interval(timer->set_date, (rtc_date_t)0);
		timer->aim_date = rtc_date_add(rtc_get_now_date(), timer->set_date);
		return true;
	}
	// 闹钟
	else
	{
		// 单次定时
		if (timer->flag == RTC_TIMER_FLAG_ONE_SHOT)
		{
			// 当前时间超过闹钟时间
			if (RTC_DATE_COMPARE(rtc_get_now_date(), timer->aim_date) >= 0)
			{
				system_printf("***************************init time is before the current time \r\n");
				return false;
			}
			else
			{
				// do nothing 直接使用create中使用的aim date
				return true;
			}
		}
		// 周期性定时
		else
		{
			// 预测下一个时间
			rtc_timer_aimdate_forecast(timer);
			return true;
		}
	}
	return true;
}

// 定时器超时以后的处理函数
static void rtc_timer_timeout(rtc_timer_t *timer)
{
	if (TimerListHead == NULL || timer == NULL)
	{
		system_printf("***************************timer is null\r\n");
		return;
	}

	rtc_timer_listhead_remove();

	// 如果是周期性定时，则重新预测目标时间
	if (timer->flag == RTC_TIMER_FLAG_PERIODIC)
	{
		rtc_timer_aimdate_forecast(timer);
		// 插入链表
		rtc_timer_list_insert(timer);
	}
	// 如果是单次定时，则free
	else if (timer->flag == RTC_TIMER_FLAG_ONE_SHOT)
	{
		vPortFree(timer);
	}
	rtc_timer_set_alarm(TimerListHead);
}

rtc_date_t rtc_timer_date_create(rtc_date_t year, rtc_date_t month, rtc_date_t day,
								 rtc_date_t hour, rtc_date_t min, rtc_date_t sec, rtc_date_t ms)
{
	rtc_date_t date = RTC_DATE_PACKAGE(year, month, day, hour, min, sec, ms);

	return date;
}

rtc_timer_t *rtc_timer_create(const char *name,
							  void (*timeout)(void *parameter),
							  void *parameter,
							  rtc_date_t date,
							  enum rtc_timer_type type,
							  enum rtc_timer_flag flag)
{
	rtc_timer_t *timer;

	timer = pvPortMalloc(sizeof(rtc_timer_t));
	memset(timer, '\0', sizeof(rtc_timer_t));

	strncpy(timer->name, name, RTC_TIMER_NAME_MAX_LEN);
	timer->timeout = timeout;
	timer->parameter = parameter;
	timer->set_date = date;
	timer->aim_date = date;
	timer->type = type;
	timer->flag = flag;
	timer->status = RTC_TIMER_STATUS_INACTIVE;

	rt_list_init(timer);

	return timer;
}

// 开始定时器
// 倒计时：重新定时
// 闹钟：若已经超时则重新预测下一次超时时间
bool rtc_timer_start(rtc_timer_t *timer)
{
	if (timer == NULL)
	{
		system_printf("***************************timer is null\r\n");
		return false;
	}

	if (timer->status == RTC_TIMER_STATUS_READY || timer->status == RTC_TIMER_STATUS_RUNNING)
	{
		system_printf("***************************timer is started\r\n");
		return false;
	}

	// 设置aim date
	// 定时器刚创建为激活，初始化aim_date
	if (timer->status == RTC_TIMER_STATUS_INACTIVE)
	{
		if (rtc_timer_aimdate_init(timer))
		{
			timer->status = RTC_TIMER_STATUS_READY;
		}
		else
		{
			return false;
		}
	}
	else // 激活以后的start,是stop以后的start
	{
		rtc_timer_aimdate_forecast(timer);

		timer->status = RTC_TIMER_STATUS_READY;
	}

	rtc_timer_list_insert(timer);
	rtc_timer_set_alarm(TimerListHead);

	return true;
}

// 停止定时器
// 倒计时：计数不保存，下次start重新倒计时
// 闹钟：不保存，下次start重新预测定时时间
void rtc_timer_stop(rtc_timer_t *timer)
{
	if (timer == NULL || TimerListHead == NULL)
	{
		system_printf("***************************timer is null\r\n");
		return;
	}

	if (timer == TimerListHead)
	{
		rtc_timer_listhead_remove();
		rtc_timer_set_alarm(TimerListHead);
	}
	else
	{
		rt_list_remove(timer);
	}

	timer->status = RTC_TIMER_STATUS_STOP;
}

// 定时器挂起
// 倒计时如果正在运行，则需要保存剩余计数值
void rtc_timer_suspend(rtc_timer_t *timer)
{
	if (TimerListHead == NULL || timer == NULL)
	{
		system_printf("***************************timer suspend fail\r\n");
		return;
	}

	// 移除定时器
	if (timer == TimerListHead)
	{
		rtc_timer_listhead_remove();
		rtc_timer_set_alarm(TimerListHead);
	}
	else
	{
		rt_list_remove(timer);
	}

	timer->status = RTC_TIMER_STATUS_SUSPEND;

	// 倒计时正在运行，定时器需要保存剩余的计数值
	if (timer->type == RTC_TIMER_TYPE_COUNT_DOWN && timer->status == RTC_TIMER_STATUS_RUNNING)
	{
		rtc_base_t seconds = rtc_seconds_interval(rtc_get_now_date(), timer->aim_date);

		timer->set_date_remain = RTC_DATE_PACKAGE((rtc_date_t)0, (rtc_date_t)0, (rtc_date_t)0,
												  (rtc_date_t)0, (rtc_date_t)0, (rtc_date_t)seconds, (rtc_date_t)0);
	}
}
// 定时器唤醒
// 倒计时：加载剩余计数值
void rtc_timer_resume(rtc_timer_t *timer)
{
	if (TimerListHead == NULL || timer == NULL)
	{
		system_printf("***************************timer resume fail\r\n");
		return;
	}

	if (timer->status != RTC_TIMER_STATUS_SUSPEND)
	{
		system_printf("***************************timer is not in suspend status\r\n");
		return;
	}

	// 倒计时有剩余计数值
	if (timer->type == RTC_TIMER_TYPE_COUNT_DOWN && timer->set_date_remain != 0)
	{
		timer->aim_date = rtc_date_add(rtc_get_now_date(), timer->set_date_remain);
		timer->set_date_remain = 0;
	}
	// 倒计时没有剩余计数值
	else if (timer->type == RTC_TIMER_TYPE_COUNT_DOWN)
	{
		timer->aim_date = rtc_date_add(rtc_get_now_date(), timer->set_date);
	}
	// 其他定时器，重新预测aim_date
	else
	{
		rtc_timer_aimdate_forecast(timer);
	}

	timer->status = RTC_TIMER_STATUS_READY;
	rtc_timer_list_insert(timer);
	rtc_timer_set_alarm(TimerListHead);
}

// 改变系统时间
void rtc_timer_systime_change_pre(void)
{
	if (TimerListHead == NULL)
	{
		return;
	}

	// 倒计时正在运行，定时器需要保存剩余的计数值
	if (TimerListHead->type == RTC_TIMER_TYPE_COUNT_DOWN)
	{
		// rtc_date_t now = rtc_get_now_date();
		rtc_base_t seconds = rtc_seconds_interval(rtc_get_now_date(), TimerListHead->aim_date);

		TimerListHead->set_date_remain = RTC_DATE_PACKAGE((rtc_date_t)0, (rtc_date_t)0, (rtc_date_t)0,
						 (rtc_date_t)0, (rtc_date_t)0, (rtc_date_t)seconds, (rtc_date_t)0);
	}
}

void rtc_timer_systime_change_after(void)
{
	if (TimerListHead == NULL)
	{
		return;
	}

	rtc_base_t len = rt_list_len(TimerListHead);

	rtc_base_t i;

	rtc_timer_t *timer = TimerListHead;

	for (i = 0; i < len; i++)
	{
		// 表头是倒计时
		if (timer->type == RTC_TIMER_TYPE_COUNT_DOWN && timer == TimerListHead)
		{
			timer->aim_date = rtc_date_add(rtc_get_now_date(), timer->set_date_remain);
			timer->set_date_remain = 0;
		}
		// 其他位置的倒计时
		else if (timer->type == RTC_TIMER_TYPE_COUNT_DOWN)
		{
			timer->aim_date = rtc_date_add(rtc_get_now_date(), timer->set_date);
		}
		// 其他定时器，重新预测aim_date
		else
		{
			rtc_timer_aimdate_forecast(timer);
		}

		// timer->status = RTC_TIMER_STATUS_READY;
		// rtc_timer_list_insert(timer);

		timer = timer->next;
	}

	rtc_timer_set_alarm(TimerListHead);
}

// 删除定时器，销毁内存
void rtc_timer_delete(rtc_timer_t *timer)
{
	if (TimerListHead == NULL || timer == NULL)
	{
		system_printf("***************************timer suspend fail\r\n");
		return;
	}

	// 移除定时器
	if (timer == TimerListHead)
	{
		rtc_timer_listhead_remove();
		// 重新设置alarm时间
		rtc_timer_set_alarm(TimerListHead);
	}
	else
	{
		rt_list_remove(timer);
	}

	// 内存释放
	vPortFree(timer);
}
#if 0
static void callbacktest(void *parameter)
{
	system_printf("###################callback\r\n");
}
static int cmd_addtimer(cmd_tbl_t *t, int argc, char *argv[])
{
	rtc_date_t a = atoi(argv[1]); // 年
	rtc_date_t b = atoi(argv[2]); // 月
	rtc_date_t c = atoi(argv[3]); // 日
	rtc_date_t d = atoi(argv[4]); // 时
	rtc_date_t e = atoi(argv[5]); // 分
	rtc_date_t f = atoi(argv[6]); // 秒
	rtc_date_t g = atoi(argv[7]); // 毫秒

	enum rtc_timer_type type = atoi(argv[8]); // 定时方式：0：倒计时   1：一天定时 2：一个月定时
	enum rtc_timer_flag flag = atoi(argv[9]); // 周期性还是单次  0：单次  1：周期性

	rtc_date_t date1 = rtc_timer_date_create(a, b, c, d, e, f, g);

	rtc_timer_t *timer1 = rtc_timer_create("timer1", callbacktest, NULL, date1, type, flag);

	rtc_timer_start(timer1);

	rtc_timer_list_show();

	return CMD_RET_SUCCESS;
}
SUBCMD(set, addtimer, cmd_addtimer, "set addtimer", "set addtimer");

static int cmd_timer(cmd_tbl_t *t, int argc, char *argv[])
{
	rtc_base_t index = atoi(argv[2]);
	rtc_timer_t *cur = TimerListHead;
	while (index--)
	{
		cur = cur->next;
	}

	if (strcmp(argv[1], "start") == 0)
	{
		rtc_timer_start(cur);
	}
	else if (strcmp(argv[1], "stop") == 0)
	{
		rtc_timer_stop(cur);
	}
	else if (strcmp(argv[1], "suspend") == 0)
	{
		rtc_timer_suspend(cur);
	}
	else if (strcmp(argv[1], "resume") == 0)
	{
		rtc_timer_resume(cur);
	}
	else if (strcmp(argv[1], "show") == 0)
	{
		rtc_timer_list_show();
	}

	return CMD_RET_SUCCESS;
}
SUBCMD(set, timer, cmd_timer, "set cmd_control", "set cmd_control");
#endif

void rtc_timer_isr(void)
{
	if (TimerListHead == NULL)
	{
		return;
	}

	rtc_date_t nowdate = rtc_get_now_date();

	// 超时
	if (RTC_DATE_COMPARE(nowdate, TimerListHead->aim_date) >= 0)
	{
		// 执行回调函数
		rtc_timer_excute_callback(TimerListHead->timeout, TimerListHead->parameter);
		// 执行超时处理
		rtc_timer_timeout(TimerListHead);
	}
	else
	{
		return;
	}

	// 将同一时刻到达的定时器全部执行
	while (TimerListHead != NULL && (RTC_DATE_COMPARE(nowdate, TimerListHead->aim_date) >= 0))
	{
		// 执行回调函数
		rtc_timer_excute_callback(TimerListHead->timeout, TimerListHead->parameter);
		// 执行超时处理
		rtc_timer_timeout(TimerListHead);
	}

#if 1
	rtc_base_t hour = RTC_DATE_UNPACKAGE_HOUR(nowdate);
	rtc_base_t min = RTC_DATE_UNPACKAGE_MIN(nowdate);
	rtc_base_t sec = RTC_DATE_UNPACKAGE_SEC(nowdate);
	rtc_base_t msec = RTC_DATE_UNPACKAGE_MS(nowdate);

	// system_printf("\r\n\r\n***************************alarm int come: %d %d %d %d\r\n", hour, min, sec, msec);
	// rtc_timer_list_show();
#endif
}

/*****************************************************************************************************/
//
//                                     参数保存
//
/*****************************************************************************************************/

#include <string.h>
// static void show(rtc_timer_t *cur)
// {
// 	system_printf("[%s]:", cur->name);
// 	rtc_timer_date_show(cur->set_date);
// 	rtc_timer_date_show(cur->aim_date);
// 	rtc_timer_status_show(cur);
// 	rtc_timer_type_flag_show(cur);
// 	system_printf("\r\n");
// }

void rtc_timer_list_nv_save(void)
{

	int length = rt_list_len(TimerListHead);
	char nv_str[8];
	int retlen = snprintf(nv_str, 8, "%d", length);
	ef_set_env_blob(RTC_TIMER_LIST_LEN, nv_str, retlen - 1); // 保存链表长度

	rtc_base_t i = 0;

	rtc_timer_t *cur = TimerListHead;

	rtc_timer_t *start = pvPortMalloc(sizeof(rtc_timer_t) * length);
	memset(start, '\0', sizeof(rtc_timer_t) * length);

	rtc_timer_t *p = start;

	for (i = 0; i < length; i++)
	{
		memcpy(p, cur, sizeof(rtc_timer_t));
		cur = cur->next;
		// system_printf("save----");
		// show(p);
		p += 1;
	}

	ef_set_env_blob(RTC_TIMER_LIST, start, sizeof(rtc_timer_t) * length);

	vPortFree(start);
}

void rtc_timer_list_nv_read(void)
{

	int i;
	char nv_str[8];
	int ret = ef_get_env_blob(RTC_TIMER_LIST_LEN, nv_str, 8, NULL);

	int len = atoi(nv_str);

	system_printf("ret :%d  lengh:%d \r\n", ret, len);

	rtc_timer_t *start = NULL;

	size_t getlen = 0;

	if (len != 0)
	{
		start = pvPortMalloc(sizeof(rtc_timer_t) * len);
		memset(start, '\0', sizeof(rtc_timer_t) * len);
		ef_get_env_blob(RTC_TIMER_LIST, start, sizeof(rtc_timer_t) * len, &getlen);
		system_printf("getlen:%d  each%d\r\n", getlen, sizeof(rtc_timer_t));
	}
	else
	{
		return;
	}

	if (len == 1)
	{
		TimerListHead = start;
		rt_list_init(TimerListHead);
	}
	else
	{
		for (i = 0; i < len; i++)
		{
			rtc_timer_t *newtimer = (rtc_timer_t *)pvPortMalloc(sizeof(rtc_timer_t));

			memcpy(newtimer, (start + i), sizeof(rtc_timer_t));

			rt_list_init(newtimer);

			if (i == 0)
			{
				TimerListHead = newtimer;
			}
			else
			{
				rt_list_insert_before(TimerListHead, newtimer);
			}

			// system_printf("read----");
			// show(newtimer);
		}
		vPortFree(start);
	}
}

#if 0
static int cmd_nv(cmd_tbl_t *t, int argc, char *argv[])
{
	int a = atoi(argv[1]); // 年

	if (a == 0)
	{
		rtc_timer_list_nv_save();
	}
	else
	{
		rtc_timer_list_nv_read();
	}

	return CMD_RET_SUCCESS;
}
SUBCMD(set, nv, cmd_nv, "set cmd_nv", "set cmd_nv");
#endif

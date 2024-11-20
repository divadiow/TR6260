/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: sntp_tr.c   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        liuwei
 * Date:          2019-4-24
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

#include "system.h"
#include "drv_rtc.h"
#include "sntp.h"



/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define  IS_LEAP_YEAR(year) (year%400==0 || (year%4==0&&year%100!=0))
#define  SEC_NUM_DAY		(24*3600)
#define  DAY_NUM_YEAR		(31+28+31+30+31+30+31+31+30+31+30+31)
#define  SEC_NUM_YEAR		(DAY_NUM_YEAR*SEC_NUM_DAY)
#define  DAY_NUM_LEAP_YEAR	(DAY_NUM_YEAR+1)
#define  SEC_NUM_LEAR_YEAR	(DAY_NUM_LEAP_YEAR*SEC_NUM_DAY)

/****************************************************************************
* 	                                           Local Types
****************************************************************************/

/****************************************************************************
* 	                                           Local Constants
****************************************************************************/
extern const char monthDays[13];
extern const char *week[7];
static unsigned int sntp_period=SNTP_UPDATE_DELAY;//ms, Must not be beolw 60 seconds by specification
static char timezone=7; // GMT+08:00, -12,-11...-1,0,1...11,12
static char server[SNTP_MAX_SERVERS][32];

#define SNTP_SERVER		"pool.ntp.org"

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
static unsigned int get_local_time(void)
{
	unsigned int total_sec = 0;
    unsigned int num;

	struct tm cur_time;
	cur_time = rtc_get_system_time();

    for(num=1970; num<cur_time.tm_year; num++)
	{
		if(IS_LEAP_YEAR(num))
			total_sec += SEC_NUM_LEAR_YEAR;
		else
			total_sec += SEC_NUM_YEAR;
	}

	for(num=1; num<cur_time.tm_mon; num++)
	{
		total_sec += monthDays[num]*SEC_NUM_DAY;
	}

	if(IS_LEAP_YEAR(cur_time.tm_year) && cur_time.tm_mon>2)
		total_sec += SEC_NUM_DAY;

	total_sec += (cur_time.tm_mday-1)*SEC_NUM_DAY;
	total_sec += cur_time.tm_hour*3600;
       total_sec += cur_time.tm_min*60;
       total_sec += cur_time.tm_sec;

	return total_sec;
}

static void set_local_time(unsigned int newtime)
{
	struct tm tblock =
	{
		.tm_wday = 4,
		.tm_year = 1970,
		.tm_mon  = 1,
		.tm_mday = 1,
		.tm_hour = 0,
		.tm_min  = 0,
		.tm_sec  = 0,
	};

	tblock.tm_wday = 4+newtime/SEC_NUM_DAY;
	tblock.tm_wday %= 7;

	for(tblock.tm_year=1970; ; tblock.tm_year++)
	{
		if(IS_LEAP_YEAR(tblock.tm_year))
		{
			if(newtime < SEC_NUM_YEAR)
				break;				
			newtime -= SEC_NUM_LEAR_YEAR;
		}
		else
		{
			if(IS_LEAP_YEAR(tblock.tm_year+1) && newtime<SEC_NUM_LEAR_YEAR)
				break;
			else if(newtime<SEC_NUM_YEAR)
				break;	
			newtime -= SEC_NUM_YEAR;	
		}
	}

	{
		unsigned int month_sec;
		for(tblock.tm_mon=1; tblock.tm_mon<=12; tblock.tm_mon++)
		{
			month_sec = monthDays[tblock.tm_mon]*SEC_NUM_DAY;
			if(tblock.tm_mon==2 && IS_LEAP_YEAR(tblock.tm_year))
				month_sec += SEC_NUM_DAY;
			if(newtime < month_sec)
				break;
			newtime -= month_sec;
		}
	}

	{
		unsigned char day_num;
		day_num = monthDays[tblock.tm_mon];
		if(tblock.tm_mon==2 && IS_LEAP_YEAR(tblock.tm_year))
			day_num++;

		for(tblock.tm_mday=1; tblock.tm_mday<=day_num; tblock.tm_mday++)
		{
			if(newtime < SEC_NUM_DAY)
				break;
			newtime -= SEC_NUM_DAY;
		}
	}

	{
		tblock.tm_hour = newtime/3600;
		tblock.tm_min  = newtime%3600/60;
		tblock.tm_sec  = newtime%3600%60;
	}

	rtc_set_time(tblock);
}

int set_system_time(unsigned int newtime)
{
	if((int)(newtime+timezone*3600) < 0)
        return -1;

	newtime += timezone*3600;
	set_local_time(newtime);

    {
        struct tm cur_time = rtc_get_system_time();
		system_printf("CurrentTime: %s %4d-%02d-%02d %02d:%02d:%02d\n", 
			week[cur_time.tm_wday],
			cur_time.tm_year,
			cur_time.tm_mon,
			cur_time.tm_mday,
			cur_time.tm_hour,
			cur_time.tm_min,
			cur_time.tm_sec);
    }

    return 0;
}


int set_timezone(char tz)
{
	unsigned int cur_time, len;
	char nv_str[4];

	if(tz>13 || tz<-13) return -1;

	memset(nv_str, 0xff, 4);
	cur_time = get_local_time();

	if((int)(cur_time+timezone*3600)<0)
		return -1;

	cur_time -= timezone*3600;

	if((int)(cur_time+tz*3600) < 0)
		return -1;

	cur_time += tz*3600;

	set_local_time(cur_time);
	timezone = tz;

	len = snprintf(nv_str, 4, "%d", tz);

	ef_set_env_blob(NV_SNTP_TIMEZONE, nv_str, len-1);
	//system_printf("set_timezone, val: %s, len: %d\n", nv_str, len);

	if(sntp_enabled())
		sntp_restart();

	return 0;
}

char get_timezone(void)
{
	return timezone;
}

int set_sntp_period(unsigned int period)
{
	char nv_str[16];
	int len;
	
	if(period > 0x3FFFFFFF)
		period = 0x3FFFFFFF;
	else if(period < 60000) 
		period = 60000;

	sntp_period = period;

	len = snprintf(nv_str, 16, "%d", period);

	ef_set_env_blob(NV_SNTP_UPDATEPERIOD, nv_str, len-1);

	if(sntp_enabled())
		sntp_restart();

	return 0;
}

unsigned int get_sntp_period(void)
{
	return sntp_period;
}


int set_servername(unsigned char idx, const char *s)
{
	ef_set_env_blob(NV_SNTP_SERVER, s, strlen(s));
	memcpy(server[idx], s, strlen(s));
       sntp_setservername(idx, s);
	return 0;
}
char *get_servername(unsigned char idx)
{
	return server[idx];
}

void sntp_load_nv(void)
{
	int ret;
	char nv_str[16];

	ret = ef_get_env_blob(NV_SNTP_TIMEZONE, nv_str, 16, NULL);
	if(ret == 0)
	{
		timezone = 0;
	}
	else
	{
		nv_str[ret] = 0;
		timezone = atoi(nv_str);
	}
	
	ret = ef_get_env_blob(NV_SNTP_UPDATEPERIOD, nv_str, 16, NULL);
	if(ret == 0)
	{
		sntp_period = SNTP_UPDATE_DELAY;
	}
	else
	{
		nv_str[ret] = 0;
		sntp_period = atoi(nv_str);
	}

	ret = ef_get_env_blob(NV_SNTP_SERVER, server[0], 32, NULL);
	if(ret == 0)
	{
		memset(server[0], 0, 32);
		memcpy(server[0], SNTP_SERVER, sizeof(SNTP_SERVER));
	}

	sntp_setservername(0, server[0]);
}


static int set_sntp_fun(cmd_tbl_t *t, int argc, char *argv[])
{
    if(argc > 1)
    {
		if(strcmp(argv[1], "on")==0)
		{
			if(sntp_enabled())
				system_printf("SNTP is already enabled\n");
			else
				sntp_start();
		}
		else if(strcmp(argv[1], "off")==0)
		{
			if(sntp_enabled())
				sntp_stop();
			else
				system_printf("SNTP is already disabled\n");
		}
	    else if(strcmp(argv[1], "tz")==0)
	    {
	        char tz = atoi(argv[2]);
	        set_timezone(tz);
	    }
	    else if(strcmp(argv[1], "period")==0)
	    {
	        unsigned int prd = atoi(argv[2]);
	        set_sntp_period(prd*1000);
	    }
	    else if(strcmp(argv[1], "server")==0 && argc==4)
	    {
	        
			ip_addr_t addr;
			unsigned char id;
			
			id = atoi(argv[2]);
			if(id >= SNTP_MAX_SERVERS)
				goto sntp_help;
			
            memset(server[id], 0, 32);
            memcpy(server[id], argv[3], MIN(31, strlen(argv[3])));
            
            if(ip4addr_aton(server[id], &addr))
                sntp_setserver(id, &addr);
            else
	            set_servername(id, server[id]);
	    }
        else
            goto sntp_help;
    }
	else
sntp_help:
    {
        unsigned char id;
        unsigned int ip=0;
        const ip_addr_t *pip = NULL;
        system_printf("set sntp on|off|tz|period|server\n");
        system_printf("================================\n");
		system_printf("SNTP: %s\n", sntp_enabled()?"Enabled":"Disabled");
		system_printf("TimeZone: %s%d\n", timezone>0?"+":"", timezone);
        system_printf("UpdatePeriod: %ds\n", sntp_period/1000);
		system_printf("OperateMode: %s\n",sntp_getoperatingmode()?"listen":"poll");
		system_printf("ServerNumber: %d\n",SNTP_MAX_SERVERS);
		for(id=0; id<SNTP_MAX_SERVERS; id++)
		{
		    pip = sntp_getserver(id);
            if(pip) ip = pip->addr;
            else    ip = 0;
			system_printf("Server%d: %d.%d.%d.%d, %s, %s\n", id+1,ip&0xff,(ip>>8)&0xff,(ip>>16)&0xff,ip>>24,sntp_getservername(id),sntp_getreachability(id)?"success":"idle or try");
		}
        system_printf("================================\n");
    }

	return CMD_RET_SUCCESS;
}
SUBCMD(set,   sntp, set_sntp_fun, "", "");


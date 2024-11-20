/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name: nv_config.h   
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       v0.1
 * Author:        wangxia
 * Date:          2019-3-26
 * History 1:      
 *     Date: 2019-12-18
 *     Version:
 *     Author: wangxia
 *     Modification:  add product information
 * History 2: 
 *     Date: 2020-01-15
 *     Version:
 *     Author: wangxia
 *     Modification:  add SLL info and aliyun info
  ********************************************************************************/

#ifndef _NV_CONFIG_H
#define _NV_CONFIG_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
****************************************************************************/
/*
#define NV_KEY_FACTORY_INFOR   				"NV_KEY_FACTORY_INFOR"
#define NV_KEY_DEVICE_INFOR   				"NV_KEY_DEVICE_INFOR"
#define NV_KEY_SYSTEM_INFOR   				"NV_KEY_SYSTEM_INFOR"
#define NV_KEY_MQTT_INFOR   				"NV_KEY_MQTT_INFOR"
#define NV_KEY_WIFI_BASE_INFOR				"NV_KEY_WIFI_BASE_INFOR"
#define NV_KEY_WIFI_INFOR					"NV_KEY_WIFI_INFOR"
#define NV_KEY_WIFI_MODEM_INFOR				"NV_KEY_WIFI_MODEM_INFOR"
*/
#define RTC_TIMER_LIST 					"RTC_TIMER_LIST"
#define RTC_TIMER_LIST_LEN 				"RTC_TIMER_LIST_LEN"





//product information
#define NV_PRODUCT_MODELSMS				"ModelSms"
#define NV_PRODUCT_VENDORINFO			"VendorInfo"
#define NV_PRODUCT_VERSIONINNER			"VersionInner"
#define NV_PRODUCT_VERSIONEXT			"VersionExt"
#define NV_PRODUCT_HAEDWAREVER			"HardVer"
#define NV_PRODUCT_DEVICEID				"DeviceId"


//System information
#define NV_PSM_FLAG						"PsmFlag"
#define NV_PSM_TYPE						"PSMSleepType"	//0:lightsleep	1:modemsleep
#define NV_PSM_TIME						"PsmSleepTime"	//0-100 /ms

#define NV_PSM_OPTION                   "PsmOption"
#define NV_WTD_FLAG						"WtdFlag"
#define NV_OTA_ADDR						"OTAAdd"
#define NV_DL_FLAG						"DlFlag"
#define NV_STARTUP_TYPE					"StartUpType"
#define NV_SYSMSG_DEF					"SysMsgDefine"

//WTD
#define NV_WTD_INTRPERIOD				"WtdIntrPeriod"

// UART
#define NV_UART1_CONFIG                 "Uart1Config"

//SNTP
#define NV_SNTP_TIMEZONE				"SntpTimeZone"
#define NV_SNTP_UPDATEPERIOD			"SntpUpdatePeriod"
#define NV_SNTP_SERVER					"SntpServer"

//WIFI
#define NV_WIFI_STA_MAC					"MAC"
#define NV_WIFI_AP_MAC					"ApMac"
#define NV_WIRELESS_MODE				"WirelessMode"
#define NV_WIFI_RATE_SET                "RateSet"
#define NV_WIFI_STA_SSID                "StaSSID"
#define NV_WIFI_STA_PWD                 "StaPW"
#define NV_WIFI_STA_BSSID               "StaBSSID"
#define NV_WIFI_STA_PMK                 "PMK"
#define NV_WIFI_STA_CHANNEL             "StaChannel"
#define NV_WIFI_OP_MODE                 "WifiMode"
#define NV_WIFI_COUNTRY                 "CountryCode"
#define NV_WIFI_START_CHANNEL           "StartChannel"
#define NV_WIFI_TOTAL_CHANNEL_NUMBEL    "TotalChannelNumber"
#define NV_WIFI_AUTO_SET_COUNTRY        "AutoSetCountry"
#define NV_PCI                          "PCI"

#define NV_WIFI_DHCPC_EN                "DHCPC_EN"
#define NV_WIFI_DHCPS_EN                "DHCPS_EN"
#define NV_WIFI_DHCPS_IP                 "DHCPS_IP"
#define NV_WIFI_DHCPS_LEASE_TIME        "DHCPS_LeaseTime"

#define NV_WIFI_AP_IP                   "AP_IP"

#define NV_WIFI_AP_GATEWAY              "AP_Gateway"
#define NV_WIFI_AP_NETMASK              "AP_NETMASK"

#define NV_WIFI_STA_IP                  "STA_IP"
#define NV_WIFI_STA_GATEWAY             "STA_Gateway"
#define NV_WIFI_STA_NETMASK             "STA_NETMASK"

#define NV_TCPIP_DNS_SERVER0            "NvTcpipDnsServer0"
#define NV_TCPIP_DNS_SERVER1            "NvTcpipDnsServer1"
#define NV_DNS_AUTO_CHANGE              "NvDnsAutoChange"




#define NV_WIFI_BW_MODE					"BandWidth"
#define NV_WIFI_BW_MODE_20M   			0
#define NV_WIFI_BW_MODE_40M   			1
#define NV_WIFI_BW_MODE_AUTO  			2


#define NV_WIFI_IP 						"wifiip"
#define NV_WIFI_AUTO_CONN				"StaAutoConn"//auto connect when power on


//soft ap
#define NV_AP_SSID                		"ApSSID"
#define NV_AP_PWD                		"ApPWD"
#define NV_AP_CHANNEL                 	"ApChannel"
#define NV_AP_AUTHMOD               	"ApAuthMode"
#define NV_AP_MAX_STA_NUM               "ApMaxStaNum"
#define NV_AP_HIDDEN_SSID             	"ApHiddenSsid"

//MAIN LDO info
#define NV_MAIN_LDO  					"MainLDO"
#define NV_MAIN_LDO_FLAG 				"MainLDOFlag"
#define NV_BUCK_1V45					"BUCK1V45"
#define NV_BUCK_1V45_FLAG 				"BUCK1V45Flag"

//AliYun info
#define NV_ALIYUN_PD_KEY  				"AliyunPdKey"
#define NV_ALIYUN_PD_SECRET  			"AliyunPdSecret"
#define NV_ALIYUN_DEV_NAME  			"AliyunDevName"
#define NV_ALIYUN_DEV_SECRET  			"AliyunDevSecret"
#define NV_ALIYUN_SVR_LOCA  			"AliyunServerLocation"

//for light
#define  LIGHT_DATA	     				"AliyunLightdata"
#define  LIGHT_REBOOT    				"AliyunLightRebootCnt"

//for translink
#define NV_TRANSLINK_EN                 "TransLinkEn"
#define NV_TRANSLINK_IP                 "TransLinkIp"
#define NV_TRANSLINK_PORT               "TransLinkPort"
#define NV_TRANSLINK_UDPPORT            "TransLinkLocalPort"
#define NV_TRANSLINK_TYPE               "TransLinkType"

//SSL
#define NV_SSL_AUTH_TYPE  				"SslAuthenticationType"

//TuyaYun info
#define FIRMWARE_KEY                    "firmware_key"
#define PRODUCT_PID                     "product_pid"

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

#endif/*_NV_CONFIG_H*/


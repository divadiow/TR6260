#ifndef LOCAL_OTA_H
#define LOCAL_OTA_H

/****************************************************************************
 * Included Files
 ****************************************************************************/


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*user config */
#define CONFIG_LOCAL_OTA_SSID                   "local_ota_test"
#define CONFIG_LOCAL_OTA_PASSWD                 "12345678"
#define CONFIG_LOCAL_OTA_CHANNEL                7
#define CONFIG_LOCAL_OTA_CONNET_TIMEOUT         20   /*unit s*/

typedef enum
{    
    OTA_UPDATE_NONE,                // 未开始升级
    OTA_UPDATE_SUCCESS,             // OTA升级成功 
    OTA_UPDATE_FAIL,                // OTA升级失败 

    OTA_UPDATE_MAX
}OTA_UPDATE_STATUS;

typedef void(* local_ota_cb)(void);
typedef struct
{
    local_ota_cb ota_success_cb;
    local_ota_cb ota_fail_cb;
    local_ota_cb ota_action_cb;
}local_ota_cb_t;


typedef struct
{
    unsigned char dest_ver[4];
    unsigned int  fail_reason;
}local_ota_reason_t;
/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/

/************************************************************************************
 * Name: local_ota_start
 *
 * Description:
 *	ota handle
 *
 * Returned Value:
 * 	-1:handle error
 * 	 0:handle success
 *
 ************************************************************************************/
int local_ota_start(void);


/************************************************************************************
 * Name: local_ota_scan_wifi
 *
 * Description:
 *	scan ota wifi
 *
 * Returned Value:
 * 	-1:scan error
 * 	 0:scan success
 *
 ************************************************************************************/
int local_ota_scan_wifi(void);


/************************************************************************************
 * Name: local_ota_connect_wifi
 *
 * Description:
 *	scan ota wifi
 *
 * Returned Value:
 * 	-1:scan error
 * 	 0:scan success
 *
 ************************************************************************************/
int local_ota_connect_wifi(void);

/************************************************************************************
 * Name: local_ota_version_info_register
 *
 * Description:
 *	register version
 *
 * Returned Value:
 * 	-1:error
 * 	 0:success
 *
 ************************************************************************************/
int local_ota_version_info_register(char * version);


/************************************************************************************
 * Name: local_ota_cb_register
 *
 * Description:
 *	register resgister ota callback
 *
 * Returned Value:
 * 	-1:error
 * 	 0:success
 *
 ************************************************************************************/
int local_ota_cb_register(local_ota_cb_t * cb);
#endif










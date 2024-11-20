#ifndef SC_H_
#define SC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * result of smartconfig
 */
typedef struct
{
    char pwd[64+1];                  /* password */
    char ssid[32+1];                 /* ssid,end with '\0' */
    unsigned char pwd_length;   /* length of password */
    unsigned char ssid_length;  /* length of ssid */
    unsigned char random;       /* when wifi connect successfully,can send the random to phone */
    unsigned char reserved;     /* reserved,now it's stored crc checksum */
} sc_result_t;

typedef enum {
    SC_STATUS_CONTINUE = 0,    /**< device is receiving packet and decoding it */
    SC_STATUS_LOCK_CHANNEL,    /**< Find the target channel */
    SC_STATUS_GOT_SSID_PSWD,   /**< Got SSID and password of target AP */
    SC_STATUS_TIMEOUT,         /**< smartconfig process timeout,when got in this status,smartconfig process should be restarted */
    SC_STATUS_STOP,            /**< smartconfig process stoped */
} smartconfig_status_t;

/**
  * @brief  The callback of SmartConfig, executed when smart-config status changed.
  *
  * @param  status  Status of SmartConfig:
  *    - SC_STATUS_LOCK_CHANNEL : find the channel of target AP,pdata is a pointer of channel value.
  *    - SC_STATUS_GOT_SSID_PSWD : pdata is a pointer of wifi_sta_config_t.
  *    - SC_STATUS_TIMEOUT: smartconfig timeout
  *    - SC_STATUS_STOP: smartconfig process is stoped
  *    - otherwise : parameter void *pdata is NULL.
  * @param  pdata  According to the different status have different values.
  *
  */
typedef void (*sc_callback_t)(smartconfig_status_t status, void *pdata);

/**
  * @brief  Get the version of SmartConfig.
  *
  * @return
  *     - SmartConfig version const char.
  */
const char* smartconfig_version(void);

/**
  * @brief     Start SmartConfig, config device to connect AP. You need to broadcast information by phone APP.
  *            Device sniffer special packets from the air that containing SSID and password of target AP.
  *
  * @attention 1. This API can be called in station or softAP-station mode.
  * @attention 2. Can not call smartconfig_start twice before it finish, please call
  *               smartconfig_stop firstly.
  *
  * @param     cb  SmartConfig callback function.
  *
  * @return
  *     - 0: succeed
  *     - others: fail
  */
int smartconfig_start(sc_callback_t cb);

/**
  * @brief     Stop SmartConfig, free the memory taken by smartconfig_start.
  *
  * @attention Whether connect to AP succeed or not, this API should be called to free
  *            memory taken by smartconfig_start.
  *
  * @return
  *     - 0: succeed
  *     - others: fail
  */
int smartconfig_stop(void);

/**
  * @brief     Set timeout of SmartConfig process.
  *
  * @param     time_s  timeout second.
  *
  * @return
  *     - 0: succeed
  *     - others: fail
  */
int smartconfig_set_timeout(uint8_t time_s);

#ifdef __cplusplus
}
#endif

#endif 


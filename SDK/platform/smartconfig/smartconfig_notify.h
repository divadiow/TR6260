#ifndef SC_NOTIFY_H
#define SC_NOTIFY_H

#ifdef __cplusplus
extern "C" {
#endif

#define SC_NOTIFY_TASK_PRIORITY             2          /*!< Priority of sending smartconfig notify task */
#define SC_NOTIFY_TASK_STACK_SIZE           512        /*!< Stack size of sending smartconfig notify task */

#define SC_NOTIFY_SERVER_PORT               33333      /*!< smartconfig UDP port of server on cellphone*/

#define SC_NOTIFY_MSG_LEN                   7          /*!< Length of smartconfig message */

#define SC_NOTIFY_MAX_COUNT                 100         /*!< Maximum count of sending smartconfig notify message */


/**
 * @brief Smartconfig parameters passed to sc_notify_send call.
 */
typedef struct sc_notify {
    uint8_t random;            /*!< Smartconfig acknowledge random to be sent */
    uint8_t mac[6];           /*!< MAC address of station */
} sc_notify_t;

/**
  * @brief  Send smartconfig notify message to cellphone.
  *
  * @param  param: smartconfig parameters;
  */
void sc_notify_send(sc_notify_t *param);

/**
  * @brief  Stop sending smartconfig notify message to cellphone.
  *
  */
void sc_notify_send_stop(void);

#ifdef __cplusplus
}
#endif
#endif

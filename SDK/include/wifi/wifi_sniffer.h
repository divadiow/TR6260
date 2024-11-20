// Copyright 2018-2019 trans-semi inc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef _WIFI_SNIFFER_H_
#define _WIFI_SNIFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "system.h"

/** @brief Received packet radio metadata header, this is the common header at the beginning of all promiscuous mode RX callback buffers */
typedef struct {
    union {
        struct {
            // word 0 : RX Vector Head0
            uint32_t reserved0     : 1;
            uint32_t smt           : 1;
            uint32_t format        : 2;  // 0=11a/g, 1=11b, 2=Non HT duplicate, 3=HT(11n, fixed format)
            uint32_t stbc          : 1;
            uint32_t aggregation   : 1;
            uint32_t gi_type       : 1;
            uint32_t bandwidth     : 1;  // 0=20, 1=40
            uint32_t reserved1     : 1;
            uint32_t mcs           : 7;
            uint32_t cfo_estimate  : 13;
            uint32_t reserved2     : 3;

            /// word 1 : RX Vector Head1
            uint32_t reserved3     : 8;
            uint32_t length        : 20;
            uint32_t reserved4     : 4;

            /// word 2 : RX Vector Tail
            uint32_t rssi          : 8;
            uint32_t rcpi          : 8;
            uint32_t snr             : 9;
            uint32_t reserved5     : 7;

            /// word 3
            uint32_t mpdu_length    : 14;
            uint32_t center_freq    : 10;
            uint32_t mpdu_count     : 8;

            /// word 4
            uint32_t ack_policy     : 2;
            uint32_t reserved7      : 6;
            uint32_t cur_active_vif : 1;
            uint32_t received_vif   : 1;
            uint32_t reserved8        : 3;
            uint32_t cipher_type    : 3;
            uint32_t reserved9      : 1;
            uint32_t rxnpayload_av_ind      : 1;
            uint32_t reserved10     : 1;
            uint32_t eof_ind        : 1;
            uint32_t ndp_ind        : 1;
            uint32_t agg            : 1;
            uint32_t protection     : 1;
            uint32_t error_delimiter: 1;
            uint32_t error_icv      : 1;
            uint32_t error_mic      : 1;
            uint32_t error_key      : 1;
            uint32_t error_length   : 1;
            uint32_t error_match    : 1;
            uint32_t error_crc      : 1;
            uint32_t error_sequence : 1;
            uint32_t okay           : 1;

            /// word 5
            uint32_t timestamp;
        };
        uint32_t   rx_vector[6];
    };
} wifi_pkt_rx_ctrl_t;

/** @brief Payload passed to 'buf' parameter of promiscuous mode RX callback.
 */
typedef struct {
    wifi_pkt_rx_ctrl_t rx_ctrl; /**< metadata header */
    uint8_t *payload;       /**< Data or management payload */
} wifi_promiscuous_pkt_t;

/**
  * @brief Promiscuous frame type
  *
  * Passed to promiscuous mode RX callback to indicate the type of parameter in the buffer.
  *
  */
 
typedef enum {
    WIFI_PKT_MGMT,  /**< Management frame, indicates 'buf' argument is wifi_promiscuous_pkt_t */
    WIFI_PKT_CTRL,  /**< Control frame, indicates 'buf' argument is wifi_promiscuous_pkt_t */
    WIFI_PKT_DATA,  /**< Data frame, indiciates 'buf' argument is wifi_promiscuous_pkt_t */
    WIFI_PKT_NULL,  /**< invalid frame type */
} wifi_promiscuous_pkt_type_t;

#define WIFI_PROMIS_FILTER_MASK_ALL         (0xFFFFFFFF)  /**< filter all packets */
#define WIFI_PROMIS_FILTER_MASK_MGMT        (1)           /**< filter the packets with type of WIFI_PKT_MGMT */
#define WIFI_PROMIS_FILTER_MASK_CTRL        (1<<1)        /**< filter the packets with type of WIFI_PKT_CTRL */
#define WIFI_PROMIS_FILTER_MASK_DATA        (1<<2)        /**< filter the packets with type of WIFI_PKT_DATA */
#define WIFI_PROMIS_FILTER_MASK_DATA_MPDU   (1<<3)        /**< filter the MPDU which is a kind of WIFI_PKT_DATA */
#define WIFI_PROMIS_FILTER_MASK_DATA_AMPDU  (1<<4)        /**< Todo: filter the AMPDU which is a kind of WIFI_PKT_DATA */

/** @brief Mask for filtering different packet types in promiscuous mode. */
typedef struct {
    uint32_t filter_mask; /**< OR of one or more filter values WIFI_PROMIS_FILTER_* */
} wifi_promiscuous_filter_t;


/**
  * @brief The RX callback function in the promiscuous mode. 
  *        Each time a packet is received, the callback function will be called.
  *
  * @param buf  Data received. Type of data in buffer (wifi_promiscuous_pkt_t) indicated by 'type' parameter.
  * @param type  promiscuous packet type.
  *
  */
typedef void (* wifi_promiscuous_cb_t)(void *buf, int len, wifi_promiscuous_pkt_type_t type);

/**
  * @brief Register the RX callback function in the promiscuous mode.
  *
  * Each time a packet is received, the registered callback function will be called.
  *
  * @param cb  callback
  *
  * @return
  *    - TRS_OK: succeed
  *    - TRS_ERR_WIFI_NOT_INIT: WiFi is not initialized
  */
trs_err_t wifi_set_promiscuous_rx_cb(wifi_promiscuous_cb_t cb);

/**
  * @brief     Enable the promiscuous mode.
  *
  * @param     en  false - disable, true - enable
  *
  * @return
  *    - TRS_OK: succeed
  *    - TRS_ERR_WIFI_NOT_INIT: WiFi is not initialized
  */
trs_err_t wifi_set_promiscuous(bool en);

/**
  * @brief     Get the promiscuous mode.
  *
  * @param[out] en  store the current status of promiscuous mode
  *
  * @return
  *    - TRS_OK: succeed
  *    - TRS_ERR_WIFI_NOT_INIT: WiFi is not initialized
  *    - TRS_ERR_INVALID_ARG: invalid argument
  */
trs_err_t wifi_get_promiscuous(bool *en);

/**
  * @brief Enable the promiscuous mode packet type filter.
  *
  * @note The default filter is to filter all packets except WIFI_PKT_MISC
  *
  * @param filter the packet type filtered in promiscuous mode.
  *
  * @return
  *    - TRS_OK: succeed
  *    - TRS_ERR_WIFI_NOT_INIT: WiFi is not initialized
  */
trs_err_t wifi_set_promiscuous_filter(wifi_promiscuous_filter_t *filter);

/**
  * @brief     Get the promiscuous filter.
  *
  * @param[out] filter  store the current status of promiscuous filter
  *
  * @return
  *    - TRS_OK: succeed
  *    - TRS_ERR_WIFI_NOT_INIT: WiFi is not initialized
  *    - TRS_ERR_INVALID_ARG: invalid argument
  */
trs_err_t wifi_get_promiscuous_filter(wifi_promiscuous_filter_t *filter);


#ifdef __cplusplus
}
#endif

#endif /*_WIFI_SNIFFER_H_*/

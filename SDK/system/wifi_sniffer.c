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

#include "system.h"
#include "driver.h"
#include "driver_wifi.h"
#include "wifi_sniffer.h"
#include <stdbool.h> 


static wifi_promiscuous_cfg_t  promiscuous_cfg = {0};

trs_err_t wifi_set_promiscuous_rx_cb(wifi_promiscuous_cb_t cb)
{
	promiscuous_cfg.promisc_cb = cb;
	return TRS_OK;
}

trs_err_t wifi_set_promiscuous(bool en)
{
    wifi_promiscuous_filter_t filter;
    
	if(en == true) {
		system_modem_api_set_promiscuous_enable(0,1);
		system_modem_api_set_promiscuous_enable(1,1);
		set_promiscuous_mode(0,1);
		set_promiscuous_mode(1,1);
	} else {
		set_promiscuous_mode(0,0);
		set_promiscuous_mode(1,0);
		system_modem_api_set_promiscuous_enable(0,0);
		system_modem_api_set_promiscuous_enable(1,0);
        memset(&filter, 0, sizeof(filter));
        system_modem_api_set_promiscuous_filter(&filter);
	}
	return TRS_OK;
}

trs_err_t wifi_get_promiscuous(bool *en)
{
	bool ret = (get_promiscuous_mode(0) & get_promiscuous_mode(1));
	if(en == NULL){
		return TRS_ERR_INVALID_ARG;
	} 
	memcpy(en,&ret,sizeof(bool));
	return TRS_OK;
}

trs_err_t wifi_set_promiscuous_filter(wifi_promiscuous_filter_t *filter)
{
	if(filter == NULL){
		return TRS_ERR_INVALID_ARG;
	}
	memcpy(&promiscuous_cfg.filter,filter,sizeof(wifi_promiscuous_filter_t));
    system_modem_api_set_promiscuous_filter(filter);
	return TRS_OK;
}

trs_err_t wifi_get_promiscuous_filter(wifi_promiscuous_filter_t *filter)
{
	if(filter == NULL){
		return TRS_ERR_INVALID_ARG;
	}
	memcpy(filter,&promiscuous_cfg.filter,sizeof(wifi_promiscuous_filter_t)); 
	return TRS_OK;
}

trs_err_t process_promiscuous_frame(void* rx_head, void* mac_head, uint8_t *frame, uint16_t len)
{
	LMAC_RXHDR * control_head = (LMAC_RXHDR *)rx_head;
	GenericMacHeader *gmh = (GenericMacHeader *)mac_head;
	wifi_promiscuous_pkt_t sniffer_pkt = {0};
	bool match = false;
	wifi_promiscuous_pkt_type_t type = WIFI_PKT_NULL;

	if(promiscuous_cfg.filter.filter_mask & WIFI_PROMIS_FILTER_MASK_MGMT){
		if(gmh->type == 0) {
			type = WIFI_PKT_MGMT;
			goto process;
		}
    }

	if(promiscuous_cfg.filter.filter_mask & WIFI_PROMIS_FILTER_MASK_CTRL){
		if(gmh->type == 1) {
			type = WIFI_PKT_CTRL;
			goto process;
		}
    }

	if(promiscuous_cfg.filter.filter_mask & WIFI_PROMIS_FILTER_MASK_DATA){
		if(gmh->type == 2) {
			type = WIFI_PKT_DATA;
			goto process;
		}
    }
	if(match == false){
		goto drop;
	}

process:
	if(promiscuous_cfg.promisc_cb != NULL){
		memcpy(&sniffer_pkt.rx_ctrl, control_head,sizeof(LMAC_RXHDR));
		sniffer_pkt.payload = frame;
		promiscuous_cfg.promisc_cb(&sniffer_pkt,len,type);
		return 0; 
	}

drop:
	return 0;
}

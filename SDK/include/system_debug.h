#ifndef _SYSTEM_DEBUG_H
#define _SYSTEM_DEBUG_H

typedef struct {
    uint8_t dir; // 0 means rx, 1 means tx.
    uint8_t vif;
    union {
        struct {
            
        }lwip;
        
        struct {
            int8_t rssi;
        }wifi;
    }u;
} debug_rx_tx_info_t;


void lwip_dump_pbuf(void *pkt, debug_rx_tx_info_t *info);
void wifi_dump_frame(void *frame, debug_rx_tx_info_t *info);
#endif

#ifndef UMAC_BCN_MONITOR_H
#define UMAC_BCN_MONITOR_H
#include <stdbool.h>

#if defined(UMAC_BCN_MONITOR)
bool umac_bcn_monitor_start(int vif_id);
void umac_bcn_monitor_stop(int vif_id);
void umac_bcn_monitor_update(int vif_id);
#else
static inline bool umac_bcn_monitor_start(int vif_id) {return true;};
static inline void umac_bcn_monitor_stop(int vif_id) {};
static inline void umac_bcn_monitor_update(int vif_id) {};
#endif //UMAC_BCN_MONITOR

int umac_bcn_bss_mode_update(uint16_t caps, uint8_t *ies, int ie_len);
void umac_bcn_monitor_update_channel(unsigned char channel);
void umac_bcn_monitor_update_speed(unsigned char id, unsigned char len, unsigned char *data);

#endif //UMAC_BCN_MONITOR_H

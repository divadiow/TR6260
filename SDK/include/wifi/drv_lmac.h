#ifndef __DRV_LMAC_H__
#define __DRV_LMAC_H__

void drv_lmac_set_ccm_monitor_interval(int interval);
void drv_lmac_set_base_address(uint32_t address);
void drv_lmac_set_alarm(uint32_t tsf);
void drv_lmac_set_basic_rate(int vif_id, uint32_t value);
uint32_t drv_lmac_get_tsf_lo(int vif_id);
uint32_t drv_lmac_get_tsf_hi(int vif_id);

#endif //__DRV_LMAC_H__

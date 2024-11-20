#define UMAC_PROBE_RESP_H
#if defined(INCLUDE_UMAC_PRESP)
int umac_presp_offl_process(GenericMacHeader *gmh, struct _SYS_BUF *buffer);
void umac_presp_offl_update(uint8_t* frame, uint16_t len);
void umac_presp_start();
void umac_presp_stop();
void umac_presp_set_ssid(uint8_t *ssid, uint8_t len);
void umac_presp_set_ssid_hide(uint8_t hide);

#else
static inline void umac_presp_offl_update(uint8_t* frame, uint16_t len) {}
static inline int umac_presp_offl_process(GenericMacHeader *gmh, struct _SYS_BUF *buffer) {
	return 0;
}
static inline void umac_presp_start() {};
static inline void umac_presp_stop() {};
#endif

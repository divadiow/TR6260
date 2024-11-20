#ifndef DRIVER_NRC_TX
#define DRIVER_NRC_TX

struct nrc_wpa_key;
int nrc_add_sec_hdr(struct nrc_wpa_key *key, uint8_t *pos);
int wifi_drv_raw_xmit(uint8_t vif_id, const uint8_t *frame, const uint16_t len);
int nrc_transmit_from_8023(uint8_t vif_id,uint8_t *frame, const uint16_t len);
int nrc_transmit_from_8023_mb(uint8_t vif_id,uint8_t **frames, const uint16_t len[], int n_frames);

typedef enum {
	ETH_EAPOL_ENC_ENABLE = 0,
	ETH_EAPOL_ENC_DISABLE = 1,
}ETH_EAPOL_ENC_E;
	
void wpa_set_eth_eapol_enc(ETH_EAPOL_ENC_E flag);

#endif // DRIVER_NRC_TX

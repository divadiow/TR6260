/*
 * wlantest - IEEE 802.11 protocol monitoring and testing tool
 * Copyright (c) 2010-2013, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifdef CONFIG_IEEE80211W
#ifndef CCMP_H
#define CCMP_H

/* Management MIC information element (IEEE 802.11w) */
struct ieee80211_mmie {
	u8 element_id;
	u8 length;
	uint16_t key_id;
	u8 sequence_number[6];
	u8 mic[8];
}STRUCT_PACKED;


u8 * ccmp_decrypt(const u8 *tk, const u8 *hdr, const u8 *data,
		  size_t data_len, size_t *decrypted_len);
int ccmp_encrypt(const u8 *tk, u8 *frame, size_t len, size_t hdrlen,
		  u8 *pn, int keyid, u8 *output, size_t maxsize);


int ccmp_encrypt_V1(const u8 *tk, u8 *frame, size_t len, size_t hdrlen, u8 *output, size_t maxsize);

int broadcast_mgmt_integrity_set_mmie(u8  *igtk, int keyid, uint64_t pn, 
		char *input, uint16_t len, char *output, int maxsize);
int broadcast_mgmt_integrity_verify(u8  *igtk, int keyid, uint64_t pn, char *input, uint16_t len, uint64_t *update_pn);


#endif /* CCMP_H */
#endif /* CONFIG_IEEE80211W */

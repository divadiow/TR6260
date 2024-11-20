/*
 * CTR with CBC-MAC Protocol (CCMP)
 * Copyright (c) 2010-2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifdef CONFIG_IEEE80211W

#include "utils/includes.h"

#include "utils/common.h"
#include "common/ieee802_11_defs.h"
#include "aes.h"
#include "aes_wrap.h"
#include "ccmp.h"

#define WLAN_EID_MMIE   (76)

#define BYTE_TO_UNIT64(a)  ((a[0] << 0) || (a[1] << 8) || (a[2] << 16) || (a[3] << 24) || (a[4] << 32) || (a[5] << 40))
#define UNIT64_TO_BYTE(a, b)  {b[0] = (a >> 0) & 0xff; b[1] = (a >> 8) & 0xff;b[2] = (a >> 16) & 0xff;b[3] = (a >> 24) & 0xff;b[4] = (a >> 32) & 0xff;b[5] = (a >> 40) & 0xff;}

#if 0
#undef wpa_hexdump

static void wpa_hexdump(int level, char *str, u8 *p, int len)
{
	int i;
	system_printf("%s[%d]\r\n", str, len);

	for(i = 0; i < len; i++)
	{
		system_printf("%02x ", p[i]);	
	}
	
	system_printf("\r\n");
}
#endif


static void ccmp_aad_nonce(const struct ieee80211_hdr *hdr, const u8 *data,
			   u8 *aad, size_t *aad_len, u8 *nonce)
{
	u16 fc, stype, seq;
	int qos = 0, addr4 = 0;
	u8 *pos;

	nonce[0] = 0;

	fc = le_to_host16(hdr->frame_control);
	stype = WLAN_FC_GET_STYPE(fc);
	if ((fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) ==
	    (WLAN_FC_TODS | WLAN_FC_FROMDS))
		addr4 = 1;

	if (WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_DATA) {
		fc &= ~0x0070; /* Mask subtype bits */
		if (stype & 0x08) {
			const u8 *qc;
			qos = 1;
			fc &= ~WLAN_FC_ORDER;
			qc = (const u8 *) (hdr + 1);
			if (addr4)
				qc += ETH_ALEN;
			nonce[0] = qc[0] & 0x0f;
		}
	} else if (WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_MGMT)
		nonce[0] |= 0x10; /* Management */

	fc &= ~(WLAN_FC_RETRY | WLAN_FC_PWRMGT | WLAN_FC_MOREDATA);
	fc |= WLAN_FC_ISWEP;
	WPA_PUT_LE16(aad, fc);
	pos = aad + 2;
	os_memcpy(pos, hdr->addr1, 3 * ETH_ALEN);
	pos += 3 * ETH_ALEN;
	seq = le_to_host16(hdr->seq_ctrl);
	seq &= ~0xfff0; /* Mask Seq#; do not modify Frag# */
	WPA_PUT_LE16(pos, seq);
	pos += 2;

	os_memcpy(pos, hdr + 1, addr4 * ETH_ALEN + qos * 2);
	pos += addr4 * ETH_ALEN;
	if (qos) {
		pos[0] &= ~0x70;
		if (1 /* FIX: either device has SPP A-MSDU Capab = 0 */)
			pos[0] &= ~0x80;
		pos++;
		*pos++ = 0x00;
	}

	*aad_len = pos - aad;

	os_memcpy(nonce + 1, hdr->addr2, ETH_ALEN);
	nonce[7] = data[7]; /* PN5 */
	nonce[8] = data[6]; /* PN4 */
	nonce[9] = data[5]; /* PN3 */
	nonce[10] = data[4]; /* PN2 */
	nonce[11] = data[1]; /* PN1 */
	nonce[12] = data[0]; /* PN0 */
}

u8 * ccmp_decrypt(const u8 *tk, const u8 *hdr, const u8 *data,
		  size_t data_len, size_t *decrypted_len)
{
	u8 aad[30], nonce[13];
	size_t aad_len;
	size_t mlen;
	u8 *plain;

	if (data_len < 8 + 8)
		return NULL;

	plain = os_zalloc(data_len + AES_BLOCK_SIZE);
	if (plain == NULL)
		return NULL;

	mlen = data_len - 8 - 8;

	os_memset(aad, 0, sizeof(aad));
	ccmp_aad_nonce((const struct ieee80211_hdr *)hdr, data, aad, &aad_len, nonce);
	//wpa_hexdump(MSG_DEBUG, "CCMP AAD", aad, aad_len);
	//wpa_hexdump(MSG_DEBUG, "CCMP nonce", nonce, 13);

	if (aes_ccm_ad(tk, 16, nonce, 8, data + 8, mlen, aad, aad_len,
		       data + 8 + mlen, plain) < 0) {
		os_free(plain);
		return NULL;
	}
	//wpa_hexdump(MSG_DEBUG, "CCMP decrypted", plain, mlen);

	*decrypted_len = mlen;
	return plain;
}

int ccmp_encrypt(const u8 *tk, u8 *frame, size_t len, size_t hdrlen,
		  u8 *pn, int keyid, u8 *output, size_t maxsize)
{
	u8 aad[30], nonce[13];
	size_t aad_len, plen;
	u8 *crypt, *pos;
	struct ieee80211_hdr *hdr;
	int result;
	system_printf("ccmp_encrypt before:%d\r\n",sys_now());

	if (len < hdrlen || hdrlen < 24)
		return -1;

	plen = len - hdrlen;
	result = hdrlen + 8 + plen + 8;
	if(result > maxsize)
		return -2;
	
	plen = len - hdrlen;

	crypt = os_zalloc(hdrlen + 8 + plen + 8 + AES_BLOCK_SIZE);
	if (crypt == NULL)
		return -3;

	os_memcpy(crypt, frame, hdrlen);
	hdr = (struct ieee80211_hdr *) crypt;
	hdr->frame_control |= host_to_le16(WLAN_FC_ISWEP);
	pos = crypt + hdrlen;
	*pos++ = pn[0]; /* PN0 */
	*pos++ = pn[1]; /* PN1 */
	*pos++ = 0x00; /* Rsvd */
	*pos++ = 0x20 | (keyid << 6);
	*pos++ = pn[2]; /* PN2 */
	*pos++ = pn[3]; /* PN3 */
	*pos++ = pn[4]; /* PN4 */
	*pos++ = pn[5]; /* PN5 */

	os_memset(aad, 0, sizeof(aad));
	ccmp_aad_nonce(hdr, crypt + hdrlen, aad, &aad_len, nonce);
	wpa_hexdump(MSG_DEBUG, "CCMP AAD", aad, aad_len);
	wpa_hexdump(MSG_DEBUG, "CCMP nonce", nonce, 13);

	if (aes_ccm_ae(tk, 16, nonce, 8, frame + hdrlen, plen, aad, aad_len,
		       pos, pos + plen) < 0) {
		os_free(crypt);
		return -4;
	}

	wpa_hexdump(MSG_DEBUG, "CCMP encrypted", crypt + hdrlen + 8, plen);
	
	os_memcpy(output, crypt, result);
	os_free(crypt);
	system_printf("ccmp_encrypt end:%d\r\n",sys_now());
	system_printf("result = %d\r\n", result);
	return result;
}	
int ccmp_encrypt_V1(const u8 *tk, u8 *frame, size_t len, size_t hdrlen, u8 *output, size_t maxsize)
{
	u8 aad[30], nonce[13];
	size_t aad_len, plen;
	u8 *crypt, *pos;
	struct ieee80211_hdr *hdr;
	int result;

	if (len < hdrlen || hdrlen < 24)
		return -1;
	system_printf("ccmp_encrypt before:%d\r\n",sys_now());
	plen = len - hdrlen - 8;
	result = hdrlen + plen + 8 + 8;
	if(result > maxsize)
		return -2;

	crypt = os_zalloc(hdrlen + 8 + plen + 8 + AES_BLOCK_SIZE);
	if (crypt == NULL)
		return -3;

	os_memcpy(crypt, frame, hdrlen + 8);
	hdr = (struct ieee80211_hdr *) crypt;
	hdr->frame_control |= host_to_le16(WLAN_FC_ISWEP);
	pos = crypt + hdrlen+8;

	os_memset(aad, 0, sizeof(aad));
	ccmp_aad_nonce(hdr, crypt + hdrlen, aad, &aad_len, nonce);
	wpa_hexdump(MSG_DEBUG, "CCMP AAD", aad, aad_len);
	wpa_hexdump(MSG_DEBUG, "CCMP nonce", nonce, 13);

	if (aes_ccm_ae(tk, 16, nonce, 8, frame + hdrlen + 8, plen, aad, aad_len,
		       pos, pos + plen) < 0) {
		os_free(crypt);
		return -4;
	}

	wpa_hexdump(MSG_DEBUG, "CCMP encrypted", crypt + hdrlen + 8, plen);
	
	os_memcpy(output, crypt, result);
	os_free(crypt);
	
	system_printf("ccmp_encrypt end:%d result=%d\r\n",sys_now(),result);
	
	return result;
}	
static int broadcast_mgmt_integrity_get_mic(u8 *igtk, u8 *frame, size_t len, u8 *mic, int mic_maxlen)
{
	struct ieee80211_mmie *mmie;
	char calc_mic[16]={0};
	int  datalen;
	
	if(igtk == NULL || frame == NULL || len == 0 || mic == NULL || mic_maxlen == 0)
		return -1;
	
	char *buf = os_zalloc(len);
	if(buf == NULL)
		return -2;

	os_memcpy(buf, frame, len);
	datalen = len -4;
	
	buf[0] &= 0xc7;
	os_memmove(&buf[2], &buf[4], len - 4);
	os_memmove(&buf[20], &buf[22], len - 22);

	mmie = (struct ieee80211_mmie *)&buf[22];
	os_memset(mmie->mic, 0, sizeof(mmie->mic));

	wpa_hexdump(0, "igtk:", igtk, 16);
	wpa_hexdump(0, "calc_data", buf, datalen);
	if(omac1_aes_128(igtk, buf, datalen, calc_mic) != 0)
	{
		os_free(buf);
		return -4;
	}

	os_free(buf);
	wpa_hexdump(0, "calc_mic", calc_mic, 16);
	os_memcpy(mic, calc_mic, 16 > mic_maxlen ? mic_maxlen: 16);

	return 0;
}
int broadcast_mgmt_integrity_set_mmie(u8 *igtk, int keyid, uint64_t pn, char *input, uint16_t len, char *output, int maxsize)
{
	struct ieee80211_mmie *mmie;
	char calc_mic[16]={0};
	int result, reallen;
	
	if(igtk == NULL || input == NULL || len == 0 || output == NULL || maxsize == 0)
		return -1;

	reallen = sizeof(struct ieee80211_mmie) + len;
	if(maxsize < reallen)
		return -2;
	
	os_memset(output, 0, maxsize);
	os_memcpy(output, input, len);

	mmie = (struct ieee80211_mmie *)&output[len];
	mmie->key_id = keyid;
	mmie->element_id = WLAN_EID_MMIE;
	mmie->length = 16;
	//os_memcpy(mmie->sequence_number, &pn, 6);
	UNIT64_TO_BYTE(pn, mmie->sequence_number);
	system_printf("pn:%lld \r\n", pn);
	wpa_hexdump(0, "num:", mmie->sequence_number, 6);
	result = broadcast_mgmt_integrity_get_mic(igtk, output, reallen, calc_mic, sizeof(calc_mic));
	if(result < 0)
		return -3;

	os_memcpy(mmie->mic, calc_mic, 8); 

	return reallen;
}

int broadcast_mgmt_integrity_verify(u8 *igtk, int keyid, uint64_t pn, char *input, uint16_t len, uint64_t *update_pn)
{
	int ie_offset = offsetof(struct ieee80211_mgmt, u.deauth.variable);
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)input;
	struct ieee80211_mmie *mmie;
	int   calc_datalen, result;
	char *calc_data;
	char calc_mic[16];
	uint64_t rpn;
	wpa_hexdump(0, "mgmt", input, len);
	
	if(len < sizeof(struct ieee80211_mmie) + ie_offset)
		return -1;

	mmie = (struct ieee80211_mmie *)((char *)input + ie_offset);
	system_printf("len=%d ie_offset: %d mmie: element_id:%d key_id:%d\r\n", len, ie_offset, mmie->element_id, mmie->key_id);
	wpa_hexdump(0, "mic:", mmie->mic, 8);
	if(mmie->element_id != WLAN_EID_MMIE || mmie->length != sizeof(struct ieee80211_mmie) - 2 || mmie->key_id != keyid)
		return -2;

	wpa_hexdump(0, "sequence_number:", mmie->sequence_number, 6);
	rpn = BYTE_TO_UNIT64(mmie->sequence_number);
	system_printf("recv pn:%lld local pn:%lld \r\n", rpn, pn);
	if(rpn <= pn)
	{
		*update_pn = ++pn;
		return -3;
	}

	result = broadcast_mgmt_integrity_get_mic(igtk, input, len, calc_mic, sizeof(calc_mic));
	if(result < 0)
	{
		*update_pn = ++pn;
		return -4;
	}

	if(os_memcmp(calc_mic, mmie->mic, 8) != 0)
	{
		*update_pn = ++pn;
		return -5;
	}

	*update_pn = rpn;
	system_printf("%s success !!\r\n", __func__);
	
	return 0;
}

inline int ieee80211_is_pmf_mgmt_frame(struct ieee80211_hdr *hdr)
{
	if(WLAN_FC_GET_TYPE(hdr->frame_control) == WLAN_FC_TYPE_MGMT && (hdr->frame_control & WLAN_FC_ISWEP))
		return 0;

	return -1;
}


void ieee80211_pmf_security_switch(int ignore)
{
	static u8 status = 0;

	if(status == 0 && ignore == 1)
	{
		hal_lmac_security_switch(1); // close security module
		status = 1;
	}
	else if(status == 1 && ignore == 0)
	{
		hal_lmac_security_switch(0); //open security module
		status = 0;
	}
}

#endif /* CONFIG_IEEE80211W */

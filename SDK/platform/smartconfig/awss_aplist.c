/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include "awss_aplist.h"
#include "zconfig_ieee80211.h"


#define ZC_MAX_CHANNEL            (14)
#define ZC_MIN_CHANNEL            (1)
#define MAX_APLIST_NUM      (100)
/* storage to store apinfo */
struct ap_info *zconfig_aplist = NULL;
/* aplist num, less than MAX_APLIST_NUM */
uint8_t zconfig_aplist_num = 0;

#ifndef PTR_NULL
#define PTR_NULL    ((void *)0)
#endif

static char *awss_strncpy(char *dest, const char *src, size_t count)
{
   if ((dest == PTR_NULL) || (src == PTR_NULL)){
       return PTR_NULL;
   }
    
    char *tmp = dest;
    while (count) {
        if ((*tmp = *src) != 0){
            src++;
        }
        tmp++;
        count--;
    }
    return dest;
}

int awss_clear_aplist(void)
{
    memset(zconfig_aplist, 0, sizeof(struct ap_info) * MAX_APLIST_NUM);
    zconfig_aplist_num = 0;

    return 0;
}

int awss_init_ieee80211_aplist(void)
{
    if (zconfig_aplist) {
        return 0;
    }
    zconfig_aplist = (struct ap_info *)os_zalloc(sizeof(struct ap_info) * MAX_APLIST_NUM);
    if (zconfig_aplist == NULL) {
        return -1;
    }
    zconfig_aplist_num = 0;
    return 0;
}

int awss_deinit_ieee80211_aplist(void)
{
    if (zconfig_aplist == NULL) {
        return 0;
    }
    os_free(zconfig_aplist);
    zconfig_aplist = NULL;
    zconfig_aplist_num = 0;
    return 0;
}

struct ap_info *zconfig_get_apinfo(uint8_t *mac)
{
    int i;

    for (i = 1; i < zconfig_aplist_num; i++) {
        if (!memcmp(zconfig_aplist[i].mac, mac, ETH_ALEN)) {
            return &zconfig_aplist[i];
        }
    }

    return NULL;
}

struct ap_info *zconfig_get_apinfo_by_ssid(uint8_t *ssid)
{
    int i;

    for (i = 1; i < zconfig_aplist_num; i ++) {
        if (!strcmp((char *)zconfig_aplist[i].ssid, (char *)ssid)) {
            return &zconfig_aplist[i];
        }
    }

    return NULL;
}

bool zconfig_check_ap_channel(int channel)
{
    int i;

    for (i = 1; i < zconfig_aplist_num; i ++) {
        if (zconfig_aplist[i].channel == channel){
            return true;
        }
    }
    return false;
}

/**
 * save apinfo
 *
 * @ssid: [IN] ap ssid
 * @bssid: [IN] ap bssid
 * @channel: [IN] ap channel
 * @auth: [IN] optional, ap auth mode, like OPEN/WEP/WPA/WPA2/WPAWPA2
 * @encry: [IN], ap encryption mode, i.e. NONE/WEP/TKIP/AES/TKIP-AES
 *
 * Note:
 *     1) if ap num exceed zconfig_aplist[], always save at [0]
 *         but why...I forgot...
 *     2) always update channel if channel != 0
 *     3) if chn is locked, save ssid to zc_ssid, because zc_ssid
 *         can be used for ssid-auto-completion
 * Return:
 *     0/success, -1/invalid params(empty ssid/bssid)
 */

int awss_save_apinfo(uint8_t *ssid, uint8_t *bssid, uint8_t channel, uint8_t auth,
                     uint8_t pairwise_cipher, uint8_t group_cipher, signed char rssi)
{
    int i;

    /* ssid, bssid cannot empty, channel can be 0, auth/encry can be invalid */
    if (!(ssid && bssid)) {
        return -1;
    }

    /* sanity check */
    if (channel > ZC_MAX_CHANNEL || channel < ZC_MIN_CHANNEL) {
        channel = 0;
    }

    if (auth > ZC_AUTH_TYPE_MAX) {
        auth = ZC_AUTH_TYPE_INVALID;
    }

    if (pairwise_cipher > ZC_ENC_TYPE_MAX) {
        pairwise_cipher = ZC_ENC_TYPE_INVALID;
    }
    if (group_cipher > ZC_ENC_TYPE_MAX) {
        group_cipher = ZC_ENC_TYPE_INVALID;
    }

    //FIXME:
    if (pairwise_cipher == ZC_ENC_TYPE_TKIPAES) {
        pairwise_cipher = ZC_ENC_TYPE_AES;    //tods
    }

    /*
     * start from zconfig_aplist[1], leave [0] for temp use
     * if zconfig_aplist[] is full, always replace [0]
     */
    if (!zconfig_aplist_num) {
        zconfig_aplist_num = 1;
    }

    for (i = 1; i < zconfig_aplist_num; i++) {
        // system_printf("assid  %s ssid %s cmp %d\n",zconfig_aplist[i].ssid, ssid,strncmp(zconfig_aplist[i].ssid, (char *)ssid, ZC_MAX_SSID_LEN));
        if (!strncmp(zconfig_aplist[i].ssid, (char *)ssid, ZC_MAX_SSID_LEN)
            && !memcmp(zconfig_aplist[i].mac, bssid, ETH_ALEN)) {
            //FIXME: useless?
            /* found the same bss */

            
            if (!zconfig_aplist[i].channel) {
                zconfig_aplist[i].channel = channel;
            }
            if (zconfig_aplist[i].auth == ZC_AUTH_TYPE_INVALID) {
                zconfig_aplist[i].auth = auth;
            }
            if (zconfig_aplist[i].encry[0] == ZC_ENC_TYPE_INVALID) {
                zconfig_aplist[i].encry[0] = group_cipher;
            }
            if (zconfig_aplist[i].encry[1] == ZC_ENC_TYPE_INVALID) {
                zconfig_aplist[i].encry[1] = pairwise_cipher;
            }

            return 0;//duplicated ssid
        } 
        // else {
        //     system_printf("zconfig_aplist[%d].ssid %s ssid %s\n",i,zconfig_aplist[i].ssid, ssid);
        // }
    }

    if (i < MAX_APLIST_NUM) {
        zconfig_aplist_num ++;
    } else {
        i = 0;    /* [0] for temp use, always replace [0] */
    }
    awss_strncpy((char *)&zconfig_aplist[i].ssid, (const char *)&ssid[0], strlen((const char *)ssid));
    memcpy(&zconfig_aplist[i].mac, bssid, ETH_ALEN);
    zconfig_aplist[i].auth = auth;
    zconfig_aplist[i].rssi = rssi;
    zconfig_aplist[i].channel = channel;
    zconfig_aplist[i].encry[0] = group_cipher;
    zconfig_aplist[i].encry[1] = pairwise_cipher;

    do {
        system_printf("[%d] ssid:%s, mac:%02x%02x%02x%02x%02x%02x, chn:%d, rssi:%d\r\n",
        i, ssid, bssid[0], bssid[1], bssid[2],
        bssid[3], bssid[4], bssid[5], channel,
        rssi > 0 ? rssi - 256 : rssi);
    } while (0);
    /*
     * if chn already locked(zc_bssid set),
     * copy ssid to zc_ssid for ssid-auto-completiont
     */

    return 0;
}

/*
 * [IN] ssid or bssid
 * [OUT] auth, encry, channel
 */
int awss_get_auth_info(uint8_t *ssid, uint8_t *bssid, uint8_t *auth,
                       uint8_t *encry, uint8_t *channel)
{
    uint8_t *valid_bssid = NULL;
    struct ap_info *ap_info = NULL;
    uint8_t zero_mac[ETH_ALEN] = {0};
    /* sanity check */
    if (!bssid || !memcmp(bssid, zero_mac, ETH_ALEN)) {
        valid_bssid = NULL;
    } else {
        valid_bssid = bssid;
    }

    /* use mac or ssid to search apinfo */
    if (valid_bssid) {
        ap_info = zconfig_get_apinfo(valid_bssid);
    } else {
        ap_info = zconfig_get_apinfo_by_ssid(ssid);
    }

    if (!ap_info) {
        return 0;
    }

    if (auth) {
        *auth = ap_info->auth;
    }
    if (encry) {
        *encry = ap_info->encry[1];    /* tods side */
    }
    if (!valid_bssid && bssid) {
        memcpy(bssid, ap_info->mac, ETH_ALEN);
    }
    if (channel) {
        *channel = ap_info->channel;
    }

    return 1;

}

struct ap_info *zconfig_get_apinfo_by_3_byte_mac(uint8_t *last_3_Byte_mac)
{
    int i;
    uint8_t *local_mac;

    if(NULL == last_3_Byte_mac) {
        return NULL;
    }

    for (i = 1; i < zconfig_aplist_num; i++) {
        local_mac = (uint8_t *)(zconfig_aplist[i].mac) + 3;
        if (!memcmp(local_mac, last_3_Byte_mac, ETH_ALEN - 3)) {
            return &zconfig_aplist[i];
        }
    }

    return NULL;
}



#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif

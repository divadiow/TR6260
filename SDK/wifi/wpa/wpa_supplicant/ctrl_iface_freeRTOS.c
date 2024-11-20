#include "utils/includes.h"
#include "utils/common.h"
#include "utils/eloop.h"
#include "config.h"
#include "wpa_supplicant_i.h"
#include "ctrl_iface.h"
#include "system.h"

#ifdef CONFIG_NO_STDOUT_DEBUG
#ifdef wpa_printf
#undef wpa_printf
static void wpa_printf(int level, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	system_vprintf(fmt, ap);
	system_printf("\n");
    va_end(ap);
}
#endif
#endif

static void processCommand(char* cmd);
int ctrl_iface_receive(struct wpa_supplicant *wpa_s, char *cmd);
void wpa_task_exec_cmd(void *eloop_ctx, void *timeout_ctx);

struct ctrl_iface_priv {
	struct wpa_supplicant *wpa_s;
};

struct ctrl_iface_global_priv {
	struct ctrl_iface_priv 	*ctrl_if[NRC_WPA_NUM_INTERFACES];
};


static struct ctrl_iface_global_priv* global_ctrl_if = NULL;

struct wpa_supplicant *wpa_get_ctrl_iface(int vif_id)
{
	 return global_ctrl_if->ctrl_if[vif_id]->wpa_s;
}


int wpa_cmd_receive_str(int vif_id, char *buf)
{
    int buf_len = strlen(buf) + 1;
    char *cli_buf;
    
    if (vif_id >= NRC_WPA_NUM_INTERFACES || !buf || !buf[0])
        return -1;
	if (!global_ctrl_if || !global_ctrl_if->ctrl_if[vif_id])
		return -1;

    cli_buf = os_zalloc(buf_len);
    if (!cli_buf)
        return -1;
    memcpy(cli_buf, buf, strlen(buf));
    eloop_register_timeout(0, 0, wpa_task_exec_cmd, (void *)global_ctrl_if->ctrl_if[vif_id]->wpa_s, 
        (void *)cli_buf);
	return 0;
}

int wpa_cmd_receive(int vif_id, int argc, char *argv[]) {
	int i = 0, j = 0, offset = 0;
    char buf[100] = {0};

	if (argc <= 1)
		return -1;

    for (i = 1; i < argc && offset < sizeof(buf) - 1; ++i) {
        j = 0;
        if (i != 1) {
            buf[offset++] = ' ';
        }
        while(argv[i][j] && offset < sizeof(buf) - 1) {
            buf[offset++] = argv[i][j++];
        }
    }

    return wpa_cmd_receive_str(vif_id, buf);
}
void wpa_task_exec_cmd(void *eloop_ctx, void *timeout_ctx)
{
    struct wpa_supplicant *wpa_s = (struct wpa_supplicant *)eloop_ctx;
    char * buf = (char *)timeout_ctx;

    ctrl_iface_receive(wpa_s, buf);
    os_free(buf);
}


int ctrl_iface_receive(struct wpa_supplicant *wpa_s, char *cmd)
{
	int i = 0;
	char *reply = NULL, *p = cmd;
	size_t reply_len = 0;

	//wpa_printf(MSG_DEBUG, "%s() cmd: %s", __func__, cmd);

	if (!wpa_s)
		return 0;

	while(*p != ' ' && *p != 0 ) {
		*p = (char) toupper((int) *p);
		p++;
	}

	reply = wpa_supplicant_ctrl_iface_process(wpa_s, cmd, &reply_len);

    if (reply)
    	reply[reply_len] = 0;
	//wpa_printf(MSG_ERROR, "reply_len: %d ", (int)reply_len);

	if(reply_len == 1) {
		wpa_printf(MSG_DEBUG, "FAIL");
	} else if(reply_len == 2) {
		wpa_printf(MSG_DEBUG, "OK");
		os_free(reply);
		return 0;
	} else if(reply) {
	    if (reply_len > PRINT_BUFFER_SIZE ) {
    		int pos = 0;
	        char atom[PRINT_BUFFER_SIZE+1];
    	    while (pos < reply_len) {
        		int len = reply_len - pos;
	            if (len > PRINT_BUFFER_SIZE)
    	        	len = PRINT_BUFFER_SIZE;
        	    memcpy(atom, reply + pos, len);
	    	    atom[len] = '\0';
    	        wpa_printf(MSG_ERROR, atom);
        	    pos += len;
	        }
    	} else {
        	reply[reply_len] = 0;
            wpa_printf(MSG_ERROR, reply);
        }
	} else {
		wpa_printf(MSG_DEBUG, "UNKNOWN");
	}

    if(reply != NULL ) {
    	os_free(reply);
    }

	return 0;
}

struct ctrl_iface_priv* wpa_supplicant_ctrl_iface_init(struct wpa_supplicant *wpa_s)
{
	struct ctrl_iface_priv* priv = NULL;
	int vif_id = 0;

	if (!global_ctrl_if) {
		wpa_printf(MSG_ERROR, "Failed to init ctrl_iface.");
		return NULL;
	}

	if (os_strcmp(NRC_WPA_INTERFACE_NAME_0, wpa_s->ifname) == 0)
		vif_id = 0;
	else if (os_strcmp(NRC_WPA_INTERFACE_NAME_1, wpa_s->ifname) == 0)
		vif_id = 1;
	else {
		wpa_printf(MSG_ERROR, "%s: Unknown interface name (%s)", __func__,
			wpa_s->ifname);
	}

	priv = (struct ctrl_iface_priv *) os_zalloc(sizeof(*priv));

	if (!priv) {
		wpa_printf(MSG_ERROR, "Failed to allocate ctrl_iface");
		return NULL;
	}
	priv->wpa_s = wpa_s;
	global_ctrl_if->ctrl_if[vif_id] = priv;

	return priv;
}

struct ctrl_iface_global_priv* wpa_supplicant_global_ctrl_iface_init(struct wpa_global *global)
{
	if (global_ctrl_if) {
		os_free(global_ctrl_if);
		global_ctrl_if = NULL;
	}

	global_ctrl_if = (struct ctrl_iface_global_priv *) os_zalloc(sizeof(*global_ctrl_if));

	if(!global_ctrl_if) {
		wpa_printf(MSG_ERROR, "Failed to allocate ctrl_iface global");
		return NULL;
	}

	return global_ctrl_if;
}

#include "system_def.h"
#include "bss.h"
#include "wpa_common.h"
#include "wpa.h"

#define WIFI_SSID_MAX_LEN   (32 + 1)
#define WIFI_PWD_MAX_LEN    (64) // pwd string support max length is 63

typedef enum {
	CIPHER_NONE = 0,
	CIPHER_WEP40,
	CIPHER_WEP104,
	CIPHER_TKIP,
	CIPHER_CCMP,
} wifi_cipher_mode_e;

typedef struct {
    uint8_t   ssid[WIFI_SSID_MAX_LEN];
	uint8_t   pwd[WIFI_PWD_MAX_LEN];
	uint8_t   ssid_len;
	uint8_t   pwd_len;
	uint8_t   auth;
	uint8_t   cipher;
	uint8_t  channel;
	uint8_t   bssid[6];
	int8_t    rssi;
} wifi_info_t;

typedef enum {
    AUTH_OPEN = 0,      /**< authenticate mode : open */
    AUTH_WEP,           /**< authenticate mode : WEP */
    AUTH_WPA_PSK,       /**< authenticate mode : WPA_PSK */
    AUTH_WPA2_PSK,      /**< authenticate mode : WPA2_PSK */
    AUTH_WPA_WPA2_PSK,  /**< authenticate mode : WPA_WPA2_PSK */
    AUTH_MAX
} wifi_auth_mode_e;

int wpa_get_scan_num(void)
{
	struct wpa_supplicant *wpa_s;	
	wpa_s = wpa_get_ctrl_iface(0);
	return dl_list_len(&wpa_s->bss_id);
}

sys_err_t wpa_get_bss_mode(struct wpa_bss *bss, uint8_t *auth, uint8_t *cipher)
{
	const u8 *ie, *ie2;
	struct wpa_ie_data data;

	if(!bss || !auth || !cipher)
		return -1;

	ie  = wpa_bss_get_vendor_ie(bss, WPA_IE_VENDOR_TYPE);
	ie2 = wpa_bss_get_ie(bss, WLAN_EID_RSN);

	if(ie || ie2)
	{
		if(ie && !ie2)
		{
			if (wpa_parse_wpa_ie(ie, ie[1]+2, &data) < 0)
				return -1;

			if(data.pairwise_cipher & WPA_CIPHER_CCMP)
				*cipher = CIPHER_CCMP;
			else
				*cipher = CIPHER_TKIP;

			*auth = AUTH_WPA_PSK;
		}
		else if(!ie && ie2)
		{
			if (wpa_parse_wpa_ie(ie2, ie2[1]+2, &data) < 0)
				return -1;
			
			if(data.pairwise_cipher & WPA_CIPHER_CCMP)
				*cipher = CIPHER_CCMP;
			else
				*cipher = CIPHER_TKIP;

			*auth = AUTH_WPA2_PSK;
		}
		else// if(ie && ie2)
		{
			if (wpa_parse_wpa_ie(ie2, ie2[1]+2, &data) < 0)
				return -1;

			if(data.pairwise_cipher & WPA_CIPHER_CCMP)
				*cipher = CIPHER_CCMP;
			else
				*cipher = CIPHER_TKIP;
			
			*auth = AUTH_WPA_WPA2_PSK;
		}
	}
    else if(bss->caps & IEEE80211_CAP_PRIVACY)
    {
	    *auth   = AUTH_WEP;
		*cipher = CIPHER_WEP40;
    }
	else
	{
		*auth   = AUTH_OPEN;
		*cipher = CIPHER_NONE;
	}

	return SYS_OK;
}

sys_err_t wpa_get_scan_result(unsigned int index, wifi_info_t *info)
{
    struct wpa_bss *bss;
	struct wpa_supplicant *wpa_s;
	unsigned char i=0;

	if(!info)
		return SYS_ERR_INVALID_ARG;
	
	memset((char*)info, 0, sizeof(*info));
	
	wpa_s = wpa_get_ctrl_iface(0);
	
	dl_list_for_each(bss, &wpa_s->bss_id, struct wpa_bss, list_id)
	{
        if(i++ == index)//if(bss->id == index)
        {
			memcpy(info->bssid, bss->bssid, 6);
			memcpy(info->ssid, bss->ssid, bss->ssid_len);
			info->ssid_len = bss->ssid_len;
			info->channel = system_modem_api_mac80211_frequency_to_channel(bss->freq);
			info->rssi = (char)(bss->level);
			wpa_get_bss_mode(bss, &info->auth, &info->cipher);
			return SYS_OK;
		}
	}
	
	return SYS_ERR_NOT_FOUND;
}


sys_err_t wpa_get_wifi_info(wifi_info_t *info)
{
	struct wpa_supplicant *wpa_s;
	struct wpa_ssid *ssid;
	struct wpa_bss  *bss;

	wpa_s = wpa_get_ctrl_iface(0);
	bss   = wpa_s->current_bss;
	if(wpa_s->wpa_state == WPA_COMPLETED)
		ssid = wpa_s->current_ssid;
	else
		ssid = wpa_s->conf->ssid;

	if((!bss) || (!ssid) || (!info))
		return SYS_ERR_INVALID_ARG;

	memset((char*)info, 0, sizeof(*info));

	memcpy(info->bssid, bss->bssid, 6);
	memcpy(info->ssid, bss->ssid, bss->ssid_len);
	info->ssid_len = bss->ssid_len;
	info->channel = system_modem_api_mac80211_frequency_to_channel(bss->freq);
	info->rssi = bss->level;
	wpa_get_bss_mode(bss, &info->auth, &info->cipher);
	if(ssid->passphrase)
	{
		info->pwd_len = strlen(ssid->passphrase);
		memcpy(info->pwd,  ssid->passphrase, info->pwd_len);
	}
	else if(ssid->wep_key_len[0] == 5 || ssid->wep_key_len[0] == 13)
	{
		info->pwd_len = ssid->wep_key_len[0];
		memcpy(info->pwd,  &ssid->wep_key[0], info->pwd_len);
	}
	return SYS_OK;
}

void at_get_ap_ssid_passwd_chanel(char *ap_ssid, char *passwd, uint8_t *channel)
{
    struct wpa_supplicant *wpa_s_ap = wpa_get_ctrl_iface(1);
	if(!wpa_s_ap)
		return;
	struct wpa_bss  *bss = wpa_s_ap->current_bss;
	struct wpa_ssid *ssid = wpa_s_ap->conf->ssid;
	
    if(ssid->ssid[0] == 0)
    {
        memset(ap_ssid, 0 ,WIFI_SSID_MAX_LEN);
        memset(passwd, 0, WIFI_PWD_MAX_LEN);
    }
    else
    {
        memcpy(ap_ssid, ssid->ssid, ssid->ssid_len);
        memcpy(passwd, ssid->passphrase, strlen(ssid->passphrase));
    }

	*channel = system_modem_api_mac80211_frequency_to_channel(ssid->frequency);
}


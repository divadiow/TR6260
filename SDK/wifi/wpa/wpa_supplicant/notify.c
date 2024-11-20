/*
 * wpa_supplicant - Event notifications
 * Copyright (c) 2009-2010, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifdef INCLUDE_STANDALONE
#include "system_event.h"
#endif

#include "utils/includes.h"

#include "utils/common.h"
#include "common/wpa_ctrl.h"
#include "config.h"
#include "wpa_supplicant_i.h"
#include "wps_supplicant.h"
#include "rsn_supp/wpa.h"
#include "driver_i.h"
#include "scan.h"
#include "sme.h"
#include "notify.h"

#include "bss.h"

#ifdef INCLUDE_STANDALONE
void report_gtk_rekey_event(struct wpa_supplicant *wpa_s, int flag)
{
    system_event_t evt;
    struct wpa_bss *bss = wpa_s->current_bss;
    int channel, authmode, wpa;

    os_memset(&evt, 0, sizeof(evt));

    evt.event_id = SYSTEM_EVENT_STA_GTK_REKEY;
    evt.event_info.in_out_rekey.in_out = flag;
    if (wpa_s->ifname && strlen(wpa_s->ifname) == 5 /*wlanX*/) {
        evt.vif = wpa_s->ifname[4] - '0';
    }

    sys_event_send(&evt);   
}

static void wifi_event_report(struct wpa_supplicant *wpa_s, enum wpa_states new_state, enum wpa_states old_state)
{
    static char gtk_rekey = 0;
    char rekey_fail = 0;
	system_event_t evt;
	struct wpa_bss *bss = wpa_s->current_bss;
	int channel, authmode, wpa;
    
    if (old_state == new_state)
        return;

	os_memset(&evt, 0, sizeof(evt));
	evt.event_id = SYSTEM_EVENT_WPA_INTERNAL_STATE;
	evt.event_info.wpa_internal_state.state = new_state;
	if (wpa_s->ifname && strlen(wpa_s->ifname) == 5 /*wlanX*/) {
        evt.vif = wpa_s->ifname[4] - '0';
    }
	sys_event_send(&evt); 
	
    if (WPA_GROUP_HANDSHAKE == new_state && WPA_COMPLETED == old_state) {
        gtk_rekey = 1;
        report_gtk_rekey_event(wpa_s, gtk_rekey);
        return; //gtk rekey
    } 
    
    if (gtk_rekey) {
        if (WPA_GROUP_HANDSHAKE != new_state && WPA_COMPLETED != new_state) {
            gtk_rekey = 0; //gtk rekey fail.
            rekey_fail = 1;
        } else {
            gtk_rekey = (WPA_GROUP_HANDSHAKE == new_state ? 1 : 0);
            report_gtk_rekey_event(wpa_s, gtk_rekey);
            return; //gtk rekey success, or still in gtk rekey.
        }
    }

	if (!rekey_fail && new_state != WPA_COMPLETED && old_state != WPA_COMPLETED)
        return;

    //report connect/disconnect event
	os_memset(&evt, 0, sizeof(evt));
	if (new_state == WPA_COMPLETED) {
        if (bss) {
            evt.event_id = SYSTEM_EVENT_STA_CONNECTED;
            evt.event_info.connected.channel = bss->freq >= 2412 ? (1 + (bss->freq - 2412) / 5) : 0;
        } else {
            evt.event_id = SYSTEM_EVENT_AP_START;
        }
	} else {
		evt.event_id = bss ? SYSTEM_EVENT_STA_DISCONNECTED : SYSTEM_EVENT_AP_STOP;
		evt.event_info.disconnected.reason = (wpa_s->disconnect_reason < 0 ?
				(uint8_t)(-1 * wpa_s->disconnect_reason) : 0);
	}

    if (bss) {
		evt.event_info.disconnected.ssid_len = bss->ssid_len;
		memcpy(&evt.event_info.disconnected.ssid[0], bss->ssid,
				evt.event_info.disconnected.ssid_len);
		memcpy(&evt.event_info.disconnected.bssid[0], bss->bssid,
				sizeof(evt.event_info.disconnected.bssid));
    }

    if (wpa_s->ifname && strlen(wpa_s->ifname) == 5 /*wlanX*/) {
        evt.vif = wpa_s->ifname[4] - '0';
    }

	sys_event_send(&evt); 
}
#endif

void wpas_notify_sta_4way_hs_failed(struct wpa_supplicant *wpa_s)
{
    system_event_t evt;

    os_memset(&evt, 0, sizeof(evt));
    evt.event_id = SYSTEM_EVENT_STA_4WAY_HS_FAIL;
    if (wpa_s->ifname && strlen(wpa_s->ifname) == 5 /*wlanX*/) {
        evt.vif = wpa_s->ifname[4] - '0';
    }
    sys_event_send(&evt);
}

void wpas_notify_state_changed(struct wpa_supplicant *wpa_s,
			       enum wpa_states new_state,
			       enum wpa_states old_state)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	/* notify the old DBus API */
	//wpa_supplicant_dbus_notify_state_change(wpa_s, new_state,
	//					old_state);

	/* notify the new DBus API */
	//wpas_dbus_signal_prop_changed(wpa_s, WPAS_DBUS_PROP_STATE);

#ifdef CONFIG_FST
	if (wpa_s->fst && !is_zero_ether_addr(wpa_s->bssid)) {
		if (new_state == WPA_COMPLETED)
			fst_notify_peer_connected(wpa_s->fst, wpa_s->bssid);
		else if (old_state >= WPA_ASSOCIATED &&
			 new_state < WPA_ASSOCIATED)
			fst_notify_peer_disconnected(wpa_s->fst, wpa_s->bssid);
	}
#endif /* CONFIG_FST */

	sme_state_changed(wpa_s);

#ifdef ANDROID
	wpa_msg_ctrl(wpa_s, MSG_INFO, WPA_EVENT_STATE_CHANGE
		     "id=%d state=%d BSSID=" MACSTR " SSID=%s",
		     wpa_s->current_ssid ? wpa_s->current_ssid->id : -1,
		     new_state,
		     MAC2STR(wpa_s->bssid),
		     wpa_s->current_ssid && wpa_s->current_ssid->ssid ?
		     wpa_ssid_txt(wpa_s->current_ssid->ssid,
				  wpa_s->current_ssid->ssid_len) : "");
#endif /* ANDROID */

#ifdef INCLUDE_STANDALONE
	wifi_event_report(wpa_s, new_state, old_state);
#endif
}

#ifdef INCLUDE_STANDALONE
void notify_wifi_ready_event()
{
    system_event_t evt;

    memset(&evt, 0, sizeof(evt));
    evt.event_id = SYSTEM_EVENT_WIFI_READY;
    sys_event_send(&evt);
}
#endif

void wpas_notify_disconnect_reason(struct wpa_supplicant *wpa_s)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_prop_changed(wpa_s, WPAS_DBUS_PROP_DISCONNECT_REASON);
}


void wpas_notify_assoc_status_code(struct wpa_supplicant *wpa_s)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_prop_changed(wpa_s, WPAS_DBUS_PROP_ASSOC_STATUS_CODE);
	system_event_t evt;
    struct wpa_bss *bss = wpa_s->current_bss;
    int channel, authmode, wpa;

    os_memset(&evt, 0, sizeof(evt));

    evt.event_id = SYSTEM_EVENT_STA_ASSOC_REJECT;
	
    if (bss) {
		evt.event_info.disconnected.ssid_len = bss->ssid_len;
		memcpy(&evt.event_info.disconnected.ssid[0], bss->ssid,
				evt.event_info.disconnected.ssid_len);
		memcpy(&evt.event_info.disconnected.bssid[0], bss->bssid,
				sizeof(evt.event_info.disconnected.bssid));
    }

	evt.event_info.disconnected.reason = wpa_s->assoc_status_code; 
    if (wpa_s->ifname && strlen(wpa_s->ifname) == 5 /*wlanX*/) {
        evt.vif = wpa_s->ifname[4] - '0';
    }

    sys_event_send(&evt); 
}


void wpas_notify_network_changed(struct wpa_supplicant *wpa_s)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_prop_changed(wpa_s, WPAS_DBUS_PROP_CURRENT_NETWORK);
}


void wpas_notify_ap_scan_changed(struct wpa_supplicant *wpa_s)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_prop_changed(wpa_s, WPAS_DBUS_PROP_AP_SCAN);
}


void wpas_notify_bssid_changed(struct wpa_supplicant *wpa_s)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_prop_changed(wpa_s, WPAS_DBUS_PROP_CURRENT_BSS);
}


void wpas_notify_auth_changed(struct wpa_supplicant *wpa_s)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_prop_changed(wpa_s, WPAS_DBUS_PROP_CURRENT_AUTH_MODE);
}


void wpas_notify_network_enabled_changed(struct wpa_supplicant *wpa_s,
					 struct wpa_ssid *ssid)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_network_enabled_changed(wpa_s, ssid);
}


void wpas_notify_network_selected(struct wpa_supplicant *wpa_s,
				  struct wpa_ssid *ssid)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_network_selected(wpa_s, ssid->id);
}


void wpas_notify_network_request(struct wpa_supplicant *wpa_s,
				 struct wpa_ssid *ssid,
				 enum wpa_ctrl_req_type rtype,
				 const char *default_txt)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_network_request(wpa_s, ssid, rtype, default_txt);
}


void wpas_notify_scanning(struct wpa_supplicant *wpa_s)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	/* notify the old DBus API */
	//wpa_supplicant_dbus_notify_scanning(wpa_s);

	/* notify the new DBus API */
	//wpas_dbus_signal_prop_changed(wpa_s, WPAS_DBUS_PROP_SCANNING);
}


void wpas_notify_scan_done(struct wpa_supplicant *wpa_s, int success)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_scan_done(wpa_s, success);

#ifdef INCLUDE_STANDALONE
	system_event_t evt;	
	os_memset(&evt, 0, sizeof(evt));
	evt.event_id = SYSTEM_EVENT_SCAN_DONE;
    if (wpa_s->ifname && strlen(wpa_s->ifname) == 5 /*wlanX*/) {
        evt.vif = wpa_s->ifname[4] - '0';
    }
	sys_event_send(&evt);	
#endif

}


void wpas_notify_scan_results(struct wpa_supplicant *wpa_s)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	/* notify the old DBus API */
	//wpa_supplicant_dbus_notify_scan_results(wpa_s);

	//wpas_wps_notify_scan_results(wpa_s);
}


void wpas_notify_wps_credential(struct wpa_supplicant *wpa_s,
				const struct wps_credential *cred)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

#ifdef CONFIG_WPS
	/* notify the old DBus API */
	wpa_supplicant_dbus_notify_wps_cred(wpa_s, cred);
	/* notify the new DBus API */
	wpas_dbus_signal_wps_cred(wpa_s, cred);
#endif /* CONFIG_WPS */
}


void wpas_notify_wps_event_m2d(struct wpa_supplicant *wpa_s,
			       struct wps_event_m2d *m2d)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

#ifdef CONFIG_WPS
	wpas_dbus_signal_wps_event_m2d(wpa_s, m2d);
#endif /* CONFIG_WPS */
}


void wpas_notify_wps_event_fail(struct wpa_supplicant *wpa_s,
				struct wps_event_fail *fail)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

#ifdef CONFIG_WPS
	wpas_dbus_signal_wps_event_fail(wpa_s, fail);
#endif /* CONFIG_WPS */
}


void wpas_notify_wps_event_success(struct wpa_supplicant *wpa_s)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

#ifdef CONFIG_WPS
	wpas_dbus_signal_wps_event_success(wpa_s);
#endif /* CONFIG_WPS */
}

void wpas_notify_wps_event_pbc_overlap(struct wpa_supplicant *wpa_s)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

#ifdef CONFIG_WPS
	wpas_dbus_signal_wps_event_pbc_overlap(wpa_s);
#endif /* CONFIG_WPS */
}


void wpas_notify_network_added(struct wpa_supplicant *wpa_s,
			       struct wpa_ssid *ssid)
{
}


void wpas_notify_persistent_group_added(struct wpa_supplicant *wpa_s,
					struct wpa_ssid *ssid)
{
#ifdef CONFIG_P2P
	wpas_dbus_register_persistent_group(wpa_s, ssid);
#endif /* CONFIG_P2P */
}


void wpas_notify_persistent_group_removed(struct wpa_supplicant *wpa_s,
					  struct wpa_ssid *ssid)
{
#ifdef CONFIG_P2P
	wpas_dbus_unregister_persistent_group(wpa_s, ssid->id);
#endif /* CONFIG_P2P */
}


void wpas_notify_network_removed(struct wpa_supplicant *wpa_s,
				 struct wpa_ssid *ssid)
{
	//if (wpa_s->next_ssid == ssid)
	//	wpa_s->next_ssid = NULL;

	if (network_is_persistent_group(ssid))
		wpas_notify_persistent_group_removed(wpa_s, ssid);

}


void wpas_notify_bss_added(struct wpa_supplicant *wpa_s,
			   u8 bssid[], unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_register_bss(wpa_s, bssid, id);
	wpa_msg_ctrl(wpa_s, MSG_INFO, WPA_EVENT_BSS_ADDED "%u " MACSTR,
		     id, MAC2STR(bssid));
}


void wpas_notify_bss_removed(struct wpa_supplicant *wpa_s,
			     u8 bssid[], unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_unregister_bss(wpa_s, bssid, id);
	wpa_msg_ctrl(wpa_s, MSG_INFO, WPA_EVENT_BSS_REMOVED "%u " MACSTR,
		     id, MAC2STR(bssid));
}


void wpas_notify_bss_freq_changed(struct wpa_supplicant *wpa_s,
				  unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_bss_signal_prop_changed(wpa_s, WPAS_DBUS_BSS_PROP_FREQ, id);
}


void wpas_notify_bss_signal_changed(struct wpa_supplicant *wpa_s,
				    unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_bss_signal_prop_changed(wpa_s, WPAS_DBUS_BSS_PROP_SIGNAL,
					  //id);
}


void wpas_notify_bss_privacy_changed(struct wpa_supplicant *wpa_s,
				     unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_bss_signal_prop_changed(wpa_s, WPAS_DBUS_BSS_PROP_PRIVACY,
					 // id);
}


void wpas_notify_bss_mode_changed(struct wpa_supplicant *wpa_s,
				  unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_bss_signal_prop_changed(wpa_s, WPAS_DBUS_BSS_PROP_MODE, id);
}


void wpas_notify_bss_wpaie_changed(struct wpa_supplicant *wpa_s,
				   unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_bss_signal_prop_changed(wpa_s, WPAS_DBUS_BSS_PROP_WPA, id);
}


void wpas_notify_bss_rsnie_changed(struct wpa_supplicant *wpa_s,
				   unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_bss_signal_prop_changed(wpa_s, WPAS_DBUS_BSS_PROP_RSN, id);
}


void wpas_notify_bss_wps_changed(struct wpa_supplicant *wpa_s,
				 unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

#ifdef CONFIG_WPS
	wpas_dbus_bss_signal_prop_changed(wpa_s, WPAS_DBUS_BSS_PROP_WPS, id);
#endif /* CONFIG_WPS */
}


void wpas_notify_bss_ies_changed(struct wpa_supplicant *wpa_s,
				   unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_bss_signal_prop_changed(wpa_s, WPAS_DBUS_BSS_PROP_IES, id);
}


void wpas_notify_bss_rates_changed(struct wpa_supplicant *wpa_s,
				   unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_bss_signal_prop_changed(wpa_s, WPAS_DBUS_BSS_PROP_RATES, id);
}


void wpas_notify_bss_seen(struct wpa_supplicant *wpa_s, unsigned int id)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_bss_signal_prop_changed(wpa_s, WPAS_DBUS_BSS_PROP_AGE, id);
}


void wpas_notify_blob_added(struct wpa_supplicant *wpa_s, const char *name)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_blob_added(wpa_s, name);
}


void wpas_notify_blob_removed(struct wpa_supplicant *wpa_s, const char *name)
{
	//if (wpa_s->p2p_mgmt)
	//	return;

	//wpas_dbus_signal_blob_removed(wpa_s, name);
}


void wpas_notify_debug_level_changed(struct wpa_global *global)
{
	//wpas_dbus_signal_debug_level_changed(global);
}


void wpas_notify_debug_timestamp_changed(struct wpa_global *global)
{
	//wpas_dbus_signal_debug_timestamp_changed(global);
}


void wpas_notify_debug_show_keys_changed(struct wpa_global *global)
{
	//wpas_dbus_signal_debug_show_keys_changed(global);
}


#ifdef CONFIG_P2P

void wpas_notify_p2p_find_stopped(struct wpa_supplicant *wpa_s)
{
	/* Notify P2P find has stopped */
	wpas_dbus_signal_p2p_find_stopped(wpa_s);
}


void wpas_notify_p2p_device_found(struct wpa_supplicant *wpa_s,
				  const u8 *dev_addr, int new_device)
{
	if (new_device) {
		/* Create the new peer object */
		wpas_dbus_register_peer(wpa_s, dev_addr);
	}

	/* Notify a new peer has been detected*/
	wpas_dbus_signal_peer_device_found(wpa_s, dev_addr);
}


void wpas_notify_p2p_device_lost(struct wpa_supplicant *wpa_s,
				 const u8 *dev_addr)
{
	wpas_dbus_unregister_peer(wpa_s, dev_addr);

	/* Create signal on interface object*/
	wpas_dbus_signal_peer_device_lost(wpa_s, dev_addr);
}


void wpas_notify_p2p_group_removed(struct wpa_supplicant *wpa_s,
				   const struct wpa_ssid *ssid,
				   const char *role)
{
	wpas_dbus_signal_p2p_group_removed(wpa_s, role);

	wpas_dbus_unregister_p2p_group(wpa_s, ssid);
}


void wpas_notify_p2p_go_neg_req(struct wpa_supplicant *wpa_s,
				const u8 *src, u16 dev_passwd_id, u8 go_intent)
{
	wpas_dbus_signal_p2p_go_neg_req(wpa_s, src, dev_passwd_id, go_intent);
}


void wpas_notify_p2p_go_neg_completed(struct wpa_supplicant *wpa_s,
				      struct p2p_go_neg_results *res)
{
	wpas_dbus_signal_p2p_go_neg_resp(wpa_s, res);
}


void wpas_notify_p2p_invitation_result(struct wpa_supplicant *wpa_s,
				       int status, const u8 *bssid)
{
	wpas_dbus_signal_p2p_invitation_result(wpa_s, status, bssid);
}


void wpas_notify_p2p_sd_request(struct wpa_supplicant *wpa_s,
				int freq, const u8 *sa, u8 dialog_token,
				u16 update_indic, const u8 *tlvs,
				size_t tlvs_len)
{
	wpas_dbus_signal_p2p_sd_request(wpa_s, freq, sa, dialog_token,
					update_indic, tlvs, tlvs_len);
}


void wpas_notify_p2p_sd_response(struct wpa_supplicant *wpa_s,
				 const u8 *sa, u16 update_indic,
				 const u8 *tlvs, size_t tlvs_len)
{
	wpas_dbus_signal_p2p_sd_response(wpa_s, sa, update_indic,
					 tlvs, tlvs_len);
}


/**
 * wpas_notify_p2p_provision_discovery - Notification of provision discovery
 * @dev_addr: Who sent the request or responded to our request.
 * @request: Will be 1 if request, 0 for response.
 * @status: Valid only in case of response (0 in case of success)
 * @config_methods: WPS config methods
 * @generated_pin: PIN to be displayed in case of WPS_CONFIG_DISPLAY method
 *
 * This can be used to notify:
 * - Requests or responses
 * - Various config methods
 * - Failure condition in case of response
 */
void wpas_notify_p2p_provision_discovery(struct wpa_supplicant *wpa_s,
					 const u8 *dev_addr, int request,
					 enum p2p_prov_disc_status status,
					 u16 config_methods,
					 unsigned int generated_pin)
{
	wpas_dbus_signal_p2p_provision_discovery(wpa_s, dev_addr, request,
						 status, config_methods,
						 generated_pin);
}


void wpas_notify_p2p_group_started(struct wpa_supplicant *wpa_s,
				   struct wpa_ssid *ssid, int persistent,
				   int client)
{
	/* Notify a group has been started */
	wpas_dbus_register_p2p_group(wpa_s, ssid);

	wpas_dbus_signal_p2p_group_started(wpa_s, client, persistent);
}


void wpas_notify_p2p_group_formation_failure(struct wpa_supplicant *wpa_s,
					     const char *reason)
{
	/* Notify a group formation failed */
	wpas_dbus_signal_p2p_group_formation_failure(wpa_s, reason);
}


void wpas_notify_p2p_wps_failed(struct wpa_supplicant *wpa_s,
				struct wps_event_fail *fail)
{
	wpas_dbus_signal_p2p_wps_failed(wpa_s, fail);
}


void wpas_notify_p2p_invitation_received(struct wpa_supplicant *wpa_s,
					 const u8 *sa, const u8 *go_dev_addr,
					 const u8 *bssid, int id, int op_freq)
{
	/* Notify a P2P Invitation Request */
	wpas_dbus_signal_p2p_invitation_received(wpa_s, sa, go_dev_addr, bssid,
						 id, op_freq);
}

#endif /* CONFIG_P2P */


static void wpas_notify_ap_sta_authorized(struct wpa_supplicant *wpa_s,
					  const u8 *sta,
					  const u8 *p2p_dev_addr)
{
#ifdef CONFIG_P2P
	wpas_p2p_notify_ap_sta_authorized(wpa_s, p2p_dev_addr);

	/*
	 * Create 'peer-joined' signal on group object -- will also
	 * check P2P itself.
	 */
	if (p2p_dev_addr)
		wpas_dbus_signal_p2p_peer_joined(wpa_s, p2p_dev_addr);
#endif /* CONFIG_P2P */

	/* Notify listeners a new station has been authorized */
	//wpas_dbus_signal_sta_authorized(wpa_s, sta);
}


static void wpas_notify_ap_sta_deauthorized(struct wpa_supplicant *wpa_s,
					    const u8 *sta,
					    const u8 *p2p_dev_addr)
{
#ifdef CONFIG_P2P
	/*
	 * Create 'peer-disconnected' signal on group object if this
	 * is a P2P group.
	 */
	if (p2p_dev_addr)
		wpas_dbus_signal_p2p_peer_disconnected(wpa_s, p2p_dev_addr);
#endif /* CONFIG_P2P */

	/* Notify listeners a station has been deauthorized */
	//wpas_dbus_signal_sta_deauthorized(wpa_s, sta);
}


void wpas_notify_sta_authorized(struct wpa_supplicant *wpa_s,
				const u8 *mac_addr, int authorized,
				const u8 *p2p_dev_addr)
{
	if (authorized)
		wpas_notify_ap_sta_authorized(wpa_s, mac_addr, p2p_dev_addr);
	else
		wpas_notify_ap_sta_deauthorized(wpa_s, mac_addr, p2p_dev_addr);
}


void wpas_notify_certification(struct wpa_supplicant *wpa_s, int depth,
			       const char *subject, const char *altsubject[],
			       int num_altsubject, const char *cert_hash,
			       const struct wpabuf *cert)
{
	wpa_msg(wpa_s, MSG_INFO, WPA_EVENT_EAP_PEER_CERT
		"depth=%d subject='%s'%s%s",
		depth, subject, cert_hash ? " hash=" : "",
		cert_hash ? cert_hash : "");
#if 0
	if (cert) {
		char *cert_hex;
		size_t len = wpabuf_len(cert) * 2 + 1;
		cert_hex = os_malloc(len);
		if (cert_hex) {
			wpa_snprintf_hex(cert_hex, len, wpabuf_head(cert),
					 wpabuf_len(cert));
			wpa_msg_ctrl(wpa_s, MSG_INFO,
				     WPA_EVENT_EAP_PEER_CERT
				     "depth=%d subject='%s' cert=%s",
				     depth, subject, cert_hex);
			os_free(cert_hex);
		}
	}

	if (altsubject) {
		int i;

		for (i = 0; i < num_altsubject; i++)
			wpa_msg(wpa_s, MSG_INFO, WPA_EVENT_EAP_PEER_ALT
				"depth=%d %s", depth, altsubject[i]);
	}

	/* notify the old DBus API */
	wpa_supplicant_dbus_notify_certification(wpa_s, depth, subject,
						 cert_hash, cert);
	/* notify the new DBus API */
	wpas_dbus_signal_certification(wpa_s, depth, subject, altsubject,
				       num_altsubject, cert_hash, cert);
#endif    
}


void wpas_notify_preq(struct wpa_supplicant *wpa_s,
		      const u8 *addr, const u8 *dst, const u8 *bssid,
		      const u8 *ie, size_t ie_len, u32 ssi_signal)
{
#ifdef CONFIG_AP
	//wpas_dbus_signal_preq(wpa_s, addr, dst, bssid, ie, ie_len, ssi_signal);
#endif /* CONFIG_AP */
}


void wpas_notify_eap_status(struct wpa_supplicant *wpa_s, const char *status,
			    const char *parameter)
{
	//wpas_dbus_signal_eap_status(wpa_s, status, parameter);
	wpa_msg_ctrl(wpa_s, MSG_INFO, WPA_EVENT_EAP_STATUS
		     "status='%s' parameter='%s'",
		     status, parameter);
}


void wpas_notify_network_bssid_set_changed(struct wpa_supplicant *wpa_s,
					   struct wpa_ssid *ssid)
{
	if (wpa_s->current_ssid != ssid)
		return;

	wpa_dbg(wpa_s, MSG_DEBUG,
		"Network bssid config changed for the current network - within-ESS roaming %s",
		ssid->bssid_set ? "disabled" : "enabled");

	wpa_drv_roaming(wpa_s, !ssid->bssid_set,
			ssid->bssid_set ? ssid->bssid : NULL);
}


void wpas_notify_network_type_changed(struct wpa_supplicant *wpa_s,
				      struct wpa_ssid *ssid)
{
#ifdef CONFIG_P2P
	if (ssid->disabled == 2) {
		/* Changed from normal network profile to persistent group */
		ssid->disabled = 0;
		wpas_dbus_unregister_network(wpa_s, ssid->id);
		ssid->disabled = 2;
		ssid->p2p_persistent_group = 1;
		wpas_dbus_register_persistent_group(wpa_s, ssid);
	} else {
		/* Changed from persistent group to normal network profile */
		wpas_dbus_unregister_persistent_group(wpa_s, ssid->id);
		ssid->p2p_persistent_group = 0;
		wpas_dbus_register_network(wpa_s, ssid);
	}
#endif /* CONFIG_P2P */
}

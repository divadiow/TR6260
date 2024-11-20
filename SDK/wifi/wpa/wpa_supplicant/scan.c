/*
 * WPA Supplicant - Scanning
 * Copyright (c) 2003-2014, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"

#include "utils/common.h"
#include "utils/eloop.h"
#include "common/ieee802_11_defs.h"
#include "common/wpa_ctrl.h"
#include "config.h"
#include "wpa_supplicant_i.h"
#include "driver_i.h"
#include "wps_supplicant.h"
#include "notify.h"
#include "bss.h"
#include "scan.h"


static void wpa_supplicant_gen_assoc_event(struct wpa_supplicant *wpa_s)
{
	struct wpa_ssid *ssid;
	union wpa_event_data data;

	ssid = wpa_supplicant_get_ssid(wpa_s);
	if (ssid == NULL)
		return;

	/*if (wpa_s->current_ssid == NULL) {
		wpa_s->current_ssid = ssid;
		wpas_notify_network_changed(wpa_s);
	}*/
	//wpa_supplicant_initiate_eapol(wpa_s);
	wpa_dbg(wpa_s, MSG_DEBUG, "Already associated with a configured "
		"network - generating associated event");
	os_memset(&data, 0, sizeof(data));
	wpa_supplicant_event(wpa_s, EVENT_ASSOC, &data);
}


#ifdef CONFIG_WPS
static int wpas_wps_in_use(struct wpa_supplicant *wpa_s,
			   enum wps_request_type *req_type)
{
	struct wpa_ssid *ssid;
	int wps = 0;

	for (ssid = wpa_s->conf->ssid; ssid; ssid = ssid->next) {
		if (!(ssid->key_mgmt & WPA_KEY_MGMT_WPS))
			continue;

		wps = 1;
		*req_type = wpas_wps_get_req_type(ssid);
		if (ssid->eap.phase1 && os_strstr(ssid->eap.phase1, "pbc=1"))
			return 2;
	}

#ifdef CONFIG_P2P
	if (!wpa_s->global->p2p_disabled && wpa_s->global->p2p &&
	    !wpa_s->conf->p2p_disabled) {
		wpa_s->wps->dev.p2p = 1;
		if (!wps) {
			wps = 1;
			*req_type = WPS_REQ_ENROLLEE_INFO;
		}
	}
#endif /* CONFIG_P2P */

	return wps;
}
#endif /* CONFIG_WPS */


/**
 * wpa_supplicant_enabled_networks - Check whether there are enabled networks
 * @wpa_s: Pointer to wpa_supplicant data
 * Returns: 0 if no networks are enabled, >0 if networks are enabled
 *
 * This function is used to figure out whether any networks (or Interworking
 * with enabled credentials and auto_interworking) are present in the current
 * configuration.
 */
int wpa_supplicant_enabled_networks(struct wpa_supplicant *wpa_s)
{
	struct wpa_ssid *ssid = wpa_s->conf->ssid;
#if 0    
	int count = 0, disabled = 0;

	//if (wpa_s->p2p_mgmt)
	//	return 0; /* no normal network profiles on p2p_mgmt interface */

	while (ssid) {
		if (!wpas_network_disabled(wpa_s, ssid))
			count++;
		else
			disabled++;
		ssid = ssid->next;
	}
	//if (wpa_s->conf->cred && wpa_s->conf->interworking &&
	    //wpa_s->conf->auto_interworking)
		//count++;
	if (count == 0 && disabled > 0) {
		wpa_dbg(wpa_s, MSG_DEBUG, "No enabled networks (%d disabled "
			"networks)", disabled);
	}
	return count;
#else
    if (!wpas_network_disabled(wpa_s, ssid))
        return 1;
    return 0;
#endif
}


static void wpa_supplicant_assoc_try(struct wpa_supplicant *wpa_s,
				     struct wpa_ssid *ssid)
{
	while (ssid) {
		if (!wpas_network_disabled(wpa_s, ssid))
			break;
		ssid = ssid->next;
	}

	/* ap_scan=2 mode - try to associate with each SSID. */
	if (ssid == NULL) {
		wpa_dbg(wpa_s, MSG_DEBUG, "wpa_supplicant_assoc_try: Reached "
			"end of scan list - go back to beginning");
		//wpa_s->prev_scan_ssid = WILDCARD_SSID_SCAN;
		wpa_supplicant_req_scan(wpa_s, 0, 0);
		return;
	}
	if (ssid->next) {
		/* Continue from the next SSID on the next attempt. */
		//wpa_s->prev_scan_ssid = ssid;
	} else {
		/* Start from the beginning of the SSID list. */
		//wpa_s->prev_scan_ssid = WILDCARD_SSID_SCAN;
	}
	wpa_supplicant_associate(wpa_s, NULL, ssid);
}


static void wpas_trigger_scan_cb(struct wpa_radio_work *work, int deinit)
{
	struct wpa_supplicant *wpa_s = work->wpa_s;
	struct wpa_driver_scan_params *params = work->ctx;
	int ret;

	if (deinit) {
		if (!work->started) {
			wpa_scan_free_params(params);
			return;
		}
		wpa_supplicant_notify_scanning(wpa_s, 0);
		wpas_notify_scan_done(wpa_s, 0);
		wpa_s->scan_work = NULL;
		return;
	}

	wpa_supplicant_notify_scanning(wpa_s, 1);

	/*if (wpa_s->clear_driver_scan_cache) {
		wpa_printf(MSG_DEBUG,
			   "Request driver to clear scan cache due to local BSS flush");
	}*/
	ret = wpa_drv_scan(wpa_s, params);
	wpa_scan_free_params(params);
	work->ctx = NULL;
	if (ret) {
		int retry = wpa_s->last_scan_req != MANUAL_SCAN_REQ;

		if (wpa_s->disconnected)
			retry = 0;

		wpa_supplicant_notify_scanning(wpa_s, 0);
		wpas_notify_scan_done(wpa_s, 0);
		if (wpa_s->wpa_state == WPA_SCANNING)
			wpa_supplicant_set_state(wpa_s,
						 wpa_s->scan_prev_wpa_state);
		wpa_msg(wpa_s, MSG_INFO, WPA_EVENT_SCAN_FAILED "ret=%d%s",
			ret, retry ? " retry=1" : "");
		radio_work_done(work);

		if (retry) {
			/* Restore scan_req since we will try to scan again */
			wpa_s->scan_req = wpa_s->last_scan_req;
			wpa_supplicant_req_scan(wpa_s, 1, 0);
		}
		return;
	}

	//os_get_reltime(&wpa_s->scan_trigger_time);
	//wpa_s->scan_runs++;
	wpa_s->normal_scans++;
	wpa_s->own_scan_requested = 1;
	wpa_s->clear_driver_scan_cache = 0;
	wpa_s->scan_work = work;
}


/**
 * wpa_supplicant_trigger_scan - Request driver to start a scan
 * @wpa_s: Pointer to wpa_supplicant data
 * @params: Scan parameters
 * Returns: 0 on success, -1 on failure
 */
int wpa_supplicant_trigger_scan(struct wpa_supplicant *wpa_s,
				struct wpa_driver_scan_params *params)
{
	struct wpa_driver_scan_params *ctx;

	if (wpa_s->scan_work) {
		wpa_dbg(wpa_s, MSG_INFO, "Reject scan trigger since one is already pending");
		return -1;
	}

	ctx = wpa_scan_clone_params(params);
	if (!ctx ||
	    radio_add_work(wpa_s, 0, "scan", 0, wpas_trigger_scan_cb, ctx) < 0)
	{
		wpa_scan_free_params(ctx);
		wpa_msg(wpa_s, MSG_INFO, WPA_EVENT_SCAN_FAILED "ret=-1");
		return -1;
	}

	return 0;
}


static void
wpa_supplicant_delayed_sched_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_supplicant *wpa_s = eloop_ctx;

	wpa_dbg(wpa_s, MSG_DEBUG, "Starting delayed sched scan");

	if (wpa_supplicant_req_sched_scan(wpa_s))
		wpa_supplicant_req_scan(wpa_s, 0, 0);
}


static void
wpa_supplicant_sched_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_supplicant *wpa_s = eloop_ctx;

	wpa_dbg(wpa_s, MSG_DEBUG, "Sched scan timeout - stopping it");

	//wpa_s->sched_scan_timed_out = 1;
	//wpa_supplicant_cancel_sched_scan(wpa_s);
}


static int
wpa_supplicant_start_sched_scan(struct wpa_supplicant *wpa_s,
				struct wpa_driver_scan_params *params)
{
#if 0
	int ret;

	wpa_supplicant_notify_scanning(wpa_s, 1);
	ret = wpa_drv_sched_scan(wpa_s, params);
	if (ret)
		wpa_supplicant_notify_scanning(wpa_s, 0);
	else
		wpa_s->sched_scanning = 1;

	return ret;
#else
    return 0;
#endif
}


static int wpa_supplicant_stop_sched_scan(struct wpa_supplicant *wpa_s)
{
	int ret;

	ret = wpa_drv_stop_sched_scan(wpa_s);
	if (ret) {
		wpa_dbg(wpa_s, MSG_DEBUG, "stopping sched_scan failed!");
		/* TODO: what to do if stopping fails? */
		return -1;
	}

	return ret;
}


static void wpa_supplicant_optimize_freqs(
	struct wpa_supplicant *wpa_s, struct wpa_driver_scan_params *params)
{
#ifdef CONFIG_P2P
	if (params->freqs == NULL && wpa_s->p2p_in_provisioning &&
	    wpa_s->go_params) {
		/* Optimize provisioning state scan based on GO information */
		if (wpa_s->p2p_in_provisioning < 5 &&
		    wpa_s->go_params->freq > 0) {
			wpa_dbg(wpa_s, MSG_DEBUG, "P2P: Scan only GO "
				"preferred frequency %d MHz",
				wpa_s->go_params->freq);
			params->freqs = os_calloc(2, sizeof(int));
			if (params->freqs)
				params->freqs[0] = wpa_s->go_params->freq;
		} else if (wpa_s->p2p_in_provisioning < 8 &&
			   wpa_s->go_params->freq_list[0]) {
			wpa_dbg(wpa_s, MSG_DEBUG, "P2P: Scan only common "
				"channels");
			int_array_concat(&params->freqs,
					 wpa_s->go_params->freq_list);
			if (params->freqs)
				int_array_sort_unique(params->freqs);
		}
		wpa_s->p2p_in_provisioning++;
	}

	if (params->freqs == NULL && wpa_s->p2p_in_invitation) {
		/*
		 * Optimize scan based on GO information during persistent
		 * group reinvocation
		 */
		if (wpa_s->p2p_in_invitation < 5 &&
		    wpa_s->p2p_invite_go_freq > 0) {
			wpa_dbg(wpa_s, MSG_DEBUG, "P2P: Scan only GO preferred frequency %d MHz during invitation",
				wpa_s->p2p_invite_go_freq);
			params->freqs = os_calloc(2, sizeof(int));
			if (params->freqs)
				params->freqs[0] = wpa_s->p2p_invite_go_freq;
		}
		wpa_s->p2p_in_invitation++;
		if (wpa_s->p2p_in_invitation > 20) {
			/*
			 * This should not really happen since the variable is
			 * cleared on group removal, but if it does happen, make
			 * sure we do not get stuck in special invitation scan
			 * mode.
			 */
			wpa_dbg(wpa_s, MSG_DEBUG, "P2P: Clear p2p_in_invitation");
			wpa_s->p2p_in_invitation = 0;
		}
	}
#endif /* CONFIG_P2P */

#ifdef CONFIG_WPS
	if (params->freqs == NULL && wpa_s->after_wps && wpa_s->wps_freq) {
		/*
		 * Optimize post-provisioning scan based on channel used
		 * during provisioning.
		 */
		wpa_dbg(wpa_s, MSG_DEBUG, "WPS: Scan only frequency %u MHz "
			"that was used during provisioning", wpa_s->wps_freq);
		params->freqs = os_calloc(2, sizeof(int));
		if (params->freqs)
			params->freqs[0] = wpa_s->wps_freq;
		wpa_s->after_wps--;
	} else if (wpa_s->after_wps)
		wpa_s->after_wps--;

	if (params->freqs == NULL && wpa_s->known_wps_freq && wpa_s->wps_freq)
	{
		/* Optimize provisioning scan based on already known channel */
		wpa_dbg(wpa_s, MSG_DEBUG, "WPS: Scan only frequency %u MHz",
			wpa_s->wps_freq);
		params->freqs = os_calloc(2, sizeof(int));
		if (params->freqs)
			params->freqs[0] = wpa_s->wps_freq;
		wpa_s->known_wps_freq = 0; /* only do this once */
	}
#endif /* CONFIG_WPS */
}


#ifdef CONFIG_INTERWORKING
static void wpas_add_interworking_elements(struct wpa_supplicant *wpa_s,
					   struct wpabuf *buf)
{
	wpabuf_put_u8(buf, WLAN_EID_INTERWORKING);
	wpabuf_put_u8(buf, is_zero_ether_addr(wpa_s->conf->hessid) ? 1 :
		      1 + ETH_ALEN);
	wpabuf_put_u8(buf, wpa_s->conf->access_network_type);
	/* No Venue Info */
	if (!is_zero_ether_addr(wpa_s->conf->hessid))
		wpabuf_put_data(buf, wpa_s->conf->hessid, ETH_ALEN);
}
#endif /* CONFIG_INTERWORKING */


void wpa_supplicant_set_default_scan_ies(struct wpa_supplicant *wpa_s)
{
	struct wpabuf *default_ies = NULL;
	u8 ext_capab[18];
	int ext_capab_len;
	enum wpa_driver_if_type type = WPA_IF_STATION;

#ifdef CONFIG_P2P
	if (wpa_s->p2p_group_interface == P2P_GROUP_INTERFACE_CLIENT)
		type = WPA_IF_P2P_CLIENT;
#endif /* CONFIG_P2P */

	//wpa_drv_get_ext_capa(wpa_s, type);

	ext_capab_len = wpas_build_ext_capab(wpa_s, ext_capab,
					     sizeof(ext_capab));
	if (ext_capab_len > 0 &&
	    wpabuf_resize(&default_ies, ext_capab_len) == 0)
		wpabuf_put_data(default_ies, ext_capab, ext_capab_len);

#ifdef CONFIG_MBO
	/* Send cellular capabilities for potential MBO STAs */
	if (wpabuf_resize(&default_ies, 9) == 0)
		wpas_mbo_scan_ie(wpa_s, default_ies);
#endif /* CONFIG_MBO */

	if (default_ies)
		wpa_drv_set_default_scan_ies(wpa_s, wpabuf_head(default_ies),
					     wpabuf_len(default_ies));
	wpabuf_free(default_ies);
}


static struct wpabuf * wpa_supplicant_extra_ies(struct wpa_supplicant *wpa_s)
{
	struct wpabuf *extra_ie = NULL;
	u8 ext_capab[18];
	int ext_capab_len;
#ifdef CONFIG_WPS
	int wps = 0;
	enum wps_request_type req_type = WPS_REQ_ENROLLEE_INFO;
#endif /* CONFIG_WPS */

#ifdef CONFIG_P2P
	if (wpa_s->p2p_group_interface == P2P_GROUP_INTERFACE_CLIENT)
		wpa_drv_get_ext_capa(wpa_s, WPA_IF_P2P_CLIENT);
	else
#endif /* CONFIG_P2P */
		//wpa_drv_get_ext_capa(wpa_s, WPA_IF_STATION);

	ext_capab_len = wpas_build_ext_capab(wpa_s, ext_capab,
					     sizeof(ext_capab));
	if (ext_capab_len > 0 &&
	    wpabuf_resize(&extra_ie, ext_capab_len) == 0)
		wpabuf_put_data(extra_ie, ext_capab, ext_capab_len);

#ifdef CONFIG_INTERWORKING
	if (wpa_s->conf->interworking &&
	    wpabuf_resize(&extra_ie, 100) == 0)
		wpas_add_interworking_elements(wpa_s, extra_ie);
#endif /* CONFIG_INTERWORKING */

#ifdef CONFIG_WPS
	wps = wpas_wps_in_use(wpa_s, &req_type);

	if (wps) {
		struct wpabuf *wps_ie;
		wps_ie = wps_build_probe_req_ie(wps == 2 ? DEV_PW_PUSHBUTTON :
						DEV_PW_DEFAULT,
						&wpa_s->wps->dev,
						wpa_s->wps->uuid, req_type,
						0, NULL);
		if (wps_ie) {
			if (wpabuf_resize(&extra_ie, wpabuf_len(wps_ie)) == 0)
				wpabuf_put_buf(extra_ie, wps_ie);
			wpabuf_free(wps_ie);
		}
	}

#ifdef CONFIG_P2P
	if (wps) {
		size_t ielen = p2p_scan_ie_buf_len(wpa_s->global->p2p);
		if (wpabuf_resize(&extra_ie, ielen) == 0)
			wpas_p2p_scan_ie(wpa_s, extra_ie);
	}
#endif /* CONFIG_P2P */

	wpa_supplicant_mesh_add_scan_ie(wpa_s, &extra_ie);

#endif /* CONFIG_WPS */

#ifdef CONFIG_HS20
	if (wpa_s->conf->hs20 && wpabuf_resize(&extra_ie, 7) == 0)
		wpas_hs20_add_indication(extra_ie, -1);
#endif /* CONFIG_HS20 */

#ifdef CONFIG_FST
	if (wpa_s->fst_ies &&
	    wpabuf_resize(&extra_ie, wpabuf_len(wpa_s->fst_ies)) == 0)
		wpabuf_put_buf(extra_ie, wpa_s->fst_ies);
#endif /* CONFIG_FST */

#ifdef CONFIG_MBO
	/* Send cellular capabilities for potential MBO STAs */
	if (wpabuf_resize(&extra_ie, 9) == 0)
		wpas_mbo_scan_ie(wpa_s, extra_ie);
#endif /* CONFIG_MBO */

#if 0
	if (wpa_s->vendor_elem[VENDOR_ELEM_PROBE_REQ]) {
		struct wpabuf *buf = wpa_s->vendor_elem[VENDOR_ELEM_PROBE_REQ];

		if (wpabuf_resize(&extra_ie, wpabuf_len(buf)) == 0)
			wpabuf_put_buf(extra_ie, buf);
	}
#endif
	return extra_ie;
}


#ifdef CONFIG_P2P

/*
 * Check whether there are any enabled networks or credentials that could be
 * used for a non-P2P connection.
 */
static int non_p2p_network_enabled(struct wpa_supplicant *wpa_s)
{
	struct wpa_ssid *ssid;

	for (ssid = wpa_s->conf->ssid; ssid; ssid = ssid->next) {
		if (wpas_network_disabled(wpa_s, ssid))
			continue;
		if (!ssid->p2p_group)
			return 1;
	}

	if (wpa_s->conf->cred && wpa_s->conf->interworking &&
	    wpa_s->conf->auto_interworking)
		return 1;

	return 0;
}

#endif /* CONFIG_P2P */


static void wpa_setband_scan_freqs_list(struct wpa_supplicant *wpa_s,
					enum hostapd_hw_mode band,
					struct wpa_driver_scan_params *params)
{
	/* Include only supported channels for the specified band */
	struct hostapd_hw_modes *mode;
	int count, i;

	mode = get_mode(wpa_s->hw.modes, wpa_s->hw.num_modes, band);
	if (mode == NULL) {
		/* No channels supported in this band - use empty list */
		params->freqs = os_zalloc(sizeof(int));
		return;
	}

	params->freqs = os_calloc(mode->num_channels + 1, sizeof(int));
	if (params->freqs == NULL)
		return;
	for (count = 0, i = 0; i < mode->num_channels; i++) {
		if (mode->channels[i].flag & HOSTAPD_CHAN_DISABLED)
			continue;
		params->freqs[count++] = mode->channels[i].freq;
	}
}

#if 0
static void wpa_setband_scan_freqs(struct wpa_supplicant *wpa_s,
				   struct wpa_driver_scan_params *params)
{
	if (wpa_s->hw.modes == NULL)
		return; /* unknown what channels the driver supports */
	if (params->freqs)
		return; /* already using a limited channel set */
	if (wpa_s->setband == WPA_SETBAND_5G)
		wpa_setband_scan_freqs_list(wpa_s, HOSTAPD_MODE_IEEE80211A,
					    params);
	else if (wpa_s->setband == WPA_SETBAND_2G)
		wpa_setband_scan_freqs_list(wpa_s, HOSTAPD_MODE_IEEE80211G,
					    params);
}
#endif

static void wpa_set_scan_ssids(struct wpa_supplicant *wpa_s,
			       struct wpa_driver_scan_params *params,
			       size_t max_ssids)
{
#if 0
	unsigned int i;
	struct wpa_ssid *ssid;

	/*
	 * For devices with max_ssids greater than 1, leave the last slot empty
	 * for adding the wildcard scan entry.
	 */
	max_ssids = max_ssids > 1 ? max_ssids - 1 : max_ssids;

	for (i = 0; i < wpa_s->scan_id_count; i++) {
		unsigned int j;

		ssid = wpa_config_get_network(wpa_s->conf, wpa_s->scan_id[i]);
		if (!ssid || !ssid->scan_ssid)
			continue;

		for (j = 0; j < params->num_ssids; j++) {
			if (params->ssids[j].ssid_len == ssid->ssid_len &&
			    params->ssids[j].ssid &&
			    os_memcmp(params->ssids[j].ssid, ssid->ssid,
				      ssid->ssid_len) == 0)
				break;
		}
		if (j < params->num_ssids)
			continue; /* already in the list */

		if (params->num_ssids + 1 > max_ssids) {
			wpa_printf(MSG_DEBUG,
				   "Over max scan SSIDs for manual request");
			break;
		}

		wpa_printf(MSG_DEBUG, "Scan SSID (manual request): %s",
			   wpa_ssid_txt(ssid->ssid, ssid->ssid_len));
		params->ssids[params->num_ssids].ssid = ssid->ssid;
		params->ssids[params->num_ssids].ssid_len = ssid->ssid_len;
		params->num_ssids++;
	}

	wpa_s->scan_id_count = 0;
#endif    
}

#if 0
static int wpa_set_ssids_from_scan_req(struct wpa_supplicant *wpa_s,
				       struct wpa_driver_scan_params *params,
				       size_t max_ssids)
{
	unsigned int i;

	if (wpa_s->ssids_from_scan_req == NULL ||
	    wpa_s->num_ssids_from_scan_req == 0)
		return 0;

	if (wpa_s->num_ssids_from_scan_req > max_ssids) {
		wpa_s->num_ssids_from_scan_req = max_ssids;
		wpa_printf(MSG_DEBUG, "Over max scan SSIDs from scan req: %u",
			   (unsigned int) max_ssids);
	}

	for (i = 0; i < wpa_s->num_ssids_from_scan_req; i++) {
		params->ssids[i].ssid = wpa_s->ssids_from_scan_req[i].ssid;
		params->ssids[i].ssid_len =
			wpa_s->ssids_from_scan_req[i].ssid_len;
		wpa_hexdump_ascii(MSG_DEBUG, "specific SSID",
				  params->ssids[i].ssid,
				  params->ssids[i].ssid_len);
	}

	params->num_ssids = wpa_s->num_ssids_from_scan_req;
	wpa_s->num_ssids_from_scan_req = 0;
	return 1;
}
#endif

static void wpa_supplicant_scan(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_supplicant *wpa_s = eloop_ctx;
	struct wpa_ssid *ssid;
	int ret, p2p_in_prog;
	//struct wpabuf *extra_ie = NULL;
	struct wpa_driver_scan_params params;
	struct wpa_driver_scan_params *scan_params;
	size_t max_ssids;

	if (wpa_s->disconnected && wpa_s->scan_req == NORMAL_SCAN_REQ) {
		wpa_dbg(wpa_s, MSG_DEBUG, "Disconnected - do not scan");
		wpa_supplicant_set_state(wpa_s, WPA_DISCONNECTED);
		return;
	}

	if (wpa_s->scanning) {
		/*
		 * If we are already in scanning state, we shall reschedule the
		 * the incoming scan request.
		 */
		wpa_dbg(wpa_s, MSG_DEBUG, "Already scanning - Reschedule the incoming scan req");
		wpa_supplicant_req_scan(wpa_s, 1, 0);
		return;
	}

	if (!wpa_supplicant_enabled_networks(wpa_s) &&
	    wpa_s->scan_req == NORMAL_SCAN_REQ) {
		wpa_dbg(wpa_s, MSG_DEBUG, "No enabled networks - do not scan");
		wpa_supplicant_set_state(wpa_s, WPA_INACTIVE);
		return;
	}

	ssid = NULL;
#if 0    
	if (wpa_s->scan_req != MANUAL_SCAN_REQ &&
	    wpa_s->connect_without_scan) {
		connect_without_scan = 1;
		for (ssid = wpa_s->conf->ssid; ssid; ssid = ssid->next) {
			if (ssid == wpa_s->connect_without_scan)
				break;
		}
	}
#endif

	max_ssids = wpa_s->max_scan_ssids;
	if (max_ssids > WPAS_MAX_SCAN_SSIDS)
		max_ssids = WPAS_MAX_SCAN_SSIDS;

	wpa_s->last_scan_req = wpa_s->scan_req;
	wpa_s->scan_req = NORMAL_SCAN_REQ;

#if 0
	if (connect_without_scan) {
		wpa_s->connect_without_scan = NULL;
		if (ssid) {
			wpa_printf(MSG_DEBUG, "Start a pre-selected network "
				   "without scan step");
			wpa_supplicant_associate(wpa_s, NULL, ssid);
			return;
		}
	}
#endif    

	os_memset(&params, 0, sizeof(params));

	wpa_s->scan_prev_wpa_state = wpa_s->wpa_state;
	if (wpa_s->wpa_state == WPA_DISCONNECTED ||
	    wpa_s->wpa_state == WPA_INACTIVE)
		wpa_supplicant_set_state(wpa_s, WPA_SCANNING);

#if 0
	if (wpa_s->last_scan_req == MANUAL_SCAN_REQ &&
	    wpa_set_ssids_from_scan_req(wpa_s, &params, max_ssids)) {
		wpa_printf(MSG_DEBUG, "Use specific SSIDs from SCAN command");
		goto ssid_list_set;
	}
#endif

	/* Find the starting point from which to continue scanning */
	ssid = wpa_s->conf->ssid;
#if 0 //don't need this feature.   
	if (wpa_s->prev_scan_ssid != WILDCARD_SSID_SCAN) {
		while (ssid) {
			if (ssid == wpa_s->prev_scan_ssid) {
				ssid = ssid->next;
				break;
			}
			ssid = ssid->next;
		}
	}
#endif    

#if 0
	struct wpa_ssid *start = ssid, *tssid;
	//int freqs_set = 0;
	//if (ssid == NULL && max_ssids > 1)
	//	ssid = wpa_s->conf->ssid;
	while (ssid) {
		if (!wpas_network_disabled(wpa_s, ssid) &&
		    ssid->scan_ssid) {
			wpa_hexdump_ascii(MSG_DEBUG, "Scan SSID",
					  ssid->ssid, ssid->ssid_len);
			params.ssids[params.num_ssids].ssid =
				ssid->ssid;
			params.ssids[params.num_ssids].ssid_len =
				ssid->ssid_len;
			params.num_ssids++;
			if (params.num_ssids + 1 >= max_ssids)
				break;
		}
		ssid = ssid->next;
		if (ssid == start)
			break;
		//if (ssid == NULL && max_ssids > 1 &&
		//    start != wpa_s->conf->ssid)
		//	ssid = wpa_s->conf->ssid;
	}

#if 0
	if (wpa_s->scan_id_count &&
	    wpa_s->last_scan_req == MANUAL_SCAN_REQ)
		wpa_set_scan_ssids(wpa_s, &params, max_ssids);
    
	for (tssid = wpa_s->conf->ssid;
	     wpa_s->last_scan_req != MANUAL_SCAN_REQ && tssid;
	     tssid = tssid->next) {
		if (wpas_network_disabled(wpa_s, tssid))
			continue;
		if ((params.freqs || !freqs_set) && tssid->scan_freq) {
			int_array_concat(&params.freqs,
					 tssid->scan_freq);
		} else {
			os_free(params.freqs);
			params.freqs = NULL;
		}
		freqs_set = 1;
	}
	int_array_sort_unique(params.freqs);
#endif

	if (ssid && max_ssids == 1) {
		/*
		 * If the driver is limited to 1 SSID at a time interleave
		 * wildcard SSID scans with specific SSID scans to avoid
		 * waiting a long time for a wildcard scan.
		 */
		if (!wpa_s->prev_scan_wildcard) {
			params.ssids[0].ssid = NULL;
			params.ssids[0].ssid_len = 0;
			wpa_s->prev_scan_wildcard = 1;
			wpa_dbg(wpa_s, MSG_DEBUG, "Starting AP scan for "
				"wildcard SSID (Interleave with specific)");
		} else {
			//wpa_s->prev_scan_ssid = ssid;
			wpa_s->prev_scan_wildcard = 0;
			wpa_dbg(wpa_s, MSG_DEBUG,
				"Starting AP scan for specific SSID: %s",
				wpa_ssid_txt(ssid->ssid, ssid->ssid_len));
		}
	} else if (ssid) {
		/* max_ssids > 1 */

		//wpa_s->prev_scan_ssid = ssid;
		wpa_dbg(wpa_s, MSG_DEBUG, "Include wildcard SSID in "
			"the scan request");
		params.num_ssids++;
	}/* else if (wpa_s->last_scan_req == MANUAL_SCAN_REQ &&
		   wpa_s->manual_scan_passive && params.num_ssids == 0) {
		wpa_dbg(wpa_s, MSG_DEBUG, "Use passive scan based on manual request");
	} else if (wpa_s->conf->passive_scan) {
		wpa_dbg(wpa_s, MSG_DEBUG,
			"Use passive scan based on configuration");
	}*/ else {
		//wpa_s->prev_scan_ssid = WILDCARD_SSID_SCAN;
		params.num_ssids++;
		wpa_dbg(wpa_s, MSG_DEBUG, "Starting AP scan for wildcard "
			"SSID");
	}
#else
    if (wpa_s->last_scan_req == MANUAL_SCAN_REQ) {
        if (wpa_s->manual_scan_cfg) {
            if (wpa_s->manual_scan_cfg->ssid[0]) {
    			params.ssids[params.num_ssids].ssid =
    				wpa_s->manual_scan_cfg->ssid;
    			params.ssids[params.num_ssids].ssid_len =
    				wpa_s->manual_scan_cfg->ssid_len;
    			params.num_ssids++;
            }
            if (wpa_s->manual_scan_cfg->bssid_set)
                params.bssid = wpa_s->manual_scan_cfg->bssid;
            if (wpa_s->manual_scan_cfg->freq) {
                params.freqs = os_malloc(sizeof(int) * 2);
                if (params.freqs) {
                    params.freqs[0] = wpa_s->manual_scan_cfg->freq;
                    params.freqs[1] = 0;
                }
            }
            if (wpa_s->manual_scan_cfg->passive)
                params.passive = 1;
            if (wpa_s->manual_scan_cfg->scan_time)
                params.scan_time = wpa_s->manual_scan_cfg->scan_time;                
        }        
    } else {
        if (ssid && WPAS_MODE_INFRA == ssid->mode) {
			params.ssids[params.num_ssids].ssid =
				ssid->ssid;
			params.ssids[params.num_ssids].ssid_len =
				ssid->ssid_len;
			params.num_ssids++;
            if (wpa_s->normal_scans < 4) { //default 2
                if (ssid->frequency) {
                    params.freqs = os_malloc(sizeof(int) * 2);
                    if (params.freqs) {
                        params.freqs[0] = ssid->frequency;
                        params.freqs[1] = 0;
                    }
                }
                if (ssid->bssid_set) {
                    params.bssid = ssid->bssid;
                }
            }
        }
    }
    
	if (params.num_ssids < max_ssids) {
        params.ssids[params.num_ssids].ssid_len = 0;
        params.num_ssids++;
    }
#endif
//ssid_list_set:
	//wpa_supplicant_optimize_freqs(wpa_s, &params);
	//extra_ie = wpa_supplicant_extra_ies(wpa_s);

	/*if (wpa_s->last_scan_req == MANUAL_SCAN_REQ &&
	    wpa_s->manual_scan_only_new) {
		wpa_printf(MSG_DEBUG,
			   "Request driver to clear scan cache due to manual only_new=1 scan");
	}

	if (wpa_s->last_scan_req == MANUAL_SCAN_REQ && params.freqs == NULL &&
	    wpa_s->manual_scan_freqs) {
		wpa_dbg(wpa_s, MSG_DEBUG, "Limit manual scan to specified channels");
		params.freqs = wpa_s->manual_scan_freqs;
		wpa_s->manual_scan_freqs = NULL;
	}

	if (params.freqs == NULL && wpa_s->next_scan_freqs) {
		wpa_dbg(wpa_s, MSG_DEBUG, "Optimize scan based on previously "
			"generated frequency list");
		params.freqs = wpa_s->next_scan_freqs;
	} else
		os_free(wpa_s->next_scan_freqs);
	wpa_s->next_scan_freqs = NULL;
	wpa_setband_scan_freqs(wpa_s, &params);
    */
	/* See if user specified frequencies. If so, scan only those. */
	/*if (wpa_s->conf->freq_list && !params.freqs) {
		wpa_dbg(wpa_s, MSG_DEBUG,
			"Optimize scan based on conf->freq_list");
		int_array_concat(&params.freqs, wpa_s->conf->freq_list);
	}*/

    #if 0
	/* Use current associated channel? */
	if (wpa_s->conf->scan_cur_freq && !params.freqs) {
		unsigned int num = wpa_s->num_multichan_concurrent;

		params.freqs = os_calloc(num + 1, sizeof(int));
		if (params.freqs) {
			num = get_shared_radio_freqs(wpa_s, params.freqs, num);
			if (num > 0) {
				wpa_dbg(wpa_s, MSG_DEBUG, "Scan only the "
					"current operating channels since "
					"scan_cur_freq is enabled");
			} else {
				os_free(params.freqs);
				params.freqs = NULL;
			}
		}
	}

	if (extra_ie) {
		params.extra_ies = wpabuf_head(extra_ie);
		params.extra_ies_len = wpabuf_len(extra_ie);
	}
#endif

#ifdef CONFIG_P2P
	if (wpa_s->p2p_in_provisioning || wpa_s->p2p_in_invitation ||
	    (wpa_s->show_group_started && wpa_s->go_params)) {
		/*
		 * The interface may not yet be in P2P mode, so we have to
		 * explicitly request P2P probe to disable CCK rates.
		 */
		params.p2p_probe = 1;
	}
#endif /* CONFIG_P2P */

	/*if (!is_zero_ether_addr(wpa_s->next_scan_bssid)) {
		struct wpa_bss *bss;

		params.bssid = wpa_s->next_scan_bssid;
		bss = wpa_bss_get_bssid_latest(wpa_s, params.bssid);
		if (bss && bss->ssid_len && params.num_ssids == 1 &&
		    params.ssids[0].ssid_len == 0) {
			params.ssids[0].ssid = bss->ssid;
			params.ssids[0].ssid_len = bss->ssid_len;
			wpa_dbg(wpa_s, MSG_DEBUG,
				"Scan a previously specified BSSID " MACSTR
				" and SSID %s",
				MAC2STR(params.bssid),
				wpa_ssid_txt(bss->ssid, bss->ssid_len));
		} else {
			wpa_dbg(wpa_s, MSG_DEBUG,
				"Scan a previously specified BSSID " MACSTR,
				MAC2STR(params.bssid));
		}
	}*/
    params.max_num = wpa_s->conf->bss_max_count;
	scan_params = &params;

	ret = wpa_supplicant_trigger_scan(wpa_s, scan_params);

#if 0
	if (ret && wpa_s->last_scan_req == MANUAL_SCAN_REQ && params.freqs &&
	    !wpa_s->manual_scan_freqs) {
		/* Restore manual_scan_freqs for the next attempt */
		wpa_s->manual_scan_freqs = params.freqs;
		params.freqs = NULL;
	}
#endif

	//wpabuf_free(extra_ie);
	os_free(params.freqs);

	if (ret) {
		wpa_msg(wpa_s, MSG_WARNING, "Failed to initiate AP scan");
		if (wpa_s->scan_prev_wpa_state != wpa_s->wpa_state)
			wpa_supplicant_set_state(wpa_s,
						 wpa_s->scan_prev_wpa_state);
		/* Restore scan_req since we will try to scan again */
		wpa_s->scan_req = wpa_s->last_scan_req;
		wpa_supplicant_req_scan(wpa_s, 1, 0);
	} else {
		wpa_s->scan_for_connection = 0;
	}
}


void wpa_supplicant_update_scan_int(struct wpa_supplicant *wpa_s, int sec)
{
	struct os_reltime remaining, new_int;
	int cancelled;

	cancelled = eloop_cancel_timeout_one(wpa_supplicant_scan, wpa_s, NULL,
					     &remaining);

	new_int.sec = sec;
	new_int.usec = 0;
	if (cancelled && os_reltime_before(&remaining, &new_int)) {
		new_int.sec = remaining.sec;
		new_int.usec = remaining.usec;
	}

	if (cancelled) {
		eloop_register_timeout(new_int.sec, new_int.usec,
				       wpa_supplicant_scan, wpa_s, NULL);
	}
	wpa_s->scan_interval = sec;
}


/**
 * wpa_supplicant_req_scan - Schedule a scan for neighboring access points
 * @wpa_s: Pointer to wpa_supplicant data
 * @sec: Number of seconds after which to scan
 * @usec: Number of microseconds after which to scan
 *
 * This function is used to schedule a scan for neighboring access points after
 * the specified time.
 */
void wpa_supplicant_req_scan(struct wpa_supplicant *wpa_s, int sec, int usec)
{
	int res;

	res = eloop_deplete_timeout(sec, usec, wpa_supplicant_scan, wpa_s,
				    NULL);
	if (res == 1) {
		wpa_dbg(wpa_s, MSG_DEBUG, "Rescheduling scan request: %d.%06d sec",
			sec, usec);
	} else if (res == 0) {
		wpa_dbg(wpa_s, MSG_DEBUG, "Ignore new scan request for %d.%06d sec since an earlier request is scheduled to trigger sooner",
			sec, usec);
	} else {
		wpa_dbg(wpa_s, MSG_DEBUG, "Setting scan request: %d.%06d sec",
			sec, usec);
		eloop_register_timeout(sec, usec, wpa_supplicant_scan, wpa_s, NULL);
	}
}

#if 0
/**
 * wpa_supplicant_delayed_sched_scan - Request a delayed scheduled scan
 * @wpa_s: Pointer to wpa_supplicant data
 * @sec: Number of seconds after which to scan
 * @usec: Number of microseconds after which to scan
 * Returns: 0 on success or -1 otherwise
 *
 * This function is used to schedule periodic scans for neighboring
 * access points after the specified time.
 */
int wpa_supplicant_delayed_sched_scan(struct wpa_supplicant *wpa_s,
				      int sec, int usec)
{
	if (!wpa_s->sched_scan_supported)
		return -1;

	eloop_register_timeout(sec, usec,
			       wpa_supplicant_delayed_sched_scan_timeout,
			       wpa_s, NULL);

	return 0;
}
#endif

/**
 * wpa_supplicant_req_sched_scan - Start a periodic scheduled scan
 * @wpa_s: Pointer to wpa_supplicant data
 * Returns: 0 is sched_scan was started or -1 otherwise
 *
 * This function is used to schedule periodic scans for neighboring
 * access points repeating the scan continuously.
 */
int wpa_supplicant_req_sched_scan(struct wpa_supplicant *wpa_s)
{
#if 0
	struct wpa_driver_scan_params params;
	struct wpa_driver_scan_params *scan_params;
	enum wpa_states prev_state;
	struct wpa_ssid *ssid = NULL;
	struct wpabuf *extra_ie = NULL;
	int ret;
	unsigned int max_sched_scan_ssids;
	int wildcard = 0;
	int need_ssids;

	if (!wpa_s->sched_scan_supported)
		return -1;

	if (wpa_s->max_sched_scan_ssids > WPAS_MAX_SCAN_SSIDS)
		max_sched_scan_ssids = WPAS_MAX_SCAN_SSIDS;
	else
		max_sched_scan_ssids = wpa_s->max_sched_scan_ssids;
	if (max_sched_scan_ssids < 1/* || wpa_s->conf->disable_scan_offload*/)
		return -1;

	wpa_s->sched_scan_stop_req = 0;

	if (wpa_s->sched_scanning) {
		wpa_dbg(wpa_s, MSG_DEBUG, "Already sched scanning");
		return 0;
	}

	need_ssids = 0;
	for (ssid = wpa_s->conf->ssid; ssid; ssid = ssid->next) {
		if (!wpas_network_disabled(wpa_s, ssid) && !ssid->scan_ssid) {
			/* Use wildcard SSID to find this network */
			wildcard = 1;
		} else if (!wpas_network_disabled(wpa_s, ssid) &&
			   ssid->ssid_len)
			need_ssids++;

#ifdef CONFIG_WPS
		if (!wpas_network_disabled(wpa_s, ssid) &&
		    ssid->key_mgmt == WPA_KEY_MGMT_WPS) {
			/*
			 * Normal scan is more reliable and faster for WPS
			 * operations and since these are for short periods of
			 * time, the benefit of trying to use sched_scan would
			 * be limited.
			 */
			wpa_dbg(wpa_s, MSG_DEBUG, "Use normal scan instead of "
				"sched_scan for WPS");
			return -1;
		}
#endif /* CONFIG_WPS */
	}
	if (wildcard)
		need_ssids++;

	if (wpa_s->normal_scans < 3 &&
	    (need_ssids <= wpa_s->max_scan_ssids ||
	     wpa_s->max_scan_ssids >= (int) max_sched_scan_ssids)) {
		/*
		 * When normal scan can speed up operations, use that for the
		 * first operations before starting the sched_scan to allow
		 * user space sleep more. We do this only if the normal scan
		 * has functionality that is suitable for this or if the
		 * sched_scan does not have better support for multiple SSIDs.
		 */
		wpa_dbg(wpa_s, MSG_DEBUG, "Use normal scan instead of "
			"sched_scan for initial scans (normal_scans=%d)",
			wpa_s->normal_scans);
		return -1;
	}

	os_memset(&params, 0, sizeof(params));

	prev_state = wpa_s->wpa_state;
	if (wpa_s->wpa_state == WPA_DISCONNECTED ||
	    wpa_s->wpa_state == WPA_INACTIVE)
		wpa_supplicant_set_state(wpa_s, WPA_SCANNING);

	/* Find the starting point from which to continue scanning */
	ssid = wpa_s->conf->ssid;
	if (wpa_s->prev_sched_ssid) {
		while (ssid) {
			if (ssid == wpa_s->prev_sched_ssid) {
				ssid = ssid->next;
				break;
			}
			ssid = ssid->next;
		}
	}

	if (!ssid || !wpa_s->prev_sched_ssid) {
		wpa_dbg(wpa_s, MSG_DEBUG, "Beginning of SSID list");
		wpa_s->sched_scan_timeout = max_sched_scan_ssids * 2;
		wpa_s->first_sched_scan = 1;
		ssid = wpa_s->conf->ssid;
		wpa_s->prev_sched_ssid = ssid;
	}

	if (wildcard) {
		wpa_dbg(wpa_s, MSG_DEBUG, "Add wildcard SSID to sched_scan");
		params.num_ssids++;
	}

	while (ssid) {
		if (wpas_network_disabled(wpa_s, ssid))
			goto next;

		if (ssid->scan_ssid && ssid->ssid && ssid->ssid_len) {
			if (params.num_ssids == max_sched_scan_ssids)
				break; /* only room for broadcast SSID */
			wpa_dbg(wpa_s, MSG_DEBUG,
				"add to active scan ssid: %s",
				wpa_ssid_txt(ssid->ssid, ssid->ssid_len));
			params.ssids[params.num_ssids].ssid =
				ssid->ssid;
			params.ssids[params.num_ssids].ssid_len =
				ssid->ssid_len;
			params.num_ssids++;
			if (params.num_ssids >= max_sched_scan_ssids) {
				wpa_s->prev_sched_ssid = ssid;
				do {
					ssid = ssid->next;
				} while (ssid &&
					 (wpas_network_disabled(wpa_s, ssid) ||
					  !ssid->scan_ssid));
				break;
			}
		}

	next:
		wpa_s->prev_sched_ssid = ssid;
		ssid = ssid->next;
	}

	extra_ie = wpa_supplicant_extra_ies(wpa_s);
	if (extra_ie) {
		params.extra_ies = wpabuf_head(extra_ie);
		params.extra_ies_len = wpabuf_len(extra_ie);
	}

	/* See if user specified frequencies. If so, scan only those. */
	if (wpa_s->conf->freq_list && !params.freqs) {
		wpa_dbg(wpa_s, MSG_DEBUG,
			"Optimize scan based on conf->freq_list");
		int_array_concat(&params.freqs, wpa_s->conf->freq_list);
	}

	scan_params = &params;

	wpa_s->sched_scan_timed_out = 0;

	if (ssid || !wpa_s->first_sched_scan) {
		wpa_dbg(wpa_s, MSG_DEBUG,
			"Starting sched scan: interval %u timeout %d",
			0,
			wpa_s->sched_scan_timeout);
	} else {
		wpa_dbg(wpa_s, MSG_DEBUG, "Starting sched scan (no timeout)");
	}

	//wpa_setband_scan_freqs(wpa_s, scan_params);

	ret = wpa_supplicant_start_sched_scan(wpa_s, scan_params);
	wpabuf_free(extra_ie);
	if (ret) {
		wpa_msg(wpa_s, MSG_WARNING, "Failed to initiate sched scan");
		if (prev_state != wpa_s->wpa_state)
			wpa_supplicant_set_state(wpa_s, prev_state);
		return ret;
	}

	/* If we have more SSIDs to scan, add a timeout so we scan them too */
	if (ssid || !wpa_s->first_sched_scan) {
		wpa_s->sched_scan_timed_out = 0;
		eloop_register_timeout(wpa_s->sched_scan_timeout, 0,
				       wpa_supplicant_sched_scan_timeout,
				       wpa_s, NULL);
		wpa_s->first_sched_scan = 0;
		wpa_s->sched_scan_timeout = max_sched_scan_ssids * 2;
	}

	/* If there is no more ssids, start next time from the beginning */
	if (!ssid)
		wpa_s->prev_sched_ssid = NULL;
#endif
	return 0;
}


/**
 * wpa_supplicant_cancel_scan - Cancel a scheduled scan request
 * @wpa_s: Pointer to wpa_supplicant data
 *
 * This function is used to cancel a scan request scheduled with
 * wpa_supplicant_req_scan().
 */
void wpa_supplicant_cancel_scan(struct wpa_supplicant *wpa_s)
{
	wpa_dbg(wpa_s, MSG_DEBUG, "Cancelling scan request");
	eloop_cancel_timeout(wpa_supplicant_scan, wpa_s, NULL);
}


#if 0
/**
 * wpa_supplicant_cancel_delayed_sched_scan - Stop a delayed scheduled scan
 * @wpa_s: Pointer to wpa_supplicant data
 *
 * This function is used to stop a delayed scheduled scan.
 */
void wpa_supplicant_cancel_delayed_sched_scan(struct wpa_supplicant *wpa_s)
{
	if (!wpa_s->sched_scan_supported)
		return;

	wpa_dbg(wpa_s, MSG_DEBUG, "Cancelling delayed sched scan");
	eloop_cancel_timeout(wpa_supplicant_delayed_sched_scan_timeout,
			     wpa_s, NULL);
}
#endif

/**
 * wpa_supplicant_cancel_sched_scan - Stop running scheduled scans
 * @wpa_s: Pointer to wpa_supplicant data
 *
 * This function is used to stop a periodic scheduled scan.
 */
void wpa_supplicant_cancel_sched_scan(struct wpa_supplicant *wpa_s)
{
#if 0
	if (!wpa_s->sched_scanning)
		return;

	if (wpa_s->sched_scanning)
		wpa_s->sched_scan_stop_req = 1;

	wpa_dbg(wpa_s, MSG_DEBUG, "Cancelling sched scan");
	eloop_cancel_timeout(wpa_supplicant_sched_scan_timeout, wpa_s, NULL);
	wpa_supplicant_stop_sched_scan(wpa_s);
#endif    
}


/**
 * wpa_supplicant_notify_scanning - Indicate possible scan state change
 * @wpa_s: Pointer to wpa_supplicant data
 * @scanning: Whether scanning is currently in progress
 *
 * This function is to generate scanning notifycations. It is called whenever
 * there may have been a change in scanning (scan started, completed, stopped).
 * wpas_notify_scanning() is called whenever the scanning state changed from the
 * previously notified state.
 */
void wpa_supplicant_notify_scanning(struct wpa_supplicant *wpa_s,
				    int scanning)
{
	if (wpa_s->scanning != scanning) {
		wpa_s->scanning = scanning;
		//wpas_notify_scanning(wpa_s);
	}
}


static int wpa_scan_get_max_rate(const struct wpa_scan_res *res)
{
	int rate = 0;
	const u8 *ie;
	int i;

	ie = wpa_scan_get_ie(res, WLAN_EID_SUPP_RATES);
	for (i = 0; ie && i < ie[1]; i++) {
		if ((ie[i + 2] & 0x7f) > rate)
			rate = ie[i + 2] & 0x7f;
	}

	ie = wpa_scan_get_ie(res, WLAN_EID_EXT_SUPP_RATES);
	for (i = 0; ie && i < ie[1]; i++) {
		if ((ie[i + 2] & 0x7f) > rate)
			rate = ie[i + 2] & 0x7f;
	}

	return rate;
}


/**
 * wpa_scan_get_ie - Fetch a specified information element from a scan result
 * @res: Scan result entry
 * @ie: Information element identitifier (WLAN_EID_*)
 * Returns: Pointer to the information element (id field) or %NULL if not found
 *
 * This function returns the first matching information element in the scan
 * result.
 */
const u8 * wpa_scan_get_ie(const struct wpa_scan_res *res, u8 ie)
{
	return get_ie((const u8 *) (res + 1), res->ie_len, ie);
}


/**
 * wpa_scan_get_vendor_ie - Fetch vendor information element from a scan result
 * @res: Scan result entry
 * @vendor_type: Vendor type (four octets starting the IE payload)
 * Returns: Pointer to the information element (id field) or %NULL if not found
 *
 * This function returns the first matching information element in the scan
 * result.
 */
const u8 * wpa_scan_get_vendor_ie(const struct wpa_scan_res *res,
				  u32 vendor_type)
{
	const u8 *end, *pos;

	pos = (const u8 *) (res + 1);
	end = pos + res->ie_len;

	while (end - pos > 1) {
		if (2 + pos[1] > end - pos)
			break;
		if (pos[0] == WLAN_EID_VENDOR_SPECIFIC && pos[1] >= 4 &&
		    vendor_type == WPA_GET_BE32(&pos[2]))
			return pos;
		pos += 2 + pos[1];
	}

	return NULL;
}


/**
 * wpa_scan_get_vendor_ie_beacon - Fetch vendor information from a scan result
 * @res: Scan result entry
 * @vendor_type: Vendor type (four octets starting the IE payload)
 * Returns: Pointer to the information element (id field) or %NULL if not found
 *
 * This function returns the first matching information element in the scan
 * result.
 *
 * This function is like wpa_scan_get_vendor_ie(), but uses IE buffer only
 * from Beacon frames instead of either Beacon or Probe Response frames.
 */
const u8 * wpa_scan_get_vendor_ie_beacon(const struct wpa_scan_res *res,
					 u32 vendor_type)
{
	const u8 *end, *pos;

	if (res->beacon_ie_len == 0)
		return NULL;

	pos = (const u8 *) (res + 1);
	pos += res->ie_len;
	end = pos + res->beacon_ie_len;

	while (end - pos > 1) {
		if (2 + pos[1] > end - pos)
			break;
		if (pos[0] == WLAN_EID_VENDOR_SPECIFIC && pos[1] >= 4 &&
		    vendor_type == WPA_GET_BE32(&pos[2]))
			return pos;
		pos += 2 + pos[1];
	}

	return NULL;
}


/**
 * wpa_scan_get_vendor_ie_multi - Fetch vendor IE data from a scan result
 * @res: Scan result entry
 * @vendor_type: Vendor type (four octets starting the IE payload)
 * Returns: Pointer to the information element payload or %NULL if not found
 *
 * This function returns concatenated payload of possibly fragmented vendor
 * specific information elements in the scan result. The caller is responsible
 * for freeing the returned buffer.
 */
struct wpabuf * wpa_scan_get_vendor_ie_multi(const struct wpa_scan_res *res,
					     u32 vendor_type)
{
	struct wpabuf *buf;
	const u8 *end, *pos;

	buf = wpabuf_alloc(res->ie_len);
	if (buf == NULL)
		return NULL;

	pos = (const u8 *) (res + 1);
	end = pos + res->ie_len;

	while (end - pos > 1) {
		if (2 + pos[1] > end - pos)
			break;
		if (pos[0] == WLAN_EID_VENDOR_SPECIFIC && pos[1] >= 4 &&
		    vendor_type == WPA_GET_BE32(&pos[2]))
			wpabuf_put_data(buf, pos + 2 + 4, pos[1] - 4);
		pos += 2 + pos[1];
	}

	if (wpabuf_len(buf) == 0) {
		wpabuf_free(buf);
		buf = NULL;
	}

	return buf;
}


/*
 * Channels with a great SNR can operate at full rate. What is a great SNR?
 * This doc https://supportforums.cisco.com/docs/DOC-12954 says, "the general
 * rule of thumb is that any SNR above 20 is good." This one
 * http://www.cisco.com/en/US/tech/tk722/tk809/technologies_q_and_a_item09186a00805e9a96.shtml#qa23
 * recommends 25 as a minimum SNR for 54 Mbps data rate. 30 is chosen here as a
 * conservative value.
 */
#define GREAT_SNR 30

#define IS_5GHZ(n) (n > 4000)

/* Compare function for sorting scan results. Return >0 if @b is considered
 * better. */
static int wpa_scan_result_compar(const void *a, const void *b)
{
#define MIN(a,b) a < b ? a : b
	struct wpa_scan_res **_wa = (void *) a;
	struct wpa_scan_res **_wb = (void *) b;
	struct wpa_scan_res *wa = *_wa;
	struct wpa_scan_res *wb = *_wb;
	int wpa_a, wpa_b;
	int snr_a, snr_b, snr_a_full, snr_b_full;

	/* WPA/WPA2 support preferred */
	wpa_a = wpa_scan_get_vendor_ie(wa, WPA_IE_VENDOR_TYPE) != NULL ||
		wpa_scan_get_ie(wa, WLAN_EID_RSN) != NULL;
	wpa_b = wpa_scan_get_vendor_ie(wb, WPA_IE_VENDOR_TYPE) != NULL ||
		wpa_scan_get_ie(wb, WLAN_EID_RSN) != NULL;

	if (wpa_b && !wpa_a)
		return 1;
	if (!wpa_b && wpa_a)
		return -1;

	/* privacy support preferred */
	if ((wa->caps & IEEE80211_CAP_PRIVACY) == 0 &&
	    (wb->caps & IEEE80211_CAP_PRIVACY))
		return 1;
	if ((wa->caps & IEEE80211_CAP_PRIVACY) &&
	    (wb->caps & IEEE80211_CAP_PRIVACY) == 0)
		return -1;

	if (wa->flags & wb->flags & WPA_SCAN_LEVEL_DBM) {
		snr_a_full = wa->snr;
		snr_a = MIN(wa->snr, GREAT_SNR);
		snr_b_full = wb->snr;
		snr_b = MIN(wb->snr, GREAT_SNR);
	} else {
		/* Level is not in dBm, so we can't calculate
		 * SNR. Just use raw level (units unknown). */
		snr_a = snr_a_full = wa->level;
		snr_b = snr_b_full = wb->level;
	}

	/* if SNR is close, decide by max rate or frequency band */
	if ((snr_a && snr_b && abs(snr_b - snr_a) < 5) ||
	    (wa->qual && wb->qual && abs(wb->qual - wa->qual) < 10)) {
		if (wa->est_throughput != wb->est_throughput)
			return wb->est_throughput - wa->est_throughput;
		if (IS_5GHZ(wa->freq) ^ IS_5GHZ(wb->freq))
			return IS_5GHZ(wa->freq) ? -1 : 1;
	}

	/* all things being equal, use SNR; if SNRs are
	 * identical, use quality values since some drivers may only report
	 * that value and leave the signal level zero */
	if (snr_b_full == snr_a_full)
		return wb->qual - wa->qual;
	return snr_b_full - snr_a_full;
#undef MIN
}


#ifdef CONFIG_WPS
/* Compare function for sorting scan results when searching a WPS AP for
 * provisioning. Return >0 if @b is considered better. */
static int wpa_scan_result_wps_compar(const void *a, const void *b)
{
	struct wpa_scan_res **_wa = (void *) a;
	struct wpa_scan_res **_wb = (void *) b;
	struct wpa_scan_res *wa = *_wa;
	struct wpa_scan_res *wb = *_wb;
	int uses_wps_a, uses_wps_b;
	struct wpabuf *wps_a, *wps_b;
	int res;

	/* Optimization - check WPS IE existence before allocated memory and
	 * doing full reassembly. */
	uses_wps_a = wpa_scan_get_vendor_ie(wa, WPS_IE_VENDOR_TYPE) != NULL;
	uses_wps_b = wpa_scan_get_vendor_ie(wb, WPS_IE_VENDOR_TYPE) != NULL;
	if (uses_wps_a && !uses_wps_b)
		return -1;
	if (!uses_wps_a && uses_wps_b)
		return 1;

	if (uses_wps_a && uses_wps_b) {
		wps_a = wpa_scan_get_vendor_ie_multi(wa, WPS_IE_VENDOR_TYPE);
		wps_b = wpa_scan_get_vendor_ie_multi(wb, WPS_IE_VENDOR_TYPE);
		res = wps_ap_priority_compar(wps_a, wps_b);
		wpabuf_free(wps_a);
		wpabuf_free(wps_b);
		if (res)
			return res;
	}

	/*
	 * Do not use current AP security policy as a sorting criteria during
	 * WPS provisioning step since the AP may get reconfigured at the
	 * completion of provisioning.
	 */

	/* all things being equal, use signal level; if signal levels are
	 * identical, use quality values since some drivers may only report
	 * that value and leave the signal level zero */
	if (wb->level == wa->level)
		return wb->qual - wa->qual;
	return wb->level - wa->level;
}
#endif /* CONFIG_WPS */


static void dump_scan_res(struct wpa_scan_results *scan_res)
{
#ifndef CONFIG_NO_STDOUT_DEBUG
	size_t i;

	if (scan_res->res == NULL || scan_res->num == 0)
		return;

	wpa_printf(MSG_EXCESSIVE, "Sorted scan results");

	for (i = 0; i < scan_res->num; i++) {
		struct wpa_scan_res *r = scan_res->res[i];
		u8 *pos;
		if (r->flags & WPA_SCAN_LEVEL_DBM) {
			int noise_valid = !(r->flags & WPA_SCAN_NOISE_INVALID);

			wpa_printf(MSG_EXCESSIVE, MACSTR " freq=%d qual=%d "
				   "noise=%d%s level=%d snr=%d%s flags=0x%x age=%u est=%u",
				   MAC2STR(r->bssid), r->freq, r->qual,
				   r->noise, noise_valid ? "" : "~", r->level,
				   r->snr, r->snr >= GREAT_SNR ? "*" : "",
				   r->flags,
				   r->age, r->est_throughput);
		} else {
			wpa_printf(MSG_EXCESSIVE, MACSTR " freq=%d qual=%d "
				   "noise=%d level=%d flags=0x%x age=%u est=%u",
				   MAC2STR(r->bssid), r->freq, r->qual,
				   r->noise, r->level, r->flags, r->age,
				   r->est_throughput);
		}
		pos = (u8 *) (r + 1);
		if (r->ie_len)
			wpa_hexdump(MSG_EXCESSIVE, "IEs", pos, r->ie_len);
		pos += r->ie_len;
		if (r->beacon_ie_len)
			wpa_hexdump(MSG_EXCESSIVE, "Beacon IEs",
				    pos, r->beacon_ie_len);
	}
#endif /* CONFIG_NO_STDOUT_DEBUG */
}

/*
 * Noise floor values to use when we have signal strength
 * measurements, but no noise floor measurements. These values were
 * measured in an office environment with many APs.
 */
#define DEFAULT_NOISE_FLOOR_2GHZ (-89)
#define DEFAULT_NOISE_FLOOR_5GHZ (-92)

void scan_snr(struct wpa_scan_res *res)
{
	if (res->flags & WPA_SCAN_NOISE_INVALID) {
		res->noise = IS_5GHZ(res->freq) ?
			DEFAULT_NOISE_FLOOR_5GHZ :
			DEFAULT_NOISE_FLOOR_2GHZ;
	}

	if (res->flags & WPA_SCAN_LEVEL_DBM) {
		res->snr = res->level - res->noise;
	} else {
		/* Level is not in dBm, so we can't calculate
		 * SNR. Just use raw level (units unknown). */
		res->snr = res->level;
	}
}


static unsigned int max_ht20_rate(int snr)
{
	if (snr < 6)
		return 6500; /* HT20 MCS0 */
	if (snr < 8)
		return 13000; /* HT20 MCS1 */
	if (snr < 13)
		return 19500; /* HT20 MCS2 */
	if (snr < 17)
		return 26000; /* HT20 MCS3 */
	if (snr < 20)
		return 39000; /* HT20 MCS4 */
	if (snr < 23)
		return 52000; /* HT20 MCS5 */
	if (snr < 24)
		return 58500; /* HT20 MCS6 */
	return 65000; /* HT20 MCS7 */
}


static unsigned int max_ht40_rate(int snr)
{
	if (snr < 3)
		return 13500; /* HT40 MCS0 */
	if (snr < 6)
		return 27000; /* HT40 MCS1 */
	if (snr < 10)
		return 40500; /* HT40 MCS2 */
	if (snr < 15)
		return 54000; /* HT40 MCS3 */
	if (snr < 17)
		return 81000; /* HT40 MCS4 */
	if (snr < 22)
		return 108000; /* HT40 MCS5 */
	if (snr < 24)
		return 121500; /* HT40 MCS6 */
	return 135000; /* HT40 MCS7 */
}


static unsigned int max_vht80_rate(int snr)
{
	if (snr < 1)
		return 0;
	if (snr < 2)
		return 29300; /* VHT80 MCS0 */
	if (snr < 5)
		return 58500; /* VHT80 MCS1 */
	if (snr < 9)
		return 87800; /* VHT80 MCS2 */
	if (snr < 11)
		return 117000; /* VHT80 MCS3 */
	if (snr < 15)
		return 175500; /* VHT80 MCS4 */
	if (snr < 16)
		return 234000; /* VHT80 MCS5 */
	if (snr < 18)
		return 263300; /* VHT80 MCS6 */
	if (snr < 20)
		return 292500; /* VHT80 MCS7 */
	if (snr < 22)
		return 351000; /* VHT80 MCS8 */
	return 390000; /* VHT80 MCS9 */
}


void scan_est_throughput(struct wpa_supplicant *wpa_s,
			 struct wpa_scan_res *res)
{
	enum local_hw_capab capab = wpa_s->hw_capab;
	int rate; /* max legacy rate in 500 kb/s units */
	const u8 *ie;
	unsigned int est, tmp;
	int snr = res->snr;

	if (res->est_throughput)
		return;

	/* Get maximum legacy rate */
	rate = wpa_scan_get_max_rate(res);

	/* Limit based on estimated SNR */
	if (rate > 1 * 2 && snr < 1)
		rate = 1 * 2;
	else if (rate > 2 * 2 && snr < 4)
		rate = 2 * 2;
	else if (rate > 6 * 2 && snr < 5)
		rate = 6 * 2;
	else if (rate > 9 * 2 && snr < 6)
		rate = 9 * 2;
	else if (rate > 12 * 2 && snr < 7)
		rate = 12 * 2;
	else if (rate > 18 * 2 && snr < 10)
		rate = 18 * 2;
	else if (rate > 24 * 2 && snr < 11)
		rate = 24 * 2;
	else if (rate > 36 * 2 && snr < 15)
		rate = 36 * 2;
	else if (rate > 48 * 2 && snr < 19)
		rate = 48 * 2;
	else if (rate > 54 * 2 && snr < 21)
		rate = 54 * 2;
	est = rate * 500;

	if (capab == CAPAB_HT || capab == CAPAB_HT40 || capab == CAPAB_VHT) {
		ie = wpa_scan_get_ie(res, WLAN_EID_HT_CAP);
		if (ie) {
			tmp = max_ht20_rate(snr);
			if (tmp > est)
				est = tmp;
		}
	}

	if (capab == CAPAB_HT40 || capab == CAPAB_VHT) {
		ie = wpa_scan_get_ie(res, WLAN_EID_HT_OPERATION);
		if (ie && ie[1] >= 2 &&
		    (ie[3] & HT_INFO_HT_PARAM_SECONDARY_CHNL_OFF_MASK)) {
			tmp = max_ht40_rate(snr);
			if (tmp > est)
				est = tmp;
		}
	}

	if (capab == CAPAB_VHT) {
		/* Use +1 to assume VHT is always faster than HT */
		ie = wpa_scan_get_ie(res, WLAN_EID_VHT_CAP);
		if (ie) {
			tmp = max_ht20_rate(snr) + 1;
			if (tmp > est)
				est = tmp;

			ie = wpa_scan_get_ie(res, WLAN_EID_HT_OPERATION);
			if (ie && ie[1] >= 2 &&
			    (ie[3] &
			     HT_INFO_HT_PARAM_SECONDARY_CHNL_OFF_MASK)) {
				tmp = max_ht40_rate(snr) + 1;
				if (tmp > est)
					est = tmp;
			}

			ie = wpa_scan_get_ie(res, WLAN_EID_VHT_OPERATION);
			if (ie && ie[1] >= 1 &&
			    (ie[2] & VHT_OPMODE_CHANNEL_WIDTH_MASK)) {
				tmp = max_vht80_rate(snr) + 1;
				if (tmp > est)
					est = tmp;
			}
		}
	}

	/* TODO: channel utilization and AP load (e.g., from AP Beacon) */

	res->est_throughput = est;
}


/**
 * wpa_supplicant_get_scan_results - Get scan results
 * @wpa_s: Pointer to wpa_supplicant data
 * @info: Information about what was scanned or %NULL if not available
 * @new_scan: Whether a new scan was performed
 * Returns: Scan results, %NULL on failure
 *
 * This function request the current scan results from the driver and updates
 * the local BSS list wpa_s->bss. The caller is responsible for freeing the
 * results with wpa_scan_results_free().
 */
struct wpa_scan_results *
wpa_supplicant_get_scan_results(struct wpa_supplicant *wpa_s,
				struct scan_info *info, int new_scan)
{
	struct wpa_scan_results *scan_res;
	size_t i;
	//int (*compar)(const void *, const void *) = wpa_scan_result_compar;

	scan_res = wpa_drv_get_scan_results2(wpa_s);
	if (scan_res == NULL) {
		wpa_dbg(wpa_s, MSG_DEBUG, "Failed to get scan results");
		return NULL;
	}
	if (scan_res->fetch_time.sec == 0) {
		/*
		 * Make sure we have a valid timestamp if the driver wrapper
		 * does not set this.
		 */
		os_get_reltime(&scan_res->fetch_time);
	}
#if 0    
	wpa_dbg(wpa_s , MSG_DEBUG , "scan_res(0x%x)" , (unsigned int )scan_res);
	for (i = 0; i < scan_res->num; i++) {
		struct wpa_scan_res *scan_res_item = scan_res->res[i];
		wpa_dbg(wpa_s , MSG_DEBUG , "scan_res_item(0x%x)" , (unsigned int )(&scan_res->res[i]));
		//scan_snr(scan_res_item);
		//scan_est_throughput(wpa_s, scan_res_item);
	}
#endif
#ifdef CONFIG_WPS
	if (wpas_wps_searching(wpa_s)) {
		wpa_dbg(wpa_s, MSG_DEBUG, "WPS: Order scan results with WPS "
			"provisioning rules");
		compar = wpa_scan_result_wps_compar;
	}
	qsort(scan_res->res, scan_res->num, sizeof(struct wpa_scan_res *), compar);
#endif /* CONFIG_WPS */

	//dump_scan_res(scan_res);

	wpa_bss_update_start(wpa_s);
	for (i = 0; i < scan_res->num; i++)
		wpa_bss_update_scan_res(wpa_s, scan_res->res[i],
					&scan_res->fetch_time);
	wpa_bss_update_end(wpa_s, info, new_scan);

	return scan_res;
}


/**
 * wpa_supplicant_update_scan_results - Update scan results from the driver
 * @wpa_s: Pointer to wpa_supplicant data
 * Returns: 0 on success, -1 on failure
 *
 * This function updates the BSS table within wpa_supplicant based on the
 * currently available scan results from the driver without requesting a new
 * scan. This is used in cases where the driver indicates an association
 * (including roaming within ESS) and wpa_supplicant does not yet have the
 * needed information to complete the connection (e.g., to perform validation
 * steps in 4-way handshake).
 */
int wpa_supplicant_update_scan_results(struct wpa_supplicant *wpa_s)
{
	struct wpa_scan_results *scan_res;
	scan_res = wpa_supplicant_get_scan_results(wpa_s, NULL, 0);
	if (scan_res == NULL)
		return -1;
	wpa_scan_results_free(scan_res);

	return 0;
}


/**
 * scan_only_handler - Reports scan results
 */
void scan_only_handler(struct wpa_supplicant *wpa_s,
		       struct wpa_scan_results *scan_res)
{
	wpa_dbg(wpa_s, MSG_DEBUG, "Scan-only results received");
	/*if (wpa_s->last_scan_req == MANUAL_SCAN_REQ &&
	    wpa_s->manual_scan_use_id && wpa_s->own_scan_running) {
		wpa_msg_ctrl(wpa_s, MSG_INFO, WPA_EVENT_SCAN_RESULTS "id=%u",
			     wpa_s->manual_scan_id);
		wpa_s->manual_scan_use_id = 0;
	} else {
		wpa_msg_ctrl(wpa_s, MSG_INFO, WPA_EVENT_SCAN_RESULTS);
	}*/
	wpas_notify_scan_results(wpa_s);
	wpas_notify_scan_done(wpa_s, 1);
	if (wpa_s->scan_work) {
		struct wpa_radio_work *work = wpa_s->scan_work;
		wpa_s->scan_work = NULL;
		radio_work_done(work);
	}

	if (wpa_s->wpa_state == WPA_SCANNING)
		wpa_supplicant_set_state(wpa_s, wpa_s->scan_prev_wpa_state);
}


int wpas_scan_scheduled(struct wpa_supplicant *wpa_s)
{
	return eloop_is_timeout_registered(wpa_supplicant_scan, wpa_s, NULL);
}


struct wpa_driver_scan_params *
wpa_scan_clone_params(const struct wpa_driver_scan_params *src)
{
	struct wpa_driver_scan_params *params;
	size_t i;
	u8 *n;

	params = os_zalloc(sizeof(*params));
	if (params == NULL)
		return NULL;

	for (i = 0; i < src->num_ssids; i++) {
		if (src->ssids[i].ssid) {
			n = os_malloc(src->ssids[i].ssid_len);
			if (n == NULL)
				goto failed;
			os_memcpy(n, src->ssids[i].ssid,
				  src->ssids[i].ssid_len);
			params->ssids[i].ssid = n;
			params->ssids[i].ssid_len = src->ssids[i].ssid_len;
		}
	}
	params->num_ssids = src->num_ssids;

	/*if (src->extra_ies) {
		n = os_malloc(src->extra_ies_len);
		if (n == NULL)
			goto failed;
		os_memcpy(n, src->extra_ies, src->extra_ies_len);
		params->extra_ies = n;
		params->extra_ies_len = src->extra_ies_len;
	}*/

	if (src->freqs) {
		int len = int_array_len(src->freqs);
		params->freqs = os_malloc((len + 1) * sizeof(int));
		if (params->freqs == NULL)
			goto failed;
		os_memcpy(params->freqs, src->freqs, (len + 1) * sizeof(int));
	}

	if (src->bssid) {
		u8 *bssid;

		bssid = os_malloc(ETH_ALEN);
		if (!bssid)
			goto failed;
		os_memcpy(bssid, src->bssid, ETH_ALEN);
		params->bssid = bssid;
	}

    params->passive = src->passive;
    params->scan_time = src->scan_time;
    params->max_num = src->max_num;

	return params;

failed:
	wpa_scan_free_params(params);
	return NULL;
}


void wpa_scan_free_params(struct wpa_driver_scan_params *params)
{
	size_t i;

	if (params == NULL)
		return;

	for (i = 0; i < params->num_ssids; i++)
		os_free((u8 *) params->ssids[i].ssid);
	os_free((u8 *) params->extra_ies);
	os_free(params->freqs);

	os_free((u8 *) params->bssid);

	os_free(params);
}


int wpas_abort_ongoing_scan(struct wpa_supplicant *wpa_s)
{
	int scan_work = !!wpa_s->scan_work;

#ifdef CONFIG_P2P
	scan_work |= !!wpa_s->p2p_scan_work;
#endif /* CONFIG_P2P */

	if (scan_work && wpa_s->own_scan_running) {
		wpa_dbg(wpa_s, MSG_DEBUG, "Abort an ongoing scan");
		return wpa_drv_abort_scan(wpa_s);
	}

	return 0;
}


int wpas_sched_scan_plans_set(struct wpa_supplicant *wpa_s, const char *cmd)
{
#if 0
	struct sched_scan_plan *scan_plans = NULL;
	const char *token, *context = NULL;
	unsigned int num = 0;

	if (!cmd)
		return -1;

	if (!cmd[0]) {
		wpa_printf(MSG_DEBUG, "Clear sched scan plans");
		wpa_s->sched_scan_plans_num = 0;
		return 0;
	}

	while ((token = cstr_token(cmd, " ", &context))) {
		int ret;
		struct sched_scan_plan *scan_plan, *n;

		n = os_realloc_array(scan_plans, num + 1, sizeof(*scan_plans));
		if (!n)
			goto fail;

		scan_plans = n;
		scan_plan = &scan_plans[num];
		num++;

		ret = sscanf(token, "%u:%u", &scan_plan->interval,
			     &scan_plan->iterations);
		if (ret <= 0 || ret > 2 || !scan_plan->interval) {
			wpa_printf(MSG_ERROR,
				   "Invalid sched scan plan input: %s", token);
			goto fail;
		}

		if (scan_plan->interval > wpa_s->max_sched_scan_plan_interval) {
			wpa_printf(MSG_WARNING,
				   "scan plan %u: Scan interval too long(%u), use the maximum allowed(%u)",
				   num, scan_plan->interval,
				   wpa_s->max_sched_scan_plan_interval);
			scan_plan->interval =
				wpa_s->max_sched_scan_plan_interval;
		}

		if (ret == 1) {
			scan_plan->iterations = 0;
			break;
		}

		if (!scan_plan->iterations) {
			wpa_printf(MSG_ERROR,
				   "scan plan %u: Number of iterations cannot be zero",
				   num);
			goto fail;
		}

		if (scan_plan->iterations >
		    wpa_s->max_sched_scan_plan_iterations) {
			wpa_printf(MSG_WARNING,
				   "scan plan %u: Too many iterations(%u), use the maximum allowed(%u)",
				   num, scan_plan->iterations,
				   wpa_s->max_sched_scan_plan_iterations);
			scan_plan->iterations =
				wpa_s->max_sched_scan_plan_iterations;
		}

		wpa_printf(MSG_DEBUG,
			   "scan plan %u: interval=%u iterations=%u",
			   num, scan_plan->interval, scan_plan->iterations);
	}

	if (!scan_plans) {
		wpa_printf(MSG_ERROR, "Invalid scan plans entry");
		goto fail;
	}

	if (cstr_token(cmd, " ", &context) || scan_plans[num - 1].iterations) {
		wpa_printf(MSG_ERROR,
			   "All scan plans but the last must specify a number of iterations");
		goto fail;
	}

	wpa_printf(MSG_DEBUG, "scan plan %u (last plan): interval=%u",
		   num, scan_plans[num - 1].interval);

	if (num > wpa_s->max_sched_scan_plans) {
		wpa_printf(MSG_WARNING,
			   "Too many scheduled scan plans (only %u supported)",
			   wpa_s->max_sched_scan_plans);
		wpa_printf(MSG_WARNING,
			   "Use only the first %u scan plans, and the last one (in infinite loop)",
			   wpa_s->max_sched_scan_plans - 1);
		os_memcpy(&scan_plans[wpa_s->max_sched_scan_plans - 1],
			  &scan_plans[num - 1], sizeof(*scan_plans));
		num = wpa_s->max_sched_scan_plans;
	}

	wpa_s->sched_scan_plans_num = num;

	return 0;

fail:
	os_free(scan_plans);
	wpa_printf(MSG_ERROR, "invalid scan plans list");
	return -1;
#endif 
    return 0;
}


/**
 * wpas_scan_reset_sched_scan - Reset sched_scan state
 * @wpa_s: Pointer to wpa_supplicant data
 *
 * This function is used to cancel a running scheduled scan and to reset an
 * internal scan state to continue with a regular scan on the following
 * wpa_supplicant_req_scan() calls.
 */
void wpas_scan_reset_sched_scan(struct wpa_supplicant *wpa_s)
{
	wpa_s->normal_scans = 0;
	/*if (wpa_s->sched_scanning) {
		wpa_s->sched_scan_timed_out = 0;
		wpa_s->prev_sched_ssid = NULL;
		//wpa_supplicant_cancel_sched_scan(wpa_s);
	}*/
}


void wpas_scan_restart_sched_scan(struct wpa_supplicant *wpa_s)
{
	/* simulate timeout to restart the sched scan */
	//wpa_s->sched_scan_timed_out = 1;
	//wpa_s->prev_sched_ssid = NULL;
	//wpa_supplicant_cancel_sched_scan(wpa_s);
}

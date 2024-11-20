

//#include <stdarg.h>
//#include <stdio.h>
//#include "sys_buf.h"
#include "system_common.h"
#include "system_modem_api.h"
#include "drv_host_interface.h"
#include "util_sysbuf_queue.h"
#include "util_cmd.h"
#include "umac_wim_dispatcher.h"
#include "umac_wim_manager.h"

static struct host_interface_ops *hops = NULL;
static bool tx_enable = false;
static void callback_from_downlink(/*int vif_id, */SYS_BUF *packet);
static void callback_update_credit_from_uplink(uint8_t ac, uint8_t value, bool inc);

void hal_host_interface_register_ops(struct host_interface_ops *ops)
{
	hops = ops;
	util_sysbuf_queue_init(&ops->to_host_que);
}

void hal_host_interface_init()
{
	struct wim_manager_external_interface wm_ext_if = {
		.tx_enable = hal_host_interface_tx_enable,
		.send_to = hal_host_interface_send_to_host
	};

	hops->init(hops->priv);
	umac_wim_manager_register_external_interface(&wm_ext_if);
}

void hal_host_interface_deinit()
{
	hops->deinit(hops->priv);
}

void hal_host_interface_send_to_host(SYS_BUF *packet)
{
	const int n_chunk = LMAC_CONFIG_BUFFER_SIZE - sizeof(SYS_HDR);
	if (!tx_enable) {
		discard(packet);
		return;
	}

	if (hops->support_stream_mode) {
		struct hif_hdr *hif = (struct hif_hdr*)&packet->lmac_rxhdr;
		SYS_BUF *iter = packet;
		int n_total = MAX(hif->len + sizeof(*hif), n_chunk);
		while(iter) {
			SYS_BUF *link = SYS_BUF_LINK(iter);
			SYS_BUF_LINK(iter) = NULL;
			uint32_t length = MIN(n_total, n_chunk);
			n_total -= length;
			if (hops->support_stream_align) {
				length = (length + 3) & 0xFFFFFFFC;
				SYS_HDR(iter).m_payload_length = length;
			}
			util_sysbuf_queue_push(&hops->to_host_que, iter);
			iter = link;
		}
	} else {
		util_sysbuf_queue_push(&hops->to_host_que, packet);
	}

	hops->request_to_host(hops->priv);
}


void hal_host_interface_update_slot(uint8_t value, bool inc)
{
	hops->update_slot(hops->priv, value, inc);
}

void hal_host_interface_start()
{
	hops->start(hops->priv);
	hal_lmac_set_dl_callback(callback_from_downlink);
	hal_lmac_set_credit_callback(callback_update_credit_from_uplink);
}

void hal_host_interface_stop()
{
	hal_lmac_set_dl_callback(NULL);
	hops->stop(hops->priv);
}

static void callback_update_credit_from_uplink(uint8_t ac, uint8_t value, bool inc)
{
	hops->update_credit(hops->priv, ac, value, inc);
}

static void callback_from_downlink(/*int vif_id, */SYS_BUF *packet)
{
	int hif_len = system_modem_api_get_dl_hif_length(packet);
	struct hif_hdr *hif = (struct hif_hdr*)&packet->lmac_rxhdr;

    hif->type = HIF_TYPE_FRAME;
    hif->len = hif_len;

	struct frame_hdr *fh = (struct frame_hdr*) (hif+1);
	fh->flags.rx.error_mic = packet->lmac_rxhdr.rxinfo.error_mic;
	fh->flags.rx.iv_stripped = 0;
	fh->flags.rx.rssi = system_modem_api_get_rssi(packet);
	fh->flags.rx.snr = min(system_modem_api_get_snr(packet), 2e6-1);
	//README swki 2019-0124 -- check that the freq is channel or frequency.
	// assume that channel is always under 1000.
	// relate to: https://192.168.1.237:8443/cb/issue/8436
	if (fh->info.rx.frequency < 1000)
		fh->info.rx.frequency = system_modem_api_get_frequency(packet);

	hal_host_interface_send_to_host(packet);
}

static bool get_tx_info(SYS_BUF *buf, struct frame_tx_info *ti)
{
	int tlv_len	 = 0, tot_len = 0, offset = 0;
	SYS_BUF *cur = buf;

	if (!buf || buf->hif_hdr.type != HIF_TYPE_FRAME
		|| !(tlv_len = FRAME_HDR(buf).info.tx.tlv_len))
		return false;

	tot_len = HIF_HDR(buf).len + sizeof(HIF_HDR);
	offset  = tot_len - tlv_len;

	while (cur && offset > SYS_BUF_LENGTH(cur) - sizeof(LMAC_TXHDR)) {
		offset -= SYS_BUF_LENGTH(cur) - sizeof(LMAC_TXHDR);
		cur		= SYS_BUF_LINK(cur);
	}

	if (offset <= SYS_BUF_LENGTH(cur) - sizeof(LMAC_TXHDR) &&
		offset + sizeof(struct frame_tx_info) > SYS_BUF_LENGTH(cur) - sizeof(LMAC_TXHDR) &&
		SYS_BUF_LINK(cur) != NULL) {
		int first	= SYS_BUF_LENGTH(cur) - sizeof(LMAC_TXHDR) - offset;
		int second	= sizeof(struct frame_tx_info) - first;
		memcpy((uint8_t *) ti, cur->more_payload + offset, first);
		memcpy((uint8_t *) ti + first, SYS_BUF_LINK(cur)->more_payload, second);
		SYS_BUF_LENGTH(cur)	-= first;
		SYS_BUF_LENGTH(SYS_BUF_LINK(cur)) -= second;
	}
	else {
		memcpy((uint8_t *) ti, cur->more_payload + offset, sizeof(struct frame_tx_info));
		SYS_BUF_LENGTH(cur)		-= tlv_len;
	}

	HIF_HDR(buf).len		-= tlv_len;


	return true;
}

static void process_tx_info(SYS_BUF *buf, struct frame_tx_info *ti)
{
	#if 0 // TODO:
	system_printf("%s: use_rts(%d), use_cts_prot(%d), short_preamble(%d), "
		"ampdu(%d), after_dtim(%d), no_ack(%d), eosp(%d), rvd(%d) \n",
		__func__, ti->v.use_rts, ti->v.use_11b_protection, ti->v.short_preamble,
		ti->v.ampdu, ti->v.after_dtim, ti->v.no_ack, ti->v.eosp, ti->v.rvd);
	#endif
}

static int32_t wq_packet_process(void *param)
{
	bool ret = false;
	bool has_txi = false;
	struct frame_tx_info txi;
	SYS_BUF *packet = (SYS_BUF*) param;

	V(TT_WIM, "%s hif-type:%d, len:%d\n", 
			__func__, packet->hif_hdr.type, packet->hif_hdr.len);
	switch (packet->hif_hdr.type) {
		case HIF_TYPE_FRAME:
			has_txi = get_tx_info(packet, &txi);
			hal_lmac_uplink_request_sysbuf(packet, has_txi ? &txi : NULL);
			ret = true;
			break;
		case HIF_TYPE_WIM:
			ret = umac_wim_dispatcher_run(packet);
			break;
		default:
			break;
	}

	if (!ret) {
		system_memory_pool_free(packet);
		return -1;
	}

	return 0;
}

void hal_host_interface_rx_done(SYS_BUF *packet)
{
	bool ret = system_schedule_work_queue_from_isr(wq_packet_process, packet, NULL);

	if (!ret) {
		system_memory_pool_free(packet);
	}
}

void hal_host_interface_tx_enable(bool enable)
{
	unsigned long flags = system_irq_save();
	tx_enable = enable;
	system_irq_restore(flags);
}

static int cmd_show_hif(cmd_tbl_t *t, int argc, char *argv[])
{
    system_printf("Host Interface Status\n");
    hops->show(hops->priv);
    return CMD_RET_SUCCESS;
}

SUBCMD(show,
       hif,
       cmd_show_hif,
       "display hif status",
       "show hif");

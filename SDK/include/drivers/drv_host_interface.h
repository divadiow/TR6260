#ifndef __HAL_HOST_INTERFACE_H__
#define __HAL_HOST_INTERFACE_H__

#include "util_sysbuf_queue.h"

struct host_interface_ops {
	struct sysbuf_queue to_host_que;
	bool support_stream_mode;
	bool support_stream_align;

	/* below fields are filled by host interface implement */
	void *priv;
	void (*init)(void *priv);
	void (*deinit)(void *priv);
	void (*start)(void *priv);
	void (*stop)(void *priv);
	void (*request_to_host)(void *priv);
	void (*show)(void *priv);
	void (*update_credit)(void *priv, uint8_t ac, uint8_t value, bool inc);
	void (*update_slot)(void *priv, uint8_t value, bool inc);
};

void hal_host_interface_register_ops(struct host_interface_ops *ops);
void hal_host_interface_init();
void hal_host_interface_start();
void hal_host_interface_stop();
void hal_host_interface_send_to_host(SYS_BUF *packet);
void hal_host_interface_update_credit(uint8_t ac, uint8_t value, bool inc);
void hal_host_interface_update_slot(uint8_t value, bool inc);
void hal_host_interface_rx_done(SYS_BUF *packet);
void hal_host_interface_tx_enable(bool enable);

#endif //__HAL_HOST_INTERFACE_H__


#include <nds32_intrinsic.h>
#include "system_common.h"
#include "bsp/soc_top_reg.h"
#include "bsp/soc_pin_mux.h"

#include "nrc-wim-types.h"
#include "util_sysbuf_queue.h"
#include "drv_host_interface.h"
#include "drv_sdio.h"
#include "drv_sdio_posedge.h"
#include "irq.h"

#define 	    HIF_SDIO_TX  0
#define 	    HIF_SDIO_RX  1


#define QUE_MAX (2)

#define STATE_IDLE 	    (0)
#define STATE_TX 	    (1)
#define STATE_RX 	    (2)
#define STATE_INFO 	    (3)
#define STATE_MAX 	    (4)

#define EVENT_TX_START 		(0)
#define EVENT_RX_START 		(1)
#define EVENT_INFO_START 	(2)
#define EVENT_COMPLETE 		(3)
#define EVENT_TX_REQUEST 	(4)

#define SDIO_ADDR_INFO		        (0x101)
#define SDIO_ADDR_INFO_ASYNC		(0x111)
#define SDIO_ADDR_DATA		        (0x200)

#define SDIO_PRIV(x) ((struct sdio_priv*)x)

#define RX_REPORT_DEFAULT (LMAC_CONFIG_POOL_1_NUM - 4)
#if 0
struct sdio_status {
    int state;
    int event;
    uint32_t argument;
    uint32_t command;
    uint32_t rx_success;
    uint32_t tx_success;
    SYS_BUF *packet;
    SYS_BUF *head;
    SYS_BUF *tail;
    struct hif_hdr hh;
    int offset;
    uint32_t dma_addr;
    uint32_t dma_ctrl;
	int tx_que_count;
	int func1_ind_len;
} __packed;
#endif

struct sdio_priv {

	int continue_transfer;
	int report_threshold;

	int state;
	bool need_info_update;
	bool exist_request_to_host;
    bool is_set_func1_ind;
	bool restart_rx;
	bool restart_tx;
	uint32_t tx_remain_bytes;
	uint32_t rx_remain_bytes;
	uint32_t sdio_argument;
	uint32_t sdio_command;
	struct sysbuf_queue hwque[QUE_MAX];
	SYS_BUF *packet_curr;
	int offset;
	volatile struct adma_descriptor *adma_curr;
	struct sysbuf_queue *to_host_que;
	uint32_t credit[VIF_MAX];
	uint16_t rx_report;
    bool info_sync;
	int n_pre_txque;
	
} __packed;

static struct sdio_priv m_priv;
static void sdio_isr(int vector);
static void run_event(void *arg, int event);
static void transit(void *arg, int new_state);
static void sdio_show(void *arg);


//int continue_transfer = 0;
//int intttt = 0;

static const char *name()
{
	return "SDIO";
}

static uint32_t requested_length(uint32_t arg)
{
	uint32_t len;
	if (arg & 0x08000000) {
		len = 512 * (arg & 0x1FF);
	} else {
		len = arg & 0x1FF;
		if (!len) len = 512;
	}

	return len;
}
static void sdio_init(void *arg)
{
	V(TT_HIF, "%s Init\n", name());
	PIN_FUNC_SET(IO_MUX0_GPIO7, FUNC_GPIO7_SD_DATA0);
	PIN_FUNC_SET(IO_MUX0_GPIO8, FUNC_GPIO8_SD_DATA1);
	PIN_FUNC_SET(IO_MUX0_GPIO9, FUNC_GPIO9_SD_DATA2);
	PIN_FUNC_SET(IO_MUX0_GPIO10, FUNC_GPIO10_SD_DATA3);
	PIN_FUNC_SET(IO_MUX0_GPIO11, FUNC_GPIO11_SD_CLK);
	PIN_FUNC_SET(IO_MUX0_GPIO12, FUNC_GPIO12_SD_CMD);

	memset(&m_priv, 0, sizeof(m_priv));
    memset((void*)SDIO_DES_BASE_INFO, 0, 16);
    memset((void*)SDIO_DES_BASE_RX, 0, 48);
    memset((void*)SDIO_DES_BASE_TX, 0, 48);

	/* share the queue between host interface and host interface driver */
	struct host_interface_ops *ops = hal_sdio_8266_ops();
	m_priv.to_host_que = &ops->to_host_que;

	irq_isr_register(IRQ_VECTOR_SDIO, (void *)sdio_isr);
	
	irq_status_clean(IRQ_VECTOR_SDIO);
	irq_unmask(IRQ_VECTOR_SDIO);

	SDIO_ADMA_EN();	

}

static void sdio_deinit(void *arg)
{
	V(TT_HIF, "%s Deinit\n", name());
}

static void sdio_start(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);

	util_sysbuf_queue_init(&priv->hwque[HIF_SDIO_RX]);
	util_sysbuf_queue_init(&priv->hwque[HIF_SDIO_TX]);

	//drv_sdio_posedge_reset();
	priv->credit[0] = 0;
	priv->credit[1] = 0;
	priv->rx_report = RX_REPORT_DEFAULT;

	SDIO_MSG_SET_CREDIT_VIF0(priv->credit[0]);
	SDIO_MSG_SET_CREDIT_VIF1(priv->credit[1]);
	SDIO_MSG_SET_RX(priv->rx_report);
	SDIO_MSG_SET_TX(0, 0);

	priv->adma_curr = (volatile struct adma_descriptor*)(SDIO_DES_BASE_INFO);
	priv->adma_curr->addr =SDIO_MSG_BASE;
	priv->adma_curr->len = 16;
	priv->adma_curr->attri = SDIO_ADMA_ATTR_ACT_TRS | SDIO_ADMA_ATTR_VALID | SDIO_ADMA_ATTR_END;

	priv->state = STATE_IDLE;
	V(TT_HIF, "%s Start\n", name());
}

static void sdio_stop(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	V(TT_HIF, "%s Stop\n", name());
}

static void sdio_request_to_host(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	run_event(priv, EVENT_TX_REQUEST);
}

static void sdio_update_slot(void *arg, uint8_t value, bool inc)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	priv->rx_report += value;
}

static void sdio_update_credit(void *arg, uint8_t ac, uint8_t value, bool inc)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	unsigned long flags = system_irq_save();
	int vif_id = 0;
	//priv->need_info_update = true;
	uint32_t old_credit = 0;

	if (ac >= 0 &&  ac<=3) {
		vif_id = 0;
	} else if (ac >= 6 &&  ac<=9) {
		vif_id = 1;
		ac -= 6;
	} else {
		ASSERT(0);
	}
	
	old_credit = priv->credit[vif_id];
	uint8_t credit = (old_credit >> ac * 8) & 0xff;
	
	if (inc)
		credit += value;
	else
		credit -= value;
		
	uint32_t new_credit = old_credit & (~(0xff << ac * 8));
	new_credit |= credit << ac * 8;

	priv->credit[vif_id] = new_credit;
	priv->rx_report += value;


#if 1
	if ( priv->need_info_update != true ) {
		priv->report_threshold += value;
		if(priv->report_threshold > 50) {
			priv->need_info_update = true;
			priv->report_threshold = 0;
		}
	}
#endif	
	system_irq_restore(flags);
}

static struct host_interface_ops ops = {
	.support_stream_mode = false,
	.priv = &m_priv,
	.init = sdio_init,
	.deinit = sdio_deinit,
	.request_to_host = sdio_request_to_host,
	.update_credit = sdio_update_credit,
	.update_slot = sdio_update_slot,
	.start = sdio_start,
	.stop = sdio_stop,
	.show = sdio_show,
};

struct host_interface_ops *hal_sdio_8266_ops()
{
	return &ops;
}

static int tx_start(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);

	if (SDIO_TRANSFER_ADDR == SDIO_ADDR_INFO) {
        priv->info_sync = true;
		run_event(priv, EVENT_INFO_START);
    } else if (SDIO_TRANSFER_ADDR == SDIO_ADDR_INFO_ASYNC) {
        priv->info_sync = false;
		run_event(priv, EVENT_INFO_START);
	} else {
		run_event(priv, EVENT_TX_START);
	}

	SDIO_INT1_STATUS = BIT4;	
	return 0;
}

static int rx_start(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	run_event(priv, EVENT_RX_START);

	SDIO_INT1_STATUS = BIT3;
	return 0;
}

static int complete(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	SDIO_INT1_STATUS = BIT0;
	run_event(priv, EVENT_COMPLETE);
	return 0;
}

static int soft_reset(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	SDIO_INT1_STATUS = BIT12;
	drv_sdio_posedge_reset();
	V(TT_HIF, ">> SDIO soft reset\n");
	return 0;
}

static int func1_reset(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	SDIO_INT1_STATUS = BIT21;
	V(TT_HIF, ">> sdio function1 reset\n");
	return 0;
}

static int funcx_abort(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	SDIO_INT1_STATUS = BIT29;
	drv_sdio_posedge_reset();
	V(TT_HIF, ">> sdio function abort\n");
	return 0;
}

static int program_start(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	SDIO_INT1_STATUS = BIT25;
	SDIO_ADMA_EN();	
	
	V(TT_HIF, ">> sdio program start\n");
	return 0;
}

static int adma_error(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	V(TT_HIF, ">> sdio adma error\n");
    sdio_show(arg);
	return 0;
}

static int adma_int(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	
	V(TT_HIF, ">> sdio adma int\n");
	return 0;
}

static void sdio_isr(int vector)
{
#define CALL_ISRH(s,b,h)\
	do {\
		if(s&b) {\
			s &= ~b;\
			res = h(&m_priv);\
			if (res < 0)\
				goto error;\
		}\
	} while (0)

	int res = 0;

	unsigned long flags = system_irq_save();
	struct sdio_priv *priv = SDIO_PRIV(&m_priv);
	uint32_t int1_status = drv_sdio_get_int1_status();
	uint32_t int2_status = drv_sdio_get_int2_status();
	uint32_t int1_status_orig = int1_status;
	uint32_t int2_status_orig = int2_status;
	priv->sdio_command 		= SDIO_COMMAND;
	priv->sdio_argument 	= SDIO_ARGUMENT1;
	
	CALL_ISRH(int1_status, BIT12, soft_reset);
	CALL_ISRH(int1_status, BIT21, func1_reset);
	CALL_ISRH(int1_status, BIT29, funcx_abort);
	CALL_ISRH(int1_status, BIT25, program_start);
	CALL_ISRH(int2_status, BIT6, adma_error);
	CALL_ISRH(int1_status, BIT1, adma_int);
	CALL_ISRH(int1_status, BIT0, complete);
	CALL_ISRH(int1_status, BIT3, rx_start);
	CALL_ISRH(int1_status, BIT4, tx_start);

	if (int1_status || int2_status) {
		E(TT_HIF, "[%s %d] Unhandle: status1:0x%08X->0x%08X, status2:0x%08X->0x%08X\n", 
				__func__, __LINE__, 
				int1_status_orig, int1_status,
				int2_status_orig, int2_status);
	}
	system_irq_restore(flags);
	return;
error:
	E(TT_HIF, "[%s %d] Error: status1:0x%08X->0x%08X, status2:0x%08X->0x%08X\n", 
			__func__, __LINE__, 
			int1_status_orig, int1_status,
			int2_status_orig, int2_status);
	SDIO_INT_CLR(int1_status_orig);
	system_irq_restore(flags);
}

static void send_info_to_host(struct sdio_priv *priv)
{
	unsigned long flags = system_irq_save();
	//priv->rx_report = lmac_test_pool_get_remain(1);
	SDIO_MSG_SET_RX(priv->rx_report);
	SDIO_MSG_SET_CREDIT_VIF0(priv->credit[0]);
	SDIO_MSG_SET_CREDIT_VIF1(priv->credit[1]);
	
	drv_sdio_posedge_start(SDIO_DES_BASE_INFO);
	system_irq_restore(flags);
}

static void receive_from_host(struct sdio_priv *priv)
{
	uint32_t len;
	SYS_BUF *packet;
	unsigned long flags = system_irq_save();

	uint32_t offset = 0;
	const int actual_len = LMAC_CONFIG_BUFFER_SIZE - sizeof(SYS_HDR) - sizeof(LMAC_TXHDR);

	len = requested_length(priv->sdio_argument);
	if (!priv->restart_rx)
	{
		priv->rx_remain_bytes = len;
		#if 0
		packet = lmac_alloc_sys_buf_hif_len(1 , len-sizeof(struct hif_hdr));
		#else
		packet = sys_buf_alloc(HIF_SDIO_RX, len-sizeof(struct hif_hdr));
		#endif
		ASSERT(packet);
		priv->packet_curr = packet;
		util_sysbuf_queue_push(&priv->hwque[HIF_SDIO_RX], packet);
	} 
	else 
	{
		priv->rx_remain_bytes += len;
		packet = priv->packet_curr;
		offset = priv->offset;
	}
	
	if (!packet) {
		sdio_show(priv);
		ASSERT(0);
		system_irq_restore(flags);
		return;
	}
	
	SYS_BUF *itor = packet, *prev_itor;
	int i = 0;
	int n_remain = len;
	int curr_len;
	int curr_remain;
	while(itor)
	{
		priv->adma_curr = (volatile struct adma_descriptor *)(SDIO_DES_BASE_RX + i*8);
		priv->adma_curr->addr = ((uint32_t)itor->more_payload) + offset;
		curr_remain = lmac_bytes_to_bufend(itor, (void*)priv->adma_curr->addr);
		offset = 0;
		curr_len = MIN(curr_remain, n_remain);
		priv->adma_curr->len = curr_len;
		priv->adma_curr->attri = 0x21;
		priv->packet_curr = itor;
		priv->offset = priv->adma_curr->len==actual_len?0:priv->adma_curr->len;
		n_remain -= priv->adma_curr->len;
		prev_itor = itor;
		itor = SYS_BUF_LINK(itor);
		if (!itor && n_remain)
		{
			ASSERT(prev_itor);
			#if 0
			itor = lmac_alloc_sys_buf_hif_len(1 , n_remain);
			#else
			itor = sys_buf_alloc(HIF_SDIO_RX,n_remain);
			#endif
			ASSERT(itor);
			SYS_BUF_LINK(prev_itor) = itor;
		}
		i++;
	}

	ASSERT(!n_remain);
	priv->adma_curr->attri = 0x23;
	drv_sdio_posedge_start(SDIO_DES_BASE_RX);
	system_irq_restore(flags);
}

static void send_to_host(struct sdio_priv *priv)
{
	unsigned long flags = system_irq_save();
	uint32_t len;
	uint32_t offset = 0;
	SYS_BUF *packet;
	const int actual_len = LMAC_CONFIG_BUFFER_SIZE - sizeof(SYS_HDR);

	len = requested_length(priv->sdio_argument);
	if (!priv->restart_tx)
	{
		priv->tx_remain_bytes = len;
		packet = util_sysbuf_queue_pop(priv->to_host_que);
		ASSERT(packet);
		priv->packet_curr = packet;
		util_sysbuf_queue_push(&priv->hwque[HIF_SDIO_TX], packet);

		struct hif_hdr *hif = (struct hif_hdr*)&packet->lmac_rxhdr;
		SYS_BUF *packet_next = util_sysbuf_queue_peek(priv->to_host_que);
		if (packet_next)
		{
			struct hif_hdr *hif_next = (struct hif_hdr*)&packet_next->lmac_rxhdr;
			hif->tlv_len = hif_next->len + 8;
			priv->continue_transfer = hif->tlv_len;
		}
		else
		{
			hif->tlv_len = 0;
			priv->continue_transfer = 0;
		}

		//hif->flags = (priv->rx_report) & 0xFF;
		//priv->need_info_update = 0;
	} 
	else
	{
		priv->tx_remain_bytes += len;
		offset = priv->offset;
		packet = priv->packet_curr;
	}

	if (!packet) {
		sdio_show(priv);
		ASSERT(0);
		system_irq_restore(flags);
		return;
	}

	SYS_BUF *itor = packet;
	int i = 0;
	int n_remain = len;
	int curr_len;
	int curr_remain;
	while(itor && n_remain)
	{
		priv->adma_curr = (volatile struct adma_descriptor *)(SDIO_DES_BASE_TX + i*8);
		priv->adma_curr->addr = ((uint32_t)&itor->lmac_txhdr) + offset;
		curr_remain = lmac_bytes_to_bufend(itor, (void*)priv->adma_curr->addr);
		offset = 0;
		curr_len = MIN(curr_remain, n_remain);
		priv->adma_curr->len = curr_len;
		priv->adma_curr->attri = 0x21;
		priv->packet_curr = itor;
		priv->offset = curr_len==actual_len?0:curr_len;
		n_remain -= curr_len;
		itor = SYS_BUF_LINK(itor);
		i++;
	}

	ASSERT(!n_remain);
	priv->adma_curr->attri = 0x23;
	drv_sdio_posedge_start(SDIO_DES_BASE_TX);
	system_irq_restore(flags);
}

static char *event_str(int event)
{
	switch(event) {
		case EVENT_TX_START:
			return "E-TX    ";
		case EVENT_RX_START:
			return "E_RX    ";
		case EVENT_INFO_START: 
			return "E-INFO  ";
		case EVENT_COMPLETE:
			return "E-COMP  ";
			break;
		case EVENT_TX_REQUEST:
			return "E-TX-REQ";
		default:
			return "E-UNDEFINED";
	}
}

static char *state_str(int state)
{
	switch(state) {
		case STATE_IDLE:
			return "S-IDLE";
		case STATE_TX:
			return "S-TX  ";
		case STATE_RX:
			return "S-RX  ";
		case STATE_INFO: 
			return "S-INFO";
		default:
			return "S-UNDEFINED";
	}
}

static void print_priv(struct sdio_priv *priv)
{
#define PRINT_PRIV_DEC(x) \
	system_printf("%s: %d\n", #x, priv->x);
#define PRINT_PRIV_HEX(x) \
	system_printf("%s: 0x%08X\n", #x, priv->x);

	system_printf("priv: %08X\n", priv);
    system_printf("%s\n", state_str(priv->state));
	PRINT_PRIV_DEC(need_info_update);
	PRINT_PRIV_DEC(exist_request_to_host);
	PRINT_PRIV_DEC(restart_rx);
	PRINT_PRIV_DEC(restart_tx);
	PRINT_PRIV_DEC(tx_remain_bytes);
	PRINT_PRIV_DEC(rx_remain_bytes);
	PRINT_PRIV_HEX(sdio_argument);
	PRINT_PRIV_HEX(sdio_command);
	PRINT_PRIV_DEC(rx_report);
	PRINT_PRIV_HEX(credit[0]);
	PRINT_PRIV_HEX(credit[1]);
	PRINT_PRIV_HEX(is_set_func1_ind);
	PRINT_PRIV_DEC(offset);
	system_printf("credit0: %08X\n",	priv->credit[0]);
}

static int32_t wq_tx_packet_post(void *param)
{
	SYS_BUF *packet = (SYS_BUF*) param;
	//ASSERT(SYS_HDR(packet).m_ref_count > 0);
	discard(packet);
	return 0;
}

#define INC_INDEX(a,m) \
    a++;\
    if (a == m) {\
       a = 0;\
    } 

#define DEC_INDEX(a,m) \
    if (a == 0) {\
       a = m-1;\
    }  else {\
        a--;\
    }


static void run_event(void *arg, int event)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	unsigned long flags = system_irq_save();
	int s = priv->state;
	
	switch(event)
	{
		case EVENT_TX_START:
			if (s == STATE_IDLE)
			{
				transit(arg, STATE_TX);
			}
			else if (s == STATE_TX)
			{
				priv->restart_tx = true;
				transit(arg, STATE_TX);
			}
			else
			{
				goto error;
			}
			break;
		case EVENT_RX_START:
			if (s == STATE_IDLE)
			{
				transit(arg, STATE_RX);
				SYS_BUF *packet = util_sysbuf_queue_peek(&priv->hwque[HIF_SDIO_RX]);
				ASSERT(packet);
			}
			else if (s == STATE_RX)
			{
				priv->restart_rx = true;
				transit(arg, STATE_RX);
				SYS_BUF *packet = util_sysbuf_queue_peek(&priv->hwque[HIF_SDIO_RX]);
				ASSERT(packet);
			} 
			else 
			{
				goto error;
			}
			break;

		case EVENT_INFO_START: 
			if (s == STATE_IDLE)
			{
				transit(arg, STATE_INFO);
			} 
			else
			{
				transit(arg, STATE_INFO);
			}
			break;

		case EVENT_COMPLETE:
			if (s == STATE_INFO)
			{
				if (priv->info_sync)
				priv->is_set_func1_ind = false;
				transit(arg, STATE_IDLE);
			}
			else if (s == STATE_TX)
			{
				priv->is_set_func1_ind = false;
				if (util_sysbuf_queue_count(&priv->hwque[HIF_SDIO_TX]) > 0)
				{
					SYS_BUF *packet = util_sysbuf_queue_peek(&priv->hwque[HIF_SDIO_TX]);
					//ASSERT(SYS_HDR(packet).m_ref_count > 0);
					struct hif_hdr *hif = (struct hif_hdr*)&packet->lmac_rxhdr;
					if ((hif->len + 8) == priv->tx_remain_bytes)
					{
						priv->tx_remain_bytes = 0;
						priv->restart_tx = false;
						priv->offset = 0;
						//priv->n_pre_txque--;
						SYS_BUF *packet = util_sysbuf_queue_pop(&priv->hwque[HIF_SDIO_TX]);
						// ASSERT(SYS_HDR(packet).m_ref_count > 0);
						discard(packet);
						transit(arg, STATE_IDLE);
					} 
					else
					{
						if (priv->restart_tx)
						{
							goto error;
						}
						priv->restart_tx = true;
					}
				} 
				else
				{
					goto error;
				}
			} 
			else if (s == STATE_RX)
			{
				if (util_sysbuf_queue_count(&priv->hwque[HIF_SDIO_RX]) > 0)
				{
					SYS_BUF *packet = util_sysbuf_queue_peek(&priv->hwque[HIF_SDIO_RX]);
					drv_sdio_posedge_write_done();
					SDIO_DMA1_CTL  = 0x0;
					if ((HIF_HDR(packet).len + 8) == priv->rx_remain_bytes)
					{
						priv->restart_rx = false;
						priv->offset = 0;
						struct hif_hdr *hh = (struct hif_hdr*)&(packet->hif_hdr);
						SYS_BUF *packet = util_sysbuf_queue_pop(&priv->hwque[HIF_SDIO_RX]);
						//ASSERT(SYS_HDR(packet).m_ref_count > 0);
#if 0
						lmac_sys_buf_len_calc_using_hif_len(packet);
#else
						sys_buf_len_calc_using_hif_len(packet);
#endif
						hal_host_interface_rx_done(packet);
						priv->rx_remain_bytes = 0;
						transit(arg, STATE_IDLE);
					} 
					else
					{
						if (priv->restart_tx)
						{
							goto error;
						}
						priv->restart_rx = true;
					}
				} 
				else
				{
					goto error;
				}
			}
			else 
			{
				transit(arg, STATE_IDLE);
			}
			break;

		case EVENT_TX_REQUEST:
			if (s == STATE_IDLE)
			{
				priv->exist_request_to_host = true;
				transit(arg, STATE_IDLE);
			}
			else
			{
				priv->exist_request_to_host = true;
			}
			break;

		default:
			goto error;
	}

	system_irq_restore(flags);
	return;

error:
	E(TT_HIF, "[%s %d] Error: event:%s, state:%s\n", 
			__func__, __LINE__, event_str(event), state_str(s));
	sdio_show(priv);
	system_irq_restore(flags);
}

static void transit(void *arg, int new_state)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
	unsigned long flags = system_irq_save();
	int os = priv->state;
	int ns = new_state;

	switch(ns)
	{
		case STATE_IDLE:
			if (priv->continue_transfer)
			{
				//do nothing ...
			} 
			else if ((util_sysbuf_queue_count(priv->to_host_que) > 0 && !priv->is_set_func1_ind))
			{
				priv->exist_request_to_host = false;
				//int n_txque = util_sysbuf_queue_count(priv->to_host_que);
				//if (n_txque > 0 && n_txque != priv->n_pre_txque && !priv->is_set_func1_ind)
				{
					SYS_BUF *packet = util_sysbuf_queue_peek(priv->to_host_que);
					struct hif_hdr *hif = (struct hif_hdr*)&packet->lmac_rxhdr;
					int hif_len = hif->len;
					while(drv_sdio_posedge_get_func1_control());
					priv->is_set_func1_ind = true;

					SDIO_FUN1_IND(hif_len + 8);

					
					//priv->n_pre_txque = util_sysbuf_queue_count(priv->to_host_que);
				}
			} 
			else if (priv->need_info_update && !priv->is_set_func1_ind)
			{
				priv->need_info_update = false;
				while(drv_sdio_posedge_get_func1_control());
				SDIO_FUN1_IND(SDIO_MSG_LEN);
				priv->is_set_func1_ind = true;
			}
			priv->state = ns;
			break;

		case STATE_TX:
			send_to_host(priv);
			priv->state = ns;
			break;
		case STATE_RX:
			receive_from_host(priv);
			priv->state = ns;
			break;
		case STATE_INFO: 
            priv->need_info_update = false;
            send_info_to_host(priv);
			priv->state = ns;
			break;
		default:
			goto error;
	}
	system_irq_restore(flags);
	return;
error:
	E(TT_HIF, "[%s %d] Error: os:%d, ns:%d\n", 
			__func__, __LINE__, os, ns);
	print_priv(priv);
	system_irq_restore(flags);
}

static void sdio_show(void *arg)
{
	struct sdio_priv *priv = SDIO_PRIV(arg);
//    print_adma_descriptor_all();
//    print_priv(priv);
//	lmac_test_pool_print_all();
}

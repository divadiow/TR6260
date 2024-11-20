#include "system.h"
#include "system_memory_manager.h"

#define SYS_MEM_SECTION __attribute__((section(".buffer")))

SYS_MEM_SECTION struct {
	uint8_t pool0[ LMAC_CONFIG_POOL_0_NUM * LMAC_CONFIG_BUFFER_SIZE ];
	uint8_t pool1[ LMAC_CONFIG_POOL_1_NUM * LMAC_CONFIG_BUFFER_SIZE ];
} system_memory_pool;

system_memory_info_t* sys_mem_info[SYSTEM_MEMORY_POOL_MAX];
static bool (*pool_free_hook)(SYS_BUF *packet) = NULL;

void *system_memory_base_address(void)
{
	return &system_memory_pool;
}

uint8_t* system_memory_pool_base_addr(uint8_t pool_id)
{
	uint8_t* addr = NULL;
	if(pool_id == SYSTEM_MEMORY_POOL_RX){
		addr=system_memory_pool.pool0;
	}else if(pool_id == SYSTEM_MEMORY_POOL_TX){
		addr=system_memory_pool.pool1;
	}else{
		system_printf("Invalid pool_id :%d\n",pool_id);
	}
	return addr;
}

uint16_t system_memory_pool_buf_size(uint8_t pool_id)
{
	system_memory_info_t* mem_info = sys_mem_info[pool_id];
	return mem_info->size;
}

uint16_t system_memory_pool_avail_number(uint8_t pool_id)
{
	system_memory_info_t* mem_info = sys_mem_info[pool_id];
	unsigned long flags = system_irq_save();
	uint8_t res = mem_info->avail;
	system_irq_restore(flags);
	return res;
}

system_memory_info_t* system_memory_pool_create(uint8_t pool_id , uint16_t size , uint16_t num )
{
	uint8_t i;
	uint8_t *buf_addr                   = system_memory_pool_base_addr(pool_id);
	system_memory_info_t* mem_info;

	mem_info = (system_memory_info_t*)pvPortMalloc(sizeof(system_memory_info_t));

	if(!mem_info){
		system_printf("mem_info allocation fail!! pool_id = %d, size = %d , num = %d\n" , pool_id , size , num );
		return NULL;
	}
	memset(buf_addr, 0, size*num);

	mem_info->pool_id = pool_id;
	mem_info->size = size;
	mem_info->num = num;
	mem_info->avail = num;
	mem_info->sys_buf_pool = (SYS_BUF*)buf_addr;

	for(i = 1 ; i < num ; i ++)
	{
		buf_addr                        += size;
		SYS_BUF_NEXT( mem_info->sys_buf_pool )     = (SYS_BUF*)buf_addr;
		SYS_BUF_LINK( mem_info->sys_buf_pool )     = 0;
		mem_info->sys_buf_pool            = (SYS_BUF*)buf_addr;
		mem_info->sys_buf_pool->sys_hdr.pool_id = pool_id;
	}

	SYS_BUF_NEXT( mem_info->sys_buf_pool )         = 0;
	mem_info->sys_buf_pool                = (SYS_BUF*)system_memory_pool_base_addr(pool_id);
	mem_info->sys_buf_pool->sys_hdr.pool_id = pool_id;

	return mem_info;
}

void system_memory_pool_init(void)
{
	sys_mem_info[SYSTEM_MEMORY_POOL_RX] = system_memory_pool_create( SYSTEM_MEMORY_POOL_RX ,
				LMAC_CONFIG_BUFFER_SIZE , LMAC_CONFIG_POOL_0_NUM );
	sys_mem_info[SYSTEM_MEMORY_POOL_TX] = system_memory_pool_create( SYSTEM_MEMORY_POOL_TX ,
				LMAC_CONFIG_BUFFER_SIZE , LMAC_CONFIG_POOL_1_NUM );
	system_memory_pool_print( SYSTEM_MEMORY_POOL_RX);
	system_memory_pool_print( SYSTEM_MEMORY_POOL_TX);
}

SYS_BUF* system_memory_pool_alloc(uint8_t pool_id )
{
	unsigned long flags = system_irq_save();
	system_memory_info_t* mem_info = sys_mem_info[pool_id];
	SYS_BUF* ret = mem_info->sys_buf_pool;
	static unsigned long g_alloc_log_ctrl = 0;

	if(ret != NULL)
	{
		#ifdef _USE_PSM
        if(psmCtx.SysPsmState == SYS_PM_STATE_MODEM_SLEEP){
                //g_lmac_ps_info.state = PS_STATE_DOZE_TO_ACTIVE;
                TrPsmModemSleepRestore();
        }
		#endif
		mem_info->sys_buf_pool = SYS_BUF_NEXT(mem_info->sys_buf_pool);
		SYS_BUF_NEXT(ret) = 0;
		SYS_BUF_LINK(ret) = 0;
		SYS_HDR(ret).m_cb = 0;
		SYS_HDR(ret).m_hooked 	 = 0;
		mem_info->avail--;
	} else if(mem_info->avail ) {
		system_printf("%s: pool_id = %d , remain = %d but buf was 0 , now buf = 0%x\n"
			, __func__ , pool_id ,mem_info->avail ,  mem_info->sys_buf_pool );
		system_memory_pool_print_detail(pool_id);
	}
	#if defined (_USR_TR6260) || defined (_USR_TR6260S1) || defined (_USR_TR6260_3)
    else
    {	
    	g_alloc_log_ctrl ++;
    	if(0 == (g_alloc_log_ctrl%100000))
    	{
        	system_printf("alloc buff fail  mem_info->avail=%d pool_id = %d\n",mem_info->avail,pool_id);
		}
	}
    #endif
	system_irq_restore(flags);
	return ret;
}

void system_memory_pool_register_free_hook(bool(*free_hook)(SYS_BUF *packet))
{
	unsigned long flags = system_irq_save();
	pool_free_hook = free_hook;
	system_irq_restore(flags);
}

// free all the SYS_BUFs linked by m_link;
void system_memory_pool_free(SYS_BUF* buf)
{
	unsigned long flags = system_irq_save();
	uint8_t pool_id = buf->sys_hdr.pool_id;
	system_memory_info_t* mem_info = sys_mem_info[pool_id];
	ASSERT(buf != NULL);
#if 0
	if(SYS_HDR(buf).m_ref_count == 0)
	{
		system_printf("system_memory_pool_free : 0x%08X Buffer is freeing which is already freed.\n", buf);
		system_memory_pool_print_detail(pool_id);
		system_irq_restore(flags);
		return;
	}
#endif
	if (pool_free_hook && SYS_HDR(buf).m_hooked) {
		pool_free_hook(buf);
		system_irq_restore(flags);
		return;
	}

	if((SYS_HDR(buf).m_cb) && (SYS_HDR(buf).m_cb(buf))) {
		I(TT_QM, "[%s] Beacon_callback exists : 0x%p (%d)\n",
			__func__, SYS_HDR(buf).m_cb, SYS_HDR(buf).m_cb(buf));
		SYS_HDR(buf).m_cb = 0;
		system_irq_restore(flags);
		return;
	}

	if(pool_id >= SYSTEM_MEMORY_POOL_MAX) {
		I(TT_QM, "[%s] Wrong pool id exists : %d\n",
			__func__, pool_id);
		system_irq_restore(flags);
		return;
	}

	SYS_BUF *link = SYS_BUF_LINK(buf);
    ASSERT(link != buf);
    if(link == buf) link = 0;
	while(link != 0) {
		SYS_BUF_NEXT(link) = mem_info->sys_buf_pool;
		mem_info->sys_buf_pool = link;
		link = SYS_BUF_LINK(link);
		SYS_BUF_LINK(mem_info->sys_buf_pool) = 0;
		mem_info->avail++;
#if 0
        ASSERT( mem_info->avail <= mem_info->num );
        if(mem_info->avail >  mem_info->num  )
		{
			system_printf("1. system_memory_pool_available[%d] = %d\n" , pool_id , mem_info->avail );
			system_memory_pool_print_detail(pool_id);
		}
#endif
	}
	SYS_BUF_NEXT(buf) = mem_info->sys_buf_pool;
	SYS_BUF_LINK(buf) = 0;
	mem_info->sys_buf_pool = buf;
	mem_info->avail ++;
	ASSERT( mem_info->sys_buf_pool != 0);
#if 0
    ASSERT( mem_info->avail <=  mem_info->num  );
    if(mem_info->avail >  mem_info->num  )
	{
	  system_printf("2. system_memory_pool_available[%d] = %d\n" , pool_id , mem_info->avail );
	  system_memory_pool_print_detail(pool_id);
	}
#endif
	system_irq_restore(flags);
}

int system_memory_pool_number_of_link(struct _SYS_BUF* buf)
{
	unsigned long flags = system_irq_save();
	int n = 0;
	struct _SYS_BUF *link = buf;
	while(link != 0) {
		n++;
		link = SYS_BUF_LINK(link);
	}
	system_irq_restore(flags);

	return n;
}

void system_memory_pool_print(uint8_t pool_id)
{
	unsigned long flags = system_irq_save();
	uint8_t num = 0;
	system_memory_info_t* mem_info = sys_mem_info[pool_id];
	SYS_BUF* iter = mem_info->sys_buf_pool;
	while(iter != (SYS_BUF*)0) {
		//system_printf("0x%08X\n", iter);
		iter = SYS_BUF_NEXT(iter);
		num ++;
	}
	system_irq_restore(flags);
	/*system_printf("total num = %d ,  buffersize = %d, remain = %d pool_id=%d\n" , num ,
		mem_info->size ,mem_info->avail,pool_id);*/
}

void system_memory_pool_print_detail(uint8_t pool_id)
{
	unsigned long flags = system_irq_save();
	system_memory_info_t* mem_info = sys_mem_info[pool_id];
	int num = mem_info->num;
	int i = 0;
	uint8_t *base = (uint8_t*)system_memory_pool_base_addr(pool_id);
	system_printf("pool_id = %d\n",pool_id);
	for(i = 0; i < num; i++) {
		SYS_HDR *itor = (SYS_HDR*) (base + LMAC_CONFIG_BUFFER_SIZE*i);
		system_printf("[%2d] 0x%08X, next:%08X, link:%08X ref:%2d type:%2d\n",
				i, itor, itor->m_next, itor->m_link, itor->m_ref_count, itor->m_type);
	}
	system_irq_restore(flags);
}

void system_memory_pool_print_all(void)
{
	int i=0;
	for(i=0; i<SYSTEM_MEMORY_POOL_MAX; i++)
		system_memory_pool_print_detail(i);
}
/********************************************************************************************************
	Tx packet format (ex. fragmented into two buffer)
	>> 1st buffer
	|<---------------------------------- system_memory_pool_buf_size ---------------------------------->|
	|<-- SYS_BUF_TOTAL_TXHDR_SIZE --->|<------------------- SYS_BUF_LENGTH(buf) ----------------------->|
	|                                 |                                                                 |
	+-------------+-------------------+-----------------+--------------+--------------------------------+
	|   SYS_HDR   |    LMAC_TXHDR     |HIF_HDR|FRAME_HDR|  MAC header  |           Payload              |
	+-------------+-------------------+-----------------+--------------+--------------------------------+
	                                  |    TXVECTOR     |
                                      +-----------------+
    >> 2nd buffer
	|<---------------------------------- system_memory_pool_buf_size ---------------------------------->|
	|<-- SYS_BUF_TOTAL_TXHDR_SIZE --->|<------------- SYS_BUF_LENGTH(buf) ------------->|
	|                                 |                                                 |
	+-------------+-------------------+-------------------------------------------------+---------------+
	|   SYS_HDR   |    LMAC_TXHDR     |                     Payload                     |     empty     |
	+-------------+-------------------+-------------------------------------------------+---------------+

*********************************************************************************************************/

SYS_BUF *sys_buf_alloc(uint8_t pool_id , uint16_t hif_len ) // HIF Length
{
	uint16_t buf_left, take_len;
	SYS_BUF* packet = 0 , *link = 0;
	SYS_BUF* iter;

	// allocate First buffer
	packet = system_memory_pool_alloc(pool_id);
	if(packet == 0) {
		//system_memory_pool_print(pool_id);
		//system_memory_pool_print(0);
		//system_memory_pool_print(1);
		return 0;
	}

	unsigned long flags = system_irq_save();

	// First buffer
	buf_left = system_memory_pool_buf_size(pool_id) - sizeof(SYS_HDR) - sizeof(LMAC_TXHDR) - sizeof(HIF_HDR);
	take_len = min( buf_left , hif_len );
	hif_len  -= take_len;
	SYS_BUF_LENGTH(packet) = sizeof(LMAC_TXHDR) + sizeof(HIF_HDR) + take_len;

	iter = packet;
	while(hif_len > 0)
	{
		link = system_memory_pool_alloc(pool_id);
		if(link == 0)
		{
			system_memory_pool_free(packet);
			system_irq_restore(flags);
			return 0;
		}
		SYS_BUF_LINK(iter)      = link;
		buf_left = system_memory_pool_buf_size(pool_id) - sizeof(SYS_HDR) - sizeof(LMAC_TXHDR);
		take_len = min(buf_left , hif_len);
		SYS_BUF_LENGTH(link) = sizeof(LMAC_TXHDR) + take_len ;
		hif_len -= take_len;
		iter = link;
	}
	system_irq_restore(flags);
	return packet;
}

void sys_buf_len_calc_using_hif_len(SYS_BUF* packet)
{
	uint16_t buf_left, take_len;
	SYS_BUF *link = 0;
	SYS_BUF *iter;
	struct hif_hdr *hh = (struct hif_hdr*)&(packet->hif_hdr);
	uint16_t hif_len = hh->len + sizeof(*hh);
	int pool_id = packet->sys_hdr.pool_id ;

	unsigned long flags = system_irq_save();
	// First buffer
	buf_left = system_memory_pool_buf_size(pool_id) - sizeof(SYS_HDR) - sizeof(LMAC_TXHDR);
	take_len = min( buf_left , hif_len );
	hif_len  -= take_len;
	SYS_BUF_LENGTH(packet) = sizeof(LMAC_TXHDR) + take_len;

	iter = packet;
	while(hif_len > 0)
	{
		link = SYS_BUF_LINK(iter);
		if(link == 0)
		{
			system_irq_restore(flags);
			return;
		}
		buf_left                = system_memory_pool_buf_size(pool_id) - sizeof(SYS_HDR) - sizeof(LMAC_TXHDR);
		take_len                = min(buf_left , hif_len);
		SYS_BUF_LENGTH(link)    = sizeof(LMAC_TXHDR) + take_len ;
		hif_len                 -= take_len;
		iter                    = link;
	}
	system_irq_restore(flags);
}

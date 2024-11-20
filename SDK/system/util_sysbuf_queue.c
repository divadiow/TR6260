#include "util_sysbuf_queue.h"

void util_sysbuf_queue_init(struct sysbuf_queue *hque)
{
	hque->count = 0;
	hque->head = NULL;
	hque->tail = NULL;
}

void util_sysbuf_queue_push(struct sysbuf_queue *hque, SYS_BUF *sysbuf)
{
	unsigned long flags = system_irq_save();
	ASSERT(hque);
	ASSERT(sysbuf);
	//ASSERT(SYS_HDR(sysbuf).m_ref_count > 0);
    
	if (hque->head) {
		SYS_BUF_NEXT(hque->tail) = sysbuf;
	} else {
		hque->head = sysbuf;
	}

	hque->tail = sysbuf;
	SYS_BUF_NEXT(hque->tail) = NULL;
	hque->count++;
	system_irq_restore(flags);
}

SYS_BUF *util_sysbuf_queue_pop(struct sysbuf_queue *hque)
{
	unsigned long flags = system_irq_save();
	ASSERT(hque);
	SYS_BUF *res = NULL;
	if (hque->count) {
		res = hque->head;
		ASSERT(res);
		hque->head = SYS_BUF_NEXT(res);
       // ASSERT(SYS_HDR(res).m_ref_count > 0);
		SYS_BUF_NEXT(res) = NULL;
		hque->count--;
	}

	system_irq_restore(flags);
	return res;
}

SYS_BUF *util_sysbuf_queue_peek(struct sysbuf_queue *hque)
{
	unsigned long flags = system_irq_save();
	ASSERT(hque);
	SYS_BUF *res = NULL;

	if (hque->count) {
		res = hque->head;
		ASSERT(res);
      //  ASSERT(SYS_HDR(res).m_ref_count > 0);
	}

	system_irq_restore(flags);
	return res;
}

uint32_t util_sysbuf_queue_count(struct sysbuf_queue *hque)
{
	unsigned long flags = system_irq_save();
	ASSERT(hque);
    uint32_t count = hque->count;
	system_irq_restore(flags);

	return count;
}

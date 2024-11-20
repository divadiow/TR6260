#ifndef SYSTEM_MEMORY_MANAGER_H
#define SYSTEM_MEMORY_MANAGER_H

enum system_memory_type {
	SYSTEM_MEMORY_POOL_RX,
	SYSTEM_MEMORY_POOL_TX,
	SYSTEM_MEMORY_POOL_MAX
};

/** System Memory information */
typedef struct system_memory_info {
	uint8_t pool_id;		// pool id
	uint16_t size;		// buffer element size
	uint16_t num;		// number of total buffers
	uint16_t avail;		// number of available buffers
	SYS_BUF* sys_buf_pool;	// sys_buf start address
}system_memory_info_t;

void system_memory_pool_init(void);
struct _SYS_BUF* system_memory_pool_alloc(uint8_t);
void system_memory_pool_free(struct _SYS_BUF* buf);
void system_memory_pool_print        (uint8_t);
int system_memory_pool_number_of_link(struct _SYS_BUF* buf);
void system_memory_pool_print_all(void);
void system_memory_pool_print_detail(uint8_t pool_id);
void system_memory_pool_register_free_hook(bool(*free_hook)(SYS_BUF *packet));
uint16_t system_memory_pool_avail_number(uint8_t pool_id);
void *system_memory_base_address(void);
uint8_t* system_memory_pool_base_addr(uint8_t pool_id);
uint16_t system_memory_pool_buf_size(uint8_t pool_id);

struct _SYS_BUF* sys_buf_alloc(uint8_t pool_id , uint16_t hif_len);
void sys_buf_len_calc_using_hif_len(struct _SYS_BUF *packet);

#endif //SYSTEM_MEMORY_MANAGER_H

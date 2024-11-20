/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:    
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       V1.0
 * Author:        lixiao
 * Date:          2019-01-30
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

/****************************************************************************
* 	                                           Include files
****************************************************************************/
#include "util_cmd.h"
#if 0
#include "system.h"
#include "util_cmd.h"
#include "system_wifi.h"
#include "system_network.h"
#include "system_debug.h"

/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define DUMP_PKT_FILTER_RULE_MAX            (4)

/****************************************************************************
* 	                                           Local Types
****************************************************************************/
typedef struct {
	uint8_t valid;
	uint8_t byte_index;
	uint8_t value;
} dump_pkt_filter_t;

typedef struct {
	uint8_t enable;
    uint8_t dir;
	dump_pkt_filter_t filter[DUMP_PKT_FILTER_RULE_MAX];
} dump_pkt_cfg_t;

typedef struct {
	uint8_t enable;
    uint8_t dir;
    uint8_t type;
    uint16_t stype;
} dump_frame_cfg_t;

static dump_frame_cfg_t dump_frame_cfg = {0};
static dump_pkt_cfg_t   dump_pkt_cfg = {0};
#endif

/****************************************************************************
* 	                                           Local Constants
****************************************************************************/

/****************************************************************************
* 	                                           Local Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Global Constants
****************************************************************************/

/****************************************************************************
* 	                                          Global Variables
****************************************************************************/

/****************************************************************************
* 	                                          Global Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/
#if defined(HEAP_MEMORY_TRACE) || defined(CPU_RUNTIME_TRACE)
#define MAX_TASK_NUM 32

typedef struct {
	char * taskname;
#ifdef HEAP_MEMORY_TRACE	
	int current_heap;
	int maxheap;
#endif	
#ifdef 	CPU_RUNTIME_TRACE
	unsigned long long runtime; 
#endif
} stats_task_t;


stats_task_t taskstats[MAX_TASK_NUM]={{"main"}};	


#define VECTOR_BASE 0x00010000
#define EDLM_BASE	0x00200000
#define FLASH_BASE  0x00c00000

unsigned char GetTaskID(char * taskname)
{
	unsigned char i=0;
	int imem=((int)taskname)&0xFFFFFF;
	int smem=0;
	for (i=0;i<MAX_TASK_NUM;i++)
	{
		smem=((int)taskstats[i].taskname)&0xFFFFFF;
		if(imem<EDLM_BASE||imem>FLASH_BASE)
		{
			if((smem<=EDLM_BASE||smem>FLASH_BASE)&&taskname==taskstats[i].taskname)
			{
				return i;
			}
			else if(NULL == taskstats[i].taskname)
			{
				taskstats[i].taskname=taskname;
				return i;
			}
		}
		else if(imem>=EDLM_BASE&&imem<=FLASH_BASE)
		{
			if(NULL == taskstats[i].taskname)
			{
				taskstats[i].taskname=os_malloc(strlen(taskname)+1);
				memcpy(taskstats[i].taskname,taskname,strlen(taskname));
				taskstats[i].taskname[strlen(taskname)]='\0';
				return i;
			}
			else if((smem>=EDLM_BASE&&smem<=FLASH_BASE)&&0==strcmp(taskname,taskstats[i].taskname))
			{
				return i;
			}
			
		}
	}
	configASSERT(0);
	return -1;
}


char *GetTaskName(unsigned char taskid)
{
	return taskstats[taskid].taskname;
}

#ifdef HEAP_MEMORY_TRACE
void heap_status(unsigned char taskID, size_t blocksize,int is_malloc)
{

	if(is_malloc)
	{
		taskstats[taskID].current_heap+=blocksize;
		if(taskstats[taskID].current_heap>taskstats[taskID].maxheap)
		{
			taskstats[taskID].maxheap=taskstats[taskID].current_heap;
		}
	}
	else
	{
		taskstats[taskID].current_heap-=blocksize;
	}

	
}
extern int print_heapblock(char *taskname);

int print_heap(struct nrc_cmd *dummy, int argc, char *argv[])
{
		char * taskname=NULL;
		int i;
		if(argc>1)
		{
			taskname=argv[1];
		}

		vTaskSuspendAll();
		
		for(i=0;i<MAX_TASK_NUM;i++)
		{
			if(taskstats[i].taskname &&(!taskname || strstr(taskstats[i].taskname,taskname)))
			{
				
				system_printf("%s:\thistory max malloc %d, current malloc %d\n",taskstats[i].taskname,taskstats[i].maxheap,taskstats[i].current_heap);
				if(taskname)
				{
					break;
				}
			}
				
		}
		
		print_heapblock(taskname);
		( void ) xTaskResumeAll();
		
		return 0;
}
CMD(heaptrace,
	print_heap,
	"heap meory trace",
	"heap");
#endif

/*
unsigned char GetTaskID(char * taskname)
{
	unsigned char i=0;

	for (i=0;i<MAX_TASK_NUM;i++)
	{
		if(taskname==taskstats[i].taskname)
		{
			return i;
		}
		else if(NULL == taskstats[i].taskname)
		{
			taskstats[i].taskname=taskname;
			return i;
		}
	}
	configASSERT(0);
	return -1;

}*/




#ifdef CPU_RUNTIME_TRACE
#include "pit.h"
uint32_t switch_in_time;
unsigned long long total_runtime;	

void runtime_init()
{
	int i=0;
	
	switch_in_time=pit_ch_count_get(PIT_BASE1, 3);
	total_runtime=0;
	
	for (i=0;i<MAX_TASK_NUM;i++)
	{
		taskstats[i].runtime=0;
		
	}
}

void runtime_status(unsigned char taskID)
{
	uint32_t tsf=pit_ch_count_get(PIT_BASE1, 3);;
	uint32_t runtime;
	if(tsf>switch_in_time)
		runtime=switch_in_time+(0xFFFFFFFF-tsf);
	else
		runtime=switch_in_time-tsf;
	taskstats[taskID].runtime+=runtime;
	total_runtime+=runtime;
	switch_in_time=tsf;
}


int cpu_print(struct nrc_cmd *dummy, int argc, char *argv[])
{
	int i;
	long long time=0;
	vTaskSuspendAll();
	
	for (i=1;i<MAX_TASK_NUM;i++)
	{
		if(NULL!= taskstats[i].taskname)
		{

			system_printf("cpucpu:%s:%lld\n",taskstats[i].taskname,taskstats[i].runtime);
			
			continue;
		}
		break;
	}
	
	system_printf("cpucpu:total runtime:%lld\n",total_runtime,time);
	xTaskResumeAll();

	return 0;
}



CMD(cpu,
	cpu_print,
	"cpu  percentage",
	"cpu ");

#endif
#endif

extern uint32_t heapcheck_in_taskswitch;
extern uint32_t g_flashdump_enable;
static int cmd_debugset(cmd_tbl_t *t, int argc, char *argv[])
{
	int ret;
	char * key;
	//char val[256] = {0};

	if(argc != 3)
	{
		system_printf("DEBUG SET ARG NUM(: %d) IS ERROR!!\n", argc);
		return CMD_RET_FAILURE;
	}

	key = argv[1];

	if(0==strcmp(key,"heapcheck"))
	{
		heapcheck_in_taskswitch=atoi(argv[2]);	
	}
	else if(0==strcmp(key,"flashdump"))
	{
		g_flashdump_enable=atoi(argv[2]);
	}
	else if(0==strcmp(key,"show"))
	{
		system_printf("heapcheck %d;flashdump %d\n", heapcheck_in_taskswitch,g_flashdump_enable);
	}

	return CMD_RET_SUCCESS;
}

CMD(debugset, cmd_debugset,  "setting debug value",  "debugset <key> <value>");
#if 0
void dump_pkt_help(void)
{
	system_printf("\tusage:\ndump_pkt [enable/disable]\ndump_pkt filter [index] [byte] [value]\n\n");
    system_printf("\tusage:\ndump_frame [enable/disable]\ndump_frame type|stype value\n\n");
	return;
}

void show_dump_pkt_filter(void)
{
	int i;
    dump_pkt_filter_t *filter;

	system_printf("----pkt dump %s|dir %d.\n", dump_pkt_cfg.enable ? "enable" : "disable",
        dump_pkt_cfg.dir);
	for (i = 0; i < DUMP_PKT_FILTER_RULE_MAX; ++i) {
        filter = &dump_pkt_cfg.filter[i];
		if (!filter->valid)
			continue;
		system_printf("rule[%d]: byte[%d], value[0x%02x]\n", i, filter->byte_index,
			filter->value);
	}
}

void set_dump_pkt_filter(unsigned char index, unsigned char byte, unsigned char value)
{
    dump_pkt_filter_t *filter;
    
	if (index >= DUMP_PKT_FILTER_RULE_MAX)
		return dump_pkt_help();

    filter = &dump_pkt_cfg.filter[index];
	filter->valid = 1;
	filter->byte_index = byte;
	filter->value = value;
}

void dump_pkt_config(int argc, char **argv)
{
	unsigned char index, byte_index, value;
	
	if (argc <= 1) {
		return dump_pkt_help();
	}
	
	if (argc == 2) {
		if (!strcasecmp(argv[1], "enable")) {
			dump_pkt_cfg.enable = 1;
		} else if (!strcasecmp(argv[1], "disable")) {
			dump_pkt_cfg.enable = 0;
		} else if (!strcasecmp(argv[1], "clear")) {
			memset(dump_pkt_cfg.filter, 0, sizeof(dump_pkt_cfg.filter));
            return;
		}  else if (!strcasecmp(argv[1], "show")) {
			return show_dump_pkt_filter();
		} else {
			return dump_pkt_help();
		}
		system_printf("----%s pkt dump.\n", dump_pkt_cfg.enable ? "enable" : "disable");
		return;
	}

    if (!strcasecmp(argv[1], "dir") && argc == 3) {
	    dump_pkt_cfg.dir = atoi(argv[2]);
        return;
    }
    
	if (5 != argc) {
		return dump_pkt_help();
	}

	if (strcasecmp(argv[1], "filter")) {
		return dump_pkt_help();
	}

	index = atoi(argv[2]);
	byte_index = atoi(argv[3]);
	value = atoi(argv[4]);
    set_dump_pkt_filter(index, byte_index, value);
	return;
}

static inline int do_pkt_filter(unsigned char *data, int len, debug_rx_tx_info_t *info)
{
	int i, match = 1;
    dump_pkt_filter_t *filter;
	
	if (!dump_pkt_cfg.enable)
		return 0;

    if (!(dump_pkt_cfg.dir & (1 << info->dir)))
        return 0;
    
	for (i = 0; i < DUMP_PKT_FILTER_RULE_MAX; ++i) {
        filter = &dump_pkt_cfg.filter[i];
		if (!filter->valid)
			continue;
		if (len <= filter->byte_index) {
			match = 0;
			break;
		}
		if (data[filter->byte_index] != filter->value)
			match = 0;
	}
	return match;
}

int dump_content(unsigned char *data, int len, int offset)
{
	int i = 0;

	for (i = 0; i < len; i++) {
		if (offset % 16 == 0)
			system_printf("\n%06x    ", offset);
		else if (offset != 0 && offset % 8 == 0)
			system_printf("   ");
		system_printf("%02x ", data[i]);
		offset++;
	}
	return offset;
}

/*******************************************************************************
 * Function: dump_pbuf
 * Description: dump a pkt which match filter.
 * Parameters: 
 *   Input: pbuf to dump, rx or tx.
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 ********************************************************************************/
void lwip_dump_pbuf(void *pkt, debug_rx_tx_info_t *info)
{
    struct pbuf *buf = (struct pbuf *)pkt;
	int offset = 0, len = 0;
	struct pbuf *tmp;
	unsigned char *data = buf->payload;
	
	if (!buf)
		return;
	if (!do_pkt_filter(data, buf->len, info))
		return;

	system_printf("\n---vif-%d---%s-[%d]----", info->vif,
		info->dir ? "tx" : "rx", buf->tot_len);

	offset = dump_content(buf->payload, buf->len, offset);
	for (tmp = buf->next; tmp != NULL; tmp = tmp->next) {
		offset = dump_content(tmp->payload, tmp->len, offset);
	}

	system_printf("\n-----end. len[%d]---\n", offset);
}

static int cmd_dump_pkt(cmd_tbl_t *t, int argc, char *argv[])
{
	dump_pkt_config(argc, argv);
	return CMD_RET_SUCCESS;
}

SUBCMD(debug, dump_pkt, cmd_dump_pkt, "for dump lwip pkt", "dump_pkt ");
#endif
#if 0
static inline int do_frame_filter(GenericMacHeader *gmh, debug_rx_tx_info_t *info)
{
	int match = 0;
	
	if (!dump_frame_cfg.enable)
		return match;

    if (!(dump_frame_cfg.dir & (1 << info->dir)))
        return match;
    
    if ((1 << gmh->type) & dump_frame_cfg.type && (1 << gmh->subtype) & dump_frame_cfg.stype)
        match = 1;
    
	return match;
}

void wifi_dump_frame(void *frame, debug_rx_tx_info_t *info)
{
    GenericMacHeader *gmh = (GenericMacHeader *)frame;
	
	if (!gmh)
		return;
	if (!do_frame_filter(gmh, info))
		return;
    
	system_printf("--vif%d-%s-type[%d|%d]-addr1[%02x:%02x:%02x:%02x:%02x:%02x]-"
        "addr2[%02x:%02x:%02x:%02x:%02x:%02x]---\n", info->vif,	info->dir ? "tx" : "rx", 
        gmh->type, gmh->subtype, gmh->address1[0], gmh->address1[1], gmh->address1[2], gmh->address1[3],
		gmh->address1[4], gmh->address1[5], gmh->address2[0], gmh->address2[1], gmh->address2[2], 
		gmh->address2[3], gmh->address2[4], gmh->address2[5]);
}

void dump_frame_config(int argc, char **argv)
{
	if (argc <= 1) {
		return;
	}
	
	if (argc >= 2) {
		if (!strcasecmp(argv[1], "enable")) {
			dump_frame_cfg.enable = 1;
		} else if (!strcasecmp(argv[1], "disable")) {
			dump_frame_cfg.enable = 0;
		} else if (!strcasecmp(argv[1], "show")) {
		    system_printf("enable[%d],dir[%d],type[0x%x],stype[0x%x]\n", dump_frame_cfg.enable,
                dump_frame_cfg.dir, dump_frame_cfg.type, dump_frame_cfg.stype);
			return;
		} else if (!strcasecmp(argv[1], "type") && argc == 3) {
            dump_frame_cfg.type = strtoul(argv[2], NULL, 0); 
            return;
        } else if (!strcasecmp(argv[1], "stype") && argc == 3) {
            dump_frame_cfg.stype = strtoul(argv[2], NULL, 0);
            return;
        } else if (!strcasecmp(argv[1], "dir") && argc == 3) {
            dump_frame_cfg.dir = strtoul(argv[2], NULL, 0);
            return;
        } else {
			return;
		}
		system_printf("----%s frame dump.\n", dump_frame_cfg.enable ? "enable" : "disable");
		return;
	}

    return;
}

static int cmd_dump_frame(cmd_tbl_t *t, int argc, char *argv[])
{
	dump_frame_config(argc, argv);
	return CMD_RET_SUCCESS;
}

SUBCMD(debug, dump_frame, cmd_dump_frame, "for dump 80211 frame", "dump_frame ");


#if 0//#ifdef LWIP_DEBUG
extern void lwip_debug(int argc, char **argv);
static int cmd_lwip_debug(cmd_tbl_t *t, int argc, char *argv[])
{
    lwip_debug(argc, argv);
    return CMD_RET_SUCCESS;
}

SUBCMD(debug, lwip, cmd_lwip_debug, "for debug lwip", "lwip ");
#endif

CMD(debug, NULL, "system debug cli", "debug [configure]");
#endif

#ifdef TELNETD_ENABLE

#include "system.h"
#include "task.h"
#include "util_cmd.h"
#include "lwip/sockets.h"

/* Telnet protocol stuff ****************************************************/
#define TELNET_NL             0x0a
#define TELNET_CR             0x0d

/* Telnet commands */
#define TELNET_ECHO           1
#define TELNET_SGA            3     /* Suppress Go Ahead */
#define TELNET_NAWS           31    /* Negotiate about window size */

/* Telnet control */
#define TELNET_IAC            255
#define TELNET_WILL           251
#define TELNET_WONT           252
#define TELNET_DO             253
#define TELNET_DONT           254
#define TELNET_SB             250
#define TELNET_SE             240

#define MAX_CMD_LEN 256
#define MAX_CMD_RINGBUFFER_LEN 256
#define MAX_LOG_RINGBUFFER_LEN 3072

//#define PROMT ">"

/* The state of the telnet parser */
enum telnet_state_e
{
	STATE_NORMAL = 0,
	STATE_IAC,
	STATE_WILL,
	STATE_WONT,
	STATE_DO,
	STATE_DONT,
	STATE_SB,
	STATE_SB_NAWS,
	STATE_SE
};

struct telnet_RingBuffer telnet_cmd_RingBuffer={0};
struct telnet_RingBuffer telnet_log_RingBuffer={0};

SemaphoreHandle_t telnet_waite_printf_Semaphore;

int telnet_state=STATE_NORMAL;
int telnet_socket_accept_sd=-1;
int telnet_negotiate_success=0;


int write_telnet_RingBuffer(struct telnet_RingBuffer* ptcmd, char *data, int len)
{
	int i=0;
	unsigned long flags;
	
	if(ptcmd->write_pos==(ptcmd->read_pos+ptcmd->buffer_len-1)%ptcmd->buffer_len)
	{
		return -1;
	}
	
	flags = system_irq_save();
	for(i=0;i<len;i++)
	{
		*(ptcmd->buffer + ptcmd->write_pos)=data[i];
		ptcmd->write_pos=(ptcmd->write_pos+1)%ptcmd->buffer_len;
		if(ptcmd->write_pos==ptcmd->read_pos)
		{
			break;
		}
	}
	system_irq_restore(flags);
	
	return len;
}

int read_telnet_RingBuffer(struct telnet_RingBuffer* ptcmd, char *data, int len)
{
	int i=0;
	if(ptcmd->write_pos==ptcmd->read_pos)
	{
		return -1;
	}
	for(i=0;i<len;i++)
	{
		data[i]=*(ptcmd->buffer + ptcmd->read_pos);
		ptcmd->read_pos=(ptcmd->read_pos+1)%ptcmd->buffer_len;
		if(ptcmd->write_pos==ptcmd->read_pos)
		{
			break;
		}
	}
	return i+1;
}

int telnet_sendopt(int sockfd, uint8_t option, uint8_t value)
{
	uint8_t optbuf[3];
	int ret;

	optbuf[0] = TELNET_IAC;
	optbuf[1] = option;
	optbuf[2] = value;

	ret = send(sockfd, optbuf, 3, 0);
	return ret;
}

int telnet_send(int sockfd, char*data,int datalen)
{
	int ret;
	//printf("send %d\n",data[0]);
	ret = send(sockfd, data, datalen, 0);
	return ret;
}

int telnet_getchar(uint8_t ch, char *dest, int *nread)
{
	int end=0;
	static int recived_cr=0;

	//telnet_send(telnet_socket_accept_sd,(char*)&ch,1);  

	if (ch == 0)
	{
		return 0;
	}
	else if(ch == TELNET_CR)
	{
		recived_cr=1;
		*(dest+*nread)=0;
		end=1;
	}
	else if(ch == TELNET_NL)
	{
		if(recived_cr==1)
		{
			recived_cr=0;
			return 0;
		}
		else
		{
			*(dest+*nread)=0;
			end=1;
		}
	}
	else
	{
		*(dest+*nread)=ch; 
		recived_cr=0;
	}

	*nread=*nread+1;
	//printf("end %x\n",end);
	return end;
}

int telnet_RingBuffer_parse(struct telnet_RingBuffer* ptcmd, char*cmd, int *cmdlen)
{
	uint8_t ch;
	int end=0;
	while(read_telnet_RingBuffer(ptcmd, (char*)&ch, 1)>0)
	{
		//printf("ch=%02x state=%d\n", ch, telnet_state);

		switch (telnet_state)
		{
			case STATE_IAC:
				if (ch == TELNET_IAC)
				{
					telnet_state = STATE_NORMAL;
				}
				else
				{
					switch (ch)
					{
						case TELNET_WILL:
							telnet_state = STATE_WILL;
							break;

						case TELNET_WONT:
							telnet_state = STATE_WONT;
							break;

						case TELNET_DO:
							telnet_state = STATE_DO;
							break;

						case TELNET_DONT:
							telnet_state = STATE_DONT;
							break;

						default:
							telnet_state = STATE_NORMAL;
							break;
					}
				}
				break;

			case STATE_WILL:
				telnet_sendopt(telnet_socket_accept_sd, TELNET_DONT, ch);
				system_printf("Suppress: 0x%02X (%d)\n", ch, ch);
				telnet_state = STATE_NORMAL;
				//telnet_send(telnet_socket_accept_sd,PROMT,strlen(PROMT));
				break;

			case STATE_WONT:
				telnet_sendopt(telnet_socket_accept_sd, TELNET_DONT, ch);
				telnet_state = STATE_NORMAL;
				//telnet_send(telnet_socket_accept_sd,PROMT,strlen(PROMT));
				break;

			case STATE_DO:
				if (ch == TELNET_SGA || ch == TELNET_ECHO)
				{
					/* If it received 'ECHO' or 'Suppress Go Ahead', then do
					* nothing.
					*/
				}
				else
				{
					/* Reply with a WONT */

					telnet_sendopt(telnet_socket_accept_sd, TELNET_WONT, ch);
					system_printf("WONT: 0x%02X\n", ch);
				}
				telnet_state = STATE_NORMAL;
				//telnet_send(telnet_socket_accept_sd,PROMT,strlen(PROMT));
				break;

			case STATE_DONT:
				/* Reply with a WONT */
				telnet_sendopt(telnet_socket_accept_sd, TELNET_WONT, ch);
				telnet_state = STATE_NORMAL;
				//telnet_send(telnet_socket_accept_sd,PROMT,strlen(PROMT));
				break;

			case STATE_NORMAL:
				if (ch == TELNET_IAC)
				{
					telnet_state = STATE_IAC;
				}
				else
				{
					if(telnet_getchar(ch, cmd, cmdlen))
					{
						end=1;
					}
				}
				break;
		}
		if(end)
		{
			//printf("parse end\n");
			return 1;
		}
	}
	return 0;
}

void telnet_printf_task()
{
	char logbuf[512];
	int loglen;
	
	//system_printf("run telnet_printf_task\r\n");
	while(1)
	{
		xSemaphoreTake(telnet_waite_printf_Semaphore, portMAX_DELAY);
		if(-1!=telnet_socket_accept_sd)
		{
			while(1)
			{
				//panic_printf("rpos %d, wpos %d\r\n", telnet_log_RingBuffer.read_pos,telnet_log_RingBuffer.write_pos);
				loglen=read_telnet_RingBuffer(&telnet_log_RingBuffer,logbuf,512);
				logbuf[511]=0;
				//panic_printf("ret=%d, rpos %d, wpos %d, %s\r\n", loglen, telnet_log_RingBuffer.read_pos,telnet_log_RingBuffer.write_pos,logbuf);
				if(loglen>0)
				{
					telnet_send(telnet_socket_accept_sd, logbuf, loglen);
				}
				else
				{
					break;
				}
			}
		}
	}
}

void telnet_task(void* t)
{
	struct sockaddr_in  ipv4;
	struct sockaddr_in  addr;
	int listensd;

	socklen_t addrlen;
	socklen_t accptlen;
	
	struct linger ling;
	int ret;
	int optval;
	
	fd_set reads;
	fd_set excepts;
	int fd_count;
	
	ssize_t read_s;
	char cmdbuf[MAX_CMD_LEN]={0};
	char telnet_cmd[MAX_CMD_LEN]={0};
	int telnet_cmd_len=0;

	ipv4.sin_family      = AF_INET;
    ipv4.sin_port        = htons(23);
    ipv4.sin_addr.s_addr = INADDR_ANY;
    addrlen              = sizeof(struct sockaddr_in);
	
	listensd = socket(AF_INET, SOCK_STREAM, 0);
	if(listensd<0)
	{
		system_printf("socket:%s listensd:%d\r\n", strerror(errno), listensd);
		vTaskDelete(NULL);
	}
	
	optval = 1;
	if (setsockopt(listensd, SOL_SOCKET, SO_REUSEADDR,(void *)&optval, sizeof(int)) < 0)
	{
		system_printf("setsockopt:%s optval:%d\r\n", strerror(errno), optval);
		vTaskDelete(NULL);
	}

	if(bind(listensd, (struct sockaddr*)&ipv4, addrlen))
	{
		system_printf("bind:%s\n",strerror(errno));
		vTaskDelete(NULL);
	}
	
	if(listen(listensd, 5))
	{
		system_printf("listen:%s\n",strerror(errno));
		vTaskDelete(NULL);
	}
	
	while(1)
	{
		accptlen = sizeof(addr);
		//system_printf("socket wait connect\r\n");
	    telnet_socket_accept_sd = accept(listensd, (struct sockaddr*)&addr, &accptlen);
		//system_printf("socket connected telnet_socket_accept_sd=%d\r\n", telnet_socket_accept_sd);
		if(telnet_socket_accept_sd<0)
		{
			system_printf("accept:%s\n",strerror(errno));
			vTaskDelete(NULL);
		}
		
		ling.l_onoff  = 1;
	    ling.l_linger = 30;     /* timeout is seconds */
	    setsockopt(telnet_socket_accept_sd, SOL_SOCKET, SO_LINGER, &ling, sizeof(struct linger));
		
		ret=telnet_sendopt(telnet_socket_accept_sd, TELNET_WILL, TELNET_SGA);
	  	if(ret<0)
		{
			system_printf("send TELNET_SGA:%s\n",strerror(errno));
			//vTaskDelete(NULL);
			goto errproc;
		}
		ret=telnet_sendopt(telnet_socket_accept_sd, TELNET_WILL, TELNET_ECHO);
		if(ret<0)
		{
			system_printf("send TELNET_ECHO:%s\n",strerror(errno));
			//vTaskDelete(NULL);
			goto errproc;
		}
	 	//close(telnet_socket_accept_sd);
	 	//close(listensd);
		system_printf("################## telnet is ready to communicate ##################\r\n");
		char* wellcome_log = "password>> ";
		ret = telnet_send(telnet_socket_accept_sd, wellcome_log, strlen(wellcome_log));
		if(ret<0)
		{
			system_printf("send wellcome_log:%s\n",strerror(errno));
			//vTaskDelete(NULL);
			goto errproc;
		}
		while(1)
		{
			FD_ZERO(&reads);
			FD_ZERO(&excepts);
			FD_SET(telnet_socket_accept_sd, &reads);
			FD_SET(telnet_socket_accept_sd, &excepts);

			fd_count= select(telnet_socket_accept_sd+1, &reads, NULL, &excepts, NULL);
			if(fd_count>0)
			{
				if(FD_ISSET(telnet_socket_accept_sd, &excepts))
				{
					system_printf("select exception:%s\n",strerror(errno));
					break;
				}
				if(FD_ISSET(telnet_socket_accept_sd, &reads))
				{
					read_s = read(telnet_socket_accept_sd, cmdbuf, sizeof(cmdbuf));
					if(read_s>0)
					{
						write_telnet_RingBuffer(&telnet_cmd_RingBuffer, cmdbuf, read_s);
						while(telnet_RingBuffer_parse(&telnet_cmd_RingBuffer, telnet_cmd, &telnet_cmd_len))
						{
							extern void execv_cmd(char*cmd);
							execv_cmd(telnet_cmd);
							telnet_cmd_len=0;
							
							//telnet_send(telnet_socket_accept_sd,PROMT,strlen(PROMT));
						}
					}
					else if(read_s<0)
					{
						system_printf("select:%s\n",strerror(errno));
						break;
					}
				}
			}
			else if(fd_count<0&&errno!=EINTR)
			{
				system_printf("select:%s\n",strerror(errno));
				break;
			}
		}
errproc:
		close(telnet_socket_accept_sd);
		telnet_socket_accept_sd=-1;
		telnet_negotiate_success=0;
	}
}

void telnet_init(void)
{
	portBASE_TYPE result = 0;
	
	telnet_negotiate_success=0;
	
	telnet_cmd_RingBuffer.buffer = (char*)os_malloc(MAX_CMD_RINGBUFFER_LEN);
	telnet_cmd_RingBuffer.buffer_len = MAX_CMD_RINGBUFFER_LEN;
	telnet_log_RingBuffer.buffer = (char*)os_malloc(MAX_LOG_RINGBUFFER_LEN);
	telnet_log_RingBuffer.buffer_len = MAX_LOG_RINGBUFFER_LEN;

	telnet_waite_printf_Semaphore = xSemaphoreCreateBinary();

	result = xTaskCreate(telnet_task, (const char *)"telnet_task", 1024, NULL, 3, NULL);
	if(result == pdPASS)
	{
		system_printf("create telnet_task ok\r\n");

		result = xTaskCreate(telnet_printf_task, (const char *)"telnet_printf_task", 1024, NULL, tskIDLE_PRIORITY+1, NULL);
		if(result == pdPASS)
		{
			system_printf("create telnet_printf_task ok\r\n");
		}else{
			system_printf("create telnet_printf_task faile %d\r\n", result);
		}
	}else{
		system_printf("create telnet_task faile %d\r\n", result);
	}
}

static int telnet_test(cmd_tbl_t *h, int argc, char *argv[])
{
 	static uint8_t telnet_is_connected = 0;
	if(telnet_is_connected)
	{
		system_printf("telnet is running\r\n");
	}
	else
	{
		telnet_init();
		telnet_is_connected = 1;
	}
	
	return CMD_RET_SUCCESS;
}

CMD(telnet,
    telnet_test,
    "run telnet",
    "no");



#endif

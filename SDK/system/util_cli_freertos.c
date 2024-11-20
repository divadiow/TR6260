#include "system.h"
#include "string.h"
#include "heap.h"

#if !defined(RELEASE)
#define CLI_HISTORY
#endif

#ifndef AMT
#define MAX_INPUT_LENGTH (NRC_MAX_CMDLINE_SIZE)
#else
#define MAX_INPUT_LENGTH 1152
extern int global_amt_uboot_flag;
#endif
#define IS_ASCII(c) (c > 0x1F && c < 0x7F)

#ifdef CLI_HISTORY
#define MAX_HISTORY 10
char g_cli_history[MAX_HISTORY][MAX_INPUT_LENGTH];
uint8_t g_cli_history_index, g_cli_history_rd_index = 0;
bool g_cli_history_escape = false;
#endif

//static char *(*g_cli_prompt_func)();
static int (*g_cli_run_command)(char*);
bool g_cli_typing;

static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
		StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer		= &xTimerTaskTCBBuffer;
	*ppxTimerTaskStackBuffer	= &xTimerStack[0];
	*pulTimerTaskStackSize		= configTIMER_TASK_STACK_DEPTH;
}

static void print_prompt()
{
#ifndef UART_WPA_SEPARATION
	static int line;
	system_printf("[%d]%s:", line++, TARGET_NAME);
#endif
}

struct temp_data
{
	struct temp_data *next;
	char data[0];
};

#define MAX_BUFFER_LEN 256
struct cli_buffer
{
	int index;
	char *current_buffer;
	char *dynamic_buffer;
	char static_buffer[MAX_BUFFER_LEN];
};

static int32_t cli_static_buffer_handler(struct cli_buffer *cli)
{
	unsigned long flags;
	struct temp_data local_temp_data;
	char* local_rxdata;
	int local_index = 0;

	struct temp_data *head=NULL;
	struct temp_data *tail=NULL;
	struct temp_data *pdata=NULL;
	struct temp_data *pnext=NULL;

	if(cli->current_buffer == cli->dynamic_buffer)	// if now use dynamic_buffer, just return
	{
		return 1;
	}
	
	flags = system_irq_save();
	
	local_rxdata=cli->current_buffer;
	while(local_index < cli->index)
	{
		if(local_rxdata[local_index++]=='\0')	// find '\0'
		{
			pdata=pvPortMalloc(sizeof(struct temp_data)+local_index);
			pdata->next=NULL;
			memcpy((struct temp_data*)pdata->data, local_rxdata, local_index);
			if(head==NULL)
			{
				head=pdata;
			}
			if(tail==NULL)
			{
				tail=pdata;
			}
			else
			{
				tail->next=pdata;
				tail=pdata;
			}
			local_rxdata = &local_rxdata[local_index];		// next ...
			cli->index=cli->index - local_index;			// delete used data
			if(cli->index < 0) cli->index=0;
			local_index = 0;								// reset index
		}
	}
	
	if(cli->index)
	{
		memcpy(cli->static_buffer, &local_rxdata[local_index], cli->index-local_index);
	}
	system_irq_restore(flags);
	
	pnext=head;
	while(pnext)
	{
		if(g_cli_run_command) {
			g_cli_run_command(pnext->data);
		}
		pnext=pnext->next;
	}
	print_prompt();
	return 0;
}


static int32_t cli_dynamic_buffer_handler(void *arg)
{
	char *str = arg;
	if(g_cli_run_command) {
		g_cli_run_command(str);
	}
	vPortFree(str);				
	print_prompt();
	return 0;
}
#ifdef TELNETD_ENABLE
extern int telnet_negotiate_success;
extern int telnet_socket_accept_sd;
extern int telnet_send(int sockfd, char*data,int datalen);
void execv_cmd(char*cmd)
{
	char password_str[20]="eswin";
	char *str=NULL;
	if(0 == telnet_negotiate_success)
	{
		//system_printf("recv %s\n",cmd);
		if(0 == strcmp(password_str, cmd))
		{

		  telnet_negotiate_success=1;
          telnet_send(telnet_socket_accept_sd, cmd, strlen(cmd));        
          char* wellcome_log0 = "\
					\r\n>> **************************\
					\r\n>>    ___  __                            \
					\r\n>>   |__  /__` |  | | |\\ |\
					\r\n>>   |___ .__/ |/\\| | | \\|\
					\r\n>>\
					\r\n>> **************************\
					\r\n>> Wellcome use telnet @eswin\r\n>> ";
           telnet_send(telnet_socket_accept_sd, wellcome_log0, strlen(wellcome_log0));
		}
		else
		{
			telnet_send(telnet_socket_accept_sd, cmd, strlen(cmd));
			char* wellcome_log = "\r\n>> Wrong password! ! !\r\npassword>> ";
			telnet_send(telnet_socket_accept_sd, wellcome_log, strlen(wellcome_log));
		}
		return;
	}
	str = os_malloc(strlen(cmd)+1);
	sprintf(str,"%s",cmd);

	system_printf("%s\r\n", str);
	if(false == system_schedule_work_queue(cli_dynamic_buffer_handler, str, NULL))
	{
		os_free(str);
	}
}
#endif
#if 0
uint8_t dump_cur_task(char *cmd)
{
    TaskHandle_t task;
    
    if (!cmd || cmd[0] == '\0')
        return -1;
    if (strcmp(cmd, "show task"))
        return 0;

    task = xTaskGetCurrentTaskHandle();
    system_printf("Current Task: %s\n", pcTaskGetName(task));

	vPortFree(cmd);
	print_prompt();
    return 1;
}
#endif
struct cli_buffer cli_buf={0};
char *g_dynbuffer_to_free=NULL;

// 20201028  liuyong
// #define UART_TEST_just_for_liuyong
#ifdef UART_TEST_just_for_liuyong
int g_test = 0;
int g_test_char_num = 16;
#endif

int uart_rx_buffer_assign(struct cli_buffer *cli)
{
	char *local_dynamic_buffer=NULL;
	char *local_rxdata=NULL;
	int local_index=0;
	
	if (NULL==cli->dynamic_buffer) {		// if no dynamic_buffer
		if(g_dynbuffer_to_free)				// and wang to free
		{
			cli->dynamic_buffer=g_dynbuffer_to_free;	// so use g_dynbuffer_to_free
			cli->index=0;								// clear cli.index
			g_dynbuffer_to_free=NULL;					// change flag=NULL to not free
			cli->current_buffer = cli->dynamic_buffer;
			return 1;  // dynamic_buffer
		}
		else if(
#ifdef UART_TEST_just_for_liuyong
			g_test<g_test_char_num || 
#endif
			heapIN_CRITICAL())		// heap in protect, just use static_buffer
		{
#ifdef UART_TEST_just_for_liuyong
			g_test++;
			system_printf(".");
#endif
			cli->current_buffer=cli->static_buffer;
			return 0;  // static_buffer
		}
		else								// malloc dynamic_buffer
		{
			if(cli->index)			// if used static_buffer, so cli->index>0
			{
				//ASSERT(cli->current_buffer==cli->static_buffer);
				local_rxdata=cli->current_buffer;
				while(local_index < cli->index)
				{
					if(local_rxdata[local_index++]=='\0')	// find '\0'
					{
						local_dynamic_buffer = pvPortMalloc(local_index);  // local_dynamic_buffer melloc enough size for '\0'
						memcpy(local_dynamic_buffer, local_rxdata, local_index);
						if(false == system_schedule_work_queue_from_isr(cli_dynamic_buffer_handler, local_dynamic_buffer, NULL))
						{
							vPortFree(cli->dynamic_buffer);
						}  // if == true, it will be free in thread

						local_rxdata = &local_rxdata[local_index]; 	// next ...
						cli->index=cli->index - local_index;		// delete used data
						if(cli->index < 0) cli->index=0;
						local_index = 0;							// reset index
					}
				}
			}

			cli->dynamic_buffer = pvPortMalloc(MAX_INPUT_LENGTH);	// malloc		
			ASSERT(cli->dynamic_buffer);
			if(cli->index)
			{
				memcpy(cli->dynamic_buffer, local_rxdata, cli->index);	// copy g_rx_buffer to cli.buffer
			}
			//memset(cli->current_buffer,0x00,sizeof(cli->current_buffer));
			cli->current_buffer=cli->dynamic_buffer;
			return 1;  // dynamic_buffer

		}
	}
	
	cli->current_buffer=cli->dynamic_buffer;
	return 1;  // dynamic_buffer
}

int uart_rx_buffer_free(struct cli_buffer *cli, int isfree)
{
	if(isfree&&cli->dynamic_buffer)
	{
		if(heapIN_CRITICAL())	// in heap protect
		{
			g_dynbuffer_to_free=cli->dynamic_buffer;	// in heap protect, so set flag, indicates that memory needs to be freed
		}
		else
		{
			vPortFree(cli->dynamic_buffer);		// free
		}
	}
	cli->dynamic_buffer = NULL;
	cli->index=0;

	return 0;
}


void util_cli_callback(char c)
{
	int is_dynamic_buff = 0;
	
	struct cli_buffer *cli = &cli_buf;
	is_dynamic_buff = uart_rx_buffer_assign(cli);	// check and assign buffer

#if 0//#ifdef CLI_HISTORY
	// To command history
	if (c == ESCAPE_KEY) {
		int i;
		for (i = 0; i < cli.index; i++) // move cursor to next of prompt
			system_oprintf("\b", 1);
		for (i = 0; i < MAX_INPUT_LENGTH; i++) 	// delete all char in console
			system_oprintf(" ", 1);
		for (i = 0; i < MAX_INPUT_LENGTH; i++) // move cursor to next of prompt
			system_oprintf("\b", 1);

		cli.index = 0;
		cli.buffer[cli.index++] = c;
		g_cli_history_escape = true;
		return;
	}

	//show history
	if (g_cli_history_escape) {
		cli.buffer[cli.index++] = c;
		if (cli.index == 3) {
			g_cli_history_escape = false;
			if (cli.buffer[0] == 27 && cli.buffer[1] == 91 && cli.buffer[2] == 65) {
				int i;
				for (i = 0; i < MAX_HISTORY; i++) {
					if (g_cli_history_rd_index == 0)
						g_cli_history_rd_index = MAX_HISTORY - 1;
					else
						g_cli_history_rd_index--;

					if (g_cli_history[g_cli_history_rd_index][0] != '\0')
						break;
				}
				char *history = &g_cli_history[g_cli_history_rd_index][0];
				system_printf("%s", history);
				cli.index = strlen(history);
				memcpy(cli.buffer, history, cli.index);
			} else {
				cli.index = 0;
			}
		}
		return;
	}

	g_cli_history_escape = false;

	if(c == RETURN_KEY) {
		if (cli.index > 0) {
			memset(&g_cli_history[g_cli_history_index][0], 0, MAX_INPUT_LENGTH);
			memcpy(&g_cli_history[g_cli_history_index][0], cli.buffer, cli.index);
			if (++g_cli_history_index == MAX_HISTORY)
				g_cli_history_index = 0;

			g_cli_history_rd_index = g_cli_history_index;
		}
	}
#endif

	switch(c) {
		case BACKSP_KEY:
			if (cli->index > 0) {
				system_oprintf("\b \b", 3);
				cli->index--;
			}
			break;
		case RETURN_KEY: 					// if receive \r
			g_cli_typing = !g_cli_typing;
#ifdef UART_WPA_SEPARATION
			system_oprintf("\n", 1);
#else
#ifndef AMT
			system_printf("\n");
#endif
#endif
			if (cli->index > 0) {			// and received valid character data
				ASSERT(cli->current_buffer);
				cli->current_buffer[cli->index++] = '\0';		// add '\0'
				if(is_dynamic_buff)		// use dynamic_buff
				{
					//if (!dump_cur_task(cli.buffer)) {
	                //}
					if(system_schedule_work_queue_from_isr(cli_dynamic_buffer_handler, cli->current_buffer, NULL))  // send this data to queue and free in thread
					{
						uart_rx_buffer_free(cli,0);		// not free
					}
					else
					{
						uart_rx_buffer_free(cli,1);		// free and set NULL
					}
					print_prompt();
				}
				else  					// use static_buff
				{
					system_schedule_work_queue_from_isr((sys_task_func)cli_static_buffer_handler, cli, NULL);
				}
			} else {
				print_prompt();
			}
			break;
		case '\n':
			break;
		default:
			if (IS_ASCII(c) && cli->index < MAX_INPUT_LENGTH) {
				g_cli_typing = true;
				cli->current_buffer[cli->index++] = c;
				//cli.buffer[cli.index] = '\0';
				#ifndef AMT
				system_oprintf(&c, 1);
				#else
				if(!global_amt_uboot_flag)
					system_oprintf(&c, 1);
				#endif
				//uart_data_write(UART0_BASE, (const unsigned char *)&c, 1);
			}
			break;
	}
}


bool util_cli_freertos_init( int (*run_command)(char *))
{
//    g_cli_prompt_func = prompt_func;
    g_cli_run_command = run_command;

    return true;
}

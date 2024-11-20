


#include "drv_uart.h"
#include "system_common.h"
#include "task.h"
#include "lwip/sockets.h"
#include "system_event.h"

int socket_server =-1;
int socket_client =-1;
int socket_new_server = -1;
socklen_t sin_size;
struct sockaddr_in sock_addr;
struct sockaddr_storage client_addr;
TaskHandle_t socket_rx_handle,socket_tx_handle;
uart_handle_t uart_handle = NULL;

extern SemaphoreHandle_t xCountingSemaphore_uart;
unsigned int task_socket_tx=0;


int dma_config = 1;
int uart_id = UART_ID_1;
int hal_socket_client()
{
	socket_client = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_client<0)
	{
		system_printf("create socket fail\n");
		return -1;
	}	
	 struct sockaddr_in sock_addr;
	 memset(&sock_addr, 0, sizeof(sock_addr));
	 sock_addr.sin_family = AF_INET;
	 sock_addr.sin_port = lwip_htons(50001);
	 char ip[18] = {0};
	 if( ef_get_env_blob(NV_WIFI_IP , ip, 18, NULL))
	 {
		system_printf("get nv ok [%s:%d]\n", ip, strlen(ip));
	 }
	 else
	 {
		strcpy(ip,"192.168.31.6");
	 }
   
	 sock_addr.sin_addr.s_addr = inet_addr(ip);
	 unsigned int connection = connect( socket_client, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
	 if(connection != 0)
	 {
		 close(socket_client);
		 system_printf("socket connect fail\n");
		 return -1;
	 }
	 system_printf("socket connect success\n");
	 return connection;
}

int total_len = 0;

static void socket_send(void *pvParameters)
{ 
	int ret=0;	 
	uint32_t socket_send_len = 0;
    int i=0;
	int retry_times = 0;
	
	 while(1)
	 {	 			
	 	xSemaphoreTake(xCountingSemaphore_uart, portMAX_DELAY); 
		socket_send_len = hal_uart_get_rx_len();	
		//system_printf("socket_send_len=%d\n",socket_send_len);
		total_len += socket_send_len;
		if(socket_send_len>0)
		{
			system_event_t evet;
			int a = IN32(0x9004c8);
			int timeout=IN32(0x9004c8);
			ret = write(socket_client ,(char*)hal_uart_get_rx_buff(),socket_send_len);		
			//system_printf("ret=%d\n",ret);
           	while(ret < 0)
           	{
           		system_printf("send fail\n");
           		system_printf("errno=%d\n",errno);
				if(ret<0)
				{
					close(socket_client);	
					ret = hal_socket_client();
        			while(ret!=0 &&(IN32(0x9004c8)-timeout)<30*1000*1000)
        			{
        				close(socket_client);
        				ret = hal_socket_client();
        			}
        			if(ret==0)
               		{
               			system_printf("connection success\n");
               			ret = write(socket_client, (char*)hal_uart_get_rx_buff(),socket_send_len);
               		}
               		else
               		{
               			system_printf("connection fail\n");
               			vTaskDelete(socket_tx_handle);
               			break;
               		}
				}
				else
				{
					close(socket_client);
					vTaskDelete(socket_tx_handle);
					break;
				}
           	}
          	
			hal_uart_get_rx_data_end();		
			int b = IN32(0x9004c8);
			if((b-a)>20000)
			system_printf("time = %d\n",b-a);
		}
	}		 
}	 	 		


void hal_socket_tx_task()
{	
    task_socket_tx =xTaskCreate(socket_send, (const char *)"socket_send", 2048, NULL, 4, &socket_tx_handle);
}

static void socket_read(void *pvParameters)
{

   int ret=0;
   int avil_length=0;
   while(1)
   {
		sin_size=sizeof(struct sockaddr_in); 
		if((socket_new_server=accept(socket_server,(struct sockaddr*)(&client_addr),&sin_size))==-1)
		{
			system_printf("Accept error:%s\n\a",strerror(errno));
			continue;
		}
 		system_printf("client connect\n\a");
	    while(1) 
	    {
	    	avil_length = hal_uart_get_txbuff_avail_len();
			if (avil_length > 0)
			{
				if((ret = read(socket_new_server,(char*)hal_uart_get_tx_buff(),avil_length))<0)
			    {
					close(socket_new_server);
					socket_new_server = -1;
					vTaskDelay(200/portTICK_PERIOD_MS);
				    break;
				}
				if (ret > 0)
				{
					hal_uart_socket_rec_end(ret);
				}
			}
				  
	   }
   }

}   


void hal_socket_server()
{
	socket_server = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_server < 0)
	{
		system_printf("create socket fail\n");
		return;
	}
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = lwip_htons(50000);
	char ip[16];
	sock_addr.sin_addr.s_addr=htonl(INADDR_ANY); 
	if(bind(socket_server,(struct sockaddr*)&sock_addr,sizeof(sock_addr))<0)
	{
		system_printf("socket bind fail\n");
		return;
	}
	listen(socket_server, 5);
				
    xTaskCreate(socket_read, (const char *)"socket_read", 2048, NULL, 4, &socket_rx_handle);
}


static int	hal_socket_tx(cmd_tbl_t *tt, int argc, char *argv[])
{
	//hal_uart_set_timeout_rx(50);
	if (uart_handle == NULL)
	{		
		uart_handle = hal_uart_open_data(dma_config,uart_id, UART_DATA_BIT_8, 115200, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
		system_printf("uart open success\n");
	}
	else if(dma_config == 1)
	{
		hal_uart_dma_rx_config(uart_handle);
	}
	else
	{
		hal_uart_start_timer();		
	}
	if(socket_client==-1)
	{
		int ret = hal_socket_client();
		if(!ret)
		{		
			hal_socket_tx_task();
		}
	}
	else
	{
		close(socket_client);
		vTaskDelete(socket_tx_handle);
		socket_client = -1;
		int ret = hal_socket_client();
		if(!ret)
		{		
			hal_socket_tx_task();
		}
	}
	return CMD_RET_SUCCESS;
}
	
CMD(socket_tx, hal_socket_tx,  "hal_socket_tx",  "hal_socket_tx");

static int	hal_len(cmd_tbl_t *tt, int argc, char *argv[])
{
	system_printf("total_len = %d\n",total_len);
	
	return CMD_RET_SUCCESS;
}
	
CMD(len, hal_len,  "len",  "len");

static int	hal_socket_rx(cmd_tbl_t *tt, int argc, char *argv[])
{
	if (uart_handle == NULL)
	{		
		uart_handle = hal_uart_open_data(dma_config,uart_id, UART_DATA_BIT_8, 115200, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
		system_printf("uart open success\n");
	}
	if(socket_server ==-1)
	{
		hal_socket_server();
	}
	else
	{
		vTaskDelete(socket_rx_handle); 
		close(socket_new_server);
		close(socket_server);
		socket_new_server = -1;
		socket_server = -1;
		hal_socket_server();
		
	}
	return CMD_RET_SUCCESS;
}

CMD(socket_rx, hal_socket_rx,	"hal_socket_rx",  "hal_socket_rx");


static int	hal_socket_trx(cmd_tbl_t *tt, int argc, char *argv[])
{
	hal_uart_set_timeout_rx(50);
	uart_handle = hal_uart_open_data(dma_config,uart_id, UART_DATA_BIT_8, UART_BAUD_RATE_115200, UART_PARITY_NONE, UART_STOP_BIT_1, 0);
	system_printf("uart open success\n");
	
	if(socket_client==-1)
	{
		int ret = hal_socket_client();
		if(!ret)
		{		
			hal_socket_tx_task();
		}
	}
	else
	{
		close(socket_client);
		vTaskDelete(socket_tx_handle);
		socket_client = -1;
		int ret = hal_socket_client();
		if(!ret)
		{		
			hal_socket_tx_task();
		}
	}
		
	system_printf("create client success\n");
	if(socket_server ==-1)
	{
		hal_socket_server();
	}
	else
	{
		vTaskDelete(socket_rx_handle); 
		close(socket_new_server);
		close(socket_server);
		socket_new_server = -1;
		socket_server = -1;
		hal_socket_server();
		
	}
	system_printf("create server success\n");
	
	return CMD_RET_SUCCESS;
}

CMD(socket_trx, hal_socket_trx,	"socket_trx",  "socket_trx");

void hal_stop_socket_read()
{	
	if(eTaskGetState(socket_rx_handle)==eDeleted)
	{
		system_printf(" task eDeleted\n");
	}
	else
	{
		vTaskDelete(socket_rx_handle); 
	}
	
	if(socket_new_server!=-1)
	{
		close(socket_new_server);
	}

	if(socket_server!=-1)
	{
		close(socket_server);
	}

	socket_new_server = -1;
	socket_server = -1;
	hal_uart_tx_chan_stop();
	hal_uart_tx_buff_reset();
}

static int stop_socket_rx(cmd_tbl_t *tt, int argc, char *argv[])
{

	hal_stop_socket_read();
	return CMD_RET_SUCCESS;
}

CMD(stop_rx, stop_socket_rx,  "stop_socket_rx",	"stop_socket_rx");

void hal_stop_socket_send()
{	
	if( task_socket_tx==1)
	{
		if(eTaskGetState(socket_tx_handle)==eDeleted)
		{
			system_printf(" task eDeleted\n");
		}
		else
		{
			vTaskDelete(socket_tx_handle);
		}
	}
	if(socket_client!=-1)
	{
		close(socket_client);
	}

	socket_client = -1;
	hal_uart_rx_chan_stop();
	hal_uart_rx_buff_reset();
}

static int stop_socket_tx(cmd_tbl_t *tt, int argc, char *argv[])
{
	
	hal_stop_socket_send();
	
	return CMD_RET_SUCCESS;
}
	
CMD(stop_tx, stop_socket_tx,  "stop_tx",  "stop_tx");

static int stop_socket_trx(cmd_tbl_t *tt, int argc, char *argv[])
{	
	if( task_socket_tx==1)
	{
		if(eTaskGetState(socket_tx_handle)==eDeleted)
		{
			system_printf(" task eDeleted\n");
		}
		else
		{
			vTaskDelete(socket_tx_handle);
		}
	}
	
	if(eTaskGetState(socket_rx_handle)==eDeleted)
	{
		system_printf(" task eDeleted\n");
	}
	else
	{
		vTaskDelete(socket_rx_handle); 
	}
	
	if(socket_client!=-1)
	{
		close(socket_client);
	}
	socket_client = -1;
	hal_uart_rx_chan_stop();

	
	if(socket_new_server!=-1)
	{
		close(socket_new_server);
	}
	if(socket_server!=-1)
	{
		
		close(socket_server);
	}
	socket_new_server = -1;
	socket_server = -1;
	hal_uart_tx_chan_stop();
	
	hal_uart_rx_buff_reset();
	hal_uart_tx_buff_reset();
	hal_uart_close(uart_handle);
	uart_handle = NULL;
	
	return CMD_RET_SUCCESS;
}
	
CMD(stop_trx, stop_socket_trx,  "stop_trx",  "stop_trx");


#if 0



/*
static void socket_read(void *pvParameters)
{

   int ret=0;
   int avil_length=0;
   while(1)
   {
		sin_size=sizeof(struct sockaddr_in); 
		if((socket_new_server=accept(socket_server,(struct sockaddr*)(&client_addr),&sin_size))==-1)
		{
			system_printf("Accept error:%s\n\a",strerror(errno));
		}
 
	    while(1) 
	    {
			

		        if((ret = read(socket_new_server,uart_src_buff+inde,avil_length))<0)
		        {
					  close(socket_new_server);
					  socket_new_server = -1;
					  vTaskDelay(200/portTICK_PERIOD_MS);
			          break;
			    }

				if (!ret)
				{
					//system_printf("read 0|%d|%d...........\n", avil_length, errno);
				}	


				
	  
	   }
	 }

}   


void hal_socket_server()
{
	socket_server = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_server < 0)
	{
		continue;
	}
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = lwip_htons(50000);
	char ip[16];
	sock_addr.sin_addr.s_addr=htonl(INADDR_ANY); 
	if(bind(socket_server,(struct sockaddr*)&sock_addr,sizeof(sock_addr))<0)
	{
		continue;
	}
	
	if (listen(socket_server, 5) == -1)
	{
		 continue;
	}
					
    xTaskCreate(socket_read, (const char *)"socket_read", 2048, NULL, 4, &socket_handle);
}
*/
void hal_stop_socket_send()
{
	close(socket_client);
	vTaskDelete(socket_tx_handle);	
}







static int	hal_socket_rx(cmd_tbl_t *tt, int argc, char *argv[])
{
	
		hal_socket_server();
		return CMD_RET_SUCCESS;
}

CMD(socket_rx, hal_socket_rx,	"hal_socket_rx",  "hal_socket_rx");

static int stop_socket_tx(cmd_tbl_t *tt, int argc, char *argv[])
{
	
		hal_stop_socket_send();
	
		return CMD_RET_SUCCESS;
}
	
CMD(stop_tx, stop_socket_tx,  "stop_socket_tx",  "stop_socket_tx");




#endif



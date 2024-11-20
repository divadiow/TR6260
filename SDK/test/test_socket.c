/******************************************************************************
*
 * Copyright by Transa Semi.
 *
 * File Name:    
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       V1.0
 * Author:        liangyu
 * Date:          2019-8-14
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  
*******************************************************************************
*/

#ifndef _FILENAME_H
#define _FILENAME_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/

#include "system_common.h"
#include "lwip/sockets.h"
#include "system_wifi.h"
#include "system.h"
#include "system_network.h"
#include "lwip/ip4_addr.h"
#include "system_wifi_def.h"
#include "lwip/netdb.h"



/****************************************************************************
* 	                                        Macros
****************************************************************************/


/******************************************************************************
*
 * Function: 
 * Description: 
 * Parameters: 
 *   Input:
 *
 *   Output:
 *
 * Returns: 
 *
 *
 * Others: 
 
*******************************************************************************
*/




/****************************************************************************
* 	                                        Types
****************************************************************************/

/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/
	TaskHandle_t socket_handle;
	char* hostip_str;
	uint16_t conn_tcp_port;
	int sizelen;
	int num;	
	char sendbuf[3000];
	int socket_type;
	int test_g_sock_client					= -1;

	TaskHandle_t recv_socket_handle;
	char* recv_hostip_str;
	uint16_t recv_conn_tcp_port;
	int recv_sizelen;
	int recv_num;	
	char recv_sendbuf[3000];
	int recv_socket_type;
	int recv_test_g_sock_client					= -1;


	static void socket_send(void *pvParameters)
	{
		
		struct ip_info if_ip;
		int timeout = 5000;
		struct sockaddr_in local_addr;	
		int ret;
		int socket_class;
		/* 等待wifi连接成功后再处理； */
		if(STA_STATUS_CONNECTED!= wifi_get_sta_status())
		{
			system_printf("----------TEST : WiFi Disconnect!!\n");
			vTaskDelete(NULL);
		}
		/* 等待获取ip后再处理；*/
		wifi_get_ip_info(STATION_IF,&if_ip);
		if(ip4_addr_isany_val(if_ip.ip))
		{
			system_printf("----------TEST : IP Is Any!!\n");
			vTaskDelete(NULL);
		}
		/* 初始化socket； */
		if(socket_type==SOCK_RAW)
			socket_class = SOCK_RAW;
		else if(socket_type==SOCK_DGRAM)
			socket_class = SOCK_DGRAM;
		else
			socket_class = SOCK_STREAM;
		test_g_sock_client = lwip_socket(AF_INET, socket_class, 0/*IPPROTO_TCP*/);
		if(test_g_sock_client==-1)
		{
			system_printf("----------TEST : Create Socket Fail!!\n");
			lwip_close(test_g_sock_client);
			test_g_sock_client = -1;
			vTaskDelete(NULL);
		}
		system_printf("----------TEST : Connect AP and IP Get!!\n");
		//int timeout = 1000;
		lwip_setsockopt(test_g_sock_client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
		lwip_setsockopt(test_g_sock_client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
				
		/* 调试直接使用本地服务器，在master ctrl task里初始化的内容： */

		local_addr.sin_addr.s_addr = inet_addr(hostip_str);
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(conn_tcp_port);
		/* 连接socket */

		ret = lwip_connect(test_g_sock_client,(struct sockaddr*)&local_addr,sizeof(local_addr));
		if(ret < 0)
		{
			system_printf("----------TEST : Connect Socket Fail!!%d!!Close Socket!!\n",ret);
			lwip_close(test_g_sock_client);
			test_g_sock_client = -1;
			vTaskDelete(NULL);
		}
		memset(sendbuf,48,3000);
		system_printf("----------TEST : Connect Socket Success.\n");	
		
		for(;num>0;num--)
		{
			ret = write(test_g_sock_client,sendbuf,sizelen);
		}
		system_printf("----------TEST : TEST Success.\n");
		lwip_close(test_g_sock_client);
		test_g_sock_client = -1;
		vTaskDelete(NULL);
	}

	static int test_socket_client_send(cmd_tbl_t *t, int argc, char *argv[])
	{
		if(argc != 6)
		{
			system_printf("The Number Of Argv Is Not 6");
			return CMD_RET_FAILURE;
		}
		system_printf("test_socket_client_send\n");
		hostip_str = argv[1];
		conn_tcp_port = (uint16_t)(atoi(argv[2]));
		sizelen = atoi(argv[3]);
		num = atoi(argv[4]);
		socket_type = atoi(argv[5]);
		if(xTaskCreate(socket_send, (const char *)"socket_send", 6000, NULL, 10, &socket_handle)!=pdTRUE)
			system_printf("TEST:ERROR!");
		return CMD_RET_SUCCESS;
	}

	SUBCMD(test,
    socket_client_send,
    test_socket_client_send,
    "",
    "");

	static void socket_recv(void *pvParameters)
	{
		
		struct ip_info if_ip;
		int timeout = 5000;
		struct sockaddr_in local_addr;	
		int ret;
		int socket_class;
		/* 等待wifi连接成功后再处理； */
		if(STA_STATUS_CONNECTED!= wifi_get_sta_status())
		{
			system_printf("----------TEST : WiFi Disconnect!!\n");
			vTaskDelete(NULL);
		}
		/* 等待获取ip后再处理；*/
		wifi_get_ip_info(STATION_IF,&if_ip);
		if(ip4_addr_isany_val(if_ip.ip))
		{
			system_printf("----------TEST : IP Is Any!!\n");
			vTaskDelete(NULL);
		}
		/* 初始化socket； */
		if(recv_socket_type==SOCK_RAW)
			socket_class = SOCK_RAW;
		else if(recv_socket_type==SOCK_DGRAM)
			socket_class = SOCK_DGRAM;
		else
			socket_class = SOCK_STREAM;
		recv_test_g_sock_client = lwip_socket(AF_INET, socket_class, 0/*IPPROTO_TCP*/);
		if(recv_test_g_sock_client==-1)
		{
			system_printf("----------TEST : Create Socket Fail!!\n");
			lwip_close(recv_test_g_sock_client);
			recv_test_g_sock_client = -1;
			vTaskDelete(NULL);
		}
		system_printf("----------TEST : Connect AP and IP Get!!\n");
		//int timeout = 1000;
		lwip_setsockopt(recv_test_g_sock_client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
		lwip_setsockopt(recv_test_g_sock_client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
				
		/* 调试直接使用本地服务器，在master ctrl task里初始化的内容： */

		local_addr.sin_addr.s_addr = inet_addr(recv_hostip_str);
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(recv_conn_tcp_port);
		/* 连接socket */

		ret = lwip_connect(recv_test_g_sock_client,(struct sockaddr*)&local_addr,sizeof(local_addr));
		if(ret < 0)
		{
			system_printf("----------TEST : Connect Socket Fail!!%d!!Close Socket!!\n",ret);
			lwip_close(recv_test_g_sock_client);
			recv_test_g_sock_client = -1;
			vTaskDelete(NULL);
		}
		system_printf("----------TEST : Connect Socket Success.\n");	
		
		for(;recv_num>0;recv_num--)
		{
			ret = read(recv_test_g_sock_client,recv_sendbuf,recv_sizelen);
		}
		system_printf("----------TEST : TEST Success.\n");
		lwip_close(recv_test_g_sock_client);
		recv_test_g_sock_client = -1;
		vTaskDelete(NULL);
	}

	static int test_socket_client_recv(cmd_tbl_t *t, int argc, char *argv[])
	{
		if(argc != 6)
		{
			system_printf("The Number Of Argv Is Not 6");
			return CMD_RET_FAILURE;
		}
		system_printf("test_socket_client_recv\n");
		recv_hostip_str = argv[1];
		recv_conn_tcp_port = (uint16_t)(atoi(argv[2]));
		recv_sizelen = atoi(argv[3]);
		recv_num = atoi(argv[4]);
		recv_socket_type = atoi(argv[5]);
		if(xTaskCreate(socket_recv, (const char *)"socket_recv", 6000, NULL, 10, &recv_socket_handle)!=pdTRUE)
			system_printf("TEST:ERROR!");
		return CMD_RET_SUCCESS;
	}

	SUBCMD(test,
    socket_client_recv,
    test_socket_client_recv,
    "",
    "");	
#endif/*_FILENAME_H*/






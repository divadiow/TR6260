#include "system.h"
#include "system_wifi.h"
#include "smartconfig.h"
#include "smartconfig_notify.h"
#include "lwip/sockets.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/semphr.h"
#include "FreeRTOS/queue.h"
#include "FreeRTOS/task.h"
#include "FreeRTOS/timers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Flag to indicate sending smartconfig notify or not. */
static bool g_sc_notify_send = false;

static int sc_notify_send_get_errno(int fd)
{
    int sock_errno = 0;
    u32_t optlen = sizeof(sock_errno);

    getsockopt(fd, SOL_SOCKET, SO_ERROR, &sock_errno, &optlen);

    return sock_errno;
}

static void sc_notify_send_task(void *pvParameters)
{
    sc_notify_t *msg = (sc_notify_t *)pvParameters;
    int remote_port = SC_NOTIFY_SERVER_PORT;
    struct sockaddr_in server_addr;
    socklen_t sin_size = sizeof(server_addr);
    int send_sock = -1;
    int optval = 1;
    int sendlen;
    uint8_t packet_count = 1;
    int err;
    int ret;

    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    server_addr.sin_port = htons(remote_port);
    wifi_get_mac_addr(STATION_IF, (uint8_t*)msg->mac);
    while (g_sc_notify_send) {
        /* Create UDP socket. */
        send_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (send_sock < 0) {
            system_printf("Creat udp socket failed\n");
            goto _end;	
        }

        setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &optval, sizeof(int));

        while (g_sc_notify_send) {
            /* Send smartconfig notify message every 100ms. */
            vTaskDelay(500 / portTICK_RATE_MS);
            system_printf("%02x %02x %02x %02x %02x %02x %02x\n",msg->random,msg->mac[0],msg->mac[1],msg->mac[2],msg->mac[3],msg->mac[4],msg->mac[5]);
            sendlen = sendto(send_sock, msg, SC_NOTIFY_MSG_LEN, 0, (struct sockaddr*) &server_addr, sin_size);
            if (sendlen > 0) {
                /* Totally send 30 message. */
                if (packet_count++ >= SC_NOTIFY_MAX_COUNT) {
                    goto _end;
                }
            }
            else {
                err = sc_notify_send_get_errno(send_sock);
                if (err == ENOMEM || err == EAGAIN) {
                    system_printf("send failed, errno %d\n", err);
                    continue;
                }
                system_printf("send failed, errno %d\n", err);
                goto _end;
            }
        }
        vTaskDelay((portTickType)(100 / portTICK_RATE_MS));
    }

_end:
    if (send_sock >= 0) {
        close(send_sock);
    }
    os_free(msg);
    vTaskDelete(NULL);
}

void sc_notify_send(sc_notify_t *param)
{
    sc_notify_t *msg = NULL;

    if (param == NULL) {
        system_printf("Smart config notify parameter error\n");
        return;
    }

    msg = os_malloc(sizeof(sc_notify_t));
    if (msg == NULL) {
        system_printf("Smart config notify parameter malloc fail");
        return;
    }
    memcpy(msg, param, sizeof(sc_notify_t));

    g_sc_notify_send = true;
    
    if (xTaskCreate(sc_notify_send_task, "sc_notify_send_task", SC_NOTIFY_TASK_STACK_SIZE, msg, SC_NOTIFY_TASK_PRIORITY, NULL) != pdPASS) {
        system_printf("Create sending smartconfig notify task fail");
        os_free(msg);
    }
}

void sc_notify_send_stop(void)
{
    g_sc_notify_send = false;
}

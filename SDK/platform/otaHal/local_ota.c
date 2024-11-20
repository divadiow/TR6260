/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "local_ota.h"
#include "FreeRTOS/FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "queue.h"
#include "projdefs.h"
#include "system_def.h"
#include "system_wifi_def.h"
#include "system_wifi.h"
#include "util_cmd.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "stdio.h"
#include "system_event.h"
#include "otaHal.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define OTA_PRINT                               system_printf
#define OTA_DEBUG                               system_printf
#define OTA_FIRM_CHECK_UNIT_SIZE                256
#define OTA_TASK_BUF_SIZE                       128
#define OTA_SERVER_PORT                         80
#define OTA_HTTP_RCV_BUFF_LEN                   2048
#define OTA_INVALID_SOCKET                      (-1)
#define IS_OTA_SOCKET_INVALID(socket)           (OTA_INVALID_SOCKET == socket)
#define OTA_RCV_TIMEO                           10000

/*ota config*/
#define CONFIG_LOCAL_OTA_RECV_PORT              5300
#define CONFIG_LOCAL_OTA_SEND_PORT              5301

#define CONFIG_LOCAL_OTA_MAX_PATH_SIZE          252
#define CONFIG_LOCAL_OTA_VERSION_NUM            6
#define CONFIG_LOCAL_OTA_VERSION_CMP_GREATER    1
#define CONFIG_LOCAL_OTA_VERSION_CMP_LESS       2
#define CONFIG_LOCAL_OTA_FIRM_INFO_LEN          4
#define CONFIG_LOCAL_OTA_UPDATE_JUDGE           5
#define CONFIG_LOCAL_OTA_TYPE_BC                0X01
#define CONFIG_LOCAL_OTA_TYPE_PUB               0X10
#define CONFIG_LOCAL_OTA_TYPE_ACTION            0X11
#define CONFIG_LOCAL_OTA_ACTION_SUCESS          0X00
#define CONFIG_LOCAL_OTA_ACTION_UPDATE          0X01
#define CONFIG_LOCAL_OTA_ACTION_ILLEGAL         0X02
#define CONFIG_LOCAL_OTA_ACTION_FAIL            0X03
#define LOCAL_OTA_FLASH_READ_SIZE               4096

#define OTA_DST_VERSION_KEY       "local_ota_verion"

typedef enum
{
    OTA_ERROR_NONE,                 // ota 没有错误
    OTA_ERROR_INIT_FAIL,            // ota hal 初始化错误
    OTA_ERROR_WRITE_FAIL,           // ota hal 写入错误
    OTA_ERROR_HTTP_START_FAIL,      // ota http 启动错误
    OTA_ERROR_SOCKET_SEND_FAIL,     // ota 套接口发送错误
    OTA_ERROR_SOCKET_RECEIVE_FAIL, 
    OTA_ERR_MAX
}OTA_ERROR_STATUS;

typedef enum
{
    OTA_DOWNLOAD_IDLE,              // 空闲 
    OTA_DOWNLOAD_ING,               // 下载中 
    OTA_DOWNLOAD_COMPLETE           // 下载完成 
}OTA_DOWNLOAD_STATUS;
/****************************************************************************
* Local Types
****************************************************************************/

typedef struct
{
    int         port;
    char*       host;                           // malloc动态申请内存，保存ota下载地址的的域名 
    char*       path;                           // 从ota upgrade_param的url里截取的字串 

    ip_addr_t   http_server_ip;                 // 解析host之后得到的http server的IP地址 
    int         socket;                         // 进行HTTP传输的socket 
    
    uint8_t     crlf_num;                       // http以crlf(\r\n)结束一行，连续的四个/r or /n代表了http header的结束，因为请求头和请求体之间隔了一行 

    char*       http_header_buff;               // malloc动态申请内存，保存从socket读取出来的HTTP的头 
    uint16_t    http_header_buff_byte_count;    // 记录HTTP头的长度 
    uint32_t    http_resp_content_length;       // HTTP中Content-Length字段，记录本次HTTP传输要发送的数据总量 
    uint32_t    rcvd_len;                       // 此次http连接已获取到的总的数据量 

    uint16_t    buff_len;                       // 设定每次从http socket获取数据的大小 
    uint8_t*    http_rcv_buff;                  // malloc动态申请内存，保存从http socket获取的数据 
}ota_to_httpserver_ctx_t;
typedef ota_to_httpserver_ctx_t* ota_to_httpserver_ctx;

typedef struct
{
    char                    url[512];               // 从at命令传入的URL地址
    uint8_t                 ota_hal_inited;         // 防止断点续传时，再次初始化ota 
    
    OTA_UPDATE_STATUS       update_status;          // OTA升级结果 
    OTA_DOWNLOAD_STATUS     download_status;        // OTA是否开始下载，下载是否完成 
    OTA_ERROR_STATUS        error_status;
    
    uint32_t                file_size;              // 首次发起HTTP下载时，server返回的HTTP响应Content-Length字段，记录了完整的版本的文件的长度 
    uint32_t                receive_len;            // ota升级过程中已获取数据总量 
    uint8_t                 is_first_http_header;   // 只有在第一次收到HTTP连接请求时，才设置rcvd_totabl_len，断点续传时不设置 
    
    ota_to_httpserver_ctx   ota2httpserver_context; // malloc动态申请内存，保存处理http下载流程的context 

    uint8_t ota_download_max_time;
}ota_instance_t;
typedef ota_instance_t* ota_instance;


typedef struct 
{
    uint8_t type;
    uint8_t len;
    char    *str;
    uint8_t srcver[CONFIG_LOCAL_OTA_VERSION_NUM];
    uint8_t destver[CONFIG_LOCAL_OTA_VERSION_NUM];
    uint32_t fm_len;
    uint32_t fm_crc;
}local_ota_recv_pack_t;

typedef struct 
{
    uint8_t type;
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t ver[CONFIG_LOCAL_OTA_VERSION_NUM];
    uint8_t action;
}local_ota_send_pack_t;

typedef struct 
{
    uint8_t version[CONFIG_LOCAL_OTA_VERSION_NUM];
    char path[CONFIG_LOCAL_OTA_MAX_PATH_SIZE];
    uint8_t src_ip[4];
    char dst_ip[16];
    uint8_t mac[6];
    struct sockaddr_in client_addr;	
}local_ota_private_t;

typedef struct
{
    union {
        local_ota_recv_pack_t recv_pack;
        local_ota_send_pack_t send_pack;
    };
}local_ota_message_t;

/****************************************************************************
 *Global Constants
 ****************************************************************************/
const uint8_t CRC_BYTE_TABLE[256] =
{
	0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
	0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
	0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
	0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
	0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
	0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
	0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
	0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
	0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
	0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
	0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
	0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
	0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
	0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
	0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
	0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
	0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
	0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
	0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
	0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
	0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
	0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
	0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
	0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
	0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
	0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
	0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
	0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
	0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
	0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
	0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
	0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

static char test_buf[] = "http://172.16.0.101/tuya_ota.bin";
extern unsigned char g_status_flag;
   
ota_to_httpserver_ctx g_ota_http_request_ctx = NULL;
ota_instance_t g_ota_instance;
static local_ota_cb_t local_ota_callback;
static local_ota_private_t private;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

uint8_t check_crc_byte(u8_t *pbuf, u16_t len)
{
	uint8_t crc = 0;
	while(len--) crc = CRC_BYTE_TABLE[crc ^ *pbuf++];
	return crc;
}

static ota_instance get_ota_instance(void)
{
    return &g_ota_instance;
}

static char *os_strcat(char *pszDest, const char *pszSrc)
{
    char *pszOrigDst = pszDest;

    while(*pszDest)
        pszDest++;
    while((*pszDest++ = *pszSrc++) != '\0');

    return pszOrigDst;
}
#define strcat os_strcat

static char* ota_strdup(const char* str)
{
      size_t len;
      char* copy;

      len = strlen(str) + 1;
      if (!(copy = (char*)os_malloc(len))) return 0;
      memcpy(copy,str,len);
      return copy;
}
static int ota_get_port_from_url(char* addr)
{
    int port = OTA_SERVER_PORT;
        
    if(addr == 0)
    {
        return port;
    }

    char *path_abs = strstr(addr, "http://");
    if(path_abs){
        path_abs = path_abs + strlen("http://");
    }
    path_abs = ota_strdup(path_abs); 

    char *port_index = strchr(path_abs, '/');
    if(port_index){
        *port_index = 0x00;
    }
    
    port_index = strchr(path_abs, ':');
    if(port_index)
    {
        port = atoi(port_index + 1);
    
    }

    OTA_DEBUG("get_port_from_url,port: %d,  path: %s\n", port, path_abs);
    
    os_free(path_abs);
    return port;

}

static char* ota_get_host_from_url(char* addr)
{
    int i = 0;
    int len  = 0;
    char* host = NULL;
    char* pAddr = NULL;
    
    if(addr == NULL){
        return NULL;
    }
    pAddr = strstr(addr, "http://");
    
    if(pAddr){
        pAddr = addr + strlen("http://");
    }else{
        pAddr = addr;
    }
    len = strlen(pAddr);
    
    for(i = 0; i < len; i++){
        if(pAddr[i] == '/' || pAddr[i] == ':'){
            break;
        }
    }
    
    host = os_malloc(i + 2 );
    memset(host, 0, i + 2);
    memcpy(host, pAddr, i);
    return host;

}
void ota_http_request_ctx_destroy()
{
    ota_to_httpserver_ctx_t *ota2httpserver_context = g_ota_http_request_ctx;
    
    if(NULL != ota2httpserver_context->host)
    {
        os_free(ota2httpserver_context->host);
        ota2httpserver_context->host = NULL;
        // OTA_DEBUG("free mem ota2httpserver_context->host\n");
    }

    if(NULL != ota2httpserver_context->http_header_buff)
    {
        os_free(ota2httpserver_context->http_header_buff);
        ota2httpserver_context->http_header_buff = NULL;
        // OTA_DEBUG("free mem ota2httpserver_context->http_header_buff\n");
    }

    if(NULL != ota2httpserver_context->http_rcv_buff)
    {
        os_free(ota2httpserver_context->http_rcv_buff);
        ota2httpserver_context->http_rcv_buff = NULL;
        // OTA_DEBUG("free mem ota2httpserver_context->http_rcv_buff\n");
    }
    
    if(OTA_INVALID_SOCKET != ota2httpserver_context->socket)
    {
        close(ota2httpserver_context->socket);
        ota2httpserver_context->socket = OTA_INVALID_SOCKET;
        // OTA_DEBUG("close ota2httpserver_context->socket\n");
    }

    if(NULL != ota2httpserver_context)
    {
        os_free(ota2httpserver_context);
        // OTA_DEBUG("free mem ota2httpserver_context\n");
    }

    OTA_DEBUG("ota httpp request ctx destroy \r\n");
}
/* http:192.168.1.100:8080/tr6260_ota.bin*/
/* 申请ota升级的http request内存 */
static sys_err_t ota_http_request_ctx_create()
{
    ota_instance ota_inst = get_ota_instance();;

    if(ota_inst->ota2httpserver_context)
    {
        ota_http_request_ctx_destroy();
        OTA_DEBUG("destroy ota inst-> ota2httpserver_context succ\n");
    }

    g_ota_http_request_ctx = (ota_to_httpserver_ctx)os_malloc(sizeof(ota_to_httpserver_ctx_t));
    
    if(NULL == g_ota_http_request_ctx)
    {
        OTA_DEBUG("create http request ctx fail\n");
        return SYS_ERR;
    }

    ota_inst->ota2httpserver_context = g_ota_http_request_ctx;
    
  	g_ota_http_request_ctx->port = ota_get_port_from_url(ota_inst->url);      /* 80 */
    g_ota_http_request_ctx->host = ota_get_host_from_url(ota_inst->url);      /* download.hismarttv.com */    /* malloc动态申请host的内存空间 */
	// g_ota_http_request_ctx->path = strstr(ota_inst->upgrde_param.url, g_ota_http_request_ctx->host) + strlen(g_ota_http_request_ctx->host);       /* /Content/WifiDeviceVersionFile/154217939399600372.rbl */
    g_ota_http_request_ctx->path = strstr(ota_inst->url, g_ota_http_request_ctx->host) + strlen(g_ota_http_request_ctx->host);
    g_ota_http_request_ctx->path = strchr(g_ota_http_request_ctx->path,'/');
    OTA_DEBUG("host:[%s] port:[%d] path:[%s]\r\n", g_ota_http_request_ctx->host, g_ota_http_request_ctx->port, g_ota_http_request_ctx->path);
    
	g_ota_http_request_ctx->crlf_num = 0;
    g_ota_http_request_ctx->http_header_buff = os_malloc(512);
    
    if(NULL == g_ota_http_request_ctx->http_header_buff)
    {
        OTA_DEBUG("http request ctx http_header_buff fail \r\n");
        return SYS_ERR;
    }

    g_ota_http_request_ctx->http_header_buff_byte_count = 0;
    g_ota_http_request_ctx->http_resp_content_length = 0;
    g_ota_http_request_ctx->rcvd_len = 0;

    g_ota_http_request_ctx->buff_len = OTA_HTTP_RCV_BUFF_LEN;
    g_ota_http_request_ctx->http_rcv_buff = os_malloc(g_ota_http_request_ctx->buff_len);
    // if(NULL != g_ota_http_request_ctx->http_rcv_buff)
    // {
    //     OTA_DEBUG("create http request ctx  http_header_buff succ,  len: %d\n", g_ota_http_request_ctx->buff_len);
    // }
    if(NULL == g_ota_http_request_ctx->http_rcv_buff)
    {
        OTA_DEBUG("create http request ctx  http_header_buff succ,  len: %d\n", g_ota_http_request_ctx->buff_len);
        return SYS_ERR;
    }

    return SYS_OK;
}
/* HTTP协议的请求行 */
/* GET /Content/WifiDeviceVersionFile/154217939399600372.rbl HTTP/1.1 */
static void ota_create_http_request_Line(char *req_data, const char *method, const char *resource)
{
    sprintf(req_data, "%s %s %s", method, resource, "HTTP/1.1\r\n");
}
/* HOST: download.hismarttv.com:8080 */
static void ota_create_http_request_header_Host(char *req_data, const char *host, const int port)
{
    os_strcat(req_data, "Host: ");
    strcat(req_data, host);
    if(port != 80)
    {
        char tmp[10];
        strcat(req_data, ":");
        sprintf(tmp, "%d", port);
        strcat(req_data, tmp);
    }
    strcat(req_data, "\r\n");
}
/* HTTP的TCP连接，保持长连接，不关闭 */
static void ota_create_http_request_header_Connection(char *req_data)
{
    strcat(req_data, "Connection: Keep-Alive\r\n");
}
/* 文件传输类型 */
static void ota_create_http_request_header_ContentType(char *req_data)
{
    strcat(req_data, "Content-Type: application/x-www-form-urlencoded\r\n");
}
/* Range: bytes=204800-307200/433769 */
/* HTTP利用Range实现断点续传，在socket断开之后，下次重连时，从断点继续传输剩下的ver文件 */
static void ota_create_http_request_header_Range(char *req_data, int receive_len)
{
    sprintf(req_data + strlen(req_data), "Range: bytes=%d-\r\n", receive_len);
}

/* HTTP协议的请求头 */
static void ota_create_http_request_header(char *req_data, const char *host, int port, OTA_DOWNLOAD_STATUS ota_status, int receive_len)
{
    ota_create_http_request_header_Host(req_data, host, port);
    ota_create_http_request_header_Connection(req_data);
    ota_create_http_request_header_ContentType(req_data);
    
    /* 说明之前请求过OTA升级，那么就断点续传 */
    if(ota_status == OTA_DOWNLOAD_ING)
    {
        ota_create_http_request_header_Range(req_data, receive_len);
    }
    
    strcat(req_data, "\r\n");
    
    OTA_DEBUG("\r\n ====================== \r\nrequest head send to server:\r\n%s\r\n======================\r\n", req_data);    
}

// 创建HTTP请求
static void ota_create_http_request(char *req_data, ota_instance_t* ota_inst)
{
    ota_create_http_request_Line(req_data, "GET", ota_inst->ota2httpserver_context->path);
    ota_create_http_request_header(req_data, ota_inst->ota2httpserver_context->host, ota_inst->ota2httpserver_context->port, ota_inst->download_status, ota_inst->receive_len);
}
static void ota_succ_proc()
{
    ota_instance ota_inst = get_ota_instance();
    
    OTA_DEBUG("ota download success \r\n");
    if(ota_inst->ota2httpserver_context)
    {
        ota_http_request_ctx_destroy();
        ota_inst->ota2httpserver_context = NULL;
        // OTA_DEBUG("download succ,  destroy http request ctx, free mem succ\n");
    }
    
    ota_inst->update_status = OTA_UPDATE_SUCCESS;
    // OTA_DEBUG("set update_status SUCCESS\n");
}
static void ota_instance_reset(void)
{
    ota_instance ota_inst = get_ota_instance();
    
    ota_inst->ota_hal_inited = 0;
    // ota_inst->has_cached_ota_cmd = 0;
    // ota_inst->upgrde_param.type  = OTA_TYPE_NUM;
    // memset(ota_inst->upgrde_param.version, 0, OTA_WIFI_VERSION_LEN);
    // ota_inst->upgrde_param.ver_len = 0;
    memset(ota_inst->url, 0, 512);

    ota_inst->update_status      = OTA_UPDATE_NONE;
    ota_inst->download_status = OTA_DOWNLOAD_IDLE;
    ota_inst->error_status        = OTA_ERROR_NONE;
    ota_inst->file_size       = 0;
    ota_inst->receive_len  = 0;
    ota_inst->is_first_http_header = 0;
    
    if(ota_inst->ota2httpserver_context)
    {
        ota_http_request_ctx_destroy();
        ota_inst->ota2httpserver_context = NULL;
        // OTA_DEBUG("ota reset, destroy http request ctx, free mem.\n");
    }
    
    OTA_DEBUG("ota instance reset succ\n");
}

// ota实例初始化
static void ota_instance_init(void)
{
    ota_instance ota_inst =get_ota_instance();

    ota_inst->ota_hal_inited = 0;
    // ota_inst->has_cached_ota_cmd = 0;
    // ota_inst->upgrde_param.type = OTA_TYPE_NUM;
    // memset(ota_inst->upgrde_param.version, 0, OTA_WIFI_VERSION_LEN);
    // ota_inst->upgrde_param.ver_len = 0;
    memset(ota_inst->url, 0, 512);

    ota_inst->update_status = OTA_UPDATE_NONE;
    ota_inst->download_status = OTA_DOWNLOAD_IDLE;
    ota_inst->error_status        = OTA_ERROR_NONE;

    ota_inst->file_size = 0;
    ota_inst->receive_len = 0;
    ota_inst->is_first_http_header = 0;
    
    ota_inst->ota2httpserver_context = NULL;

    ota_inst->ota_download_max_time = 5;

    OTA_DEBUG("ota instance init success \r\n");
}

// ota下载失败后的处理
void ota_fail_process()
{
    // OTA_DEBUG("ota fail proc:*******\n");

    ota_instance_reset();
    
    ota_instance ota_inst = get_ota_instance();
    ota_inst->update_status = OTA_UPDATE_FAIL;

    // OTA_DEBUG("set ota result: UPGRADE_FAIL\n");
}

// ota从http服务器接受的消息进行处理
#define UINT16_MAX_VALUE 65535
static sys_err_t ota_recv_data_handle(const uint8_t *data, int32_t len)
{
    if ((data == NULL) || (len <= 0) || (len >= UINT16_MAX_VALUE))
    {
        OTA_DEBUG("PARA err(len:%d)\n", len);
        return SYS_ERR;
    }
    
    ota_instance ota_inst = get_ota_instance();

    /* 第一次从HTTP server获取到版本文件 */
    if(OTA_DOWNLOAD_IDLE == ota_inst->download_status)
    {
        ota_inst->download_status = OTA_DOWNLOAD_ING;
        OTA_DEBUG("first get stream from http server \r\n");
    }
    
    ota_to_httpserver_ctx_t *ota2httpserver_context = ota_inst->ota2httpserver_context;
    if(NULL == ota2httpserver_context)
    {
        OTA_DEBUG("ota int http request ctx null\n");
        return SYS_ERR;
    }

    const uint8_t *data_start_pos = NULL;       /* 保存此次从socket获取的数据，去掉HTTP头之后的起始地址 */
    uint16_t data_len = 0;                      /* 保存此次从socket获取的数据, 去掉HTTP头之后的长度 */
    uint16_t http_header_len = 0;               /* 保存此次http下载，收到的http响应行和响应头的长度 */
    
    uint16_t i = 0;
    /* HTTP头以\r\n\r\n结束 */
    /* 处理接收报文中的HTTP header */
    if(ota2httpserver_context->crlf_num < 4)
    {
        /* 逐个字节的读取http接收到的数据 */
        for(i = 0; i < len; i++)
        {
            ota2httpserver_context->http_header_buff[ota2httpserver_context->http_header_buff_byte_count++] = data[i];
            ota2httpserver_context->http_header_buff[ota2httpserver_context->http_header_buff_byte_count] = '\0';

            /* http的响应头最后结束的标识是：\r\n\r\n，保证是连续的4个 */
            if(data[i] == '\r' || data[i] == '\n')
            {
                ota2httpserver_context->crlf_num++;
            }
            else
            {
                ota2httpserver_context->crlf_num = 0;
            }

            /* http的头读取完毕 */
            if(ota2httpserver_context->crlf_num == 4)
            {
                OTA_DEBUG("\r\n#####################################\r\n");
                OTA_DEBUG("recv http header len: %d, content is following:\r\n%s", 
                            ota2httpserver_context->http_header_buff_byte_count, ota2httpserver_context->http_header_buff);
                OTA_DEBUG("\r\n#####################################\r\n");

                /* Content-Length: 143\r\n */   /* 是否有 Content-Length:  字段，有的话读取 */
                char *content_length_string = strstr((char *)ota2httpserver_context->http_header_buff, "Content-Length: ");
                if(content_length_string)
                {   /* "143\r\n" */
                    char *content_length_value_string = content_length_string + strlen("Content-Length: "); 
                    char len_str[20] = {0};
                    char *p = strchr(content_length_value_string, '\r');
                    memcpy(len_str, content_length_value_string, p - content_length_value_string);
                    ota2httpserver_context->http_resp_content_length = atoi(len_str);

                    ota2httpserver_context->rcvd_len = 0;
                    
                    /* 由于利用的rcvd_total_len判断的断点续传和是否是下载完成，所以只有在第一次收到HTTP响应时才设置为0，断点续传时不能再重置为0 */
                    if(0 == ota_inst->is_first_http_header)
                    {
                        ota_inst->receive_len = 0;
                        ota_inst->is_first_http_header = 1;
                    }
                    
                    
                    OTA_DEBUG("http server response content length: %d \r\n", ota2httpserver_context->http_resp_content_length);
                }

                http_header_len = i + 1;
                // OTA_DEBUG("http_header_len: %d\n", http_header_len);
                
                /* 偏移掉HTTP的响应头 */
                data_start_pos = data + http_header_len;
                /* socket中去掉HTTP响应头的数据长度 */
                data_len = (unsigned short)(len - http_header_len);
                // OTA_DEBUG("rcvd first pkt from http server, take off http header, data_start_pos: 0x%x,  data len: %d\n\n", data_start_pos, data_len);           
                break;
            }
        }
    }

    /* 此次socket接收没有收全http的响应头 */
    if(ota2httpserver_context->crlf_num < 4)
    {
        OTA_DEBUG("this time socket rcv, http header is not complete, wait netx socket read \r\n");
        return SYS_OK;
    }

    /* 上方能保证，只有在读取到完整的HTTP头之后，才会执行下面的流程 */
    
    /* 根据下载过程中的首次HTTP响应中指定的Content-Length，记录版本文件的大小 */
    if(ota2httpserver_context->http_resp_content_length != 0)
    {
        if(0 == ota_inst->file_size)
        {   /* 第一次发起http请求，然后收到的响应Content-Length字段里，指明了本次下载的文件的大小 */
            ota_inst->file_size = ota2httpserver_context->http_resp_content_length;
            // ota_inst->upgrde_param.ver_len = ota2httpserver_context->http_resp_content_length;
            OTA_DEBUG("set ota_inst ver file_size: %d\n", ota_inst->file_size);
        }
                    
        /* http_header_len 为0时，说明此次socket read到的都是data */
        if(0 == http_header_len)
        {
            data_start_pos = data;
            data_len = len;
        }
        
        // OTA_DEBUG("this time recvd data len: %d\n", data_len);
        if(data_len == 0)
        {
            OTA_DEBUG("this time only rcvd http heder, return\n");
            return SYS_OK;
        }
        
        /* 写入flash */
        if(otaHal_write(data_start_pos, data_len) != 0)
        {
            ota_inst->error_status = OTA_ERROR_WRITE_FAIL;
            return SYS_ERR;
        }

        /* hal_write完成之后，再更新rcvd_total_len */
        ota2httpserver_context->rcvd_len += data_len;
        ota_inst->receive_len += data_len;

        int down = ota_inst->receive_len * 100 / ota_inst->file_size;
        static int last;
        if(down != last)
        {
            last = down;
            OTA_DEBUG("downloading %d%% \r\n",down);
        }
            
        // OTA_DEBUG("this time http request rcvd_len: %d\n", ota2httpserver_context->rcvd_len);
        // OTA_DEBUG("write_total_len: %d\n\n", ota_inst->rcvd_total_len);
    }

    /* 此次HTTP连接指定的数据已经获取完毕 */
    if(ota_inst->receive_len >= ota_inst->file_size)
    {
        ota_inst->download_status = OTA_DOWNLOAD_COMPLETE;
        OTA_DEBUG("\nhttp_rcvd_len: %d,  rcvd_total_len: %d,   download ver ok\n\n", ota2httpserver_context->rcvd_len, ota_inst->receive_len);
    }
    
    return SYS_OK;
}

// ota创建套接口，并连接http服务器
static sys_err_t ota_connect_http_server()
{
    ota_instance ota_inst = get_ota_instance();
    ota_to_httpserver_ctx ota2httpserver_context = ota_inst->ota2httpserver_context;
    
    
    ota2httpserver_context->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);            
    if(IS_OTA_SOCKET_INVALID(ota2httpserver_context->socket))
    {
        ota2httpserver_context->socket = OTA_INVALID_SOCKET;
        OTA_DEBUG("create ota http socket fail\n");
        return SYS_ERR;
    }
    
    OTA_DEBUG("create ota http socket succ, socket: %d\n", ota2httpserver_context->socket);

    int timeout = OTA_RCV_TIMEO;
    setsockopt(ota2httpserver_context->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    struct sockaddr_in http_server_addr;
    http_server_addr.sin_family = AF_INET;
    http_server_addr.sin_addr.s_addr = ota2httpserver_context->http_server_ip.addr;
    http_server_addr.sin_port = htons(ota2httpserver_context->port);

    /* 连接socket */
    if(connect(ota2httpserver_context->socket, (struct sockaddr*)&http_server_addr, sizeof(http_server_addr)) < 0)
    {
        close(ota2httpserver_context->socket);
        ota2httpserver_context->socket = OTA_INVALID_SOCKET;
        OTA_DEBUG("connect ota http socket  Fail!!  Close socket!!\n");
        
        return SYS_ERR;
    }

    return SYS_OK;
}

/* 从ota升级命令的url中解析出host port ip，动态创建http_ctx，create http server socket，connect socket */
static sys_err_t ota_connect_to_http_server(void)
{
    /* 收到ota命令后，解析host，port，动态创建处理HTTP协议需要的的ctx */
    if(SYS_OK != ota_http_request_ctx_create())
    {
        OTA_DEBUG("ota http request ctx create fail\r\n");
        return SYS_ERR;
    }
    
    ota_instance ota_inst = get_ota_instance();
    
    ota_to_httpserver_ctx_t *ota2httpserver_context = ota_inst->ota2httpserver_context;
    
    if(NULL == ota2httpserver_context)
    {
        OTA_DEBUG("create http request ctx fail!\n");
        return SYS_ERR;
    }

    /* 根据HTTP的域名获取HTTP服务器IP*/
    struct hostent *host_ip_info = gethostbyname(ota2httpserver_context->host);
    if (host_ip_info == NULL)
    {
        OTA_DEBUG("GetHostByName get http server ip Fail! Retry!!\n");
        return SYS_ERR;
    }
    memcpy(&ota2httpserver_context->http_server_ip, host_ip_info->h_addr_list[0], host_ip_info->h_length);
    OTA_DEBUG("get ota http server IP is:%s\n", inet_ntoa(ota2httpserver_context->http_server_ip));

    if(SYS_OK != ota_connect_http_server())
    {
        return SYS_ERR;
    }

    return SYS_OK;
}
static sys_err_t ota_downloading()
{
    ota_instance ota_inst = get_ota_instance();

    // 解析url，并连接Http server
    if(SYS_OK != ota_connect_to_http_server())
    {
        OTA_DEBUG("ota connect to http server fail\n");
        ota_inst->error_status = OTA_ERROR_HTTP_START_FAIL;
        return SYS_ERR;
    }

    ota_to_httpserver_ctx_t *ota2httpserver_context = ota_inst->ota2httpserver_context;

    // 创建http请求
    char req_data[2048] = {0};
    ota_create_http_request(req_data, ota_inst);
    
    if(send(ota2httpserver_context->socket, req_data, strlen(req_data), 0) < 0)
    {
        if(ota2httpserver_context->socket !=OTA_INVALID_SOCKET)
        {
            close(ota2httpserver_context->socket);
            OTA_DEBUG("close http socket because of socket send fail\n");
            /* send失败，说明网络不好，close socket，设置err_code为send_fail，然后返回，在外层进行断点续传处理 */
            ota_inst->error_status = OTA_ERROR_SOCKET_SEND_FAIL;
        }

        OTA_DEBUG("ota send http request fail!\n");
        
        return SYS_ERR;
    }

    // OTA_DEBUG("ota send http request succ\n");
    vTaskDelay(pdMS_TO_TICKS(20));
#if 1
    while(1)
    {
        int32_t ret = recv(ota2httpserver_context->socket, ota2httpserver_context->http_rcv_buff, ota2httpserver_context->buff_len, 0);
         OTA_DEBUG("-----------------each socket recv ret len: %d from socket\n", ret);

        /* socket接收数据失败，认为不是真的失败，需要断点续传，设置err_code，然后在外层断点续传处理 */
        if((0 == ret) || (-1 == ret))
        {
            if(ota2httpserver_context->socket != OTA_INVALID_SOCKET)
            {
                close(ota2httpserver_context->socket);
                OTA_DEBUG("close http socket because of socket recv err\n");
            }
            ota_inst->error_status = OTA_ERROR_SOCKET_RECEIVE_FAIL;
            return SYS_ERR;
        }

        // 接收的数据处理
        if(SYS_OK != ota_recv_data_handle(ota2httpserver_context->http_rcv_buff, ret))
        {
            OTA_DEBUG("ota recv data handle fail \r\n");
            return SYS_ERR;
        }

        /*============================================
         =               DBG_CODE start              =
         =============================================*/
        /* 断开与HTTP服务器的socket连接，人为构造OTA的断点续传场景 */
        static uint8_t dbg_ota_breakpoint_continue_flag = 0;
        if (dbg_ota_breakpoint_continue_flag == 1)
        {
            if (ota_inst->receive_len >= 7000)
            {
                if(ota2httpserver_context->socket != OTA_INVALID_SOCKET)
                {
                    close(ota2httpserver_context->socket);
                    OTA_DEBUG("close http socket because of dbg\n");
                }      
                
                ota_inst->error_status = OTA_ERROR_SOCKET_SEND_FAIL;
                dbg_ota_breakpoint_continue_flag = 0;
                
                return SYS_ERR;
            }
        }
        /*============================================
         =               DBG_CODE end                =
         =============================================*/

        if(OTA_DOWNLOAD_COMPLETE == ota_inst->download_status)
        {
            ota_succ_proc();         
            break;
        }
    }
#endif 
    return SYS_OK;
}

sys_err_t ota_downloatd_pre()
{
    ota_instance ota_inst = get_ota_instance();


        if(otaHal_init() != 0)
        {
            OTA_DEBUG("ota hal_init fail\n");
            ota_inst->error_status = OTA_ERROR_INIT_FAIL;
            return SYS_ERR;
        }
        OTA_DEBUG("ota hal_init success \r\n");
        
        // ota_inst->ota_hal_inited = 1;
    // }
    
    /* 下载版本 */
    if(SYS_OK != ota_downloading())
    {
        OTA_DEBUG("ota down ver fail\n");
        return SYS_ERR;
    }

	return SYS_OK;
}


// 根据获取的Url，开始ota
static sys_err_t ota_circle_download(void)
{
    ota_instance ota_inst = get_ota_instance();

    if(SYS_OK != ota_downloatd_pre())
    {
            ota_fail_process();
            return SYS_ERR;
    }
    OTA_DEBUG("ota update ok \r\n");
    return SYS_OK;
}
static int local_ota_extract_ip(char * src, char* dest)
{
    char * ptr1;
    char * ptr2;
    int len;
    ptr1 = strstr(src, "http://");
    ptr1 += 7;
    ptr2 = strstr(ptr1, "/");
    len = ptr2- ptr1;
    memcpy(dest, ptr1, len);
    dest[len] = 0;
    return 0;
}

static size_t  local_ota_message_outgoing(uint8_t *buf,
         local_ota_message_t* message)
{
    int i = 0;
    int len;
    buf[i] = message->send_pack.type;
    i++;
    memcpy(&buf[i], message->send_pack.mac, 6);
    i += 6;
    memcpy(&buf[i], message->send_pack.ip, 4);
    i += 4;
    memcpy(&buf[i], &message->send_pack.ver, 6);
    i += 6;
    buf[i] = message->send_pack.action;
    i++;
    buf[i] = check_crc_byte(buf, i);
    i++;
    len = i;
    return i;

}
extern uint32_t crc32(uint32_t crc, const void *buf, size_t size);
static int local_ote_firmware_check(uint32_t crc_data_len, uint32_t crc)
{    
    uint32_t crctemp = 0;
    uint32_t addr, partion_len;
    static uint32_t last_len, last_crc;
    size_t len = crc_data_len, size;
    static int ret;

    if(crc == 0) {
        return 0;
    }
    
    if(last_len == crc_data_len) {
        if(last_crc == crc) {
            return ret;
        } else {
            last_crc = crc;
        }
        
    } else {
        last_len = crc_data_len;
        last_crc = crc; 
    }
	unsigned char *  buf = (unsigned char *)os_calloc(1, LOCAL_OTA_FLASH_READ_SIZE);

    if(buf == NULL) {
        OTA_DEBUG("no mem left\n");
        ret = -1;
        return -1;
    }
    if (partion_info_get(PARTION_NAME_CPU1, (unsigned int *)&addr, (unsigned int *)&partion_len) != 0) {
        OTA_DEBUG("can not get %s info\n", PARTION_NAME_DATA_OTA);
		os_free(buf);
        return -1;
    }
    while(len > 0) {
        memset(buf, 0, LOCAL_OTA_FLASH_READ_SIZE);
        size = (len > LOCAL_OTA_FLASH_READ_SIZE) ? 
            LOCAL_OTA_FLASH_READ_SIZE : len;
        ef_port_read(addr + crc_data_len - len,  (uint32_t *)buf,  LOCAL_OTA_FLASH_READ_SIZE);
        crctemp = ef_calc_crc32(crctemp, buf, size);
        len -= size;
    }
    os_free(buf);
    if(crctemp == crc) {
        ret = 0;
        return 0;
    } else {
        ret = -1;
        return -1;
    }
    return 0;
}

static size_t  local_ota_message_incoming(uint8_t *buf,
         local_ota_message_t* message,  
         uint8_t len)
{
    uint8_t crc;
    int i = -1, j = 0;
    crc = check_crc_byte(buf, len - 1);
    if(crc == buf[len - 1]) {
		i = 0;
        message->recv_pack.type = buf[i];
        i++;
        if(message->recv_pack.type == 
                CONFIG_LOCAL_OTA_TYPE_ACTION) {
            for(j = 0; j < 6; j++) {
                if(private.mac[j] != buf[i + j]) {
                            
                    return 0;
                }
            }
            if(local_ota_callback.ota_action_cb
                    != NULL) {
                local_ota_callback.ota_action_cb();
            }
            return 0;
        }

        //OTA_DEBUG("type %d\r\n", message->recv_pack.type);
        message->recv_pack.len  = buf[i];
        if(message->recv_pack.str != NULL) {
            os_free(message->recv_pack.str);
        }
        i++;
        message->recv_pack.str = (char*)os_malloc(message->recv_pack.len + 1);
        memcpy(message->recv_pack.str, &buf[i], message->recv_pack.len );
        message->recv_pack.str[message->recv_pack.len] = 0;
        local_ota_extract_ip(message->recv_pack.str, private.dst_ip);

        //OTA_DEBUG("path %s\r\n", message->recv_pack.str);
        i += message->recv_pack.len;

        memcpy(message->recv_pack.srcver, &buf[i], CONFIG_LOCAL_OTA_VERSION_NUM);
        i += CONFIG_LOCAL_OTA_VERSION_NUM;
        
        memcpy(message->recv_pack.destver, &buf[i],
                CONFIG_LOCAL_OTA_VERSION_NUM);
        i += CONFIG_LOCAL_OTA_VERSION_NUM;
        memcpy(&message->recv_pack.fm_len, &buf[i],
            CONFIG_LOCAL_OTA_FIRM_INFO_LEN);
        i += CONFIG_LOCAL_OTA_FIRM_INFO_LEN;
        memcpy(&message->recv_pack.fm_crc, &buf[i],
            CONFIG_LOCAL_OTA_FIRM_INFO_LEN);
        i += CONFIG_LOCAL_OTA_FIRM_INFO_LEN;

    } else {
        OTA_DEBUG("crc error\r\n");
    }
    return i;
}


static size_t  local_ota_test_outging(uint8_t* *ptr)
{
    uint8_t crc;
    uint8_t len;
    int i;
    uint8_t version[CONFIG_LOCAL_OTA_VERSION_NUM];
    uint8_t *buf;
    len = 1 + 1 + strlen(test_buf) + 6 + 6 + 4 + 4 + 1;
    buf = os_malloc(len);
    *ptr = buf;

    buf[0] = 0x01;
    buf[1] = strlen(test_buf);
    memcpy(&buf[2], test_buf, strlen(test_buf));
    version[0] = 0x00;
    version[1] = 0x00;
    version[2] = 0x00;
    version[3] = 0x00;
    version[4] = 0x00;
    version[5] = 0x01;
    memcpy(&buf[len - 15],&version, 6);
    version[3] = 0x01;

    memcpy(&buf[len - 21], &version, 6);
    
    memcpy(&buf[len - 5],&version, 4);
    

    memcpy(&buf[len - 9], &version, 4);
    buf[len - 1] = check_crc_byte(buf, len - 1);
    return len;
}

extern void wifi_hanlde_sta_connect_event_nosave(system_event_t *event);
extern void wifi_handle_sta_connect_event(system_event_t *event);
int local_ota_connect_wifi(void)
{
    wifi_config_u sta_cfg;
    struct ip_info if_ip;
    int wifi_status = 0;
    int connect_cnt = 0;
    wifi_sniffer_stop();
    memset(&sta_cfg, 0, sizeof(sta_cfg));
    if (CONFIG_LOCAL_OTA_PASSWD  && strlen(CONFIG_LOCAL_OTA_PASSWD ) >= 5) {
        system_printf("set pwd [%s]\n", CONFIG_LOCAL_OTA_PASSWD );
        strlcpy(sta_cfg.sta.password, CONFIG_LOCAL_OTA_PASSWD , WIFI_PWD_MAX_LEN);
    }

    strlcpy((char *)sta_cfg.sta.ssid, CONFIG_LOCAL_OTA_SSID , WIFI_SSID_MAX_LEN);

    sys_event_reset_wifi_handlers(SYSTEM_EVENT_STA_CONNECTED, (system_event_handler_t)wifi_hanlde_sta_connect_event_nosave);
    sys_event_reset_wifi_handlers(SYSTEM_EVENT_STA_DISCONNECTED, (system_event_handler_t)wifi_hanlde_sta_connect_event_nosave);
    if (SYS_OK == wifi_start_station(&sta_cfg)) {
        //g_status_flag = 1;
    } else {
        sys_event_reset_wifi_handlers(SYSTEM_EVENT_STA_CONNECTED, (system_event_handler_t)wifi_handle_sta_connect_event);
        sys_event_reset_wifi_handlers(SYSTEM_EVENT_STA_DISCONNECTED, (system_event_handler_t)wifi_handle_sta_connect_event);
        return -1;
    }

    wifi_status = wifi_get_status(STATION_IF); 
    wifi_get_ip_info(STATION_IF,&if_ip);
    
    while(((wifi_status !=  STA_STATUS_CONNECTED) ||
             ip4_addr_isany_val(if_ip.ip)) && (connect_cnt != CONFIG_LOCAL_OTA_CONNET_TIMEOUT)) {
       wifi_status = wifi_get_status(STATION_IF); 
       wifi_get_ip_info(STATION_IF,&if_ip);
       vTaskDelay(pdMS_TO_TICKS(1000));
       connect_cnt++;
    }
             
    sys_event_reset_wifi_handlers(SYSTEM_EVENT_STA_CONNECTED, (system_event_handler_t)wifi_handle_sta_connect_event);
    sys_event_reset_wifi_handlers(SYSTEM_EVENT_STA_DISCONNECTED, (system_event_handler_t)wifi_handle_sta_connect_event);

    if(connect_cnt == CONFIG_LOCAL_OTA_CONNET_TIMEOUT) {
        return -1;
    } else {
        return 0;
    }
}

static int local_ota_recv_sock_init(void)
{
    struct sockaddr_in server_addr;	
	int recv_socket		= -1;
    int ret;  
    ip_addr_t addr;
    
    
    if(SYS_OK != wifi_get_ip_addr(0, (unsigned int *)&(addr.addr)))
    {
        return -1;
    }

	recv_socket = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if(recv_socket < 0) {
        system_printf("send task error\n");
    }
    //get myself ip addr/mac
    memcpy(private.src_ip, &addr.addr, 4);
    //system_printf("src addr %d %d %d %d\r\n", private.src_ip[0], 
      //      private.src_ip[1],private.src_ip[2],private.src_ip[3]);
    //strlcpy(private.src_ip, inet_ntoa(addr), sizeof(private.src_ip));
    wifi_get_mac_addr(0, private.mac);
    server_addr.sin_addr.s_addr = addr.addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(CONFIG_LOCAL_OTA_RECV_PORT);
    ret = bind(recv_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret <0) {
        system_printf("can not bind\r\n");   
        return ret;
    }
    return recv_socket;
}
static int local_ota_version_compare(uint8_t * src,
        uint8_t *dest)
{
    int i;
    for(i = 0; i < CONFIG_LOCAL_OTA_VERSION_NUM; i++) {
        if(src[CONFIG_LOCAL_OTA_VERSION_NUM - i - 1] 
                != dest[CONFIG_LOCAL_OTA_VERSION_NUM - i - 1]) {
            break;
        }
    }

    if(i == CONFIG_LOCAL_OTA_VERSION_NUM) {
        return 0;
    } else {
        if(src[CONFIG_LOCAL_OTA_VERSION_NUM - i - 1] 
                > dest[CONFIG_LOCAL_OTA_VERSION_NUM - i - 1]) {
            return CONFIG_LOCAL_OTA_VERSION_CMP_GREATER;
        } else {
            return CONFIG_LOCAL_OTA_VERSION_CMP_LESS;
        }
    }
}



static int local_ota_send_sock_init(char * addr)
{
    static int send_socket = 0;
    if(send_socket <= 0) {
        send_socket = lwip_socket(AF_INET, SOCK_DGRAM, 0);
        if(send_socket < 0) {
            system_printf("send task error\n");
            return -1;
        }
    } 
    private.client_addr.sin_addr.s_addr = inet_addr(addr);
    private.client_addr.sin_family = AF_INET;
    private.client_addr.sin_port = htons(CONFIG_LOCAL_OTA_SEND_PORT);

    return send_socket;
}


void local_ota_set_outgoing_message(local_ota_private_t *priv, 
        local_ota_message_t* message)
{
    memcpy( message->send_pack.ip, priv->src_ip, 4);
    memcpy( message->send_pack.mac, priv->mac, 6);
    memcpy( message->send_pack.ver, priv->version, 6);
}

int local_ota_cb_register(local_ota_cb_t * cb)
{
    local_ota_callback.ota_success_cb = cb->ota_success_cb;
    local_ota_callback.ota_fail_cb    = cb->ota_fail_cb;
    local_ota_callback.ota_action_cb    = cb->ota_action_cb;
    return 0;
}


int local_ota_version_info_register(char * version)
{
    int i,j = 0,len;
    uint8_t buf[CONFIG_LOCAL_OTA_VERSION_NUM];
    len = strlen(version);
    for(i = 0; i <
        CONFIG_LOCAL_OTA_VERSION_NUM;i++) {
        private.version[i] = 0;
        buf[i] = 0;
    }

    for(i = 0; i < len; i++) {
        if(version[i] == '.') {
            j += 1;
            continue;
        }
        if((version[i] < 0x30) || (version[i] > 0x39)) {
            return -1;
        }
        buf[j] = buf[j] * 10 + (version[i] - 48);
    }
    if ((j == 0) || (j > 3)) {
        return -1;
    }

    j += 1;
    for(i = 0; i < CONFIG_LOCAL_OTA_VERSION_NUM; i++) {
       private.version[CONFIG_LOCAL_OTA_VERSION_NUM - i - 1] = buf[i];  
    }
    
    return 0;

}

int local_ota_scan_wifi(void)
{
    uint8_t ssid_buf[32];
    int ap_num, retry;
    int find_ssid = 0;
    
    memset((void *)ssid_buf, 0, 32);
    memcpy(ssid_buf, CONFIG_LOCAL_OTA_SSID, strlen(CONFIG_LOCAL_OTA_SSID));
    wifi_scan_config_t scan_cfg;
    memset((void *)&scan_cfg, 0, sizeof(wifi_scan_config_t));
    scan_cfg.ssid = ssid_buf;
    scan_cfg.channel = CONFIG_LOCAL_OTA_CHANNEL;

    for(retry = 0; retry < 2; retry++)
    {
        if(0 != wifi_scan_start(1, &scan_cfg)) {
            system_printf("can not scan aim wifi\r\n");
            continue;
        }

        ap_num = wpa_get_scan_num();
        if(ap_num == 0)
        {
            system_printf("ap num is 0\r\n");
            continue;
        }
        
        system_printf("scaned_num; %d\n", ap_num);
        wifi_info_t ap_info;
        memset((void *)&ap_info, 0, sizeof(wifi_info_t));
        wifi_get_scan_result(0, &ap_info);
        
        if(0 != strcmp((char *)ap_info.ssid, (char *)ssid_buf))
        {
            system_printf("scaned ap, but not assigned ssid\n");
            continue;
        }

        find_ssid = 1;
        break;
    }

    if(find_ssid == 0)
    {
        return -1;
    }
    
    return 0;
}

int local_ota_start(void)
{
    int ret,recv_sock,send_sock,i;
    uint8_t buf[OTA_TASK_BUF_SIZE];
    uint8_t *ptr = NULL;
    bool cb_flag = false;
    int update_count = 0;
    local_ota_message_t recv_message;
    local_ota_message_t send_message;
    local_ota_reason_t ota_reason;
    recv_message.recv_pack.str = NULL;
   /* 
    ret = local_ota_test_outging(&ptr);
    for(i = 0; i < ret; i++) {
        system_printf("%x ", ptr[i]);
    }
    system_printf("\r\n");
    */
    recv_sock = local_ota_recv_sock_init();
    
    ota_instance_init();
    
    ota_instance ota_inst = get_ota_instance();
    
    //send_sock = local_ota_send_sock_init();
    
    system_printf("@@@@@@@@the version %d %d %d %d %d %d\r\n", 
            private.version[5], private.version[4],
            private.version[3], private.version[2],
            private.version[1], private.version[0]);

    while(1) {
        
        ret = recv(recv_sock, buf, OTA_TASK_BUF_SIZE,0);
		if (ret < 0)
		{
			continue;
		}
        ret = local_ota_message_incoming(buf, &recv_message, ret);     
        if(ret < 0) {
            continue;
        }

        if(recv_message.recv_pack.type ==
                CONFIG_LOCAL_OTA_TYPE_ACTION) {
            continue;
        }

        local_ota_set_outgoing_message(&private, &send_message);

        if (local_ota_version_compare(recv_message.recv_pack.destver, private.version) == 0)
        {
            send_message.send_pack.action = CONFIG_LOCAL_OTA_ACTION_SUCESS;
        } else if (local_ota_version_compare(recv_message.recv_pack.srcver, private.version) == 0) 
        {
            if(local_ota_version_compare(private.version, 
                recv_message.recv_pack.destver)
                 == CONFIG_LOCAL_OTA_VERSION_CMP_LESS) {
                if(local_ote_firmware_check(recv_message.recv_pack.fm_len, 
                    recv_message.recv_pack.fm_crc) == 0) {
                    send_message.send_pack.action =
                        CONFIG_LOCAL_OTA_ACTION_UPDATE;
                        update_count++;

                } else {
                    send_message.send_pack.action =
                        CONFIG_LOCAL_OTA_ACTION_ILLEGAL;
                    
                }
            } else {
                    send_message.send_pack.action =
                        CONFIG_LOCAL_OTA_ACTION_ILLEGAL;
            }
        } else 
        {

            system_printf("action illegal\r\n");
            send_message.send_pack.action = CONFIG_LOCAL_OTA_ACTION_ILLEGAL;
        }

        switch(send_message.send_pack.action) {
        case CONFIG_LOCAL_OTA_ACTION_SUCESS:
            if((local_ota_callback.ota_success_cb != NULL)
                    && (cb_flag != true))
            {
                cb_flag = true;
                local_ota_callback.ota_success_cb();
            }
        
            break;
        case CONFIG_LOCAL_OTA_ACTION_ILLEGAL:
            if((local_ota_callback.ota_fail_cb != NULL)
                    && (cb_flag != true))
            {
                cb_flag = true;
                local_ota_callback.ota_fail_cb();
            }
            break;
        default:
            break;
        }

        send_message.send_pack.type = CONFIG_LOCAL_OTA_TYPE_PUB;
        send_sock = local_ota_send_sock_init(private.dst_ip);
        ret = local_ota_message_outgoing(buf, &send_message);   
        
        if (ret > 0) {
            sendto(send_sock, buf, ret, 0, 
                            (struct sockaddr *)&private.client_addr, sizeof(private.client_addr));    
        }

        if((send_message.send_pack.action == CONFIG_LOCAL_OTA_ACTION_UPDATE)
                && (update_count == CONFIG_LOCAL_OTA_UPDATE_JUDGE)) {

            memcpy(ota_inst->url, recv_message.recv_pack.str, recv_message.recv_pack.len);
            ota_circle_download();
            if(OTA_UPDATE_SUCCESS == ota_inst->update_status)
            {
                otaHal_done();
            }
            else
            {
                if(local_ota_callback.ota_fail_cb != NULL) {
                    local_ota_callback.ota_fail_cb();
                }
                send_message.send_pack.action = CONFIG_LOCAL_OTA_ACTION_FAIL;
                ret = local_ota_message_outgoing(buf, &send_message);   
                if (ret > 0) {
                    sendto(send_sock, buf, ret, 0, 
                                    (struct sockaddr *)&private.client_addr, sizeof(private.client_addr));    
                }
            }            
        }        
    }
 
    return 0;
    
}





#include "system.h"
#include "system_wifi.h"
#include "smartconfig.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/semphr.h"
#include "FreeRTOS/queue.h"
#include "FreeRTOS/task.h"
#include "FreeRTOS/timers.h"
#include "zconfig_ieee80211.h"
#include "awss_aplist.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PASSWORD_MAX_LEN    64
#define SSID_MAX_LEN        32
#define MAX_CHANNELS 14
#define CHANNEL_TIME_FREQ 200   //500ms
#define CHANNEL_LOCK_TIME 3000   //15000ms
#define DEBUG 0

#define sc_log system_printf
#define sc_error system_printf

static void wifiSnifferCallback(void *buf, int len, wifi_promiscuous_pkt_type_t type);

typedef struct sc_head
{
    unsigned char dummyap[26];
    unsigned char dummy[32];
} sc_head_t;


#define USR_DATA_BUFF_MAX_SIZE    (PASSWORD_MAX_LEN + 1 + SSID_MAX_LEN)
typedef enum
{
    SC_STATE_STOPED = 0,
    SC_STATE_IDLE,
    SC_STATE_SRC_LOCKED,
    SC_STATE_MAGIC_CODE_COMPLETE,
    SC_STATE_PREFIX_CODE_COMPLETE,
    SC_STATE_COMPLETE
} SC_STATE;

#define MAX_PTYPE           4
#define MULTI_TYPE_TODS     0
#define MULTI_TYPE_FRDS     1
#define BRODA_TYPE_TODS     2
#define BRODA_TYPE_FRDS     3
#define MAX_GUIDE_RECORD    4
typedef struct
{
    unsigned short  length_record[MAX_PTYPE][MAX_GUIDE_RECORD + 1];
}guide_code_record;

#define MAX_MAGIC_CODE_RECORD    5
typedef struct
{
    unsigned short record[MAX_PTYPE][MAX_MAGIC_CODE_RECORD + 1];
}magic_code_record;

#define MAX_PREFIX_CODE_RECORD    4
typedef struct
{
    unsigned short record[MAX_PTYPE][MAX_PREFIX_CODE_RECORD + 1];
}prefix_code_record;

#define MAX_SEQ_CODE_RECORD    6
typedef struct
{
    unsigned short record[MAX_PTYPE][MAX_SEQ_CODE_RECORD + 1];
}seq_code_record;

union sc_data{
        guide_code_record guide_code;
        magic_code_record magic_code;
        prefix_code_record prefix_code;
        seq_code_record  seq_code;
};

typedef struct
{
    char pwd[ZC_MAX_PASSWD_LEN];                        
    char ssid[ZC_MAX_SSID_LEN];
    unsigned char pswd_len;
    unsigned char ssid_len;
    unsigned char random_num;
    unsigned char ssid_crc;
    unsigned char usr_data[USR_DATA_BUFF_MAX_SIZE];
    SC_STATE sc_state;
    unsigned char src_mac[6];
    unsigned char need_seq;
    unsigned char base_len[MAX_PTYPE];
    unsigned char total_len;
    unsigned char pswd_lencrc;
    unsigned char recv_len;
    unsigned short seq_success_map;
    unsigned short seq_success_map_cmp;
    union sc_data data;
}sc_local_context;

typedef struct _sc_config
{
    sc_callback_t callback;
    unsigned int timeout_s;
    TimerHandle_t channel_timer;
    // TimerHandle_t lock_chn_timer;
    TimerHandle_t task_timer;
}sc_config;

const char sc_vers[] = "V1.0";
static sc_head_t sc_head = {{0},{0}};
static sc_local_context sc_context;
static sc_config sc_cf;
static TaskHandle_t sc_task_handle = NULL;
static QueueHandle_t g_event_queue = NULL;
static sc_result_t sc_result;

int g_current_channel = 0;
int g_round = 0;
//crc8
unsigned char calcrc_1byte(unsigned char abyte)    
{    
    unsigned char i,crc_1byte;     
    crc_1byte=0;                
    for(i = 0; i < 8; i++)    
    {    
        if(((crc_1byte^abyte)&0x01))    
        {    

            crc_1byte^=0x18;     
            crc_1byte>>=1;    
            crc_1byte|=0x80;    
        }          
        else    
        {
            crc_1byte>>=1; 
        }
        abyte>>=1;          
    }   
    return crc_1byte;   
}  


unsigned char calcrc_bytes(unsigned char *p,unsigned int num_of_bytes)  
{  
    unsigned char crc=0;  
    while(num_of_bytes--) 
    {  
        crc=calcrc_1byte(crc^*p++);  
    }  
    return crc;   
} 

const char* smartconfig_version(void)
{
    return sc_vers;
}

static int sc_memcmp(unsigned char *str1, unsigned char *str2, size_t n)
{
    if(str1 == NULL || str2 == NULL || n == 0){
        return -1;
    }
	const unsigned char *p1 = str1, *p2 = str2;

    if (n == 0)
            return 0;

    while (*p1 == *p2) {
            p1++;
            p2++;
            n--;
            if (n == 0)
                return 0;
    }

    return *p1 - *p2;
}

static void resest_sc_data()
{
    memset(&sc_context.data, 0, sizeof(union sc_data));
}

static int sc_get_result(sc_result_t* result)
{
    system_printf("sc_get_result sc_context ssid %s\n",sc_context.ssid);
    memcpy(result, &sc_context, sizeof(sc_result_t));
    system_printf("2 sc_get_result result ssid %s %d\n",result->ssid,sizeof(sc_result_t));
    return 0;
}
static int sc_change_channel(void)
{
    memset(&sc_head, 0, sizeof(sc_head_t));
    resest_sc_data();
    return 0;
}


static int sn_check(uint16_t sn, int type)
{
    static uint16_t last_sn[MAX_PTYPE];
    // sc_log("type %d last sn %d\n",type,last_sn[type] );
    if(sn != last_sn[type] || sn == 0){
        last_sn[type] = sn;
        return 1;
    } else {
        return 0;
    }
}

static int find_ap_in_aplist(const uint8_t * ap_mac, struct ap_info * t_ap)
{
    uint8_t apmac[6];
    struct ap_info * target_ap = NULL;
    system_printf("%02x %02x %02x",ap_mac[0],ap_mac[1],ap_mac[2]);

    memcpy(apmac, ap_mac, 6);
    target_ap = zconfig_get_apinfo(apmac);
    if(target_ap != NULL){
        system_printf("target ap ssid %s channel %d",target_ap->ssid,target_ap->channel);
        xTimerStop(sc_cf.channel_timer,0);
        g_round = 0;
        sc_change_channel();
        wifi_rf_set_channel(target_ap->channel);
        wifi_set_promiscuous(false);
        wifi_promiscuous_filter_t filter = {0};
        filter.filter_mask = WIFI_PROMIS_FILTER_MASK_DATA;
        wifi_set_promiscuous_filter(&filter);
        wifi_set_promiscuous_rx_cb(wifiSnifferCallback);
        wifi_set_promiscuous(true);
        sc_context.sc_state = SC_STATE_SRC_LOCKED;
        memcpy(t_ap,target_ap,sizeof(struct ap_info));
        // resest_sc_data();
    }

    return 0;
}

/* return 1 is valid 
 * return 0 is invalid
 * */
int sc_filter(const unsigned char *frame, int size, int *type)
{
    int isvalid = 0;
    static bool qos_flag = false;
    struct ap_info  target_ap = {0};
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)frame;
    unsigned char mutil_addr[6] = {0x01,0x00,0x5e,0x00,0x00,0x4c};
    unsigned char broad_addr[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    // after discover
    if(size < 24)
        goto invalid;

    int i;
    unsigned char ch;
    for(i=0; i<24; i++)
    {
        ch = *((unsigned char*)frame + i);
        sc_head.dummy[i] = ch;
    }
   
    // retrasmition
    if((sc_head.dummy[1] & 0x08) != 0x00){
        // sc_log("retran sn %d \n",(sc_head.dummy[22]+sc_head.dummy[23]*256)>>4);
        goto invalid;
    }

    //protected
    if((sc_head.dummy[1] & 0x40) == 0x00){
        // sc_log("open sn %d \n",(sc_head.dummy[22]+sc_head.dummy[23]*256)>>4);
        goto invalid;
    }
    //deretion
    if (26 == ieee80211_hdrlen(hdr->frame_control)){
        qos_flag = true;
    } else {
        qos_flag = false;
    }

    if((sc_head.dummy[1] & 0x02) == 0x02){
        //from ds = 1
        // sc_log("sn %d len %d\n",(sc_head.dummy[22]+sc_head.dummy[23]*256)>>4,size);
        // system_printf("1 qos_flag %d addr1 %02x %02x %02x %02x %02x %02x\n",qos_flag, sc_head.dummy[4],sc_head.dummy[5],sc_head.dummy[6],sc_head.dummy[7],sc_head.dummy[8],sc_head.dummy[9]);
        // system_printf("1 qos_flag %d addr2 %02x %02x %02x %02x %02x %02x\n",qos_flag, sc_head.dummy[10],sc_head.dummy[11],sc_head.dummy[12],sc_head.dummy[13],sc_head.dummy[14],sc_head.dummy[15]);
        // system_printf("1 qos_flag %d addr3 %02x %02x %02x %02x %02x %02x\n",qos_flag, sc_head.dummy[16],sc_head.dummy[17],sc_head.dummy[18],sc_head.dummy[19],sc_head.dummy[20],sc_head.dummy[21]);
        if(qos_flag == true){
            if(0 == sc_memcmp(mutil_addr,&sc_head.dummy[4],6)){
            *type = MULTI_TYPE_FRDS;
            if(sc_context.sc_state < SC_STATE_SRC_LOCKED){
                find_ap_in_aplist(&sc_head.dummy[10],&target_ap);
            }
                
            } else if(0 == sc_memcmp(broad_addr,&sc_head.dummy[4],6)){
                *type = BRODA_TYPE_FRDS;
                // sc_log("from ds\n");
            } else {
                goto invalid;
            }
        } else {
            if(0 == sc_memcmp(mutil_addr,&sc_head.dummy[4],6)){
                *type = MULTI_TYPE_FRDS;
                if(sc_context.sc_state < SC_STATE_SRC_LOCKED){
                    find_ap_in_aplist(&sc_head.dummy[10],&target_ap);
                }
                    
            } else if(0 == sc_memcmp(broad_addr,&sc_head.dummy[4],6)){
                *type = BRODA_TYPE_FRDS;
                // sc_log("from ds\n");
            } else {
                goto invalid;
            }
        }
    } else if((sc_head.dummy[1] & 0x01) == 0x01) {

        // system_printf("2 qos_flag %d addr1 %02x %02x %02x %02x %02x %02x\n",qos_flag, sc_head.dummy[4],sc_head.dummy[5],sc_head.dummy[6],sc_head.dummy[7],sc_head.dummy[8],sc_head.dummy[9]);
        // system_printf("2 qos_flag %d addr2 %02x %02x %02x %02x %02x %02x\n",qos_flag, sc_head.dummy[10],sc_head.dummy[11],sc_head.dummy[12],sc_head.dummy[13],sc_head.dummy[14],sc_head.dummy[15]);
        // system_printf("2 qos_flag %d addr3 %02x %02x %02x %02x %02x %02x\n",qos_flag, sc_head.dummy[16],sc_head.dummy[17],sc_head.dummy[18],sc_head.dummy[19],sc_head.dummy[20],sc_head.dummy[21]);
        if(qos_flag == true){
            if(0 == sc_memcmp(mutil_addr,&sc_head.dummy[16],6)){
                if(sc_context.sc_state < SC_STATE_SRC_LOCKED)
                    find_ap_in_aplist(&sc_head.dummy[4],&target_ap);
                *type = MULTI_TYPE_TODS;
            } else if(0 == sc_memcmp(broad_addr,&sc_head.dummy[16],6)){
                *type = BRODA_TYPE_TODS;
                // sc_log("to ds\n");
            } else {
                goto invalid;
            }
        } else {
            if(0 == sc_memcmp(mutil_addr,&sc_head.dummy[16],6)){
                if(sc_context.sc_state < SC_STATE_SRC_LOCKED)
                    find_ap_in_aplist(&sc_head.dummy[4],&target_ap);
                *type = MULTI_TYPE_TODS;
                } else if(0 == sc_memcmp(broad_addr,&sc_head.dummy[16],6)){
                    *type = BRODA_TYPE_TODS;
                    // sc_log("to ds\n");
                } else {
                goto invalid;
            }
        }
    } 
    int ret = sn_check((sc_head.dummy[22]+sc_head.dummy[23]*256)>>4, *type);
    if(ret == 0){
        goto invalid;
    }
    // //Address 1
    // for(i=4; i<10; i++)
    //     if(sc_head.dummy[i]!=sc_head.dummyap[i])
    //         goto invalid;
    // //Address 2
    // for(i=10; i<16; i++)
    //     if(sc_head.dummy[i]!=sc_head.dummyap[i])
    //         goto invalid;
    // //Address 3
    // for(i=16; i<22; i++)
    //     if(sc_head.dummy[i]!=sc_head.dummyap[i])
    //         goto invalid;

    isvalid = 1;

invalid:
    return isvalid;
}

/* return 1 is valid 
 * return 0 is invalid
 * */
int sc_discover_filter(const unsigned char *frame, int size)
{
    int isvalid = 0;
    // before discover
    if(size < 50)
        goto invalid;

    isvalid = 1;
invalid:
    return isvalid;
}

static void sc_record_move_ones(void *base_addr, int record_num)
{
    int i; 
    unsigned short *record_base = base_addr;

    for(i = 0; i < record_num; i++)
    {
        if(record_base[i+1] != record_base[i])
            record_base[i] = record_base[i+1];
    }
}

static void sc_add_seq_data(const unsigned char *data, int seq)
{
    if(seq < sc_context.need_seq)
    {
        if((seq*4 + 4) <= USR_DATA_BUFF_MAX_SIZE)
        {
            if((sc_context.seq_success_map & (1 << seq)) == 0) 
            {
                memcpy(sc_context.usr_data + seq*4, data, 4);
                sc_context.seq_success_map |= (1 << seq);
            }
        }
    }
}
static void swap(uint16_t *p1,uint16_t *p2){
    uint16_t temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

static bool append_prefix(void *base_addr, unsigned short length, int type)
{
    int i; 
    unsigned short *record_base = base_addr;
    record_base[MAX_GUIDE_RECORD] = length;
    //if duplicate
    for(i = 0; i < 4; i++){
        if(length == record_base[i])
            goto check;           
    }
    for(i = 3; i >= 0; i--)
    {
        if(record_base[i] == 0)
            swap(&record_base[i+1],&record_base[i]);
        else if(record_base[i+1] < record_base[i])
            swap(&record_base[i],&record_base[i+1]);
        else{
            break;
        }
    }
    
check:
    system_printf("type ----------- %d length %d\n",type,length );
    for(i = 0; i <= 4; i++)
    {
        system_printf("%d ",record_base[i]);
    }
    system_printf("\n");
    for(i = 0; i < 4; i++)
    {
        if(record_base[i] == 0){
            return false;
        }
    }
   
    return true;
}

static void clear_record(void *base_addr)
{
    int i; 
    unsigned short *record_base = base_addr;
    for(i = 0; i < 4; i++)
    {
        record_base[i] = 0;
    }
}

static int get_base_len(const uint8_t* frame, unsigned short length, int type)
{
    struct ap_info target_ap = {0};
    if(length > 1094 && length < 1112 && sc_context.base_len[type] == 0){
        sc_context.data.guide_code.length_record[type][MAX_GUIDE_RECORD] = length;
    } else {
        return 0;
    }
    bool ret = append_prefix(sc_context.data.guide_code.length_record[type],length, type);
    // sc_record_move_ones(sc_context.data.guide_code.length_record[type], MAX_GUIDE_RECORD);
    if(ret == true){
        if((sc_context.data.guide_code.length_record[type][1] - sc_context.data.guide_code.length_record[type][0] == 1) &&
        (sc_context.data.guide_code.length_record[type][2] - sc_context.data.guide_code.length_record[type][1] == 1) &&
        (sc_context.data.guide_code.length_record[type][3] - sc_context.data.guide_code.length_record[type][2] == 1))
        {
            sc_context.base_len[type] = sc_context.data.guide_code.length_record[type][0] - 1024;
            sc_log("base len %d:%d\n", type, sc_context.base_len[type]);
            if(type == MULTI_TYPE_FRDS || type == BRODA_TYPE_FRDS){
                find_ap_in_aplist(&frame[10],&target_ap);
            } else{
                find_ap_in_aplist(&frame[4],&target_ap);
            }
            if(target_ap.ssid[0] == 0){
                clear_record(sc_context.data.guide_code.length_record[type]);
                return 0; 
            }
            sc_context.ssid_len = strlen(target_ap.ssid);
            memcpy(sc_context.ssid, target_ap.ssid, sc_context.ssid_len);
            // system_printf("sc_context.data.guide_code.length_record[type+1][1] %d\n",sc_context.data.guide_code.length_record[type+1][1]);
            return 1;
        }
        clear_record(sc_context.data.guide_code.length_record[type]);
        return 0; 
    } 

	return 0;
}

static void sc_recv_discover(const uint8_t* frame, unsigned short length, int type)
{
    int success = 0;
    success = get_base_len(frame,length,type);
    if(success)
    {
        sc_context.sc_state = SC_STATE_SRC_LOCKED;
        resest_sc_data();
        sc_log("sc_recv_discover success\n");
        // sc_log("base len %d:%d\n", type, sc_context.base_len[type]);

        // int i;
        // unsigned char ch;
        // for(i=0; i<24; i++)
        // {
        //     ch = *((unsigned char*)frame + i);
        //     sc_head.dummyap[i] = ch;
        // }
    }
}


static void sc_process_magic_code(int type, unsigned short length)
{
    // system_printf("length %d type %d base %d\n",length,type,sc_context.base_len[type]);
    if(sc_context.base_len[type] == 0){
        return;
    }

    sc_context.data.magic_code.record[type][MAX_MAGIC_CODE_RECORD] = length - sc_context.base_len[type];

    sc_record_move_ones(sc_context.data.magic_code.record[type], MAX_MAGIC_CODE_RECORD);

    if(((sc_context.data.magic_code.record[type][0]&0x01f0)==0x0040)&&
        ((sc_context.data.magic_code.record[type][1]&0x01f0)==0x0010)&&
            ((sc_context.data.magic_code.record[type][2]&0x01f0)==0x0020)&&
            ((sc_context.data.magic_code.record[type][3]&0x01f0)==0x0030))
    {
        if((sc_context.data.magic_code.record[type][0] ^ 
            sc_context.data.magic_code.record[type][1] ^ 
            sc_context.data.magic_code.record[type][2] ^
            sc_context.data.magic_code.record[type][3]) == sc_context.data.magic_code.record[type][4]) {
            sc_context.total_len = ((sc_context.data.magic_code.record[type][0] & 0x000F) << 4) + (sc_context.data.magic_code.record[type][1] & 0x000F);
            if(sc_context.total_len > 128) 
                sc_context.total_len -= 128;
            sc_context.ssid_crc = ((sc_context.data.magic_code.record[type][2] & 0x000F) << 4) + (sc_context.data.magic_code.record[type][3] & 0x000F);
            //TODO:double check magic code
            sc_context.sc_state = SC_STATE_MAGIC_CODE_COMPLETE;
            resest_sc_data();
            sc_log("sc_process_magic_code success\n");
            sc_log("total_len:%d, ssid crc:%x\n", sc_context.total_len, sc_context.ssid_crc);
        }
    }
}

static void sc_process_prefix_code(int type, unsigned short length)
{
    if(sc_context.base_len[type] == 0){
        return;
    }
    sc_context.data.prefix_code.record[type][MAX_PREFIX_CODE_RECORD] = length - sc_context.base_len[type];

    sc_record_move_ones(sc_context.data.prefix_code.record[type], MAX_PREFIX_CODE_RECORD );

    if((sc_context.data.prefix_code.record[type][0]&0x01f0)==0x0040&&
        (sc_context.data.prefix_code.record[type][1]&0x01f0)==0x0050&&
            (sc_context.data.prefix_code.record[type][2]&0x01f0)==0x0060&&
            (sc_context.data.prefix_code.record[type][3]&0x01f0)==0x0070)
    {
        sc_context.pswd_len = ((sc_context.data.prefix_code.record[type][0] & 0x000F) << 4) + (sc_context.data.prefix_code.record[type][1] & 0x000F);
        if(sc_context.pswd_len > PASSWORD_MAX_LEN)
            sc_context.pswd_len = 0;
        sc_context.pswd_lencrc = ((sc_context.data.prefix_code.record[type][2] & 0x000F) << 4) + (sc_context.data.prefix_code.record[type][3] & 0x000F);
        if(calcrc_1byte(sc_context.pswd_len)==sc_context.pswd_lencrc)
        {
            sc_context.sc_state = SC_STATE_PREFIX_CODE_COMPLETE;
        }
        else
        {
            sc_log("password length crc error.\n");
            resest_sc_data();
            return;
        }

        // only receive password, and random
        sc_context.need_seq = (sc_context.total_len + 3)/4; 
        sc_context.seq_success_map_cmp = (1 << sc_context.need_seq) - 1; // EXAMPLE: need_seq = 5; seq_success_map_cmp = 0x1f; 
        sc_log("ssid %s len %d need_seq %d sc_context.total_len %d\n",sc_context.ssid, sc_context.ssid_len, sc_context.need_seq,sc_context.total_len);
            
        resest_sc_data();
        sc_log("sc_add_seq_data success\n");
        sc_log("pswd_len:%d, pswd_lencrc:%x, need seq:%d, seq map:%x\n", 
                sc_context.pswd_len, sc_context.pswd_lencrc, sc_context.need_seq, sc_context.seq_success_map_cmp);
    }
}

static int sc_pkt_seq_type(int type) 
{
    if(((sc_context.data.seq_code.record[type][0]&0x180)==0x80) &&
        ((sc_context.data.seq_code.record[type][1]&0x180)==0x80) && 
        ((sc_context.data.seq_code.record[type][2]&0x0100)==0x0100) && 
        ((sc_context.data.seq_code.record[type][3]&0x0100)==0x0100) && 
        ((sc_context.data.seq_code.record[type][4]&0x0100)==0x0100) && 
        ((sc_context.data.seq_code.record[type][5]&0x0100)==0x0100) && 
        ((sc_context.data.seq_code.record[type][1]&0x7F) <= ((sc_context.total_len>>2)+1)))
    {
        //middle seq
        return 0;
    } else if (((sc_context.data.seq_code.record[type][0]&0x180)==0x80) &&
        ((sc_context.data.seq_code.record[type][1]&0x7f)==(sc_context.need_seq - 1)) && 
        ((sc_context.data.seq_code.record[type][2]&0x0100)==0x0100)){
        //last seq
        return 1;
    } else {
        return 2;
    }
}

static void sc_process_sequence(int type,unsigned short length)
{
    if(sc_context.base_len[type] == 0){
        return;
    }
    sc_context.data.seq_code.record[type][MAX_SEQ_CODE_RECORD] = length - sc_context.base_len[type];

    sc_record_move_ones(sc_context.data.seq_code.record[type], MAX_SEQ_CODE_RECORD);
    int pkt_type = sc_pkt_seq_type(type);
    if(pkt_type != 2)
    {
        unsigned char tempBuffer[MAX_PTYPE][6] = {0};
        tempBuffer[type][0]=sc_context.data.seq_code.record[type][0]&0x7F; //seq crc
        tempBuffer[type][1]=sc_context.data.seq_code.record[type][1]&0x7F; //seq index
        tempBuffer[type][2]=sc_context.data.seq_code.record[type][2]&0xFF; //data, same as following
        tempBuffer[type][3]=sc_context.data.seq_code.record[type][3]&0xFF;
        tempBuffer[type][4]=sc_context.data.seq_code.record[type][4]&0xFF;
        tempBuffer[type][5]=sc_context.data.seq_code.record[type][5]&0xFF;

        sc_log("[crc:%02x][index:%d]:%02x,%02x,%02x,%02x; ",
                tempBuffer[type][0], tempBuffer[type][1],
                tempBuffer[type][2], tempBuffer[type][3], tempBuffer[type][4], tempBuffer[type][5]);
        if(pkt_type == 0 && tempBuffer[type][0] ==  (calcrc_bytes(&tempBuffer[type][1],5)&0x7F))
        {
            int cur_seq = tempBuffer[type][1];
            sc_log("cur_seq %d\n",cur_seq);
            sc_add_seq_data(&tempBuffer[type][2], cur_seq);

            sc_log("type %d sc_context.seq_success_map_cmp %x seq mapped:%x\n",type, sc_context.seq_success_map_cmp, sc_context.seq_success_map);
            resest_sc_data();
        } else if(pkt_type == 1 && tempBuffer[type][0] == (calcrc_bytes(&tempBuffer[type][1],sc_context.total_len%4 + 1)&0x7F)){
            int cur_seq = tempBuffer[type][1];
            sc_log("cur_seq %d\n",cur_seq);
            sc_add_seq_data(&tempBuffer[type][2], cur_seq);

            sc_log("sc_context.seq_success_map_cmp %x seq mapped:%x\n",sc_context.seq_success_map_cmp, sc_context.seq_success_map);
            resest_sc_data();
        } else
        {
            sc_log("crc check error. calc crc:[%02x != %02x]\n",
                    tempBuffer[type][0],
                    calcrc_bytes(&tempBuffer[type][1],5));
        }
        if(sc_context.seq_success_map_cmp == sc_context.seq_success_map)
        {
            int i;
            sc_log("ssid len %d\n",sc_context.ssid_len);
            sc_log("User data is :");
            for(i=0;i<sc_context.pswd_len + 1; i++) {
                sc_log("%02x ",sc_context.usr_data[i]);
            }
            sc_log("\n");
            sc_context.random_num = sc_context.usr_data[sc_context.pswd_len];
            memcpy(sc_context.pwd, sc_context.usr_data, sc_context.pswd_len);
            sc_context.usr_data[sc_context.pswd_len + 1] = 0;
            sc_context.sc_state = SC_STATE_COMPLETE;
        }
    } 
}

int sc_recv(const uint8_t * frame, int length)
{
    int type;
    if(!sc_filter(frame, length, &type))
        return SC_STATUS_CONTINUE;
    //  sc_log("sc_context.sc_state %d\n",sc_context.sc_state);
    uint8_t *temp = (uint8_t *)frame;
    sc_log("type %d len %d sn %d\n",type,length,(temp[22]+temp[23]*256)>>4);
    switch(sc_context.sc_state)
    {
        case SC_STATE_IDLE:
            if(sc_discover_filter(frame, length)) {
                sc_recv_discover(frame, length, type);
                if(sc_context.sc_state == SC_STATE_SRC_LOCKED)
                    return SC_STATUS_LOCK_CHANNEL;
            }
            break;
        case SC_STATE_SRC_LOCKED:
            get_base_len(frame, length, type);
            sc_process_magic_code(type,length);
            break;
        case SC_STATE_MAGIC_CODE_COMPLETE:
            get_base_len(frame, length, type);
            sc_process_prefix_code(type,length);
            break;    
        case SC_STATE_PREFIX_CODE_COMPLETE:
            get_base_len(frame, length, type);
            // if(xTimerIsTimerActive(sc_cf.lock_chn_timer))
            //     xTimerStop(sc_cf.lock_chn_timer,0);
            sc_process_sequence(type,length);
            if(sc_context.sc_state == SC_STATE_COMPLETE)
                return SC_STATUS_GOT_SSID_PSWD;
            break;        
        default:
            sc_context.sc_state = SC_STATE_IDLE;
            break;
    }
    return SC_STATUS_CONTINUE;
}


static int process_data(void *packet, int size)
{
    // todo: add mutex
    int ret;
    int msg;
    ret = sc_recv((const uint8_t *)packet, size);
    if(ret == SC_STATUS_CONTINUE)
    {
        //pass
    }
    else if(ret == SC_STATUS_LOCK_CHANNEL)
    {
        xTimerStop(sc_cf.channel_timer,0);
        msg = SC_STATUS_LOCK_CHANNEL;
        g_round = 0;
        xQueueSend(g_event_queue, &msg, portMAX_DELAY);
    }
    else if(ret == SC_STATUS_GOT_SSID_PSWD)
    {
        xTimerStop(sc_cf.task_timer,0);
        sc_log("smartconfig got ssid&pwd.");
        sc_get_result(&sc_result);
        sc_log("Result:\nssid_crc:[%x]\nkey_len:[%d]\nkey:[%s]\nrandom:[0x%02x]\nssid:[%s]", 
            sc_result.reserved,
            sc_result.pwd_length,
            sc_result.pwd,
            sc_result.random,
            sc_result.ssid);
        msg = SC_STATUS_GOT_SSID_PSWD;
        xQueueSend(g_event_queue, &msg, portMAX_DELAY);

    }
    //todo : add mutex

    return ret;
}

#define ZC_MAX_SSID_LEN     (32 + 1)/* ssid: 32 octets at most, include the NULL-terminated */
#define ZC_MAX_PASSWD_LEN   (64 + 1)/* 8-63 ascii */

int process_manager(void *packet, int size, int8_t rssi)
{
    uint8_t ssid[ZC_MAX_SSID_LEN] = {0}, bssid[ETH_ALEN] = {0};
    uint8_t auth, pairwise_cipher, group_cipher;
    struct ieee80211_hdr *hdr;
    int fc, ret, channel;
    struct ieee80211_hdr *mgmt_header = (struct ieee80211_hdr *)packet;

    if (mgmt_header == NULL) {
        return -1;
    }

    hdr = (struct ieee80211_hdr *)mgmt_header;
    fc = hdr->frame_control;

    /*
     * just for save ap in aplist for ssid amend.
     */
    if (!ieee80211_is_beacon(fc) && !ieee80211_is_probe_resp(fc)) {
        return -1;
	}

    ret = ieee80211_get_bssid((uint8_t *)mgmt_header, bssid);
    if (ret < 0) {
        return -1;
	}

    ret = ieee80211_get_ssid((uint8_t *)mgmt_header, size, ssid);
    if (ret < 0) {
        return -1;
	}

    channel = cfg80211_get_bss_channel((uint8_t *)mgmt_header, size);
    rssi = rssi > 0 ? rssi - 256 : rssi;

    cfg80211_get_cipher_info((uint8_t *)mgmt_header, size, &auth,
                             &pairwise_cipher, &group_cipher);
    
    awss_save_apinfo(ssid, bssid, channel, auth,
                     pairwise_cipher, group_cipher, rssi);

    return -1;
}

static void wifiSnifferCallback(void *buf, int len, wifi_promiscuous_pkt_type_t type)
{
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf; 
    if(type == WIFI_PKT_MGMT){
        process_manager((char *)pkt->payload, len, pkt->rx_ctrl.rssi);
    }
    else if(type == WIFI_PKT_DATA){
        if (SC_STATUS_GOT_SSID_PSWD==process_data((char *)pkt->payload, len)){
            wifi_set_promiscuous(false);
        }
    }
    
}

void sc_task(void* parameter)
{
    int event;
    
    memset(&sc_context , 0, sizeof(sc_local_context));
    sc_context.sc_state = SC_STATE_IDLE;
    g_event_queue = xQueueCreate(3, sizeof(int));
    
    wifi_promiscuous_filter_t filter = {0};
    filter.filter_mask = WIFI_PROMIS_FILTER_MASK_DATA | WIFI_PROMIS_FILTER_MASK_MGMT;
    wifi_set_promiscuous_filter(&filter);
    wifi_set_promiscuous_rx_cb(wifiSnifferCallback);
    wifi_set_promiscuous(true);
    awss_init_ieee80211_aplist();
    xTimerStart(sc_cf.channel_timer, 0);
    xTimerStart(sc_cf.task_timer, 0 );
    while(1){
        if (pdPASS == xQueueReceive(g_event_queue, &event, (portTickType)portMAX_DELAY)){
            switch (event)
            {
                case SC_STATUS_LOCK_CHANNEL:
                    // xTimerStart(sc_cf.lock_chn_timer, 0);
                    if(sc_cf.callback != NULL){
                        sc_cf.callback(SC_STATUS_LOCK_CHANNEL,&g_current_channel);
                    }
                    break;
                case SC_STATUS_GOT_SSID_PSWD:
                    if(sc_cf.callback != NULL){
                        sc_cf.callback(SC_STATUS_GOT_SSID_PSWD,&sc_result);
                    }
                    break;
                case SC_STATUS_TIMEOUT:
                    system_printf("SC_STATUS_TIMEOUT\n");
                    sc_context.sc_state = SC_STATE_STOPED;
                    memset(&sc_head, 0, sizeof(sc_head_t));
                    resest_sc_data();
                    if(sc_cf.callback != NULL){
                        sc_cf.callback(SC_STATUS_TIMEOUT,NULL);
                    }
                    goto stop;
                case SC_STATUS_STOP:
                    system_printf("SC_STATUS_STOP\n");
                    sc_context.sc_state = SC_STATE_STOPED;
                    memset(&sc_head, 0, sizeof(sc_head_t));
                    resest_sc_data();
                    if(sc_cf.callback != NULL){
                        sc_cf.callback(SC_STATUS_STOP,NULL);
                    }
                    goto stop;
            }
        }
    }
stop:
    if(g_event_queue) {
        vQueueDelete(g_event_queue);
    }   
    g_event_queue = NULL;
    vTaskDelete(NULL);
}

// void sc_channel_lock_func(void* param)
// {
//     if(sc_context.sc_state != SC_STATE_COMPLETE){
//         sc_log("lock the channel but can't get result,clear the record and start again\n");
//         memset(&sc_head, 0, sizeof(sc_head_t));
//         memset(&sc_context , 0, sizeof(sc_local_context));
//         sc_context.sc_state = SC_STATE_IDLE;
//         xTimerStart(sc_cf.channel_timer, 0);
//     }
// }

bool channel_is_valid(int channel)
{
    return zconfig_check_ap_channel(channel);
}

void sc_channel_switch_func(void* param)
{
    int msg = 0;
    uint8_t chn = wifi_rf_get_channel();
    chn++;
    if(chn > MAX_CHANNELS) {
        chn = 1;
    }
    sc_log("sc set channel %d\n",chn);
    while(g_round >= 20 && channel_is_valid(chn) == false){
        chn++;
    }
    g_round++;
    g_current_channel = chn;
    sc_change_channel();
    wifi_rf_set_channel(chn);
    if(chn == 1 || chn == 6 || chn == 11){
        xTimerChangePeriod(sc_cf.channel_timer, 500 / portTICK_PERIOD_MS, 0);
    }else {
        xTimerChangePeriod(sc_cf.channel_timer, 200 / portTICK_PERIOD_MS, 0);
    }
}

void sc_timeout_func(void* param)
{
    int msg;
    if(sc_context.sc_state < SC_STATE_COMPLETE && sc_context.sc_state != SC_STATE_STOPED) {
        sc_log("smartconfig timeout\n");
        xTimerDelete(sc_cf.channel_timer,0);
        xTimerDelete(sc_cf.task_timer,0);
        // xTimerDelete(sc_cf.lock_chn_timer,0);
        msg = SC_STATUS_TIMEOUT;
        if(g_event_queue) {
            xQueueSend(g_event_queue, &msg, portMAX_DELAY);
        }
        wifi_set_promiscuous(false);
    } else {
        return;
    }
}


int smartconfig_start(sc_callback_t cb)
{
    portBASE_TYPE result;
    if(sc_context.sc_state == SC_STATE_STOPED){
        wifi_stop_station();
        // wifi_start_station();
        memset(&sc_context , 0, sizeof(sc_local_context));
        memset(&sc_cf , 0, sizeof(sc_config));
        sc_context.sc_state = SC_STATE_IDLE;
        sc_cf.callback = cb;
        if(sc_cf.timeout_s == 0){
            sc_cf.timeout_s = 240;//default expired time is 120s
        }
	    system_printf("smart config start...\n version %s\n",sc_vers);
        sc_cf.channel_timer = xTimerCreate("ChannelTimer",CHANNEL_TIME_FREQ / portTICK_RATE_MS, pdTRUE, NULL, sc_channel_switch_func);
        // sc_cf.lock_chn_timer = xTimerCreate("LockchannelTimer",CHANNEL_LOCK_TIME, pdFALSE, NULL, sc_channel_lock_func);
        sc_cf.task_timer = xTimerCreate("TaskTimer",sc_cf.timeout_s*100, pdFALSE, NULL, sc_timeout_func);
	    result = xTaskCreate(sc_task, (const char *)"smart_config_task", 1024, NULL, 7, &sc_task_handle);

        if(result != pdTRUE || sc_cf.channel_timer == NULL || sc_cf.task_timer == NULL) {
            if(sc_task_handle) {vTaskDelete(sc_task_handle);}
            if(sc_cf.channel_timer) {xTimerDelete(sc_cf.channel_timer,0);}
            // if(sc_cf.lock_chn_timer) {xTimerDelete(sc_cf.lock_chn_timer,0);}
            if(sc_cf.task_timer) {xTimerDelete(sc_cf.task_timer,0);}
        }
        return 0;
    } else {
        sc_error("smartconfig is already started,can't start it twice\n");
        return -1;
    }
}


int smartconfig_stop(void)
{
    int msg;

    if(sc_context.sc_state != SC_STATE_STOPED)
    {
        sc_log("smartconfig stop\n");
        wifi_set_promiscuous(false);

        if(sc_cf.channel_timer) 
        {
            g_round = 0;
            xTimerStop(sc_cf.channel_timer,0);
            xTimerDelete(sc_cf.channel_timer,0);
        }
        // if(sc_cf.lock_chn_timer) 
        // {
        //     xTimerStop(sc_cf.lock_chn_timer,0);
        //     xTimerDelete(sc_cf.lock_chn_timer,0);
        // }
        if(sc_cf.task_timer) 
        {
            xTimerStop(sc_cf.task_timer,0);
            xTimerDelete(sc_cf.task_timer,0);
        }
        msg = SC_STATUS_STOP;
        if(g_event_queue) {
            xQueueSend(g_event_queue, &msg, portMAX_DELAY);
        }
        awss_deinit_ieee80211_aplist();
        sc_context.sc_state = SC_STATE_STOPED;
        
        vTaskDelay(100 / portTICK_RATE_MS);
        return 0;
    }else {
        sc_error("smartconfig is already stopped,can't stop it twice\n");
        return -1;
    }
    
    
    
}

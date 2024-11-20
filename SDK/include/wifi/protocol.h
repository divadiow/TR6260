#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

#define FC_PV0 0
#define FC_PV1 1

#define FC_PV0_TYPE_MGMT        0
#define FC_PV0_TYPE_CTRL        1
#define FC_PV0_TYPE_DATA        2
#define FC_PV0_TYPE_EXT         3

#define FC_PV0_TYPE_CTRL_PS_POLL            10
#define FC_PV0_TYPE_CTRL_CF_END             14

#define FC_PV0_TYPE_MGMT_ASSOC_REQ          0
#define FC_PV0_TYPE_MGMT_ASSOC_RSP          1
#define FC_PV0_TYPE_MGMT_REASSOC_REQ        2
#define FC_PV0_TYPE_MGMT_REASSOC_RSP        3
#define FC_PV0_TYPE_MGMT_PROBE_REQ          4
#define FC_PV0_TYPE_MGMT_PROBE_RSP          5
#define FC_PV0_TYPE_MGMT_TIMING_ADVERTISE   6
#define FC_PV0_TYPE_MGMT_RESERVED           7
#define FC_PV0_TYPE_MGMT_BEACON             8
#define FC_PV0_TYPE_MGMT_ATIM               9
#define FC_PV0_TYPE_MGMT_DISASSOC           10
#define FC_PV0_TYPE_MGMT_AUTH               11
#define FC_PV0_TYPE_MGMT_DEAUTH             12
#define FC_PV0_TYPE_MGMT_ACTION             13
#define FC_PV0_TYPE_MGMT_ACTION_NOACK       14
#define FC_PV0_TYPE_MGMT_MAX                15

#define FC_PV0_TYPE_DATA_DATA                   0
#define FC_PV0_TYPE_DATA_DATA_CF_ACK            1
#define FC_PV0_TYPE_DATA_DATA_CF_POLL           2
#define FC_PV0_TYPE_DATA_DATA_CF_ACK_POLL       3
#define FC_PV0_TYPE_DATA_DATA_NULL              4
#define FC_PV0_TYPE_DATA_CF_ACK                 5
#define FC_PV0_TYPE_DATA_CF_POLL                6
#define FC_PV0_TYPE_DATA_CF_ACK_POLL            7
#define FC_PV0_TYPE_DATA_QOS_DATA               8
#define FC_PV0_TYPE_DATA_QOS_DATA_CF_ACK        9
#define FC_PV0_TYPE_DATA_QOS_DATA_CF_POLL       10
#define FC_PV0_TYPE_DATA_QOS_DATA_CF_ACK_POLL   11
#define FC_PV0_TYPE_DATA_QOS_NULL               12
#define FC_PV0_TYPE_DATA_RESERVED               13
#define FC_PV0_TYPE_DATA_QOS_CF_POLL            14
#define FC_PV0_TYPE_DATA_QOS_CF_ACK_POLL        15
#define FC_PV0_TYPE_DATA_MAX                    16

#define FC_PV0_TYPE_EXT_S1G_BEACON				1

#define FC_PV0_PROTECTED    0x4000

#define FC_PV1_TYPE_QOSDATA0    0
#define FC_PV1_TYPE_MGMT        1
#define FC_PV1_TYPE_CTRL        2
#define FC_PV1_TYPE_QOSDATA3    3

#define FC_PV1_TYPE_MGMT_ACTION             0
#define FC_PV1_TYPE_MGMT_ACTION_NOACK       1
#define FC_PV1_TYPE_MGMT_SHORT_PROBE_RSP    2
#define FC_PV1_TYPE_MGMT_RESOURCE_ALLOC     3

#define ETH_P_PAE 0x888E /* Port Access Entity (IEEE 802.1X) */

typedef struct _QoSField {
	uint16_t    qos_tid                 : 4;
    uint16_t    qos_eosp                : 1;
	uint16_t    qos_ack_policy          : 2;   // 00: normal ack, 01: no ack, 10: no explicit ack or scheduled ack under PSMP, 11: block ack
	uint16_t    qos_amsdu_present       : 1;   // AMSDU Presence
	uint16_t    qos_bit8_15             : 8;
} QoSField;

typedef struct _GenericMacHeader {
    // Word 0 : MAC Header Word 0
    // 16 bit Frame Control
	uint16_t    version        : 2;    // default 0
	uint16_t    type           : 2;    // 0: Management , 1: Control,  2: Data,  3: reserved
	uint16_t    subtype        : 4;
	uint16_t    to_ds          : 1;    // to AP
	uint16_t    from_ds        : 1;    // from AP
	uint16_t    more_frag      : 1;
	uint16_t    retry          : 1;
	uint16_t    pwr_mgt        : 1;
	uint16_t    more_data      : 1;
	uint16_t    protect        : 1;
	uint16_t    order          : 1;
    // 16 bit Duration
	uint16_t duration_id;

    // Word 1 ~ Word 5: MAC Header Word 1 ~ MAC Header Word 5(2byte)
	uint8_t     address1[6];
	uint8_t     address2[6];
	uint8_t     address3[6];

	// Word 5 : MAC Header Word 5(2byte)
	uint16_t    fragment_number    : 4;
	uint16_t    sequence_number    : 12;

    // Qos Control Field(16bit) exist only subtype is QoS Data or Qos Null
    // Word 6 : MAC Header Word 6(2Byte)
    union {
        struct {
        	uint16_t    qos_tid                 : 4;
    	    uint16_t    qos_eosp                : 1;
        	uint16_t    qos_ack_policy          : 2;   // 00: normal ack, 01: no ack, 10: no explicit ack or scheduled ack under PSMP, 11: block ack
        	uint16_t    qos_amsdu_present       : 1;   // AMSDU Presence
        	uint16_t    qos_bit8_15             : 8;
            uint8_t     qos_payload[0];
        };
        uint8_t payload[0];
    };
} GenericMacHeader;

typedef struct _ADDBA_Req {
	uint8_t category 				: 8;
	uint8_t ba_action 				: 8;
	uint8_t dial_token 				: 8;

	uint8_t amsdu 					: 1;
	uint8_t policy 					: 1;
	uint8_t tid 					: 4;
	uint16_t buf_size 				: 10;
	
	uint16_t timeout				: 16;
	uint16_t frag 					: 4;
	uint16_t ssn 					: 12;
} __attribute__((packed)) ADDBA_Req ;

typedef struct _ADDBA_Resp {
	uint8_t category 				: 8;
	uint8_t ba_action 				: 8;
	uint8_t dial_token 				: 8;
	uint16_t status 				: 16;
	uint8_t amsdu 					: 1;
	uint8_t policy 					: 1;
	uint8_t tid 					: 4;
	uint16_t buf_size 				: 10;
	uint16_t timeout 				: 16;
} __attribute__((packed)) ADDBA_Resp;

typedef struct _DELBA_Req {
	uint8_t category 				: 8;
	uint8_t ba_action 				: 8;
	uint16_t rsv 					: 11;
	uint8_t init 					: 1;
	uint8_t tid 					: 4;
	uint16_t reason 				: 16;
} __attribute__((packed)) DELBA_Req;

typedef struct _CCMPHeader {
	uint8_t	pn0;
	uint8_t	pn1;

	uint8_t	reserved1;

	uint8_t	reserved2		: 5;
	uint8_t	ext_iv			: 1;
	uint8_t	key_id			: 2;

	uint8_t	pn2;
	uint8_t pn3;
	uint8_t	pn4;
	uint8_t	pn5;
} CCMPHeader;
#define CCMPH(x) reinterpret_cast<CCMPHeader*>(x)

typedef struct _WEPHeader {
	uint8_t	iv0;
	uint8_t	iv1;
	uint8_t	iv2;


	uint8_t	reserved2		: 6;
	uint8_t	key_id			: 2;

} WEPHeader;
#define WEPH(x) reinterpret_cast<WEPHeader*>(x)

typedef struct _TKIPHeader {
	uint8_t	tsc1;
	uint8_t	wep_seed;
	uint8_t	tsc0;

	uint8_t	reserved2		: 5;
	uint8_t	ext_iv			: 1;
	uint8_t	key_id			: 2;

	uint8_t	tsc2;
	uint8_t	tsc3;
	uint8_t	tsc4;
	uint8_t	tsc5;

} TKIPHeader;
#define TKIPH(x) reinterpret_cast<TKIPHeader*>(x)

bool lmac_check_ver_frame(uint8_t* frame);
bool lmac_check_is_pv0_frame(uint8_t* frame);
bool lmac_check_is_pv1_frame(uint8_t* frame);
bool lmac_check_group_addr_frame(uint8_t *addr);
bool lmac_check_management_frame(GenericMacHeader *gmh);
bool lmac_check_control_frame(GenericMacHeader *gmh); 
bool lmac_check_data_frame(GenericMacHeader *gmh);	 
bool lmac_check_extension_frame(GenericMacHeader *gmh);
bool lmac_check_qos_data_frame(GenericMacHeader *gmh);
bool lmac_check_qos_null_frame(GenericMacHeader *gmh);
bool lmac_check_probe_req_frame(GenericMacHeader *gmh);
bool lmac_check_probe_rsp_frame(GenericMacHeader *gmh);
bool lmac_check_pspoll_frame(GenericMacHeader *gmh);
bool lmac_check_retry_frame(GenericMacHeader *gmh);
bool lmac_check_more_data_frame(GenericMacHeader *gmh);
bool lmac_check_beacon_frame(GenericMacHeader* gmh);
bool lmac_check_auth_frame(GenericMacHeader* gmh);
bool lmac_check_protected_frame(GenericMacHeader* gmh);
bool lmac_check_s1g_beacon_frame(GenericMacHeader *gmh);
bool lmac_check_fragmented_frame(GenericMacHeader* gmh);
bool lmac_check_amsdu_frame(GenericMacHeader* gmh);
bool lmac_check_eapol_frame(GenericMacHeader *gmh, int len);
bool lmac_check_broadc_addr_frame(uint8_t *addr);	 
bool lmac_check_nulldata_frame(GenericMacHeader* gmh);
#endif //__PROTOCOL_H__

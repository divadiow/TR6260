#ifndef WIFI_REGS_H
#define WIFI_REGS_H
#define VER 1
/*------------------------------------------------------------------------------------------------------------------------------------
=================================================================================
Module Name  : mac_register_bank
File Name    : mac_reg_bank.v
Register Num : 314
=================================================================================
---------------------------------------------------------------------------------
    Address  : 0x00900000
    Name     : RST_SW
    Bit      : 1
    R/W      : R/W
---------------------------------------------------------------------------------
   [    0]    RST_SW                        R/W  default = 0
               -Write 0 and Write 1 will reset MAC state


---------------------------------------------------------------------------------
    Address  : 0x00900004
    Name     : CONFIG
    Bit      : 26
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [   25]    CH_SWITCH_MODE                R/W  default = 0
               -Mode Selection for channel switching
                 0 : SW Mode
                 1 : HW Mode(HW switch Channel when switching BSS)

   [   24]    BSS_SWITCH_MODE               R/W  default = 1
               -Mode Selection for concurrent switching
               -This flag only covers on/off of MAC hardware
                 0 : SW Mode
                 1 : HW Mode(HW switch BSS according to the time written in CONCURRENT_TIMER_PERIOD register automatically

   [23:20]    MAC_ADDRESS_INTF_ID           R/W  default = 0
               -Each of bit represent the Interface ID of each MAC Address
                 0 : Interface ID 0
                 1 : Interface ID 1

   [19:16]    OWN_MAC_ADDRESS_SET_EN        R/W  default = 0
               -Enable its own MAC address set.
               -Each of bit represents below.
               -[3] : Enable 'STA_MAC_ADDRESS3_LOWER & UPPER' (offset 0x78 , 0x7c)
               -[2] : Enable 'STA_MAC_ADDRESS3_LOWER & UPPER' (offset 0x70 , 0x74)
               -[1] : Enable 'STA_MAC_ADDRESS3_LOWER & UPPER' (offset 0x68 , 0x6c)
               -[0] : Enable 'STA_MAC_ADDRESS3_LOWER & UPPER' (offset 0x60 , 0x64)
                 0 : Disable
                 1 : Enable

   [ 8: 7]    TIM_MATCH_MODE                R/W  default = 0
               -Description of TIM_MATCH_MODE

   [    6]    RX_BUFFER_LOOKUP_IRQ_EN       R/W  default = 1
               -Valid only if RX_BUFFER_LOOP_EN is 1
                 0 : IRQ OFF
                 1 : IRQ ON

   [    5]    RX_BUFFER_LOOKUP_MODE_TYPE    R/W  default = 1
                 0 : When hardware detects Rx buffer shortage, it attempts to transfer receiving MPDU to RX DMA even though there's not enough buffer(doesn't generate ACK)
                 1 : When a hardware detects RX buffer shortage, it discards received one(doesn't generate ACK)

   [    4]    RX_BUFFER_LOOKUP_EN           R/W  default = 1

   [    3]    RX_DMA_PROMISCUOUS_MODE       R/W  default = 0

   [    2]    RANDOM_RX_CRASH               R/W  default = 0

   [    1]    CCA_IGNORE                    R/W  default = 0

   [    0]    MAC_LOOPBACK                  R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900008
    Name     : CLOCK_GATING_CONFIG
    Bit      : 6
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [ 5: 4]    CCMP_CG_MODE                  R/W  default = 2
                 0 : Always OFF
                 1 : Always ON
                 2 : Auto

   [ 3: 2]    TKIP_CG_MODE                  R/W  default = 2
                 0 : Always OFF
                 1 : Always ON
                 2 : Auto

   [ 1: 0]    LUT_CG_MODE                   R/W  default = 2
                 0 : Always OFF
                 1 : Always ON
                 2 : Auto


---------------------------------------------------------------------------------
    Address  : 0x0090000c
    Name     : US_CLOCK
    Bit      : 8
    R/W      : R/W
---------------------------------------------------------------------------------
   [ 7: 0]    US_CLOCK                      R/W  default = 40
               -Clock Count for 1 us


---------------------------------------------------------------------------------
    Address  : 0x00900010
    Name     : SIFS_DURATION_CLOCK
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    SIFS_DURATION_CLOCK           R/W  default = 300
               -SIFS Duration represented by clock unit, Default 300 clock = 7.5usec (10usec SIFS - 2.5usec CCA Delay)


---------------------------------------------------------------------------------
    Address  : 0x00900014
    Name     : SLOT_DURATION_CLOCK
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31:16]    SLOT_DURATION_CLOCK_INTF1     R/W  default = 360
               -Slot Duration for second interface, Clcok unit in 40MHz

   [15: 0]    SLOT_DURATION_CLOCK_INTF0     R/W  default = 360
               -Slot Duration for first interface, Clock unit in 40MHz


---------------------------------------------------------------------------------
    Address  : 0x00900018
    Name     : TXPPDU_DELAY_CLOCK
    Bit      : 10
    R/W      : R/W
---------------------------------------------------------------------------------
   [ 9: 0]    TXPPDU_DELAY_CLOCK            R/W  default = 128
               -TXPPDU post delay clock. Delay to align from the end of TXPPDU to the end of CCA (128clock = 3.2usec)


---------------------------------------------------------------------------------
    Address  : 0x0090001c
    Name     : READY_BEFORE_TXPPDU_CLOCK
    Bit      : 8
    R/W      : R/W
---------------------------------------------------------------------------------
   [ 7: 0]    READY_BEFORE_TXPPDU_CLOCK     R/W  default = 160
               -Preparation time prior to actual transmit time.
               -Setting 160(4usec) means that transmit decision is made 4usec before last backoff slot time.
               -Within this time, H/W read transmits information and passes Tx vector to PHY


---------------------------------------------------------------------------------
    Address  : 0x00900020
    Name     : READY_TO_TXPOWER_CLOCK
    Bit      : 30
    R/W      : R/W
---------------------------------------------------------------------------------
   [19:10]    READY_TO_TXPOWER_CLOCK_20M    R/W  default = 64
               -Time gap between transmit decision time and Tx power time (64clock = 1.6usec)

   [ 9: 0]    READY_TO_TXPOWER_CLOCK_40M    R/W  default = 64
               -Time gap between transmit decision time and Tx power time (64clock = 1.6usec)


---------------------------------------------------------------------------------
    Address  : 0x00900024
    Name     : TXPOWER_TO_TXREQUEST_CLOCK
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    TXPOWER_TO_TXREQUEST_CLOCK    R/W  default = 40
               -Time gap between Tx power time and Tx request time


---------------------------------------------------------------------------------
    Address  : 0x00900028
    Name     : RESPONSE_TIMEOUT_US
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TIMEOUT_US           R/W  default = 64
               -Response timeout duration after transmitting packet except 11b
               -SIFS + SLOT + RXPhyStartDelay(25us at OFDM)


---------------------------------------------------------------------------------
    Address  : 0x0090002c
    Name     : RESPONSE_TIMEOUT_11B_US
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TIMEOUT_11B_US       R/W  default = 256
               -Response timeout duration after transmitting 11b packet
               -SIFS + SLOT + RXPhyStartDelay(192us at OFDM)


---------------------------------------------------------------------------------
    Address  : 0x00900030
    Name     : BSS_OPERATION_MODE
    Bit      : 4
    R/W      : R/W
---------------------------------------------------------------------------------
   [    3]    STA_MODE_BSS3                 R/W  default = 0

   [    2]    STA_MODE_BSS2                 R/W  default = 0

   [    1]    STA_MODE_BSS1                 R/W  default = 0

   [    0]    STA_MODE_BSS0                 R/W  default = 0
               -Each bit represents operating mode of device in designated BSS.
               -Valid only if dedicated bit position in BSS_ACTIVE register is enabled
                 0 : Station mode
                 1 : AP mode


---------------------------------------------------------------------------------
    Address  : 0x00900034
    Name     : STA_FILTER_MODE
    Bit      : 8
    R/W      : R/W
---------------------------------------------------------------------------------
   - Add additional filtering rule to default principle.
   - Each bit represents detailed rule. It is a kind of amendment applied to current rule of standard
   [    7]    RECEIVE_ALL                   R/W  default = 0
               -All of received frames

   [    6]    RECEIVE_ALL_EXCEPT_CONTROL_FRAMER/W  default = 0
               -All of received frames except control frames

   [    5]    RECEIVE_GROUP_ADDRESSED       R/W  default = 0
               -Group addressed frames

   [    4]    RECEIVE_CONTROL_FRAME         R/W  default = 0
               -All of control frames

   [    3]    RECEIVE_MANAGEMENT_FRAME      R/W  default = 0
               -All of management frames

   [    2]    RECEIVE_MY_CONTROL_FRAME      R/W  default = 0
               -Address matched control frames

   [    1]    RECEIVE_MY_ACK_FRAME          R/W  default = 0
               -Address matched Ack realted frames

   [    0]    RECEIVE_MY_PM_BIT_FRAME       R/W  default = 0
               -Address matched and power management enabled frames
                 0 : Disalbe
                 1 : Enable


---------------------------------------------------------------------------------
    Address  : 0x00900038
    Name     : BSS_ACTIVE
    Bit      : 8
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [ 7: 4]    BSS_INTF_ID                   R/W  default = 0
               -Each bit represents interface ID of BSS where the device join in
               -[7]: Interface ID of BSSID3
               -[6]: Interface ID of BSSID2
               -[5]: Interface ID of BSSID1
               -[4]: Interface ID of BSSID0
                 0 : Interface 0
                 1 : Interface 1

   [ 3: 0]    BSS_ACTIVE_ENABLE             R/W  default = 0
               -Each bit represents activeness of BSS
               -Software shall make it active when it attempts to associate
               -[3]: Active state of BSS3
               -[2]: Active state of BSS2
               -[1]: Active state of BSS1
               -[0]: Active state of BSS0
                 0 : Inactive
                 1 : Active


---------------------------------------------------------------------------------
    Address  : 0x0090003c
    Name     : RESPONSE_TX_POWER_1MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_1MBPS       R/W  default = 0
               -12 bit txpower when sending 1Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x00900040
    Name     : RESPONSE_TX_POWER_2MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_2MBPS       R/W  default = 0
               -12 bit txpower when sending 2Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x00900044
    Name     : RESPONSE_TX_POWER_5_5MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_5_5MBPS     R/W  default = 0
               -12 bit txpower when sending 5_5Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x00900048
    Name     : RESPONSE_TX_POWER_11MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_11MBPS      R/W  default = 0
               -12 bit txpower when sending 11Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x0090004c
    Name     : RESPONSE_TX_POWER_6MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_6MBPS       R/W  default = 0
               -12 bit txpower when sending 6Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x00900050
    Name     : RESPONSE_TX_POWER_9MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_9MBPS       R/W  default = 0
               -12 bit txpower when sending 9Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x00900054
    Name     : RESPONSE_TX_POWER_12MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_12MBPS      R/W  default = 0
               -12 bit txpower when sending 12Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x00900058
    Name     : RESPONSE_TX_POWER_18MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_18MBPS      R/W  default = 0
               -12 bit txpower when sending 18Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x0090005c
    Name     : RESPONSE_TX_POWER_24MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_24MBPS      R/W  default = 0
               -12 bit txpower when sending 24Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x00900060
    Name     : RESPONSE_TX_POWER_36MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_36MBPS      R/W  default = 0
               -12 bit txpower when sending 36Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x00900064
    Name     : RESPONSE_TX_POWER_48MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_48MBPS      R/W  default = 0
               -12 bit txpower when sending 48Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x00900068
    Name     : RESPONSE_TX_POWER_54MBPS
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_54MBPS      R/W  default = 0
               -12 bit txpower when sending 54Mbps phyrate response frame


---------------------------------------------------------------------------------
    Address  : 0x0090006c
    Name     : RESPONSE_TX_POWER_6MBPS_DUP
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_6MBPS_DUP   R/W  default = 0
               -12 bit txpower when sending 6Mbps non-ht duplicate format response frame


---------------------------------------------------------------------------------
    Address  : 0x00900070
    Name     : RESPONSE_TX_POWER_9MBPS_DUP
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_9MBPS_DUP   R/W  default = 0
               -12 bit txpower when sending 9Mbps non-ht duplicate format response frame


---------------------------------------------------------------------------------
    Address  : 0x00900074
    Name     : RESPONSE_TX_POWER_12MBPS_DUP
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_12MBPS_DUP  R/W  default = 0
               -12 bit txpower when sending 12Mbps non-ht duplicate format response frame


---------------------------------------------------------------------------------
    Address  : 0x00900078
    Name     : RESPONSE_TX_POWER_18MBPS_DUP
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_18MBPS_DUP  R/W  default = 0
               -12 bit txpower when sending 18Mbps non-ht duplicate format response frame


---------------------------------------------------------------------------------
    Address  : 0x0090007c
    Name     : RESPONSE_TX_POWER_24MBPS_DUP
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_24MBPS_DUP  R/W  default = 0
               -12 bit txpower when sending 24Mbps non-ht duplicate format response frame


---------------------------------------------------------------------------------
    Address  : 0x00900080
    Name     : RESPONSE_TX_POWER_36MBPS_DUP
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_36MBPS_DUP  R/W  default = 0
               -12 bit txpower when sending 36Mbps non-ht duplicate format response frame


---------------------------------------------------------------------------------
    Address  : 0x00900084
    Name     : RESPONSE_TX_POWER_48MBPS_DUP
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_48MBPS_DUP  R/W  default = 0
               -12 bit txpower when sending 48Mbps non-ht duplicate format response frame


---------------------------------------------------------------------------------
    Address  : 0x00900088
    Name     : RESPONSE_TX_POWER_54MBPS_DUP
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    RESPONSE_TX_POWER_54MBPS_DUP  R/W  default = 0
               -12 bit txpower when sending 54Mbps non-ht duplicate format response frame


---------------------------------------------------------------------------------
    Address  : 0x0090008c
    Name     : STA_BSSID0_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    STA_BSSID0_LOWER              R/W  default = 0
               -Lower 32bit width BSSID of the BSS0 where the device is associated with


---------------------------------------------------------------------------------
    Address  : 0x00900090
    Name     : STA_BSSID0_UPPER
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    STA_BSSID0_UPPER              R/W  default = 0
               -Upper 16bit width BSSID of the BSS0 where the device is associated with


---------------------------------------------------------------------------------
    Address  : 0x00900094
    Name     : STA_BSSID1_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    STA_BSSID1_LOWER              R/W  default = 0
               -Lower 32bit width BSSID of the BSS1 where the device is associated with


---------------------------------------------------------------------------------
    Address  : 0x00900098
    Name     : STA_BSSID1_UPPER
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    STA_BSSID1_UPPER              R/W  default = 0
               -Upper 16bit width BSSID of the BSS1 where the device is associated with


---------------------------------------------------------------------------------
    Address  : 0x0090009c
    Name     : STA_BSSID2_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    STA_BSSID2_LOWER              R/W  default = 0
               -Lower 32bit width BSSID of the BSS2 where the device is associated with


---------------------------------------------------------------------------------
    Address  : 0x009000a0
    Name     : STA_BSSID2_UPPER
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    STA_BSSID2_UPPER              R/W  default = 0
               -Upper 16bit width BSSID of the BSS2 where the device is associated with


---------------------------------------------------------------------------------
    Address  : 0x009000a4
    Name     : STA_BSSID3_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    STA_BSSID3_LOWER              R/W  default = 0
               -Lower 32bit width BSSID of the BSS3 where the device is associated with


---------------------------------------------------------------------------------
    Address  : 0x009000a8
    Name     : STA_BSSID3_UPPER
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    STA_BSSID3_UPPER              R/W  default = 0
               -Upper 16bit width BSSID of the BSS3 where the device is associated with


---------------------------------------------------------------------------------
    Address  : 0x009000ac
    Name     : STA_MAC_ADDRESS0_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    STA_MAC_ADDRESS0_LOWER        R/W  default = 0
               -Lower 32bit width MAC Address0


---------------------------------------------------------------------------------
    Address  : 0x009000b0
    Name     : STA_MAC_ADDRESS0_UPPER
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    STA_MAC_ADDRESS0_UPPER        R/W  default = 0
               -Upper 16bit width MAC Address0


---------------------------------------------------------------------------------
    Address  : 0x009000b4
    Name     : STA_MAC_ADDRESS1_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    STA_MAC_ADDRESS1_LOWER        R/W  default = 0
               -Lower 32bit width MAC Address1


---------------------------------------------------------------------------------
    Address  : 0x009000b8
    Name     : STA_MAC_ADDRESS1_UPPER
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    STA_MAC_ADDRESS1_UPPER        R/W  default = 0
               -Upper 16bit width MAC Address1


---------------------------------------------------------------------------------
    Address  : 0x009000bc
    Name     : STA_MAC_ADDRESS2_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    STA_MAC_ADDRESS2_LOWER        R/W  default = 0
               -Lower 32bit width MAC Address2


---------------------------------------------------------------------------------
    Address  : 0x009000c0
    Name     : STA_MAC_ADDRESS2_UPPER
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    STA_MAC_ADDRESS2_UPPER        R/W  default = 0
               -Upper 16bit width MAC Address2


---------------------------------------------------------------------------------
    Address  : 0x009000c4
    Name     : STA_MAC_ADDRESS3_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    STA_MAC_ADDRESS3_LOWER        R/W  default = 0
               -Lower 32bit width MAC Address3


---------------------------------------------------------------------------------
    Address  : 0x009000c8
    Name     : STA_MAC_ADDRESS3_UPPER
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    STA_MAC_ADDRESS3_UPPER        R/W  default = 0
               -Upper 16bit width MAC Address3


---------------------------------------------------------------------------------
    Address  : 0x009000cc
    Name     : STA_AID0_0
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID0_0                    R/W  default = 0
               -Granted AID0 by the AP which has identical address stated in BSSID0 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000d0
    Name     : STA_AID0_1
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID0_1                    R/W  default = 0
               -Granted AID1 by the AP which has identical address stated in BSSID0 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000d4
    Name     : STA_AID0_2
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID0_2                    R/W  default = 0
               -Granted AID2 by the AP which has identical address stated in BSSID0 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000d8
    Name     : STA_AID1_0
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID1_0                    R/W  default = 0
               -Granted AID0 by the AP which has identical address stated in BSSID1 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000dc
    Name     : STA_AID1_1
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID1_1                    R/W  default = 0
               -Granted AID1 by the AP which has identical address stated in BSSID1 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000e0
    Name     : STA_AID1_2
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID1_2                    R/W  default = 0
               -Granted AID2 by the AP which has identical address stated in BSSID1 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000e4
    Name     : STA_AID2_0
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID2_0                    R/W  default = 0
               -Granted AID0 by the AP which has identical address stated in BSSID2 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000e8
    Name     : STA_AID2_1
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID2_1                    R/W  default = 0
               -Granted AID1 by the AP which has identical address stated in BSSID2 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000ec
    Name     : STA_AID2_2
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID2_2                    R/W  default = 0
               -Granted AID2 by the AP which has identical address stated in BSSID2 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000f0
    Name     : STA_AID3_0
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID3_0                    R/W  default = 0
               -Granted AID0 by the AP which has identical address stated in BSSID3 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000f4
    Name     : STA_AID3_1
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID3_1                    R/W  default = 0
               -Granted AID1 by the AP which has identical address stated in BSSID3 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000f8
    Name     : STA_AID3_2
    Bit      : 13
    R/W      : R/W
---------------------------------------------------------------------------------
   [12: 0]    STA_AID3_2                    R/W  default = 0
               -Granted AID2 by the AP which has identical address stated in BSSID3 register of device


---------------------------------------------------------------------------------
    Address  : 0x009000fc
    Name     : DMA_BURST
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [11: 6]    DMA_BURST_MAX                 R/W  default = 16
               -Set the maximum beat of DMA Burst

   [ 5: 0]    DMA_BURST_MIN                 R/W  default = 1
               -Set the minimum beat of DMA Burst


---------------------------------------------------------------------------------
    Address  : 0x00900100
    Name     : DMA_RDFIFO_ADJUST
    Bit      : 5
    R/W      : R/W
---------------------------------------------------------------------------------
   [ 4: 0]    DMA_RDFIFO_ADJUST             R/W  default = 0
               -Internal use only
               -MAX DMA FIFO Size is subtracted from this value.
               -This register is used to find optimal size of TX DMA FIFO


---------------------------------------------------------------------------------
    Address  : 0x00900104
    Name     : STA_DETECT_11B_PREAMBLE
    Bit      : 2
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [    1]    PREAMBLE_TYPE                 R/W  default = 0
               -This preamble type is used to make control frame when format of control frame is 11b
                 0 : Long preamble
                 1 : Short preamble

   [    0]    DETECT_11B                    R/W  default = 0
                 0 : No 11B STA in current BSS
                 1 : There exist 11B STA in current BSS


---------------------------------------------------------------------------------
    Address  : 0x00900108
    Name     : STA_DETECT_11B_PREAMBLE_2ND
    Bit      : 2
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [    1]    PREAMBLE_TYPE_2ND             R/W  default = 0
               -This preamble type is used to make control frame when format of control frame is 11b and its active interface is second one
                 0 : Long preamble
                 1 : Short preamble

   [    0]    DETECT_11B_2ND                R/W  default = 0
                 0 : No 11B STA in current BSS
                 1 : There exist 11B STA in current BSS


---------------------------------------------------------------------------------
    Address  : 0x0090010c
    Name     : BASIC_RATE_BITMAP
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    BASIC_RATE_BITMAP             R/W  default = 0
               -Basic rate bitmap
               -[11]: Legacy, 54Mbps
               -[10]: Legacy, 48Mbps
               -[9] : Legacy, 36Mbps
               -[8] : Legacy, 24Mbps
               -[7] : Legacy, 18Mbps
               -[6] : Legacy, 12Mbps
               -[5] : Legacy, 9Mbps
               -[4] : Legacy, 6Mbps
               -[3] : 11B   , 11Mbps
               -[2] : 11B   , 5.5Mbps
               -[1] : 11B   , 2Mbps
               -[0] : 11B   , 1Mbps


---------------------------------------------------------------------------------
    Address  : 0x00900110
    Name     : BASIC_RATE_BITMAP_2ND
    Bit      : 12
    R/W      : R/W
---------------------------------------------------------------------------------
   [11: 0]    BASIC_RATE_BITMAP_2ND         R/W  default = 0
               -Basic rate bitmap for second interface
               -[11]: Legacy, 54Mbps
               -[10]: Legacy, 48Mbps
               -[9] : Legacy, 36Mbps
               -[8] : Legacy, 24Mbps
               -[7] : Legacy, 18Mbps
               -[6] : Legacy, 12Mbps
               -[5] : Legacy, 9Mbps
               -[4] : Legacy, 6Mbps
               -[3] : 11B   , 11Mbps
               -[2] : 11B   , 5.5Mbps
               -[1] : 11B   , 2Mbps
               -[0] : 11B   , 1Mbps


---------------------------------------------------------------------------------
    Address  : 0x00900114
    Name     : TX_SUPPRESS_COMMAND
    Bit      : 4
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [ 3: 0]    TX_SUPPRESS_COMMAND           R/W  default = 0
               -Suppression Command is as below
                 7 : Force to exit a suppression mode immediately
                 6 : Start suppression immediately and it is ended when specified duration (refer TX_SUPPRESS_DURATION register) is expired
                 5 : Start suppression immediately and it is ended when lower 32 bit TSF timer reaches specified value (refer to TX_SUPPRESS_END_TSF register)
                 4 : Start suppression when TSF reaches specified value (refer to TX_SUPPRESS_START_TSF register) and it is ended when specified duration (refer TX_SUPPRESS_DURATION) is expired since it begins suppression
                 3 : Start suppression when TSF timer reaches specified value (STAR_TSF register) and then it is scheduled to be ended when TSF timer reaches specified value (END_TSF register)
                 2 : Start suppression immediately
                 1 : Start suppression when TSF timer reaches specified value resides in 'TX_SUPPRESS_START_TSF' register. Suppression continues permanently until any external command enters
                 0 : Make an internal FSM return to idle state


---------------------------------------------------------------------------------
    Address  : 0x00900118
    Name     : TX_SUPPRESS_START_TSF
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    TX_SUPPRESS_START_TSF         R/W  default = 0
               -Start time of Tx suppression


---------------------------------------------------------------------------------
    Address  : 0x0090011c
    Name     : TX_SUPPRESS_END_TSF
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    TX_SUPPRESS_END_TSF           R/W  default = 0
               -End time of Tx suppression


---------------------------------------------------------------------------------
    Address  : 0x00900120
    Name     : TX_SUPPRESS_DURATION
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    TX_SUPPRESS_DURATION          R/W  default = 1024
               -Duration of Tx suppression


---------------------------------------------------------------------------------
    Address  : 0x00900124
    Name     : TX_SUPPRESS_SETTING
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    TX_SUPPRESS_SETTING           R/W  default = 32735
               -Specify the queue not to transmit
               -[15]: SIFS Queue
               -[14:11]: Reserved
               -[10]: Queue10
               -[9] : Queue9
               -[8] : Queue8
               -[7] : Queue7
               -[6] : Queue6
               -[5] : Queue5
               -[4] : Queue4
               -[3] : Queue3
               -[2] : Queue2
               -[1] : Queue1
               -[0] : Queue0


---------------------------------------------------------------------------------
    Address  : 0x00900128
    Name     : TX_CONTROL_PARAMETER
    Bit      : 20
    R/W      : R/W
---------------------------------------------------------------------------------
   [19:18]    RX_RESPONSE_CONTROL           R/W  default = 0
                 0 : Accept received frame as correct response frame only if Address matched and CRC succeed
                 1 : Accept received frame as correct response frame for any frame which CRC was succeed
                 2 : Accept received frame as correct response frame for any frame received during response timeout period

   [   17]    TX_ERROR_RECOVERY             R/W  default = 1
               -TX Control runs recovery process if PHY TX has been ended (TXPPDU signal goes down) while TX Control state is still processing

   [   16]    CCA_LATE_DETECTION            R/W  default = 0
               -TX Control runs recovery process after TX Ready when CCA has been detected before RF TX Power ON

   [15:12]    OTHER_FRAG_DATA_WORD_OFFSET   R/W  default = 0
               -Other Fragment Data word offset after 36byte(buffer header)

   [11: 8]    FIRST_FRAG_DATA_WORD_OFFSET   R/W  default = 7
               -First  Fragment Data word offset after 48byte(buffer header + vector)

   [ 7: 0]    RESPONSE_WAIT_TIME_US         R/W  default = 2
               -S/W Response wait timeout in us


---------------------------------------------------------------------------------
    Address  : 0x0090012c
    Name     : RSP0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received Not OK Frame : Send Nothing
   [   31]    RSP0_VALID                    R/W  default = 1

   [30:26]    RSP0_CASE                     R/W  default = 0

   [   25]    RSP0_MASK_RX_INFO_OK          R/W  default = 1

   [   24]    RSP0_MASK_RX_INFO_NDP         R/W  default = 0

   [   23]    RSP0_MASK_RX_NDP_PSPOLL       R/W  default = 0

   [22:21]    RSP0_MASK_RX_PV0_TYPE         R/W  default = 0

   [20:17]    RSP0_MASK_RX_PV0_SUBTYPE      R/W  default = 0

   [   16]    RSP0_MASK_RX_PV               R/W  default = 0

   [   15]    RSP0_MASK_GROUP_ADDRESS       R/W  default = 0

   [   14]    RSP0_MASK_OPERATION           R/W  default = 0

   [   13]    RSP0_MASK_RX_INFO_AGG         R/W  default = 0

   [   12]    RSP0_RX_INFO_OK               R/W  default = 0

   [   11]    RSP0_RX_INFO_NDP              R/W  default = 0

   [   10]    RSP0_RX_NDP_PSPOLL            R/W  default = 0

   [ 9: 8]    RSP0_RX_PV0_TYPE              R/W  default = 0
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP0_RX_PV0_SUBTYPE           R/W  default = 0

   [    3]    RSP0_RX_PV                    R/W  default = 0

   [    2]    RSP0_GROUP_ADDRESS            R/W  default = 0

   [    1]    RSP0_OPERATION                R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP0_RX_INFO_AGG              R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900130
    Name     : RSP1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received Beacon Frame : Send Nothing
   [   31]    RSP1_VALID                    R/W  default = 1

   [30:26]    RSP1_CASE                     R/W  default = 0

   [   25]    RSP1_MASK_RX_INFO_OK          R/W  default = 1

   [   24]    RSP1_MASK_RX_INFO_NDP         R/W  default = 1

   [   23]    RSP1_MASK_RX_NDP_PSPOLL       R/W  default = 0

   [22:21]    RSP1_MASK_RX_PV0_TYPE         R/W  default = 3

   [20:17]    RSP1_MASK_RX_PV0_SUBTYPE      R/W  default = 15

   [   16]    RSP1_MASK_RX_PV               R/W  default = 1

   [   15]    RSP1_MASK_GROUP_ADDRESS       R/W  default = 0

   [   14]    RSP1_MASK_OPERATION           R/W  default = 0

   [   13]    RSP1_MASK_RX_INFO_AGG         R/W  default = 0

   [   12]    RSP1_RX_INFO_OK               R/W  default = 1

   [   11]    RSP1_RX_INFO_NDP              R/W  default = 0

   [   10]    RSP1_RX_NDP_PSPOLL            R/W  default = 0

   [ 9: 8]    RSP1_RX_PV0_TYPE              R/W  default = 0
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP1_RX_PV0_SUBTYPE           R/W  default = 8

   [    3]    RSP1_RX_PV                    R/W  default = 0

   [    2]    RSP1_GROUP_ADDRESS            R/W  default = 0

   [    1]    RSP1_OPERATION                R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP1_RX_INFO_AGG              R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900134
    Name     : RSP2
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received Action NoACK Frame : Send Nothing
   [   31]    RSP2_VALID                    R/W  default = 1

   [30:26]    RSP2_CASE                     R/W  default = 0

   [   25]    RSP2_MASK_RX_INFO_OK          R/W  default = 1

   [   24]    RSP2_MASK_RX_INFO_NDP         R/W  default = 1

   [   23]    RSP2_MASK_RX_NDP_PSPOLL       R/W  default = 0

   [22:21]    RSP2_MASK_RX_PV0_TYPE         R/W  default = 3

   [20:17]    RSP2_MASK_RX_PV0_SUBTYPE      R/W  default = 15

   [   16]    RSP2_MASK_RX_PV               R/W  default = 1

   [   15]    RSP2_MASK_GROUP_ADDRESS       R/W  default = 0

   [   14]    RSP2_MASK_OPERATION           R/W  default = 0

   [   13]    RSP2_MASK_RX_INFO_AGG         R/W  default = 0

   [   12]    RSP2_RX_INFO_OK               R/W  default = 1

   [   11]    RSP2_RX_INFO_NDP              R/W  default = 0

   [   10]    RSP2_RX_NDP_PSPOLL            R/W  default = 0

   [ 9: 8]    RSP2_RX_PV0_TYPE              R/W  default = 0
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP2_RX_PV0_SUBTYPE           R/W  default = 14

   [    3]    RSP2_RX_PV                    R/W  default = 0

   [    2]    RSP2_GROUP_ADDRESS            R/W  default = 0

   [    1]    RSP2_OPERATION                R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP2_RX_INFO_AGG              R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900138
    Name     : RSP3
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received Group Addressed Management  Frame : Send Nothing
   [   31]    RSP3_VALID                    R/W  default = 1

   [30:26]    RSP3_CASE                     R/W  default = 0

   [   25]    RSP3_MASK_RX_INFO_OK          R/W  default = 1

   [   24]    RSP3_MASK_RX_INFO_NDP         R/W  default = 1

   [   23]    RSP3_MASK_RX_NDP_PSPOLL       R/W  default = 0

   [22:21]    RSP3_MASK_RX_PV0_TYPE         R/W  default = 3

   [20:17]    RSP3_MASK_RX_PV0_SUBTYPE      R/W  default = 0

   [   16]    RSP3_MASK_RX_PV               R/W  default = 1

   [   15]    RSP3_MASK_GROUP_ADDRESS       R/W  default = 1

   [   14]    RSP3_MASK_OPERATION           R/W  default = 0

   [   13]    RSP3_MASK_RX_INFO_AGG         R/W  default = 0

   [   12]    RSP3_RX_INFO_OK               R/W  default = 1

   [   11]    RSP3_RX_INFO_NDP              R/W  default = 0

   [   10]    RSP3_RX_NDP_PSPOLL            R/W  default = 0

   [ 9: 8]    RSP3_RX_PV0_TYPE              R/W  default = 0
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP3_RX_PV0_SUBTYPE           R/W  default = 0

   [    3]    RSP3_RX_PV                    R/W  default = 0

   [    2]    RSP3_GROUP_ADDRESS            R/W  default = 1

   [    1]    RSP3_OPERATION                R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP3_RX_INFO_AGG              R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x0090013c
    Name     : RSP4
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received Management  Frame : Send ACK
   [   31]    RSP4_VALID                    R/W  default = 1

   [30:26]    RSP4_CASE                     R/W  default = 1

   [   25]    RSP4_MASK_RX_INFO_OK          R/W  default = 1

   [   24]    RSP4_MASK_RX_INFO_NDP         R/W  default = 1

   [   23]    RSP4_MASK_RX_NDP_PSPOLL       R/W  default = 0

   [22:21]    RSP4_MASK_RX_PV0_TYPE         R/W  default = 3

   [20:17]    RSP4_MASK_RX_PV0_SUBTYPE      R/W  default = 0

   [   16]    RSP4_MASK_RX_PV               R/W  default = 1

   [   15]    RSP4_MASK_GROUP_ADDRESS       R/W  default = 0

   [   14]    RSP4_MASK_OPERATION           R/W  default = 0

   [   13]    RSP4_MASK_RX_INFO_AGG         R/W  default = 0

   [   12]    RSP4_RX_INFO_OK               R/W  default = 1

   [   11]    RSP4_RX_INFO_NDP              R/W  default = 0

   [   10]    RSP4_RX_NDP_PSPOLL            R/W  default = 0

   [ 9: 8]    RSP4_RX_PV0_TYPE              R/W  default = 0
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP4_RX_PV0_SUBTYPE           R/W  default = 0

   [    3]    RSP4_RX_PV                    R/W  default = 0

   [    2]    RSP4_GROUP_ADDRESS            R/W  default = 0

   [    1]    RSP4_OPERATION                R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP4_RX_INFO_AGG              R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900140
    Name     : RSP5
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received RTS Frame in STA Mode: Send CTS
   [   31]    RSP5_VALID                    R/W  default = 1

   [30:26]    RSP5_CASE                     R/W  default = 3

   [   25]    RSP5_MASK_RX_INFO_OK          R/W  default = 1

   [   24]    RSP5_MASK_RX_INFO_NDP         R/W  default = 1

   [   23]    RSP5_MASK_RX_NDP_PSPOLL       R/W  default = 0

   [22:21]    RSP5_MASK_RX_PV0_TYPE         R/W  default = 3

   [20:17]    RSP5_MASK_RX_PV0_SUBTYPE      R/W  default = 15

   [   16]    RSP5_MASK_RX_PV               R/W  default = 1

   [   15]    RSP5_MASK_GROUP_ADDRESS       R/W  default = 0

   [   14]    RSP5_MASK_OPERATION           R/W  default = 1

   [   13]    RSP5_MASK_RX_INFO_AGG         R/W  default = 0

   [   12]    RSP5_RX_INFO_OK               R/W  default = 1

   [   11]    RSP5_RX_INFO_NDP              R/W  default = 0

   [   10]    RSP5_RX_NDP_PSPOLL            R/W  default = 0

   [ 9: 8]    RSP5_RX_PV0_TYPE              R/W  default = 1
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP5_RX_PV0_SUBTYPE           R/W  default = 11

   [    3]    RSP5_RX_PV                    R/W  default = 0

   [    2]    RSP5_GROUP_ADDRESS            R/W  default = 0

   [    1]    RSP5_OPERATION                R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP5_RX_INFO_AGG              R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900144
    Name     : RSP6
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received RTS Frame in AP Mode: Send CTS
   [   31]    RSP6_VALID                    R/W  default = 1

   [30:26]    RSP6_CASE                     R/W  default = 3

   [   25]    RSP6_MASK_RX_INFO_OK          R/W  default = 1

   [   24]    RSP6_MASK_RX_INFO_NDP         R/W  default = 1

   [   23]    RSP6_MASK_RX_NDP_PSPOLL       R/W  default = 0

   [22:21]    RSP6_MASK_RX_PV0_TYPE         R/W  default = 3

   [20:17]    RSP6_MASK_RX_PV0_SUBTYPE      R/W  default = 15

   [   16]    RSP6_MASK_RX_PV               R/W  default = 1

   [   15]    RSP6_MASK_GROUP_ADDRESS       R/W  default = 0

   [   14]    RSP6_MASK_OPERATION           R/W  default = 1

   [   13]    RSP6_MASK_RX_INFO_AGG         R/W  default = 0

   [   12]    RSP6_RX_INFO_OK               R/W  default = 1

   [   11]    RSP6_RX_INFO_NDP              R/W  default = 0

   [   10]    RSP6_RX_NDP_PSPOLL            R/W  default = 0

   [ 9: 8]    RSP6_RX_PV0_TYPE              R/W  default = 1
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP6_RX_PV0_SUBTYPE           R/W  default = 11

   [    3]    RSP6_RX_PV                    R/W  default = 0

   [    2]    RSP6_GROUP_ADDRESS            R/W  default = 0

   [    1]    RSP6_OPERATION                R/W  default = 1
                 0 : STA
                 1 : AP

   [    0]    RSP6_RX_INFO_AGG              R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900148
    Name     : RSP7
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received PS-POLL Frame : Send ACK
   [   31]    RSP7_VALID                    R/W  default = 1

   [30:26]    RSP7_CASE                     R/W  default = 1

   [   25]    RSP7_MASK_RX_INFO_OK          R/W  default = 1

   [   24]    RSP7_MASK_RX_INFO_NDP         R/W  default = 1

   [   23]    RSP7_MASK_RX_NDP_PSPOLL       R/W  default = 0

   [22:21]    RSP7_MASK_RX_PV0_TYPE         R/W  default = 3

   [20:17]    RSP7_MASK_RX_PV0_SUBTYPE      R/W  default = 15

   [   16]    RSP7_MASK_RX_PV               R/W  default = 1

   [   15]    RSP7_MASK_GROUP_ADDRESS       R/W  default = 0

   [   14]    RSP7_MASK_OPERATION           R/W  default = 0

   [   13]    RSP7_MASK_RX_INFO_AGG         R/W  default = 0

   [   12]    RSP7_RX_INFO_OK               R/W  default = 1

   [   11]    RSP7_RX_INFO_NDP              R/W  default = 0

   [   10]    RSP7_RX_NDP_PSPOLL            R/W  default = 0

   [ 9: 8]    RSP7_RX_PV0_TYPE              R/W  default = 1
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP7_RX_PV0_SUBTYPE           R/W  default = 10

   [    3]    RSP7_RX_PV                    R/W  default = 0

   [    2]    RSP7_GROUP_ADDRESS            R/W  default = 0

   [    1]    RSP7_OPERATION                R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP7_RX_INFO_AGG              R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x0090014c
    Name     : RSP8
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received BAR Frame : Send BA
   [   31]    RSP8_VALID                    R/W  default = 1

   [30:26]    RSP8_CASE                     R/W  default = 2

   [   25]    RSP8_MASK_RX_INFO_OK          R/W  default = 1

   [   24]    RSP8_MASK_RX_INFO_NDP         R/W  default = 1

   [   23]    RSP8_MASK_RX_NDP_PSPOLL       R/W  default = 0

   [22:21]    RSP8_MASK_RX_PV0_TYPE         R/W  default = 3

   [20:17]    RSP8_MASK_RX_PV0_SUBTYPE      R/W  default = 15

   [   16]    RSP8_MASK_RX_PV               R/W  default = 1

   [   15]    RSP8_MASK_GROUP_ADDRESS       R/W  default = 0

   [   14]    RSP8_MASK_OPERATION           R/W  default = 0

   [   13]    RSP8_MASK_RX_INFO_AGG         R/W  default = 0

   [   12]    RSP8_RX_INFO_OK               R/W  default = 1

   [   11]    RSP8_RX_INFO_NDP              R/W  default = 0

   [   10]    RSP8_RX_NDP_PSPOLL            R/W  default = 0

   [ 9: 8]    RSP8_RX_PV0_TYPE              R/W  default = 1
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP8_RX_PV0_SUBTYPE           R/W  default = 8

   [    3]    RSP8_RX_PV                    R/W  default = 0

   [    2]    RSP8_GROUP_ADDRESS            R/W  default = 0

   [    1]    RSP8_OPERATION                R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP8_RX_INFO_AGG              R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900150
    Name     : RSP9
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received Groupaddress Data Frame : Send Nothing
   [   31]    RSP9_VALID                    R/W  default = 1

   [30:26]    RSP9_CASE                     R/W  default = 0

   [   25]    RSP9_MASK_RX_INFO_OK          R/W  default = 1

   [   24]    RSP9_MASK_RX_INFO_NDP         R/W  default = 1

   [   23]    RSP9_MASK_RX_NDP_PSPOLL       R/W  default = 0

   [22:21]    RSP9_MASK_RX_PV0_TYPE         R/W  default = 3

   [20:17]    RSP9_MASK_RX_PV0_SUBTYPE      R/W  default = 0

   [   16]    RSP9_MASK_RX_PV               R/W  default = 1

   [   15]    RSP9_MASK_GROUP_ADDRESS       R/W  default = 1

   [   14]    RSP9_MASK_OPERATION           R/W  default = 0

   [   13]    RSP9_MASK_RX_INFO_AGG         R/W  default = 0

   [   12]    RSP9_RX_INFO_OK               R/W  default = 1

   [   11]    RSP9_RX_INFO_NDP              R/W  default = 0

   [   10]    RSP9_RX_NDP_PSPOLL            R/W  default = 0

   [ 9: 8]    RSP9_RX_PV0_TYPE              R/W  default = 2
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP9_RX_PV0_SUBTYPE           R/W  default = 0

   [    3]    RSP9_RX_PV                    R/W  default = 0

   [    2]    RSP9_GROUP_ADDRESS            R/W  default = 1

   [    1]    RSP9_OPERATION                R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP9_RX_INFO_AGG              R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900154
    Name     : RSP10
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received Single Data Frame : Send ACK
   [   31]    RSP10_VALID                   R/W  default = 1

   [30:26]    RSP10_CASE                    R/W  default = 1

   [   25]    RSP10_MASK_RX_INFO_OK         R/W  default = 1

   [   24]    RSP10_MASK_RX_INFO_NDP        R/W  default = 1

   [   23]    RSP10_MASK_RX_NDP_PSPOLL      R/W  default = 0

   [22:21]    RSP10_MASK_RX_PV0_TYPE        R/W  default = 3

   [20:17]    RSP10_MASK_RX_PV0_SUBTYPE     R/W  default = 15

   [   16]    RSP10_MASK_RX_PV              R/W  default = 1

   [   15]    RSP10_MASK_GROUP_ADDRESS      R/W  default = 0

   [   14]    RSP10_MASK_OPERATION          R/W  default = 0

   [   13]    RSP10_MASK_RX_INFO_AGG        R/W  default = 1

   [   12]    RSP10_RX_INFO_OK              R/W  default = 1

   [   11]    RSP10_RX_INFO_NDP             R/W  default = 0

   [   10]    RSP10_RX_NDP_PSPOLL           R/W  default = 0

   [ 9: 8]    RSP10_RX_PV0_TYPE             R/W  default = 2
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP10_RX_PV0_SUBTYPE          R/W  default = 0

   [    3]    RSP10_RX_PV                   R/W  default = 0

   [    2]    RSP10_GROUP_ADDRESS           R/W  default = 0

   [    1]    RSP10_OPERATION               R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP10_RX_INFO_AGG             R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900158
    Name     : RSP11
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received A-MPDU Frame : Send BA
   [   31]    RSP11_VALID                   R/W  default = 1

   [30:26]    RSP11_CASE                    R/W  default = 2

   [   25]    RSP11_MASK_RX_INFO_OK         R/W  default = 1

   [   24]    RSP11_MASK_RX_INFO_NDP        R/W  default = 1

   [   23]    RSP11_MASK_RX_NDP_PSPOLL      R/W  default = 0

   [22:21]    RSP11_MASK_RX_PV0_TYPE        R/W  default = 3

   [20:17]    RSP11_MASK_RX_PV0_SUBTYPE     R/W  default = 15

   [   16]    RSP11_MASK_RX_PV              R/W  default = 1

   [   15]    RSP11_MASK_GROUP_ADDRESS      R/W  default = 0

   [   14]    RSP11_MASK_OPERATION          R/W  default = 0

   [   13]    RSP11_MASK_RX_INFO_AGG        R/W  default = 1

   [   12]    RSP11_RX_INFO_OK              R/W  default = 1

   [   11]    RSP11_RX_INFO_NDP             R/W  default = 0

   [   10]    RSP11_RX_NDP_PSPOLL           R/W  default = 0

   [ 9: 8]    RSP11_RX_PV0_TYPE             R/W  default = 2
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP11_RX_PV0_SUBTYPE          R/W  default = 0

   [    3]    RSP11_RX_PV                   R/W  default = 0

   [    2]    RSP11_GROUP_ADDRESS           R/W  default = 0

   [    1]    RSP11_OPERATION               R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP11_RX_INFO_AGG             R/W  default = 1


---------------------------------------------------------------------------------
    Address  : 0x0090015c
    Name     : RSP12
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received NULL Data Frame : Send Ack
   [   31]    RSP12_VALID                   R/W  default = 1

   [30:26]    RSP12_CASE                    R/W  default = 1

   [   25]    RSP12_MASK_RX_INFO_OK         R/W  default = 1

   [   24]    RSP12_MASK_RX_INFO_NDP        R/W  default = 1

   [   23]    RSP12_MASK_RX_NDP_PSPOLL      R/W  default = 0

   [22:21]    RSP12_MASK_RX_PV0_TYPE        R/W  default = 3

   [20:17]    RSP12_MASK_RX_PV0_SUBTYPE     R/W  default = 15

   [   16]    RSP12_MASK_RX_PV              R/W  default = 1

   [   15]    RSP12_MASK_GROUP_ADDRESS      R/W  default = 0

   [   14]    RSP12_MASK_OPERATION          R/W  default = 0

   [   13]    RSP12_MASK_RX_INFO_AGG        R/W  default = 0

   [   12]    RSP12_RX_INFO_OK              R/W  default = 1

   [   11]    RSP12_RX_INFO_NDP             R/W  default = 0

   [   10]    RSP12_RX_NDP_PSPOLL           R/W  default = 0

   [ 9: 8]    RSP12_RX_PV0_TYPE             R/W  default = 2
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP12_RX_PV0_SUBTYPE          R/W  default = 4

   [    3]    RSP12_RX_PV                   R/W  default = 0

   [    2]    RSP12_GROUP_ADDRESS           R/W  default = 0

   [    1]    RSP12_OPERATION               R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP12_RX_INFO_AGG             R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900160
    Name     : RSP13
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received QoS Data : Send Ack
   [   31]    RSP13_VALID                   R/W  default = 1

   [30:26]    RSP13_CASE                    R/W  default = 1

   [   25]    RSP13_MASK_RX_INFO_OK         R/W  default = 1

   [   24]    RSP13_MASK_RX_INFO_NDP        R/W  default = 1

   [   23]    RSP13_MASK_RX_NDP_PSPOLL      R/W  default = 0

   [22:21]    RSP13_MASK_RX_PV0_TYPE        R/W  default = 3

   [20:17]    RSP13_MASK_RX_PV0_SUBTYPE     R/W  default = 15

   [   16]    RSP13_MASK_RX_PV              R/W  default = 1

   [   15]    RSP13_MASK_GROUP_ADDRESS      R/W  default = 0

   [   14]    RSP13_MASK_OPERATION          R/W  default = 0

   [   13]    RSP13_MASK_RX_INFO_AGG        R/W  default = 1

   [   12]    RSP13_RX_INFO_OK              R/W  default = 1

   [   11]    RSP13_RX_INFO_NDP             R/W  default = 0

   [   10]    RSP13_RX_NDP_PSPOLL           R/W  default = 0

   [ 9: 8]    RSP13_RX_PV0_TYPE             R/W  default = 2
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP13_RX_PV0_SUBTYPE          R/W  default = 8

   [    3]    RSP13_RX_PV                   R/W  default = 0

   [    2]    RSP13_GROUP_ADDRESS           R/W  default = 0

   [    1]    RSP13_OPERATION               R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP13_RX_INFO_AGG             R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900164
    Name     : RSP14
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received A-MPDU QoS Data : Send BA
   [   31]    RSP14_VALID                   R/W  default = 1

   [30:26]    RSP14_CASE                    R/W  default = 2

   [   25]    RSP14_MASK_RX_INFO_OK         R/W  default = 1

   [   24]    RSP14_MASK_RX_INFO_NDP        R/W  default = 1

   [   23]    RSP14_MASK_RX_NDP_PSPOLL      R/W  default = 0

   [22:21]    RSP14_MASK_RX_PV0_TYPE        R/W  default = 3

   [20:17]    RSP14_MASK_RX_PV0_SUBTYPE     R/W  default = 15

   [   16]    RSP14_MASK_RX_PV              R/W  default = 1

   [   15]    RSP14_MASK_GROUP_ADDRESS      R/W  default = 0

   [   14]    RSP14_MASK_OPERATION          R/W  default = 0

   [   13]    RSP14_MASK_RX_INFO_AGG        R/W  default = 1

   [   12]    RSP14_RX_INFO_OK              R/W  default = 1

   [   11]    RSP14_RX_INFO_NDP             R/W  default = 0

   [   10]    RSP14_RX_NDP_PSPOLL           R/W  default = 0

   [ 9: 8]    RSP14_RX_PV0_TYPE             R/W  default = 2
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP14_RX_PV0_SUBTYPE          R/W  default = 8

   [    3]    RSP14_RX_PV                   R/W  default = 0

   [    2]    RSP14_GROUP_ADDRESS           R/W  default = 0

   [    1]    RSP14_OPERATION               R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP14_RX_INFO_AGG             R/W  default = 1


---------------------------------------------------------------------------------
    Address  : 0x00900168
    Name     : RSP15
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - This register describes which control frame to send as a response frame when frame is received
   - Received QoS NULL Data : Send Ack
   [   31]    RSP15_VALID                   R/W  default = 1

   [30:26]    RSP15_CASE                    R/W  default = 1

   [   25]    RSP15_MASK_RX_INFO_OK         R/W  default = 1

   [   24]    RSP15_MASK_RX_INFO_NDP        R/W  default = 1

   [   23]    RSP15_MASK_RX_NDP_PSPOLL      R/W  default = 0

   [22:21]    RSP15_MASK_RX_PV0_TYPE        R/W  default = 3

   [20:17]    RSP15_MASK_RX_PV0_SUBTYPE     R/W  default = 15

   [   16]    RSP15_MASK_RX_PV              R/W  default = 1

   [   15]    RSP15_MASK_GROUP_ADDRESS      R/W  default = 0

   [   14]    RSP15_MASK_OPERATION          R/W  default = 0

   [   13]    RSP15_MASK_RX_INFO_AGG        R/W  default = 0

   [   12]    RSP15_RX_INFO_OK              R/W  default = 1

   [   11]    RSP15_RX_INFO_NDP             R/W  default = 0

   [   10]    RSP15_RX_NDP_PSPOLL           R/W  default = 0

   [ 9: 8]    RSP15_RX_PV0_TYPE             R/W  default = 2
                 0 : MANAGEMENT
                 1 : CONTROL
                 2 : DATA

   [ 7: 4]    RSP15_RX_PV0_SUBTYPE          R/W  default = 12

   [    3]    RSP15_RX_PV                   R/W  default = 0

   [    2]    RSP15_GROUP_ADDRESS           R/W  default = 0

   [    1]    RSP15_OPERATION               R/W  default = 0
                 0 : STA
                 1 : AP

   [    0]    RSP15_RX_INFO_AGG             R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x0090016c
    Name     : RSP_CASE0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - Response Case 0 : Send Nothing
   - CF_TYPE_NONE : 0
   - CF_TYPE_CTS  : 2
   - CF_TYPE_ACK  : 3
   - CF_TYPE_BA   : 4
   [   31]    CASE0_IRQ_NO_ACK              R/W  default = 0

   [28:24]    CASE0_CF_TYPE_NO_ACK          R/W  default = 0

   [   15]    CASE0_IRQ_NORMAL_ACK          R/W  default = 0

   [12: 8]    CASE0_CF_TYPE_NORMAL_ACK      R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900170
    Name     : RSP_CASE1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - Response Case 1 : Send ACK
   - CF_TYPE_NONE : 0
   - CF_TYPE_CTS  : 2
   - CF_TYPE_ACK  : 3
   - CF_TYPE_BA   : 4
   [   31]    CASE1_IRQ_NO_ACK              R/W  default = 0

   [28:24]    CASE1_CF_TYPE_NO_ACK          R/W  default = 0

   [   15]    CASE1_IRQ_NORMAL_ACK          R/W  default = 0

   [12: 8]    CASE1_CF_TYPE_NORMAL_ACK      R/W  default = 3


---------------------------------------------------------------------------------
    Address  : 0x00900174
    Name     : RSP_CASE2
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - Response Case 2 : Send BA
   - CF_TYPE_NONE : 0
   - CF_TYPE_CTS  : 2
   - CF_TYPE_ACK  : 3
   - CF_TYPE_BA   : 4
   [   31]    CASE2_IRQ_NO_ACK              R/W  default = 0

   [28:24]    CASE2_CF_TYPE_NO_ACK          R/W  default = 0

   [   15]    CASE2_IRQ_NORMAL_ACK          R/W  default = 0

   [12: 8]    CASE2_CF_TYPE_NORMAL_ACK      R/W  default = 4


---------------------------------------------------------------------------------
    Address  : 0x00900178
    Name     : RSP_CASE3
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   - Response Case 3 : Send CTS
   - CF_TYPE_NONE : 0
   - CF_TYPE_CTS  : 2
   - CF_TYPE_ACK  : 3
   - CF_TYPE_BA   : 4
   [   31]    CASE3_IRQ_NO_ACK              R/W  default = 0

   [28:24]    CASE3_CF_TYPE_NO_ACK          R/W  default = 0

   [   15]    CASE3_IRQ_NORMAL_ACK          R/W  default = 0

   [12: 8]    CASE3_CF_TYPE_NORMAL_ACK      R/W  default = 2


---------------------------------------------------------------------------------
    Address  : 0x0090017c
    Name     : RSP_CASE4
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RSP_CASE4                     R/W  default = 0
               -Response Frame Type


---------------------------------------------------------------------------------
    Address  : 0x00900180
    Name     : RSP_CASE5
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RSP_CASE5                     R/W  default = 0
               -Response Frame Type


---------------------------------------------------------------------------------
    Address  : 0x00900184
    Name     : AHB_WAIT_MODE_WR
    Bit      : 25
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [   24]    AHB_WAIT_MODE_WR_ON           R/W  default = 0
               -Waiting mode for AHB writing
                 0 : OFF
                 1 : ON

   [23: 8]    AHB_WR_WAIT_CLOCK             R/W  default = 10000
               -The number of waiting clock

   [ 7: 0]    AHB_WR_WAIT_INTERVAL          R/W  default = 200
               -The number of waiting AHB bus transaction


---------------------------------------------------------------------------------
    Address  : 0x00900188
    Name     : AHB_WAIT_MODE_RD
    Bit      : 25
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [   24]    AHB_WAIT_MODE_RD_ON           R/W  default = 0
               -Waiting mode for AHB reading
                 0 : OFF
                 1 : ON

   [23: 8]    AHB_RD_WAIT_CLOCK             R/W  default = 100
               -The number of waiting clock

   [ 7: 0]    AHB_RD_WAIT_INTERVAL          R/W  default = 20
               -The number of waiting AHB bus transaction


---------------------------------------------------------------------------------
    Address  : 0x0090018c
    Name     : DEV_INTF_MODE
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate + write_enable 
---------------------------------------------------------------------------------
   [    4]    RESET_CHANNEL_EN              R/W  default = 0
               -Reset Channel Estimator
                 0 : No action
                 1 : Reset channel estimator

   [    0]    DEV_INTF_ID                   R/W  default = 0
               -Interface ID valid only if SW switching mode is activated
                 0 : First interface
                 1 : Second interface


---------------------------------------------------------------------------------
    Address  : 0x00900190
    Name     : DEV_INTF_CONFIG_BITMAP
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31:16]    2ND_INTF                      R/W  default = 34784
               -Upper 16bit is allocated for second interface
               -[31]: Allocatd for SIFS
               -[30:27]: Reserved
               -[26:16]: Allocated to Queue10 ~ Queue0 in order

   [15: 0]    1ST_INTF                      R/W  default = 32831
               -Lower 16bit is allocated for first interface
               -[15]: Allocatd for SIFS
               -[14:11]: Reserved
               -[10:0]: Allocated to Queue10 ~ Queue0 in order1


---------------------------------------------------------------------------------
    Address  : 0x00900194
    Name     : CONCURRENT_TIMER_PERIOD0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    CONCURRENT_TIMER_PERIOD0      R/W  default = 0
               -Set period for first BSS (usec resolution)


---------------------------------------------------------------------------------
    Address  : 0x00900198
    Name     : CONCURRENT_TIMER_PERIOD1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    CONCURRENT_TIMER_PERIOD1      R/W  default = 0
               -Set period for second BSS (usec resolution)


---------------------------------------------------------------------------------
    Address  : 0x0090019c
    Name     : CONCURRENT_TIMER_COMMAND
    Bit      : 4
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [ 3: 0]    CONCURRENT_TIMER_COMMAND      R/W  default = 0
                 0 : Start timer which is starting from first BSS
                 1 : Start timer which is starting from second BSS
                 2 : Stop timer and stay at current BSS
                 3 : Resume timer
                 4 : Activate both BSS
                 5 : End concurrent mode


---------------------------------------------------------------------------------
    Address  : 0x009001a0
    Name     : NULL_FRAME_INFO0
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [26:17]    NULL0_RETRY_LIMIT             R/W  default = 10
               -Retry limit of null frame transmission (msec unit)

   [16:13]    NULL0_RETRY_COUNT             R/W  default = 8
               -Retry count of null frame transmission including first attempt

   [   12]    NULL0_REQUEST                 R/W  default = 1
               -Request to send null frame using second inteface BSSID1

   [11:10]    NULL0_BSSID_INDEX             R/W  default = 0

   [ 9: 8]    NULL0_MAC_ADDR_INDEX          R/W  default = 0

   [    7]    NULL0_MCS32                   R/W  default = 1
               -Use MCS32 modulation

   [ 6: 4]    NULL0_MCS                     R/W  default = 0
               -Specifies MCS

   [    3]    NULL0_GI_TYPE                 R/W  default = 0
                 0 : Long
                 1 : Short

   [ 2: 0]    NULL0_FORMAT                  R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x009001a4
    Name     : NULL_FRAME_INFO1
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [26:17]    NULL1_RETRY_LIMIT             R/W  default = 10
               -Retry limit of null frame transmission (msec unit)

   [16:13]    NULL1_RETRY_COUNT             R/W  default = 8
               -Retry count of null frame transmission including first attempt

   [   12]    NULL1_REQUEST                 R/W  default = 1
               -Request to send null frame using first inteface BSSID0

   [11:10]    NULL1_BSSID_INDEX             R/W  default = 1

   [ 9: 8]    NULL1_MAC_ADDR_INDEX          R/W  default = 1

   [    7]    NULL1_MCS32                   R/W  default = 1
               -Use MCS32 modulation

   [ 6: 4]    NULL1_MCS                     R/W  default = 0
               -Specifies MCS

   [    3]    NULL1_GI_TYPE                 R/W  default = 0
                 0 : Long
                 1 : Short

   [ 2: 0]    NULL1_FORMAT                  R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x009001a8
    Name     : TX_COMMAND_0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31:16]    BUFFER_WORD_OFFSET            R/W  default = 0
               -Word offset from TX_BASE_ADDRESS
               -BUFFER_WORD_OFFSET = (BUFFER Address - TX_BASE_ADDRESS) >> 2
               -For example, when TX_BASE_ADDRESS == 0x12340000,
               -BUFFER_WORD_OFFSET of BUFFERADDRESS 0x12340008 is 2

   [15:14]    BANDWIDTH                     R/W  default = 0
               -Bandwidth of the frame

   [13:10]    AIFSN                         R/W  default = 0
               -AIFSN value used at Random Backoff

   [ 9: 0]    CW_COUNT                      R/W  default = 0
               -RANDOM CW Number (0 ~ Current CW)


---------------------------------------------------------------------------------
    Address  : 0x009001ac
    Name     : TX_COMMAND_1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TX_COMMAND_1                  R/W  default = 0
               -Same as TX_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001b0
    Name     : TX_COMMAND_2
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TX_COMMAND_2                  R/W  default = 0
               -Same as TX_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001b4
    Name     : TX_COMMAND_3
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TX_COMMAND_3                  R/W  default = 0
               -Same as TX_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001b8
    Name     : TX_COMMAND_4
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TX_COMMAND_4                  R/W  default = 0
               -Same as TX_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001bc
    Name     : TX_COMMAND_5
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TX_COMMAND_5                  R/W  default = 0
               -Same as TX_COMMAND_0
               -In addition, this queue is specially used to send NULL packet by HW
               -when Channel Switching Mode is set to H/W
               -So S/W should not use this queue if Channel Switching Mode is set to H/W


---------------------------------------------------------------------------------
    Address  : 0x009001c0
    Name     : TX_COMMAND_6
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TX_COMMAND_6                  R/W  default = 0
               -Same as TX_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001c4
    Name     : TX_COMMAND_7
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TX_COMMAND_7                  R/W  default = 0
               -Same as TX_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001c8
    Name     : TX_COMMAND_8
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TX_COMMAND_8                  R/W  default = 0
               -Same as TX_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001cc
    Name     : TX_COMMAND_9
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TX_COMMAND_9                  R/W  default = 0
               -Same as TX_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001d0
    Name     : TX_COMMAND_10
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TX_COMMAND_10                 R/W  default = 0
               -Same as TX_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001d4
    Name     : TXOP_COMMAND_0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31:16]    BUFFER_WORD_OFFSET            R/W  default = 0
               -Word offset from TX_BASE_ADDRESS
               -BUFFER_WORD_OFFSET = (BUFFER Address - TX_BASE_ADDRESS) >> 2
               -For example, when TX_BASE_ADDRESS == 0x12340000,
               -BUFFER_WORD_OFFSET of BUFFERADDRESS 0x12340008 is 2


---------------------------------------------------------------------------------
    Address  : 0x009001d8
    Name     : TXOP_COMMAND_1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TXOP_COMMAND_1                R/W  default = 0
               -Same as TXOP_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001dc
    Name     : TXOP_COMMAND_2
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TXOP_COMMAND_2                R/W  default = 0
               -Same as TXOP_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001e0
    Name     : TXOP_COMMAND_3
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TXOP_COMMAND_3                R/W  default = 0
               -Same as TXOP_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001e4
    Name     : TXOP_COMMAND_4
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TXOP_COMMAND_4                R/W  default = 0
               -Same as TXOP_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001e8
    Name     : TXOP_COMMAND_5
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TXOP_COMMAND_5                R/W  default = 0
               -Same as TXOP_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001ec
    Name     : TXOP_COMMAND_6
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TXOP_COMMAND_6                R/W  default = 0
               -Same as TXOP_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001f0
    Name     : TXOP_COMMAND_7
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TXOP_COMMAND_7                R/W  default = 0
               -Same as TXOP_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001f4
    Name     : TXOP_COMMAND_8
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TXOP_COMMAND_8                R/W  default = 0
               -Same as TXOP_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001f8
    Name     : TXOP_COMMAND_9
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TXOP_COMMAND_9                R/W  default = 0
               -Same as TXOP_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x009001fc
    Name     : TXOP_COMMAND_10
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TXOP_COMMAND_10               R/W  default = 0
               -Same as TXOP_COMMAND_0


---------------------------------------------------------------------------------
    Address  : 0x00900200
    Name     : TX_BASE_ADDRESS
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    TX_BASE_ADDRESS               R/W  default = 0
               -Base Address of TX Buffer


---------------------------------------------------------------------------------
    Address  : 0x00900204
    Name     : TX_DATA_IN_WAIT
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [   31]    SECURITY_OFFLINE              R/W  default = 0
               -Disable inline security mode in TX Side

   [23:16]    WAPI_DATA_IN_WAIT             R/W  default = 31
               -WAIT Counter for WAPI block input

   [15: 8]    TIKIP_DATA_IN_WAIT            R/W  default = 38
               -WAIT Counter for TKIP/WEP block input

   [ 7: 0]    CCMP_DATA_IN_WAIT             R/W  default = 11
               -WAIT Counter for CCMP block input


---------------------------------------------------------------------------------
    Address  : 0x00900208
    Name     : PHY_TXFIFO_MAX_DEPTH
    Bit      : 6
    R/W      : R/W
---------------------------------------------------------------------------------
   [ 5: 0]    PHY_TXFIFO_MAX_DEPTH          R/W  default = 24
               -This value is used at flow control of tx data not to exceed PHY TX FIFO limit


---------------------------------------------------------------------------------
    Address  : 0x0090020c
    Name     : RESPONSE_TX_COMMAND
    Bit      : 6
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [    5]    RESPONSE_TYPE                 R/W  default = 0
                 0 : Use H/W Control Frame Template
                 1 : User defined response control frame

   [ 4: 0]    COMMAND                       R/W  default = 0
               -If RESPONSE_TYPE == 1 , Word Length of Data(MAC Header + Payload)
               -If RESPONSE_TYPE == 0 ,
                 0 : NONE
                 1 : RTS
                 2 : CTS
                 3 : ACK
                 4 : BA
                 5 : CF END
                 6 : CTS TO Self


---------------------------------------------------------------------------------
    Address  : 0x00900210
    Name     : RESPONSE_TX_VECTOR0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_VECTOR0           R/W  default = 0
               -First TX_VECTOR of reponse frame


---------------------------------------------------------------------------------
    Address  : 0x00900214
    Name     : RESPONSE_TX_VECTOR1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_VECTOR1           R/W  default = 0
               -Second TX_VECTOR of reponse frame


---------------------------------------------------------------------------------
    Address  : 0x00900218
    Name     : RESPONSE_TX_VECTOR2
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_VECTOR2           R/W  default = 0
               -Third TX_VECTOR of reponse frame


---------------------------------------------------------------------------------
    Address  : 0x0090021c
    Name     : RESPONSE_TX_INFO0
    Bit      : 29
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   - First TX_INFO of reponse frame
   [28:23]    RSP_TXINFO_TIMESTAMP_POSITION R/W  default = 0

   [   22]    RSP_TXINFO_TIMESTAMP_UPDATE   R/W  default = 0

   [17:15]    RSP_TXINFO_CIPHER_TYPE        R/W  default = 0

   [   14]    RSP_TXINFO_SINGLE_AMPDU       R/W  default = 0

   [13: 0]    RSP_TXINFO_MPDU_LENGTH        R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900220
    Name     : RESPONSE_TX_INFO1
    Bit      : 24
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   - Second TX_INFO of reponse frame
   [23:21]    RSP_TXINFO_FORMAT             R/W  default = 0

   [   20]    RSP_TXINFO_AGGREGATION        R/W  default = 0

   [19: 0]    RSP_TXINFO_PSDU_LENGTH        R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900224
    Name     : RESPONSE_TX_DATA0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_DATA0             R/W  default = 0
               -TX_DATA0 of response frame


---------------------------------------------------------------------------------
    Address  : 0x00900228
    Name     : RESPONSE_TX_DATA1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_DATA1             R/W  default = 0
               -TX_DATA1 of response frame


---------------------------------------------------------------------------------
    Address  : 0x0090022c
    Name     : RESPONSE_TX_DATA2
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_DATA2             R/W  default = 0
               -TX_DATA2 of response frame


---------------------------------------------------------------------------------
    Address  : 0x00900230
    Name     : RESPONSE_TX_DATA3
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_DATA3             R/W  default = 0
               -TX_DATA3 of response frame


---------------------------------------------------------------------------------
    Address  : 0x00900234
    Name     : RESPONSE_TX_DATA4
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_DATA4             R/W  default = 0
               -TX_DATA4 of response frame


---------------------------------------------------------------------------------
    Address  : 0x00900238
    Name     : RESPONSE_TX_DATA5
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_DATA5             R/W  default = 0
               -TX_DATA5 of response frame


---------------------------------------------------------------------------------
    Address  : 0x0090023c
    Name     : RESPONSE_TX_DATA6
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_DATA6             R/W  default = 0
               -TX_DATA6 of response frame


---------------------------------------------------------------------------------
    Address  : 0x00900240
    Name     : RESPONSE_TX_DATA7
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_DATA7             R/W  default = 0
               -TX_DATA7 of response frame


---------------------------------------------------------------------------------
    Address  : 0x00900244
    Name     : RESPONSE_TX_DATA8
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_DATA8             R/W  default = 0
               -TX_DATA8 of response frame


---------------------------------------------------------------------------------
    Address  : 0x00900248
    Name     : RESPONSE_TX_DATA9
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_DATA9             R/W  default = 0
               -TX_DATA9 of response frame


---------------------------------------------------------------------------------
    Address  : 0x0090024c
    Name     : RESPONSE_TX_DATA10
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RESPONSE_TX_DATA10            R/W  default = 0
               -TX_DATA10 of response frame


---------------------------------------------------------------------------------
    Address  : 0x00900250
    Name     : RX_BASE_ADDRESS
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RX_BASE_ADDRESS               R/W  default = 0
               -Base Address for RX Buffer


---------------------------------------------------------------------------------
    Address  : 0x00900254
    Name     : RX_DSC_NUM
    Bit      : 5
    R/W      : R/W
---------------------------------------------------------------------------------
   [ 4: 0]    RX_DSC_NUM                    R/W  default = 0
               -Number of RX Descriptor


---------------------------------------------------------------------------------
    Address  : 0x00900258
    Name     : RX_REG_DL_DESC0
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26:16]    BUFFER_LENGTH                 R/W  default = 1024
               -11bit Byte Length of RX BUFFER

   [15: 0]    DATA_ADDRESS_OFFSET           R/W  default = 0
               -Address of buffer in 4Byte Unit, based on RX_BASE_ADDR


---------------------------------------------------------------------------------
    Address  : 0x0090025c
    Name     : RX_REG_DL_DESC1
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC1               R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900260
    Name     : RX_REG_DL_DESC2
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC2               R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900264
    Name     : RX_REG_DL_DESC3
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC3               R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900268
    Name     : RX_REG_DL_DESC4
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC4               R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x0090026c
    Name     : RX_REG_DL_DESC5
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC5               R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900270
    Name     : RX_REG_DL_DESC6
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC6               R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900274
    Name     : RX_REG_DL_DESC7
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC7               R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900278
    Name     : RX_REG_DL_DESC8
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC8               R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x0090027c
    Name     : RX_REG_DL_DESC9
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC9               R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900280
    Name     : RX_REG_DL_DESC10
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC10              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900284
    Name     : RX_REG_DL_DESC11
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC11              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900288
    Name     : RX_REG_DL_DESC12
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC12              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x0090028c
    Name     : RX_REG_DL_DESC13
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC13              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900290
    Name     : RX_REG_DL_DESC14
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC14              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900294
    Name     : RX_REG_DL_DESC15
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC15              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900298
    Name     : RX_REG_DL_DESC16
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC16              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x0090029c
    Name     : RX_REG_DL_DESC17
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC17              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002a0
    Name     : RX_REG_DL_DESC18
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC18              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002a4
    Name     : RX_REG_DL_DESC19
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC19              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002a8
    Name     : RX_REG_DL_DESC20
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC20              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002ac
    Name     : RX_REG_DL_DESC21
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC21              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002b0
    Name     : RX_REG_DL_DESC22
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC22              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002b4
    Name     : RX_REG_DL_DESC23
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC23              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002b8
    Name     : RX_REG_DL_DESC24
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC24              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002bc
    Name     : RX_REG_DL_DESC25
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC25              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002c0
    Name     : RX_REG_DL_DESC26
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC26              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002c4
    Name     : RX_REG_DL_DESC27
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC27              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002c8
    Name     : RX_REG_DL_DESC28
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC28              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002cc
    Name     : RX_REG_DL_DESC29
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC29              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002d0
    Name     : RX_REG_DL_DESC30
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC30              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002d4
    Name     : RX_REG_DL_DESC31
    Bit      : 27
    R/W      : R/W
---------------------------------------------------------------------------------
   [26: 0]    RX_REG_DL_DESC31              R/W  default = 33554432
               -Same as RX_REG_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009002d8
    Name     : RX_SET_OWNER_ADDR
    Bit      : 5
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [ 4: 0]    RX_SET_OWNER_ADDR             R/W  default = 0
               -Address of ownership bit set to 1


---------------------------------------------------------------------------------
    Address  : 0x009002dc
    Name     : RX_DATA_IN_WAIT
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [   31]    SECURITY_OFFLINE              R/W  default = 0
               -Disable inline security mode in TX Side

   [23:16]    WAPI_DATA_IN_WAIT             R/W  default = 16
               -WAIT Counter for WAPI block input

   [15: 8]    TIKIP_DATA_IN_WAIT            R/W  default = 19
               -WAIT Counter for TKIP/WEP block input

   [ 7: 0]    CCMP_DATA_IN_WAIT             R/W  default = 6
               -WAIT Counter for CCMP block input


---------------------------------------------------------------------------------
    Address  : 0x009002e0
    Name     : RX_DMA_SEG_THRESHOLD
    Bit      : 8
    R/W      : R/W
---------------------------------------------------------------------------------
   [ 7: 0]    RX_DMA_SEG_THRESHOLD          R/W  default = 8
               -Threshold for Rx DMA Command


---------------------------------------------------------------------------------
    Address  : 0x009002e4
    Name     : CRC_ERR_CRITERIA
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    CRC_ERR_CRITERIA              R/W  default = 100
               -CRC error criteria


---------------------------------------------------------------------------------
    Address  : 0x009002e8
    Name     : SEC_KEY_CMD
    Bit      : 10
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [ 9: 6]    SEC_KEY_CMD_TYPE              R/W  default = 0
               -Command type
                 0 : ADD KEY
                 1 : DELETE KEY
                 2 : DELETE ALL KEY

   [ 5: 3]    SEC_KEY_CMD_CIPHER_TYPE       R/W  default = 0
               -Cipher type

   [ 2: 1]    SEC_KEY_CMD_KEY_ID            R/W  default = 0

   [    0]    SEC_KEY_CMD_KEY_TYPE          R/W  default = 0
               -key type
                 0 : PTK
                 1 : GTK


---------------------------------------------------------------------------------
    Address  : 0x009002ec
    Name     : SEC_KEY_CMD_ENABLE
    Bit      : 1
    R/W      : R/W
---------------------------------------------------------------------------------
   [    0]    SEC_KEY_CMD_ENABLE            R/W  default = 0
               -When SW writes key command, this value should be set to '1'
               -And, after operation of key command is done by HW, this value is chaged to '0'


---------------------------------------------------------------------------------
    Address  : 0x009002f0
    Name     : SEC_KEY_VALUE0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_KEY_VALUE0                R/W  default = 0
               -LSB 32bit of 128bit KEY


---------------------------------------------------------------------------------
    Address  : 0x009002f4
    Name     : SEC_KEY_VALUE1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_KEY_VALUE1                R/W  default = 0
               -Next LSB 32bit of 128bit KEY


---------------------------------------------------------------------------------
    Address  : 0x009002f8
    Name     : SEC_KEY_VALUE2
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_KEY_VALUE2                R/W  default = 0
               -Next MSB 32bit of 128bit KEY


---------------------------------------------------------------------------------
    Address  : 0x009002fc
    Name     : SEC_KEY_VALUE3
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_KEY_VALUE3                R/W  default = 0
               -MSB 32bit of 128bit KEY


---------------------------------------------------------------------------------
    Address  : 0x00900300
    Name     : SEC_TX_MIC_KEY_VALUE0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_TX_MIC_KEY_VALUE0         R/W  default = 0
               -LSB 32bit of Tx MIC KEY


---------------------------------------------------------------------------------
    Address  : 0x00900304
    Name     : SEC_TX_MIC_KEY_VALUE1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_TX_MIC_KEY_VALUE1         R/W  default = 0
               -MSB 32bit of Tx MIC KEY


---------------------------------------------------------------------------------
    Address  : 0x00900308
    Name     : SEC_RX_MIC_KEY_VALUE0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_RX_MIC_KEY_VALUE0         R/W  default = 0
               -LSB 32bit of Rx MIC KEY


---------------------------------------------------------------------------------
    Address  : 0x0090030c
    Name     : SEC_RX_MIC_KEY_VALUE1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_RX_MIC_KEY_VALUE1         R/W  default = 0
               -MSB 32bit of Rx MIC KEY


---------------------------------------------------------------------------------
    Address  : 0x00900310
    Name     : SEC_STA_ADDRESS_0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_STA_ADDRESS_0             R/W  default = 0
               -LSB 32bit of peer STA's MAC address


---------------------------------------------------------------------------------
    Address  : 0x00900314
    Name     : SEC_STA_ADDRESS_1
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   [15: 0]    SEC_STA_ADDRESS_1             R/W  default = 0
               -MSB 16bit of peer STA's MAC address


---------------------------------------------------------------------------------
    Address  : 0x00900318
    Name     : SEC_SPP_ENABLE
    Bit      : 1
    R/W      : R/W
---------------------------------------------------------------------------------
   [    0]    SEC_SPP_ENABLE                R/W  default = 0
               -SPP A-MSDU enable


---------------------------------------------------------------------------------
    Address  : 0x0090031c
    Name     : SEC_KEY_LOC_ENABLE
    Bit      : 1
    R/W      : R/W
---------------------------------------------------------------------------------
   [    0]    SEC_KEY_LOC_ENABLE            R/W  default = 0
               -SEC Key Location enable


---------------------------------------------------------------------------------
    Address  : 0x00900320
    Name     : SEC_READ_BASE_ADDRESS
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_READ_BASE_ADDRESS         R/W  default = 0
               -Base address of read operation in offline mode


---------------------------------------------------------------------------------
    Address  : 0x00900324
    Name     : SEC_WRITE_BASE_ADDRESS
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_WRITE_BASE_ADDRESS        R/W  default = 0
               -Base address of write operation in offline mode


---------------------------------------------------------------------------------
    Address  : 0x00900328
    Name     : SEC_INFO_0
    Bit      : 18
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [17: 4]    MPDU_LENGTH                   R/W  default = 0
               -MPDU length for offline mode

   [ 3: 1]    CIPHER_TYPE                   R/W  default = 0
               -Cipher type for offline mode

   [    0]    ENCRYPTION                    R/W  default = 0
               -Encryption or Decryption for offline mode


---------------------------------------------------------------------------------
    Address  : 0x0090032c
    Name     : SEC_INFO_1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_INFO_1                    R/W  default = 0
               -Key value 0 - LSB 32bit of 128bit KEY for offline mode


---------------------------------------------------------------------------------
    Address  : 0x00900330
    Name     : SEC_INFO_2
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_INFO_2                    R/W  default = 0
               -Key value 1 - Next LSB 32bit of 128bit KEY for offline mode


---------------------------------------------------------------------------------
    Address  : 0x00900334
    Name     : SEC_INFO_3
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_INFO_3                    R/W  default = 0
               -Key value 2 - Next MSB 32bit of 128bit KEY for offline mode


---------------------------------------------------------------------------------
    Address  : 0x00900338
    Name     : SEC_INFO_4
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_INFO_4                    R/W  default = 0
               -Key value 3 - MSB 32bit of 128bit KEY for offline mode


---------------------------------------------------------------------------------
    Address  : 0x0090033c
    Name     : SEC_INFO_5
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_INFO_5                    R/W  default = 0
               -Tx MIC key value 0 - LSB 32bit of Tx MIC KEY for offline mode


---------------------------------------------------------------------------------
    Address  : 0x00900340
    Name     : SEC_INFO_6
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_INFO_6                    R/W  default = 0
               -Tx MIC key value 1 - MSB 32bit of Tx MIC KEY for offline mode


---------------------------------------------------------------------------------
    Address  : 0x00900344
    Name     : SEC_INFO_7
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_INFO_7                    R/W  default = 0
               -Rx MIC key value 0 - LSB 32bit of Rx MIC KEY for offline mode


---------------------------------------------------------------------------------
    Address  : 0x00900348
    Name     : SEC_INFO_8
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_INFO_8                    R/W  default = 0
               -Rx MIC key value 1 - MSB 32bit of Rx MIC KEY for offline mode


---------------------------------------------------------------------------------
    Address  : 0x0090034c
    Name     : SEC_OFFSET
    Bit      : 15
    R/W      : R/W
---------------------------------------------------------------------------------
   + separate 
---------------------------------------------------------------------------------
   [14:10]    READ_OFFSET_DECRYPT           R/W  default = 0
               -Offset of read word in decryption

   [ 9: 5]    READ_OFFSET_ENCRYPT_1         R/W  default = 0
               -Offset of read word in encryption 1

   [ 4: 0]    READ_OFFSET_ENCRYPT_0         R/W  default = 0
               -Offset of read word in encryption 0


---------------------------------------------------------------------------------
    Address  : 0x00900350
    Name     : TKIP_MIC_CAL_OFF
    Bit      : 1
    R/W      : R/W
---------------------------------------------------------------------------------
   [    0]    TKIP_MIC_CAL_OFF              R/W  default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900354
    Name     : TX_INDIR_REG_ADDR
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    TX_INDIR_REG_ADDR             R/W  default = 0
               -Tx indirect address - Index of Tx indirect registers for debugging


---------------------------------------------------------------------------------
    Address  : 0x00900358
    Name     : RX_INDIR_REG_ADDR
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    RX_INDIR_REG_ADDR             R/W  default = 0
               -Rx indirect address - Index of Rx indirect registers for debugging


---------------------------------------------------------------------------------
    Address  : 0x0090035c
    Name     : SEC_INDIR_REG_ADDR
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    SEC_INDIR_REG_ADDR            R/W  default = 0
               -SEC indirect address - Index of security indirect registers for debugging


---------------------------------------------------------------------------------
    Address  : 0x00900360
    Name     : DMA_INDIR_REG_ADDR
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    DMA_INDIR_REG_ADDR            R/W  default = 0
               -DMA indirect address - Index of DMA indirect registers for debugging


---------------------------------------------------------------------------------
    Address  : 0x00900364
    Name     : IRQ_INDIR_REG_ADDR
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    IRQ_INDIR_REG_ADDR            R/W  default = 0
               -IRQ indirect address - Index of IRQ indirect registers for debugging


---------------------------------------------------------------------------------
    Address  : 0x00900368
    Name     : TSF_0_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TSF_0_LOWER                   R/W  default = 0
               -Setting lower 32bit of TSF 0
               -Internal TSF0 which has 64bit width will only update it value if TSF_UPPER and TSF_LOWER are sequentially written both


---------------------------------------------------------------------------------
    Address  : 0x0090036c
    Name     : TSF_0_UPPER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TSF_0_UPPER                   R/W  default = 0
               -Setting upper 32bit of TSF 0
               -Internal TSF0 which has 64bit width will only update it value if TSF_UPPER and TSF_LOWER are sequentially written both


---------------------------------------------------------------------------------
    Address  : 0x00900370
    Name     : TSF_0_ALARM_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TSF_0_ALARM_LOWER             R/W  default = 0
               -When value of TSF0 reaches the value designated in TSF_0_ALARM_LOWER register, the system will raise TSF Alarm interrupt


---------------------------------------------------------------------------------
    Address  : 0x00900374
    Name     : BCN_INTERVAL_0
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [15: 0]    BCN_INTERVAL_0                R/W  default = 0
               -Time interval between consecutive target beacon transmission times (TBTTs) of TSF0. The unit is TU equal to 1024 microseconds


---------------------------------------------------------------------------------
    Address  : 0x00900378
    Name     : TBTT_INTERRUPT_MARGIN_0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TBTT_INTERRUPT_MARGIN_0       R/W  default = 0
               -Time margin between TBTT interrupt and actual TBTT of TSF0
               -TBTT interrupt occurs in advance before the actual TBTT timing
               -The value is represented in the unit of microsecond


---------------------------------------------------------------------------------
    Address  : 0x0090037c
    Name     : TBTT_RESTART_RANGE_0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TBTT_RESTART_RANGE_0          R/W  default = 0
               -Time threshold to recalculate next TBTT of TSF0
               -When MAC timer is updated to new value with larger difference than the threshold, next TBTT may be recalculated


---------------------------------------------------------------------------------
    Address  : 0x00900380
    Name     : TSF_1_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TSF_1_LOWER                   R/W  default = 0
               -Setting lower 32bit of TSF 1
               -Internal TSF1 which has 64bit width will only update it value if TSF_UPPER and TSF_LOWER are sequentially written both


---------------------------------------------------------------------------------
    Address  : 0x00900384
    Name     : TSF_1_UPPER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TSF_1_UPPER                   R/W  default = 0
               -Setting upper 32bit of TSF 1
               -Internal TSF1 which has 64bit width will only update it value if TSF_UPPER and TSF_LOWER are sequentially written both


---------------------------------------------------------------------------------
    Address  : 0x00900388
    Name     : TSF_1_ALARM_LOWER
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TSF_1_ALARM_LOWER             R/W  default = 0
               -When value of TSF1 reaches the value designated in TSF_1_ALARM_LOWER register, the system will raise TSF Alarm interrupt


---------------------------------------------------------------------------------
    Address  : 0x0090038c
    Name     : BCN_INTERVAL_1
    Bit      : 16
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [15: 0]    BCN_INTERVAL_1                R/W  default = 0
               -Time interval between consecutive target beacon transmission times (TBTTs) of TSF1. The unit is TU equal to 1024 microseconds


---------------------------------------------------------------------------------
    Address  : 0x00900390
    Name     : TBTT_INTERRUPT_MARGIN_1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TBTT_INTERRUPT_MARGIN_1       R/W  default = 0
               -Time margin between TBTT interrupt and actual TBTT of TSF1
               -TBTT interrupt occurs in advance before the actual TBTT timing
               -The value is represented in the unit of microsecond


---------------------------------------------------------------------------------
    Address  : 0x00900394
    Name     : TBTT_RESTART_RANGE_1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   + write_enable 
---------------------------------------------------------------------------------
   [31: 0]    TBTT_RESTART_RANGE_1          R/W  default = 0
               -Time threshold to recalculate next TBTT of TSF1
               -When MAC timer is updated to new value with larger difference than the threshold, next TBTT may be recalculated


---------------------------------------------------------------------------------
    Address  : 0x00900398
    Name     : IRQ_MASK0
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    IRQ_MASK0                     R/W  default = 4294967295
               -Bit value 0 of IRQ_MASK will disable interrupt for corresponding IRQ source


---------------------------------------------------------------------------------
    Address  : 0x0090039c
    Name     : IRQ_MASK1
    Bit      : 32
    R/W      : R/W
---------------------------------------------------------------------------------
   [31: 0]    IRQ_MASK1                     R/W  default = 4294967295
               -Bit value 0 of IRQ_MASK will disable interrupt for corresponding IRQ source


---------------------------------------------------------------------------------
    Address  : 0x009003a0
    Name     : TX_RESULT_0
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31:28]    WINAC                         R    default = 0

   [21:16]    INTERNAL_COLLISION_BITMAP     R    default = 0
               -Bit 16 : AC_BK
               -Bit 17 : AC_BE
               -Bit 18 : AC_VI
               -Bit 19 : AC_VO
               -Bit 20 : AC_GP0
               -Bit 21 : AC_GP1

   [    6]    ACK_POLICY                    R    default = 0
               -0: Normal ACK Transmit
               -1: NoACK Transmit

   [ 5: 3]    FAIL_REASON                   R    default = 0
               -Value of FAIL_REASON is only valid when ACK_SUCCESS was 0
                 0 : ACK Timeout(Doesn't receive ACK)
                 1 : CTS Timeout(Doesn't receive CTS)
                 2 : Received ACK but CRC fail or Address Match Fail
                 3 : Received CTS but CRC fail or Address Match Fail
                 4 : Received ACK after sending A-MPDU
                 5 : Received Block ACK after sending non A-MPDU
                 6 : Received non ACK while waiting for ACK
                 7 : Received non CTS while waiting for CTS

   [    2]    TXOP_END                      R    default = 0
                 0 : Next MPDU is scheduled to send within TXOP
                 1 : Current TX was the end of TXOP

   [    1]    FAILED_BITMAP_VALID           R    default = 0
                 0 : All transmitted A-MPDU has been acknowledged
                 1 : One or more MPDU in A-MPDU has been failed. Should see corresponding TX_FAILED_BITMAP register

   [    0]    ACK_SUCCESS                   R    default = 0
                 0 : Transmission sequence has been failed. Reason for fail can be seen in bit [5:3]
                 1 : Transmission sequence success. No-ACK and No-RTS transmit will always set to 1. In A-MPDU transmit, it will set to 1 if BA has been received regardless of contents of BA


---------------------------------------------------------------------------------
    Address  : 0x009003a4
    Name     : TX_RESULT_1
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_RESULT_1                   R    default = 0
               -Same as TX_RESULT_0


---------------------------------------------------------------------------------
    Address  : 0x009003a8
    Name     : TX_RESULT_2
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_RESULT_2                   R    default = 0
               -Same as TX_RESULT_0


---------------------------------------------------------------------------------
    Address  : 0x009003ac
    Name     : TX_RESULT_3
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_RESULT_3                   R    default = 0
               -Same as TX_RESULT_0


---------------------------------------------------------------------------------
    Address  : 0x009003b0
    Name     : TX_RESULT_4
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_RESULT_4                   R    default = 0
               -Same as TX_RESULT_0


---------------------------------------------------------------------------------
    Address  : 0x009003b4
    Name     : TX_RESULT_5
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_RESULT_5                   R    default = 0
               -Same as TX_RESULT_0


---------------------------------------------------------------------------------
    Address  : 0x009003b8
    Name     : TX_RESULT_6
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_RESULT_6                   R    default = 0
               -Same as TX_RESULT_0


---------------------------------------------------------------------------------
    Address  : 0x009003bc
    Name     : TX_RESULT_7
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_RESULT_7                   R    default = 0
               -Same as TX_RESULT_0


---------------------------------------------------------------------------------
    Address  : 0x009003c0
    Name     : TX_RESULT_8
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_RESULT_8                   R    default = 0
               -Same as TX_RESULT_0


---------------------------------------------------------------------------------
    Address  : 0x009003c4
    Name     : TX_RESULT_9
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_RESULT_9                   R    default = 0
               -Same as TX_RESULT_0


---------------------------------------------------------------------------------
    Address  : 0x009003c8
    Name     : TX_RESULT_10
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_RESULT_10                  R    default = 0
               -Same as TX_RESULT_0


---------------------------------------------------------------------------------
    Address  : 0x009003cc
    Name     : TX_FAILED_BITMAP_LOWER_0
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_LOWER_0      R    default = 0
               -Lower 32 bit of 64 bit failed bitmap. Bit 0 correspond to TX SSN


---------------------------------------------------------------------------------
    Address  : 0x009003d0
    Name     : TX_FAILED_BITMAP_UPPER_0
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_UPPER_0      R    default = 0
               -Upper 32 bit of 64 bit failed bitmap


---------------------------------------------------------------------------------
    Address  : 0x009003d4
    Name     : TX_FAILED_BITMAP_LOWER_1
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_LOWER_1      R    default = 0
               -Lower 32 bit of 64 bit failed bitmap. Bit 0 correspond to TX SSN


---------------------------------------------------------------------------------
    Address  : 0x009003d8
    Name     : TX_FAILED_BITMAP_UPPER_1
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_UPPER_1      R    default = 0
               -Upper 32 bit of 64 bit failed bitmap


---------------------------------------------------------------------------------
    Address  : 0x009003dc
    Name     : TX_FAILED_BITMAP_LOWER_2
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_LOWER_2      R    default = 0
               -Lower 32 bit of 64 bit failed bitmap. Bit 0 correspond to TX SSN


---------------------------------------------------------------------------------
    Address  : 0x009003e0
    Name     : TX_FAILED_BITMAP_UPPER_2
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_UPPER_2      R    default = 0
               -Upper 32 bit of 64 bit failed bitmap


---------------------------------------------------------------------------------
    Address  : 0x009003e4
    Name     : TX_FAILED_BITMAP_LOWER_3
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_LOWER_3      R    default = 0
               -Lower 32 bit of 64 bit failed bitmap. Bit 0 correspond to TX SSN


---------------------------------------------------------------------------------
    Address  : 0x009003e8
    Name     : TX_FAILED_BITMAP_UPPER_3
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_UPPER_3      R    default = 0
               -Upper 32 bit of 64 bit failed bitmap


---------------------------------------------------------------------------------
    Address  : 0x009003ec
    Name     : TX_FAILED_BITMAP_LOWER_4
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_LOWER_4      R    default = 0
               -Lower 32 bit of 64 bit failed bitmap. Bit 0 correspond to TX SSN


---------------------------------------------------------------------------------
    Address  : 0x009003f0
    Name     : TX_FAILED_BITMAP_UPPER_4
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_UPPER_4      R    default = 0
               -Upper 32 bit of 64 bit failed bitmap


---------------------------------------------------------------------------------
    Address  : 0x009003f4
    Name     : TX_FAILED_BITMAP_LOWER_5
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_LOWER_5      R    default = 0
               -Lower 32 bit of 64 bit failed bitmap. Bit 0 correspond to TX SSN


---------------------------------------------------------------------------------
    Address  : 0x009003f8
    Name     : TX_FAILED_BITMAP_UPPER_5
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_UPPER_5      R    default = 0
               -Upper 32 bit of 64 bit failed bitmap


---------------------------------------------------------------------------------
    Address  : 0x009003fc
    Name     : TX_FAILED_BITMAP_LOWER_6
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_LOWER_6      R    default = 0
               -Lower 32 bit of 64 bit failed bitmap. Bit 0 correspond to TX SSN


---------------------------------------------------------------------------------
    Address  : 0x00900400
    Name     : TX_FAILED_BITMAP_UPPER_6
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_UPPER_6      R    default = 0
               -Upper 32 bit of 64 bit failed bitmap


---------------------------------------------------------------------------------
    Address  : 0x00900404
    Name     : TX_FAILED_BITMAP_LOWER_7
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_LOWER_7      R    default = 0
               -Lower 32 bit of 64 bit failed bitmap. Bit 0 correspond to TX SSN


---------------------------------------------------------------------------------
    Address  : 0x00900408
    Name     : TX_FAILED_BITMAP_UPPER_7
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_UPPER_7      R    default = 0
               -Upper 32 bit of 64 bit failed bitmap


---------------------------------------------------------------------------------
    Address  : 0x0090040c
    Name     : TX_FAILED_BITMAP_LOWER_8
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_LOWER_8      R    default = 0
               -Lower 32 bit of 64 bit failed bitmap. Bit 0 correspond to TX SSN


---------------------------------------------------------------------------------
    Address  : 0x00900410
    Name     : TX_FAILED_BITMAP_UPPER_8
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_UPPER_8      R    default = 0
               -Upper 32 bit of 64 bit failed bitmap


---------------------------------------------------------------------------------
    Address  : 0x00900414
    Name     : TX_FAILED_BITMAP_LOWER_9
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_LOWER_9      R    default = 0
               -Lower 32 bit of 64 bit failed bitmap. Bit 0 correspond to TX SSN


---------------------------------------------------------------------------------
    Address  : 0x00900418
    Name     : TX_FAILED_BITMAP_UPPER_9
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_FAILED_BITMAP_UPPER_9      R    default = 0
               -Upper 32 bit of 64 bit failed bitmap


---------------------------------------------------------------------------------
    Address  : 0x0090041c
    Name     : RESPONSE_TX_RESULT
    Bit      : 16
    R/W      : R
---------------------------------------------------------------------------------
   [15: 0]    RESPONSE_TX_RESULT            R    default = 0


---------------------------------------------------------------------------------
    Address  : 0x00900420
    Name     : CONCURRENT_INFO
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [11: 8]    LAST_COMMAND                  R    default = 0

   [ 7: 4]    STATE                         R    default = 0
                 0 : Single interface mode
                 1 : Both interface active mode
                 2 : Transit to both interface active mode
                 3 : BSS1 in periodic switching mode
                 4 : BSS2 in periodic switching mode
                 5 : Transit to BSS1
                 6 : Transit to BSS2

   [ 3: 0]    RESULT                        R    default = 0
                 0 : Success
                 1 : Already in single mode
                 2 : No running timer
                 3 : Timer is already running
                 4 : Timer is already stopped
                 5 : Already in both active mode


---------------------------------------------------------------------------------
    Address  : 0x00900424
    Name     : CONCURRENT_TIMER
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    CONCURRENT_TIMER              R    default = 0
               -Ready only timer - Current value of concurrent timer which is counting switching time. Microsecond resultion


---------------------------------------------------------------------------------
    Address  : 0x00900428
    Name     : RX_WRITE_EN_DL_DESC0
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [    2]    OWNERSHIP                     R    default = 0
               -Indicate that this register is valid. S/W should write this field to 1 after reading valid Data Address Offset, and H/W write this field to 0 after DMA write to the address indicated in Data Address Offset field has been done

   [ 1: 0]    FRAGMENT                      R    default = 0
               -Fragment Position of MPDU
                 0 : Single MPDU
                 1 : Last Fragment
                 2 : First Fragment
                 3 : Middle Fragment


---------------------------------------------------------------------------------
    Address  : 0x0090042c
    Name     : RX_WRITE_EN_DL_DESC1
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC1          R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900430
    Name     : RX_WRITE_EN_DL_DESC2
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC2          R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900434
    Name     : RX_WRITE_EN_DL_DESC3
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC3          R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900438
    Name     : RX_WRITE_EN_DL_DESC4
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC4          R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x0090043c
    Name     : RX_WRITE_EN_DL_DESC5
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC5          R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900440
    Name     : RX_WRITE_EN_DL_DESC6
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC6          R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900444
    Name     : RX_WRITE_EN_DL_DESC7
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC7          R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900448
    Name     : RX_WRITE_EN_DL_DESC8
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC8          R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x0090044c
    Name     : RX_WRITE_EN_DL_DESC9
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC9          R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900450
    Name     : RX_WRITE_EN_DL_DESC10
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC10         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900454
    Name     : RX_WRITE_EN_DL_DESC11
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC11         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900458
    Name     : RX_WRITE_EN_DL_DESC12
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC12         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x0090045c
    Name     : RX_WRITE_EN_DL_DESC13
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC13         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900460
    Name     : RX_WRITE_EN_DL_DESC14
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC14         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900464
    Name     : RX_WRITE_EN_DL_DESC15
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC15         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900468
    Name     : RX_WRITE_EN_DL_DESC16
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC16         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x0090046c
    Name     : RX_WRITE_EN_DL_DESC17
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC17         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900470
    Name     : RX_WRITE_EN_DL_DESC18
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC18         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900474
    Name     : RX_WRITE_EN_DL_DESC19
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC19         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900478
    Name     : RX_WRITE_EN_DL_DESC20
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC20         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x0090047c
    Name     : RX_WRITE_EN_DL_DESC21
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC21         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900480
    Name     : RX_WRITE_EN_DL_DESC22
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC22         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900484
    Name     : RX_WRITE_EN_DL_DESC23
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC23         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900488
    Name     : RX_WRITE_EN_DL_DESC24
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC24         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x0090048c
    Name     : RX_WRITE_EN_DL_DESC25
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC25         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900490
    Name     : RX_WRITE_EN_DL_DESC26
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC26         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900494
    Name     : RX_WRITE_EN_DL_DESC27
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC27         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x00900498
    Name     : RX_WRITE_EN_DL_DESC28
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC28         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x0090049c
    Name     : RX_WRITE_EN_DL_DESC29
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC29         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009004a0
    Name     : RX_WRITE_EN_DL_DESC30
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC30         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009004a4
    Name     : RX_WRITE_EN_DL_DESC31
    Bit      : 3
    R/W      : R
---------------------------------------------------------------------------------
   [ 2: 0]    RX_WRITE_EN_DL_DESC31         R    default = 0
               -Same as RX_WRITE_EN_DL_DESC0


---------------------------------------------------------------------------------
    Address  : 0x009004a8
    Name     : SEC_KEY_CMD_RESULT_EN
    Bit      : 1
    R/W      : R
---------------------------------------------------------------------------------
   [    0]    SEC_KEY_CMD_RESULT_EN         R    default = 0
               -Command result enable


---------------------------------------------------------------------------------
    Address  : 0x009004ac
    Name     : SEC_KEY_CMD_RESULT
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31:28]    COMMAND                       R    default = 0
               -Command of operation    
                 0 : Add key
                 1 : Delete key
                 2 : Delete all key

   [27:20]    RESULT                        R    default = 0
               -Result of operation
                 1 : Unsupported command type
                 2 : Add KEY fail because entries are full
                 3 : Add KEY success
                 4 : Delete KEY success
                 5 : Delete KEY fail because address matching is fail

   [ 2: 0]    ADDRESS                       R    default = 0
               -Address of memory


---------------------------------------------------------------------------------
    Address  : 0x009004b0
    Name     : SEC_WRITE_DESC_RESULT
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [30:23]    FRAGMENT                      R    default = 0
               -Fragment bits of write descriptors

   [    1]    ICV_FAIL                      R    default = 0
               -Result of ICV check

   [    0]    MIC_FAIL                      R    default = 0
               -Result of MIC check


---------------------------------------------------------------------------------
    Address  : 0x009004b4
    Name     : TX_INDIR_REG_DATA
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TX_INDIR_REG_DATA             R    default = 0
               -Indirect register data in TX Category


---------------------------------------------------------------------------------
    Address  : 0x009004b8
    Name     : RX_INDIR_REG_DATA
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    RX_INDIR_REG_DATA             R    default = 0
               -Indirect register data in RX Category


---------------------------------------------------------------------------------
    Address  : 0x009004bc
    Name     : SEC_INDIR_REG_DATA
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    SEC_INDIR_REG_DATA            R    default = 0
               -Indirect register data in Security Category


---------------------------------------------------------------------------------
    Address  : 0x009004c0
    Name     : DMA_INDIR_REG_DATA
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    DMA_INDIR_REG_DATA            R    default = 0
               -Indirect register data in DMA Category


---------------------------------------------------------------------------------
    Address  : 0x009004c4
    Name     : IRQ_INDIR_REG_DATA
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    IRQ_INDIR_REG_DATA            R    default = 0
               -Indirect register data in Interrupt Category


---------------------------------------------------------------------------------
    Address  : 0x009004c8
    Name     : TSF_0_LOWER_READONLY
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TSF_0_LOWER_READONLY          R    default = 0
               -Lower 32bit of 64bit TSF0 


---------------------------------------------------------------------------------
    Address  : 0x009004cc
    Name     : TSF_0_UPPER_READONLY
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TSF_0_UPPER_READONLY          R    default = 0
               -Upper 32bit of 64bit TSF0 


---------------------------------------------------------------------------------
    Address  : 0x009004d0
    Name     : TSF_1_LOWER_READONLY
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TSF_1_LOWER_READONLY          R    default = 0
               -Lower 32bit of 64bit TSF1 


---------------------------------------------------------------------------------
    Address  : 0x009004d4
    Name     : TSF_1_UPPER_READONLY
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    TSF_1_UPPER_READONLY          R    default = 0
               -Upper 32bit of 64bit TSF1 


---------------------------------------------------------------------------------
    Address  : 0x009004d8
    Name     : IRQ_SRC0
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   + read_enable 
---------------------------------------------------------------------------------
   [   31]    CONCURRENT_SWITCH             R    default = 0

   [   27]    RX_TIM                        R    default = 0

   [   26]    RX_DONE                       R    default = 0

   [23:13]    TX_DONE_BITMAP                R    default = 0

   [10: 0]    WIN_AC_BITMAP                 R    default = 0


---------------------------------------------------------------------------------
    Address  : 0x009004dc
    Name     : IRQ_SRC1
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   + read_enable 
---------------------------------------------------------------------------------
   [    7]    TSF1_ALARM                    R    default = 0

   [    6]    TBTT_IRQ_TSF1                 R    default = 0

   [    5]    TSF0_ALARM                    R    default = 0

   [    4]    TBTT_IRQ_TSF0                 R    default = 0

   [    3]    RX_BUFFER_LOOKUP              R    default = 0

   [    2]    SW_RSP_REQ_IRQ                R    default = 0

   [    1]    AHB_SEC_IRQ                   R    default = 0

   [    0]    DMA_ERROR                     R    default = 0


---------------------------------------------------------------------------------
    Address  : 0x009004e0
    Name     : IRQ_SRC0_NOCLEAR
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    IRQ_SRC0_NOCLEAR              R    default = 0
               -Same as IRQ_SRC0


---------------------------------------------------------------------------------
    Address  : 0x009004e4
    Name     : IRQ_SRC1_NOCLEAR
    Bit      : 32
    R/W      : R
---------------------------------------------------------------------------------
   [31: 0]    IRQ_SRC1_NOCLEAR              R    default = 0
               -Same as IRQ_SRC1


---------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
enum {
    MAC_REG_BASE_START                                                          = 0x00900000,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RST_SW                                                              = 0x00900000,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_CONFIG                                                              = 0x00900004,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_CONFIG_CH_SWITCH_MODE                                         = MAC_REG_CONFIG,
          MAC_REG_CONFIG_CH_SWITCH_MODE_SHIFT                                   = 25,
          MAC_REG_CONFIG_CH_SWITCH_MODE_MASK                                    = 0x02000000,
          MAC_REG_CONFIG_BSS_SWITCH_MODE                                        = MAC_REG_CONFIG,
          MAC_REG_CONFIG_BSS_SWITCH_MODE_SHIFT                                  = 24,
          MAC_REG_CONFIG_BSS_SWITCH_MODE_MASK                                   = 0x01000000,
          MAC_REG_CONFIG_MAC_ADDRESS_INTF_ID                                    = MAC_REG_CONFIG,
          MAC_REG_CONFIG_MAC_ADDRESS_INTF_ID_SHIFT                              = 20,
          MAC_REG_CONFIG_MAC_ADDRESS_INTF_ID_MASK                               = 0x00f00000,
          MAC_REG_CONFIG_OWN_MAC_ADDRESS_SET_EN                                 = MAC_REG_CONFIG,
          MAC_REG_CONFIG_OWN_MAC_ADDRESS_SET_EN_SHIFT                           = 16,
          MAC_REG_CONFIG_OWN_MAC_ADDRESS_SET_EN_MASK                            = 0x000f0000,
          MAC_REG_CONFIG_TIM_MATCH_MODE                                         = MAC_REG_CONFIG,
          MAC_REG_CONFIG_TIM_MATCH_MODE_SHIFT                                   = 7,
          MAC_REG_CONFIG_TIM_MATCH_MODE_MASK                                    = 0x00000180,
          MAC_REG_CONFIG_RX_BUFFER_LOOKUP_IRQ_EN                                = MAC_REG_CONFIG,
          MAC_REG_CONFIG_RX_BUFFER_LOOKUP_IRQ_EN_SHIFT                          = 6,
          MAC_REG_CONFIG_RX_BUFFER_LOOKUP_IRQ_EN_MASK                           = 0x00000040,
          MAC_REG_CONFIG_RX_BUFFER_LOOKUP_MODE_TYPE                             = MAC_REG_CONFIG,
          MAC_REG_CONFIG_RX_BUFFER_LOOKUP_MODE_TYPE_SHIFT                       = 5,
          MAC_REG_CONFIG_RX_BUFFER_LOOKUP_MODE_TYPE_MASK                        = 0x00000020,
          MAC_REG_CONFIG_RX_BUFFER_LOOKUP_EN                                    = MAC_REG_CONFIG,
          MAC_REG_CONFIG_RX_BUFFER_LOOKUP_EN_SHIFT                              = 4,
          MAC_REG_CONFIG_RX_BUFFER_LOOKUP_EN_MASK                               = 0x00000010,
          MAC_REG_CONFIG_RX_DMA_PROMISCUOUS_MODE                                = MAC_REG_CONFIG,
          MAC_REG_CONFIG_RX_DMA_PROMISCUOUS_MODE_SHIFT                          = 3,
          MAC_REG_CONFIG_RX_DMA_PROMISCUOUS_MODE_MASK                           = 0x00000008,
          MAC_REG_CONFIG_RANDOM_RX_CRASH                                        = MAC_REG_CONFIG,
          MAC_REG_CONFIG_RANDOM_RX_CRASH_SHIFT                                  = 2,
          MAC_REG_CONFIG_RANDOM_RX_CRASH_MASK                                   = 0x00000004,
          MAC_REG_CONFIG_CCA_IGNORE                                             = MAC_REG_CONFIG,
          MAC_REG_CONFIG_CCA_IGNORE_SHIFT                                       = 1,
          MAC_REG_CONFIG_CCA_IGNORE_MASK                                        = 0x00000002,
          MAC_REG_CONFIG_MAC_LOOPBACK                                           = MAC_REG_CONFIG,
          MAC_REG_CONFIG_MAC_LOOPBACK_SHIFT                                     = 0,
          MAC_REG_CONFIG_MAC_LOOPBACK_MASK                                      = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_CLOCK_GATING_CONFIG                                                 = 0x00900008,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_CLOCK_GATING_CONFIG_CCMP_CG_MODE                              = MAC_REG_CLOCK_GATING_CONFIG,
          MAC_REG_CLOCK_GATING_CONFIG_CCMP_CG_MODE_SHIFT                        = 4,
          MAC_REG_CLOCK_GATING_CONFIG_CCMP_CG_MODE_MASK                         = 0x00000030,
          MAC_REG_CLOCK_GATING_CONFIG_TKIP_CG_MODE                              = MAC_REG_CLOCK_GATING_CONFIG,
          MAC_REG_CLOCK_GATING_CONFIG_TKIP_CG_MODE_SHIFT                        = 2,
          MAC_REG_CLOCK_GATING_CONFIG_TKIP_CG_MODE_MASK                         = 0x0000000c,
          MAC_REG_CLOCK_GATING_CONFIG_LUT_CG_MODE                               = MAC_REG_CLOCK_GATING_CONFIG,
          MAC_REG_CLOCK_GATING_CONFIG_LUT_CG_MODE_SHIFT                         = 0,
          MAC_REG_CLOCK_GATING_CONFIG_LUT_CG_MODE_MASK                          = 0x00000003,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_US_CLOCK                                                            = 0x0090000c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SIFS_DURATION_CLOCK                                                 = 0x00900010,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SLOT_DURATION_CLOCK                                                 = 0x00900014,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_SLOT_DURATION_CLOCK_SLOT_DURATION_CLOCK_INTF1                 = MAC_REG_SLOT_DURATION_CLOCK,
          MAC_REG_SLOT_DURATION_CLOCK_SLOT_DURATION_CLOCK_INTF1_SHIFT           = 16,
          MAC_REG_SLOT_DURATION_CLOCK_SLOT_DURATION_CLOCK_INTF1_MASK            = 0xffff0000,
          MAC_REG_SLOT_DURATION_CLOCK_SLOT_DURATION_CLOCK_INTF0                 = MAC_REG_SLOT_DURATION_CLOCK,
          MAC_REG_SLOT_DURATION_CLOCK_SLOT_DURATION_CLOCK_INTF0_SHIFT           = 0,
          MAC_REG_SLOT_DURATION_CLOCK_SLOT_DURATION_CLOCK_INTF0_MASK            = 0x0000ffff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXPPDU_DELAY_CLOCK                                                  = 0x00900018,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_READY_BEFORE_TXPPDU_CLOCK                                           = 0x0090001c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_READY_TO_TXPOWER_CLOCK                                              = 0x00900020,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_READY_TO_TXPOWER_CLOCK_READY_TO_TXPOWER_CLOCK_20M             = MAC_REG_READY_TO_TXPOWER_CLOCK,
          MAC_REG_READY_TO_TXPOWER_CLOCK_READY_TO_TXPOWER_CLOCK_20M_SHIFT       = 10,
          MAC_REG_READY_TO_TXPOWER_CLOCK_READY_TO_TXPOWER_CLOCK_20M_MASK        = 0x000ffc00,
          MAC_REG_READY_TO_TXPOWER_CLOCK_READY_TO_TXPOWER_CLOCK_40M             = MAC_REG_READY_TO_TXPOWER_CLOCK,
          MAC_REG_READY_TO_TXPOWER_CLOCK_READY_TO_TXPOWER_CLOCK_40M_SHIFT       = 0,
          MAC_REG_READY_TO_TXPOWER_CLOCK_READY_TO_TXPOWER_CLOCK_40M_MASK        = 0x000003ff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXPOWER_TO_TXREQUEST_CLOCK                                          = 0x00900024,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TIMEOUT_US                                                 = 0x00900028,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TIMEOUT_11B_US                                             = 0x0090002c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_BSS_OPERATION_MODE                                                  = 0x00900030,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_BSS_OPERATION_MODE_STA_MODE                                   = MAC_REG_BSS_OPERATION_MODE,
          MAC_REG_BSS_OPERATION_MODE_STA_MODE_SHIFT                             = 0,
          MAC_REG_BSS_OPERATION_MODE_STA_MODE_MASK                              = 0x0000000f,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_FILTER_MODE                                                     = 0x00900034,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_STA_FILTER_MODE_RECEIVE_ALL                                   = MAC_REG_STA_FILTER_MODE,
          MAC_REG_STA_FILTER_MODE_RECEIVE_ALL_SHIFT                             = 7,
          MAC_REG_STA_FILTER_MODE_RECEIVE_ALL_MASK                              = 0x00000080,
          MAC_REG_STA_FILTER_MODE_RECEIVE_ALL_EXCEPT_CONTROL_FRAME              = MAC_REG_STA_FILTER_MODE,
          MAC_REG_STA_FILTER_MODE_RECEIVE_ALL_EXCEPT_CONTROL_FRAME_SHIFT        = 6,
          MAC_REG_STA_FILTER_MODE_RECEIVE_ALL_EXCEPT_CONTROL_FRAME_MASK         = 0x00000040,
          MAC_REG_STA_FILTER_MODE_RECEIVE_GROUP_ADDRESSED                       = MAC_REG_STA_FILTER_MODE,
          MAC_REG_STA_FILTER_MODE_RECEIVE_GROUP_ADDRESSED_SHIFT                 = 5,
          MAC_REG_STA_FILTER_MODE_RECEIVE_GROUP_ADDRESSED_MASK                  = 0x00000020,
          MAC_REG_STA_FILTER_MODE_RECEIVE_CONTROL_FRAME                         = MAC_REG_STA_FILTER_MODE,
          MAC_REG_STA_FILTER_MODE_RECEIVE_CONTROL_FRAME_SHIFT                   = 4,
          MAC_REG_STA_FILTER_MODE_RECEIVE_CONTROL_FRAME_MASK                    = 0x00000010,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MANAGEMENT_FRAME                      = MAC_REG_STA_FILTER_MODE,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MANAGEMENT_FRAME_SHIFT                = 3,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MANAGEMENT_FRAME_MASK                 = 0x00000008,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MY_CONTROL_FRAME                      = MAC_REG_STA_FILTER_MODE,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MY_CONTROL_FRAME_SHIFT                = 2,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MY_CONTROL_FRAME_MASK                 = 0x00000004,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MY_ACK_FRAME                          = MAC_REG_STA_FILTER_MODE,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MY_ACK_FRAME_SHIFT                    = 1,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MY_ACK_FRAME_MASK                     = 0x00000002,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MY_PM_BIT_FRAME                       = MAC_REG_STA_FILTER_MODE,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MY_PM_BIT_FRAME_SHIFT                 = 0,
          MAC_REG_STA_FILTER_MODE_RECEIVE_MY_PM_BIT_FRAME_MASK                  = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_BSS_ACTIVE                                                          = 0x00900038,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_BSS_ACTIVE_BSS_INTF_ID                                        = MAC_REG_BSS_ACTIVE,
          MAC_REG_BSS_ACTIVE_BSS_INTF_ID_SHIFT                                  = 4,
          MAC_REG_BSS_ACTIVE_BSS_INTF_ID_MASK                                   = 0x000000f0,
          MAC_REG_BSS_ACTIVE_BSS_ACTIVE_ENABLE                                  = MAC_REG_BSS_ACTIVE,
          MAC_REG_BSS_ACTIVE_BSS_ACTIVE_ENABLE_SHIFT                            = 0,
          MAC_REG_BSS_ACTIVE_BSS_ACTIVE_ENABLE_MASK                             = 0x0000000f,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_1MBPS                                             = 0x0090003c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_2MBPS                                             = 0x00900040,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_5_5MBPS                                           = 0x00900044,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_11MBPS                                            = 0x00900048,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_6MBPS                                             = 0x0090004c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_9MBPS                                             = 0x00900050,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_12MBPS                                            = 0x00900054,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_18MBPS                                            = 0x00900058,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_24MBPS                                            = 0x0090005c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_36MBPS                                            = 0x00900060,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_48MBPS                                            = 0x00900064,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_54MBPS                                            = 0x00900068,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_6MBPS_DUP                                         = 0x0090006c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_9MBPS_DUP                                         = 0x00900070,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_12MBPS_DUP                                        = 0x00900074,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_18MBPS_DUP                                        = 0x00900078,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_24MBPS_DUP                                        = 0x0090007c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_36MBPS_DUP                                        = 0x00900080,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_48MBPS_DUP                                        = 0x00900084,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_POWER_54MBPS_DUP                                        = 0x00900088,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_BSSID0_LOWER                                                    = 0x0090008c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_BSSID0_UPPER                                                    = 0x00900090,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_BSSID1_LOWER                                                    = 0x00900094,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_BSSID1_UPPER                                                    = 0x00900098,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_BSSID2_LOWER                                                    = 0x0090009c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_BSSID2_UPPER                                                    = 0x009000a0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_BSSID3_LOWER                                                    = 0x009000a4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_BSSID3_UPPER                                                    = 0x009000a8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_MAC_ADDRESS0_LOWER                                              = 0x009000ac,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_MAC_ADDRESS0_UPPER                                              = 0x009000b0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_MAC_ADDRESS1_LOWER                                              = 0x009000b4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_MAC_ADDRESS1_UPPER                                              = 0x009000b8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_MAC_ADDRESS2_LOWER                                              = 0x009000bc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_MAC_ADDRESS2_UPPER                                              = 0x009000c0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_MAC_ADDRESS3_LOWER                                              = 0x009000c4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_MAC_ADDRESS3_UPPER                                              = 0x009000c8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID0_0                                                          = 0x009000cc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID0_1                                                          = 0x009000d0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID0_2                                                          = 0x009000d4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID1_0                                                          = 0x009000d8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID1_1                                                          = 0x009000dc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID1_2                                                          = 0x009000e0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID2_0                                                          = 0x009000e4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID2_1                                                          = 0x009000e8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID2_2                                                          = 0x009000ec,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID3_0                                                          = 0x009000f0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID3_1                                                          = 0x009000f4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_AID3_2                                                          = 0x009000f8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_DMA_BURST                                                           = 0x009000fc,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_DMA_BURST_DMA_BURST_MAX                                       = MAC_REG_DMA_BURST,
          MAC_REG_DMA_BURST_DMA_BURST_MAX_SHIFT                                 = 6,
          MAC_REG_DMA_BURST_DMA_BURST_MAX_MASK                                  = 0x00000fc0,
          MAC_REG_DMA_BURST_DMA_BURST_MIN                                       = MAC_REG_DMA_BURST,
          MAC_REG_DMA_BURST_DMA_BURST_MIN_SHIFT                                 = 0,
          MAC_REG_DMA_BURST_DMA_BURST_MIN_MASK                                  = 0x0000003f,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_DMA_RDFIFO_ADJUST                                                   = 0x00900100,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_DETECT_11B_PREAMBLE                                             = 0x00900104,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_STA_DETECT_11B_PREAMBLE_PREAMBLE_TYPE                         = MAC_REG_STA_DETECT_11B_PREAMBLE,
          MAC_REG_STA_DETECT_11B_PREAMBLE_PREAMBLE_TYPE_SHIFT                   = 1,
          MAC_REG_STA_DETECT_11B_PREAMBLE_PREAMBLE_TYPE_MASK                    = 0x00000002,
          MAC_REG_STA_DETECT_11B_PREAMBLE_DETECT_11B                            = MAC_REG_STA_DETECT_11B_PREAMBLE,
          MAC_REG_STA_DETECT_11B_PREAMBLE_DETECT_11B_SHIFT                      = 0,
          MAC_REG_STA_DETECT_11B_PREAMBLE_DETECT_11B_MASK                       = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_STA_DETECT_11B_PREAMBLE_2ND                                         = 0x00900108,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_STA_DETECT_11B_PREAMBLE_2ND_PREAMBLE_TYPE_2ND                 = MAC_REG_STA_DETECT_11B_PREAMBLE_2ND,
          MAC_REG_STA_DETECT_11B_PREAMBLE_2ND_PREAMBLE_TYPE_2ND_SHIFT           = 1,
          MAC_REG_STA_DETECT_11B_PREAMBLE_2ND_PREAMBLE_TYPE_2ND_MASK            = 0x00000002,
          MAC_REG_STA_DETECT_11B_PREAMBLE_2ND_DETECT_11B_2ND                    = MAC_REG_STA_DETECT_11B_PREAMBLE_2ND,
          MAC_REG_STA_DETECT_11B_PREAMBLE_2ND_DETECT_11B_2ND_SHIFT              = 0,
          MAC_REG_STA_DETECT_11B_PREAMBLE_2ND_DETECT_11B_2ND_MASK               = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_BASIC_RATE_BITMAP                                                   = 0x0090010c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_BASIC_RATE_BITMAP_2ND                                               = 0x00900110,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_SUPPRESS_COMMAND                                                 = 0x00900114,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_SUPPRESS_START_TSF                                               = 0x00900118,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_SUPPRESS_END_TSF                                                 = 0x0090011c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_SUPPRESS_DURATION                                                = 0x00900120,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_SUPPRESS_SETTING                                                 = 0x00900124,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_CONTROL_PARAMETER                                                = 0x00900128,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_TX_CONTROL_PARAMETER_RX_RESPONSE_CONTROL                      = MAC_REG_TX_CONTROL_PARAMETER,
          MAC_REG_TX_CONTROL_PARAMETER_RX_RESPONSE_CONTROL_SHIFT                = 18,
          MAC_REG_TX_CONTROL_PARAMETER_RX_RESPONSE_CONTROL_MASK                 = 0x000c0000,
          MAC_REG_TX_CONTROL_PARAMETER_TX_ERROR_RECOVERY                        = MAC_REG_TX_CONTROL_PARAMETER,
          MAC_REG_TX_CONTROL_PARAMETER_TX_ERROR_RECOVERY_SHIFT                  = 17,
          MAC_REG_TX_CONTROL_PARAMETER_TX_ERROR_RECOVERY_MASK                   = 0x00020000,
          MAC_REG_TX_CONTROL_PARAMETER_CCA_LATE_DETECTION                       = MAC_REG_TX_CONTROL_PARAMETER,
          MAC_REG_TX_CONTROL_PARAMETER_CCA_LATE_DETECTION_SHIFT                 = 16,
          MAC_REG_TX_CONTROL_PARAMETER_CCA_LATE_DETECTION_MASK                  = 0x00010000,
          MAC_REG_TX_CONTROL_PARAMETER_OTHER_FRAG_DATA_WORD_OFFSET              = MAC_REG_TX_CONTROL_PARAMETER,
          MAC_REG_TX_CONTROL_PARAMETER_OTHER_FRAG_DATA_WORD_OFFSET_SHIFT        = 12,
          MAC_REG_TX_CONTROL_PARAMETER_OTHER_FRAG_DATA_WORD_OFFSET_MASK         = 0x0000f000,
          MAC_REG_TX_CONTROL_PARAMETER_FIRST_FRAG_DATA_WORD_OFFSET              = MAC_REG_TX_CONTROL_PARAMETER,
          MAC_REG_TX_CONTROL_PARAMETER_FIRST_FRAG_DATA_WORD_OFFSET_SHIFT        = 8,
          MAC_REG_TX_CONTROL_PARAMETER_FIRST_FRAG_DATA_WORD_OFFSET_MASK         = 0x00000f00,
          MAC_REG_TX_CONTROL_PARAMETER_RESPONSE_WAIT_TIME_US                    = MAC_REG_TX_CONTROL_PARAMETER,
          MAC_REG_TX_CONTROL_PARAMETER_RESPONSE_WAIT_TIME_US_SHIFT              = 0,
          MAC_REG_TX_CONTROL_PARAMETER_RESPONSE_WAIT_TIME_US_MASK               = 0x000000ff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP0                                                                = 0x0090012c,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP0_RSP0_VALID                                               = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_VALID_SHIFT                                         = 31,
          MAC_REG_RSP0_RSP0_VALID_MASK                                          = 0x80000000,
          MAC_REG_RSP0_RSP0_CASE                                                = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_CASE_SHIFT                                          = 26,
          MAC_REG_RSP0_RSP0_CASE_MASK                                           = 0x7c000000,
          MAC_REG_RSP0_RSP0_MASK_RX_INFO_OK                                     = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_MASK_RX_INFO_OK_SHIFT                               = 25,
          MAC_REG_RSP0_RSP0_MASK_RX_INFO_OK_MASK                                = 0x02000000,
          MAC_REG_RSP0_RSP0_MASK_RX_INFO_NDP                                    = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_MASK_RX_INFO_NDP_SHIFT                              = 24,
          MAC_REG_RSP0_RSP0_MASK_RX_INFO_NDP_MASK                               = 0x01000000,
          MAC_REG_RSP0_RSP0_MASK_RX_NDP_PSPOLL                                  = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_MASK_RX_NDP_PSPOLL_SHIFT                            = 23,
          MAC_REG_RSP0_RSP0_MASK_RX_NDP_PSPOLL_MASK                             = 0x00800000,
          MAC_REG_RSP0_RSP0_MASK_RX_PV0_TYPE                                    = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_MASK_RX_PV0_TYPE_SHIFT                              = 21,
          MAC_REG_RSP0_RSP0_MASK_RX_PV0_TYPE_MASK                               = 0x00600000,
          MAC_REG_RSP0_RSP0_MASK_RX_PV0_SUBTYPE                                 = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_MASK_RX_PV0_SUBTYPE_SHIFT                           = 17,
          MAC_REG_RSP0_RSP0_MASK_RX_PV0_SUBTYPE_MASK                            = 0x001e0000,
          MAC_REG_RSP0_RSP0_MASK_RX_PV                                          = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_MASK_RX_PV_SHIFT                                    = 16,
          MAC_REG_RSP0_RSP0_MASK_RX_PV_MASK                                     = 0x00010000,
          MAC_REG_RSP0_RSP0_MASK_GROUP_ADDRESS                                  = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_MASK_GROUP_ADDRESS_SHIFT                            = 15,
          MAC_REG_RSP0_RSP0_MASK_GROUP_ADDRESS_MASK                             = 0x00008000,
          MAC_REG_RSP0_RSP0_MASK_OPERATION                                      = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_MASK_OPERATION_SHIFT                                = 14,
          MAC_REG_RSP0_RSP0_MASK_OPERATION_MASK                                 = 0x00004000,
          MAC_REG_RSP0_RSP0_MASK_RX_INFO_AGG                                    = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_MASK_RX_INFO_AGG_SHIFT                              = 13,
          MAC_REG_RSP0_RSP0_MASK_RX_INFO_AGG_MASK                               = 0x00002000,
          MAC_REG_RSP0_RSP0_RX_INFO_OK                                          = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_RX_INFO_OK_SHIFT                                    = 12,
          MAC_REG_RSP0_RSP0_RX_INFO_OK_MASK                                     = 0x00001000,
          MAC_REG_RSP0_RSP0_RX_INFO_NDP                                         = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_RX_INFO_NDP_SHIFT                                   = 11,
          MAC_REG_RSP0_RSP0_RX_INFO_NDP_MASK                                    = 0x00000800,
          MAC_REG_RSP0_RSP0_RX_NDP_PSPOLL                                       = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_RX_NDP_PSPOLL_SHIFT                                 = 10,
          MAC_REG_RSP0_RSP0_RX_NDP_PSPOLL_MASK                                  = 0x00000400,
          MAC_REG_RSP0_RSP0_RX_PV0_TYPE                                         = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_RX_PV0_TYPE_SHIFT                                   = 8,
          MAC_REG_RSP0_RSP0_RX_PV0_TYPE_MASK                                    = 0x00000300,
          MAC_REG_RSP0_RSP0_RX_PV0_SUBTYPE                                      = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_RX_PV0_SUBTYPE_SHIFT                                = 4,
          MAC_REG_RSP0_RSP0_RX_PV0_SUBTYPE_MASK                                 = 0x000000f0,
          MAC_REG_RSP0_RSP0_RX_PV                                               = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_RX_PV_SHIFT                                         = 3,
          MAC_REG_RSP0_RSP0_RX_PV_MASK                                          = 0x00000008,
          MAC_REG_RSP0_RSP0_GROUP_ADDRESS                                       = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_GROUP_ADDRESS_SHIFT                                 = 2,
          MAC_REG_RSP0_RSP0_GROUP_ADDRESS_MASK                                  = 0x00000004,
          MAC_REG_RSP0_RSP0_OPERATION                                           = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_OPERATION_SHIFT                                     = 1,
          MAC_REG_RSP0_RSP0_OPERATION_MASK                                      = 0x00000002,
          MAC_REG_RSP0_RSP0_RX_INFO_AGG                                         = MAC_REG_RSP0,
          MAC_REG_RSP0_RSP0_RX_INFO_AGG_SHIFT                                   = 0,
          MAC_REG_RSP0_RSP0_RX_INFO_AGG_MASK                                    = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP1                                                                = 0x00900130,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP1_RSP1_VALID                                               = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_VALID_SHIFT                                         = 31,
          MAC_REG_RSP1_RSP1_VALID_MASK                                          = 0x80000000,
          MAC_REG_RSP1_RSP1_CASE                                                = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_CASE_SHIFT                                          = 26,
          MAC_REG_RSP1_RSP1_CASE_MASK                                           = 0x7c000000,
          MAC_REG_RSP1_RSP1_MASK_RX_INFO_OK                                     = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_MASK_RX_INFO_OK_SHIFT                               = 25,
          MAC_REG_RSP1_RSP1_MASK_RX_INFO_OK_MASK                                = 0x02000000,
          MAC_REG_RSP1_RSP1_MASK_RX_INFO_NDP                                    = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_MASK_RX_INFO_NDP_SHIFT                              = 24,
          MAC_REG_RSP1_RSP1_MASK_RX_INFO_NDP_MASK                               = 0x01000000,
          MAC_REG_RSP1_RSP1_MASK_RX_NDP_PSPOLL                                  = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_MASK_RX_NDP_PSPOLL_SHIFT                            = 23,
          MAC_REG_RSP1_RSP1_MASK_RX_NDP_PSPOLL_MASK                             = 0x00800000,
          MAC_REG_RSP1_RSP1_MASK_RX_PV0_TYPE                                    = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_MASK_RX_PV0_TYPE_SHIFT                              = 21,
          MAC_REG_RSP1_RSP1_MASK_RX_PV0_TYPE_MASK                               = 0x00600000,
          MAC_REG_RSP1_RSP1_MASK_RX_PV0_SUBTYPE                                 = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_MASK_RX_PV0_SUBTYPE_SHIFT                           = 17,
          MAC_REG_RSP1_RSP1_MASK_RX_PV0_SUBTYPE_MASK                            = 0x001e0000,
          MAC_REG_RSP1_RSP1_MASK_RX_PV                                          = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_MASK_RX_PV_SHIFT                                    = 16,
          MAC_REG_RSP1_RSP1_MASK_RX_PV_MASK                                     = 0x00010000,
          MAC_REG_RSP1_RSP1_MASK_GROUP_ADDRESS                                  = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_MASK_GROUP_ADDRESS_SHIFT                            = 15,
          MAC_REG_RSP1_RSP1_MASK_GROUP_ADDRESS_MASK                             = 0x00008000,
          MAC_REG_RSP1_RSP1_MASK_OPERATION                                      = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_MASK_OPERATION_SHIFT                                = 14,
          MAC_REG_RSP1_RSP1_MASK_OPERATION_MASK                                 = 0x00004000,
          MAC_REG_RSP1_RSP1_MASK_RX_INFO_AGG                                    = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_MASK_RX_INFO_AGG_SHIFT                              = 13,
          MAC_REG_RSP1_RSP1_MASK_RX_INFO_AGG_MASK                               = 0x00002000,
          MAC_REG_RSP1_RSP1_RX_INFO_OK                                          = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_RX_INFO_OK_SHIFT                                    = 12,
          MAC_REG_RSP1_RSP1_RX_INFO_OK_MASK                                     = 0x00001000,
          MAC_REG_RSP1_RSP1_RX_INFO_NDP                                         = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_RX_INFO_NDP_SHIFT                                   = 11,
          MAC_REG_RSP1_RSP1_RX_INFO_NDP_MASK                                    = 0x00000800,
          MAC_REG_RSP1_RSP1_RX_NDP_PSPOLL                                       = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_RX_NDP_PSPOLL_SHIFT                                 = 10,
          MAC_REG_RSP1_RSP1_RX_NDP_PSPOLL_MASK                                  = 0x00000400,
          MAC_REG_RSP1_RSP1_RX_PV0_TYPE                                         = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_RX_PV0_TYPE_SHIFT                                   = 8,
          MAC_REG_RSP1_RSP1_RX_PV0_TYPE_MASK                                    = 0x00000300,
          MAC_REG_RSP1_RSP1_RX_PV0_SUBTYPE                                      = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_RX_PV0_SUBTYPE_SHIFT                                = 4,
          MAC_REG_RSP1_RSP1_RX_PV0_SUBTYPE_MASK                                 = 0x000000f0,
          MAC_REG_RSP1_RSP1_RX_PV                                               = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_RX_PV_SHIFT                                         = 3,
          MAC_REG_RSP1_RSP1_RX_PV_MASK                                          = 0x00000008,
          MAC_REG_RSP1_RSP1_GROUP_ADDRESS                                       = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_GROUP_ADDRESS_SHIFT                                 = 2,
          MAC_REG_RSP1_RSP1_GROUP_ADDRESS_MASK                                  = 0x00000004,
          MAC_REG_RSP1_RSP1_OPERATION                                           = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_OPERATION_SHIFT                                     = 1,
          MAC_REG_RSP1_RSP1_OPERATION_MASK                                      = 0x00000002,
          MAC_REG_RSP1_RSP1_RX_INFO_AGG                                         = MAC_REG_RSP1,
          MAC_REG_RSP1_RSP1_RX_INFO_AGG_SHIFT                                   = 0,
          MAC_REG_RSP1_RSP1_RX_INFO_AGG_MASK                                    = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP2                                                                = 0x00900134,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP2_RSP2_VALID                                               = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_VALID_SHIFT                                         = 31,
          MAC_REG_RSP2_RSP2_VALID_MASK                                          = 0x80000000,
          MAC_REG_RSP2_RSP2_CASE                                                = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_CASE_SHIFT                                          = 26,
          MAC_REG_RSP2_RSP2_CASE_MASK                                           = 0x7c000000,
          MAC_REG_RSP2_RSP2_MASK_RX_INFO_OK                                     = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_MASK_RX_INFO_OK_SHIFT                               = 25,
          MAC_REG_RSP2_RSP2_MASK_RX_INFO_OK_MASK                                = 0x02000000,
          MAC_REG_RSP2_RSP2_MASK_RX_INFO_NDP                                    = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_MASK_RX_INFO_NDP_SHIFT                              = 24,
          MAC_REG_RSP2_RSP2_MASK_RX_INFO_NDP_MASK                               = 0x01000000,
          MAC_REG_RSP2_RSP2_MASK_RX_NDP_PSPOLL                                  = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_MASK_RX_NDP_PSPOLL_SHIFT                            = 23,
          MAC_REG_RSP2_RSP2_MASK_RX_NDP_PSPOLL_MASK                             = 0x00800000,
          MAC_REG_RSP2_RSP2_MASK_RX_PV0_TYPE                                    = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_MASK_RX_PV0_TYPE_SHIFT                              = 21,
          MAC_REG_RSP2_RSP2_MASK_RX_PV0_TYPE_MASK                               = 0x00600000,
          MAC_REG_RSP2_RSP2_MASK_RX_PV0_SUBTYPE                                 = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_MASK_RX_PV0_SUBTYPE_SHIFT                           = 17,
          MAC_REG_RSP2_RSP2_MASK_RX_PV0_SUBTYPE_MASK                            = 0x001e0000,
          MAC_REG_RSP2_RSP2_MASK_RX_PV                                          = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_MASK_RX_PV_SHIFT                                    = 16,
          MAC_REG_RSP2_RSP2_MASK_RX_PV_MASK                                     = 0x00010000,
          MAC_REG_RSP2_RSP2_MASK_GROUP_ADDRESS                                  = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_MASK_GROUP_ADDRESS_SHIFT                            = 15,
          MAC_REG_RSP2_RSP2_MASK_GROUP_ADDRESS_MASK                             = 0x00008000,
          MAC_REG_RSP2_RSP2_MASK_OPERATION                                      = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_MASK_OPERATION_SHIFT                                = 14,
          MAC_REG_RSP2_RSP2_MASK_OPERATION_MASK                                 = 0x00004000,
          MAC_REG_RSP2_RSP2_MASK_RX_INFO_AGG                                    = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_MASK_RX_INFO_AGG_SHIFT                              = 13,
          MAC_REG_RSP2_RSP2_MASK_RX_INFO_AGG_MASK                               = 0x00002000,
          MAC_REG_RSP2_RSP2_RX_INFO_OK                                          = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_RX_INFO_OK_SHIFT                                    = 12,
          MAC_REG_RSP2_RSP2_RX_INFO_OK_MASK                                     = 0x00001000,
          MAC_REG_RSP2_RSP2_RX_INFO_NDP                                         = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_RX_INFO_NDP_SHIFT                                   = 11,
          MAC_REG_RSP2_RSP2_RX_INFO_NDP_MASK                                    = 0x00000800,
          MAC_REG_RSP2_RSP2_RX_NDP_PSPOLL                                       = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_RX_NDP_PSPOLL_SHIFT                                 = 10,
          MAC_REG_RSP2_RSP2_RX_NDP_PSPOLL_MASK                                  = 0x00000400,
          MAC_REG_RSP2_RSP2_RX_PV0_TYPE                                         = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_RX_PV0_TYPE_SHIFT                                   = 8,
          MAC_REG_RSP2_RSP2_RX_PV0_TYPE_MASK                                    = 0x00000300,
          MAC_REG_RSP2_RSP2_RX_PV0_SUBTYPE                                      = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_RX_PV0_SUBTYPE_SHIFT                                = 4,
          MAC_REG_RSP2_RSP2_RX_PV0_SUBTYPE_MASK                                 = 0x000000f0,
          MAC_REG_RSP2_RSP2_RX_PV                                               = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_RX_PV_SHIFT                                         = 3,
          MAC_REG_RSP2_RSP2_RX_PV_MASK                                          = 0x00000008,
          MAC_REG_RSP2_RSP2_GROUP_ADDRESS                                       = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_GROUP_ADDRESS_SHIFT                                 = 2,
          MAC_REG_RSP2_RSP2_GROUP_ADDRESS_MASK                                  = 0x00000004,
          MAC_REG_RSP2_RSP2_OPERATION                                           = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_OPERATION_SHIFT                                     = 1,
          MAC_REG_RSP2_RSP2_OPERATION_MASK                                      = 0x00000002,
          MAC_REG_RSP2_RSP2_RX_INFO_AGG                                         = MAC_REG_RSP2,
          MAC_REG_RSP2_RSP2_RX_INFO_AGG_SHIFT                                   = 0,
          MAC_REG_RSP2_RSP2_RX_INFO_AGG_MASK                                    = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP3                                                                = 0x00900138,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP3_RSP3_VALID                                               = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_VALID_SHIFT                                         = 31,
          MAC_REG_RSP3_RSP3_VALID_MASK                                          = 0x80000000,
          MAC_REG_RSP3_RSP3_CASE                                                = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_CASE_SHIFT                                          = 26,
          MAC_REG_RSP3_RSP3_CASE_MASK                                           = 0x7c000000,
          MAC_REG_RSP3_RSP3_MASK_RX_INFO_OK                                     = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_MASK_RX_INFO_OK_SHIFT                               = 25,
          MAC_REG_RSP3_RSP3_MASK_RX_INFO_OK_MASK                                = 0x02000000,
          MAC_REG_RSP3_RSP3_MASK_RX_INFO_NDP                                    = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_MASK_RX_INFO_NDP_SHIFT                              = 24,
          MAC_REG_RSP3_RSP3_MASK_RX_INFO_NDP_MASK                               = 0x01000000,
          MAC_REG_RSP3_RSP3_MASK_RX_NDP_PSPOLL                                  = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_MASK_RX_NDP_PSPOLL_SHIFT                            = 23,
          MAC_REG_RSP3_RSP3_MASK_RX_NDP_PSPOLL_MASK                             = 0x00800000,
          MAC_REG_RSP3_RSP3_MASK_RX_PV0_TYPE                                    = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_MASK_RX_PV0_TYPE_SHIFT                              = 21,
          MAC_REG_RSP3_RSP3_MASK_RX_PV0_TYPE_MASK                               = 0x00600000,
          MAC_REG_RSP3_RSP3_MASK_RX_PV0_SUBTYPE                                 = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_MASK_RX_PV0_SUBTYPE_SHIFT                           = 17,
          MAC_REG_RSP3_RSP3_MASK_RX_PV0_SUBTYPE_MASK                            = 0x001e0000,
          MAC_REG_RSP3_RSP3_MASK_RX_PV                                          = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_MASK_RX_PV_SHIFT                                    = 16,
          MAC_REG_RSP3_RSP3_MASK_RX_PV_MASK                                     = 0x00010000,
          MAC_REG_RSP3_RSP3_MASK_GROUP_ADDRESS                                  = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_MASK_GROUP_ADDRESS_SHIFT                            = 15,
          MAC_REG_RSP3_RSP3_MASK_GROUP_ADDRESS_MASK                             = 0x00008000,
          MAC_REG_RSP3_RSP3_MASK_OPERATION                                      = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_MASK_OPERATION_SHIFT                                = 14,
          MAC_REG_RSP3_RSP3_MASK_OPERATION_MASK                                 = 0x00004000,
          MAC_REG_RSP3_RSP3_MASK_RX_INFO_AGG                                    = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_MASK_RX_INFO_AGG_SHIFT                              = 13,
          MAC_REG_RSP3_RSP3_MASK_RX_INFO_AGG_MASK                               = 0x00002000,
          MAC_REG_RSP3_RSP3_RX_INFO_OK                                          = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_RX_INFO_OK_SHIFT                                    = 12,
          MAC_REG_RSP3_RSP3_RX_INFO_OK_MASK                                     = 0x00001000,
          MAC_REG_RSP3_RSP3_RX_INFO_NDP                                         = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_RX_INFO_NDP_SHIFT                                   = 11,
          MAC_REG_RSP3_RSP3_RX_INFO_NDP_MASK                                    = 0x00000800,
          MAC_REG_RSP3_RSP3_RX_NDP_PSPOLL                                       = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_RX_NDP_PSPOLL_SHIFT                                 = 10,
          MAC_REG_RSP3_RSP3_RX_NDP_PSPOLL_MASK                                  = 0x00000400,
          MAC_REG_RSP3_RSP3_RX_PV0_TYPE                                         = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_RX_PV0_TYPE_SHIFT                                   = 8,
          MAC_REG_RSP3_RSP3_RX_PV0_TYPE_MASK                                    = 0x00000300,
          MAC_REG_RSP3_RSP3_RX_PV0_SUBTYPE                                      = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_RX_PV0_SUBTYPE_SHIFT                                = 4,
          MAC_REG_RSP3_RSP3_RX_PV0_SUBTYPE_MASK                                 = 0x000000f0,
          MAC_REG_RSP3_RSP3_RX_PV                                               = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_RX_PV_SHIFT                                         = 3,
          MAC_REG_RSP3_RSP3_RX_PV_MASK                                          = 0x00000008,
          MAC_REG_RSP3_RSP3_GROUP_ADDRESS                                       = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_GROUP_ADDRESS_SHIFT                                 = 2,
          MAC_REG_RSP3_RSP3_GROUP_ADDRESS_MASK                                  = 0x00000004,
          MAC_REG_RSP3_RSP3_OPERATION                                           = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_OPERATION_SHIFT                                     = 1,
          MAC_REG_RSP3_RSP3_OPERATION_MASK                                      = 0x00000002,
          MAC_REG_RSP3_RSP3_RX_INFO_AGG                                         = MAC_REG_RSP3,
          MAC_REG_RSP3_RSP3_RX_INFO_AGG_SHIFT                                   = 0,
          MAC_REG_RSP3_RSP3_RX_INFO_AGG_MASK                                    = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP4                                                                = 0x0090013c,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP4_RSP4_VALID                                               = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_VALID_SHIFT                                         = 31,
          MAC_REG_RSP4_RSP4_VALID_MASK                                          = 0x80000000,
          MAC_REG_RSP4_RSP4_CASE                                                = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_CASE_SHIFT                                          = 26,
          MAC_REG_RSP4_RSP4_CASE_MASK                                           = 0x7c000000,
          MAC_REG_RSP4_RSP4_MASK_RX_INFO_OK                                     = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_MASK_RX_INFO_OK_SHIFT                               = 25,
          MAC_REG_RSP4_RSP4_MASK_RX_INFO_OK_MASK                                = 0x02000000,
          MAC_REG_RSP4_RSP4_MASK_RX_INFO_NDP                                    = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_MASK_RX_INFO_NDP_SHIFT                              = 24,
          MAC_REG_RSP4_RSP4_MASK_RX_INFO_NDP_MASK                               = 0x01000000,
          MAC_REG_RSP4_RSP4_MASK_RX_NDP_PSPOLL                                  = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_MASK_RX_NDP_PSPOLL_SHIFT                            = 23,
          MAC_REG_RSP4_RSP4_MASK_RX_NDP_PSPOLL_MASK                             = 0x00800000,
          MAC_REG_RSP4_RSP4_MASK_RX_PV0_TYPE                                    = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_MASK_RX_PV0_TYPE_SHIFT                              = 21,
          MAC_REG_RSP4_RSP4_MASK_RX_PV0_TYPE_MASK                               = 0x00600000,
          MAC_REG_RSP4_RSP4_MASK_RX_PV0_SUBTYPE                                 = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_MASK_RX_PV0_SUBTYPE_SHIFT                           = 17,
          MAC_REG_RSP4_RSP4_MASK_RX_PV0_SUBTYPE_MASK                            = 0x001e0000,
          MAC_REG_RSP4_RSP4_MASK_RX_PV                                          = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_MASK_RX_PV_SHIFT                                    = 16,
          MAC_REG_RSP4_RSP4_MASK_RX_PV_MASK                                     = 0x00010000,
          MAC_REG_RSP4_RSP4_MASK_GROUP_ADDRESS                                  = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_MASK_GROUP_ADDRESS_SHIFT                            = 15,
          MAC_REG_RSP4_RSP4_MASK_GROUP_ADDRESS_MASK                             = 0x00008000,
          MAC_REG_RSP4_RSP4_MASK_OPERATION                                      = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_MASK_OPERATION_SHIFT                                = 14,
          MAC_REG_RSP4_RSP4_MASK_OPERATION_MASK                                 = 0x00004000,
          MAC_REG_RSP4_RSP4_MASK_RX_INFO_AGG                                    = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_MASK_RX_INFO_AGG_SHIFT                              = 13,
          MAC_REG_RSP4_RSP4_MASK_RX_INFO_AGG_MASK                               = 0x00002000,
          MAC_REG_RSP4_RSP4_RX_INFO_OK                                          = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_RX_INFO_OK_SHIFT                                    = 12,
          MAC_REG_RSP4_RSP4_RX_INFO_OK_MASK                                     = 0x00001000,
          MAC_REG_RSP4_RSP4_RX_INFO_NDP                                         = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_RX_INFO_NDP_SHIFT                                   = 11,
          MAC_REG_RSP4_RSP4_RX_INFO_NDP_MASK                                    = 0x00000800,
          MAC_REG_RSP4_RSP4_RX_NDP_PSPOLL                                       = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_RX_NDP_PSPOLL_SHIFT                                 = 10,
          MAC_REG_RSP4_RSP4_RX_NDP_PSPOLL_MASK                                  = 0x00000400,
          MAC_REG_RSP4_RSP4_RX_PV0_TYPE                                         = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_RX_PV0_TYPE_SHIFT                                   = 8,
          MAC_REG_RSP4_RSP4_RX_PV0_TYPE_MASK                                    = 0x00000300,
          MAC_REG_RSP4_RSP4_RX_PV0_SUBTYPE                                      = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_RX_PV0_SUBTYPE_SHIFT                                = 4,
          MAC_REG_RSP4_RSP4_RX_PV0_SUBTYPE_MASK                                 = 0x000000f0,
          MAC_REG_RSP4_RSP4_RX_PV                                               = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_RX_PV_SHIFT                                         = 3,
          MAC_REG_RSP4_RSP4_RX_PV_MASK                                          = 0x00000008,
          MAC_REG_RSP4_RSP4_GROUP_ADDRESS                                       = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_GROUP_ADDRESS_SHIFT                                 = 2,
          MAC_REG_RSP4_RSP4_GROUP_ADDRESS_MASK                                  = 0x00000004,
          MAC_REG_RSP4_RSP4_OPERATION                                           = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_OPERATION_SHIFT                                     = 1,
          MAC_REG_RSP4_RSP4_OPERATION_MASK                                      = 0x00000002,
          MAC_REG_RSP4_RSP4_RX_INFO_AGG                                         = MAC_REG_RSP4,
          MAC_REG_RSP4_RSP4_RX_INFO_AGG_SHIFT                                   = 0,
          MAC_REG_RSP4_RSP4_RX_INFO_AGG_MASK                                    = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP5                                                                = 0x00900140,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP5_RSP5_VALID                                               = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_VALID_SHIFT                                         = 31,
          MAC_REG_RSP5_RSP5_VALID_MASK                                          = 0x80000000,
          MAC_REG_RSP5_RSP5_CASE                                                = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_CASE_SHIFT                                          = 26,
          MAC_REG_RSP5_RSP5_CASE_MASK                                           = 0x7c000000,
          MAC_REG_RSP5_RSP5_MASK_RX_INFO_OK                                     = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_MASK_RX_INFO_OK_SHIFT                               = 25,
          MAC_REG_RSP5_RSP5_MASK_RX_INFO_OK_MASK                                = 0x02000000,
          MAC_REG_RSP5_RSP5_MASK_RX_INFO_NDP                                    = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_MASK_RX_INFO_NDP_SHIFT                              = 24,
          MAC_REG_RSP5_RSP5_MASK_RX_INFO_NDP_MASK                               = 0x01000000,
          MAC_REG_RSP5_RSP5_MASK_RX_NDP_PSPOLL                                  = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_MASK_RX_NDP_PSPOLL_SHIFT                            = 23,
          MAC_REG_RSP5_RSP5_MASK_RX_NDP_PSPOLL_MASK                             = 0x00800000,
          MAC_REG_RSP5_RSP5_MASK_RX_PV0_TYPE                                    = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_MASK_RX_PV0_TYPE_SHIFT                              = 21,
          MAC_REG_RSP5_RSP5_MASK_RX_PV0_TYPE_MASK                               = 0x00600000,
          MAC_REG_RSP5_RSP5_MASK_RX_PV0_SUBTYPE                                 = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_MASK_RX_PV0_SUBTYPE_SHIFT                           = 17,
          MAC_REG_RSP5_RSP5_MASK_RX_PV0_SUBTYPE_MASK                            = 0x001e0000,
          MAC_REG_RSP5_RSP5_MASK_RX_PV                                          = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_MASK_RX_PV_SHIFT                                    = 16,
          MAC_REG_RSP5_RSP5_MASK_RX_PV_MASK                                     = 0x00010000,
          MAC_REG_RSP5_RSP5_MASK_GROUP_ADDRESS                                  = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_MASK_GROUP_ADDRESS_SHIFT                            = 15,
          MAC_REG_RSP5_RSP5_MASK_GROUP_ADDRESS_MASK                             = 0x00008000,
          MAC_REG_RSP5_RSP5_MASK_OPERATION                                      = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_MASK_OPERATION_SHIFT                                = 14,
          MAC_REG_RSP5_RSP5_MASK_OPERATION_MASK                                 = 0x00004000,
          MAC_REG_RSP5_RSP5_MASK_RX_INFO_AGG                                    = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_MASK_RX_INFO_AGG_SHIFT                              = 13,
          MAC_REG_RSP5_RSP5_MASK_RX_INFO_AGG_MASK                               = 0x00002000,
          MAC_REG_RSP5_RSP5_RX_INFO_OK                                          = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_RX_INFO_OK_SHIFT                                    = 12,
          MAC_REG_RSP5_RSP5_RX_INFO_OK_MASK                                     = 0x00001000,
          MAC_REG_RSP5_RSP5_RX_INFO_NDP                                         = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_RX_INFO_NDP_SHIFT                                   = 11,
          MAC_REG_RSP5_RSP5_RX_INFO_NDP_MASK                                    = 0x00000800,
          MAC_REG_RSP5_RSP5_RX_NDP_PSPOLL                                       = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_RX_NDP_PSPOLL_SHIFT                                 = 10,
          MAC_REG_RSP5_RSP5_RX_NDP_PSPOLL_MASK                                  = 0x00000400,
          MAC_REG_RSP5_RSP5_RX_PV0_TYPE                                         = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_RX_PV0_TYPE_SHIFT                                   = 8,
          MAC_REG_RSP5_RSP5_RX_PV0_TYPE_MASK                                    = 0x00000300,
          MAC_REG_RSP5_RSP5_RX_PV0_SUBTYPE                                      = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_RX_PV0_SUBTYPE_SHIFT                                = 4,
          MAC_REG_RSP5_RSP5_RX_PV0_SUBTYPE_MASK                                 = 0x000000f0,
          MAC_REG_RSP5_RSP5_RX_PV                                               = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_RX_PV_SHIFT                                         = 3,
          MAC_REG_RSP5_RSP5_RX_PV_MASK                                          = 0x00000008,
          MAC_REG_RSP5_RSP5_GROUP_ADDRESS                                       = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_GROUP_ADDRESS_SHIFT                                 = 2,
          MAC_REG_RSP5_RSP5_GROUP_ADDRESS_MASK                                  = 0x00000004,
          MAC_REG_RSP5_RSP5_OPERATION                                           = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_OPERATION_SHIFT                                     = 1,
          MAC_REG_RSP5_RSP5_OPERATION_MASK                                      = 0x00000002,
          MAC_REG_RSP5_RSP5_RX_INFO_AGG                                         = MAC_REG_RSP5,
          MAC_REG_RSP5_RSP5_RX_INFO_AGG_SHIFT                                   = 0,
          MAC_REG_RSP5_RSP5_RX_INFO_AGG_MASK                                    = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP6                                                                = 0x00900144,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP6_RSP6_VALID                                               = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_VALID_SHIFT                                         = 31,
          MAC_REG_RSP6_RSP6_VALID_MASK                                          = 0x80000000,
          MAC_REG_RSP6_RSP6_CASE                                                = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_CASE_SHIFT                                          = 26,
          MAC_REG_RSP6_RSP6_CASE_MASK                                           = 0x7c000000,
          MAC_REG_RSP6_RSP6_MASK_RX_INFO_OK                                     = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_MASK_RX_INFO_OK_SHIFT                               = 25,
          MAC_REG_RSP6_RSP6_MASK_RX_INFO_OK_MASK                                = 0x02000000,
          MAC_REG_RSP6_RSP6_MASK_RX_INFO_NDP                                    = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_MASK_RX_INFO_NDP_SHIFT                              = 24,
          MAC_REG_RSP6_RSP6_MASK_RX_INFO_NDP_MASK                               = 0x01000000,
          MAC_REG_RSP6_RSP6_MASK_RX_NDP_PSPOLL                                  = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_MASK_RX_NDP_PSPOLL_SHIFT                            = 23,
          MAC_REG_RSP6_RSP6_MASK_RX_NDP_PSPOLL_MASK                             = 0x00800000,
          MAC_REG_RSP6_RSP6_MASK_RX_PV0_TYPE                                    = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_MASK_RX_PV0_TYPE_SHIFT                              = 21,
          MAC_REG_RSP6_RSP6_MASK_RX_PV0_TYPE_MASK                               = 0x00600000,
          MAC_REG_RSP6_RSP6_MASK_RX_PV0_SUBTYPE                                 = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_MASK_RX_PV0_SUBTYPE_SHIFT                           = 17,
          MAC_REG_RSP6_RSP6_MASK_RX_PV0_SUBTYPE_MASK                            = 0x001e0000,
          MAC_REG_RSP6_RSP6_MASK_RX_PV                                          = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_MASK_RX_PV_SHIFT                                    = 16,
          MAC_REG_RSP6_RSP6_MASK_RX_PV_MASK                                     = 0x00010000,
          MAC_REG_RSP6_RSP6_MASK_GROUP_ADDRESS                                  = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_MASK_GROUP_ADDRESS_SHIFT                            = 15,
          MAC_REG_RSP6_RSP6_MASK_GROUP_ADDRESS_MASK                             = 0x00008000,
          MAC_REG_RSP6_RSP6_MASK_OPERATION                                      = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_MASK_OPERATION_SHIFT                                = 14,
          MAC_REG_RSP6_RSP6_MASK_OPERATION_MASK                                 = 0x00004000,
          MAC_REG_RSP6_RSP6_MASK_RX_INFO_AGG                                    = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_MASK_RX_INFO_AGG_SHIFT                              = 13,
          MAC_REG_RSP6_RSP6_MASK_RX_INFO_AGG_MASK                               = 0x00002000,
          MAC_REG_RSP6_RSP6_RX_INFO_OK                                          = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_RX_INFO_OK_SHIFT                                    = 12,
          MAC_REG_RSP6_RSP6_RX_INFO_OK_MASK                                     = 0x00001000,
          MAC_REG_RSP6_RSP6_RX_INFO_NDP                                         = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_RX_INFO_NDP_SHIFT                                   = 11,
          MAC_REG_RSP6_RSP6_RX_INFO_NDP_MASK                                    = 0x00000800,
          MAC_REG_RSP6_RSP6_RX_NDP_PSPOLL                                       = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_RX_NDP_PSPOLL_SHIFT                                 = 10,
          MAC_REG_RSP6_RSP6_RX_NDP_PSPOLL_MASK                                  = 0x00000400,
          MAC_REG_RSP6_RSP6_RX_PV0_TYPE                                         = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_RX_PV0_TYPE_SHIFT                                   = 8,
          MAC_REG_RSP6_RSP6_RX_PV0_TYPE_MASK                                    = 0x00000300,
          MAC_REG_RSP6_RSP6_RX_PV0_SUBTYPE                                      = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_RX_PV0_SUBTYPE_SHIFT                                = 4,
          MAC_REG_RSP6_RSP6_RX_PV0_SUBTYPE_MASK                                 = 0x000000f0,
          MAC_REG_RSP6_RSP6_RX_PV                                               = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_RX_PV_SHIFT                                         = 3,
          MAC_REG_RSP6_RSP6_RX_PV_MASK                                          = 0x00000008,
          MAC_REG_RSP6_RSP6_GROUP_ADDRESS                                       = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_GROUP_ADDRESS_SHIFT                                 = 2,
          MAC_REG_RSP6_RSP6_GROUP_ADDRESS_MASK                                  = 0x00000004,
          MAC_REG_RSP6_RSP6_OPERATION                                           = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_OPERATION_SHIFT                                     = 1,
          MAC_REG_RSP6_RSP6_OPERATION_MASK                                      = 0x00000002,
          MAC_REG_RSP6_RSP6_RX_INFO_AGG                                         = MAC_REG_RSP6,
          MAC_REG_RSP6_RSP6_RX_INFO_AGG_SHIFT                                   = 0,
          MAC_REG_RSP6_RSP6_RX_INFO_AGG_MASK                                    = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP7                                                                = 0x00900148,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP7_RSP7_VALID                                               = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_VALID_SHIFT                                         = 31,
          MAC_REG_RSP7_RSP7_VALID_MASK                                          = 0x80000000,
          MAC_REG_RSP7_RSP7_CASE                                                = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_CASE_SHIFT                                          = 26,
          MAC_REG_RSP7_RSP7_CASE_MASK                                           = 0x7c000000,
          MAC_REG_RSP7_RSP7_MASK_RX_INFO_OK                                     = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_MASK_RX_INFO_OK_SHIFT                               = 25,
          MAC_REG_RSP7_RSP7_MASK_RX_INFO_OK_MASK                                = 0x02000000,
          MAC_REG_RSP7_RSP7_MASK_RX_INFO_NDP                                    = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_MASK_RX_INFO_NDP_SHIFT                              = 24,
          MAC_REG_RSP7_RSP7_MASK_RX_INFO_NDP_MASK                               = 0x01000000,
          MAC_REG_RSP7_RSP7_MASK_RX_NDP_PSPOLL                                  = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_MASK_RX_NDP_PSPOLL_SHIFT                            = 23,
          MAC_REG_RSP7_RSP7_MASK_RX_NDP_PSPOLL_MASK                             = 0x00800000,
          MAC_REG_RSP7_RSP7_MASK_RX_PV0_TYPE                                    = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_MASK_RX_PV0_TYPE_SHIFT                              = 21,
          MAC_REG_RSP7_RSP7_MASK_RX_PV0_TYPE_MASK                               = 0x00600000,
          MAC_REG_RSP7_RSP7_MASK_RX_PV0_SUBTYPE                                 = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_MASK_RX_PV0_SUBTYPE_SHIFT                           = 17,
          MAC_REG_RSP7_RSP7_MASK_RX_PV0_SUBTYPE_MASK                            = 0x001e0000,
          MAC_REG_RSP7_RSP7_MASK_RX_PV                                          = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_MASK_RX_PV_SHIFT                                    = 16,
          MAC_REG_RSP7_RSP7_MASK_RX_PV_MASK                                     = 0x00010000,
          MAC_REG_RSP7_RSP7_MASK_GROUP_ADDRESS                                  = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_MASK_GROUP_ADDRESS_SHIFT                            = 15,
          MAC_REG_RSP7_RSP7_MASK_GROUP_ADDRESS_MASK                             = 0x00008000,
          MAC_REG_RSP7_RSP7_MASK_OPERATION                                      = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_MASK_OPERATION_SHIFT                                = 14,
          MAC_REG_RSP7_RSP7_MASK_OPERATION_MASK                                 = 0x00004000,
          MAC_REG_RSP7_RSP7_MASK_RX_INFO_AGG                                    = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_MASK_RX_INFO_AGG_SHIFT                              = 13,
          MAC_REG_RSP7_RSP7_MASK_RX_INFO_AGG_MASK                               = 0x00002000,
          MAC_REG_RSP7_RSP7_RX_INFO_OK                                          = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_RX_INFO_OK_SHIFT                                    = 12,
          MAC_REG_RSP7_RSP7_RX_INFO_OK_MASK                                     = 0x00001000,
          MAC_REG_RSP7_RSP7_RX_INFO_NDP                                         = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_RX_INFO_NDP_SHIFT                                   = 11,
          MAC_REG_RSP7_RSP7_RX_INFO_NDP_MASK                                    = 0x00000800,
          MAC_REG_RSP7_RSP7_RX_NDP_PSPOLL                                       = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_RX_NDP_PSPOLL_SHIFT                                 = 10,
          MAC_REG_RSP7_RSP7_RX_NDP_PSPOLL_MASK                                  = 0x00000400,
          MAC_REG_RSP7_RSP7_RX_PV0_TYPE                                         = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_RX_PV0_TYPE_SHIFT                                   = 8,
          MAC_REG_RSP7_RSP7_RX_PV0_TYPE_MASK                                    = 0x00000300,
          MAC_REG_RSP7_RSP7_RX_PV0_SUBTYPE                                      = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_RX_PV0_SUBTYPE_SHIFT                                = 4,
          MAC_REG_RSP7_RSP7_RX_PV0_SUBTYPE_MASK                                 = 0x000000f0,
          MAC_REG_RSP7_RSP7_RX_PV                                               = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_RX_PV_SHIFT                                         = 3,
          MAC_REG_RSP7_RSP7_RX_PV_MASK                                          = 0x00000008,
          MAC_REG_RSP7_RSP7_GROUP_ADDRESS                                       = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_GROUP_ADDRESS_SHIFT                                 = 2,
          MAC_REG_RSP7_RSP7_GROUP_ADDRESS_MASK                                  = 0x00000004,
          MAC_REG_RSP7_RSP7_OPERATION                                           = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_OPERATION_SHIFT                                     = 1,
          MAC_REG_RSP7_RSP7_OPERATION_MASK                                      = 0x00000002,
          MAC_REG_RSP7_RSP7_RX_INFO_AGG                                         = MAC_REG_RSP7,
          MAC_REG_RSP7_RSP7_RX_INFO_AGG_SHIFT                                   = 0,
          MAC_REG_RSP7_RSP7_RX_INFO_AGG_MASK                                    = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP8                                                                = 0x0090014c,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP8_RSP8_VALID                                               = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_VALID_SHIFT                                         = 31,
          MAC_REG_RSP8_RSP8_VALID_MASK                                          = 0x80000000,
          MAC_REG_RSP8_RSP8_CASE                                                = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_CASE_SHIFT                                          = 26,
          MAC_REG_RSP8_RSP8_CASE_MASK                                           = 0x7c000000,
          MAC_REG_RSP8_RSP8_MASK_RX_INFO_OK                                     = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_MASK_RX_INFO_OK_SHIFT                               = 25,
          MAC_REG_RSP8_RSP8_MASK_RX_INFO_OK_MASK                                = 0x02000000,
          MAC_REG_RSP8_RSP8_MASK_RX_INFO_NDP                                    = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_MASK_RX_INFO_NDP_SHIFT                              = 24,
          MAC_REG_RSP8_RSP8_MASK_RX_INFO_NDP_MASK                               = 0x01000000,
          MAC_REG_RSP8_RSP8_MASK_RX_NDP_PSPOLL                                  = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_MASK_RX_NDP_PSPOLL_SHIFT                            = 23,
          MAC_REG_RSP8_RSP8_MASK_RX_NDP_PSPOLL_MASK                             = 0x00800000,
          MAC_REG_RSP8_RSP8_MASK_RX_PV0_TYPE                                    = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_MASK_RX_PV0_TYPE_SHIFT                              = 21,
          MAC_REG_RSP8_RSP8_MASK_RX_PV0_TYPE_MASK                               = 0x00600000,
          MAC_REG_RSP8_RSP8_MASK_RX_PV0_SUBTYPE                                 = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_MASK_RX_PV0_SUBTYPE_SHIFT                           = 17,
          MAC_REG_RSP8_RSP8_MASK_RX_PV0_SUBTYPE_MASK                            = 0x001e0000,
          MAC_REG_RSP8_RSP8_MASK_RX_PV                                          = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_MASK_RX_PV_SHIFT                                    = 16,
          MAC_REG_RSP8_RSP8_MASK_RX_PV_MASK                                     = 0x00010000,
          MAC_REG_RSP8_RSP8_MASK_GROUP_ADDRESS                                  = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_MASK_GROUP_ADDRESS_SHIFT                            = 15,
          MAC_REG_RSP8_RSP8_MASK_GROUP_ADDRESS_MASK                             = 0x00008000,
          MAC_REG_RSP8_RSP8_MASK_OPERATION                                      = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_MASK_OPERATION_SHIFT                                = 14,
          MAC_REG_RSP8_RSP8_MASK_OPERATION_MASK                                 = 0x00004000,
          MAC_REG_RSP8_RSP8_MASK_RX_INFO_AGG                                    = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_MASK_RX_INFO_AGG_SHIFT                              = 13,
          MAC_REG_RSP8_RSP8_MASK_RX_INFO_AGG_MASK                               = 0x00002000,
          MAC_REG_RSP8_RSP8_RX_INFO_OK                                          = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_RX_INFO_OK_SHIFT                                    = 12,
          MAC_REG_RSP8_RSP8_RX_INFO_OK_MASK                                     = 0x00001000,
          MAC_REG_RSP8_RSP8_RX_INFO_NDP                                         = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_RX_INFO_NDP_SHIFT                                   = 11,
          MAC_REG_RSP8_RSP8_RX_INFO_NDP_MASK                                    = 0x00000800,
          MAC_REG_RSP8_RSP8_RX_NDP_PSPOLL                                       = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_RX_NDP_PSPOLL_SHIFT                                 = 10,
          MAC_REG_RSP8_RSP8_RX_NDP_PSPOLL_MASK                                  = 0x00000400,
          MAC_REG_RSP8_RSP8_RX_PV0_TYPE                                         = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_RX_PV0_TYPE_SHIFT                                   = 8,
          MAC_REG_RSP8_RSP8_RX_PV0_TYPE_MASK                                    = 0x00000300,
          MAC_REG_RSP8_RSP8_RX_PV0_SUBTYPE                                      = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_RX_PV0_SUBTYPE_SHIFT                                = 4,
          MAC_REG_RSP8_RSP8_RX_PV0_SUBTYPE_MASK                                 = 0x000000f0,
          MAC_REG_RSP8_RSP8_RX_PV                                               = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_RX_PV_SHIFT                                         = 3,
          MAC_REG_RSP8_RSP8_RX_PV_MASK                                          = 0x00000008,
          MAC_REG_RSP8_RSP8_GROUP_ADDRESS                                       = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_GROUP_ADDRESS_SHIFT                                 = 2,
          MAC_REG_RSP8_RSP8_GROUP_ADDRESS_MASK                                  = 0x00000004,
          MAC_REG_RSP8_RSP8_OPERATION                                           = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_OPERATION_SHIFT                                     = 1,
          MAC_REG_RSP8_RSP8_OPERATION_MASK                                      = 0x00000002,
          MAC_REG_RSP8_RSP8_RX_INFO_AGG                                         = MAC_REG_RSP8,
          MAC_REG_RSP8_RSP8_RX_INFO_AGG_SHIFT                                   = 0,
          MAC_REG_RSP8_RSP8_RX_INFO_AGG_MASK                                    = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP9                                                                = 0x00900150,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP9_RSP9_VALID                                               = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_VALID_SHIFT                                         = 31,
          MAC_REG_RSP9_RSP9_VALID_MASK                                          = 0x80000000,
          MAC_REG_RSP9_RSP9_CASE                                                = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_CASE_SHIFT                                          = 26,
          MAC_REG_RSP9_RSP9_CASE_MASK                                           = 0x7c000000,
          MAC_REG_RSP9_RSP9_MASK_RX_INFO_OK                                     = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_MASK_RX_INFO_OK_SHIFT                               = 25,
          MAC_REG_RSP9_RSP9_MASK_RX_INFO_OK_MASK                                = 0x02000000,
          MAC_REG_RSP9_RSP9_MASK_RX_INFO_NDP                                    = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_MASK_RX_INFO_NDP_SHIFT                              = 24,
          MAC_REG_RSP9_RSP9_MASK_RX_INFO_NDP_MASK                               = 0x01000000,
          MAC_REG_RSP9_RSP9_MASK_RX_NDP_PSPOLL                                  = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_MASK_RX_NDP_PSPOLL_SHIFT                            = 23,
          MAC_REG_RSP9_RSP9_MASK_RX_NDP_PSPOLL_MASK                             = 0x00800000,
          MAC_REG_RSP9_RSP9_MASK_RX_PV0_TYPE                                    = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_MASK_RX_PV0_TYPE_SHIFT                              = 21,
          MAC_REG_RSP9_RSP9_MASK_RX_PV0_TYPE_MASK                               = 0x00600000,
          MAC_REG_RSP9_RSP9_MASK_RX_PV0_SUBTYPE                                 = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_MASK_RX_PV0_SUBTYPE_SHIFT                           = 17,
          MAC_REG_RSP9_RSP9_MASK_RX_PV0_SUBTYPE_MASK                            = 0x001e0000,
          MAC_REG_RSP9_RSP9_MASK_RX_PV                                          = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_MASK_RX_PV_SHIFT                                    = 16,
          MAC_REG_RSP9_RSP9_MASK_RX_PV_MASK                                     = 0x00010000,
          MAC_REG_RSP9_RSP9_MASK_GROUP_ADDRESS                                  = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_MASK_GROUP_ADDRESS_SHIFT                            = 15,
          MAC_REG_RSP9_RSP9_MASK_GROUP_ADDRESS_MASK                             = 0x00008000,
          MAC_REG_RSP9_RSP9_MASK_OPERATION                                      = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_MASK_OPERATION_SHIFT                                = 14,
          MAC_REG_RSP9_RSP9_MASK_OPERATION_MASK                                 = 0x00004000,
          MAC_REG_RSP9_RSP9_MASK_RX_INFO_AGG                                    = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_MASK_RX_INFO_AGG_SHIFT                              = 13,
          MAC_REG_RSP9_RSP9_MASK_RX_INFO_AGG_MASK                               = 0x00002000,
          MAC_REG_RSP9_RSP9_RX_INFO_OK                                          = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_RX_INFO_OK_SHIFT                                    = 12,
          MAC_REG_RSP9_RSP9_RX_INFO_OK_MASK                                     = 0x00001000,
          MAC_REG_RSP9_RSP9_RX_INFO_NDP                                         = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_RX_INFO_NDP_SHIFT                                   = 11,
          MAC_REG_RSP9_RSP9_RX_INFO_NDP_MASK                                    = 0x00000800,
          MAC_REG_RSP9_RSP9_RX_NDP_PSPOLL                                       = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_RX_NDP_PSPOLL_SHIFT                                 = 10,
          MAC_REG_RSP9_RSP9_RX_NDP_PSPOLL_MASK                                  = 0x00000400,
          MAC_REG_RSP9_RSP9_RX_PV0_TYPE                                         = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_RX_PV0_TYPE_SHIFT                                   = 8,
          MAC_REG_RSP9_RSP9_RX_PV0_TYPE_MASK                                    = 0x00000300,
          MAC_REG_RSP9_RSP9_RX_PV0_SUBTYPE                                      = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_RX_PV0_SUBTYPE_SHIFT                                = 4,
          MAC_REG_RSP9_RSP9_RX_PV0_SUBTYPE_MASK                                 = 0x000000f0,
          MAC_REG_RSP9_RSP9_RX_PV                                               = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_RX_PV_SHIFT                                         = 3,
          MAC_REG_RSP9_RSP9_RX_PV_MASK                                          = 0x00000008,
          MAC_REG_RSP9_RSP9_GROUP_ADDRESS                                       = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_GROUP_ADDRESS_SHIFT                                 = 2,
          MAC_REG_RSP9_RSP9_GROUP_ADDRESS_MASK                                  = 0x00000004,
          MAC_REG_RSP9_RSP9_OPERATION                                           = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_OPERATION_SHIFT                                     = 1,
          MAC_REG_RSP9_RSP9_OPERATION_MASK                                      = 0x00000002,
          MAC_REG_RSP9_RSP9_RX_INFO_AGG                                         = MAC_REG_RSP9,
          MAC_REG_RSP9_RSP9_RX_INFO_AGG_SHIFT                                   = 0,
          MAC_REG_RSP9_RSP9_RX_INFO_AGG_MASK                                    = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP10                                                               = 0x00900154,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP10_RSP10_VALID                                             = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_VALID_SHIFT                                       = 31,
          MAC_REG_RSP10_RSP10_VALID_MASK                                        = 0x80000000,
          MAC_REG_RSP10_RSP10_CASE                                              = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_CASE_SHIFT                                        = 26,
          MAC_REG_RSP10_RSP10_CASE_MASK                                         = 0x7c000000,
          MAC_REG_RSP10_RSP10_MASK_RX_INFO_OK                                   = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_MASK_RX_INFO_OK_SHIFT                             = 25,
          MAC_REG_RSP10_RSP10_MASK_RX_INFO_OK_MASK                              = 0x02000000,
          MAC_REG_RSP10_RSP10_MASK_RX_INFO_NDP                                  = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_MASK_RX_INFO_NDP_SHIFT                            = 24,
          MAC_REG_RSP10_RSP10_MASK_RX_INFO_NDP_MASK                             = 0x01000000,
          MAC_REG_RSP10_RSP10_MASK_RX_NDP_PSPOLL                                = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_MASK_RX_NDP_PSPOLL_SHIFT                          = 23,
          MAC_REG_RSP10_RSP10_MASK_RX_NDP_PSPOLL_MASK                           = 0x00800000,
          MAC_REG_RSP10_RSP10_MASK_RX_PV0_TYPE                                  = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_MASK_RX_PV0_TYPE_SHIFT                            = 21,
          MAC_REG_RSP10_RSP10_MASK_RX_PV0_TYPE_MASK                             = 0x00600000,
          MAC_REG_RSP10_RSP10_MASK_RX_PV0_SUBTYPE                               = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_MASK_RX_PV0_SUBTYPE_SHIFT                         = 17,
          MAC_REG_RSP10_RSP10_MASK_RX_PV0_SUBTYPE_MASK                          = 0x001e0000,
          MAC_REG_RSP10_RSP10_MASK_RX_PV                                        = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_MASK_RX_PV_SHIFT                                  = 16,
          MAC_REG_RSP10_RSP10_MASK_RX_PV_MASK                                   = 0x00010000,
          MAC_REG_RSP10_RSP10_MASK_GROUP_ADDRESS                                = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_MASK_GROUP_ADDRESS_SHIFT                          = 15,
          MAC_REG_RSP10_RSP10_MASK_GROUP_ADDRESS_MASK                           = 0x00008000,
          MAC_REG_RSP10_RSP10_MASK_OPERATION                                    = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_MASK_OPERATION_SHIFT                              = 14,
          MAC_REG_RSP10_RSP10_MASK_OPERATION_MASK                               = 0x00004000,
          MAC_REG_RSP10_RSP10_MASK_RX_INFO_AGG                                  = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_MASK_RX_INFO_AGG_SHIFT                            = 13,
          MAC_REG_RSP10_RSP10_MASK_RX_INFO_AGG_MASK                             = 0x00002000,
          MAC_REG_RSP10_RSP10_RX_INFO_OK                                        = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_RX_INFO_OK_SHIFT                                  = 12,
          MAC_REG_RSP10_RSP10_RX_INFO_OK_MASK                                   = 0x00001000,
          MAC_REG_RSP10_RSP10_RX_INFO_NDP                                       = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_RX_INFO_NDP_SHIFT                                 = 11,
          MAC_REG_RSP10_RSP10_RX_INFO_NDP_MASK                                  = 0x00000800,
          MAC_REG_RSP10_RSP10_RX_NDP_PSPOLL                                     = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_RX_NDP_PSPOLL_SHIFT                               = 10,
          MAC_REG_RSP10_RSP10_RX_NDP_PSPOLL_MASK                                = 0x00000400,
          MAC_REG_RSP10_RSP10_RX_PV0_TYPE                                       = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_RX_PV0_TYPE_SHIFT                                 = 8,
          MAC_REG_RSP10_RSP10_RX_PV0_TYPE_MASK                                  = 0x00000300,
          MAC_REG_RSP10_RSP10_RX_PV0_SUBTYPE                                    = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_RX_PV0_SUBTYPE_SHIFT                              = 4,
          MAC_REG_RSP10_RSP10_RX_PV0_SUBTYPE_MASK                               = 0x000000f0,
          MAC_REG_RSP10_RSP10_RX_PV                                             = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_RX_PV_SHIFT                                       = 3,
          MAC_REG_RSP10_RSP10_RX_PV_MASK                                        = 0x00000008,
          MAC_REG_RSP10_RSP10_GROUP_ADDRESS                                     = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_GROUP_ADDRESS_SHIFT                               = 2,
          MAC_REG_RSP10_RSP10_GROUP_ADDRESS_MASK                                = 0x00000004,
          MAC_REG_RSP10_RSP10_OPERATION                                         = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_OPERATION_SHIFT                                   = 1,
          MAC_REG_RSP10_RSP10_OPERATION_MASK                                    = 0x00000002,
          MAC_REG_RSP10_RSP10_RX_INFO_AGG                                       = MAC_REG_RSP10,
          MAC_REG_RSP10_RSP10_RX_INFO_AGG_SHIFT                                 = 0,
          MAC_REG_RSP10_RSP10_RX_INFO_AGG_MASK                                  = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP11                                                               = 0x00900158,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP11_RSP11_VALID                                             = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_VALID_SHIFT                                       = 31,
          MAC_REG_RSP11_RSP11_VALID_MASK                                        = 0x80000000,
          MAC_REG_RSP11_RSP11_CASE                                              = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_CASE_SHIFT                                        = 26,
          MAC_REG_RSP11_RSP11_CASE_MASK                                         = 0x7c000000,
          MAC_REG_RSP11_RSP11_MASK_RX_INFO_OK                                   = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_MASK_RX_INFO_OK_SHIFT                             = 25,
          MAC_REG_RSP11_RSP11_MASK_RX_INFO_OK_MASK                              = 0x02000000,
          MAC_REG_RSP11_RSP11_MASK_RX_INFO_NDP                                  = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_MASK_RX_INFO_NDP_SHIFT                            = 24,
          MAC_REG_RSP11_RSP11_MASK_RX_INFO_NDP_MASK                             = 0x01000000,
          MAC_REG_RSP11_RSP11_MASK_RX_NDP_PSPOLL                                = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_MASK_RX_NDP_PSPOLL_SHIFT                          = 23,
          MAC_REG_RSP11_RSP11_MASK_RX_NDP_PSPOLL_MASK                           = 0x00800000,
          MAC_REG_RSP11_RSP11_MASK_RX_PV0_TYPE                                  = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_MASK_RX_PV0_TYPE_SHIFT                            = 21,
          MAC_REG_RSP11_RSP11_MASK_RX_PV0_TYPE_MASK                             = 0x00600000,
          MAC_REG_RSP11_RSP11_MASK_RX_PV0_SUBTYPE                               = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_MASK_RX_PV0_SUBTYPE_SHIFT                         = 17,
          MAC_REG_RSP11_RSP11_MASK_RX_PV0_SUBTYPE_MASK                          = 0x001e0000,
          MAC_REG_RSP11_RSP11_MASK_RX_PV                                        = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_MASK_RX_PV_SHIFT                                  = 16,
          MAC_REG_RSP11_RSP11_MASK_RX_PV_MASK                                   = 0x00010000,
          MAC_REG_RSP11_RSP11_MASK_GROUP_ADDRESS                                = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_MASK_GROUP_ADDRESS_SHIFT                          = 15,
          MAC_REG_RSP11_RSP11_MASK_GROUP_ADDRESS_MASK                           = 0x00008000,
          MAC_REG_RSP11_RSP11_MASK_OPERATION                                    = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_MASK_OPERATION_SHIFT                              = 14,
          MAC_REG_RSP11_RSP11_MASK_OPERATION_MASK                               = 0x00004000,
          MAC_REG_RSP11_RSP11_MASK_RX_INFO_AGG                                  = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_MASK_RX_INFO_AGG_SHIFT                            = 13,
          MAC_REG_RSP11_RSP11_MASK_RX_INFO_AGG_MASK                             = 0x00002000,
          MAC_REG_RSP11_RSP11_RX_INFO_OK                                        = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_RX_INFO_OK_SHIFT                                  = 12,
          MAC_REG_RSP11_RSP11_RX_INFO_OK_MASK                                   = 0x00001000,
          MAC_REG_RSP11_RSP11_RX_INFO_NDP                                       = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_RX_INFO_NDP_SHIFT                                 = 11,
          MAC_REG_RSP11_RSP11_RX_INFO_NDP_MASK                                  = 0x00000800,
          MAC_REG_RSP11_RSP11_RX_NDP_PSPOLL                                     = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_RX_NDP_PSPOLL_SHIFT                               = 10,
          MAC_REG_RSP11_RSP11_RX_NDP_PSPOLL_MASK                                = 0x00000400,
          MAC_REG_RSP11_RSP11_RX_PV0_TYPE                                       = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_RX_PV0_TYPE_SHIFT                                 = 8,
          MAC_REG_RSP11_RSP11_RX_PV0_TYPE_MASK                                  = 0x00000300,
          MAC_REG_RSP11_RSP11_RX_PV0_SUBTYPE                                    = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_RX_PV0_SUBTYPE_SHIFT                              = 4,
          MAC_REG_RSP11_RSP11_RX_PV0_SUBTYPE_MASK                               = 0x000000f0,
          MAC_REG_RSP11_RSP11_RX_PV                                             = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_RX_PV_SHIFT                                       = 3,
          MAC_REG_RSP11_RSP11_RX_PV_MASK                                        = 0x00000008,
          MAC_REG_RSP11_RSP11_GROUP_ADDRESS                                     = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_GROUP_ADDRESS_SHIFT                               = 2,
          MAC_REG_RSP11_RSP11_GROUP_ADDRESS_MASK                                = 0x00000004,
          MAC_REG_RSP11_RSP11_OPERATION                                         = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_OPERATION_SHIFT                                   = 1,
          MAC_REG_RSP11_RSP11_OPERATION_MASK                                    = 0x00000002,
          MAC_REG_RSP11_RSP11_RX_INFO_AGG                                       = MAC_REG_RSP11,
          MAC_REG_RSP11_RSP11_RX_INFO_AGG_SHIFT                                 = 0,
          MAC_REG_RSP11_RSP11_RX_INFO_AGG_MASK                                  = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP12                                                               = 0x0090015c,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP12_RSP12_VALID                                             = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_VALID_SHIFT                                       = 31,
          MAC_REG_RSP12_RSP12_VALID_MASK                                        = 0x80000000,
          MAC_REG_RSP12_RSP12_CASE                                              = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_CASE_SHIFT                                        = 26,
          MAC_REG_RSP12_RSP12_CASE_MASK                                         = 0x7c000000,
          MAC_REG_RSP12_RSP12_MASK_RX_INFO_OK                                   = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_MASK_RX_INFO_OK_SHIFT                             = 25,
          MAC_REG_RSP12_RSP12_MASK_RX_INFO_OK_MASK                              = 0x02000000,
          MAC_REG_RSP12_RSP12_MASK_RX_INFO_NDP                                  = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_MASK_RX_INFO_NDP_SHIFT                            = 24,
          MAC_REG_RSP12_RSP12_MASK_RX_INFO_NDP_MASK                             = 0x01000000,
          MAC_REG_RSP12_RSP12_MASK_RX_NDP_PSPOLL                                = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_MASK_RX_NDP_PSPOLL_SHIFT                          = 23,
          MAC_REG_RSP12_RSP12_MASK_RX_NDP_PSPOLL_MASK                           = 0x00800000,
          MAC_REG_RSP12_RSP12_MASK_RX_PV0_TYPE                                  = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_MASK_RX_PV0_TYPE_SHIFT                            = 21,
          MAC_REG_RSP12_RSP12_MASK_RX_PV0_TYPE_MASK                             = 0x00600000,
          MAC_REG_RSP12_RSP12_MASK_RX_PV0_SUBTYPE                               = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_MASK_RX_PV0_SUBTYPE_SHIFT                         = 17,
          MAC_REG_RSP12_RSP12_MASK_RX_PV0_SUBTYPE_MASK                          = 0x001e0000,
          MAC_REG_RSP12_RSP12_MASK_RX_PV                                        = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_MASK_RX_PV_SHIFT                                  = 16,
          MAC_REG_RSP12_RSP12_MASK_RX_PV_MASK                                   = 0x00010000,
          MAC_REG_RSP12_RSP12_MASK_GROUP_ADDRESS                                = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_MASK_GROUP_ADDRESS_SHIFT                          = 15,
          MAC_REG_RSP12_RSP12_MASK_GROUP_ADDRESS_MASK                           = 0x00008000,
          MAC_REG_RSP12_RSP12_MASK_OPERATION                                    = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_MASK_OPERATION_SHIFT                              = 14,
          MAC_REG_RSP12_RSP12_MASK_OPERATION_MASK                               = 0x00004000,
          MAC_REG_RSP12_RSP12_MASK_RX_INFO_AGG                                  = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_MASK_RX_INFO_AGG_SHIFT                            = 13,
          MAC_REG_RSP12_RSP12_MASK_RX_INFO_AGG_MASK                             = 0x00002000,
          MAC_REG_RSP12_RSP12_RX_INFO_OK                                        = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_RX_INFO_OK_SHIFT                                  = 12,
          MAC_REG_RSP12_RSP12_RX_INFO_OK_MASK                                   = 0x00001000,
          MAC_REG_RSP12_RSP12_RX_INFO_NDP                                       = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_RX_INFO_NDP_SHIFT                                 = 11,
          MAC_REG_RSP12_RSP12_RX_INFO_NDP_MASK                                  = 0x00000800,
          MAC_REG_RSP12_RSP12_RX_NDP_PSPOLL                                     = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_RX_NDP_PSPOLL_SHIFT                               = 10,
          MAC_REG_RSP12_RSP12_RX_NDP_PSPOLL_MASK                                = 0x00000400,
          MAC_REG_RSP12_RSP12_RX_PV0_TYPE                                       = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_RX_PV0_TYPE_SHIFT                                 = 8,
          MAC_REG_RSP12_RSP12_RX_PV0_TYPE_MASK                                  = 0x00000300,
          MAC_REG_RSP12_RSP12_RX_PV0_SUBTYPE                                    = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_RX_PV0_SUBTYPE_SHIFT                              = 4,
          MAC_REG_RSP12_RSP12_RX_PV0_SUBTYPE_MASK                               = 0x000000f0,
          MAC_REG_RSP12_RSP12_RX_PV                                             = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_RX_PV_SHIFT                                       = 3,
          MAC_REG_RSP12_RSP12_RX_PV_MASK                                        = 0x00000008,
          MAC_REG_RSP12_RSP12_GROUP_ADDRESS                                     = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_GROUP_ADDRESS_SHIFT                               = 2,
          MAC_REG_RSP12_RSP12_GROUP_ADDRESS_MASK                                = 0x00000004,
          MAC_REG_RSP12_RSP12_OPERATION                                         = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_OPERATION_SHIFT                                   = 1,
          MAC_REG_RSP12_RSP12_OPERATION_MASK                                    = 0x00000002,
          MAC_REG_RSP12_RSP12_RX_INFO_AGG                                       = MAC_REG_RSP12,
          MAC_REG_RSP12_RSP12_RX_INFO_AGG_SHIFT                                 = 0,
          MAC_REG_RSP12_RSP12_RX_INFO_AGG_MASK                                  = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP13                                                               = 0x00900160,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP13_RSP13_VALID                                             = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_VALID_SHIFT                                       = 31,
          MAC_REG_RSP13_RSP13_VALID_MASK                                        = 0x80000000,
          MAC_REG_RSP13_RSP13_CASE                                              = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_CASE_SHIFT                                        = 26,
          MAC_REG_RSP13_RSP13_CASE_MASK                                         = 0x7c000000,
          MAC_REG_RSP13_RSP13_MASK_RX_INFO_OK                                   = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_MASK_RX_INFO_OK_SHIFT                             = 25,
          MAC_REG_RSP13_RSP13_MASK_RX_INFO_OK_MASK                              = 0x02000000,
          MAC_REG_RSP13_RSP13_MASK_RX_INFO_NDP                                  = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_MASK_RX_INFO_NDP_SHIFT                            = 24,
          MAC_REG_RSP13_RSP13_MASK_RX_INFO_NDP_MASK                             = 0x01000000,
          MAC_REG_RSP13_RSP13_MASK_RX_NDP_PSPOLL                                = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_MASK_RX_NDP_PSPOLL_SHIFT                          = 23,
          MAC_REG_RSP13_RSP13_MASK_RX_NDP_PSPOLL_MASK                           = 0x00800000,
          MAC_REG_RSP13_RSP13_MASK_RX_PV0_TYPE                                  = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_MASK_RX_PV0_TYPE_SHIFT                            = 21,
          MAC_REG_RSP13_RSP13_MASK_RX_PV0_TYPE_MASK                             = 0x00600000,
          MAC_REG_RSP13_RSP13_MASK_RX_PV0_SUBTYPE                               = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_MASK_RX_PV0_SUBTYPE_SHIFT                         = 17,
          MAC_REG_RSP13_RSP13_MASK_RX_PV0_SUBTYPE_MASK                          = 0x001e0000,
          MAC_REG_RSP13_RSP13_MASK_RX_PV                                        = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_MASK_RX_PV_SHIFT                                  = 16,
          MAC_REG_RSP13_RSP13_MASK_RX_PV_MASK                                   = 0x00010000,
          MAC_REG_RSP13_RSP13_MASK_GROUP_ADDRESS                                = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_MASK_GROUP_ADDRESS_SHIFT                          = 15,
          MAC_REG_RSP13_RSP13_MASK_GROUP_ADDRESS_MASK                           = 0x00008000,
          MAC_REG_RSP13_RSP13_MASK_OPERATION                                    = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_MASK_OPERATION_SHIFT                              = 14,
          MAC_REG_RSP13_RSP13_MASK_OPERATION_MASK                               = 0x00004000,
          MAC_REG_RSP13_RSP13_MASK_RX_INFO_AGG                                  = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_MASK_RX_INFO_AGG_SHIFT                            = 13,
          MAC_REG_RSP13_RSP13_MASK_RX_INFO_AGG_MASK                             = 0x00002000,
          MAC_REG_RSP13_RSP13_RX_INFO_OK                                        = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_RX_INFO_OK_SHIFT                                  = 12,
          MAC_REG_RSP13_RSP13_RX_INFO_OK_MASK                                   = 0x00001000,
          MAC_REG_RSP13_RSP13_RX_INFO_NDP                                       = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_RX_INFO_NDP_SHIFT                                 = 11,
          MAC_REG_RSP13_RSP13_RX_INFO_NDP_MASK                                  = 0x00000800,
          MAC_REG_RSP13_RSP13_RX_NDP_PSPOLL                                     = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_RX_NDP_PSPOLL_SHIFT                               = 10,
          MAC_REG_RSP13_RSP13_RX_NDP_PSPOLL_MASK                                = 0x00000400,
          MAC_REG_RSP13_RSP13_RX_PV0_TYPE                                       = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_RX_PV0_TYPE_SHIFT                                 = 8,
          MAC_REG_RSP13_RSP13_RX_PV0_TYPE_MASK                                  = 0x00000300,
          MAC_REG_RSP13_RSP13_RX_PV0_SUBTYPE                                    = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_RX_PV0_SUBTYPE_SHIFT                              = 4,
          MAC_REG_RSP13_RSP13_RX_PV0_SUBTYPE_MASK                               = 0x000000f0,
          MAC_REG_RSP13_RSP13_RX_PV                                             = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_RX_PV_SHIFT                                       = 3,
          MAC_REG_RSP13_RSP13_RX_PV_MASK                                        = 0x00000008,
          MAC_REG_RSP13_RSP13_GROUP_ADDRESS                                     = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_GROUP_ADDRESS_SHIFT                               = 2,
          MAC_REG_RSP13_RSP13_GROUP_ADDRESS_MASK                                = 0x00000004,
          MAC_REG_RSP13_RSP13_OPERATION                                         = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_OPERATION_SHIFT                                   = 1,
          MAC_REG_RSP13_RSP13_OPERATION_MASK                                    = 0x00000002,
          MAC_REG_RSP13_RSP13_RX_INFO_AGG                                       = MAC_REG_RSP13,
          MAC_REG_RSP13_RSP13_RX_INFO_AGG_SHIFT                                 = 0,
          MAC_REG_RSP13_RSP13_RX_INFO_AGG_MASK                                  = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP14                                                               = 0x00900164,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP14_RSP14_VALID                                             = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_VALID_SHIFT                                       = 31,
          MAC_REG_RSP14_RSP14_VALID_MASK                                        = 0x80000000,
          MAC_REG_RSP14_RSP14_CASE                                              = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_CASE_SHIFT                                        = 26,
          MAC_REG_RSP14_RSP14_CASE_MASK                                         = 0x7c000000,
          MAC_REG_RSP14_RSP14_MASK_RX_INFO_OK                                   = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_MASK_RX_INFO_OK_SHIFT                             = 25,
          MAC_REG_RSP14_RSP14_MASK_RX_INFO_OK_MASK                              = 0x02000000,
          MAC_REG_RSP14_RSP14_MASK_RX_INFO_NDP                                  = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_MASK_RX_INFO_NDP_SHIFT                            = 24,
          MAC_REG_RSP14_RSP14_MASK_RX_INFO_NDP_MASK                             = 0x01000000,
          MAC_REG_RSP14_RSP14_MASK_RX_NDP_PSPOLL                                = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_MASK_RX_NDP_PSPOLL_SHIFT                          = 23,
          MAC_REG_RSP14_RSP14_MASK_RX_NDP_PSPOLL_MASK                           = 0x00800000,
          MAC_REG_RSP14_RSP14_MASK_RX_PV0_TYPE                                  = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_MASK_RX_PV0_TYPE_SHIFT                            = 21,
          MAC_REG_RSP14_RSP14_MASK_RX_PV0_TYPE_MASK                             = 0x00600000,
          MAC_REG_RSP14_RSP14_MASK_RX_PV0_SUBTYPE                               = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_MASK_RX_PV0_SUBTYPE_SHIFT                         = 17,
          MAC_REG_RSP14_RSP14_MASK_RX_PV0_SUBTYPE_MASK                          = 0x001e0000,
          MAC_REG_RSP14_RSP14_MASK_RX_PV                                        = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_MASK_RX_PV_SHIFT                                  = 16,
          MAC_REG_RSP14_RSP14_MASK_RX_PV_MASK                                   = 0x00010000,
          MAC_REG_RSP14_RSP14_MASK_GROUP_ADDRESS                                = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_MASK_GROUP_ADDRESS_SHIFT                          = 15,
          MAC_REG_RSP14_RSP14_MASK_GROUP_ADDRESS_MASK                           = 0x00008000,
          MAC_REG_RSP14_RSP14_MASK_OPERATION                                    = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_MASK_OPERATION_SHIFT                              = 14,
          MAC_REG_RSP14_RSP14_MASK_OPERATION_MASK                               = 0x00004000,
          MAC_REG_RSP14_RSP14_MASK_RX_INFO_AGG                                  = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_MASK_RX_INFO_AGG_SHIFT                            = 13,
          MAC_REG_RSP14_RSP14_MASK_RX_INFO_AGG_MASK                             = 0x00002000,
          MAC_REG_RSP14_RSP14_RX_INFO_OK                                        = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_RX_INFO_OK_SHIFT                                  = 12,
          MAC_REG_RSP14_RSP14_RX_INFO_OK_MASK                                   = 0x00001000,
          MAC_REG_RSP14_RSP14_RX_INFO_NDP                                       = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_RX_INFO_NDP_SHIFT                                 = 11,
          MAC_REG_RSP14_RSP14_RX_INFO_NDP_MASK                                  = 0x00000800,
          MAC_REG_RSP14_RSP14_RX_NDP_PSPOLL                                     = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_RX_NDP_PSPOLL_SHIFT                               = 10,
          MAC_REG_RSP14_RSP14_RX_NDP_PSPOLL_MASK                                = 0x00000400,
          MAC_REG_RSP14_RSP14_RX_PV0_TYPE                                       = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_RX_PV0_TYPE_SHIFT                                 = 8,
          MAC_REG_RSP14_RSP14_RX_PV0_TYPE_MASK                                  = 0x00000300,
          MAC_REG_RSP14_RSP14_RX_PV0_SUBTYPE                                    = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_RX_PV0_SUBTYPE_SHIFT                              = 4,
          MAC_REG_RSP14_RSP14_RX_PV0_SUBTYPE_MASK                               = 0x000000f0,
          MAC_REG_RSP14_RSP14_RX_PV                                             = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_RX_PV_SHIFT                                       = 3,
          MAC_REG_RSP14_RSP14_RX_PV_MASK                                        = 0x00000008,
          MAC_REG_RSP14_RSP14_GROUP_ADDRESS                                     = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_GROUP_ADDRESS_SHIFT                               = 2,
          MAC_REG_RSP14_RSP14_GROUP_ADDRESS_MASK                                = 0x00000004,
          MAC_REG_RSP14_RSP14_OPERATION                                         = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_OPERATION_SHIFT                                   = 1,
          MAC_REG_RSP14_RSP14_OPERATION_MASK                                    = 0x00000002,
          MAC_REG_RSP14_RSP14_RX_INFO_AGG                                       = MAC_REG_RSP14,
          MAC_REG_RSP14_RSP14_RX_INFO_AGG_SHIFT                                 = 0,
          MAC_REG_RSP14_RSP14_RX_INFO_AGG_MASK                                  = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP15                                                               = 0x00900168,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP15_RSP15_VALID                                             = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_VALID_SHIFT                                       = 31,
          MAC_REG_RSP15_RSP15_VALID_MASK                                        = 0x80000000,
          MAC_REG_RSP15_RSP15_CASE                                              = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_CASE_SHIFT                                        = 26,
          MAC_REG_RSP15_RSP15_CASE_MASK                                         = 0x7c000000,
          MAC_REG_RSP15_RSP15_MASK_RX_INFO_OK                                   = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_MASK_RX_INFO_OK_SHIFT                             = 25,
          MAC_REG_RSP15_RSP15_MASK_RX_INFO_OK_MASK                              = 0x02000000,
          MAC_REG_RSP15_RSP15_MASK_RX_INFO_NDP                                  = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_MASK_RX_INFO_NDP_SHIFT                            = 24,
          MAC_REG_RSP15_RSP15_MASK_RX_INFO_NDP_MASK                             = 0x01000000,
          MAC_REG_RSP15_RSP15_MASK_RX_NDP_PSPOLL                                = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_MASK_RX_NDP_PSPOLL_SHIFT                          = 23,
          MAC_REG_RSP15_RSP15_MASK_RX_NDP_PSPOLL_MASK                           = 0x00800000,
          MAC_REG_RSP15_RSP15_MASK_RX_PV0_TYPE                                  = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_MASK_RX_PV0_TYPE_SHIFT                            = 21,
          MAC_REG_RSP15_RSP15_MASK_RX_PV0_TYPE_MASK                             = 0x00600000,
          MAC_REG_RSP15_RSP15_MASK_RX_PV0_SUBTYPE                               = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_MASK_RX_PV0_SUBTYPE_SHIFT                         = 17,
          MAC_REG_RSP15_RSP15_MASK_RX_PV0_SUBTYPE_MASK                          = 0x001e0000,
          MAC_REG_RSP15_RSP15_MASK_RX_PV                                        = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_MASK_RX_PV_SHIFT                                  = 16,
          MAC_REG_RSP15_RSP15_MASK_RX_PV_MASK                                   = 0x00010000,
          MAC_REG_RSP15_RSP15_MASK_GROUP_ADDRESS                                = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_MASK_GROUP_ADDRESS_SHIFT                          = 15,
          MAC_REG_RSP15_RSP15_MASK_GROUP_ADDRESS_MASK                           = 0x00008000,
          MAC_REG_RSP15_RSP15_MASK_OPERATION                                    = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_MASK_OPERATION_SHIFT                              = 14,
          MAC_REG_RSP15_RSP15_MASK_OPERATION_MASK                               = 0x00004000,
          MAC_REG_RSP15_RSP15_MASK_RX_INFO_AGG                                  = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_MASK_RX_INFO_AGG_SHIFT                            = 13,
          MAC_REG_RSP15_RSP15_MASK_RX_INFO_AGG_MASK                             = 0x00002000,
          MAC_REG_RSP15_RSP15_RX_INFO_OK                                        = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_RX_INFO_OK_SHIFT                                  = 12,
          MAC_REG_RSP15_RSP15_RX_INFO_OK_MASK                                   = 0x00001000,
          MAC_REG_RSP15_RSP15_RX_INFO_NDP                                       = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_RX_INFO_NDP_SHIFT                                 = 11,
          MAC_REG_RSP15_RSP15_RX_INFO_NDP_MASK                                  = 0x00000800,
          MAC_REG_RSP15_RSP15_RX_NDP_PSPOLL                                     = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_RX_NDP_PSPOLL_SHIFT                               = 10,
          MAC_REG_RSP15_RSP15_RX_NDP_PSPOLL_MASK                                = 0x00000400,
          MAC_REG_RSP15_RSP15_RX_PV0_TYPE                                       = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_RX_PV0_TYPE_SHIFT                                 = 8,
          MAC_REG_RSP15_RSP15_RX_PV0_TYPE_MASK                                  = 0x00000300,
          MAC_REG_RSP15_RSP15_RX_PV0_SUBTYPE                                    = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_RX_PV0_SUBTYPE_SHIFT                              = 4,
          MAC_REG_RSP15_RSP15_RX_PV0_SUBTYPE_MASK                               = 0x000000f0,
          MAC_REG_RSP15_RSP15_RX_PV                                             = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_RX_PV_SHIFT                                       = 3,
          MAC_REG_RSP15_RSP15_RX_PV_MASK                                        = 0x00000008,
          MAC_REG_RSP15_RSP15_GROUP_ADDRESS                                     = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_GROUP_ADDRESS_SHIFT                               = 2,
          MAC_REG_RSP15_RSP15_GROUP_ADDRESS_MASK                                = 0x00000004,
          MAC_REG_RSP15_RSP15_OPERATION                                         = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_OPERATION_SHIFT                                   = 1,
          MAC_REG_RSP15_RSP15_OPERATION_MASK                                    = 0x00000002,
          MAC_REG_RSP15_RSP15_RX_INFO_AGG                                       = MAC_REG_RSP15,
          MAC_REG_RSP15_RSP15_RX_INFO_AGG_SHIFT                                 = 0,
          MAC_REG_RSP15_RSP15_RX_INFO_AGG_MASK                                  = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP_CASE0                                                           = 0x0090016c,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP_CASE0_CASE0_IRQ_NO_ACK                                    = MAC_REG_RSP_CASE0,
          MAC_REG_RSP_CASE0_CASE0_IRQ_NO_ACK_SHIFT                              = 31,
          MAC_REG_RSP_CASE0_CASE0_IRQ_NO_ACK_MASK                               = 0x80000000,
          MAC_REG_RSP_CASE0_CASE0_CF_TYPE_NO_ACK                                = MAC_REG_RSP_CASE0,
          MAC_REG_RSP_CASE0_CASE0_CF_TYPE_NO_ACK_SHIFT                          = 24,
          MAC_REG_RSP_CASE0_CASE0_CF_TYPE_NO_ACK_MASK                           = 0x1f000000,
          MAC_REG_RSP_CASE0_CASE0_IRQ_NORMAL_ACK                                = MAC_REG_RSP_CASE0,
          MAC_REG_RSP_CASE0_CASE0_IRQ_NORMAL_ACK_SHIFT                          = 15,
          MAC_REG_RSP_CASE0_CASE0_IRQ_NORMAL_ACK_MASK                           = 0x00008000,
          MAC_REG_RSP_CASE0_CASE0_CF_TYPE_NORMAL_ACK                            = MAC_REG_RSP_CASE0,
          MAC_REG_RSP_CASE0_CASE0_CF_TYPE_NORMAL_ACK_SHIFT                      = 8,
          MAC_REG_RSP_CASE0_CASE0_CF_TYPE_NORMAL_ACK_MASK                       = 0x00001f00,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP_CASE1                                                           = 0x00900170,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP_CASE1_CASE1_IRQ_NO_ACK                                    = MAC_REG_RSP_CASE1,
          MAC_REG_RSP_CASE1_CASE1_IRQ_NO_ACK_SHIFT                              = 31,
          MAC_REG_RSP_CASE1_CASE1_IRQ_NO_ACK_MASK                               = 0x80000000,
          MAC_REG_RSP_CASE1_CASE1_CF_TYPE_NO_ACK                                = MAC_REG_RSP_CASE1,
          MAC_REG_RSP_CASE1_CASE1_CF_TYPE_NO_ACK_SHIFT                          = 24,
          MAC_REG_RSP_CASE1_CASE1_CF_TYPE_NO_ACK_MASK                           = 0x1f000000,
          MAC_REG_RSP_CASE1_CASE1_IRQ_NORMAL_ACK                                = MAC_REG_RSP_CASE1,
          MAC_REG_RSP_CASE1_CASE1_IRQ_NORMAL_ACK_SHIFT                          = 15,
          MAC_REG_RSP_CASE1_CASE1_IRQ_NORMAL_ACK_MASK                           = 0x00008000,
          MAC_REG_RSP_CASE1_CASE1_CF_TYPE_NORMAL_ACK                            = MAC_REG_RSP_CASE1,
          MAC_REG_RSP_CASE1_CASE1_CF_TYPE_NORMAL_ACK_SHIFT                      = 8,
          MAC_REG_RSP_CASE1_CASE1_CF_TYPE_NORMAL_ACK_MASK                       = 0x00001f00,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP_CASE2                                                           = 0x00900174,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP_CASE2_CASE2_IRQ_NO_ACK                                    = MAC_REG_RSP_CASE2,
          MAC_REG_RSP_CASE2_CASE2_IRQ_NO_ACK_SHIFT                              = 31,
          MAC_REG_RSP_CASE2_CASE2_IRQ_NO_ACK_MASK                               = 0x80000000,
          MAC_REG_RSP_CASE2_CASE2_CF_TYPE_NO_ACK                                = MAC_REG_RSP_CASE2,
          MAC_REG_RSP_CASE2_CASE2_CF_TYPE_NO_ACK_SHIFT                          = 24,
          MAC_REG_RSP_CASE2_CASE2_CF_TYPE_NO_ACK_MASK                           = 0x1f000000,
          MAC_REG_RSP_CASE2_CASE2_IRQ_NORMAL_ACK                                = MAC_REG_RSP_CASE2,
          MAC_REG_RSP_CASE2_CASE2_IRQ_NORMAL_ACK_SHIFT                          = 15,
          MAC_REG_RSP_CASE2_CASE2_IRQ_NORMAL_ACK_MASK                           = 0x00008000,
          MAC_REG_RSP_CASE2_CASE2_CF_TYPE_NORMAL_ACK                            = MAC_REG_RSP_CASE2,
          MAC_REG_RSP_CASE2_CASE2_CF_TYPE_NORMAL_ACK_SHIFT                      = 8,
          MAC_REG_RSP_CASE2_CASE2_CF_TYPE_NORMAL_ACK_MASK                       = 0x00001f00,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP_CASE3                                                           = 0x00900178,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RSP_CASE3_CASE3_IRQ_NO_ACK                                    = MAC_REG_RSP_CASE3,
          MAC_REG_RSP_CASE3_CASE3_IRQ_NO_ACK_SHIFT                              = 31,
          MAC_REG_RSP_CASE3_CASE3_IRQ_NO_ACK_MASK                               = 0x80000000,
          MAC_REG_RSP_CASE3_CASE3_CF_TYPE_NO_ACK                                = MAC_REG_RSP_CASE3,
          MAC_REG_RSP_CASE3_CASE3_CF_TYPE_NO_ACK_SHIFT                          = 24,
          MAC_REG_RSP_CASE3_CASE3_CF_TYPE_NO_ACK_MASK                           = 0x1f000000,
          MAC_REG_RSP_CASE3_CASE3_IRQ_NORMAL_ACK                                = MAC_REG_RSP_CASE3,
          MAC_REG_RSP_CASE3_CASE3_IRQ_NORMAL_ACK_SHIFT                          = 15,
          MAC_REG_RSP_CASE3_CASE3_IRQ_NORMAL_ACK_MASK                           = 0x00008000,
          MAC_REG_RSP_CASE3_CASE3_CF_TYPE_NORMAL_ACK                            = MAC_REG_RSP_CASE3,
          MAC_REG_RSP_CASE3_CASE3_CF_TYPE_NORMAL_ACK_SHIFT                      = 8,
          MAC_REG_RSP_CASE3_CASE3_CF_TYPE_NORMAL_ACK_MASK                       = 0x00001f00,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP_CASE4                                                           = 0x0090017c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RSP_CASE5                                                           = 0x00900180,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_AHB_WAIT_MODE_WR                                                    = 0x00900184,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_AHB_WAIT_MODE_WR_AHB_WAIT_MODE_WR_ON                          = MAC_REG_AHB_WAIT_MODE_WR,
          MAC_REG_AHB_WAIT_MODE_WR_AHB_WAIT_MODE_WR_ON_SHIFT                    = 24,
          MAC_REG_AHB_WAIT_MODE_WR_AHB_WAIT_MODE_WR_ON_MASK                     = 0x01000000,
          MAC_REG_AHB_WAIT_MODE_WR_AHB_WR_WAIT_CLOCK                            = MAC_REG_AHB_WAIT_MODE_WR,
          MAC_REG_AHB_WAIT_MODE_WR_AHB_WR_WAIT_CLOCK_SHIFT                      = 8,
          MAC_REG_AHB_WAIT_MODE_WR_AHB_WR_WAIT_CLOCK_MASK                       = 0x00ffff00,
          MAC_REG_AHB_WAIT_MODE_WR_AHB_WR_WAIT_INTERVAL                         = MAC_REG_AHB_WAIT_MODE_WR,
          MAC_REG_AHB_WAIT_MODE_WR_AHB_WR_WAIT_INTERVAL_SHIFT                   = 0,
          MAC_REG_AHB_WAIT_MODE_WR_AHB_WR_WAIT_INTERVAL_MASK                    = 0x000000ff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_AHB_WAIT_MODE_RD                                                    = 0x00900188,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_AHB_WAIT_MODE_RD_AHB_WAIT_MODE_RD_ON                          = MAC_REG_AHB_WAIT_MODE_RD,
          MAC_REG_AHB_WAIT_MODE_RD_AHB_WAIT_MODE_RD_ON_SHIFT                    = 24,
          MAC_REG_AHB_WAIT_MODE_RD_AHB_WAIT_MODE_RD_ON_MASK                     = 0x01000000,
          MAC_REG_AHB_WAIT_MODE_RD_AHB_RD_WAIT_CLOCK                            = MAC_REG_AHB_WAIT_MODE_RD,
          MAC_REG_AHB_WAIT_MODE_RD_AHB_RD_WAIT_CLOCK_SHIFT                      = 8,
          MAC_REG_AHB_WAIT_MODE_RD_AHB_RD_WAIT_CLOCK_MASK                       = 0x00ffff00,
          MAC_REG_AHB_WAIT_MODE_RD_AHB_RD_WAIT_INTERVAL                         = MAC_REG_AHB_WAIT_MODE_RD,
          MAC_REG_AHB_WAIT_MODE_RD_AHB_RD_WAIT_INTERVAL_SHIFT                   = 0,
          MAC_REG_AHB_WAIT_MODE_RD_AHB_RD_WAIT_INTERVAL_MASK                    = 0x000000ff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_DEV_INTF_MODE                                                       = 0x0090018c,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_DEV_INTF_MODE_RESET_CHANNEL_EN                                = MAC_REG_DEV_INTF_MODE,
          MAC_REG_DEV_INTF_MODE_RESET_CHANNEL_EN_SHIFT                          = 4,
          MAC_REG_DEV_INTF_MODE_RESET_CHANNEL_EN_MASK                           = 0x00000010,
          MAC_REG_DEV_INTF_MODE_DEV_INTF_ID                                     = MAC_REG_DEV_INTF_MODE,
          MAC_REG_DEV_INTF_MODE_DEV_INTF_ID_SHIFT                               = 0,
          MAC_REG_DEV_INTF_MODE_DEV_INTF_ID_MASK                                = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_DEV_INTF_CONFIG_BITMAP                                              = 0x00900190,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_DEV_INTF_CONFIG_BITMAP_2ND_INTF                               = MAC_REG_DEV_INTF_CONFIG_BITMAP,
          MAC_REG_DEV_INTF_CONFIG_BITMAP_2ND_INTF_SHIFT                         = 16,
          MAC_REG_DEV_INTF_CONFIG_BITMAP_2ND_INTF_MASK                          = 0xffff0000,
          MAC_REG_DEV_INTF_CONFIG_BITMAP_1ST_INTF                               = MAC_REG_DEV_INTF_CONFIG_BITMAP,
          MAC_REG_DEV_INTF_CONFIG_BITMAP_1ST_INTF_SHIFT                         = 0,
          MAC_REG_DEV_INTF_CONFIG_BITMAP_1ST_INTF_MASK                          = 0x0000ffff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_CONCURRENT_TIMER_PERIOD0                                            = 0x00900194,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_CONCURRENT_TIMER_PERIOD1                                            = 0x00900198,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_CONCURRENT_TIMER_COMMAND                                            = 0x0090019c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_NULL_FRAME_INFO0                                                    = 0x009001a0,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_NULL_FRAME_INFO0_NULL0_RETRY_LIMIT                            = MAC_REG_NULL_FRAME_INFO0,
          MAC_REG_NULL_FRAME_INFO0_NULL0_RETRY_LIMIT_SHIFT                      = 17,
          MAC_REG_NULL_FRAME_INFO0_NULL0_RETRY_LIMIT_MASK                       = 0x07fe0000,
          MAC_REG_NULL_FRAME_INFO0_NULL0_RETRY_COUNT                            = MAC_REG_NULL_FRAME_INFO0,
          MAC_REG_NULL_FRAME_INFO0_NULL0_RETRY_COUNT_SHIFT                      = 13,
          MAC_REG_NULL_FRAME_INFO0_NULL0_RETRY_COUNT_MASK                       = 0x0001e000,
          MAC_REG_NULL_FRAME_INFO0_NULL0_REQUEST                                = MAC_REG_NULL_FRAME_INFO0,
          MAC_REG_NULL_FRAME_INFO0_NULL0_REQUEST_SHIFT                          = 12,
          MAC_REG_NULL_FRAME_INFO0_NULL0_REQUEST_MASK                           = 0x00001000,
          MAC_REG_NULL_FRAME_INFO0_NULL0_BSSID_INDEX                            = MAC_REG_NULL_FRAME_INFO0,
          MAC_REG_NULL_FRAME_INFO0_NULL0_BSSID_INDEX_SHIFT                      = 10,
          MAC_REG_NULL_FRAME_INFO0_NULL0_BSSID_INDEX_MASK                       = 0x00000c00,
          MAC_REG_NULL_FRAME_INFO0_NULL0_MAC_ADDR_INDEX                         = MAC_REG_NULL_FRAME_INFO0,
          MAC_REG_NULL_FRAME_INFO0_NULL0_MAC_ADDR_INDEX_SHIFT                   = 8,
          MAC_REG_NULL_FRAME_INFO0_NULL0_MAC_ADDR_INDEX_MASK                    = 0x00000300,
          MAC_REG_NULL_FRAME_INFO0_NULL0_MCS32                                  = MAC_REG_NULL_FRAME_INFO0,
          MAC_REG_NULL_FRAME_INFO0_NULL0_MCS32_SHIFT                            = 7,
          MAC_REG_NULL_FRAME_INFO0_NULL0_MCS32_MASK                             = 0x00000080,
          MAC_REG_NULL_FRAME_INFO0_NULL0_MCS                                    = MAC_REG_NULL_FRAME_INFO0,
          MAC_REG_NULL_FRAME_INFO0_NULL0_MCS_SHIFT                              = 4,
          MAC_REG_NULL_FRAME_INFO0_NULL0_MCS_MASK                               = 0x00000070,
          MAC_REG_NULL_FRAME_INFO0_NULL0_GI_TYPE                                = MAC_REG_NULL_FRAME_INFO0,
          MAC_REG_NULL_FRAME_INFO0_NULL0_GI_TYPE_SHIFT                          = 3,
          MAC_REG_NULL_FRAME_INFO0_NULL0_GI_TYPE_MASK                           = 0x00000008,
          MAC_REG_NULL_FRAME_INFO0_NULL0_FORMAT                                 = MAC_REG_NULL_FRAME_INFO0,
          MAC_REG_NULL_FRAME_INFO0_NULL0_FORMAT_SHIFT                           = 0,
          MAC_REG_NULL_FRAME_INFO0_NULL0_FORMAT_MASK                            = 0x00000007,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_NULL_FRAME_INFO1                                                    = 0x009001a4,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_NULL_FRAME_INFO1_NULL1_RETRY_LIMIT                            = MAC_REG_NULL_FRAME_INFO1,
          MAC_REG_NULL_FRAME_INFO1_NULL1_RETRY_LIMIT_SHIFT                      = 17,
          MAC_REG_NULL_FRAME_INFO1_NULL1_RETRY_LIMIT_MASK                       = 0x07fe0000,
          MAC_REG_NULL_FRAME_INFO1_NULL1_RETRY_COUNT                            = MAC_REG_NULL_FRAME_INFO1,
          MAC_REG_NULL_FRAME_INFO1_NULL1_RETRY_COUNT_SHIFT                      = 13,
          MAC_REG_NULL_FRAME_INFO1_NULL1_RETRY_COUNT_MASK                       = 0x0001e000,
          MAC_REG_NULL_FRAME_INFO1_NULL1_REQUEST                                = MAC_REG_NULL_FRAME_INFO1,
          MAC_REG_NULL_FRAME_INFO1_NULL1_REQUEST_SHIFT                          = 12,
          MAC_REG_NULL_FRAME_INFO1_NULL1_REQUEST_MASK                           = 0x00001000,
          MAC_REG_NULL_FRAME_INFO1_NULL1_BSSID_INDEX                            = MAC_REG_NULL_FRAME_INFO1,
          MAC_REG_NULL_FRAME_INFO1_NULL1_BSSID_INDEX_SHIFT                      = 10,
          MAC_REG_NULL_FRAME_INFO1_NULL1_BSSID_INDEX_MASK                       = 0x00000c00,
          MAC_REG_NULL_FRAME_INFO1_NULL1_MAC_ADDR_INDEX                         = MAC_REG_NULL_FRAME_INFO1,
          MAC_REG_NULL_FRAME_INFO1_NULL1_MAC_ADDR_INDEX_SHIFT                   = 8,
          MAC_REG_NULL_FRAME_INFO1_NULL1_MAC_ADDR_INDEX_MASK                    = 0x00000300,
          MAC_REG_NULL_FRAME_INFO1_NULL1_MCS32                                  = MAC_REG_NULL_FRAME_INFO1,
          MAC_REG_NULL_FRAME_INFO1_NULL1_MCS32_SHIFT                            = 7,
          MAC_REG_NULL_FRAME_INFO1_NULL1_MCS32_MASK                             = 0x00000080,
          MAC_REG_NULL_FRAME_INFO1_NULL1_MCS                                    = MAC_REG_NULL_FRAME_INFO1,
          MAC_REG_NULL_FRAME_INFO1_NULL1_MCS_SHIFT                              = 4,
          MAC_REG_NULL_FRAME_INFO1_NULL1_MCS_MASK                               = 0x00000070,
          MAC_REG_NULL_FRAME_INFO1_NULL1_GI_TYPE                                = MAC_REG_NULL_FRAME_INFO1,
          MAC_REG_NULL_FRAME_INFO1_NULL1_GI_TYPE_SHIFT                          = 3,
          MAC_REG_NULL_FRAME_INFO1_NULL1_GI_TYPE_MASK                           = 0x00000008,
          MAC_REG_NULL_FRAME_INFO1_NULL1_FORMAT                                 = MAC_REG_NULL_FRAME_INFO1,
          MAC_REG_NULL_FRAME_INFO1_NULL1_FORMAT_SHIFT                           = 0,
          MAC_REG_NULL_FRAME_INFO1_NULL1_FORMAT_MASK                            = 0x00000007,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_COMMAND_0                                                        = 0x009001a8,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_TX_COMMAND_0_BUFFER_WORD_OFFSET                               = MAC_REG_TX_COMMAND_0,
          MAC_REG_TX_COMMAND_0_BUFFER_WORD_OFFSET_SHIFT                         = 16,
          MAC_REG_TX_COMMAND_0_BUFFER_WORD_OFFSET_MASK                          = 0xffff0000,
          MAC_REG_TX_COMMAND_0_BANDWIDTH                                        = MAC_REG_TX_COMMAND_0,
          MAC_REG_TX_COMMAND_0_BANDWIDTH_SHIFT                                  = 14,
          MAC_REG_TX_COMMAND_0_BANDWIDTH_MASK                                   = 0x0000c000,
          MAC_REG_TX_COMMAND_0_AIFSN                                            = MAC_REG_TX_COMMAND_0,
          MAC_REG_TX_COMMAND_0_AIFSN_SHIFT                                      = 10,
          MAC_REG_TX_COMMAND_0_AIFSN_MASK                                       = 0x00003c00,
          MAC_REG_TX_COMMAND_0_CW_COUNT                                         = MAC_REG_TX_COMMAND_0,
          MAC_REG_TX_COMMAND_0_CW_COUNT_SHIFT                                   = 0,
          MAC_REG_TX_COMMAND_0_CW_COUNT_MASK                                    = 0x000003ff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_COMMAND_1                                                        = 0x009001ac,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_COMMAND_2                                                        = 0x009001b0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_COMMAND_3                                                        = 0x009001b4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_COMMAND_4                                                        = 0x009001b8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_COMMAND_5                                                        = 0x009001bc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_COMMAND_6                                                        = 0x009001c0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_COMMAND_7                                                        = 0x009001c4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_COMMAND_8                                                        = 0x009001c8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_COMMAND_9                                                        = 0x009001cc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_COMMAND_10                                                       = 0x009001d0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXOP_COMMAND_0                                                      = 0x009001d4,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_TXOP_COMMAND_0_BUFFER_WORD_OFFSET                             = MAC_REG_TXOP_COMMAND_0,
          MAC_REG_TXOP_COMMAND_0_BUFFER_WORD_OFFSET_SHIFT                       = 16,
          MAC_REG_TXOP_COMMAND_0_BUFFER_WORD_OFFSET_MASK                        = 0xffff0000,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXOP_COMMAND_1                                                      = 0x009001d8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXOP_COMMAND_2                                                      = 0x009001dc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXOP_COMMAND_3                                                      = 0x009001e0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXOP_COMMAND_4                                                      = 0x009001e4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXOP_COMMAND_5                                                      = 0x009001e8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXOP_COMMAND_6                                                      = 0x009001ec,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXOP_COMMAND_7                                                      = 0x009001f0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXOP_COMMAND_8                                                      = 0x009001f4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXOP_COMMAND_9                                                      = 0x009001f8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TXOP_COMMAND_10                                                     = 0x009001fc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_BASE_ADDRESS                                                     = 0x00900200,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_DATA_IN_WAIT                                                     = 0x00900204,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_TX_DATA_IN_WAIT_SECURITY_OFFLINE                              = MAC_REG_TX_DATA_IN_WAIT,
          MAC_REG_TX_DATA_IN_WAIT_SECURITY_OFFLINE_SHIFT                        = 31,
          MAC_REG_TX_DATA_IN_WAIT_SECURITY_OFFLINE_MASK                         = 0x80000000,
          MAC_REG_TX_DATA_IN_WAIT_WAPI_DATA_IN_WAIT                             = MAC_REG_TX_DATA_IN_WAIT,
          MAC_REG_TX_DATA_IN_WAIT_WAPI_DATA_IN_WAIT_SHIFT                       = 16,
          MAC_REG_TX_DATA_IN_WAIT_WAPI_DATA_IN_WAIT_MASK                        = 0x00ff0000,
          MAC_REG_TX_DATA_IN_WAIT_TIKIP_DATA_IN_WAIT                            = MAC_REG_TX_DATA_IN_WAIT,
          MAC_REG_TX_DATA_IN_WAIT_TIKIP_DATA_IN_WAIT_SHIFT                      = 8,
          MAC_REG_TX_DATA_IN_WAIT_TIKIP_DATA_IN_WAIT_MASK                       = 0x0000ff00,
          MAC_REG_TX_DATA_IN_WAIT_CCMP_DATA_IN_WAIT                             = MAC_REG_TX_DATA_IN_WAIT,
          MAC_REG_TX_DATA_IN_WAIT_CCMP_DATA_IN_WAIT_SHIFT                       = 0,
          MAC_REG_TX_DATA_IN_WAIT_CCMP_DATA_IN_WAIT_MASK                        = 0x000000ff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_PHY_TXFIFO_MAX_DEPTH                                                = 0x00900208,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_COMMAND                                                 = 0x0090020c,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RESPONSE_TX_COMMAND_RESPONSE_TYPE                             = MAC_REG_RESPONSE_TX_COMMAND,
          MAC_REG_RESPONSE_TX_COMMAND_RESPONSE_TYPE_SHIFT                       = 5,
          MAC_REG_RESPONSE_TX_COMMAND_RESPONSE_TYPE_MASK                        = 0x00000020,
          MAC_REG_RESPONSE_TX_COMMAND_COMMAND                                   = MAC_REG_RESPONSE_TX_COMMAND,
          MAC_REG_RESPONSE_TX_COMMAND_COMMAND_SHIFT                             = 0,
          MAC_REG_RESPONSE_TX_COMMAND_COMMAND_MASK                              = 0x0000001f,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_VECTOR0                                                 = 0x00900210,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_VECTOR1                                                 = 0x00900214,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_VECTOR2                                                 = 0x00900218,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_INFO0                                                   = 0x0090021c,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_TIMESTAMP_POSITION               = MAC_REG_RESPONSE_TX_INFO0,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_TIMESTAMP_POSITION_SHIFT         = 23,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_TIMESTAMP_POSITION_MASK          = 0x1f800000,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_TIMESTAMP_UPDATE                 = MAC_REG_RESPONSE_TX_INFO0,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_TIMESTAMP_UPDATE_SHIFT           = 22,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_TIMESTAMP_UPDATE_MASK            = 0x00400000,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_CIPHER_TYPE                      = MAC_REG_RESPONSE_TX_INFO0,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_CIPHER_TYPE_SHIFT                = 15,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_CIPHER_TYPE_MASK                 = 0x00038000,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_SINGLE_AMPDU                     = MAC_REG_RESPONSE_TX_INFO0,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_SINGLE_AMPDU_SHIFT               = 14,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_SINGLE_AMPDU_MASK                = 0x00004000,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_MPDU_LENGTH                      = MAC_REG_RESPONSE_TX_INFO0,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_MPDU_LENGTH_SHIFT                = 0,
          MAC_REG_RESPONSE_TX_INFO0_RSP_TXINFO_MPDU_LENGTH_MASK                 = 0x00003fff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_INFO1                                                   = 0x00900220,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RESPONSE_TX_INFO1_RSP_TXINFO_FORMAT                           = MAC_REG_RESPONSE_TX_INFO1,
          MAC_REG_RESPONSE_TX_INFO1_RSP_TXINFO_FORMAT_SHIFT                     = 21,
          MAC_REG_RESPONSE_TX_INFO1_RSP_TXINFO_FORMAT_MASK                      = 0x00e00000,
          MAC_REG_RESPONSE_TX_INFO1_RSP_TXINFO_AGGREGATION                      = MAC_REG_RESPONSE_TX_INFO1,
          MAC_REG_RESPONSE_TX_INFO1_RSP_TXINFO_AGGREGATION_SHIFT                = 20,
          MAC_REG_RESPONSE_TX_INFO1_RSP_TXINFO_AGGREGATION_MASK                 = 0x00100000,
          MAC_REG_RESPONSE_TX_INFO1_RSP_TXINFO_PSDU_LENGTH                      = MAC_REG_RESPONSE_TX_INFO1,
          MAC_REG_RESPONSE_TX_INFO1_RSP_TXINFO_PSDU_LENGTH_SHIFT                = 0,
          MAC_REG_RESPONSE_TX_INFO1_RSP_TXINFO_PSDU_LENGTH_MASK                 = 0x000fffff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_DATA0                                                   = 0x00900224,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_DATA1                                                   = 0x00900228,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_DATA2                                                   = 0x0090022c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_DATA3                                                   = 0x00900230,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_DATA4                                                   = 0x00900234,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_DATA5                                                   = 0x00900238,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_DATA6                                                   = 0x0090023c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_DATA7                                                   = 0x00900240,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_DATA8                                                   = 0x00900244,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_DATA9                                                   = 0x00900248,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_DATA10                                                  = 0x0090024c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_BASE_ADDRESS                                                     = 0x00900250,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_DSC_NUM                                                          = 0x00900254,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC0                                                     = 0x00900258,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RX_REG_DL_DESC0_BUFFER_LENGTH                                 = MAC_REG_RX_REG_DL_DESC0,
          MAC_REG_RX_REG_DL_DESC0_BUFFER_LENGTH_SHIFT                           = 16,
          MAC_REG_RX_REG_DL_DESC0_BUFFER_LENGTH_MASK                            = 0x07ff0000,
          MAC_REG_RX_REG_DL_DESC0_DATA_ADDRESS_OFFSET                           = MAC_REG_RX_REG_DL_DESC0,
          MAC_REG_RX_REG_DL_DESC0_DATA_ADDRESS_OFFSET_SHIFT                     = 0,
          MAC_REG_RX_REG_DL_DESC0_DATA_ADDRESS_OFFSET_MASK                      = 0x0000ffff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC1                                                     = 0x0090025c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC2                                                     = 0x00900260,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC3                                                     = 0x00900264,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC4                                                     = 0x00900268,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC5                                                     = 0x0090026c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC6                                                     = 0x00900270,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC7                                                     = 0x00900274,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC8                                                     = 0x00900278,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC9                                                     = 0x0090027c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC10                                                    = 0x00900280,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC11                                                    = 0x00900284,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC12                                                    = 0x00900288,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC13                                                    = 0x0090028c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC14                                                    = 0x00900290,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC15                                                    = 0x00900294,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC16                                                    = 0x00900298,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC17                                                    = 0x0090029c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC18                                                    = 0x009002a0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC19                                                    = 0x009002a4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC20                                                    = 0x009002a8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC21                                                    = 0x009002ac,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC22                                                    = 0x009002b0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC23                                                    = 0x009002b4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC24                                                    = 0x009002b8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC25                                                    = 0x009002bc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC26                                                    = 0x009002c0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC27                                                    = 0x009002c4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC28                                                    = 0x009002c8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC29                                                    = 0x009002cc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC30                                                    = 0x009002d0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_REG_DL_DESC31                                                    = 0x009002d4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_SET_OWNER_ADDR                                                   = 0x009002d8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_DATA_IN_WAIT                                                     = 0x009002dc,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RX_DATA_IN_WAIT_SECURITY_OFFLINE                              = MAC_REG_RX_DATA_IN_WAIT,
          MAC_REG_RX_DATA_IN_WAIT_SECURITY_OFFLINE_SHIFT                        = 31,
          MAC_REG_RX_DATA_IN_WAIT_SECURITY_OFFLINE_MASK                         = 0x80000000,
          MAC_REG_RX_DATA_IN_WAIT_WAPI_DATA_IN_WAIT                             = MAC_REG_RX_DATA_IN_WAIT,
          MAC_REG_RX_DATA_IN_WAIT_WAPI_DATA_IN_WAIT_SHIFT                       = 16,
          MAC_REG_RX_DATA_IN_WAIT_WAPI_DATA_IN_WAIT_MASK                        = 0x00ff0000,
          MAC_REG_RX_DATA_IN_WAIT_TIKIP_DATA_IN_WAIT                            = MAC_REG_RX_DATA_IN_WAIT,
          MAC_REG_RX_DATA_IN_WAIT_TIKIP_DATA_IN_WAIT_SHIFT                      = 8,
          MAC_REG_RX_DATA_IN_WAIT_TIKIP_DATA_IN_WAIT_MASK                       = 0x0000ff00,
          MAC_REG_RX_DATA_IN_WAIT_CCMP_DATA_IN_WAIT                             = MAC_REG_RX_DATA_IN_WAIT,
          MAC_REG_RX_DATA_IN_WAIT_CCMP_DATA_IN_WAIT_SHIFT                       = 0,
          MAC_REG_RX_DATA_IN_WAIT_CCMP_DATA_IN_WAIT_MASK                        = 0x000000ff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_DMA_SEG_THRESHOLD                                                = 0x009002e0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_CRC_ERR_CRITERIA                                                    = 0x009002e4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_KEY_CMD                                                         = 0x009002e8,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_TYPE                                  = MAC_REG_SEC_KEY_CMD,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_TYPE_SHIFT                            = 6,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_TYPE_MASK                             = 0x000003c0,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_CIPHER_TYPE                           = MAC_REG_SEC_KEY_CMD,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_CIPHER_TYPE_SHIFT                     = 3,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_CIPHER_TYPE_MASK                      = 0x00000038,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_KEY_ID                                = MAC_REG_SEC_KEY_CMD,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_KEY_ID_SHIFT                          = 1,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_KEY_ID_MASK                           = 0x00000006,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_KEY_TYPE                              = MAC_REG_SEC_KEY_CMD,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_KEY_TYPE_SHIFT                        = 0,
          MAC_REG_SEC_KEY_CMD_SEC_KEY_CMD_KEY_TYPE_MASK                         = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_KEY_CMD_ENABLE                                                  = 0x009002ec,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_KEY_VALUE0                                                      = 0x009002f0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_KEY_VALUE1                                                      = 0x009002f4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_KEY_VALUE2                                                      = 0x009002f8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_KEY_VALUE3                                                      = 0x009002fc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_TX_MIC_KEY_VALUE0                                               = 0x00900300,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_TX_MIC_KEY_VALUE1                                               = 0x00900304,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_RX_MIC_KEY_VALUE0                                               = 0x00900308,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_RX_MIC_KEY_VALUE1                                               = 0x0090030c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_STA_ADDRESS_0                                                   = 0x00900310,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_STA_ADDRESS_1                                                   = 0x00900314,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_SPP_ENABLE                                                      = 0x00900318,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_KEY_LOC_ENABLE                                                  = 0x0090031c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_READ_BASE_ADDRESS                                               = 0x00900320,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_WRITE_BASE_ADDRESS                                              = 0x00900324,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_INFO_0                                                          = 0x00900328,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_SEC_INFO_0_MPDU_LENGTH                                        = MAC_REG_SEC_INFO_0,
          MAC_REG_SEC_INFO_0_MPDU_LENGTH_SHIFT                                  = 4,
          MAC_REG_SEC_INFO_0_MPDU_LENGTH_MASK                                   = 0x0003fff0,
          MAC_REG_SEC_INFO_0_CIPHER_TYPE                                        = MAC_REG_SEC_INFO_0,
          MAC_REG_SEC_INFO_0_CIPHER_TYPE_SHIFT                                  = 1,
          MAC_REG_SEC_INFO_0_CIPHER_TYPE_MASK                                   = 0x0000000e,
          MAC_REG_SEC_INFO_0_ENCRYPTION                                         = MAC_REG_SEC_INFO_0,
          MAC_REG_SEC_INFO_0_ENCRYPTION_SHIFT                                   = 0,
          MAC_REG_SEC_INFO_0_ENCRYPTION_MASK                                    = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_INFO_1                                                          = 0x0090032c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_INFO_2                                                          = 0x00900330,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_INFO_3                                                          = 0x00900334,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_INFO_4                                                          = 0x00900338,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_INFO_5                                                          = 0x0090033c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_INFO_6                                                          = 0x00900340,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_INFO_7                                                          = 0x00900344,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_INFO_8                                                          = 0x00900348,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_OFFSET                                                          = 0x0090034c,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_SEC_OFFSET_READ_OFFSET_DECRYPT                                = MAC_REG_SEC_OFFSET,
          MAC_REG_SEC_OFFSET_READ_OFFSET_DECRYPT_SHIFT                          = 10,
          MAC_REG_SEC_OFFSET_READ_OFFSET_DECRYPT_MASK                           = 0x00007c00,
          MAC_REG_SEC_OFFSET_READ_OFFSET_ENCRYPT_1                              = MAC_REG_SEC_OFFSET,
          MAC_REG_SEC_OFFSET_READ_OFFSET_ENCRYPT_1_SHIFT                        = 5,
          MAC_REG_SEC_OFFSET_READ_OFFSET_ENCRYPT_1_MASK                         = 0x000003e0,
          MAC_REG_SEC_OFFSET_READ_OFFSET_ENCRYPT_0                              = MAC_REG_SEC_OFFSET,
          MAC_REG_SEC_OFFSET_READ_OFFSET_ENCRYPT_0_SHIFT                        = 0,
          MAC_REG_SEC_OFFSET_READ_OFFSET_ENCRYPT_0_MASK                         = 0x0000001f,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TKIP_MIC_CAL_OFF                                                    = 0x00900350,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_INDIR_REG_ADDR                                                   = 0x00900354,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_INDIR_REG_ADDR                                                   = 0x00900358,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_INDIR_REG_ADDR                                                  = 0x0090035c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_DMA_INDIR_REG_ADDR                                                  = 0x00900360,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_IRQ_INDIR_REG_ADDR                                                  = 0x00900364,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TSF_0_LOWER                                                         = 0x00900368,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TSF_0_UPPER                                                         = 0x0090036c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TSF_0_ALARM_LOWER                                                   = 0x00900370,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_BCN_INTERVAL_0                                                      = 0x00900374,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TBTT_INTERRUPT_MARGIN_0                                             = 0x00900378,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TBTT_RESTART_RANGE_0                                                = 0x0090037c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TSF_1_LOWER                                                         = 0x00900380,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TSF_1_UPPER                                                         = 0x00900384,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TSF_1_ALARM_LOWER                                                   = 0x00900388,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_BCN_INTERVAL_1                                                      = 0x0090038c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TBTT_INTERRUPT_MARGIN_1                                             = 0x00900390,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TBTT_RESTART_RANGE_1                                                = 0x00900394,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_IRQ_MASK0                                                           = 0x00900398,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_IRQ_MASK1                                                           = 0x0090039c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_RESULT_0                                                         = 0x009003a0,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_TX_RESULT_0_WINAC                                             = MAC_REG_TX_RESULT_0,
          MAC_REG_TX_RESULT_0_WINAC_SHIFT                                       = 28,
          MAC_REG_TX_RESULT_0_WINAC_MASK                                        = 0xf0000000,
          MAC_REG_TX_RESULT_0_INTERNAL_COLLISION_BITMAP                         = MAC_REG_TX_RESULT_0,
          MAC_REG_TX_RESULT_0_INTERNAL_COLLISION_BITMAP_SHIFT                   = 16,
          MAC_REG_TX_RESULT_0_INTERNAL_COLLISION_BITMAP_MASK                    = 0x003f0000,
          MAC_REG_TX_RESULT_0_ACK_POLICY                                        = MAC_REG_TX_RESULT_0,
          MAC_REG_TX_RESULT_0_ACK_POLICY_SHIFT                                  = 6,
          MAC_REG_TX_RESULT_0_ACK_POLICY_MASK                                   = 0x00000040,
          MAC_REG_TX_RESULT_0_FAIL_REASON                                       = MAC_REG_TX_RESULT_0,
          MAC_REG_TX_RESULT_0_FAIL_REASON_SHIFT                                 = 3,
          MAC_REG_TX_RESULT_0_FAIL_REASON_MASK                                  = 0x00000038,
          MAC_REG_TX_RESULT_0_TXOP_END                                          = MAC_REG_TX_RESULT_0,
          MAC_REG_TX_RESULT_0_TXOP_END_SHIFT                                    = 2,
          MAC_REG_TX_RESULT_0_TXOP_END_MASK                                     = 0x00000004,
          MAC_REG_TX_RESULT_0_FAILED_BITMAP_VALID                               = MAC_REG_TX_RESULT_0,
          MAC_REG_TX_RESULT_0_FAILED_BITMAP_VALID_SHIFT                         = 1,
          MAC_REG_TX_RESULT_0_FAILED_BITMAP_VALID_MASK                          = 0x00000002,
          MAC_REG_TX_RESULT_0_ACK_SUCCESS                                       = MAC_REG_TX_RESULT_0,
          MAC_REG_TX_RESULT_0_ACK_SUCCESS_SHIFT                                 = 0,
          MAC_REG_TX_RESULT_0_ACK_SUCCESS_MASK                                  = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_RESULT_1                                                         = 0x009003a4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_RESULT_2                                                         = 0x009003a8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_RESULT_3                                                         = 0x009003ac,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_RESULT_4                                                         = 0x009003b0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_RESULT_5                                                         = 0x009003b4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_RESULT_6                                                         = 0x009003b8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_RESULT_7                                                         = 0x009003bc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_RESULT_8                                                         = 0x009003c0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_RESULT_9                                                         = 0x009003c4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_RESULT_10                                                        = 0x009003c8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_LOWER_0                                            = 0x009003cc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_UPPER_0                                            = 0x009003d0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_LOWER_1                                            = 0x009003d4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_UPPER_1                                            = 0x009003d8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_LOWER_2                                            = 0x009003dc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_UPPER_2                                            = 0x009003e0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_LOWER_3                                            = 0x009003e4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_UPPER_3                                            = 0x009003e8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_LOWER_4                                            = 0x009003ec,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_UPPER_4                                            = 0x009003f0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_LOWER_5                                            = 0x009003f4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_UPPER_5                                            = 0x009003f8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_LOWER_6                                            = 0x009003fc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_UPPER_6                                            = 0x00900400,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_LOWER_7                                            = 0x00900404,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_UPPER_7                                            = 0x00900408,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_LOWER_8                                            = 0x0090040c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_UPPER_8                                            = 0x00900410,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_LOWER_9                                            = 0x00900414,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_FAILED_BITMAP_UPPER_9                                            = 0x00900418,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RESPONSE_TX_RESULT                                                  = 0x0090041c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_CONCURRENT_INFO                                                     = 0x00900420,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_CONCURRENT_INFO_LAST_COMMAND                                  = MAC_REG_CONCURRENT_INFO,
          MAC_REG_CONCURRENT_INFO_LAST_COMMAND_SHIFT                            = 8,
          MAC_REG_CONCURRENT_INFO_LAST_COMMAND_MASK                             = 0x00000f00,
          MAC_REG_CONCURRENT_INFO_STATE                                         = MAC_REG_CONCURRENT_INFO,
          MAC_REG_CONCURRENT_INFO_STATE_SHIFT                                   = 4,
          MAC_REG_CONCURRENT_INFO_STATE_MASK                                    = 0x000000f0,
          MAC_REG_CONCURRENT_INFO_RESULT                                        = MAC_REG_CONCURRENT_INFO,
          MAC_REG_CONCURRENT_INFO_RESULT_SHIFT                                  = 0,
          MAC_REG_CONCURRENT_INFO_RESULT_MASK                                   = 0x0000000f,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_CONCURRENT_TIMER                                                    = 0x00900424,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC0                                                = 0x00900428,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_RX_WRITE_EN_DL_DESC0_OWNERSHIP                                = MAC_REG_RX_WRITE_EN_DL_DESC0,
          MAC_REG_RX_WRITE_EN_DL_DESC0_OWNERSHIP_SHIFT                          = 2,
          MAC_REG_RX_WRITE_EN_DL_DESC0_OWNERSHIP_MASK                           = 0x00000004,
          MAC_REG_RX_WRITE_EN_DL_DESC0_FRAGMENT                                 = MAC_REG_RX_WRITE_EN_DL_DESC0,
          MAC_REG_RX_WRITE_EN_DL_DESC0_FRAGMENT_SHIFT                           = 0,
          MAC_REG_RX_WRITE_EN_DL_DESC0_FRAGMENT_MASK                            = 0x00000003,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC1                                                = 0x0090042c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC2                                                = 0x00900430,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC3                                                = 0x00900434,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC4                                                = 0x00900438,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC5                                                = 0x0090043c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC6                                                = 0x00900440,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC7                                                = 0x00900444,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC8                                                = 0x00900448,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC9                                                = 0x0090044c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC10                                               = 0x00900450,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC11                                               = 0x00900454,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC12                                               = 0x00900458,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC13                                               = 0x0090045c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC14                                               = 0x00900460,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC15                                               = 0x00900464,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC16                                               = 0x00900468,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC17                                               = 0x0090046c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC18                                               = 0x00900470,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC19                                               = 0x00900474,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC20                                               = 0x00900478,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC21                                               = 0x0090047c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC22                                               = 0x00900480,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC23                                               = 0x00900484,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC24                                               = 0x00900488,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC25                                               = 0x0090048c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC26                                               = 0x00900490,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC27                                               = 0x00900494,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC28                                               = 0x00900498,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC29                                               = 0x0090049c,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC30                                               = 0x009004a0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_WRITE_EN_DL_DESC31                                               = 0x009004a4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_KEY_CMD_RESULT_EN                                               = 0x009004a8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_KEY_CMD_RESULT                                                  = 0x009004ac,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_SEC_KEY_CMD_RESULT_COMMAND                                    = MAC_REG_SEC_KEY_CMD_RESULT,
          MAC_REG_SEC_KEY_CMD_RESULT_COMMAND_SHIFT                              = 28,
          MAC_REG_SEC_KEY_CMD_RESULT_COMMAND_MASK                               = 0xf0000000,
          MAC_REG_SEC_KEY_CMD_RESULT_RESULT                                     = MAC_REG_SEC_KEY_CMD_RESULT,
          MAC_REG_SEC_KEY_CMD_RESULT_RESULT_SHIFT                               = 20,
          MAC_REG_SEC_KEY_CMD_RESULT_RESULT_MASK                                = 0x0ff00000,
          MAC_REG_SEC_KEY_CMD_RESULT_ADDRESS                                    = MAC_REG_SEC_KEY_CMD_RESULT,
          MAC_REG_SEC_KEY_CMD_RESULT_ADDRESS_SHIFT                              = 0,
          MAC_REG_SEC_KEY_CMD_RESULT_ADDRESS_MASK                               = 0x00000007,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_WRITE_DESC_RESULT                                               = 0x009004b0,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_SEC_WRITE_DESC_RESULT_FRAGMENT                                = MAC_REG_SEC_WRITE_DESC_RESULT,
          MAC_REG_SEC_WRITE_DESC_RESULT_FRAGMENT_SHIFT                          = 23,
          MAC_REG_SEC_WRITE_DESC_RESULT_FRAGMENT_MASK                           = 0x7f800000,
          MAC_REG_SEC_WRITE_DESC_RESULT_ICV_FAIL                                = MAC_REG_SEC_WRITE_DESC_RESULT,
          MAC_REG_SEC_WRITE_DESC_RESULT_ICV_FAIL_SHIFT                          = 1,
          MAC_REG_SEC_WRITE_DESC_RESULT_ICV_FAIL_MASK                           = 0x00000002,
          MAC_REG_SEC_WRITE_DESC_RESULT_MIC_FAIL                                = MAC_REG_SEC_WRITE_DESC_RESULT,
          MAC_REG_SEC_WRITE_DESC_RESULT_MIC_FAIL_SHIFT                          = 0,
          MAC_REG_SEC_WRITE_DESC_RESULT_MIC_FAIL_MASK                           = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TX_INDIR_REG_DATA                                                   = 0x009004b4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_RX_INDIR_REG_DATA                                                   = 0x009004b8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_SEC_INDIR_REG_DATA                                                  = 0x009004bc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_DMA_INDIR_REG_DATA                                                  = 0x009004c0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_IRQ_INDIR_REG_DATA                                                  = 0x009004c4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TSF_0_LOWER_READONLY                                                = 0x009004c8,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TSF_0_UPPER_READONLY                                                = 0x009004cc,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TSF_1_LOWER_READONLY                                                = 0x009004d0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_TSF_1_UPPER_READONLY                                                = 0x009004d4,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_IRQ_SRC0                                                            = 0x009004d8,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_IRQ_SRC0_CONCURRENT_SWITCH                                    = MAC_REG_IRQ_SRC0,
          MAC_REG_IRQ_SRC0_CONCURRENT_SWITCH_SHIFT                              = 31,
          MAC_REG_IRQ_SRC0_CONCURRENT_SWITCH_MASK                               = 0x80000000,
          MAC_REG_IRQ_SRC0_RX_TIM                                               = MAC_REG_IRQ_SRC0,
          MAC_REG_IRQ_SRC0_RX_TIM_SHIFT                                         = 27,
          MAC_REG_IRQ_SRC0_RX_TIM_MASK                                          = 0x08000000,
          MAC_REG_IRQ_SRC0_RX_DONE                                              = MAC_REG_IRQ_SRC0,
          MAC_REG_IRQ_SRC0_RX_DONE_SHIFT                                        = 26,
          MAC_REG_IRQ_SRC0_RX_DONE_MASK                                         = 0x04000000,
          MAC_REG_IRQ_SRC0_TX_DONE_BITMAP                                       = MAC_REG_IRQ_SRC0,
          MAC_REG_IRQ_SRC0_TX_DONE_BITMAP_SHIFT                                 = 13,
          MAC_REG_IRQ_SRC0_TX_DONE_BITMAP_MASK                                  = 0x00ffe000,
          MAC_REG_IRQ_SRC0_WIN_AC_BITMAP                                        = MAC_REG_IRQ_SRC0,
          MAC_REG_IRQ_SRC0_WIN_AC_BITMAP_SHIFT                                  = 0,
          MAC_REG_IRQ_SRC0_WIN_AC_BITMAP_MASK                                   = 0x000007ff,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_IRQ_SRC1                                                            = 0x009004dc,
//-------------------------------------------------------------------------------------------------------------------------------,
          MAC_REG_IRQ_SRC1_TSF1_ALARM                                           = MAC_REG_IRQ_SRC1,
          MAC_REG_IRQ_SRC1_TSF1_ALARM_SHIFT                                     = 7,
          MAC_REG_IRQ_SRC1_TSF1_ALARM_MASK                                      = 0x00000080,
          MAC_REG_IRQ_SRC1_TBTT_IRQ_TSF1                                        = MAC_REG_IRQ_SRC1,
          MAC_REG_IRQ_SRC1_TBTT_IRQ_TSF1_SHIFT                                  = 6,
          MAC_REG_IRQ_SRC1_TBTT_IRQ_TSF1_MASK                                   = 0x00000040,
          MAC_REG_IRQ_SRC1_TSF0_ALARM                                           = MAC_REG_IRQ_SRC1,
          MAC_REG_IRQ_SRC1_TSF0_ALARM_SHIFT                                     = 5,
          MAC_REG_IRQ_SRC1_TSF0_ALARM_MASK                                      = 0x00000020,
          MAC_REG_IRQ_SRC1_TBTT_IRQ_TSF0                                        = MAC_REG_IRQ_SRC1,
          MAC_REG_IRQ_SRC1_TBTT_IRQ_TSF0_SHIFT                                  = 4,
          MAC_REG_IRQ_SRC1_TBTT_IRQ_TSF0_MASK                                   = 0x00000010,
          MAC_REG_IRQ_SRC1_RX_BUFFER_LOOKUP                                     = MAC_REG_IRQ_SRC1,
          MAC_REG_IRQ_SRC1_RX_BUFFER_LOOKUP_SHIFT                               = 3,
          MAC_REG_IRQ_SRC1_RX_BUFFER_LOOKUP_MASK                                = 0x00000008,
          MAC_REG_IRQ_SRC1_SW_RSP_REQ_IRQ                                       = MAC_REG_IRQ_SRC1,
          MAC_REG_IRQ_SRC1_SW_RSP_REQ_IRQ_SHIFT                                 = 2,
          MAC_REG_IRQ_SRC1_SW_RSP_REQ_IRQ_MASK                                  = 0x00000004,
          MAC_REG_IRQ_SRC1_AHB_SEC_IRQ                                          = MAC_REG_IRQ_SRC1,
          MAC_REG_IRQ_SRC1_AHB_SEC_IRQ_SHIFT                                    = 1,
          MAC_REG_IRQ_SRC1_AHB_SEC_IRQ_MASK                                     = 0x00000002,
          MAC_REG_IRQ_SRC1_DMA_ERROR                                            = MAC_REG_IRQ_SRC1,
          MAC_REG_IRQ_SRC1_DMA_ERROR_SHIFT                                      = 0,
          MAC_REG_IRQ_SRC1_DMA_ERROR_MASK                                       = 0x00000001,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_IRQ_SRC0_NOCLEAR                                                    = 0x009004e0,
//-------------------------------------------------------------------------------------------------------------------------------,
//-------------------------------------------------------------------------------------------------------------------------------,
    MAC_REG_IRQ_SRC1_NOCLEAR                                                    = 0x009004e4,
//-------------------------------------------------------------------------------------------------------------------------------
};
static inline int mac_register_ver_check(void)
{ return *(unsigned int volatile *)(0x0090fffc) == VER; }
#endif

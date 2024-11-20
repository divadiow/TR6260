ifeq ($(type), tr6260)
WIFI_PRODUCT := tr6260
else ifeq ($(type), tr6260s1)
WIFI_PRODUCT := tr6260
else ifeq ($(type), tr6260_3)
WIFI_PRODUCT := tr6260
else
WIFI_PRODUCT := $(type)
endif

#// WIFI/HAL
VPATH   += $(WIFI_PATH)/hal/$(WIFI_PRODUCT)
INCLUDE += -I$(WIFI_PATH)/hal/$(WIFI_PRODUCT)
WIFI_CSRCS   += hal_phy_$(WIFI_PRODUCT).c hal_rf_$(WIFI_PRODUCT).c

#// WIFI/MAC
VPATH   += $(WIFI_PATH)/mac/lmac
INCLUDE += -I$(WIFI_PATH)/mac/lmac
WIFI_CSRCS   += hal_lmac_common.c hal_lmac_util.c hal_lmac_ps_common.c hal_lmac_ps_psnonpoll.c hal_test_tr6260.c
WIFI_CSRCS   += system_memory_manager.c \
                hal_lmac_11n.c protocol.c hal_lmac_debug.c hal_lmac_downlink.c \
                util_modem.c util_cmd_lmac.c drv_lmac.c hal_lmac_test.c hal_lmac_queue_manager.c util_cmd_lmac_indirect.c

VPATH   += $(WIFI_PATH)/mac/umac
INCLUDE += -I$(WIFI_PATH)/mac/umac
WIFI_CSRCS   += umac_wim_dispatcher.c umac_wim_manager.c umac_wim_builder.c
WIFI_CSRCS   += umac_scan.c umac_beacon.c umac_probe_resp.c umac_info.c
WIFI_CSRCS   += umac_bcn_monitor.c
DEFINE  += -DUMAC_BCN_MONITOR -DINCLUDE_UMAC_BEACON -DINCLUDE_UMAC_PRESP

#// WIFI/DRIVERS
VPATH   += $(WIFI_PATH)/drivers
INCLUDE += -I$(WIFI_PATH)/drivers
WIFI_CSRCS   += driver_wifi.c driver_wifi_scan.c driver_wifi_tx.c driver_wifi_rx.c driver_wifi_debug.c


WIFI_CSRCS   += system_modem_api.c


WIFI_OBJS    := $(patsubst %.S,$(OBJ_PATH)/%.S.o,$(WIFI_ASRCS))
WIFI_OBJS    += $(patsubst %.c,$(OBJ_PATH)/%.c.o,$(WIFI_CSRCS))
WIFI_OBJS    += $(patsubst %.cpp,$(OBJ_PATH)/%.o,$(WIFI_SRCS))

$(WIFI_LIBS): $(WIFI_OBJS)
	@rm -f $@
	@$(AR) -crs $@ $(WIFI_OBJS) 
#	@$(ST) --strip-unneeded $@

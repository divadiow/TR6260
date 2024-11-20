ifeq ($(type), tr6260)
WIFI_PRODUCT := tr6260
else ifeq ($(type), tr6260s1)
WIFI_PRODUCT := tr6260
else ifeq ($(type), tr6260_3)
WIFI_PRODUCT := tr6260
else
WIFI_PRODUCT := $(type)
endif

VPATH	+= $(WIFI_PATH)/hal/tr6260
INCLUDE	+= -I$(WIFI_PATH)/hal/tr6260
WIFILMAC_CSRCS	:= hal_phy_tr6260.c hal_rf_tr6260.c hal_test_tr6260.c

#// WIFI/MAC
VPATH	+= $(WIFI_PATH)/mac/lmac
VPATH	+= $(WIFI_PATH)/
INCLUDE	+= -I$(WIFI_PATH)/mac/lmac -I$(WIFI_PATH)/mac/umac
WIFILMAC_CSRCS	+= hal_lmac_common.c hal_lmac_util.c util_modem.c hal_lmac_ps_common.c hal_lmac_ps_dummy.c \
	               hal_lmac_debug.c hal_lmac_test.c hal_lmac_queue_manager.c hal_lmac_downlink.c \
                   system_memory_manager.c drv_lmac.c hal_lmac_11n.c protocol.c 
WIFILMAC_CSRCS	+= util_cmd_lmac.c util_cmd_lmac_indirect.c

WIFILMAC_OBJS    := $(patsubst %.c,$(OBJ_PATH)/%.c.o,$(WIFILMAC_CSRCS))


$(WIFILMAC_LIBS): $(WIFILMAC_OBJS)
	@rm -f $@
	@$(AR) -crs $@ $(WIFILMAC_OBJS) 
	@$(ST) --strip-unneeded $@

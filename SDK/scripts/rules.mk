#//===============================================================================
#//
#//	file name: rules.mk
#//
#//===============================================================================
CROSS_COMPILE	:=	nds32le-elf-
#//===============================================================================
#// Common Tool
#//===============================================================================
CC	:= $(CROSS_COMPILE)g++
OP	:= $(CROSS_COMPILE)objcopy
AR	:= $(CROSS_COMPILE)ar
AS	:= $(CROSS_COMPILE)g++
LD	:= $(CROSS_COMPILE)g++
NM	:= $(CROSS_COMPILE)nm
OD	:= $(CROSS_COMPILE)objdump
RD	:= $(CROSS_COMPILE)readelf
ST	:= $(CROSS_COMPILE)strip


#// --------------------------------------------------
#//	Build
#// --------------------------------------------------
OBJS	:= $(patsubst %.S,$(OBJ_PATH)/%.S.o,$(ASRCS))
OBJS	+= $(patsubst %.c,$(OBJ_PATH)/%.c.o,$(CSRCS))
OBJS	+= $(patsubst %.cpp,$(OBJ_PATH)/%.o,$(SRCS))

DEPS	:= $(patsubst %.S,$(OBJ_PATH)/%.S.d,$(ASRCS))
DEPS	+= $(patsubst %.c,$(OBJ_PATH)/%.c.d,$(CSRCS))
DEPS	+= $(patsubst %.cpp,$(OBJ_PATH)/%.d,$(SRCS))

#// --------------------------------------------------
#// Bulid Flag
#// --------------------------------------------------
ARCH_FLAGS	+= -mcmodel=large 
SYS_FLAGS	+= --specs=nosys.specs

CFLAGS 	+= -x none -Wall -Wno-unused-value -Wno-unused-variable -Wno-unused-but-set-variable -Wno-misleading-indentation -Wno-memset-elt-size -Wno-maybe-uninitialized -Wno-unused-function -Wno-format-overflow -Wno-format-truncation -Wno-bool-compare
CFLAGS	+= -fno-omit-frame-pointer -fno-common -fno-exceptions -gdwarf-2 -ffunction-sections -fdata-sections
CFLAGS	+= -Os -mno-ex9 -mext-zol -mext-dsp -DCONFIG_HWZOL -D__TARGET_IFC_EXT -D__TARGET_ZOL_EXT
CFLAGS	+= $(INCLUDE) $(ARCH_FLAGS) $(DEFINE)

#CFLAGS	+=-DHEAP_MEMORY_TRACE 
#CFLAGS	+=-DCPU_RUNTIME_TRACE
ifeq ($(type), mpw)
LFLAGS	+=  -nostartfiles -Wl,--gc-sections -mext-zol -mext-dsp -lm -ldsp
else
LFLAGS	+=  -nostartfiles -nostdlib -Wl,--gc-sections -mext-zol -mext-dsp -lm -ldsp
#LFLAGS	+=  -nostartfiles -Wl,--gc-sections

LFLAGS	+= \
			$(TOPDIR)/lib/lib_a-w_log10.o \
			$(TOPDIR)/lib/lib_a-rand.o \
			$(TOPDIR)/lib/lib_a-data-rand.o \
			$(TOPDIR)/lib/lib_a-reent_errno.o \
			$(TOPDIR)/lib/lib_a-andes_dpexlog.o \
			$(TOPDIR)/lib/lib_a-ctype_.o \

LFLAGS	+= \
			$(TOPDIR)/lib/_div_sf.o \
			$(TOPDIR)/lib/_sf_to_df.o \
			$(TOPDIR)/lib/_umoddi3.o \
			$(TOPDIR)/lib/_si_to_sf.o \
			$(TOPDIR)/lib/lib_a-s_ceil.o \
			$(TOPDIR)/lib/lib_a-s_round.o \
			$(TOPDIR)/lib/lib_a-errno.o \
			$(TOPDIR)/lib/lib_a-strerror.o \
			$(TOPDIR)/lib/lib_a-impure.o \
			$(TOPDIR)/lib/lib_a-u_strerr.o \
			$(TOPDIR)/lib/lib_a-w_pow.o \
			$(TOPDIR)/lib/lib_a-andes_dpexexp.o \
			$(TOPDIR)/lib/_mul_sf.o \
			$(TOPDIR)/lib/_addsub_sf.o \
			$(TOPDIR)/lib/_df_to_sf.o \
			$(TOPDIR)/lib/lib_a-s_scalbn.o \
			$(TOPDIR)/lib/lib_a-sf_scalbn.o \
			$(TOPDIR)/lib/lib_a-s_copysign.o \
			$(TOPDIR)/lib/lib_a-abs.o \
			$(TOPDIR)/lib/lib_a-w_sqrt.o \
			$(TOPDIR)/lib/lib_a-andes_dpexsqrt.o \


endif

LFLAGS	+= $(ARCH_FLAGS) $(SYS_FLAGS)
LFLAGS	+= -L$(LIB_PATH) -Os -Wl,-Map=$(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).map
LFLAGS	+= -gdwarf-2
ifeq (,$(ota))
LFLAGS	+= -T$(LD_FILE) -mno-ex9
else
LFLAGS	+= -mno-ex9
endif

CPFLAGS	= -O binary
ODFLAGS = -S -C --demangle
ASFLAGS	=
NMFLAGS	= -S --demangle --size-sort -s -r

default: all
.SUFFIXES : .o .cpp .c .S
-include $(DEPS)
#.PHONY : all firmware
#.DEFAULT_GOAL := firmware

all: firmware

ifeq (,$(ota))

firmware: $(OUT_PATH) $(OBJ_PATH) $(OBJS) $(WIFI_LIBS) $(PSM_LIBS) $(WIFILMAC_LIBS) $(ALIYUN_SDK_LIBS) $(SECURE_LIBS) 
#	@echo "Link Script : $(LD_FILE)"
	@echo "Linking $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).elf"
	@$(CC) $(LFLAGS) -Wl,--whole-archive $(WIFI_LIBS) $(PSM_LIBS) $(WIFILMAC_LIBS) $(ALIYUN_SDK_LIBS) $(SECURE_LIBS) $(TUYA_SDK_LIBS) -Wl,--no-whole-archive $(OBJS) -o $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).elf
	@echo "generating $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).bin"
	@$(OP) -O binary $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).elf -R .xip $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).bin
	@echo "generating $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_xip.bin"
	@$(OP) $(CPFLAGS) $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).elf -j .xip $(OUT_PATH)/$(type)/$(TARGET)/xip.bin
	@echo "generating $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).lst"
	@$(OD) $(ODFLAGS) $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).elf > $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).lst
	@echo "generating $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).nm"
	@$(NM) $(NMFLAGS) $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).elf > $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).nm
	@$(RD) -e -g -t $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT).elf > $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_readelf.txt

else

firmware: $(OUT_PATH) $(OBJ_PATH) $(OBJS) $(WIFI_LIBS) $(PSM_LIBS) $(WIFILMAC_LIBS) $(ALIYUN_SDK_LIBS) $(SECURE_LIBS) $(TUYA_SDK_LIBS)
	@echo "Linking OTA $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_1.elf"
	@$(CC) $(LFLAGS) -T$(LD_FILE)  -Wl,--whole-archive $(WIFI_LIBS) $(PSM_LIBS) $(WIFILMAC_LIBS) $(ALIYUN_SDK_LIBS) $(SECURE_LIBS) $(TUYA_SDK_LIBS) -Wl,--no-whole-archive $(OBJS) -o $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_1.elf
	@echo "generating $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_1.bin"
	@$(OP) -O binary $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_1.elf -R .xip $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_1.bin
	@echo "generating $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_xip_1.bin"
	@$(OP) $(CPFLAGS) $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_1.elf -j .xip $(OUT_PATH)/$(type)/$(TARGET)/xip_1.bin
	@echo "Linking OTA $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_2.elf"
	@$(CC) $(LFLAGS) -T$(LD_FILE_OTA)  -Wl,--whole-archive $(WIFI_LIBS) $(PSM_LIBS) $(WIFILMAC_LIBS) -Wl,--no-whole-archive  $(OBJS) -o $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_2.elf
	@echo "generating $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_2.bin"
	@$(OP) -O binary $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_2.elf -R .xip $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_2.bin
	@echo "generating $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_xip_2.bin"
	@$(OP) $(CPFLAGS) $(OUT_PATH)/$(type)/$(TARGET)/$(OUTPUT)_2.elf -j .xip $(OUT_PATH)/$(type)/$(TARGET)/xip_2.bin
	
endif  # ifeq (,$(ota))
	

$(OBJ_PATH)/%.o:%.cpp | $(OBJ_PATH)
	@echo "compile $<"
	@$(CC) -E -MMD $(CFLAGS) $(CPPFLAGS) -c $< -o $@.i
	@$(CC) -MMD $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJ_PATH)/%.c.o:%.c $(TOPDIR)/scripts/rules.mk | $(OBJ_PATH)
	@echo "compile $<"
	@$(CC) -MMD $(CFLAGS) -c $< -o $@

$(OBJ_PATH)/%.S.o:%.S $(TOPDIR)/scripts/rules.mk | $(OBJ_PATH)
	@echo "compile $<"
	@$(AS) -MMD $(CFLAGS) $(ASFLAGS) -c $< -o $@

$(OUT_PATH):
	@mkdir -p $@/$(type)/$(TARGET)

$(OBJ_PATH): $(OUT_PATH)
	@mkdir -p $@

clean:
	@rm -rf $(OUT_PATH)/$(type)/$(TARGET)
	@echo clean all
#// --------------------------------------------------

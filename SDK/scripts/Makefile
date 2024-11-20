#===============================================================================
#
#	file name: tr6260 makeflie top
#
#===============================================================================
TIMER_START = $(shell date)

#===============================================================================
#   path
#===============================================================================

export TOPDIR=..
export DRV_PATH=$(TOPDIR)/drivers
export PLATFORM_PATH=$(TOPDIR)/platform
export WIFI_PATH=$(TOPDIR)/wifi
export REF_PATH=$(TOPDIR)/ref_design
export SYSTEM_PATH=$(TOPDIR)/system
export APP_PATH= $(TOPDIR)/apps
export INC_PATH= $(TOPDIR)/include
export SCRIPT_PATH= $(TOPDIR)/scripts
export OUT_PATH= $(TOPDIR)/out


default: help

help:
	@echo "================== Make help ==================="
	@echo "make [help]               help"
	@echo "make minfpga              compile fpga min-system"
	@echo "make minfpgaclean         clean fpga min-system"
	@echo "make minmpw               compile mpw min-system"
	@echo "make minmpwclean          clean mpw min-system"


::--------------------------------- min system----------------------------------------*/
mintr6260  mt:
	@make --no-print-directory -f Makefile.minSys type=tr6260
	@echo "$(TIMER_START)"
	@date

mintr6260clean  mtc:
	@make --no-print-directory -f Makefile.minSys type=tr6260 clean
	@echo "$(TIMER_START)"
	@date

mintr6260s1  mt_tr6260s1:
	@make --no-print-directory -f Makefile.minSys type=tr6260s1
	@echo "$(TIMER_START)"
	@date

mintr6260s1clean  mtc_tr6260s1:
	@make --no-print-directory -f Makefile.minSys type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date

mintr6260_3  mt_tr6260_3:
	@make --no-print-directory -f Makefile.minSys type=tr6260_3
	@echo "$(TIMER_START)"
	@date

mintr6260_3clean  mtc_tr6260_3:
	@make --no-print-directory -f Makefile.minSys type=tr6260_3 clean
	@echo "$(TIMER_START)"
	@date

minfpga  mf:
	@make --no-print-directory -f Makefile.minSys type=fpga
	@echo "$(TIMER_START)"
	@date

minfpgaclean  mfc:
	@make --no-print-directory -f Makefile.minSys type=fpga clean
	@echo "$(TIMER_START)"
	@date

minmpw mm:
	@make --no-print-directory -f Makefile.minSys type=mpw all
	@echo "$(TIMER_START)"
	@date

minmpwclean mmc:
	@make --no-print-directory -f Makefile.minSys type=mpw clean
	@echo "$(TIMER_START)"
	@date

::--------------------------------- test version ----------------------------------------*/
lmacfpga  lf:
	@make --no-print-directory -f Makefile.lmacTest type=fpga
	@echo "$(TIMER_START)"
	@date

lmacfpgaclean  lfc:
	@make --no-print-directory -f Makefile.lmacTest type=fpga clean
	@echo "$(TIMER_START)"
	@date

lmacmpw lm:
	@make --no-print-directory -f Makefile.lmacTest type=mpw all
	@echo "$(TIMER_START)"
	@date

lmacmpwclean lmc:
	@make --no-print-directory -f Makefile.lmacTest type=mpw clean
	@echo "$(TIMER_START)"
	@date
	
lmactr6260 lt:
	@make --no-print-directory -f Makefile.lmacTest type=tr6260 all
	@echo "$(TIMER_START)"
	@date

lmactr6260clean ltc:
	@make --no-print-directory -f Makefile.lmacTest type=tr6260 clean
	@echo "$(TIMER_START)"
	@date

sigtr6260 sig:
	@make --no-print-directory -f Makefile.sig type=tr6260 all
	@echo "$(TIMER_START)"
	@date

sigtr6260clean sigc:
	@make --no-print-directory -f Makefile.sig type=tr6260 clean
	@echo "$(TIMER_START)"
	@date

sigdtr6260 sigd:
	@make --no-print-directory -f Makefile.sigd type=tr6260 all
	@echo "$(TIMER_START)"
	@date

sigdtr6260clean sigdc:
	@make --no-print-directory -f Makefile.sigd type=tr6260 clean
	@echo "$(TIMER_START)"
	@date

amttr6260 amt:
	@make --no-print-directory -f Makefile.amt type=tr6260 all
	@echo "$(TIMER_START)"
	@date

amttr6260clean amtc:
	@make --no-print-directory -f Makefile.amt type=tr6260 clean
	@echo "$(TIMER_START)"
	@date

amttr6260_3 amt_3:
	@make --no-print-directory -f Makefile.amt type=tr6260_3 all
	@echo "$(TIMER_START)"
	@date

amttr6260clean_3 amtc_3:
	@make --no-print-directory -f Makefile.amt type=tr6260_3 clean
	@echo "$(TIMER_START)"
	@date
	
amttr6260s1 amt_s1:
	@make --no-print-directory -f Makefile.amt type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

amttr6260cleans1 amtc_s1:
	@make --no-print-directory -f Makefile.amt type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
	
lmactr6260s1 lt_tr6260s1:
	@make --no-print-directory -f Makefile.lmacTest type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

lmactr6260s1clean ltc_tr6260s1:
	@make --no-print-directory -f Makefile.lmacTest type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date

lmactr6260_3 lt_tr6260_3:
	@make --no-print-directory -f Makefile.lmacTest type=tr6260_3 all
	@echo "$(TIMER_START)"
	@date

lmactr6260_3clean ltc_tr6260_3:
	@make --no-print-directory -f Makefile.lmacTest type=tr6260_3 clean
	@echo "$(TIMER_START)"
	@date

ATETest at:
	@make --no-print-directory -f Makefile.ATETest type=tr6260 all
#	@make --no-print-directory -f Makefile.ATETest type=fpga all
	@echo "$(TIMER_START)"
	@date

ATETestclean atc:
	@make --no-print-directory -f Makefile.ATETest type=tr6260 clean
#	@make --no-print-directory -f Makefile.ATETest type=fpga clean
	@echo "$(TIMER_START)"
	@date
::--------------------------------- transmission ----------------------------------------*/
transmissiontr6260  tt:
	@make --no-print-directory -f Makefile.transmission type=tr6260
	@echo "$(TIMER_START)"
	@date

transmissiontr6260clean  ttc:
	@make --no-print-directory -f Makefile.transmission type=tr6260 clean
	@echo "$(TIMER_START)"
	@date

transmissionfpga  tf:
	@make --no-print-directory -f Makefile.transmission type=fpga
	@echo "$(TIMER_START)"
	@date

transmissionfpgaclean  tfc:
	@make --no-print-directory -f Makefile.transmission type=fpga clean
	@echo "$(TIMER_START)"
	@date

transmissionmpw tm:
	@make --no-print-directory -f Makefile.transmission type=mpw all
	@echo "$(TIMER_START)"
	@date

transissionmpwclean tmc:
	@make --no-print-directory -f Makefile.transmission type=mpw clean
	@echo "$(TIMER_START)"
	@date

::--------------------------------- standalone ----------------------------------------*/
standalonetr6260  st:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone type=tr6260 all
	@echo "$(TIMER_START)"
	@date

standalonetr6260clean  stc:
	@make --no-print-directory -rR -f Makefile.standalone type=tr6260 clean
	@echo "$(TIMER_START)"
	@date
::--------------------------------- standalone fpga----------------------------------------*/
standalonetr6260f  stf:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone type=fpga all
	@echo "$(TIMER_START)"
	@date

standalonetr6260fclean  stfc:
	@make --no-print-directory -rR -f Makefile.standalone type=fpga clean
	@echo "$(TIMER_START)"
	@date

::----------add by wangxia 20190318 for TR6260S1 Begin------------------*/
standalonetr6260s1  st_tr6260s1:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

standalonetr6260s1clean  stc_tr6260s1clean:
	@make --no-print-directory -rR -f Makefile.standalone type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
::-----add by wangxia 20190318 for TR6260S1 End-----------------------*/

::----------add by liangyu 20190814 for TR6260S1test Begin------------------*/
testtr6260s1  test_tr6260s1:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_test type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

testtr6260s1clean  test_tr6260s1clean:
	@make --no-print-directory -rR -f Makefile.standalone_test type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
::-----add by liangyu 20190814 for TR6260S1test End-----------------------*/

::----------add by wangxia 20190326 for TR6260_3 Begin------------------*/
standalonetr6260_3  st_tr6260_3:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone type=tr6260_3 all
	@echo "$(TIMER_START)"
	@date

standalonetr6260_3clean  stc_tr6260_3clean:
	@make --no-print-directory -rR -f Makefile.standalone type=tr6260_3 clean
	@echo "$(TIMER_START)"
	@date
::-----add by wangxia 20190318 for TR6260_3 End-----------------------*/

::----------add by jiayt 20200414 for TR6260_diff Begin------------------*/
standalonetr6260_3_diff  st_tr6260_3_diff:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_diff type=tr6260_3 all
	@echo "$(TIMER_START)"
	@date

standalonetr6260_3_diff_clean  stc_tr6260_3_diff_clean:
	@make --no-print-directory -rR -f Makefile.standalone_diff type=tr6260_3 clean
	@echo "$(TIMER_START)"
	@date
standalonetr6260_diff  st_tr6260_diff:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_diff type=tr6260 all
	@echo "$(TIMER_START)"
	@date

standalonetr6260_diffclean  stc_tr6260_diffclean:
	@make --no-print-directory -rR -f Makefile.standalone_diff type=tr6260 clean
	@echo "$(TIMER_START)"
	@date
standalonetr6260s1_diff  st_tr6260s1_diff:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_diff type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

standalonetr6260s1_diffclean  stc_tr6260s1_diffclean:
	@make --no-print-directory -rR -f Makefile.standalone_diff type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
::-----add by jiayt 20200414 for TR6260_diff End-----------------------*/

standalonefpga  sf:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone type=fpga
	@echo "$(TIMER_START)"
	@date

standalonefpgaclean  sfc:
	@make --no-print-directory -rR -f Makefile.standalone type=fpga clean
	@echo "$(TIMER_START)"
	@date

standalonempw sm:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone type=mpw all
	@echo "$(TIMER_START)"
	@date

standalonempwclean smc:
	@make --no-print-directory -rR -f Makefile.standalone type=mpw clean
	@echo "$(TIMER_START)"
	@date

dna:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_dna type=tr6260 all
	@echo "$(TIMER_START)"
	@date

dnaclean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_dna type=tr6260 clean
	@echo "$(TIMER_START)"
	@date

::----------add by wangxia 20190520 for DNA TR6260S1/TR6260_3 Begin------------------*/
dnatr6260s1:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_dna type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

dnatr6260s1clean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_dna type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date

dnatr6260s1_diff:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_dna_diff type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

dnatr6260s1_diffclean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_dna_diff type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date

dnatr6260_3:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_dna type=tr6260_3 all
	@echo "$(TIMER_START)"
	@date

dnatr6260_3clean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_dna type=tr6260_3 clean
	@echo "$(TIMER_START)"
	@date
::-----add by wangxia 20190520 for DNA TR6260S1/TR6260_3 End-----------------------*/

kkstr6260s1:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_kks type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

kkstr6260s1clean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_kks type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date

tbtr6260s1:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tb type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

tbtr6260s1clean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tb type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
	
tbtr6260s1_diff:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tb_diff type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

tbtr6260s1_diffclean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tb_diff type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date

tuyaled:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyaled type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

tuyaledclean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyaled type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
	
tuyaled_single_nv:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyaled_single_nv type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

tuyaled_single_nvclean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyaled_single_nv type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
	
tuyaled_double_nv:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyaled_double_nv type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

tuyaled_double_nvclean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyaled_double_nv type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
	

tuyaswitch:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyaswitch type=tr6260 all
	@echo "$(TIMER_START)"
	@date

tuyaswitchclean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyaswitch type=tr6260 clean
	@echo "$(TIMER_START)"
	@date

tuyaplug:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyaplug type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

tuyaplugclean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyaplug type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date

tuyabreaker:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyabreaker type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

tuyabreakerclean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_tuyabreaker type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date

::-------------add by caosipeng 20191219 for Hisense TR6260 Begin------------------*/
hisense:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_hisense type=tr6260 all
	@echo "$(TIMER_START)"
	@date

hisensec:
	@make --no-print-directory -rR -f Makefile.standalone_hisense type=tr6260 clean
	@echo "$(TIMER_START)"
	@date
::-------------add by caosipeng 20191219 for Hisense TR6260 Begin------------------*/


::-----------add by caosipeng 20200109 for Hisense TR6260s1 Begin------------------*/
hisenses1:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_hisense type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

hisenses1clean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_hisense type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
::-----------add by caosipeng 20200109 for Hisense TR6260s1 Begin------------------*/


::-----------add by caosipeng 20191219 for Hisense TR6260_3 Begin------------------*/
hisense_3:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_hisense type=tr6260_3 all
	@echo "$(TIMER_START)"
	@date

hisense_3clean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_hisense type=tr6260_3 clean
	@echo "$(TIMER_START)"
	@date
::-----------add by caosipeng 20191219 for Hisense TR6260_3 Begin------------------*/


::---------------add by caosipeng 20200109 for AT TR6260 Begin---------------------*/
sta_at:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_at type=tr6260 all
	@echo "$(TIMER_START)"
	@date

atclean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_at type=tr6260 clean
	@echo "$(TIMER_START)"
	@date
::---------------add by caosipeng 20200109 for AT TR6260 Begin---------------------*/


::---------------add by caosipeng 20200115 for AT TR6260s1 Begin-------------------*/
sta_ats1:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_at type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

ats1clean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_at type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
::--------------add by caosipeng 20200115 for AT TR6260s1 Begin-------------------*/

::-------------add by weifeng 20200218 for aliyun TR6260 Begin------------------*/
aliyun:
	@make  -rR -j 4 -f Makefile.standalone_aliyun type=tr6260 all
	@echo "$(TIMER_START)"
	@date

aliyunc:
	@make  -rR -f Makefile.standalone_aliyun type=tr6260 clean
	@echo "$(TIMER_START)"
	@date
::-------------add by weifeng 20200218 for aliyun TR6260 End-----------------------*/

::-------------add by wangxia 20200326 for aliyun TR6260S1 Begin------------------*/
aliyun_s1:
	@make  -rR -j 4 -f Makefile.standalone_aliyun type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

aliyunc_s1:
	@make  -rR -f Makefile.standalone_aliyun type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
::-------------add by wangxia 20200326 for aliyun TR6260S1 End------------------*/

::-------------add by jiayt 20200416 for diff Begin------------------*/
sta_ats1_diff:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_at_diff type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

ats1clean_diff:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_at_diff type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
aliyun_diff:
	@make  -rR -j 4 -f Makefile.standalone_aliyun_diff type=tr6260 all
	@echo "$(TIMER_START)"
	@date

aliyunc_diff:
	@make  -rR -f Makefile.standalone_aliyun_diff type=tr6260 clean
	@echo "$(TIMER_START)"
	@date
aliyun_s1_diff:
	@make  -rR -j 4 -f Makefile.standalone_aliyun_diff type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

aliyunc_s1_diff:
	@make  -rR -f Makefile.standalone_aliyun_diff type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date

aliyun_s1_plug_diff:
	@make  -rR -j 4 -f Makefile.standalone_aliyun_plug_diff type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

aliyunc_s1_plug_diff:
	@make  -rR -f Makefile.standalone_aliyun_plug_diff type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date

lmactr6260_diff lt_diff:
	@make --no-print-directory -f Makefile.lmacTest_diff type=tr6260 all
	@echo "$(TIMER_START)"
	@date

lmactr6260diffclean ltc_diff:
	@make --no-print-directory -f Makefile.lmacTest_diff type=tr6260 clean
	@echo "$(TIMER_START)"
	@date
::-------------add by jiayt 20200416 for diff End-----------------------*/


::-------------add by weifeng 20200722 for Hisense_Lock TR6260 Begin------------------*/
hislock:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_hislock type=tr6260 all
	@echo "$(TIMER_START)"
	@date

hislockclean:
	@make --no-print-directory -rR -f Makefile.standalone_hislock type=tr6260 clean
	@echo "$(TIMER_START)"
	@date
::-------------add by weifeng 20200722 for Hisense_Lock TR6260 End------------------*/


::-----------add by weifeng 20200722 for Hisense_Lock TR6260s1 Begin------------------*/
hislocks1:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_hislock type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

hislocks1clean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_hislock type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
	
hislocks1_diff:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_hislock_diff type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date

hislocks1_diffclean:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_hislock_diff type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
::-----------add by weifeng 20200722 for Hisense_Lock TR6260s1 End------------------*/

st_tr6260s1_wpa3_diff:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_wpa3_diff type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date
stc_tr6260s1_wpa3_diffclean:
	@make --no-print-directory -rR -f Makefile.standalone_wpa3_diff type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
	
st_tr6260s1_eziot_led_diff:
	@make --no-print-directory -rR -j 4 -f Makefile.standalone_eziot_led_diff type=tr6260s1 all
	@echo "$(TIMER_START)"
	@date
stc_tr6260s1_eziot_led_diffclean:
	@make --no-print-directory -rR -f Makefile.standalone_eziot_led_diff type=tr6260s1 clean
	@echo "$(TIMER_START)"
	@date
::--------------------------------- ALL ----------------------------------------*/
allf: mfc lfc tfc sfc mf lf tf sf
allm: mmc lmc tmc smc mm lm tm sm





/*****************************************************************************
 * PIT - BASE
 ****************************************************************************/
#define PIT_BASE0		(0x00604000) /* Device base address */
#define PIT_BASE1		(0x00605000) /* Device base address */


#define PIT_NUM_0	PIT_BASE0
#define PIT_NUM_1	PIT_BASE1

#define PIT_CHN_0	0
#define PIT_CHN_1	1
#define PIT_CHN_2	2
#define PIT_CHN_3	3

int pit_ch_count_get(unsigned int pit_base, unsigned int num);

int pit_ch_reload_value(unsigned int pit_base, unsigned int num, unsigned int value);

#define PIT_CHCLK_EXTERN	0
#define PIT_CHCLK_APB		1

#define PIT_CHMODE_RES1		0
#define PIT_CHMODE_32BIT_TM	1
#define PIT_CHMODE_16BIT_TM	2
#define PIT_CHMODE_8BIT_TM		3
#define PIT_CHMODE_PWM		4
#define PIT_CHMODE_RES2		5
#define PIT_CHMODE_MIXED_PWM_16BIT_TM	6
#define PIT_CHMODE_MIXED_PWM_8BIT_TM	7
int pit_ch_ctrl(unsigned int pit_base, unsigned int num, unsigned int PWMPark, unsigned int chClk, unsigned int chMode);


#define PIT_CH_TM0_EN			0x1
#define PIT_CH_TM1_EN			0x2
#define PIT_CH_TM2_EN			0x4
#define PIT_CH_TM3_PWM_EN		0x8
#define PIT_CH_DISABLE			0x0
int pit_ch_mode_set(unsigned int pit_base, unsigned int num, unsigned int enable);
//int pit_ch_mode_enable(unsigned int pit_base, unsigned int num, unsigned int enable);

#define PIT_INT_STATUS_CLEAN		1
#define PIT_INT_STATUS_GET			0
int pit_int_status_handle(unsigned int pit_base, unsigned int num, unsigned int action);

int pit_int_enable(unsigned int pit_base, unsigned int num, unsigned int enable);

void pit_init(void);

void pit_delay(unsigned long delay);

//#define mdelay(x)	pit_delay((x)*100)


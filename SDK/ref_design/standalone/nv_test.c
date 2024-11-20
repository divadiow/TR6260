#include "system_wifi.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
/****************************************************************************
* 	                                           Local Macros
****************************************************************************/
#define IS_ASCII(c) (c > 0x1F && c < 0x7F)
/****************************************************************************
* 	                                           Local Types
****************************************************************************/

/****************************************************************************
* 	                                           Local Constants
****************************************************************************/

/****************************************************************************
* 	                                           Local Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Global Constants
****************************************************************************/

/****************************************************************************
* 	                                          Global Variables
****************************************************************************/

/****************************************************************************
* 	                                          Global Function Prototypes
****************************************************************************/

/****************************************************************************
* 	                                          Function Definitions
****************************************************************************/

#include "easyflash.h"
// size_t ef_get_env_blob(const char *key, void *value_buf, size_t buf_len, size_t *value_len);
// EfErrCode ef_set_env_blob(const char *key, const void *value_buf, size_t buf_len);

typedef struct test_nv_param_t {
	int a;
	char b;
	long c;
	char * a_p;
} test_nv_param;

static int nv_test_func(cmd_tbl_t *t, int argc, char *argv[])
{
	uint8_t i;
	easyflash_init();
	test_nv_param test_value;
	test_value.a = 1;
	test_value.b = 0;
	test_value.c = 0x55;
	test_value.a_p = NULL;
	uint8_t mode=2;
	char* key=NULL;

	for(i=1; i<argc; i+=2)
	{
		if(strcmp(argv[i], "mode")==0)
		{
			mode = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "key")==0)
		{
			key = argv[i+1];
		}
	}
	if(mode == 0 && key != NULL){
		ef_set_env_blob(key, &test_value, sizeof(test_nv_param));
		int i = 500;
		while(i--){
			test_value.a++;
			ef_set_env_blob(key, &test_value, sizeof(test_nv_param));
		}
	} else if(mode == 1 && key != NULL){
		test_nv_param test_value_2 = {0};
		size_t len = 0;
		ef_get_env_blob(key, &test_value_2, sizeof(test_nv_param),&len);
		system_printf("a %d b %d c %d a_p %p\n",test_value_2.a,test_value_2.b,test_value_2.c,test_value_2.a_p);

	} else {
			return CMD_RET_FAILURE;
	}
	

	return CMD_RET_SUCCESS;
}
SUBCMD(test, nv_test, nv_test_func, "", "");
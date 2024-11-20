// Copyright 2018-2019 trans-semi inc
// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <strings.h>
#include "system.h"
#include "mbedtls/platform.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "trs_debug.h"
#include "default_config.h"
#ifdef CONFIG_MBEDTLS_DEBUG

static void mbedtls_trs_debug(void *ctx, int level,
                              const char *file, int line,
                              const char *str);

void mbedtls_trs_enable_debug_log(mbedtls_ssl_config *conf, int threshold)
{
    mbedtls_debug_set_threshold(threshold);
    mbedtls_ssl_conf_dbg(conf, mbedtls_trs_debug, NULL);
}

void mbedtls_trs_disable_debug_log(mbedtls_ssl_config *conf)
{
    mbedtls_ssl_conf_dbg(conf, NULL, NULL);
}

/* Default mbedtls debug function that translates mbedTLS debug output
   to trs_LOGx debug output.
*/
static void mbedtls_trs_debug(void *ctx, int level,
                     const char *file, int line,
                     const char *str)
{
    system_printf("mbedtls: %s:%d %s\n", file, line, str);
}

#endif
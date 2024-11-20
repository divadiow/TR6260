/*******************************************************************************
 * Copyright by Transa Semi.
 *
 * File Name:     system_log.h
 * File Mark:    
 * Description:  
 * Others:        
 * Version:       V1.0
 * Author:        lixiao
 * Date:          2018-12-21
 * History 1:      
 *     Date: 
 *     Version:
 *     Author: 
 *     Modification:  
 * History 2: 
  ********************************************************************************/

#ifndef _SYSTEM_LOG_H
#define _SYSTEM_LOG_H


/****************************************************************************
* 	                                        Include files
****************************************************************************/


/****************************************************************************
* 	                                        Macros
****************************************************************************/
#define LOG_LOCAL_LEVEL SYS_LOG_VERBOSE

#define SYS_LOG_EARLY_IMPL(format, log_level, ...) do {                         \
        if (LOG_LOCAL_LEVEL >= log_level) {                                     \
            system_printf(format"\n", ##__VA_ARGS__);                           \
        }} while(0)


#define SYS_EARLY_LOGE(format, ...) SYS_LOG_EARLY_IMPL(format, SYS_LOG_ERROR,   ##__VA_ARGS__)
#define SYS_EARLY_LOGW(format, ...) SYS_LOG_EARLY_IMPL(format, SYS_LOG_WARN,    ##__VA_ARGS__)
#define SYS_EARLY_LOGI(format, ...) SYS_LOG_EARLY_IMPL(format, SYS_LOG_INFO,    ##__VA_ARGS__)
#define SYS_EARLY_LOGD(format, ...) SYS_LOG_EARLY_IMPL(format, SYS_LOG_DEBUG,   ##__VA_ARGS__)
#define SYS_EARLY_LOGV(format, ...) SYS_LOG_EARLY_IMPL(format, SYS_LOG_VERBOSE, ##__VA_ARGS__)


#define SYS_LOGE(format, ...)  SYS_EARLY_LOGE(format, ##__VA_ARGS__)
#define SYS_LOGW(format, ...)  SYS_EARLY_LOGW(format, ##__VA_ARGS__)
#define SYS_LOGI(format, ...)  SYS_EARLY_LOGI(format, ##__VA_ARGS__)
#define SYS_LOGD(format, ...)  SYS_EARLY_LOGD(format, ##__VA_ARGS__)
#define SYS_LOGV(format, ...)  SYS_EARLY_LOGV(format, ##__VA_ARGS__)


/****************************************************************************
* 	                                        Types
****************************************************************************/
typedef enum {
    SYS_LOG_NONE = 0,   /*!< No log output */
    SYS_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    SYS_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    SYS_LOG_INFO,       /*!< Information messages which describe normal flow of events */
    SYS_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    SYS_LOG_VERBOSE,    /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */

    SYS_LOG_MAX
} sys_log_level_t;

/****************************************************************************
* 	                                        Constants
****************************************************************************/

/****************************************************************************
* 	                                        Global  Variables
****************************************************************************/

/****************************************************************************
* 	                                        Function Prototypes
****************************************************************************/



#endif/*_SYSTEM_LOG_H*/


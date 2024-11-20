// Copyright 2018-2019 Transa-Semi Technology Inc. and its subsidiaries.
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

#ifndef _DRV_I2C_H_
#define _DRV_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <system_def.h>
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/semphr.h"
#include "FreeRTOS/queue.h"
#include "FreeRTOS/task.h"
#include "drv_i2c_struct.h"

#define I2C_OK                       0
#define ERROR_I2C_PARAM              1
#define ERROR_I2C_BUSY               2
#define ERROR_I2C_MEM                3
#define ERROR_I2C_TIMEOUT            4
#define ERROR_I2C_FAILED             5

typedef enum{
    I2C_MODE_SLAVE,       /*!< I2C slave mode */
    I2C_MODE_MASTER,      /*!< I2C master mode */
    I2C_MODE_MAX,
}i2c_mode_t;

typedef enum{
    I2C_CLK_50K,          //todo
    I2C_CLK_100K,
    I2C_CLK_400K,
    I2C_CLK_1M,
}i2c_clk_t;

typedef enum{
    I2C_CMD_NOACTION,
    I2C_CMD_ISSUEDATATRANS, 
    I2C_CMD_RESPWITHACK,     
    I2C_CMD_RESPWITHNACK,    
    I2C_CMD_CLEARFIFO,       
    I2C_CMD_RESET,           
}i2c_cmd_t;


typedef enum {
    I2C_ADDR_BIT_7 = 0,    /*!< I2C 7bit address for slave mode */
    I2C_ADDR_BIT_10,       /*!< I2C 10bit address for slave mode */
    I2C_ADDR_BIT_MAX,
} i2c_addr_mode_t;

typedef enum {
    I2C_DMA_DISABLE = 0,  /*!< I2C enable DMA */
    I2C_DMA_ENABLE,       /*!< I2C disable DMA */
} i2c_dma_mode_t;

/**
 * @brief I2C initialization parameters
 */
typedef struct{
    i2c_mode_t mode;       /*!< I2C mode */
    i2c_addr_mode_t addr_mode;  /*!< I2C address mode*/
    i2c_dma_mode_t dma_mode_enbale;      /*!< I2C DMA mode enable*/

    union {
        struct {
            i2c_clk_t clk_speed;     /*!< I2C clock frequency for master mode, (no higher than 1MHz for now) */
        } master;
        struct {
            uint16_t slave_addr;       /*!< I2C address for slave mode */
        } slave;
    };
}i2c_config_t;

typedef struct {
    int type;
} i2c_evt_t;

typedef void (*i2c_callback_t)(unsigned char slave_address, int event, void* arg);

int i2c_driver_init(i2c_config_t *i2c_config);

int i2c_driver_deinit(void);

int i2c_master_write(uint16_t slave_addr, uint8_t* data, size_t data_len, bool dma_mode, TickType_t ticks_to_wait);

int i2c_master_read(uint16_t slave_addr, uint8_t* data, size_t data_len, bool dma_mode, TickType_t ticks_to_wait);

int i2c_slave_write(uint8_t* data, size_t data_len, bool dma_mode, TickType_t ticks_to_wait);

int i2c_slave_read(uint8_t* data, size_t data_len, bool dma_mode, TickType_t ticks_to_wait);

int i2c_reset_fifo(void);

bool i2c_bus_is_busy(void);

int i2c_register_callback(i2c_callback_t i2c_callback_func, void* arg);

#ifdef __cplusplus
}
#endif

#endif /*_DRIVER_I2C_H_*/

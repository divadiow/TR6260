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

#ifndef _I2C_STRUCT_H_
#define _I2C_STRUCT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* these macro defines are I2C register address, will be replaced with a struct define so that 
the registers macro symbols will not be used in source code*/  
#define I2C_BASE_REG          (0x00608000UL)
#define I2C_IDREV_REG         (I2C_BASE_REG + 0x0000)
#define I2C_CFG_REG           (I2C_BASE_REG + 0x0010)
#define I2C_INTR_ENABLE_REG   (I2C_BASE_REG + 0x0014)
#define I2C_STATUS_REG        (I2C_BASE_REG + 0x0018)
#define I2C_ADDR_REG          (I2C_BASE_REG + 0x001C)
#define I2C_DATA_REG          (I2C_BASE_REG + 0x0020)
#define I2C_CTRL_REG          (I2C_BASE_REG + 0x0024)
#define I2C_CMD_REG           (I2C_BASE_REG + 0x0028)
#define I2C_SET_REG           (I2C_BASE_REG + 0x002C)
#define I2C_TMP_REG           (I2C_BASE_REG + 0x0030)

typedef volatile struct {
    union {
        struct {
            uint32_t RevMinor:     4;                          /*RO,*/
            uint32_t RevMajor:     4;                          /*RO,*/
            uint32_t ID:           24;                         /*RO,*/
        };
        uint32_t val;
    } id_verion;
    uint32_t reserved_0;
    uint32_t reserved_1;
    uint32_t reserved_2;
    union {
        struct {
            uint32_t fifo_size:    2;               
            uint32_t reserved30:   30;           
        };
        uint32_t val;
    } fifo_size;
    union {
        struct {
            uint32_t fifo_empty:   1;
            uint32_t fifo_full:    1;
            uint32_t fifo_half:    1;
            uint32_t addr_hit:     1;
            uint32_t arb_lose:     1;
            uint32_t stop:         1;
            uint32_t start:        1;
            uint32_t bytetrans:    1;
            uint32_t byterecv:     1;
            uint32_t cmpl:         1; 
            uint32_t reserved20:   20;
        };
        uint32_t val;
    } intr_reg;
    union {
        struct {
            uint32_t fifo_empty:   1;  
            uint32_t fifo_full:    1;
            uint32_t fifo_half:    1;
            uint32_t addr_hit:     1;
            uint32_t arb_lose:     1;
            uint32_t stop:         1;
            uint32_t start:        1;
            uint32_t bytetrans:    1;
            uint32_t byterecv:     1;
            uint32_t cmpl:         1; 
            uint32_t ack:          1;
            uint32_t bus_busy:     1;
            uint32_t gencall:      1;
            uint32_t linescl:      1;
            uint32_t linesda:      1;
            uint32_t reserved17:   17;
        };
        uint32_t val;
    } intr_bus_status;
    union {
        struct {
            uint32_t addr:         10;               
            uint32_t reserved22:   22;
        };
        uint32_t val;
    } slave_addr;
    union {
        struct {
            uint32_t data:         8;               
            uint32_t reserved24:   24;
        };
        uint32_t val;
    } fifo_data;
    union {
        struct {
            uint32_t datacnt_l4bit:4;   
            uint32_t datacnt_h4bit:4; 
            uint32_t direction:    1; 
            uint32_t phasestop:    1; 
            uint32_t phasedata:    1;
            uint32_t phaseaddr:    1;
            uint32_t phasestart:   1;              
            uint32_t reserved19:   19;
        };
        uint32_t val;
    } i2c_control;
    union {
        struct {
            uint32_t cmd:          3;
            uint32_t reserved29:   29;
        };
        uint32_t val;
    } i2c_cmd;
    union {
        struct {
            uint32_t i2c_en:       1;   
            uint32_t addr_mode:    1;          
            uint32_t work_mode:    1;   
            uint32_t dma_enable:   1; 
            uint32_t t_sclhi:      9;         
            uint32_t t_sclratio:   1;  
            uint32_t reserved_2:   2;        
            uint32_t t_hddat:      5;        
            uint32_t t_sp:         3;    
            uint32_t t_sudat:      5;     
            uint32_t reserved_3:   3;  
        };
        uint32_t val;
    } setup_cfg;
    union {
        struct {
            uint32_t tpm:          5;      
            uint32_t reserved_27:  27;  
        };
        uint32_t val;
    } tpm_cfg;
} i2c_dev_t;

extern i2c_dev_t I2C;

#ifdef __cplusplus
}
#endif

#endif  /* _I2C_STRUCT_H_ */

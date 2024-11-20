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

#include "drv_i2c.h"
#include "bsp/soc.h"
#include "bsp/soc_pin_mux.h"

//#define NAU8810U
#define APB_CLOCK   40000000
#define APB_CLOCK_BASE (APB_CLOCK/10000000)
#define APB_MOD_BASE 100
#define I2C_MAX(a,b) (a)>=(b)?(a):(b)
#define I2C_EVT_QUEUE_LEN              (1)
#define I2C_CMD_VALID_TICK             (100 / portTICK_PERIOD_MS)

/* DMA controller */
#define DMA_BASE         (0x00800000UL)
#define DMA_IDREV        (*(volatile unsigned int *)(DMA_BASE + 0x00))
#define DMA_CONFIG       (*(volatile unsigned int *)(DMA_BASE + 0x10))
#define DMA_CTRL         (*(volatile unsigned int *)(DMA_BASE + 0x20))
#define DMA_INTRST       (*(volatile unsigned int *)(DMA_BASE + 0x30))
#define DMA_CHEN         (*(volatile unsigned int *)(DMA_BASE + 0x34))
#define DMA_CHABORT      (*(volatile unsigned int *)(DMA_BASE + 0x40))
#define DMA_CHCTRL(N)    (*(volatile unsigned int *)(DMA_BASE + 0x44 + (N)*0x14))
#define DMA_CHSRCADDR(N) (*(volatile unsigned int *)(DMA_BASE + 0x48 + (N)*0x14))
#define DMA_CHDSTADDR(N) (*(volatile unsigned int *)(DMA_BASE + 0x4C + (N)*0x14))
#define DMA_CHTRANSZ(N)  (*(volatile unsigned int *)(DMA_BASE + 0x50 + (N)*0x14))
#define DMA_CHLLP(N)     (*(volatile unsigned int *)(DMA_BASE + 0x54 + (N)*0x14))

typedef enum {
    I2C_ADDR_HIT,
    I2C_FIFO_FULL,
    I2C_FIFO_EMPTY,
    I2C_CMPL,
} i2c_event;

//todo: the tx/rx information will replaced with ringbuf in next version

typedef struct {
    uint32_t dma_mode;                   /*!< I2C read and write with DMA or not */
    QueueHandle_t status_evt_queue;  /*!< I2C status event queue */
    xSemaphoreHandle cmd_mux;        /*!< semaphore to lock command process */
    size_t tx_data_remain;           /*!< tx data remain length */
    size_t rx_data_remain;           /*!< rx data remain length */
} i2c_handle_t;

static i2c_dev_t* const I2C_CONF = (i2c_dev_t*)(0x00608000);
static i2c_handle_t *p_i2c_handle = NULL;
static i2c_callback_t g_i2c_callback; 
static void* g_i2c_arg;

void DMA_Write_Config(const unsigned char *src, unsigned int size)
{
    DMA_CTRL = 1;  /* Reset DMA */
    DMA_INTRST = -1;        // clear interrupt status
    DMA_CHSRCADDR(0) = *src;
    DMA_CHDSTADDR(0) = (unsigned int)&(I2C_CONF->fifo_data.val);
    DMA_CHTRANSZ(0) = size;
    /* Enable DMAC channel #0
     * 1 burst transfers, word width transfer, unmask ABT/ERR/TC
     * SRC : normal mode, increment address
     * DST : handshake mode, request select, fixed address
     */
    DMA_CHCTRL(0) = (0 << 22) | (0 << 20) | (0 << 18) | (1 << 16) | (2 << 12) | (4 << 4) | 0xF;
}

void DMA_Read_Config(unsigned char *dst, unsigned int size)
{
    DMA_CTRL = 1;  /* Reset DMA */
    DMA_INTRST = -1;        // clear interrupt status
    DMA_CHSRCADDR(0) = (unsigned int)&(I2C_CONF->fifo_data.val);
    DMA_CHDSTADDR(0) = *dst;
    DMA_CHTRANSZ(0) = size;
    /* Enable DMAC channel #0
     * 1 burst transfers, word width transfer, unmask ABT/ERR/TC
     * SRC : normal mode, increment address
     * DST : handshake mode, request select, fixed address
     */
    DMA_CHCTRL(0) = (0 << 22) | (0 << 20) | (0 << 18) | (1 << 16) | (2 << 12) | (4 << 4) | 0xF;
}

int32_t hal_i2c_isr(int32_t vector)
{
    int i2c_int_status = 0;
    i2c_evt_t eve;
    irq_status_clean(vector);
    portBASE_TYPE high_priority_task_awoken = 0;
#ifdef NAU8810U
    if (I2C_CONF->intr_bus_status.fifo_full) {
        system_printf("enter fifo_full\n");
        I2C_CONF->intr_bus_status.fifo_full = 1;
        eve.type = I2C_FIFO_FULL;
        goto out;
    }

    if (I2C_CONF->intr_bus_status.fifo_empty) {
        system_printf("enter fifo_empty\n");
        eve.type = I2C_FIFO_EMPTY;
        I2C_CONF->intr_reg.fifo_empty = 0;
        goto out;
    }
out:
#else
    if (I2C_CONF->intr_bus_status.addr_hit) {
        eve.type = I2C_ADDR_HIT;
    }
    if (I2C_CONF->intr_bus_status.addr_hit) {
        eve.type = I2C_FIFO_FULL;
    }
    if (I2C_CONF->intr_bus_status.cmpl) {
        eve.type = I2C_FIFO_EMPTY;
    }
    if (I2C_CONF->intr_bus_status.fifo_empty) {
        eve.type = I2C_CMPL;
    }
#endif
    xQueueSendFromISR(p_i2c_handle->status_evt_queue, (void *)&eve, &high_priority_task_awoken);

    if (high_priority_task_awoken == pdTRUE) {
        portYIELD_FROM_ISR(high_priority_task_awoken);
    }
    // if (g_i2c_callback) {
    //     g_i2c_callback(I2C_CONF->slave_addr.addr, I2C_CONF->intr_bus_status.val, g_i2c_arg);
    // }
    return 0;
}

int i2c_driver_init(i2c_config_t *i2c_config)
{
    if ((i2c_config == NULL) || (p_i2c_handle != NULL)) {
        return ERROR_I2C_PARAM;
    }
    uint16_t t_sudat = 0;
    uint16_t t_sp = 2;
    uint16_t t_tpm = 0;
    uint16_t t_hddat = 0;
    uint16_t t_sclratio = 0;
    uint16_t t_sclhi = 0;
    CLK_ENABLE(CLK_I2C);
    CLK_ENABLE(CLK_BUF_MEM);
    CLK_ENABLE(CLK_AHB_DAMC);
    PIN_FUNC_SET(IO_MUX0_GPIO0, FUNC_GPIO0_I2C_SCL);
    PIN_FUNC_SET(IO_MUX0_GPIO1, FUNC_GPIO1_I2C_SDA);

    p_i2c_handle = (i2c_handle_t*)os_malloc(sizeof(i2c_handle_t));
    if (!p_i2c_handle) {
        return ERROR_I2C_MEM;
    }

    I2C_CONF->i2c_cmd.cmd = I2C_CMD_RESET;
    p_i2c_handle->cmd_mux = xSemaphoreCreateMutex();
    p_i2c_handle->status_evt_queue = xQueueCreate(I2C_EVT_QUEUE_LEN, sizeof(i2c_evt_t));
    
    if (i2c_config->mode == I2C_MODE_MASTER) {
        I2C_CONF->setup_cfg.work_mode = I2C_MODE_MASTER;
        I2C_CONF->tpm_cfg.tpm = t_tpm = 0;    // assuming TPM = 0
        I2C_CONF->setup_cfg.addr_mode = i2c_config->addr_mode;
        I2C_CONF->setup_cfg.dma_enable = i2c_config->dma_mode_enbale;
        if (i2c_config->master.clk_speed == I2C_CLK_100K) {
            t_sudat = ((250*APB_CLOCK_BASE)/APB_MOD_BASE) - 4 - t_sp;
            t_hddat = ((300*APB_CLOCK_BASE)/APB_MOD_BASE) - 4 - t_sp;
            t_sclratio = 0;
            uint16_t t_sclhi_a = ((4000*APB_CLOCK_BASE)/APB_MOD_BASE) - 4 - t_sp;
            uint16_t t_sclhi_b = ((4700*APB_CLOCK_BASE)/APB_MOD_BASE) - 4 - t_sp;
            t_sclhi = I2C_MAX(t_sclhi_a,t_sclhi_b);
        } else if (i2c_config->master.clk_speed == I2C_CLK_400K) {
            t_sudat = ((100*APB_CLOCK_BASE)/APB_MOD_BASE) - 4 - t_sp;
            t_hddat = ((300*APB_CLOCK_BASE)/APB_MOD_BASE) - 4 - t_sp;
            t_sclratio = 1;
            uint16_t t_sclhi_a = ((600*APB_CLOCK_BASE)/APB_MOD_BASE) - 4 - t_sp;
            uint16_t t_sclhi_b = (((1300*APB_CLOCK_BASE)/APB_MOD_BASE) - 4 - t_sp)/2;
            t_sclhi = I2C_MAX(t_sclhi_a,t_sclhi_b);
        } else if (i2c_config->master.clk_speed == I2C_CLK_1M) {
            t_sudat = ((50*APB_CLOCK_BASE)/APB_MOD_BASE) - 4 - t_sp;
            t_hddat = 0;
            t_sclratio = 1;
            uint16_t t_sclhi_a = ((260*APB_CLOCK_BASE)/APB_MOD_BASE) - 4 - t_sp;
            uint16_t t_sclhi_b = (((500*APB_CLOCK_BASE)/APB_MOD_BASE) - 4 - t_sp)/2;
            t_sclhi = I2C_MAX(t_sclhi_a,t_sclhi_b);
        } else {
            i2c_driver_deinit();
            system_printf("IIC clock parameter set error\n");
            return ERROR_I2C_PARAM;
        }
        if (t_sudat < 0|| t_hddat <0 || t_sclhi < 0 || t_sclratio < 0) {
            i2c_driver_deinit();
            return ERROR_I2C_FAILED;
        }
        I2C_CONF->setup_cfg.t_sudat = t_sudat;
        I2C_CONF->setup_cfg.t_hddat = t_hddat;
        I2C_CONF->setup_cfg.t_sclhi = t_sclhi;
        I2C_CONF->setup_cfg.t_sclratio = t_sclratio;
        I2C_CONF->setup_cfg.i2c_en = 1;
    } else if (i2c_config->mode == I2C_MODE_SLAVE) {
        I2C_CONF->setup_cfg.work_mode = I2C_MODE_SLAVE;
        I2C_CONF->setup_cfg.addr_mode = i2c_config->addr_mode;
        I2C_CONF->setup_cfg.dma_enable = i2c_config->dma_mode_enbale;
        I2C_CONF->setup_cfg.i2c_en = 1;
    } else {
        i2c_driver_deinit();
        return ERROR_I2C_PARAM;
    }
    irq_isr_register(IRQ_VECTOR_I2C, (void *)hal_i2c_isr);
    irq_status_clean(IRQ_VECTOR_I2C);
    irq_unmask(IRQ_VECTOR_I2C);
    return I2C_OK;
}

int i2c_driver_deinit(void)
{
    CLK_DISABLE(CLK_I2C);
    CLK_DISABLE(CLK_BUF_MEM);
    CLK_DISABLE(CLK_AHB_DAMC);
    I2C_CONF->i2c_cmd.cmd = I2C_CMD_CLEARFIFO;
    I2C_CONF->i2c_cmd.cmd = I2C_CMD_RESET;
    I2C_CONF->intr_reg.val = 0;
    I2C_CONF->setup_cfg.i2c_en = 0;
    irq_status_clean(IRQ_VECTOR_I2C);
    irq_mask(IRQ_VECTOR_I2C);

    if (p_i2c_handle) {
        os_free(p_i2c_handle);
    }
    if (!p_i2c_handle->cmd_mux) {
        xSemaphoreTake(p_i2c_handle->cmd_mux, portMAX_DELAY);
        vSemaphoreDelete(p_i2c_handle->cmd_mux);
    }
    if (p_i2c_handle->status_evt_queue) {
        vQueueDelete(p_i2c_handle->status_evt_queue);
    } 
    return I2C_OK;
}

int i2c_master_write(uint16_t slave_addr, uint8_t* data, size_t data_len, bool dma_mode, TickType_t ticks_to_wait)
{
#ifdef NAU8810U
    int i;

    vTaskDelay(pdMS_TO_TICKS(20));
    I2C_CONF->i2c_control.phasestart = 1;
    I2C_CONF->i2c_control.phaseaddr = 1;
    I2C_CONF->i2c_control.phasedata = 1;
    I2C_CONF->i2c_control.phasestop = 1;
    I2C_CONF->i2c_control.direction = 0;
    I2C_CONF->slave_addr.addr = slave_addr;
    I2C_CONF->i2c_control.datacnt_l4bit = data_len;
    I2C_CONF->i2c_control.datacnt_h4bit = 0x0;
    for (i = 0; i < data_len; i++) {
        I2C_CONF->fifo_data.data = *(data++);
    }
    I2C_CONF->i2c_cmd.cmd = I2C_CMD_ISSUEDATATRANS;
#else
    unsigned int index = 0;       
    i2c_evt_t evt;
    if (!data) {
        return ERROR_I2C_PARAM;
    }
    if (I2C_CONF->intr_bus_status.bus_busy)
        return ERROR_I2C_BUSY;

    portBASE_TYPE res = xSemaphoreTake(p_i2c_handle->cmd_mux, ticks_to_wait);
    if (res == pdFALSE) {
        return ERROR_I2C_TIMEOUT;
    }
    portTickType ticks_start = xTaskGetTickCount();
    I2C_CONF->i2c_control.phasestart = 1;
    I2C_CONF->i2c_control.phaseaddr = 1;
    I2C_CONF->i2c_control.phasedata = 1;
    I2C_CONF->i2c_control.phasestop = 1;
    I2C_CONF->i2c_control.direction = 0;
    I2C_CONF->slave_addr.addr = slave_addr;
    //I2C_CONF->intr_reg.addr_hit = 1;
    //I2C_CONF->intr_reg.cmpl = 1;
    if (dma_mode == false) {
        p_i2c_handle->dma_mode = I2C_CONF->setup_cfg.dma_enable = 0;

        TickType_t wait_time = xTaskGetTickCount();
        if (wait_time - ticks_start > ticks_to_wait) { // out of time
            wait_time = I2C_CMD_VALID_TICK;
        } else {
            wait_time = ticks_to_wait - (wait_time - ticks_start);
            if (wait_time < I2C_CMD_VALID_TICK) {
                wait_time = I2C_CMD_VALID_TICK;
            }
        }
        p_i2c_handle->tx_data_remain = data_len;
        while(1) {
            I2C_CONF->intr_reg.fifo_empty = 1;
            portBASE_TYPE evt_res = xQueueReceive(p_i2c_handle->status_evt_queue, &evt, wait_time);
            if (evt_res == pdTRUE) {
                if (evt.type == I2C_FIFO_EMPTY) {
                    if(p_i2c_handle->tx_data_remain == 0){
                        //diable all i2s interruption and break
                        I2C_CONF->intr_reg.val = 0;
                        break;
                    }
                    I2C_CONF->i2c_cmd.cmd = I2C_CMD_ISSUEDATATRANS;
                    if (p_i2c_handle->tx_data_remain >= 0xFF) {
                        // put data in fifo
                        I2C_CONF->i2c_control.datacnt_l4bit = 0xF;
                        I2C_CONF->i2c_control.datacnt_h4bit = 0xF;
                        index = 0xFF;
                        while(index){
                            I2C_CONF->fifo_data.data = *(data++);
                            index--;
                        }
                        I2C_CONF->i2c_cmd.cmd = I2C_CMD_ISSUEDATATRANS;
                        p_i2c_handle->tx_data_remain -= 0xFF;
                    } else {
                        I2C_CONF->i2c_control.datacnt_l4bit = p_i2c_handle->tx_data_remain & 0xF;
                        I2C_CONF->i2c_control.datacnt_h4bit = (p_i2c_handle->tx_data_remain & 0xF0) >> 4;
                        index = p_i2c_handle->tx_data_remain;
                        while(index){
                            I2C_CONF->fifo_data.data = *(data++);
                            index--;
                        }

                        p_i2c_handle->tx_data_remain = 0;
                    }
                    
                }else if (evt.type == I2C_FIFO_FULL) {
                    while (I2C_CONF->intr_bus_status.fifo_full) {
                        vTaskDelay(1/portTICK_RATE_MS);
                    }
                } else if(evt.type == I2C_CMPL) {
                    if (I2C_CONF->intr_bus_status.addr_hit == 1) {
                        I2C_CONF->intr_bus_status.cmpl = 1;
                    } else {
                        I2C_CONF->intr_reg.val = 0;
                        xSemaphoreGive(p_i2c_handle->cmd_mux);
                        return ERROR_I2C_FAILED;
                    }
                }
            }else {
                xSemaphoreGive(p_i2c_handle->cmd_mux);
                return ERROR_I2C_TIMEOUT;
            }
        }
    }else {
        p_i2c_handle->dma_mode = I2C_CONF->setup_cfg.dma_enable = 1;
        I2C_CONF->i2c_control.datacnt_l4bit = p_i2c_handle->tx_data_remain & 0xF;
        I2C_CONF->i2c_control.datacnt_h4bit = (p_i2c_handle->tx_data_remain & 0xF0) >> 4;
        DMA_Write_Config(data, p_i2c_handle->tx_data_remain);
        I2C_CONF->i2c_cmd.cmd = I2C_CMD_ISSUEDATATRANS;
        portBASE_TYPE evt_res = xQueueReceive(p_i2c_handle->status_evt_queue, &evt, I2C_CMD_VALID_TICK);
        if (evt_res == pdTRUE) {
            if (evt.type == I2C_CMPL) {
                if ((I2C_CONF->i2c_control.datacnt_l4bit | (I2C_CONF->i2c_control.datacnt_h4bit) << 4) == 0) {
                    xSemaphoreGive(p_i2c_handle->cmd_mux);
                    return I2C_OK;
                }
            }
        } else {
            xSemaphoreGive(p_i2c_handle->cmd_mux);
            return ERROR_I2C_TIMEOUT;
        }
        //todo: need IMP data size > 256 with DMA mode
    }
    xSemaphoreGive(p_i2c_handle->cmd_mux);
#endif
    return I2C_OK;
}

int i2c_master_read(uint16_t slave_addr, uint8_t* data, size_t data_len, bool dma_mode, TickType_t ticks_to_wait)
{
    unsigned int index = 0;      
    i2c_evt_t evt;
    if (!data) {
        return ERROR_I2C_PARAM;
    }
#ifdef NAU8810U
    vTaskDelay(pdMS_TO_TICKS(10));
    I2C_CONF->i2c_control.phasestart = 1;
    I2C_CONF->i2c_control.phaseaddr = 1;
    I2C_CONF->i2c_control.phasedata = 1;
    I2C_CONF->i2c_control.phasestop = 0;
    I2C_CONF->i2c_control.direction = 0;
    I2C_CONF->slave_addr.addr = slave_addr;
    I2C_CONF->i2c_control.datacnt_l4bit = 1;
    I2C_CONF->i2c_control.datacnt_h4bit = 0;
    I2C_CONF->i2c_cmd.cmd = I2C_CMD_ISSUEDATATRANS;
    I2C_CONF->fifo_data.data = *(data++);

    vTaskDelay(pdMS_TO_TICKS(10));
    I2C_CONF->i2c_control.phasestart = 1;
    I2C_CONF->i2c_control.phaseaddr = 1;
    I2C_CONF->i2c_control.phasedata = 1;
    I2C_CONF->i2c_control.phasestop = 1;
    I2C_CONF->i2c_control.direction = 1;
    I2C_CONF->slave_addr.addr = slave_addr;
    I2C_CONF->i2c_control.datacnt_l4bit = 2;
    I2C_CONF->i2c_control.datacnt_h4bit = 0;
    I2C_CONF->i2c_cmd.cmd = I2C_CMD_ISSUEDATATRANS;
    while (data_len) {
        if (!I2C_CONF->intr_bus_status.fifo_empty) {
            *(data++) = I2C_CONF->fifo_data.data;
            --data_len;
        }
    }
#else
    if(I2C_CONF->intr_bus_status.bus_busy)
        return ERROR_I2C_BUSY;
    
    portBASE_TYPE res = xSemaphoreTake(p_i2c_handle->cmd_mux, ticks_to_wait);
    if (res == pdFALSE) {
        return ERROR_I2C_TIMEOUT;
    }

    I2C_CONF->i2c_control.phasestart = 1;
    I2C_CONF->i2c_control.phaseaddr = 1;
    I2C_CONF->i2c_control.phasedata = 1;
    I2C_CONF->i2c_control.phasestop = 1;
    I2C_CONF->i2c_control.direction = 1;
    I2C_CONF->slave_addr.addr = slave_addr;
    I2C_CONF->intr_reg.cmpl = 1;
    I2C_CONF->intr_reg.addr_hit = 1;
    if (dma_mode == false) {
        p_i2c_handle->rx_data_remain = data_len;
        I2C_CONF->i2c_cmd.cmd = I2C_CMD_ISSUEDATATRANS;
        while (1) {
            //I2C_CONF->intr_reg.fifo_full = 1;
            portBASE_TYPE evt_res = xQueueReceive(p_i2c_handle->status_evt_queue, &evt, (portTickType)portMAX_DELAY);
            if (evt_res == pdTRUE) {
                if (evt.type == I2C_FIFO_FULL) {
                    while(!I2C_CONF->intr_bus_status.fifo_empty) {
                        *(data++) = I2C_CONF->fifo_data.data;
                    }
                    I2C_CONF->i2c_cmd.cmd = I2C_CMD_ISSUEDATATRANS;
                    //because I2C_CONF->fifo_size.fifo_size is 1,means the fifo is 4 bytes deapth
                    p_i2c_handle->rx_data_remain -= 4;
                } else if (evt.type == I2C_CMPL) {
                    if(I2C_CONF->intr_bus_status.addr_hit == 1){
                        I2C_CONF->intr_bus_status.cmpl = 1;
                        while ((I2C_CONF->i2c_control.datacnt_l4bit | (I2C_CONF->i2c_control.datacnt_h4bit) << 4)) {
                            *(data++) = I2C_CONF->fifo_data.data;
                            p_i2c_handle->rx_data_remain--;
                        }
                        if (p_i2c_handle->rx_data_remain == 0) {
                            xSemaphoreGive(p_i2c_handle->cmd_mux);
                            return I2C_OK;
                        } else {
                            continue;
                        }
                    }
                } else {
                    I2C_CONF->intr_reg.val = 0;
                    xSemaphoreGive(p_i2c_handle->cmd_mux);
                    return ERROR_I2C_FAILED;
                }
            } else {
                xSemaphoreGive(p_i2c_handle->cmd_mux);
                return ERROR_I2C_TIMEOUT;
            }
        }
        xSemaphoreGive(p_i2c_handle->cmd_mux);
        return I2C_OK;
    } else {
        p_i2c_handle->dma_mode = I2C_CONF->setup_cfg.dma_enable = 1;
        I2C_CONF->i2c_control.datacnt_l4bit = p_i2c_handle->rx_data_remain;
        I2C_CONF->i2c_control.datacnt_h4bit = p_i2c_handle->rx_data_remain >> 4;
        DMA_Read_Config(data, data_len);
        I2C_CONF->i2c_cmd.cmd = I2C_CMD_ISSUEDATATRANS;
        portBASE_TYPE evt_res = xQueueReceive(p_i2c_handle->status_evt_queue, &evt, I2C_CMD_VALID_TICK);
        if (evt_res == pdTRUE) {
            if (evt.type == I2C_CMPL) {
                if ((I2C_CONF->i2c_control.datacnt_l4bit | (I2C_CONF->i2c_control.datacnt_h4bit) << 4) == 0) {
                    xSemaphoreGive(p_i2c_handle->cmd_mux);
                    return I2C_OK;
                }
            }
        } else {
            xSemaphoreGive(p_i2c_handle->cmd_mux);
            return ERROR_I2C_TIMEOUT;
        }
    }
    xSemaphoreGive(p_i2c_handle->cmd_mux);
#endif
    return I2C_OK;
}

int i2c_slave_write(uint8_t* data, size_t data_len, bool dma_mode, TickType_t ticks_to_wait)
{
    unsigned int index = 0;
    i2c_evt_t evt;
    if (!data) {
        return ERROR_I2C_PARAM;
    }
    if(I2C_CONF->intr_bus_status.bus_busy)
        return ERROR_I2C_BUSY;
    
    portBASE_TYPE res = xSemaphoreTake(p_i2c_handle->cmd_mux, ticks_to_wait);
    if (res == pdFALSE) {
        return ERROR_I2C_TIMEOUT;
    }

    I2C_CONF->i2c_control.direction = 1;
    I2C_CONF->intr_reg.cmpl = 1;
    I2C_CONF->intr_reg.addr_hit = 1;
    //waiting for addr hit interrupt
    if (dma_mode == false) {
        p_i2c_handle->dma_mode = I2C_CONF->setup_cfg.dma_enable = 0;
        portTickType ticks_start = xTaskGetTickCount();
        
        TickType_t wait_time = xTaskGetTickCount();
        if (wait_time - ticks_start > ticks_to_wait) { // out of time
            wait_time = I2C_CMD_VALID_TICK;
        } else {
            wait_time = ticks_to_wait - (wait_time - ticks_start);
            if (wait_time < I2C_CMD_VALID_TICK) {
                wait_time = I2C_CMD_VALID_TICK;
            }
        }
        p_i2c_handle->tx_data_remain = data_len;
        while(1) {
            portBASE_TYPE evt_res = xQueueReceive(p_i2c_handle->status_evt_queue, &evt, wait_time);
            if (evt_res == pdTRUE) {
                if (evt.type == I2C_ADDR_HIT) {
                    I2C_CONF->intr_reg.fifo_empty = 1;
                } else if(evt.type == I2C_FIFO_EMPTY){
                    index = 4;
                    while(index){
                        I2C_CONF->fifo_data.data = *(data++);
                        index--;
                    }
                    p_i2c_handle->tx_data_remain -= 4;
                } else if(evt.type == I2C_CMPL) {
                    I2C_CONF->i2c_cmd.cmd = I2C_CMD_CLEARFIFO;
                    if (data_len == (I2C_CONF->i2c_control.datacnt_l4bit | (I2C_CONF->i2c_control.datacnt_h4bit) << 4) && p_i2c_handle->tx_data_remain == 0) {
                        I2C_CONF->intr_bus_status.cmpl = 1;
                        I2C_CONF->intr_reg.val = 0;
                        break;
                    } else {
                        I2C_CONF->intr_bus_status.cmpl = 1;
                        continue;
                    }
                }
            } else {
                xSemaphoreGive(p_i2c_handle->cmd_mux);
                return ERROR_I2C_TIMEOUT;
            }
        }
    } else {
        I2C_CONF->setup_cfg.dma_enable = 1;
        I2C_CONF->i2c_control.datacnt_l4bit = data_len;
        I2C_CONF->i2c_control.datacnt_h4bit = data_len >> 4;
        DMA_Write_Config(data, data_len);
        portBASE_TYPE evt_res = xQueueReceive(p_i2c_handle->status_evt_queue, &evt, I2C_CMD_VALID_TICK);
        if (evt_res == pdTRUE) {
            if (evt.type == I2C_ADDR_HIT) {
                if (I2C_CONF->intr_bus_status.gencall) {
                    //don't support now;
                    return ERROR_I2C_FAILED;
                }
            } else if (evt.type == I2C_CMPL) {
                if ((I2C_CONF->i2c_control.datacnt_l4bit | (I2C_CONF->i2c_control.datacnt_h4bit)<<4) == 0) {
                     p_i2c_handle->tx_data_remain = 0;
                    xSemaphoreGive(p_i2c_handle->cmd_mux);
                    return I2C_OK;
                }
            }
        } else {
            xSemaphoreGive(p_i2c_handle->cmd_mux);
            return ERROR_I2C_TIMEOUT;
        }
    }
    xSemaphoreGive(p_i2c_handle->cmd_mux);
    return I2C_OK;
}

int i2c_slave_read(uint8_t* data, size_t data_len, bool dma_mode, TickType_t ticks_to_wait)
{
    unsigned int index = 0;
    i2c_evt_t evt;
    if (!data) {
        return ERROR_I2C_PARAM;
    }
    if (I2C_CONF->intr_bus_status.bus_busy)
        return ERROR_I2C_BUSY;
    
    portBASE_TYPE res = xSemaphoreTake(p_i2c_handle->cmd_mux, ticks_to_wait);
    if (res == pdFALSE) {
        return ERROR_I2C_TIMEOUT;
    }

    I2C_CONF->i2c_control.direction = 1;
    I2C_CONF->intr_reg.cmpl = 1;
    I2C_CONF->intr_reg.addr_hit = 1;
    //waiting for addr hit interrupt
    if (dma_mode == false) {
        p_i2c_handle->dma_mode = I2C_CONF->setup_cfg.dma_enable = 0;
        portTickType ticks_start = xTaskGetTickCount();
        
        TickType_t wait_time = xTaskGetTickCount();
        if (wait_time - ticks_start > ticks_to_wait) { // out of time
            wait_time = I2C_CMD_VALID_TICK;
        } else {
            wait_time = ticks_to_wait - (wait_time - ticks_start);
            if (wait_time < I2C_CMD_VALID_TICK) {
                wait_time = I2C_CMD_VALID_TICK;
            }
        }
        p_i2c_handle->tx_data_remain = data_len;
        while(1) {
            portBASE_TYPE evt_res = xQueueReceive(p_i2c_handle->status_evt_queue, &evt, wait_time);
            if (evt_res == pdTRUE) {
                if (evt.type == I2C_ADDR_HIT) {
                    I2C_CONF->intr_reg.fifo_full = 1;
                } else if (evt.type == I2C_FIFO_FULL) {
                    index = 4;
                    while (index) {
                        *(data++) = I2C_CONF->fifo_data.data;
                        index--;
                    }
                    p_i2c_handle->tx_data_remain -= 4;
                } else if(evt.type == I2C_CMPL) {
                    I2C_CONF->i2c_cmd.cmd = I2C_CMD_CLEARFIFO;
                    if (data_len == (I2C_CONF->i2c_control.datacnt_l4bit | (I2C_CONF->i2c_control.datacnt_h4bit) << 4) && p_i2c_handle->tx_data_remain == 0) {
                        I2C_CONF->intr_bus_status.cmpl = 1;
                        I2C_CONF->intr_reg.val = 0;
                        break;
                    } else {
                        I2C_CONF->intr_bus_status.cmpl = 1;
                        continue;
                    }
                }
            } else {
                xSemaphoreGive(p_i2c_handle->cmd_mux);
                return ERROR_I2C_TIMEOUT;
            }
        }
    } else {
        I2C_CONF->setup_cfg.dma_enable = 1;
        I2C_CONF->i2c_control.datacnt_l4bit = data_len;
        I2C_CONF->i2c_control.datacnt_h4bit = data_len >> 4;
        DMA_Write_Config(data, data_len);
        portBASE_TYPE evt_res = xQueueReceive(p_i2c_handle->status_evt_queue, &evt, I2C_CMD_VALID_TICK);
        if (evt_res == pdTRUE) {
            if (evt.type == I2C_ADDR_HIT) {
                if (I2C_CONF->intr_bus_status.gencall) {
                    //don't support now;
                    return ERROR_I2C_FAILED;
                }
            } else if (evt.type == I2C_CMPL) {
                if ((I2C_CONF->i2c_control.datacnt_l4bit | (I2C_CONF->i2c_control.datacnt_h4bit) << 4) == 0) {
                     p_i2c_handle->tx_data_remain = 0;
                    xSemaphoreGive(p_i2c_handle->cmd_mux);
                    return I2C_OK;
                }
            }
        } else {
            xSemaphoreGive(p_i2c_handle->cmd_mux);
            return ERROR_I2C_TIMEOUT;
        }
    }
    xSemaphoreGive(p_i2c_handle->cmd_mux);
    return I2C_OK;
}

int i2c_reset_fifo(void)
{
    I2C_CONF->i2c_cmd.cmd = I2C_CMD_RESET;
    return I2C_OK;
}

bool i2c_bus_is_busy(void)
{
    if(I2C_CONF->intr_bus_status.bus_busy)
        return true;
    else
        return false;
}

int i2c_register_callback(i2c_callback_t i2c_callback_func, void* arg)
{
    g_i2c_callback = i2c_callback_func;
    g_i2c_arg = arg;
    return I2C_OK;
}

#ifdef NAU8810U
static int i2c_nau_driver_init(void)
{
    i2c_config_t i2c_config;
    i2c_config.addr_mode = I2C_ADDR_BIT_7;
    i2c_config.dma_mode_enbale = I2C_DMA_DISABLE;
    i2c_config.master.clk_speed = I2C_CLK_100K;
    i2c_config.mode = I2C_MODE_MASTER;

    i2c_driver_init(&i2c_config);

    return CMD_RET_SUCCESS;
}

static int i2c_nau_readcfg(cmd_tbl_t *t, int argc, char *argv[])
{
    int i = 0;
    unsigned char data[3] = {0};
    bool dma_en = false;

    data[0] = (strtoul(argv[1], NULL, 0) << 1);
    i2c_master_read(0x1A, (uint8_t *)data, 2, dma_en, 0);
    system_printf("data[1]=0x%x\n", data[1]);
    system_printf("data[2]=0x%x\n", data[2]);

    return CMD_RET_SUCCESS;
}
CMD(i2c_nau_read, i2c_nau_readcfg, "i2c_nau_read", "i2c_nau_read");

static int i2c_nau_writecfg(cmd_tbl_t *t, int argc, char *argv[])
{
    int i = 0;
    unsigned char data[2] = {0};
    bool dma_en = false;

    data[0] = ((strtoul(argv[1], NULL, 0) << 1) | strtoul(argv[2], NULL, 0));
    data[1] = (strtoul(argv[3], NULL, 0));
    i2c_master_write(0x1A, (uint8_t *)data, 2, dma_en, 0);

    return CMD_RET_SUCCESS;
}
CMD(i2c_nau_write, i2c_nau_writecfg, "i2c_nau_write", "i2c_nau_write");

static int i2c_anuconfig_init(cmd_tbl_t *t, int argc, char *argv[])
{
    int i = 0;
    unsigned char data[2] = {0};
    bool dma_en = false;
    const uint8_t conifg[55][3] = {
        //04-68  //07-00  //2c-00 //2d-50 //3c-20
        //alaw 05=>0x3e
        {0x00,0x0,0x01}, {0x01,0x1,0x1d}, {0x02,0x0,0x15}, {0x03,0x0,0xed}, {0x04,0x0,0x08}, {0x05,0x0,0x00},
        {0x06,0x0,0x00}, {0x07,0x0,0x06}, {0x0a,0x0,0x08}, {0x0b,0x0,0xff}, {0x0e,0x1,0x08}, {0x0f,0x0,0xff},
        {0x12,0x1,0x2c}, {0x13,0x0,0x2c}, {0x14,0x0,0x2c}, {0x15,0x0,0x2c}, {0x16,0x0,0x2c}, {0x18,0x0,0x32},
        {0x19,0x0,0x00}, {0x1b,0x0,0x00}, {0x1c,0x0,0x00}, {0x1d,0x0,0x00}, {0x1e,0x0,0x00}, {0x20,0x0,0x38},
        {0x21,0x0,0x0b}, {0x22,0x0,0x32}, {0x23,0x0,0x00}, {0x24,0x0,0x08}, {0x25,0x0,0x0c}, {0x26,0x0,0x93},
        {0x27,0x0,0xe9}, {0x28,0x0,0x00}, {0x2c,0x0,0x03}, {0x2d,0x0,0x10}, {0x2f,0x1,0x00}, {0x31,0x0,0x02},
        {0x32,0x0,0x01}, {0x36,0x0,0x3f}, {0x38,0x0,0x40}, {0x3a,0x0,0x00}, {0x3b,0x0,0x00}, {0x3c,0x0,0x24},
        {0x3e,0x0,0xee}, {0x3f,0x0,0x1a}, {0x40,0x0,0x07}, {0x41,0x0,0x00}, {0x45,0x1,0xff}, {0x46,0x1,0xff},
        {0x47,0x1,0xff}, {0x49,0x1,0xff}, {0x4b,0x0,0x00}, {0x4c,0x0,0x00}, {0x4d,0x1,0xff}, {0x4e,0x0,0x20},
        {0x4f,0x1,0xff}};

    i2c_nau_driver_init();
    vTaskDelay(pdMS_TO_TICKS(200));
    for (i = 0; i < 42; i++) {
        data[0] = (conifg[i][0] << 1) | (conifg[i][1]);
        data[1] = conifg[i][2];
        i2c_master_write(0x1A, (uint8_t *)data, 2, dma_en, 0);
    }

    return CMD_RET_SUCCESS;
}
CMD(i2c_nau_init, i2c_anuconfig_init, "i2c_anuconfig_init", "i2c_anuconfig_init");

static int i2c_nauconfig_read(cmd_tbl_t *t, int argc, char *argv[])
{
    int i = 0;
    unsigned char data[3] = {0};
    bool dma_en = false;

    for(i = 1; i <= 0x3C; i++) {
        data[0] = (i << 1);
        i2c_master_read(0x1A, (uint8_t *)data, 2, dma_en, 0);
        system_printf("data[0x%x]=0x%x\n", i, (data[1] << 8) | data[2]);
    }

    return CMD_RET_SUCCESS;
}
CMD(i2c_nau_config, i2c_nauconfig_read, "i2c_nauconfig_read", "i2c_nauconfig_read");

#endif

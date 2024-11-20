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

#include "drv_i2s.h"
#include "bsp/soc.h"
#include "bsp/soc_pin_mux.h"
//#include "hello_input.h"

#define FFI (1 << 16)
int32_t hal_i2s_isr(int32_t vector)
{
    uint32_t lev;
    uint32_t value;
    uint32_t value1;
    uint32_t value2;
    uint32_t value3;
    uint32_t irq_type;
    int i;

    lev = IN32(I2S_BASE + 0x818);
    if (vector != IRQ_VECTOR_I2S) {
        system_printf("vector not correct\n");
        return 0;
    }

    irq_type = IN32(I2S_BASE + 0x810) & FFI;
    if (irq_type != 0) {
        //lev = IN32(I2S_BASE + 0x818);
        if (lev > 0) {
            value = IN32(I2S_BASE + 0x800);
            value1 = IN32(I2S_BASE + 0x800);
            value2 = IN32(I2S_BASE + 0x800);
            value3 = IN32(I2S_BASE + 0x800);
            system_printf("lev=%d 0x%x-0x%x-0x%x-0x%x\n", lev, value, value1, value2, value3);
        }
    }

    return 0;
}

void hal_i2s_init()
{
    OUT32(0x60180c, 0xFFFFFFFF);
    OUT32(0x601810, 0xffdfffff);
    OUT32(0x601810, 0xffffffff);

    //PIN_MUX
    unsigned int value;
    value = IN32(SOC_PIN0_MUX_BASE) & (~(7 << 12));
    OUT32(SOC_PIN0_MUX_BASE, value | (6 << 12));     /* I2S_MCLK GPIO4 */
    value = IN32(SOC_PIN1_MUX_BASE) & (~(7 << 9));
    OUT32(SOC_PIN1_MUX_BASE, value | (2<<9));         /* I2S_TXD GPIO13 */
    value = IN32(SOC_PIN1_MUX_BASE) & (~(7 << 20));
    OUT32(SOC_PIN1_MUX_BASE, value | (2 << 20));      /* I2S_TXWS GPIO20 */
    value = IN32(SOC_PIN1_MUX_BASE) & (~(7 << 23));
    OUT32(SOC_PIN1_MUX_BASE, value | (2 << 23));      /* I2S_TXSCK GPIO21 */

    value = IN32(SOC_PIN1_MUX_BASE) & (~(7 << 26));
    OUT32(SOC_PIN1_MUX_BASE, value | (2 << 26));      /* I2S_RXD GPIO22 */
    //value = IN32(SOC_PIN1_MUX_BASE) & (~(7 << 29));
    //OUT32(SOC_PIN1_MUX_BASE, value | (2 << 29));    /* I2S_RXWS GPIO23 */
    //value = IN32(SOC_PIN0_MUX_BASE) & (~(3 << 30));
    //OUT32(SOC_PIN0_MUX_BASE, value | (2 << 30));    /* I2S_RXSCK GPIO24 */


    //I2S_MCLK 48Khz 12.288MHz
    //OUT32(0x601840, 0x17);
    //OUT32(0x601848, 0x6);
    //OUT32(0x60184C, 0x30000001);

    //I2S_MCLK 16Khz 4.096MHz
    OUT32(0x601840, 0x17);
    OUT32(0x601848, 0x13);
    OUT32(0x60184C, 0x8000005);

    OUT32(I2S_BASE + 0x004, 0x2);
    OUT32(I2S_BASE + 0x008, 0x2403df0);
    OUT32(I2S_BASE + 0x00c, 0xF);

    OUT32(I2S_BASE + 0x804, 0x4);
    OUT32(I2S_BASE + 0x808, 0x403df0);
    OUT32(I2S_BASE + 0x80C, 0x1);

    OUT32(I2S_BASE + 0x810, 0x01);
    OUT32(I2S_BASE + 0xC10, 0x30);

    //irq_isr_register(IRQ_VECTOR_I2S, (void *)hal_i2s_isr);
    //irq_status_clean(IRQ_VECTOR_I2S);
    //irq_unmask(IRQ_VECTOR_I2S);

    OUT32(I2S_BASE + 0xc00, 0x33);
}

int hal_i2s_read()
{
    OUT32(I2S_BASE + 0x804, 0x4);
    OUT32(I2S_BASE + 0x808, 0x403df0);
    OUT32(I2S_BASE + 0x80C, 0x1);

    OUT32(I2S_BASE + 0x810, 0x01);
    OUT32(I2S_BASE + 0xC10, 0x30);

    irq_isr_register(IRQ_VECTOR_I2S, (void *)hal_i2s_isr);
    irq_status_clean(IRQ_VECTOR_I2S);
    irq_unmask(IRQ_VECTOR_I2S);

    OUT32(I2S_BASE + 0xc00, 0x33);

    return CMD_RET_SUCCESS;
}

CMD(i2s_read, hal_i2s_read, "i2s_read", "i2s_read");

static int i2s_init_test(cmd_tbl_t *t, int argc, char *argv[])
{
    hal_i2s_init();

    //hal_i2s_read();

    return CMD_RET_SUCCESS;
}

CMD(i2s_init, i2s_init_test, "i2s_init", "i2s_init");

static int i2s_reset_test(cmd_tbl_t *t, int argc, char *argv[])
{
    OUT32(0x601810, 0xffdfffff);
    OUT32(0x601810, 0xffffffff);

    return CMD_RET_SUCCESS;
}

CMD(i2s_reset, i2s_reset_test, "i2s_reset", "i2s_reset");

static unsigned char linear2alaw(short pcm_data)
{
    int mask;
    int seg;
    int pcm_val;
    unsigned char aval;
    static int seg_end[8] = {0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF};

    pcm_val = pcm_data >> 3;
    if (pcm_val >= 0) {
        mask = 0xD5;
    } else {
        mask = 0x55;
        pcm_val = -pcm_val - 1;
    }

    for (seg = 0; seg < 8; seg++) {
        if (pcm_val <= seg_end[seg])
            break;
    }

    if (seg >= 8) {
        return (0x7F ^ mask);
    } else {
        aval = seg << 4;
        if (seg < 2)
            aval |= (pcm_val >> 1) & 0xf;
        else
            aval |= (pcm_val >> seg) & 0xf;

        return (aval ^ mask);
    }
}

static unsigned char linear2ulaw(short pcm_data)
{
    int mask;
    int seg;
    int uval;
    int pcm_val;
    static int seg_uend[8] = {0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF};
 
    pcm_val = (pcm_data + 0x84) >> 2;
    if (pcm_val < 0) {
        pcm_val = -pcm_val;
        mask = 0x7F;
    } else {
        mask = 0xFF;
    }

    if (pcm_val > 8159)
        pcm_val = 8159;
 
    for (seg = 0; seg < 8; seg++) {
        if (pcm_val <= seg_uend[seg])
            break;
    }
 
    if (seg >= 8) {
        return (0x7F ^ mask);
    } else {
        uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0xF);
        return (uval ^ mask);
    }
}

static int i2s_play_alaw(cmd_tbl_t *t, int argc, char *argv[])
{
    system_printf("i2s_play_alaw\n");

    int i,j;
    short pcm_data;
    uint32_t value = 0;
    uint32_t fifo;

    system_printf("begin play alaw\n");
    for (i = 0; i < sizeof(hello_data); i++) {
        j = i % 2;
        pcm_data = hello_data[i] << (8 * j);

        if (j == 1) {
            //value = linear2alaw(pcm_data);
            value = linear2ulaw(pcm_data);
            value = value << 24;
            fifo = (IN32(0xe00018) & 0x1f);
            while (fifo > 14) {
                fifo = (IN32(0xe00018) & 0x1f);
            }
            OUT32(0xe00000, value);
        }
    }
    system_printf("end play alaw\n");

    return CMD_RET_SUCCESS;
}

static int i2s_play_test(cmd_tbl_t *t, int argc, char *argv[])
{
    system_printf("i2s_play_test\n");

    int i,j;
    uint32_t value = 0;
    uint32_t fifo;

    system_printf("begin play\n");
    for (i = 0; i < sizeof(hello_data); i++) {
        j = i % 2;
        value |= ((hello_data[i]) << (8 * (j + 2)));
        fifo = (IN32(0xe00018) & 0x1f);
        if (j == 1) {
            while (fifo > 14) {
                fifo = (IN32(0xe00018) & 0x1f);
            }
            OUT32(0xe00000, value);
            value = 0;
        }
    }
    system_printf("end play\n");

    return CMD_RET_SUCCESS;
}

CMD(i2s_play, i2s_play_alaw, "i2s_play", "i2s_play");


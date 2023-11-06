/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_gpio.h"

#define DBG_TAG "if_gpio"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

typedef enum
{
    CPU_LOAD = 1,
    CPU_CHILD = 2
} E_CPU_TYPE;

typedef struct
{
    const char *pin_name;
    rt_base_t pin_index;
} S_GPIO_INFO;

typedef struct
{
    S_GPIO_INFO *cpu_type_pin;
    S_GPIO_INFO *board_type_pin[3];
    S_GPIO_INFO *led_pin[E_IO_ID_MAX];

    E_CPU_TYPE cpu_type;
} S_BOARD_GPIO;

static S_GPIO_INFO pin_cfg[6] = { { .pin_name = "PI.0" }, };

static S_BOARD_GPIO board_gpio;

E_SLOT_ID if_gpio_getSlotId(void);

void if_gpio_init(void)
{
    uint8_t i;

    /* 1. 配置底板子板识别引脚 */
    board_gpio.cpu_type_pin = &pin_cfg[0];

    board_gpio.cpu_type_pin->pin_index = rt_pin_get(board_gpio.cpu_type_pin->pin_name);

    rt_pin_mode(board_gpio.cpu_type_pin->pin_index, PIN_MODE_INPUT);
    if (PIN_HIGH == rt_pin_read(board_gpio.cpu_type_pin->pin_index))
    {
        /* 子板 */
        board_gpio.cpu_type = CPU_CHILD;

        pin_cfg[1].pin_name = "PH.7";
        pin_cfg[2].pin_name = "PH.8";
        pin_cfg[3].pin_name = "PD.11";

        /* led */
        pin_cfg[4].pin_name = "PE.4";
        pin_cfg[5].pin_name = "PE.3";
    }
    else
    {
        /* 底板 */
        board_gpio.cpu_type = CPU_LOAD;

        pin_cfg[1].pin_name = "PI.2";
        pin_cfg[2].pin_name = "PI.3";
        pin_cfg[3].pin_name = "PD.11";

        /* led */
        pin_cfg[4].pin_name = "PI.5";
        pin_cfg[5].pin_name = "PI.6";
    }

    /* 2. 配置板子类型识别引脚 */

    for(i = 0; i < 3; i++)
    {
        board_gpio.board_type_pin[i] = &pin_cfg[i + 1];
        board_gpio.board_type_pin[i]->pin_index = rt_pin_get(board_gpio.board_type_pin[i]->pin_name);
        rt_pin_mode(board_gpio.board_type_pin[i]->pin_index, PIN_MODE_INPUT);
    }

    /* 3. 配置LED引脚 */
    for(i = 0; i < 2; i++)
    {
        board_gpio.led_pin[i] = &pin_cfg[i + 4];
        board_gpio.led_pin[i]->pin_index = rt_pin_get(board_gpio.led_pin[i]->pin_name);
        rt_pin_mode(board_gpio.led_pin[i]->pin_index, PIN_MODE_OUTPUT);
    }
}

void if_gpio_set(E_IO_ID id)
{
    if (id >= E_IO_ID_3)
    {
        LOG_E("id >= E_IO_ID_3");
        return;
    }
    else
    {
        rt_pin_write(board_gpio.led_pin[id]->pin_index, PIN_HIGH);
    }
}

void if_gpio_reset(E_IO_ID id)
{
    if (id >= E_IO_ID_3)
    {
        LOG_E("id >= E_IO_ID_3");
        return;
    }
    else
    {
        rt_pin_write(board_gpio.led_pin[id]->pin_index, PIN_LOW);
    }
}

void if_gpio_toggle(E_IO_ID id)
{
    if (id >= E_IO_ID_3)
    {
        LOG_E("id >= E_IO_ID_3");
        return;
    }
    else
    {
        if (rt_pin_read(board_gpio.led_pin[id]->pin_index) == 1)
        {
            rt_pin_write(board_gpio.led_pin[id]->pin_index, PIN_LOW);
        }
        else
        {
            rt_pin_write(board_gpio.led_pin[id]->pin_index, PIN_HIGH);
        }
    }
}

BOOL if_gpio_get(E_IO_ID id)
{
    if (id >= E_IO_ID_3)
    {
        LOG_E("id >= E_IO_ID_3");
        return FALSE;
    }
    else
    {
        return rt_pin_read(board_gpio.led_pin[id]->pin_index);
    }
}

E_SLOT_ID if_gpio_getSlotId(void)
{
    static E_SLOT_ID slot_id = E_SLOT_ID_MAX;
    int gpio_status;
    uint8_t board_data = 0;

    if (E_SLOT_ID_MAX != slot_id)
    {
        return slot_id;
    }
    else
    {
        gpio_status = rt_pin_read(board_gpio.board_type_pin[0]->pin_index);
        board_data |= gpio_status;

        gpio_status = rt_pin_read(board_gpio.board_type_pin[1]->pin_index);
        board_data |= gpio_status << 1;

        gpio_status = rt_pin_read(board_gpio.board_type_pin[2]->pin_index);
        board_data |= gpio_status << 2;

        switch ((board_data & 0x07))
        {
        case 0x01:
            if (CPU_LOAD == board_gpio.cpu_type)
            {
                slot_id = E_SLOT_ID_1; /* I系通信1 底板 */
            }
            else
            {
                slot_id = E_SLOT_ID_2; /* I系通信1 子板 */
            }
            break;
        case 0x02:
            if (CPU_LOAD == board_gpio.cpu_type)
            {
                slot_id = E_SLOT_ID_3; /* I系通信2 底板 */
            }
            else
            {
                slot_id = E_SLOT_ID_4; /* I系通信2 子板 */
            }
            break;
        case 0x05:
            if (CPU_LOAD == board_gpio.cpu_type)
            {
                slot_id = E_SLOT_ID_5; /* II系通信1 底板 */
            }
            else
            {
                slot_id = E_SLOT_ID_6; /* II系通信1 子板 */
            }
            break;
        case 0x06:
            if (CPU_LOAD == board_gpio.cpu_type)
            {
                slot_id = E_SLOT_ID_7; /* II系通信2 底板 */
            }
            else
            {
                slot_id = E_SLOT_ID_8; /* II系通信2 子板 */
            }
            break;
        default:
            return E_SLOT_ID_MAX;
        }
    }
    return slot_id;
}

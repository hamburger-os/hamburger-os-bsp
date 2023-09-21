/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-17     lvhan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "selftest.h"

#define DBG_TAG "gpio"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static char * error_log1[] = {
    "PWM1       ----> GPIO1     ",
    "GPIO5      ----> GPIO6     ",
    "GPIO8      ----> SPI2_CS1  ",
    "GPIO4      ----> GPIO7     ",
    "SPI1_CS1   ----> SPI1_CS2  ",
    "PWM2       ----> GPIO2     ",
};

static char * error_log2[] = {
    "PWM1       <---- GPIO1     ",
    "GPIO5      <---- GPIO6     ",
    "GPIO8      <---- SPI2_CS1  ",
    "GPIO4      <---- GPIO7     ",
    "SPI1_CS1   <---- SPI1_CS2  ",
    "PWM2       <---- GPIO2     ",
};

void selftest_gpio_test(SelftestUserData *puserdata)
{
    //TODO: 测试短路
    for (int value = 0; value < 2; value++)
    {
        uint8_t is_error = 0;
        for (int x = 0; x<6; x++)
        {
            puserdata->gpio_pin[x][0] = rt_pin_get(puserdata->gpio_devname[x][0]);
            puserdata->gpio_pin[x][1] = rt_pin_get(puserdata->gpio_devname[x][1]);

            rt_pin_mode(puserdata->gpio_pin[x][0], PIN_MODE_OUTPUT);
            rt_pin_write(puserdata->gpio_pin[x][0], value);

            rt_pin_mode(puserdata->gpio_pin[x][1], PIN_MODE_INPUT);
            if (rt_pin_read(puserdata->gpio_pin[x][1]) != value)
            {
                LOG_E("%s %d error!", error_log1[x], value);
                is_error++;
            }

            rt_pin_mode(puserdata->gpio_pin[x][1], PIN_MODE_OUTPUT);
            rt_pin_write(puserdata->gpio_pin[x][1], value);

            rt_pin_mode(puserdata->gpio_pin[x][0], PIN_MODE_INPUT);
            if (rt_pin_read(puserdata->gpio_pin[x][0]) != value)
            {
                LOG_E("%s %d error!", error_log2[x], value);
                is_error++;
            }

            rt_pin_mode(puserdata->gpio_pin[x][1], PIN_MODE_INPUT);
        }
        if (is_error == 0)
        {
            if (value == 0)
            {
                LOG_D("gpio LOW    pass");
                puserdata->result[RESULT_GPIO_LOW].result = 0;
            }
            else
            {
                LOG_D("gpio HIGH   pass");
                puserdata->result[RESULT_GPIO_HIGH].result = 0;
            }
        }
        else
        {
            if (value == 0)
            {
                puserdata->result[RESULT_GPIO_LOW].result = is_error;
            }
            else
            {
                puserdata->result[RESULT_GPIO_HIGH].result = is_error;
            }
        }
    }
}

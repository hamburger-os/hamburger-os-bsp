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
    uint8_t is_f_h_error = 0;
    uint8_t is_f_l_error = 0;
    uint8_t is_r_h_error = 0;
    uint8_t is_r_l_error = 0;

    for (int x = 0; x<6; x++)
    {
        puserdata->gpio_pin[x][0] = rt_pin_get(puserdata->gpio_devname[x][0]);
        puserdata->gpio_pin[x][1] = rt_pin_get(puserdata->gpio_devname[x][1]);

        //正向 低
        rt_pin_mode(puserdata->gpio_pin[x][0], PIN_MODE_OUTPUT);
        rt_pin_write(puserdata->gpio_pin[x][0], PIN_LOW);

        rt_pin_mode(puserdata->gpio_pin[x][1], PIN_MODE_OUTPUT);
        rt_pin_write(puserdata->gpio_pin[x][1], PIN_HIGH);
        rt_thread_delay(1);
        rt_pin_mode(puserdata->gpio_pin[x][1], PIN_MODE_INPUT);
        if (rt_pin_read(puserdata->gpio_pin[x][1]) != PIN_LOW)
        {
            LOG_E("%s LOW  error!", error_log1[x]);
            is_f_l_error++;
        }
        rt_pin_mode(puserdata->gpio_pin[x][0], PIN_MODE_INPUT);

        //正向 高
        rt_pin_mode(puserdata->gpio_pin[x][0], PIN_MODE_OUTPUT);
        rt_pin_write(puserdata->gpio_pin[x][0], PIN_HIGH);

        rt_pin_mode(puserdata->gpio_pin[x][1], PIN_MODE_OUTPUT);
        rt_pin_write(puserdata->gpio_pin[x][1], PIN_LOW);
        rt_thread_delay(1);
        rt_pin_mode(puserdata->gpio_pin[x][1], PIN_MODE_INPUT);
        if (rt_pin_read(puserdata->gpio_pin[x][1]) != PIN_HIGH)
        {
            LOG_E("%s HIGH error!", error_log1[x]);
            is_f_h_error++;
        }
        rt_pin_mode(puserdata->gpio_pin[x][0], PIN_MODE_INPUT);

        //反向 低
        rt_pin_mode(puserdata->gpio_pin[x][1], PIN_MODE_OUTPUT);
        rt_pin_write(puserdata->gpio_pin[x][1], PIN_LOW);

        rt_pin_mode(puserdata->gpio_pin[x][0], PIN_MODE_OUTPUT);
        rt_pin_write(puserdata->gpio_pin[x][0], PIN_HIGH);
        rt_thread_delay(1);
        rt_pin_mode(puserdata->gpio_pin[x][0], PIN_MODE_INPUT);
        if (rt_pin_read(puserdata->gpio_pin[x][0]) != PIN_LOW)
        {
            LOG_E("%s LOW  error!", error_log2[x]);
            is_r_l_error++;
        }
        rt_pin_mode(puserdata->gpio_pin[x][1], PIN_MODE_INPUT);

        //反向 高
        rt_pin_mode(puserdata->gpio_pin[x][1], PIN_MODE_OUTPUT);
        rt_pin_write(puserdata->gpio_pin[x][1], PIN_HIGH);

        rt_pin_mode(puserdata->gpio_pin[x][0], PIN_MODE_OUTPUT);
        rt_pin_write(puserdata->gpio_pin[x][0], PIN_LOW);
        rt_thread_delay(1);
        rt_pin_mode(puserdata->gpio_pin[x][0], PIN_MODE_INPUT);
        if (rt_pin_read(puserdata->gpio_pin[x][0]) != PIN_HIGH)
        {
            LOG_E("%s HIGH error!", error_log2[x]);
            is_r_h_error++;
        }
        rt_pin_mode(puserdata->gpio_pin[x][1], PIN_MODE_INPUT);
    }

    if (is_f_l_error == 0)
    {
        LOG_D("gpio f LOW  pass");
        puserdata->result[RESULT_GPIO_LOW_F].result = 0;
    }
    else
    {
        LOG_E("gpio f LOW  error!");
        puserdata->result[RESULT_GPIO_LOW_F].result = is_f_l_error;
    }

    if (is_f_h_error == 0)
    {
        LOG_D("gpio f HIGH pass");
        puserdata->result[RESULT_GPIO_HIGH_F].result = 0;
    }
    else
    {
        LOG_E("gpio f HIGH error!");
        puserdata->result[RESULT_GPIO_HIGH_F].result = is_f_h_error;
    }

    if (is_r_l_error == 0)
    {
        LOG_D("gpio r LOW  pass");
        puserdata->result[RESULT_GPIO_LOW_R].result = 0;
    }
    else
    {
        LOG_E("gpio r LOW  error!");
        puserdata->result[RESULT_GPIO_LOW_R].result = is_r_l_error;
    }

    if (is_r_h_error == 0)
    {
        LOG_D("gpio r HIGH pass");
        puserdata->result[RESULT_GPIO_HIGH_R].result = 0;
    }
    else
    {
        LOG_E("gpio r HIGH error!");
        puserdata->result[RESULT_GPIO_HIGH_R].result = is_r_h_error;
    }
}

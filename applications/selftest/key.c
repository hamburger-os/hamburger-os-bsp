/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-21     lvhan       the first version
 */

#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>

#include "selftest.h"

#define DBG_TAG "key "
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static void key_hz_test(rt_base_t pin, uint16_t hz, uint16_t s)
{
    uint32_t us = 1000000.0f/hz/2;
    uint32_t count = s * 1000000.0f / us / 2;

    for (uint32_t n = 0; n < count; n++)
    {
        rt_hw_us_delay(us);
        rt_pin_write(pin, PIN_HIGH);
        rt_hw_us_delay(us);
        rt_pin_write(pin, PIN_LOW);
    }
    rt_thread_delay(10);
}

void selftest_key_test(SelftestlUserData *puserdata)
{
    puserdata->key_pin = rt_pin_get(puserdata->key_devname);

    LOG_I("pwm voice run...");
    rt_pin_mode(puserdata->key_pin, PIN_MODE_OUTPUT);
    key_hz_test(puserdata->key_pin, 262, 1);
    key_hz_test(puserdata->key_pin, 294, 1);
    key_hz_test(puserdata->key_pin, 330, 1);
    key_hz_test(puserdata->key_pin, 349, 1);
    key_hz_test(puserdata->key_pin, 392, 1);
    key_hz_test(puserdata->key_pin, 440, 1);
    key_hz_test(puserdata->key_pin, 494, 1);
    rt_pin_mode(puserdata->key_pin, PIN_MODE_INPUT);
    LOG_I("pwm voice end");
}

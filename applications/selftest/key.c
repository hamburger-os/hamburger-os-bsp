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

static struct rt_device_pwm *pwm_dev;

//频率控制20~20000 hz
#define KEY_PWM_PERIOD 1000000000
static void output_frequency(SelftestUserData *puserdata, uint32_t hz)
{
    uint32_t cycle = KEY_PWM_PERIOD/hz;
    /* Convert nanosecond to frequency and duty cycle. 1s = 1 * 1000 * 1000 * 1000 ns */
    rt_pwm_set(pwm_dev, puserdata->key_channel, cycle, cycle/2);
}

void selftest_key_test(SelftestUserData *puserdata)
{
    pwm_dev = (struct rt_device_pwm *)rt_device_find(puserdata->key_devname);

    LOG_I("pwm voice run...");
    rt_pwm_enable(pwm_dev, puserdata->key_channel);
    output_frequency(puserdata, 262);
    rt_thread_delay(200);
    output_frequency(puserdata, 294);
    rt_thread_delay(200);
    output_frequency(puserdata, 330);
    rt_thread_delay(200);
    output_frequency(puserdata, 349);
    rt_thread_delay(200);
    output_frequency(puserdata, 392);
    rt_thread_delay(200);
    rt_pwm_disable(pwm_dev, puserdata->key_channel);
    LOG_I("pwm voice end");
}

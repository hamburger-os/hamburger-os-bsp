/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-01-05     lvhan       the first version
 */

#include "board.h"

#include "tpx86_voice_ctrl.h"

#define DBG_TAG "tpx86-led"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static void led_thread_entry(void *parameter)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)parameter;

    puserdata->led_pin[LED_CTL1] = rt_pin_get(puserdata->led_devname[LED_CTL1]);
    rt_pin_mode(puserdata->led_pin[LED_CTL1], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->led_pin[LED_CTL1], PIN_HIGH);

    puserdata->led_pin[LED_CTL2] = rt_pin_get(puserdata->led_devname[LED_CTL2]);
    rt_pin_mode(puserdata->led_pin[LED_CTL2], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->led_pin[LED_CTL2], PIN_HIGH);

    puserdata->led_pin[LED_CTL3] = rt_pin_get(puserdata->led_devname[LED_CTL3]);
    rt_pin_mode(puserdata->led_pin[LED_CTL3], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->led_pin[LED_CTL3], PIN_HIGH);

    puserdata->led_pin[LED_CTL4] = rt_pin_get(puserdata->led_devname[LED_CTL4]);
    rt_pin_mode(puserdata->led_pin[LED_CTL4], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->led_pin[LED_CTL4], PIN_HIGH);

    LOG_I("led thread startup...");
    while(puserdata->isThreadRun)
    {
        rt_thread_delay(500);
        rt_pin_write(puserdata->led_pin[LED_CTL1], !rt_pin_read(puserdata->led_pin[LED_CTL1]));
        rt_pin_write(puserdata->led_pin[LED_CTL2], !rt_pin_read(puserdata->led_pin[LED_CTL2]));
        rt_pin_write(puserdata->led_pin[LED_CTL3], !rt_pin_read(puserdata->led_pin[LED_CTL3]));
        rt_pin_write(puserdata->led_pin[LED_CTL4], !rt_pin_read(puserdata->led_pin[LED_CTL4]));
    }
}

int tpx86_voice_ctrl_led_init(void)
{
    /* 创建 led 线程 */
    rt_thread_t thread = rt_thread_create("led", led_thread_entry, &voice_ctrl_userdata, 2048, 30, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E("thread startup error!");
    }

    return RT_EOK;
}

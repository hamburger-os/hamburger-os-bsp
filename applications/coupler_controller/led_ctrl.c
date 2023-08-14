/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-26     lvhan       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include "coupler_controller.h"

#define DBG_TAG "led"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static void led_thread_entry(void *parameter)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)parameter;

    puserdata->led_pin[LED_RUN] = rt_pin_get(puserdata->led_devname[LED_RUN]);
    rt_pin_mode(puserdata->led_pin[LED_RUN], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->led_pin[LED_RUN], PIN_HIGH);

    puserdata->led_pin[LED_CAN2] = rt_pin_get(puserdata->led_devname[LED_CAN2]);
    rt_pin_mode(puserdata->led_pin[LED_CAN2], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->led_pin[LED_CAN2], PIN_HIGH);

    puserdata->led_pin[LED_CAN1] = rt_pin_get(puserdata->led_devname[LED_CAN1]);
    rt_pin_mode(puserdata->led_pin[LED_CAN1], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->led_pin[LED_CAN1], PIN_HIGH);

    puserdata->led_pin[LED_485] = rt_pin_get(puserdata->led_devname[LED_485]);
    rt_pin_mode(puserdata->led_pin[LED_485], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->led_pin[LED_485], PIN_HIGH);

    puserdata->led_pin[LED_DIO] = rt_pin_get(puserdata->led_devname[LED_DIO]);
    rt_pin_mode(puserdata->led_pin[LED_DIO], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->led_pin[LED_DIO], PIN_HIGH);

    LOG_I("led thread startup...");
    while(puserdata->isThreadRun)
    {
        rt_thread_delay(500);
        coupler_controller_led_toggle(LED_RUN);
//        rt_pin_write(puserdata->led_pin[LED_RUN], !rt_pin_read(puserdata->led_pin[LED_RUN]));
//        rt_pin_write(puserdata->led_pin[LED_CAN2], !rt_pin_read(puserdata->led_pin[LED_CAN2]));
//        rt_pin_write(puserdata->led_pin[LED_CAN1], !rt_pin_read(puserdata->led_pin[LED_CAN1]));
//        rt_pin_write(puserdata->led_pin[LED_485], !rt_pin_read(puserdata->led_pin[LED_485]));
//        rt_pin_write(puserdata->led_pin[LED_DIO], !rt_pin_read(puserdata->led_pin[LED_DIO]));
    }
    rt_pin_write(puserdata->led_pin[LED_RUN], PIN_HIGH);
}

void coupler_controller_led_toggle(int pin)
{
    if (pin >= 0 && pin < 5)
        rt_pin_write(coupler_controller_userdata.led_pin[pin], !rt_pin_read(coupler_controller_userdata.led_pin[pin]));
}

void coupler_controller_ledinit(void)
{
    /* 创建 led 线程 */
    rt_thread_t thread = rt_thread_create("led", led_thread_entry, &coupler_controller_userdata, 2048, 30, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E("thread startup error!");
    }
}

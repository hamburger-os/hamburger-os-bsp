/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-05     lvhan       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include "coupler_controller.h"

#define DBG_TAG "bat"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

enum {
    BAT_DI = 0,
    BAT_EN,
};

static void bat_thread_entry(void *parameter)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)parameter;

    puserdata->bat_pin[BAT_DI] = rt_pin_get(puserdata->bat_devname[BAT_DI]);
    rt_pin_mode(puserdata->bat_pin[BAT_DI], PIN_MODE_INPUT);

    puserdata->bat_pin[BAT_EN] = rt_pin_get(puserdata->bat_devname[BAT_EN]);
    rt_pin_mode(puserdata->bat_pin[BAT_EN], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->bat_pin[BAT_EN], PIN_LOW);

    while(puserdata->isThreadRun)
    {
        rt_thread_delay(100);
        if (rt_pin_read(puserdata->bat_pin[BAT_DI]) == PIN_LOW)
        {
            rt_thread_delay(10);
            if (rt_pin_read(puserdata->bat_pin[BAT_DI]) == PIN_LOW)
            {
                rt_pin_write(puserdata->bat_pin[BAT_EN], PIN_HIGH);
                LOG_D("bat enable");
                puserdata->isThreadRun = 0;//关闭应用线程
                rt_thread_delay(60 * 1000);
                rt_pin_write(puserdata->bat_pin[BAT_EN], PIN_LOW);
                LOG_D("bat disable");
                rt_thread_delay(60 * 1000);
            }
        }
    }
}

void coupler_controller_batinit(void)
{
    /* 创建 bat 线程 */
    rt_thread_t thread = rt_thread_create("bat", bat_thread_entry, &coupler_controller_userdata, 2048, 21, 10);
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

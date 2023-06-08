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

#define DBG_TAG "pressure"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static void pressure_thread_entry(void *parameter)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)parameter;

    /* 使能adc设备 */
    puserdata->adc_dev = (rt_adc_device_t)rt_device_find(puserdata->adc_devname);
    if (puserdata->adc_dev == RT_NULL)
    {
        LOG_E("can't find %s device!", puserdata->adc_devname);
    }
    rt_adc_enable(puserdata->adc_dev, 0);
    rt_adc_enable(puserdata->adc_dev, 1);
    LOG_I("pressure adc startup...");

    while(puserdata->isThreadRun)
    {
        rt_thread_delay(10);
        puserdata->adc[0] = rt_adc_voltage(puserdata->adc_dev, 0);
        puserdata->adc[1] = rt_adc_voltage(puserdata->adc_dev, 1);
    }

    /* 关闭通道 */
    rt_adc_disable(puserdata->adc_dev, 0);
    rt_adc_disable(puserdata->adc_dev, 1);
}

void coupler_controller_pressureinit(void)
{
    /* 创建 读值 线程 */
    rt_thread_t thread = rt_thread_create("pressure", pressure_thread_entry, &coupler_controller_userdata, 2048, 21, 10);
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

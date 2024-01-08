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

#define DBG_TAG "tpx86-adc"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static void adc_thread_entry(void *parameter)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)parameter;

    puserdata->adc_dev = (rt_adc_device_t)rt_device_find(puserdata->adc_devname);
    if (puserdata->adc_dev == RT_NULL)
    {
        LOG_E("can't find %s device!", puserdata->adc_devname);
        return;
    }

    LOG_I("adc thread startup...");
    while(puserdata->isThreadRun)
    {
        rt_thread_delay(1000);

        /* 使能设备 */
        rt_adc_enable(puserdata->adc_dev, puserdata->adc_channel);
        /* 读取采样值 */
        puserdata->adc_value = rt_adc_read(puserdata->adc_dev, puserdata->adc_channel);
        /* 关闭通道 */
        rt_adc_disable(puserdata->adc_dev, puserdata->adc_channel);

        LOG_D("value  : %d", puserdata->adc_value);
    }
}

int tpx86_voice_ctrl_adc_init(void)
{
    /* 创建 adc 线程 */
    rt_thread_t thread = rt_thread_create("adc", adc_thread_entry, &voice_ctrl_userdata, 2048, 27, 10);
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

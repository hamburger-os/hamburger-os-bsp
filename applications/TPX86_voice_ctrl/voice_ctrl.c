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

#define DBG_TAG "tpx86-voice"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static void voice_thread_entry(void *parameter)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)parameter;

    puserdata->voice_pin[VOICE_CT1] = rt_pin_get(puserdata->voice_devname[VOICE_CT1]);
    rt_pin_mode(puserdata->voice_pin[VOICE_CT1], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->voice_pin[VOICE_CT1], PIN_LOW);

    puserdata->voice_pin[VOICE_CT2] = rt_pin_get(puserdata->voice_devname[VOICE_CT2]);
    rt_pin_mode(puserdata->voice_pin[VOICE_CT2], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->voice_pin[VOICE_CT2], PIN_LOW);

    puserdata->voice_pin[VOICE_CT3] = rt_pin_get(puserdata->voice_devname[VOICE_CT3]);
    rt_pin_mode(puserdata->voice_pin[VOICE_CT3], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->voice_pin[VOICE_CT3], PIN_LOW);

    puserdata->voice_pin[VOICE_ADD] = rt_pin_get(puserdata->voice_devname[VOICE_ADD]);
    rt_pin_mode(puserdata->voice_pin[VOICE_ADD], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->voice_pin[VOICE_ADD], PIN_HIGH);

    puserdata->voice_pin[VOICE_SUB] = rt_pin_get(puserdata->voice_devname[VOICE_SUB]);
    rt_pin_mode(puserdata->voice_pin[VOICE_SUB], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->voice_pin[VOICE_SUB], PIN_HIGH);

    LOG_I("voice thread startup...");
    while(puserdata->isThreadRun)
    {
        rt_thread_delay(500);
//        rt_pin_write(puserdata->voice_pin[VOICE_CT1], !rt_pin_read(puserdata->voice_pin[VOICE_CT1]));
//        rt_pin_write(puserdata->voice_pin[VOICE_CT2], !rt_pin_read(puserdata->voice_pin[VOICE_CT2]));
//        rt_pin_write(puserdata->voice_pin[VOICE_CT3], !rt_pin_read(puserdata->voice_pin[VOICE_CT3]));
        rt_pin_write(puserdata->voice_pin[VOICE_ADD], !rt_pin_read(puserdata->voice_pin[VOICE_ADD]));
        rt_pin_write(puserdata->voice_pin[VOICE_SUB], !rt_pin_read(puserdata->voice_pin[VOICE_SUB]));
    }
}

int tpx86_voice_ctrl_voice_init(void)
{
    /* 创建 voice 线程 */
    rt_thread_t thread = rt_thread_create("voice", voice_thread_entry, &voice_ctrl_userdata, 2048, 28, 10);
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

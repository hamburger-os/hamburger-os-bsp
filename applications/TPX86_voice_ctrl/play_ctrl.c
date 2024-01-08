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

#define DBG_TAG "tpx86-play"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static void play_thread_entry(void *parameter)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)parameter;

    puserdata->play_pin[PLAY_CTL0] = rt_pin_get(puserdata->play_devname[PLAY_CTL0]);
    rt_pin_mode(puserdata->play_pin[PLAY_CTL0], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->play_pin[PLAY_CTL0], PIN_HIGH);

    puserdata->play_pin[PLAY_CTL1] = rt_pin_get(puserdata->play_devname[PLAY_CTL1]);
    rt_pin_mode(puserdata->play_pin[PLAY_CTL1], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->play_pin[PLAY_CTL1], PIN_HIGH);

    LOG_I("play thread startup...");
    while(puserdata->isThreadRun)
    {
        rt_thread_delay(500);
        rt_pin_write(puserdata->play_pin[PLAY_CTL0], !rt_pin_read(puserdata->play_pin[PLAY_CTL0]));
        rt_pin_write(puserdata->play_pin[PLAY_CTL1], !rt_pin_read(puserdata->play_pin[PLAY_CTL1]));
    }
}

int tpx86_voice_ctrl_play_init(void)
{
    /* 创建 play 线程 */
    rt_thread_t thread = rt_thread_create("play", play_thread_entry, &voice_ctrl_userdata, 2048, 29, 10);
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

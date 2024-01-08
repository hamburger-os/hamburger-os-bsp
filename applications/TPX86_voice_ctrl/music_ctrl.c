/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-01-08     lvhan       the first version
 */

#include "board.h"

#include "tpx86_voice_ctrl.h"

#include "wavplayer.h"

#define DBG_TAG "tpx86-music"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static void music_thread_entry(void *parameter)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)parameter;

    uint8_t volume = 0;
    LOG_I("music thread startup...");
    while(puserdata->isThreadRun)
    {
        volume += 20;
        if (volume > 100)
            volume = 20;
        wavplayer_volume_set(volume);
        wavplayer_play(puserdata->wav_path);

        while(wavplayer_state_get() != 0)
        {
            rt_thread_delay(100);
        }
        
        rt_thread_delay(1000);
    }
}

int tpx86_music_ctrl_music_init(void)
{
    /* 创建 music 线程 */
    rt_thread_t thread = rt_thread_create("music", music_thread_entry, &voice_ctrl_userdata, 2048, 28, 10);
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

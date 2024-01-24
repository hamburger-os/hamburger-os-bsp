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

void tpx86_voice_ctrl_play_press(int key)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)&voice_ctrl_userdata;
    switch(key)
    {
    case PLAY_CTL0:
    case PLAY_CTL1:
        rt_pin_write(puserdata->play_pin[key], PIN_LOW);
        break;
    default:
        break;
    }
}

void tpx86_voice_ctrl_play_release(int key)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)&voice_ctrl_userdata;
    switch(key)
    {
    case PLAY_CTL0:
    case PLAY_CTL1:
        rt_pin_write(puserdata->play_pin[key], PIN_HIGH);
        break;
    default:
        break;
    }
}

int tpx86_voice_ctrl_play_init(void)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)&voice_ctrl_userdata;

    puserdata->play_pin[PLAY_CTL0] = rt_pin_get(puserdata->play_devname[PLAY_CTL0]);
    rt_pin_mode(puserdata->play_pin[PLAY_CTL0], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->play_pin[PLAY_CTL0], PIN_HIGH);

    puserdata->play_pin[PLAY_CTL1] = rt_pin_get(puserdata->play_devname[PLAY_CTL1]);
    rt_pin_mode(puserdata->play_pin[PLAY_CTL1], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->play_pin[PLAY_CTL1], PIN_HIGH);

    return RT_EOK;
}

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

void tpx86_voice_ctrl_voice_press(int key)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)&voice_ctrl_userdata;
    switch(key)
    {
    case VOICE_CT1:
    case VOICE_CT2:
    case VOICE_CT3:
        rt_pin_write(puserdata->voice_pin[key], PIN_LOW);
        break;
    default:
        break;
    }
}

void tpx86_voice_ctrl_voice_release(int key)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)&voice_ctrl_userdata;
    switch(key)
    {
    case VOICE_CT1:
    case VOICE_CT2:
    case VOICE_CT3:
        rt_pin_write(puserdata->voice_pin[key], PIN_HIGH);
        break;
    default:
        break;
    }
}

int tpx86_voice_ctrl_voice_init(void)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)&voice_ctrl_userdata;

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

    return RT_EOK;
}

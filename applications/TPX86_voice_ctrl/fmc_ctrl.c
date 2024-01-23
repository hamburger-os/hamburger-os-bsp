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

#define DBG_TAG "tpx86-fmc"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static void fmc_thread_entry(void *parameter)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)parameter;

    LOG_I("fmc thread startup...");
    while(puserdata->isThreadRun)
    {
        rt_thread_delay(1000);

        puserdata->NSBankValue[0] = *(FMC_ValueDef *)puserdata->NSBankAddress[0];
        puserdata->NSBankValue[1] = *(FMC_ValueDef *)puserdata->NSBankAddress[1];

        if (puserdata->NSBankValue[1].key0 == 0)
        {
            LOG_I("key0 press");
        }
        if (puserdata->NSBankValue[1].key1 == 0)
        {
            LOG_I("key1 press");
        }
        if (puserdata->NSBankValue[1].key2 == 0)
        {
            LOG_I("key2 press");
        }
        if (puserdata->NSBankValue[1].key3 == 0)
        {
            LOG_I("key3 press");
        }
        if (puserdata->NSBankValue[1].key4 == 0)
        {
            LOG_I("key4 press");
        }

        LOG_D("value  : 0x%x 0x%x", puserdata->NSBankValue[0].value, puserdata->NSBankValue[1].value);
    }
}

int tpx86_voice_ctrl_fmc_init(void)
{
    /* 创建 fmc 线程 */
    rt_thread_t thread = rt_thread_create("fmc", fmc_thread_entry, &voice_ctrl_userdata, 2048, 26, 10);
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

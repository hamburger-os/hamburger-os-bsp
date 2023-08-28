/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-17     lvhan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "selftest.h"

#include "wavplayer.h"

#define DBG_TAG "i2s "
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

void selftest_i2s_test(SelftestlUserData *puserdata)
{
    wavplayer_play(puserdata->wav_path);
    rt_thread_delay(6 * 1000);
}

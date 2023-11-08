/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_timer.h"

#define DBG_TAG "if_timer"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

BOOL if_timer_init(void)
{
    return TRUE;
}

uint32 if_timer_getTicks(void)
{
    return rt_tick_get();
}


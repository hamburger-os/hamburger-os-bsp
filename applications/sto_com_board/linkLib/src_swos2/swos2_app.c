/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_app.h"

#define DBG_TAG "if_app"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include "task_loop.h"

S_APP_ARCH_IF g_appArchIf;

BOOL app_archRunning( p_init appArchIf_init )
{
    appArchIf_init();

    if(TaskInit() < 0)
    {
        LOG_E("TaskInit Error");
        return FALSE;
    }

    LOG_I("init ok");

    return TRUE;
}


extern void test_main_init( void );

void app_archInit( void )
{
    support_osRunning( test_main_init );
}




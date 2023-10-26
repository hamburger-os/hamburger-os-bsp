/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-14     zm       the first version
 */
#include "task_loop.h"
#include "if_os.h"
#include "if_app.h"

#define DBG_TAG "loop"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define TASK_HIGH_THREAD_PRIORITY         15
#define TASK_HIGH_THREAD_STACK_SIZE       (1024 * 10)

#define TASK_LOW_THREAD_PRIORITY         20
#define TASK_LOW_THREAD_STACK_SIZE       (1024 * 10)

static void *TaskHighThreadEntry(void *parameter)
{
    while (1)
    {
        if (NULL != g_appArchIf.app_polling_proc)
        {
            g_appArchIf.app_polling_proc();
        }
        if_OSTimeDly(10);
    }
}

static sint32 TaskHighThreadInit(void)
{
    if (if_OSTaskCreate("task high", TaskHighThreadEntry, NULL,
    TASK_HIGH_THREAD_STACK_SIZE,
    TASK_HIGH_THREAD_PRIORITY) < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

static void *TaskLowThreadEntry(void *parameter)
{
    while (1)
    {
        if (NULL != g_appArchIf.app_idle_proc)
        {
            g_appArchIf.app_idle_proc();
        }
        if_OSTimeDly(10);
    }
}

static sint32 TaskLowThreadInit(void)
{
    if (if_OSTaskCreate("task low", TaskLowThreadEntry, NULL,
    TASK_LOW_THREAD_STACK_SIZE,
    TASK_LOW_THREAD_PRIORITY) < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

sint32 TaskInit(void)
{

    if (NULL != g_appArchIf.app_init)
    {
        g_appArchIf.app_init();
    }
    else
    {
        LOG_E("app_init is null");
    }

    if (TaskHighThreadInit() < 0)
    {
        LOG_E("task high thread init error");
        return -1;
    }

    if (TaskLowThreadInit() < 0)
    {
        LOG_E("task low thread init error");
        return -1;
    }
    return 0;
}


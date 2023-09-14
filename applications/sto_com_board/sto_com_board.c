/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-31     zm       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG "STOComBoard"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "if_os.h"
#include "linklib/inc/if_app.h"

#define BOARD_INIT_THREAD_PRIORITY         20
#define BOARD_INIT_THREAD_STACK_SIZE       (1024)

static void *BoardInitThreadEntry(void *parameter)
{
    if(TaskInit() < 0)
    {
        LOG_E("TaskInit Error");
    }

    LOG_I("init ok");
}

static int BoardInitThreadInit(void)
{
    if(if_OSTaskCreate("board_init",
            BoardInitThreadEntry, NULL,
            BOARD_INIT_THREAD_STACK_SIZE,
            BOARD_INIT_THREAD_PRIORITY) < 0)
    {
        return -RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }
}

static void STOComBoardInit(void)
{
#if 0
    BoardInitThreadInit();
#else
    app_archInit();
#endif
}

INIT_APP_EXPORT(STOComBoardInit);

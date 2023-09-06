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

#include "hal_os/hal_os.h"

#define BOARD_INIT_THREAD_PRIORITY         20
#define BOARD_INIT_THREAD_STACK_SIZE       (1024)

static void *BoardInitThreadEntry(void *parameter)
{
    LOG_I("init ok");
    HALOSTestThreadInit();
    while(1)
    {
        rt_thread_mdelay(10);
    }
}

static int BoardInitThreadInit(void)
{
    if(hal_os_creat_task("board_init",
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
    BoardInitThreadInit();
}

INIT_APP_EXPORT(STOComBoardInit);

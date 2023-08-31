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

#define DBG_TAG "STOCom2LoardBoard"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define BOARD_INIT_THREAD_PRIORITY         20
#define BOARD_INIT_THREAD_STACK_SIZE       (1024)
#define BOARD_INIT_THREAD_TIMESLICE        5

static void *BoardInitThreadEntry(void *parameter)
{
    LOG_I("init ok");
    while(1)
    {
        rt_thread_mdelay(10);
    }
}

static int BoardInitThreadInit(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("board_init",
                            BoardInitThreadEntry, RT_NULL,
                            BOARD_INIT_THREAD_STACK_SIZE,
                            BOARD_INIT_THREAD_PRIORITY, BOARD_INIT_THREAD_TIMESLICE);

    if(tid != NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    return -RT_ERROR;
}

static void STOCom2LoardBoardInit(void)
{
    BoardInitThreadInit();
}

INIT_APP_EXPORT(STOCom2LoardBoardInit);

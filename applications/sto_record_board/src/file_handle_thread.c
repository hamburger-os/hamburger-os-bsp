/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-20     zm       the first version
 */

#include "file_handle_thread.h"

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#include "sto_record_board.h"
#include "data_handle.h"
#include "Record_FileCreate.h"

#define DBG_TAG "FileThread"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define FILE_THREAD_PRIORITY         15//19
#define FILE_THREAD_STACK_SIZE       (2048 * 2)
#define FILE_THREAD_TIMESLICE        5

static void *FileThreadEntry(void *parameter)
{
    rt_err_t ret = RT_EOK;

    while(1)
    {
        CanDataHandle(&eth_can_data_handle);
        RecordBoard_FileCreate();
        rt_thread_mdelay(1);
    }
}

int FileThreadInit(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("file_thread",
                            FileThreadEntry, RT_NULL,
                            FILE_THREAD_STACK_SIZE,
                            FILE_THREAD_PRIORITY, FILE_THREAD_TIMESLICE);

    if(tid != NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    return -RT_ERROR;
}



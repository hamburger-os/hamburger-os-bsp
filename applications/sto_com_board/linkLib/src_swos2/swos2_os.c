/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_os.h"

#define DBG_TAG "if_os"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

sint32 if_OSSemCreate(OS_EVENT *psem)
{
    if (NULL == psem)
    {
        return -1;
    }

    psem->sem = (void *) rt_sem_create("if_sem", 0, RT_IPC_FLAG_PRIO);
    if (NULL == psem->sem)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

void if_OSSemPend(OS_EVENT *psem, sint32 timeout, sint8 *perr)
{
    if (NULL == psem || NULL == psem->sem || NULL == perr)
    {
        *perr = -1;
        return;
    }

    if (rt_sem_take(psem->sem, timeout) != RT_EOK)
    {
        *perr = -1;
    }
    else
    {
        *perr = 1;
    }
}

void if_OSSemPost(OS_EVENT *psem)
{
    if (NULL == psem || NULL == psem->sem)
    {
        return;
    }

    rt_sem_release(psem->sem);
}

sint32 if_OSTaskCreate(const char *task_name, void (*entry)(void *param), void *param, uint32 stack_size,
        uint8 priority)
{
    rt_thread_t tid;

    if (priority < 0 || priority > 32)
    {
        return -1;
    }

    if (NULL == task_name)
    {
        return -1;
    }

    if (NULL == entry)
    {
        return -1;
    }

    tid = rt_thread_create(task_name, entry, param, stack_size, priority, HAL_OS_THREAD_TIMESLICE);

    if (tid != NULL)
    {
        rt_thread_startup(tid);
        return 1;
    }
    else
    {
        return -1;
    }
}

void if_OSTimeDly(uint32 ticks)
{
    rt_thread_mdelay(ticks);
}

/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-05     zm       the first version
 */
#include "hal_os.h"

#define DBG_TAG "hal_os"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

sint8 hal_os_creat_task(const char *task_name,
                        void (*entry)(void *param),
                        void *param,
                        uint32 stack_size,
                        uint8 priority)
{
    rt_thread_t tid;

    if(priority < 0 || priority > 32)
    {
        return -1;
    }

    if(NULL == task_name)
    {
        return -1;
    }

    if(NULL == entry)
    {
        return -1;
    }

    tid = rt_thread_create(task_name,
                            entry, param,
                            stack_size,
                            priority, HAL_OS_THREAD_TIMESLICE);

    if(tid != NULL)
    {
        rt_thread_startup(tid);
        return 1;
    }
    else
    {
        return -1;
    }
}


sint8 hal_os_sem_creat(S_HLA_OS_SEM *sem, const char *name, uint32 value)
{
    if(NULL == sem)
    {
        return -1;
    }

    sem->sem = (void *)rt_sem_create(name, value, RT_IPC_FLAG_PRIO);
    if(NULL == sem->sem)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

sint8 hal_os_sem_delete(S_HLA_OS_SEM *sem)
{
    rt_err_t ret = -RT_ERROR;

    if(NULL == sem)
    {
        return -1;
    }

    if(NULL == sem->sem)
    {
        return -1;
    }

    if(rt_sem_delete((rt_sem_t *)sem->sem) != RT_EOK)
    {
        return -1;
    }
    else
    {
        sem->sem = NULL;
        return 1;
    }
}

sint8 hal_os_sem_take(S_HLA_OS_SEM *sem, sint32 timeout)
{
    if(NULL == sem)
    {
        return -1;
    }

    if(NULL == sem->sem)
    {
        return -1;
    }


    if(rt_sem_take(sem->sem, timeout) != RT_EOK)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

sint8 hal_os_sem_release(S_HLA_OS_SEM *sem)
{
    if(NULL == sem)
    {
        return -1;
    }

    if(NULL == sem->sem)
    {
        return -1;
    }

    if(rt_sem_release(sem->sem) != RT_EOK)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

sint8 hal_os_mq_creat(S_HAL_OS_MQ *mq, const char *name, uint32 mq_size, uint32 max_mq_num)
{
    if(NULL == mq)
    {
        return -1;
    }

    mq->mq = rt_mq_create(name, mq_size, max_mq_num, RT_IPC_FLAG_PRIO);
    if(NULL == mq->mq)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

sint8 hal_os_mq_delete(S_HAL_OS_MQ *mq)
{
    if(NULL == mq)
    {
        return -1;
    }

    if(NULL == mq->mq)
    {
        return -1;
    }

    if(rt_mq_delete(mq->mq) < 0)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

sint8 hal_os_mq_send(S_HAL_OS_MQ *mq, const void *buffer, uint32 size, uint32 timeout)
{
    rt_err_t ret = -RT_ERROR;

    if(NULL == mq)
    {
        return -1;
    }

    if(NULL == buffer)
    {
        return -1;
    }

    if(size <= 0)
    {
        return -1;
    }

    if(timeout < 0)
    {
        return -1;
    }

    if(0 == timeout)
    {
        ret = rt_mq_send(mq->mq, buffer, size);
    }
    else
    {
        ret = rt_mq_send_wait(mq->mq, buffer, size, timeout);
    }

    if(ret != RT_EOK)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

sint8 hal_os_mq_recv(S_HAL_OS_MQ *mq, void *buffer, uint32 size, uint32 timeout)
{
    if(NULL == mq)
    {
        return -1;
    }

    if(NULL == buffer)
    {
        return -1;
    }

    if(size <= 0)
    {
        return -1;
    }

    if(rt_mq_recv(mq->mq, buffer, size, timeout) != RT_EOK)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

void hal_os_printf(const char *fmt, ...)
{
    rt_kprintf(fmt);
}

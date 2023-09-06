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

#define DBG_TAG "hal_os test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static S_HLA_OS_SEM dynamic_sem;
static S_HAL_OS_MQ mq_test;

static void *HALOSTest1ThreadEntry(void *parameter)
{
    static uint8 count = 0;
    while(1)
    {
        if(count <= 100)
        {
            count++;
        }
        else
        {
            count = 0;
        }

        /* count 每计数 10 次，就释放一次信号量 */
        if(0 == (count % 10))
        {
            rt_kprintf("release a dynamic semaphore.\n");
            if(hal_os_sem_release(&dynamic_sem) < 0)
            {
                LOG_E("hal_os_sem_release error");
            }
            else
            {
                LOG_I("hal_os_sem_release ok");
            }
        }

        rt_thread_mdelay(10);
    }
}

static void *HALOSTest2ThreadEntry(void *parameter)
{
    uint32 number = 0;

    while(1)
    {
        if(hal_os_sem_take(&dynamic_sem, HAL_OS_WAITING_FOREVER) < 0)
        {
            LOG_E("hal_os_sem_take error");
        }
        else
        {
            LOG_I("number %d", number);
            number++;
        }

        rt_thread_mdelay(10);
    }
}

static void *HALOSMQTest1ThreadEntry(void *parameter)
{
    uint32 number = 0;

    while(1)
    {
        if(hal_os_mq_recv(&mq_test, &number, sizeof(uint32), HAL_OS_WAITING_FOREVER) > 0)
        {
            LOG_I("mq number %d", number);
        }

        rt_thread_mdelay(10);
    }
}

static void *HALOSMQTest2ThreadEntry(void *parameter)
{
    uint32 number = 0;

    while(1)
    {
        if(hal_os_mq_send(&mq_test, &number, sizeof(uint32), 0) < 0)
        {
            LOG_E("hal_os_mq_send error");
        }

        hal_os_printf("abc %d\r\n", 1);
        rt_thread_mdelay(100);
        number++;
    }
}

int HALOSTestThreadInit(void)
{
    if(hal_os_sem_creat(&dynamic_sem, "dynamic_sem", 0) < 0)
    {
        LOG_E("sem creat error");
        return -1;
    }

    if(hal_os_mq_creat(&mq_test, "mq test", sizeof(uint8), 10) < 0)
    {
        LOG_E("mq creat error");
        return -1;
    }

    if(hal_os_creat_task("hal_os test 1",
            HALOSTest1ThreadEntry, NULL,
            1024,
            19) < 0)
    {
        return -1;
    }

    if(hal_os_creat_task("hal_os test 2",
            HALOSTest2ThreadEntry, NULL,
            1024,
            20) < 0)
    {
        return -1;
    }

    if(hal_os_creat_task("hal_os mq 1",
            HALOSMQTest1ThreadEntry, NULL,
            1024,
            19) < 0)
    {
        return -1;
    }

    if(hal_os_creat_task("hal_os mq 2",
            HALOSMQTest2ThreadEntry, NULL,
            1024,
            19) < 0)
    {
        return -1;
    }
    return 0;
}

void hal_os_delay_ms(uint32 ms)
{
    rt_thread_mdelay(ms);
}




/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-31     lvhan       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include "selftest.h"

#define DBG_TAG "selftest"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static int selftest_init(void)
{

    LOG_I("startup...");
    return RT_EOK;
}

#if 0 //如果使用dl动态模块方式
int main(int argc, char *argv[])
{
    selftest_init();

    while(1)
    {
        rt_thread_delay(1000);
    }
    return 0;
}
#else
INIT_APP_EXPORT(selftest_init);
#endif

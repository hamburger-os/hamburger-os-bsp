/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-01     lvhan       the first version
 *
 * RT-Thread MicroPython C 绑定代码自动生成器：
 * https://summergift.github.io/RT-MicroPython-Generator/
 */
#include "board.h"

#include <unistd.h>

#define DBG_TAG "mpy"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define THREAD_STACK_NO_SYNC   4096
#define THREAD_STACK_WITH_SYNC 8192

extern void mpy_main(const char *filename);

#if defined MPY_PORT_USING_EXEC_DEFAULT || defined MPY_PORT_USING_SELF_STARTING
static void mpy_thread_entry(void* parameter)
{
#ifdef MPY_PORT_USING_EXEC_DEFAULT
    chdir(MPY_PORT_WORKING_DIR);
    mpy_main(MPY_PORT_DEFAULT_SCRIPT);
#endif

#ifdef MPY_PORT_USING_SELF_STARTING
    while(1)
    {
        /* 切换工作地址 */
        chdir(MPY_PORT_WORKING_DIR);
        /* 在这里让程序进入循环，通过这种方式实现 MicroPython 的软复位*/
         /* 启动 MicroPython */
        mpy_main(NULL);
    }
#endif
}
#endif

static int mpy_init(void)
{
#if defined MPY_PORT_USING_EXEC_DEFAULT || defined MPY_PORT_USING_SELF_STARTING
    int stack_size_check;
#if defined(MICROPYTHON_USING_FILE_SYNC_VIA_IDE)
    stack_size_check = THREAD_STACK_WITH_SYNC;
#else
    stack_size_check = THREAD_STACK_NO_SYNC;
#endif

    rt_thread_t mpy_thread = rt_thread_create( "mpy",
                                            mpy_thread_entry,
                                            NULL,
                                            stack_size_check,
                                            14,
                                            10);
    if ( mpy_thread != RT_NULL)
    {
        rt_thread_startup(mpy_thread);
    }
#endif

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_SERVICE_EXPORT(mpy_init);

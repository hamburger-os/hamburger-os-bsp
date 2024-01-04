/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: MIT
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-18     Meco Man     the first version
 * 2022-05-10     Meco Man     improve rt-thread initialization process
 */

#ifdef __RTTHREAD__

#include <lvgl.h>
#include <rtthread.h>

#define DBG_TAG    "LVGL"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

#ifndef PKG_LVGL_THREAD_STACK_SIZE
#define PKG_LVGL_THREAD_STACK_SIZE 4096
#endif /* PKG_LVGL_THREAD_STACK_SIZE */

#ifndef PKG_LVGL_THREAD_PRIO
#define PKG_LVGL_THREAD_PRIO (RT_THREAD_PRIORITY_MAX*2/3)
#endif /* PKG_LVGL_THREAD_PRIO */

extern void lv_port_disp_init(void);
extern void lv_port_indev_init(void);
extern void lv_user_gui_init(void);

static struct rt_mutex lvgl_mutex;
static rt_thread_t lvgl_thread;

#ifdef rt_align
rt_align(RT_ALIGN_SIZE)
#else
ALIGN(RT_ALIGN_SIZE)
#endif

#if LV_USE_LOG
static void lv_rt_log(const char *buf)
{
    LOG_I(buf);
}
#endif /* LV_USE_LOG */

void lv_mutex_take(void)
{
    rt_mutex_take(&lvgl_mutex, RT_WAITING_FOREVER);
}

void lv_mutex_release(void)
{
    rt_mutex_release(&lvgl_mutex);
}

static void lvgl_thread_entry(void *parameter)
{
#if LV_USE_LOG
    lv_log_register_print_cb(lv_rt_log);
#endif /* LV_USE_LOG */
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    lv_user_gui_init();

    /* handle the tasks of LVGL */
    while(1)
    {
        lv_mutex_take();
        lv_task_handler();
        lv_mutex_release();

        rt_thread_mdelay(10);
    }
}

static int lvgl_thread_init(void)
{
    /* 初始化互斥 */
    rt_mutex_init(&lvgl_mutex, "lvgl", RT_IPC_FLAG_PRIO);

    lvgl_thread = rt_thread_create( "lvgl",
                                    lvgl_thread_entry,
                                    NULL,
                                    PKG_LVGL_THREAD_STACK_SIZE,
                                    PKG_LVGL_THREAD_PRIO,
                                    10);
    if ( lvgl_thread != RT_NULL)
    {
        rt_thread_startup(lvgl_thread);
    }
    else
    {
        LOG_E("Failed to create LVGL thread");
        return -RT_ENOMEM;
    }

    return RT_EOK;
}
INIT_SERVICE_EXPORT(lvgl_thread_init);

#endif /*__RTTHREAD__*/

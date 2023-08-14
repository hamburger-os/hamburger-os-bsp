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

#if 0 //运行Gui-Guider创建的app

#include <lvgl.h>
#include "gui_guider.h"
#include "custom.h"
#include "widgets_init.h"

lv_ui guider_ui;

void lv_user_gui_init(void)
{
    /*Create a GUI-Guider app */
    setup_ui(&guider_ui);
    custom_init(&guider_ui);

//    extern void lv_demo_calendar(void);
//    lv_demo_calendar();
}
#endif

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

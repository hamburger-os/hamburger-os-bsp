/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2021-10-17     Meco Man      first version
 * 2022-05-10     Meco Man      improve rt-thread initialization process
 */
#include "board.h"
#include <lvgl.h>

#define DBG_TAG "lv_demo"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

__WEAK void lv_user_gui_init(void)
{
    LOG_D("lvgl demo start...");
    /* display demo; you may replace with your LVGL application at here */
#if defined(PKG_USING_LV_MUSIC_DEMO)
    extern void lv_demo_music(void);
    lv_demo_music();
#elif defined(PKG_USING_GUI_GUIDER_DEMO)
    extern void gui_guider_setup(void);
    gui_guider_setup();
#elif defined(BSP_USING_LVGL_DEMO)
    extern void lv_demo_calendar(void);
    lv_demo_calendar();
#else

#endif
}

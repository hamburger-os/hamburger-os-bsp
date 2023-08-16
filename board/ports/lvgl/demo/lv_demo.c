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

#ifdef PKG_LVGL_USING_DEMOS
#include "lv_demos.h"
#endif

#define DBG_TAG "lv_demo"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

__WEAK void lv_user_gui_init(void)
{
    LOG_D("lvgl demo start...");
    /* display demo; you may replace with your LVGL application at here */
#if defined(BSP_USING_LVGL_WIDGETS_DEMO)
    lv_demo_widgets();
#elif defined(BSP_USING_LVGL_BENCHMARK_DEMO)
    lv_demo_benchmark();
#elif defined(BSP_USING_LVGL_STRESS_DEMO)
    lv_demo_stress();
#elif defined(BSP_USING_LVGL_KEYPAD_AND_ENCODER_DEMO)
    lv_demo_keypad_encoder();
#elif defined(BSP_USING_LVGL_MUSIC_DEMO)
    lv_demo_music();
#elif defined(BSP_USING_LVGL_RT_MUSIC_DEMO)
    extern void lv_demo_music(void);
    lv_demo_music();
#elif defined(BSP_USING_LVGL_DEMO)
    extern void lv_demo_calendar(void);
    lv_demo_calendar();
#else

#endif
}

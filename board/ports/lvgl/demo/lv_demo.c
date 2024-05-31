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

#ifdef PKG_USING_LVGL
#include <lvgl.h>
#endif

#ifdef PKG_LVGL_USING_DEMOS
#include "lv_demos.h"
#endif

#define DBG_TAG "lv_demo"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef PKG_USING_LVGL
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
    lv_obj_t * label_logo = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label_logo, LV_LABEL_LONG_WRAP);
    lv_label_set_recolor(label_logo, true);
    lv_label_set_text_fmt(label_logo,
            "SWOS2 %s\n"
            "Thread Operating System\n"
            "%d.%d.%d build %s %s\n"
            "Powered by RT-Thread\n"
            "2006 - 2023 Copyright\n"
            "by hnthinker team\n"
            , SYS_VERSION
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
            , RT_VERSION_MAJOR, RT_VERSION_MINOR, RT_VERSION_PATCH, __DATE__, __TIME__);
#else
            , RT_VERSION, RT_SUBVERSION, RT_REVISION, __DATE__, __TIME__);
#endif
    lv_obj_set_width(label_logo, LV_HOR_RES_MAX*3/4);
    lv_obj_set_style_text_align(label_logo, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_logo, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * btn_enter = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_enter, LV_HOR_RES_MAX/8, LV_HOR_RES_MAX/8);
    lv_obj_align(btn_enter, LV_ALIGN_CENTER, 0, LV_VER_RES_MAX/4);

    lv_obj_t * btn_label = lv_label_create(btn_enter);
    lv_obj_set_style_text_align(btn_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(btn_label, LV_SYMBOL_PLAY);
#endif
}
#endif

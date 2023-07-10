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
#if LV_USE_DEMO_RTT_MUSIC == 1
    extern void lv_demo_music(void);
    lv_demo_music();
#elif defined(BSP_USING_LVGL_DEMO)
    extern void lv_demo_calendar(void);
//    lv_demo_calendar();
#else
    lv_color_t *cbuf = rt_malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(LCD_WIDTH, LCD_HEIGHT));
    extern lv_obj_t * lv_100ask_sketchpad_create(lv_obj_t * parent);
    lv_obj_t * sketchpad = lv_100ask_sketchpad_create(lv_scr_act());

    lv_canvas_set_buffer(sketchpad, cbuf, LCD_WIDTH, LCD_HEIGHT, LV_IMG_CF_RGB565);
    lv_obj_center(sketchpad);
    lv_canvas_fill_bg(sketchpad, lv_palette_lighten(LV_PALETTE_RED, 3), LV_OPA_TRANSP);
#endif
}

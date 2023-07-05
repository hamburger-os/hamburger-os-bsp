/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-05     lvhan       the first version
 */
#ifndef BOARD_PORTS_LVGL_DEMO_LV_100ASK_SKETCHPAD_H_
#define BOARD_PORTS_LVGL_DEMO_LV_100ASK_SKETCHPAD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    LV_100ASK_SKETCHPAD_TOOLBAR_OPT_ALL = 0,
    LV_100ASK_SKETCHPAD_TOOLBAR_OPT_CW,
    LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH,
    LV_100ASK_SKETCHPAD_TOOLBAR_OPT_LAST
}lv_100ask_sketchpad_toolbar_t;

/*Data of canvas*/
typedef struct {
    lv_img_t img;
    lv_img_dsc_t dsc;
    lv_draw_line_dsc_t line_rect_dsc;
} lv_100ask_sketchpad_t;

/***********************
 * GLOBAL VARIABLES
 ***********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/*=====================
 * Setter functions
 *====================*/

/*=====================
 * Getter functions
 *====================*/

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* BOARD_PORTS_LVGL_DEMO_LV_100ASK_SKETCHPAD_H_ */

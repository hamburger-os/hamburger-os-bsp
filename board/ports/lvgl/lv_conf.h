/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2021-10-18     Meco Man      First version
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <rtconfig.h>

#define LV_COLOR_DEPTH          16
#define LV_USE_PERF_MONITOR     1
#define LV_HOR_RES_MAX          LCD_WIDTH
#define LV_VER_RES_MAX          LCD_HEIGHT

/*==================
* DEMO
*==================*/
#ifdef BSP_USING_LVGL_WIDGETS_DEMO
#  define LV_USE_DEMO_WIDGETS 1
#ifdef LVGL_WIDGETS_DEMO_SLIDESHOW
#  define LV_DEMO_WIDGETS_SLIDESHOW 1
#endif
#endif

#ifdef BSP_USING_LVGL_BENCHMARK_DEMO
#  define LV_USE_DEMO_BENCHMARK 1
#endif

#ifdef BSP_USING_LVGL_STRESS_DEMO
#  define LV_USE_DEMO_STRESS 1
#endif

#ifdef BSP_USING_LVGL_KEYPAD_AND_ENCODER_DEMO
#  define LV_USE_DEMO_KEYPAD_AND_ENCODER 1
#endif

#ifdef BSP_USING_LVGL_MUSIC_DEMO
#  define LV_USE_DEMO_MUSIC 1
#ifdef LVGL_MUSIC_DEMO_SQUARE
#  define LV_DEMO_MUSIC_SQUARE 1
#endif
#ifdef LVGL_MUSIC_DEMO_LANDSCAPE
#  define LV_DEMO_MUSIC_LANDSCAPE 1
#endif
#ifdef LVGL_MUSIC_DEMO_ROUND
#  define LV_DEMO_MUSIC_ROUND 1
#endif
#ifdef LVGL_MUSIC_DEMO_LARGE
#  define LV_DEMO_MUSIC_LARGE 1
#endif
#ifdef LVGL_MUSIC_DEMO_AUTO_PLAY
#  define LV_DEMO_MUSIC_AUTO_PLAY 1
#endif

#define LV_FONT_MONTSERRAT_12       1
#define LV_FONT_MONTSERRAT_16       1
#define LV_FONT_MONTSERRAT_22       1
#define LV_FONT_MONTSERRAT_32       1
#endif

/*==================
* GPU
*==================*/
#define LV_USE_GPU                  0   /*Only enables `gpu_fill_cb` and `gpu_blend_cb` in the disp. drv- */
#define LV_USE_GPU_STM32_DMA2D      1
/*If enabling LV_USE_GPU_STM32_DMA2D, LV_GPU_DMA2D_CMSIS_INCLUDE must be defined to include path of CMSIS header of target processor
e.g. "stm32f769xx.h" or "stm32f429xx.h" */
#ifdef SOC_M4COREBOARD_SDRAM
#define LV_GPU_DMA2D_CMSIS_INCLUDE  "stm32f429xx.h"
#endif
#ifdef SOC_H7COREBOARD_SDRAM
#define LV_GPU_DMA2D_CMSIS_INCLUDE  "stm32h743xx.h"
#endif

#endif

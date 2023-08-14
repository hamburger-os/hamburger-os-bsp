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

#ifdef PKG_USING_LV_MUSIC_DEMO
/* music player demo */
#define LV_USE_DEMO_RTT_MUSIC       1
#define LV_DEMO_RTT_MUSIC_AUTO_PLAY 1
#define LV_FONT_MONTSERRAT_12       1
#define LV_FONT_MONTSERRAT_16       1
#define LV_COLOR_SCREEN_TRANSP      1
#endif /* PKG_USING_LV_MUSIC_DEMO */

#define LV_USE_GPU                  1   /*Only enables `gpu_fill_cb` and `gpu_blend_cb` in the disp. drv- */
#define LV_USE_GPU_STM32_DMA2D      0
/*If enabling LV_USE_GPU_STM32_DMA2D, LV_GPU_DMA2D_CMSIS_INCLUDE must be defined to include path of CMSIS header of target processor
e.g. "stm32f769xx.h" or "stm32f429xx.h" */
#ifdef SOC_M4COREBOARD_SDRAM
#define LV_GPU_DMA2D_CMSIS_INCLUDE  "stm32f429xx.h"
#endif
#ifdef SOC_H7COREBOARD_SDRAM
#define LV_GPU_DMA2D_CMSIS_INCLUDE  "stm32h743xx.h"
#endif

#endif

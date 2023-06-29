/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-02-08     Zhangyihong  the first version
 */

#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>

#ifdef BSP_USING_TOUCH
#include "drv_touch.h"

#define DBG_TAG "drv.touch"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef PKG_USING_GUIENGINE
#include <rtgui/event.h>
#include <rtgui/rtgui_server.h>
#elif defined(PKG_USING_LITTLEVGL2RTT)
#include <littlevgl2rtt.h>
#elif defined(PKG_USING_LVGL)
#include <lvgl.h>
extern void lv_port_indev_input(rt_int16_t x, rt_int16_t y, lv_indev_state_t state);
#endif /* PKG_USING_GUIENGINE */

int drv_touch_hw_init(void)
{
    rt_err_t result = RT_EOK;

    return result;
}
INIT_DEVICE_EXPORT(drv_touch_hw_init);

#endif

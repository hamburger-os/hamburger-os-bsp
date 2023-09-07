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
#include "sw_touch.h"
#include "xpt2046.h"

#define DBG_TAG "drv.touch"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#if defined(PKG_USING_LVGL)
#include <lvgl.h>
#endif /* PKG_USING_LVGL */

static struct _rt_drv_touch drv_touch;
extern struct rt_touch_ops drv_touch_ops;

__weak int drv_touch_bus_init(struct _rt_drv_touch *config)
{
    return RT_EOK;
}

int drv_touch_hw_init(void)
{
    rt_err_t result = RT_EOK;

    drv_touch.dev.info.range_x = LCD_WIDTH;
    drv_touch.dev.info.range_y = LCD_HEIGHT;
#ifdef TOUCH_USING_XPT2046
    drv_touch.dev.info.point_num = 1;
    drv_touch.dev.info.type = RT_TOUCH_TYPE_RESISTANCE;
    drv_touch.dev.info.vendor = RT_TOUCH_VENDOR_UNKNOWN;
#endif
#ifdef TOUCH_USING_SW
    drv_touch.dev.info.point_num = 1;
    drv_touch.dev.info.type = RT_TOUCH_TYPE_RESISTANCE;
    drv_touch.dev.info.vendor = RT_TOUCH_VENDOR_UNKNOWN;
#endif
    drv_touch.dev.config.user_data = &drv_touch;
#ifdef RT_TOUCH_PIN_IRQ
    drv_touch.dev.config.irq_pin.pin = rt_pin_get(TOUCH_IRQ_PIN);
    drv_touch.dev.config.irq_pin.mode = PIN_MODE_INPUT_PULLUP;
    rt_pin_mode(drv_touch.dev.config.irq_pin.pin, drv_touch.dev.config.irq_pin.mode);
#endif /* RT_TOUCH_PIN_IRQ */
    drv_touch.dev.ops = &drv_touch_ops;

    drv_touch_bus_init(&drv_touch);
    /* register lcd device */
    result = rt_hw_touch_register(&drv_touch.dev, "touch", RT_DEVICE_FLAG_INT_RX, &drv_touch);

    return result;
}
INIT_DEVICE_EXPORT(drv_touch_hw_init);

int drv_touch_env_init(void)
{
#ifdef TOUCH_USING_SW
    sw_calibration("lcd", "touch");
#endif

#ifdef TOUCH_USING_XPT2046
    xpt2046_calibration("lcd", "touch");
#endif

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(drv_touch_env_init);

#endif

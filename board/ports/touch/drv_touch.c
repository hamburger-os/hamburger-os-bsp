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

#ifdef PKG_USING_GUIENGINE
#include <rtgui/event.h>
#include <rtgui/rtgui_server.h>
#elif defined(PKG_USING_LITTLEVGL2RTT)
#include <littlevgl2rtt.h>
#elif defined(PKG_USING_LVGL)
#include <lvgl.h>
extern void lv_port_indev_input(rt_int16_t x, rt_int16_t y, lv_indev_state_t state);
#endif /* PKG_USING_GUIENGINE */

static struct _rt_drv_touch drv_touch;
extern struct rt_touch_ops drv_touch_ops;

__weak int drv_touch_bus_init(struct _rt_drv_touch *config)
{
    return RT_EOK;
}

static void touch_thread_entry(void *parameter)
{
    struct rt_touch_data read_data;

    /* Find the touch device */
    rt_device_t touch = rt_device_find("touch");
    if (touch == RT_NULL)
    {
        LOG_E("can't find touch device");
        return;
    }
    if (rt_device_open(touch, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        LOG_E("can't open touch device");
        return;
    }

    while (1)
    {
        /* Prepare variable to read out the touch data */
        rt_memset(&read_data, 0, sizeof(struct rt_touch_data));
        if (rt_device_read(touch, 0, &read_data, 1) == 1)
        {
#ifdef PKG_USING_LVGL
            lv_port_indev_input(read_data.x_coordinate, read_data.y_coordinate,
                                ((read_data.event = RT_TOUCH_EVENT_DOWN) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL));
#else /* PKG_USING_LVGL */
            const rt_uint32_t black = 0x0;
            rt_graphix_ops(lcd)->set_pixel((const char *)(&black),
                                        read_data.x_coordinate,
                                         read_data.y_coordinate);
#endif /* PKG_USING_LVGL */
        }
        rt_thread_mdelay(1);
    }
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
#endif /* RT_TOUCH_PIN_IRQ */
    drv_touch.dev.ops = &drv_touch_ops;

    drv_touch_bus_init(&drv_touch);
    /* register lcd device */
    result = rt_hw_touch_register(&drv_touch.dev, "touch", RT_DEVICE_FLAG_INT_RX, &drv_touch);

    /* 创建 touch 线程 */
    drv_touch.touch_thread = rt_thread_create("touch", touch_thread_entry, &drv_touch, 2048, 19, 10);
    /* 创建成功则启动线程 */
    if (drv_touch.touch_thread != RT_NULL)
    {
        rt_thread_startup(drv_touch.touch_thread);
    }
    else
    {
        LOG_E("thread create error!");
    }

    return result;
}
INIT_DEVICE_EXPORT(drv_touch_hw_init);

int drv_touch_env_init(void)
{
#ifdef TOUCH_USING_SW
//    sw_calibration("lcd", "touch");
#endif

#ifdef TOUCH_USING_XPT2046
    xpt2046_calibration("lcd", "touch");
#endif

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(drv_touch_env_init);

#endif

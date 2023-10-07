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

#ifdef PKG_USING_FLASHDB_PORT
#include "flashdb_port.h"
#endif

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

#ifdef TOUCH_USING_CALIBRATION
int drv_touch_calibration(void)
{
#ifdef TOUCH_USING_SW
    sw_calibration("lcd", "touch");
#endif

#ifdef TOUCH_USING_XPT2046
    xpt2046_calibration("lcd", "touch");
#endif

    return RT_EOK;
}
INIT_ENV_EXPORT(drv_touch_calibration);
#endif

#ifdef BSP_USING_NO_LVGL_DEMO
static void touch_show_thread_entry(void* parameter)
{
    uint16_t black = 0b1111100000000000;
    uint16_t white = 0b1111111111111111;
    char *lcd_name = "lcd";
    char *touch_name = "touch";

    /* Find the TFT LCD device */
    rt_device_t lcd = rt_device_find(lcd_name);
    if (lcd == RT_NULL)
    {
        LOG_E(" cannot find lcd device named %s", lcd_name);
        return;
    }
    if (rt_device_open(lcd, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        LOG_E(" cannot open lcd device named %s", lcd_name);
        return;
    }

    /* Find the Touch device */
    rt_device_t touch = rt_device_find(touch_name);
    if (touch == RT_NULL)
    {
        LOG_E(" cannot find touch device named %s", touch_name);
        return;
    }
    if (rt_device_open(touch, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        LOG_E(" cannot open touch device named %s", touch_name);
        return;
    }

    /* Get TFT LCD screen information */
    struct rt_device_graphic_info lcd_info = { 0 };
    rt_device_control(lcd, RTGRAPHIC_CTRL_GET_INFO, &lcd_info);

    /* clear screen */
    LOG_D(" lcd (%d, %d)", lcd_info.width, lcd_info.height);
    for (uint16_t y = 0; y < lcd_info.height; ++y)
    {
        rt_graphix_ops(lcd)->draw_hline((const char *) (&white), 0, lcd_info.width, y);
    }
    rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);

    /* Set the information for the four points used for calibration */
    uint16_t cross_size = (lcd_info.width > lcd_info.height ? lcd_info.height : lcd_info.width) / 32;

    uint8_t raw_idx = 0;
    struct rt_touch_data read_data = {0};
    while (1)
    {
        rt_thread_mdelay(100);

        if (rt_device_read((rt_device_t) touch, 0, &read_data, 1) == 1)
        {
            LOG_I(" %d point press (%d, %d)(%d, %d)", raw_idx, read_data.x, read_data.y,
                    read_data.x_coordinate, read_data.y_coordinate);
            if (read_data.x_coordinate < lcd_info.width - cross_size || read_data.y_coordinate < lcd_info.height - cross_size)
            {
                if (raw_idx % 16 == 0)
                {
                    /* After processing a point, proceed to clear the screen */
                    for (uint16_t y = 0; y < lcd_info.height; ++y)
                    {
                        rt_graphix_ops(lcd)->draw_hline((const char *) (&white), 0, lcd_info.width, y);
                    }
                    rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
                }

                rt_graphix_ops(lcd)->draw_hline((const char *) (&black), read_data.x_coordinate - cross_size,
                        read_data.x_coordinate + cross_size, read_data.y_coordinate);
                rt_graphix_ops(lcd)->draw_vline((const char *) (&black), read_data.x_coordinate,
                        read_data.y_coordinate - cross_size, read_data.y_coordinate + cross_size);
                rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);

                raw_idx++;
            }
        }
    }

    rt_device_close(lcd);
    rt_device_close(touch);
}

int drv_touch_show(void)
{
    rt_thread_t touch_show_thread = rt_thread_create( "touch_show",
                                    touch_show_thread_entry,
                                    NULL,
                                    2048,
                                    24,
                                    10);
    if ( touch_show_thread != RT_NULL)
    {
        rt_thread_startup(touch_show_thread);
    }

    return RT_EOK;
}
INIT_APP_EXPORT(drv_touch_show);
#endif

#ifdef PKG_USING_FLASHDB_PORT
int touch_adjust_del(void)
{
    return kvdb_del("sw_touch");
}
MSH_CMD_EXPORT(touch_adjust_del, touch adjust del);
#endif

#endif

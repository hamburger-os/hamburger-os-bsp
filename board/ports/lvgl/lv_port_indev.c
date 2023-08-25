/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-18     Meco Man     The first version
 */

#include <board.h>

#ifdef PKG_USING_LVGL
#include <lvgl.h>

#define DRV_DEBUG
#define LOG_TAG "drv.lv"
#include <drv_log.h>

static lv_indev_t *touch_indev;
static lv_indev_drv_t indev_drv;
static rt_device_t touch;

static void input_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    struct rt_touch_data read_data;
    if (rt_device_read(touch, 0, &read_data, 1) == 1)
    {
#ifdef PKG_USING_LVGL
        data->point.x = read_data.x_coordinate;
        data->point.y = read_data.y_coordinate;
        data->state = ((read_data.event = RT_TOUCH_EVENT_DOWN) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL);
#else /* PKG_USING_LVGL */
        const rt_uint32_t black = 0x0;
        rt_graphix_ops(lcd)->set_pixel((const char *)(&black),
                                        read_data.x_coordinate,
                                        read_data.y_coordinate);
#endif /* PKG_USING_LVGL */
    }
}

void lv_port_indev_init(void)
{
    touch = rt_device_find("touch");
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

    lv_indev_drv_init(&indev_drv); /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = input_read;

    /*Register the driver in LVGL and save the created input device object*/
    touch_indev = lv_indev_drv_register(&indev_drv);
}

#endif

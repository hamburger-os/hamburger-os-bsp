/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-02-08     Zhangyihong  the first version
 */

#ifndef __DRV_TOUCH_H__
#define __DRV_TOUCH_H__

#include "rtthread.h"
#include "rtdevice.h"

struct _rt_drv_touch
{
    struct rt_touch_device dev;
    struct rt_spi_device *spidev;

    rt_thread_t  touch_thread;
};

int drv_touch_bus_init(struct _rt_drv_touch *config);

#endif

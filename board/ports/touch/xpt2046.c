/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-30     lvhan       the first version
 */
#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_spi.h"
#include "drv_soft_spi.h"

#ifdef TOUCH_USING_XPT2046
#include "drv_touch.h"
#include "xpt2046.h"

#define DBG_TAG "xpt2046"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

rt_err_t touch_Read_toucX(struct rt_touch_device *touch, uint16_t *value)
{
    rt_err_t ret = RT_EOK;
    struct _rt_drv_touch *config = (struct _rt_drv_touch *)touch->parent.user_data;

    uint8_t cmd[] = {0XD0, 0X00};
    ret = rt_spi_send_then_recv(config->spidev, &cmd, sizeof(cmd), value, 2);
    if (ret != RT_EOK)
    {
        LOG_E("read x error %d!", ret);
        ret = -RT_EIO;
    }
    *value = __SWP16(*value);
    *value = *value >> 3;

    return ret;
}

rt_err_t touch_Read_toucY(struct rt_touch_device *touch, uint16_t *value)
{
    rt_err_t ret = RT_EOK;
    struct _rt_drv_touch *config = (struct _rt_drv_touch *)touch->parent.user_data;

    uint8_t cmd[] = {0X90, 0X00};
    ret = rt_spi_send_then_recv(config->spidev, &cmd, sizeof(cmd), value, 2);
    if (ret != RT_EOK)
    {
        LOG_E("read y error %d!", ret);
        ret = -RT_EIO;
    }
    *value = __SWP16(*value);
    *value = *value >> 3;

    return ret;
}

int drv_touch_bus_init(struct _rt_drv_touch *config)
{
    rt_uint8_t dev_num = 0;
    char dev_name[RT_NAME_MAX];
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", TOUCH_XPT2046_SPI_BUS, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

    rt_hw_soft_spi_device_attach(TOUCH_XPT2046_SPI_BUS, dev_name, TOUCH_XPT2046_CS_PIN);
//    rt_hw_spi_device_attach(TOUCH_XPT2046_SPI_BUS, dev_name, rt_pin_get(TOUCH_XPT2046_CS_PIN));
    config->spidev = (struct rt_spi_device *) rt_device_find(dev_name);
    if (config->spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }
    struct rt_spi_configuration cfg = { 0 };
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = TOUCH_XPT2046_SPI_SPEED;
    if (rt_spi_configure(config->spidev, &cfg) != RT_EOK)
    {
        LOG_E("device %s configure error!", dev_name);
        return -RT_EIO;
    }

    return RT_EOK;
}

#endif

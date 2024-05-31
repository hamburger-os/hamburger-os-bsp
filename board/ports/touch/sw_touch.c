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

#ifdef TOUCH_USING_SW
#include "drv_touch.h"
#include "sw_touch.h"

#define DBG_TAG "sw_touch"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

rt_err_t touch_Read_toucX(struct rt_touch_device *touch, uint16_t *value)
{
    rt_err_t ret = RT_EOK;
    struct _rt_drv_touch *config = (struct _rt_drv_touch *)touch->parent.user_data;

    uint8_t cmd[] = {0XD0};
    uint8_t rx_data[2] = {0};
    ret = rt_spi_send_then_recv(config->spidev, cmd, 1, rx_data, 2);
    if (ret != RT_EOK)
    {
        LOG_E("read x error %d!", ret);
        ret = -RT_EIO;
    }
    LOG_HEX("rd", 16, rx_data, 2);
    *value = (rx_data[0] << 8 | rx_data[1]) >> 4;
    LOG_D("read x %d", *value);

    return ret;
}

rt_err_t touch_Read_toucY(struct rt_touch_device *touch, uint16_t *value)
{
    rt_err_t ret = RT_EOK;
    struct _rt_drv_touch *config = (struct _rt_drv_touch *)touch->parent.user_data;

    uint8_t cmd[] = {0X90};
    uint8_t rx_data[2] = {0};
    ret = rt_spi_send_then_recv(config->spidev, cmd, 1, rx_data, 2);
    if (ret != RT_EOK)
    {
        LOG_E("read y error %d!", ret);
        ret = -RT_EIO;
    }
    LOG_HEX("rd", 16, rx_data, 2);
    *value = (rx_data[0] << 8 | rx_data[1]) >> 4;
    LOG_D("read y %d", *value);

    return ret;
}

int drv_touch_bus_init(struct _rt_drv_touch *config)
{
    rt_uint8_t dev_num = 0;
    char dev_name[RT_NAME_MAX];
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", TOUCH_SW_SPI_BUS, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

//    rt_hw_soft_spi_device_attach(TOUCH_SW_SPI_BUS, dev_name, rt_pin_get(TOUCH_SW_CS_PIN));
    rt_hw_spi_device_attach(TOUCH_SW_SPI_BUS, dev_name, rt_pin_get(TOUCH_SW_CS_PIN));
    config->spidev = (struct rt_spi_device *) rt_device_find(dev_name);
    if (config->spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }
    struct rt_spi_configuration cfg = { 0 };
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = TOUCH_SW_SPI_SPEED;
    if (rt_spi_configure(config->spidev, &cfg) != RT_EOK)
    {
        LOG_E("device %s configure error!", dev_name);
        return -RT_EIO;
    }

    //初始化默认参数
    config->adjuct.A = 0.46f;
    config->adjuct.B = 0.00f;
    config->adjuct.C = -76.07f;
    config->adjuct.D = 0.00f;
    config->adjuct.E = 0.34f;
    config->adjuct.F = -38.40f;

    return RT_EOK;
}

#endif

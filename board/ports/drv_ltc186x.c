/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-13     lvhan       the first version
 */
#include "board.h"

#define DBG_TAG "ltc186x"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#include "drv_spi.h"
#include "drv_soft_spi.h"

enum {
    LTC186x_CHANNEL_0 = 0x0080,
    LTC186x_CHANNEL_1 = 0x00C0,
    LTC186x_DIFF_0 = 0x0000,
    LTC186x_DIFF_1 = 0x0040,
};

/* ltc186x dirver class */
struct ltc186x_driver
{
    char *                  spibus;
    char *                  cspin;
    uint8_t                 resolution;
    uint16_t                vref_mv;
    rt_mutex_t              lock;
    struct rt_spi_device *  spidev;
    struct rt_adc_device    adcdev;
};
static struct ltc186x_driver ltc186x_dev = {
    .spibus = BSP_LTC186X_SPI_BUS,
    .cspin  = BSP_LTC186X_SPI_CS_PIN,
    .resolution = BSP_LTC186X_RESOLUTION,
    .vref_mv = BSP_LTC186X_VREF,
};

static rt_err_t ltc186x_transfer(struct rt_spi_device *device, uint16_t cmd, uint16_t *data)
{
    rt_err_t ret = RT_EOK;

    ret = rt_spi_send(device, &cmd, 2);
    if (ret != 2)
    {
        LOG_E("spi_send error %d!", ret);
        ret = -RT_EIO;
    }
    rt_thread_delay(1);
    ret = rt_spi_recv(device, data, 2);
    if (ret != 2)
    {
        LOG_E("spi_recv error %d!", ret);
        ret = -RT_EIO;
    }
    *data = (*data >> 8) | ((uint8_t)*data << 8);
    LOG_D("transfer: 0x%04x 0x%04x", cmd, *data);

    return ret;
}

static rt_err_t ltc186x_adc_enabled(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled)
{
    RT_ASSERT(device != RT_NULL);

    struct ltc186x_driver *driver = device->parent.user_data;

    return RT_EOK;
}

static rt_err_t ltc186x_adc_get_value(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value)
{
    RT_ASSERT(device != RT_NULL);
    rt_err_t ret = RT_EOK;

    uint16_t data = 0xffff;
    struct ltc186x_driver *driver = device->parent.user_data;

    switch(channel)
    {
    case 0:
        ret = ltc186x_transfer(driver->spidev, LTC186x_CHANNEL_0, &data);
        break;
    case 1:
        ret = ltc186x_transfer(driver->spidev, LTC186x_CHANNEL_1, &data);
        break;
    case 2:
        ret = ltc186x_transfer(driver->spidev, LTC186x_DIFF_0, &data);
        break;
    case 3:
        ret = ltc186x_transfer(driver->spidev, LTC186x_DIFF_1, &data);
        break;
    default:
        break;
    }

    *value = data;

    return ret;
}

static rt_uint8_t ltc186x_adc_get_resolution(struct rt_adc_device *device)
{
    RT_ASSERT(device != RT_NULL);

    struct ltc186x_driver *driver = device->parent.user_data;

    return driver->resolution;
}

static rt_uint32_t ltc186x_adc_get_vref (struct rt_adc_device *device)
{
    RT_ASSERT(device != RT_NULL);

    struct ltc186x_driver *driver = device->parent.user_data;

    return driver->vref_mv;
}

static const struct rt_adc_ops ltc186x_adc_ops =
{
    .enabled = ltc186x_adc_enabled,
    .convert = ltc186x_adc_get_value,
    .get_resolution = ltc186x_adc_get_resolution,
    .get_vref = ltc186x_adc_get_vref,
};

static int rt_hw_ltc186x_init(void)
{
    rt_err_t ret = RT_EOK;

    rt_uint8_t dev_num = 0;
    char dev_name[RT_NAME_MAX];
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", ltc186x_dev.spibus, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

    rt_hw_soft_spi_device_attach(ltc186x_dev.spibus, dev_name, ltc186x_dev.cspin);
//    rt_hw_spi_device_attach(ltc186x_dev.spibus, dev_name, rt_pin_get(ltc186x_dev.cspin));
    ltc186x_dev.spidev = (struct rt_spi_device *)rt_device_find(dev_name);
    if (ltc186x_dev.spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }
    struct rt_spi_configuration cfg = {0};
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = BSP_LTC186X_SPI_SPEED;
    ret = rt_spi_configure(ltc186x_dev.spidev, &cfg);
    if (ret != RT_EOK)
    {
        LOG_E("device %s configure error %d!", dev_name, ret);
        return -RT_EIO;
    }

    /* register ADC device */
    if (rt_hw_adc_register(&ltc186x_dev.adcdev, "ltc186x", &ltc186x_adc_ops, &ltc186x_dev) == RT_EOK)
    {
        LOG_I("init success");
    }
    else
    {
        LOG_E("register failed");
        return -RT_ERROR;
    }

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_hw_ltc186x_init);

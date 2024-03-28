/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-6-14      solar        first version
 */
#include <board.h>
#include "drv_soft_spi.h"
#include "drv_config.h"

#if defined(RT_USING_SPI) && defined(RT_USING_SPI_BITOPS) && defined(RT_USING_PIN)

//#define DRV_DEBUG
#define LOG_TAG             "drv.soft_spi"
#include <drv_log.h>

enum
{
#ifdef BSP_USING_SOFT_SPI1
    SOFT_SPI1_INDEX,
#endif

#ifdef BSP_USING_SOFT_SPI2
    SOFT_SPI2_INDEX,
#endif

#ifdef BSP_USING_SOFT_SPI3
    SOFT_SPI3_INDEX,
#endif
};

static struct stm32_soft_spi_config soft_spi_config[] =
{
#ifdef BSP_USING_SOFT_SPI1
        SOFT_SPI1_BUS_CONFIG,
#endif

#ifdef BSP_USING_SOFT_SPI2
        SOFT_SPI2_BUS_CONFIG,
#endif

#ifdef BSP_USING_SOFT_SPI3
    SOFT_SPI3_BUS_CONFIG,
#endif
};

/**
  * Attach the spi device to soft SPI bus, this function must be used after initialization.
  */
rt_err_t rt_hw_soft_spi_device_attach(const char *bus_name, const char *device_name, rt_base_t cs_pin)
{
    RT_ASSERT(bus_name != RT_NULL);
    RT_ASSERT(device_name != RT_NULL);

    rt_err_t result;
    struct rt_spi_device *spi_device;

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
#else
    /* initialize the cs pin && select the slave*/
    rt_pin_mode(cs_pin,PIN_MODE_OUTPUT);
    rt_pin_write(cs_pin,PIN_HIGH);

    rt_base_t *pcs_pin;
    pcs_pin = (rt_base_t *)rt_malloc(sizeof(rt_base_t));
    RT_ASSERT(pcs_pin != RT_NULL);
    *pcs_pin = cs_pin;
#endif

    /* attach the device to soft spi bus*/
    spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    RT_ASSERT(spi_device != RT_NULL);

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
    result = rt_spi_bus_attach_device_cspin(spi_device, device_name, bus_name, cs_pin, RT_NULL);
#else
    result = rt_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)pcs_pin);
#endif
    if (result != RT_EOK)
    {
        LOG_E("%s attach to %s faild, %d", device_name, bus_name, result);
    }

    RT_ASSERT(result == RT_EOK);

    LOG_D("%s attach to %s done", device_name, bus_name);

    return result;
}
RTM_EXPORT(rt_hw_soft_spi_device_attach);

static void stm32_spi_gpio_init(struct stm32_soft_spi *spi)
{
    struct stm32_soft_spi_config *cfg = (struct stm32_soft_spi_config *)spi->cfg;
    rt_pin_mode(cfg->sck, PIN_MODE_OUTPUT);
    rt_pin_mode(cfg->miso, PIN_MODE_INPUT);
    rt_pin_mode(cfg->mosi, PIN_MODE_OUTPUT);

    rt_pin_write(cfg->miso, PIN_HIGH);
    rt_pin_write(cfg->sck, PIN_HIGH);
    rt_pin_write(cfg->mosi, PIN_HIGH);
}

void stm32_tog_sclk(void *data)
{
    struct stm32_soft_spi_config* cfg = (struct stm32_soft_spi_config*)data;
    if(rt_pin_read(cfg->sck) == PIN_HIGH)
    {
        rt_pin_write(cfg->sck, PIN_LOW);
    }
    else
    {
        rt_pin_write(cfg->sck, PIN_HIGH);
    }
}

void stm32_set_sclk(void *data, rt_int32_t state)
{

    struct stm32_soft_spi_config* cfg = (struct stm32_soft_spi_config*)data;
    if (state)
    {
        rt_pin_write(cfg->sck, PIN_HIGH);
    }
    else
    {
        rt_pin_write(cfg->sck, PIN_LOW);
    }
}

void stm32_set_mosi(void *data, rt_int32_t state)
{
    struct stm32_soft_spi_config* cfg = (struct stm32_soft_spi_config*)data;
    if (state)
    {
        rt_pin_write(cfg->mosi, PIN_HIGH);
    }
    else
    {
        rt_pin_write(cfg->mosi, PIN_LOW);
    }
}

void stm32_set_miso(void *data, rt_int32_t state)
{
    struct stm32_soft_spi_config* cfg = (struct stm32_soft_spi_config*)data;
    if (state)
    {
        rt_pin_write(cfg->miso, PIN_HIGH);
    }
    else
    {
        rt_pin_write(cfg->miso, PIN_LOW);
    }
}

rt_int32_t stm32_get_sclk(void *data)
{
    struct stm32_soft_spi_config* cfg = (struct stm32_soft_spi_config*)data;
    return rt_pin_read(cfg->sck);
}

rt_int32_t stm32_get_mosi(void *data)
{
    struct stm32_soft_spi_config* cfg = (struct stm32_soft_spi_config*)data;
    return rt_pin_read(cfg->mosi);
}

rt_int32_t stm32_get_miso(void *data)
{
    struct stm32_soft_spi_config* cfg = (struct stm32_soft_spi_config*)data;
    return rt_pin_read(cfg->miso);
}

void stm32_dir_mosi(void *data, rt_int32_t state)
{
    struct stm32_soft_spi_config* cfg = (struct stm32_soft_spi_config*)data;
    if (state)
    {
        rt_pin_mode(cfg->mosi, PIN_MODE_INPUT);
    }
    else
    {
        rt_pin_mode(cfg->mosi, PIN_MODE_OUTPUT);
    }
}

void stm32_dir_miso(void *data, rt_int32_t state)
{
    struct stm32_soft_spi_config* cfg = (struct stm32_soft_spi_config*)data;
    if (state)
    {
        rt_pin_mode(cfg->miso, PIN_MODE_INPUT);
    }
    else
    {
        rt_pin_mode(cfg->miso, PIN_MODE_OUTPUT);
    }
}

static void stm32_udelay(rt_uint32_t us)
{
    rt_hw_us_delay(us);
}

static struct rt_spi_bit_ops stm32_soft_spi_ops =
    {
        .data = RT_NULL,
        .tog_sclk = stm32_tog_sclk,
        .set_sclk = stm32_set_sclk,
        .set_mosi = stm32_set_mosi,
        .set_miso = stm32_set_miso,
        .get_sclk = stm32_get_sclk,
        .get_mosi = stm32_get_mosi,
        .get_miso = stm32_get_miso,
        .dir_mosi = stm32_dir_mosi,
        .dir_miso = stm32_dir_miso,
        .udelay = stm32_udelay,
        .delay_us = 1,
};

static struct stm32_soft_spi spi_obj[sizeof(soft_spi_config) / sizeof(soft_spi_config[0])];

static void soft_spi_get_config(void)
{
#ifdef BSP_USING_SOFT_SPI1
    soft_spi_config[SOFT_SPI1_INDEX].sck = rt_pin_get(BSP_S_SPI1_SCK);
    soft_spi_config[SOFT_SPI1_INDEX].mosi = rt_pin_get(BSP_S_SPI1_MOSI);
    soft_spi_config[SOFT_SPI1_INDEX].miso = rt_pin_get(BSP_S_SPI1_MISO);
#endif

#ifdef BSP_USING_SOFT_SPI2
    soft_spi_config[SOFT_SPI2_INDEX].sck = rt_pin_get(BSP_S_SPI2_SCK);
    soft_spi_config[SOFT_SPI2_INDEX].mosi = rt_pin_get(BSP_S_SPI2_MOSI);
    soft_spi_config[SOFT_SPI2_INDEX].miso = rt_pin_get(BSP_S_SPI2_MISO);
#endif

#ifdef BSP_USING_SOFT_SPI3
    soft_spi_config[SOFT_SPI3_INDEX].sck = rt_pin_get(BSP_S_SPI3_SCK);
    soft_spi_config[SOFT_SPI3_INDEX].mosi = rt_pin_get(BSP_S_SPI3_MOSI);
    soft_spi_config[SOFT_SPI3_INDEX].miso = rt_pin_get(BSP_S_SPI3_MISO);
#endif
}

/* Soft SPI initialization function */
int rt_soft_spi_init(void)
{
    rt_err_t result;
    rt_size_t obj_num = sizeof(spi_obj) / sizeof(struct stm32_soft_spi);
    soft_spi_get_config();

    for (int i = 0; i < obj_num; i++)
    {
        stm32_soft_spi_ops.data = (void *)&soft_spi_config[i];
        spi_obj[i].spi.ops = &stm32_soft_spi_ops;
        spi_obj[i].cfg = (void *)&soft_spi_config[i];
        stm32_spi_gpio_init(&spi_obj[i]);
        result = rt_spi_bit_add_bus(&spi_obj[i].spi, soft_spi_config[i].bus_name, &stm32_soft_spi_ops);
        RT_ASSERT(result == RT_EOK);
    }

    return RT_EOK;
}
INIT_BOARD_EXPORT(rt_soft_spi_init);

#endif /* defined(RT_USING_SPI) && defined(RT_USING_SPI_BITOPS) && defined(RT_USING_PIN) */

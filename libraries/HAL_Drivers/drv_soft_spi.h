/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-6-14      solar        first version
 */

#ifndef __DRV_SOFT_SPI__
#define __DRV_SOFT_SPI__

#include <rthw.h>
#include <rtdevice.h>
#include <spi-bit-ops.h>

/* stm32 soft spi config */
struct stm32_soft_spi_config
{
    rt_uint8_t sck;
    rt_uint8_t mosi;
    rt_uint8_t miso;
    const char *bus_name;
};

/* stm32 soft spi dirver */
struct stm32_soft_spi
{
    struct rt_spi_bit_obj spi;
    struct stm32_soft_spi_config *cfg;
};

#ifdef BSP_USING_SOFT_SPI1
#define SOFT_SPI1_BUS_CONFIG                                    \
    {                                                       \
        .bus_name = "sspi1",                                \
    }
#endif /* BSP_USING_SOFT_SPI1 */

#ifdef BSP_USING_SOFT_SPI2
#define SOFT_SPI2_BUS_CONFIG                                    \
    {                                                       \
        .bus_name = "sspi2",                                \
    }
#endif /* BSP_USING_SOFT_SPI2 */

#ifdef BSP_USING_SOFT_SPI3
#define SOFT_SPI3_BUS_CONFIG                                    \
    {                                                       \
        .bus_name = "sspi3",                                \
    }
#endif /* BSP_USING_SOFT_SPI3 */

rt_err_t rt_hw_soft_spi_device_attach(const char *bus_name, const char *device_name, const char *pin_name);
int rt_soft_spi_init(void);

#endif /* __DRV_SOFT_SPI__ */

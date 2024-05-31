/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-26     lvhan       the first version
 */
#include "board.h"

#define DBG_TAG "sc16is752"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "drv_spi.h"
#include "drv_soft_spi.h"

#ifdef RT_USING_SERIAL_V1
#include "drv_usart.h"
#endif
#ifdef RT_USING_SERIAL_V2
#include "drv_usart_v2.h"
#endif

/* SC16IS752 dirver class */
struct SC16IS752Def
{
    const char *spi_bus;
    uint32_t spi_speed;
    const char *cs;
    const char *irq;
    const char *rst;

    struct rt_spi_device *spidev;
    rt_base_t cs_pin;
    rt_base_t irq_pin;
    rt_base_t rst_pin;

    const char *uart_name[2];
    struct rt_serial_device serial[2];
};

static struct SC16IS752Def sc16is752_config= {
    .spi_bus = SC16IS752_SPI_BUS,
    .spi_speed = SC16IS752_SPI_SPEED,
    .cs = SC16IS752_SPI_CS_PIN,
    .irq = SC16IS752_IRQ_PIN,
    .rst = SC16IS752_RST_PIN,

    .uart_name = {"scuart1", "scuart2"},
};

static int sc16is752_spi_device_init(void)
{
    rt_err_t ret = RT_EOK;

    rt_uint8_t dev_num = 0;
    char dev_name[RT_NAME_MAX];
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", sc16is752_config.spi_bus, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

//    rt_hw_soft_spi_device_attach(sc16is752_config.spi_bus, dev_name, rt_pin_get(sc16is752_config.cs));
    rt_hw_spi_device_attach(sc16is752_config.spi_bus, dev_name, rt_pin_get(sc16is752_config.cs));
    sc16is752_config.spidev = (struct rt_spi_device *)rt_device_find(dev_name);
    if (sc16is752_config.spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }
    struct rt_spi_configuration cfg = {0};
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = BSP_LTC186X_SPI_SPEED;
    ret = rt_spi_configure(sc16is752_config.spidev, &cfg);
    if (ret != RT_EOK)
    {
        LOG_E("device %s configure error %d!", dev_name, ret);
        return -RT_EIO;
    }

    return RT_EOK;
}
INIT_PREV_EXPORT(sc16is752_spi_device_init);

static rt_err_t sc16is752_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    return RT_EOK;
}

static rt_err_t sc16is752_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    return RT_EOK;
}

static int sc16is752_putc(struct rt_serial_device *serial, char c)
{
    return 1;
}

static int sc16is752_getc(struct rt_serial_device *serial)
{
    int ch = 0;
    return ch;
}

static rt_size_t sc16is752_transmit(struct rt_serial_device     *serial,
                                       rt_uint8_t           *buf,
                                       rt_size_t             size,
                                       rt_uint32_t           tx_flag)
{
    return size;
}

static const struct rt_uart_ops sc16is752_uart_ops =
{
    .configure = sc16is752_configure,
    .control = sc16is752_control,
    .putc = sc16is752_putc,
    .getc = sc16is752_getc,
    .transmit = sc16is752_transmit
};

static int sc16is752_init(void)
{
    rt_err_t result = RT_EOK;

    sc16is752_config.irq_pin = rt_pin_get(sc16is752_config.irq);
    sc16is752_config.rst_pin = rt_pin_get(sc16is752_config.rst);

    for (uint8_t i = 0; i < sizeof(sc16is752_config.serial) / sizeof(sc16is752_config.serial[0]); i++)
    {
        /* init UART object */
        sc16is752_config.serial[i].ops = &sc16is752_uart_ops;
        /* register UART device */
        result = rt_hw_serial_register(
                &sc16is752_config.serial[i],
                sc16is752_config.uart_name[i],
                RT_DEVICE_FLAG_RDWR,
                NULL);
    }
    if (result == RT_EOK)
    {
        LOG_I("register success");
    }
    else
    {
        LOG_E("register failed");
        return -RT_ERROR;
    }

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_PREV_EXPORT(sc16is752_init);

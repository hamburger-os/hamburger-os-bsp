/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-19     zm       the first version
 */

#include "board.h"

#ifdef BSP_USING_KSZ8794

#include "drv_spi.h"

#define DBG_TAG "ksz8794"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define KSZ8794_SPIREAD_COMMAND       0x6000      /*SPI 读寄存器指令 */
#define KSZ8794_SPIWRITE_COMMAND      0x4000      /*SPI 写寄存器指令 */

#define SPI_COMMAND_TIMEOUT           5           /*SPI 传输指令和地址超时时间 */
#define SPI_DATA_TIMEOUT              100          /*SPI 传输读写数据超时时间 */

/* 寄存器地址描述 */
#define KSZ8794_CHIPID0                 0
#define KSZ8794_CHIPIDSWITCH            1
#define KSZ8794_GLOBALCONTROL0          2
#define KSZ8794_GLOBALCONTROL1          3
#define KSZ8794_GLOBALCONTROL2          4
#define KSZ8794_GLOBALCONTROL3          5
#define KSZ8794_GLOBALCONTROL4          6
#define KSZ8794_GLOBALCONTROL5          7
#define KSZ8794_GLOBALCONTROL6          8
#define KSZ8794_GLOBALCONTROL7          9
#define KSZ8794_GLOBALCONTROL8          0x0A
#define KSZ8794_GLOBALCONTROL9          0x0B
#define KSZ8794_GLOBALCONTROL10         0x0C
#define KSZ8794_GLOBALCONTROL11         0x0D
#define KSZ8794_POWERCONTROL1           0x0E
#define KSZ8794_POWERCONTROL2           0x0F

typedef struct {
    struct rt_device      dev;
    struct rt_spi_device  *spidev;
    char *                irqpin;
    rt_base_t             irq_pin_index;
    char *                rstpin;
    rt_base_t             rst_pin_index;
} KSZ8794Dev;

static KSZ8794Dev ksz8794_dev = {
        .irqpin = BSP_KSZ8794_IRQ_PIN,
        .rstpin = BSP_KSZ8794_RST_PIN,
};

static rt_err_t ksz8794_read_reg(KSZ8794Dev *ksz8794, uint32_t address, uint8_t *buffer, uint32_t length)
{
    rt_err_t ret = -RT_ERROR;

    if(NULL == ksz8794 || NULL == buffer)
    {
        return -RT_EINVAL;
    }

    uint16_t command = KSZ8794_SPIREAD_COMMAND | (address << 1);
    uint8_t send_buf[2];

    send_buf[0] = command >> 8;
    send_buf[1] = command;

    ret = rt_spi_send_then_recv(ksz8794->spidev, send_buf, 2, buffer, length);
    if(ret != RT_EOK)
    {
        LOG_E("read reg send error");
        return ret;
    }
    return ret;
}

static rt_err_t ksz8794_write_reg(KSZ8794Dev *ksz8794, uint32_t address, const uint8_t *buffer, uint32_t length)
{
    rt_err_t ret = -RT_ERROR;

    if(NULL == ksz8794 || NULL == buffer)
    {
        return -RT_EINVAL;
    }

    uint16_t command = KSZ8794_SPIWRITE_COMMAND | (address<<1);
    uint8_t send_buf[2];

    send_buf[0] = command >> 8;
    send_buf[1] = command;

    ret = rt_spi_send_then_send(ksz8794->spidev, send_buf, 2, buffer, length);
    if(ret != RT_EOK)
    {
        LOG_E("send error");
        return ret;
    }
    return ret;
}

static rt_size_t ksz8794_read (rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    KSZ8794Dev *ksz8794_dev = (KSZ8794Dev *)dev;

    if(ksz8794_read_reg(ksz8794_dev, pos, buffer, size) != RT_EOK)
    {
        return 0;
    }

    return size;
}

static rt_size_t ksz8794_write (rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    KSZ8794Dev *ksz8794_dev = (KSZ8794Dev *)dev;

    if(ksz8794_write_reg(ksz8794_dev, pos, buffer, size) != RT_EOK)
    {
        return 0;
    }
    return size;
}

static rt_err_t ksz8794_open (rt_device_t dev, rt_uint16_t oflag)
{
    uint8_t write_buf;

    write_buf = 1;
    if(ksz8794_write(dev, KSZ8794_CHIPIDSWITCH, (const void *)&write_buf, 1) != 1)
    {
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t ksz8794_close (rt_device_t dev)
{
    uint8_t write_buf;

    write_buf = 0;
    if(ksz8794_write(dev, KSZ8794_CHIPIDSWITCH, (const void *)&write_buf, 1) != 1)
    {
        return -RT_ERROR;
    }
    return RT_EOK;
}

static int rt_hw_ksz8794_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint8_t dev_num = 0;
    char dev_name[RT_NAME_MAX];

    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", BSP_KSZ8794_SPI_BUS, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

    rt_hw_spi_device_attach(BSP_KSZ8794_SPI_BUS, (const char *)dev_name, rt_pin_get(BSP_KSZ8794_SPI_CS_PIN));
    ksz8794_dev.spidev = (struct rt_spi_device *)rt_device_find(dev_name);
    if (NULL == ksz8794_dev.spidev)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }

    struct rt_spi_configuration cfg = {0};
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_3 | RT_SPI_MSB;
    cfg.max_hz = BSP_KSZ8794_SPI_SPEED;
    ret = rt_spi_configure(ksz8794_dev.spidev, &cfg);
    if (ret != RT_EOK)
    {
        LOG_E("device %s configure error %d!", dev_name, ret);
        return -RT_EIO;
    }

    //irq pin
    ksz8794_dev.rst_pin_index = rt_pin_get(ksz8794_dev.rstpin);
    rt_pin_mode(ksz8794_dev.rst_pin_index, (rt_base_t)PIN_MODE_INPUT);
    rt_pin_irq_enable(ksz8794_dev.rst_pin_index, (rt_uint32_t)PIN_IRQ_ENABLE);

    //rst pin
    ksz8794_dev.rst_pin_index = rt_pin_get(ksz8794_dev.rstpin);
    rt_pin_mode(ksz8794_dev.rst_pin_index, PIN_MODE_OUTPUT);

    rt_pin_write(ksz8794_dev.rst_pin_index, PIN_LOW);
    rt_thread_delay(40);
    rt_pin_write(ksz8794_dev.rst_pin_index, PIN_HIGH);
    rt_thread_delay(40);

    ksz8794_dev.dev.init = RT_NULL;
    ksz8794_dev.dev.open = ksz8794_open;
    ksz8794_dev.dev.close = ksz8794_close;
    ksz8794_dev.dev.read = ksz8794_read;
    ksz8794_dev.dev.write = ksz8794_write;
    ksz8794_dev.dev.control = RT_NULL;

    /* register ksz8794 device */
    if(rt_device_register(&ksz8794_dev.dev, "ksz8794", RT_DEVICE_FLAG_RDWR) == RT_EOK)
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
INIT_DEVICE_EXPORT(rt_hw_ksz8794_init);

#endif /* BSP_USING_KSZ8794 */



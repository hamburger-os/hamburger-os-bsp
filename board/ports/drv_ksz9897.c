/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-26     Administrator       the first version
 */
#include "drv_ksz9897.h"
#include "board.h"
#include "drv_spi.h"
#include <rtdbg.h>

#ifdef BSP_USING_KSZ9897
#define KSZ9897_WRITE_CMD 0x02
#define KSZ9897_READ_CMD  0x03


typedef struct {
    struct rt_device      dev;
    const char*           dev_name;
    struct rt_spi_device  *spidev;
    const char*           spi_bus_name;
    char *                spi_cs_pin;
} KSZ9897Dev;



static KSZ9897Dev ksz9897_dev[] =
{
#ifdef BSP_USING_KSZ9897_CHIP1
    /* KSZ9897 1*/
    {
        .dev_name = "ksz9896_1",
        .spi_bus_name = BSP_KSZ9897_CHIP1_SPI_BUS,
        .spi_cs_pin = BSP_KSZ9897_CHIP1_CS_PIN,
    },
#endif

#ifdef BSP_USING_KSZ9897_CHIP2
    /* KSZ9897 2*/
    {
        .dev_name = "ksz9896_2",
        .spi_bus_name = BSP_KSZ9897_CHIP2_SPI_BUS,
        .spi_cs_pin = BSP_KSZ9897_CHIP2_CS_PIN,
    },
#endif

#ifdef BSP_USING_KSZ9897_CHIP3
    /* KSZ9897 3*/
    {
        .dev_name = "ksz9896_3",
        .spi_bus_name = BSP_KSZ9897_CHIP3_SPI_BUS,
        .spi_cs_pin = BSP_KSZ9897_CHIP3_CS_PIN,
    },
#endif
};


/* 写寄存器 */
static rt_size_t ksz9897_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    rt_uint8_t cmd[4];
    rt_uint8_t data[4];
    KSZ9897Dev *pksz9897_dev;
    pksz9897_dev = (KSZ9897Dev*)dev;
    if((dev == RT_NULL) || (buffer == RT_NULL))
    {
        return -RT_ERROR;
    }

    switch (size)
    {
        case 1:
            data[0] = *(rt_uint8_t*)buffer;
            break;
        case 2:
            data[0] = *((rt_uint8_t*)buffer + 1);
            data[1] = *(rt_uint8_t*)buffer;
            break;
        case 4:
            data[0] = *((rt_uint8_t*)buffer + 3);
            data[1] = *((rt_uint8_t*)buffer + 2);
            data[2] = *((rt_uint8_t*)buffer + 1);
            data[3] = *(rt_uint8_t*)buffer;
            break;
        default:
            return -RT_ERROR;
            break;
    }
    cmd[0] = (rt_uint8_t)(KSZ9897_WRITE_CMD<<5);
    cmd[1] = (rt_uint8_t)(pos>>11);
    cmd[2] = (rt_uint8_t)((pos<<5)>>8);
    cmd[3] = (rt_uint8_t)((pos&0x07)<<5);

    if(4 != rt_spi_send(pksz9897_dev->spidev, cmd, 4))
    {
        return 0;
    }
    if(size != rt_spi_send(pksz9897_dev->spidev, data, size))
    {
        return 0;
    }

    return size;
}

/* 读寄存器 */
static rt_size_t ksz9897_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_uint8_t cmd[4];
    KSZ9897Dev *pksz9897_dev;
    pksz9897_dev = (KSZ9897Dev*)dev;
    if((dev == RT_NULL) || (buffer == RT_NULL))
    {
        return 0;
    }

    cmd[0] = (rt_uint8_t)(KSZ9897_READ_CMD<<5);
    cmd[1] = (rt_uint8_t)(pos>>11);
    cmd[2] = (rt_uint8_t)((pos<<5)>>8);
    cmd[3] = (rt_uint8_t)((pos&0x07)<<5);

    if(4 != rt_spi_send(pksz9897_dev->spidev, cmd, 4))
    {
        return 0;
    }

    if(4 != rt_spi_recv(pksz9897_dev->spidev, buffer, size))
    {
        return 0;
    }

    return size;
}

static void ksz9897_init(void)
{
    rt_uint8_t i,j;
    rt_uint16_t writeval;
    for(i = 0; i < 1; i++)
    {
        for(j = 0; j < 5; j++)
        {
            writeval = 0x0002;
            ksz9897_write(&ksz9897_dev[i].dev, j | 0x011A, &writeval, 2);

            writeval = 0x0000;
            ksz9897_write(&ksz9897_dev[i].dev, j | 0x011C, &writeval, 2);

            writeval = 0x4002;
            ksz9897_write(&ksz9897_dev[i].dev, j | 0x011A, &writeval, 2);

            writeval = 0x0010;
            ksz9897_write(&ksz9897_dev[i].dev, j | 0x011C, &writeval, 2);
        }
    }
}

static int rt_hw_ksz9897_init(void)
{
    rt_uint8_t i;
    rt_err_t ret = RT_EOK;
    rt_uint8_t dev_num = 0;
    char dev_name[RT_NAME_MAX];

    for(i = 0; i < 3; i++)
    {

        do
        {
            rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", ksz9897_dev[i].spi_bus_name, dev_num++);
            if (dev_num == 255)
            {
                return -RT_EIO;
            }
        } while (rt_device_find(dev_name));

        rt_hw_spi_device_attach(ksz9897_dev[i].spi_bus_name, (const char *)dev_name, rt_pin_get(ksz9897_dev[i].spi_cs_pin));
        ksz9897_dev[i].spidev = (struct rt_spi_device *)rt_device_find(dev_name);
        if (NULL == ksz9897_dev[i].spidev)
        {
            LOG_E("device %s find error!", dev_name);
            return -RT_EIO;
        }

        struct rt_spi_configuration cfg = {0};
        cfg.data_width = 8;
        cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_3 | RT_SPI_MSB;
        cfg.max_hz = BSP_KSZ9897_SPI_SPEED;
        ret = rt_spi_configure(ksz9897_dev[i].spidev, &cfg);
        if (ret != RT_EOK)
        {
            LOG_E("device %s configure error %d!", dev_name, ret);
            return -RT_EIO;
        }

        ksz9897_dev[i].dev.init = RT_NULL;
        ksz9897_dev[i].dev.open = RT_NULL;
        ksz9897_dev[i].dev.close = RT_NULL;
        ksz9897_dev[i].dev.read = ksz9897_read;
        ksz9897_dev[i].dev.write = ksz9897_write;
        ksz9897_dev[i].dev.control = RT_NULL;

        /* register ksz8794 device */
        if(rt_device_register(&ksz9897_dev[i].dev, ksz9897_dev[i].dev_name, RT_DEVICE_FLAG_RDWR) == RT_EOK)
        {
            LOG_I("register success");
        }
        else
        {
            LOG_E("register failed");
            return -RT_ERROR;
        }
    }
    ksz9897_init();
    return RT_EOK;
}

/* 导出到自动初始化 */
//INIT_DEVICE_EXPORT(rt_hw_ksz9897_init);
#endif


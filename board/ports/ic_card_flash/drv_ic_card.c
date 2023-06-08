/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-02-21     lvhan       the first version
 */
#include "board.h"

#ifdef BSP_USING_ICCARD_FLASH
#include "fal_cfg.h"

#define DBG_TAG "iccard"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "drv_spi.h"
#include "drv_soft_spi.h"

struct hnthinker_iccard_config
{
    struct rt_spi_device *spidev;
    rt_base_t power;
    rt_base_t busy;
    rt_base_t rst;
    rt_base_t isr;
    struct fal_flash_dev *faldev;
    struct rt_device *dev;

    rt_sem_t sem;
    rt_thread_t thread;

    int is_insert;
    int last_insert;
};
static struct hnthinker_iccard_config iccard_config = {0};

extern struct fal_flash_dev s25fl512_faldev;
extern struct fal_flash_dev at45db_faldev;

extern uint32_t at45db_read_id(struct rt_spi_device *device);
extern uint32_t s25fl512_read_id(struct rt_spi_device *device);
struct hnthinker_iccard_dev
{
    uint32_t (* read_id)(struct rt_spi_device *);
    struct fal_flash_dev *faldev;
};
static struct hnthinker_iccard_dev iccard_list[] =
{
    {
        .read_id = s25fl512_read_id,
        .faldev = &s25fl512_faldev,
    },
    {
        .read_id = at45db_read_id,
        .faldev = &at45db_faldev,
    },
};

static uint32_t iccard_read_id(void)
{
    uint32_t id = 0;
    for (uint8_t i = 0; i<sizeof(iccard_list)/sizeof(struct hnthinker_iccard_dev); i++)
    {
        struct hnthinker_iccard_dev *iccard = &iccard_list[i];
        id = iccard->read_id(iccard_config.spidev);
        if (id != 0)
        {
            iccard_config.faldev = iccard->faldev;
            return id;
        }
    }

    iccard_config.faldev = NULL;
    return 0;
}

static void iccard_power_set(uint8_t onoff)
{
    if (onoff > 0)
    {
        rt_pin_write(iccard_config.power, PIN_LOW);
    }
    else
    {
        rt_pin_write(iccard_config.power, PIN_HIGH);
    }
}

static void iccard_rst(void)
{
    rt_pin_write(iccard_config.rst, PIN_LOW);
    rt_thread_delay(100);
    rt_pin_write(iccard_config.rst, PIN_HIGH);
    rt_thread_delay(10);
}

static void iccard_isr_callback(void *args)
{
    struct hnthinker_iccard_config *config = (struct hnthinker_iccard_config *)args;
    config->is_insert = rt_pin_read(config->isr);
    if (config->is_insert > 0)
    {
        //卡插入
        config->is_insert = 1;
    }
    else
    {
        //卡拔出
        config->is_insert = 0;
    }
    if (config->last_insert != config->is_insert)
    {
        config->last_insert = config->is_insert;
        rt_sem_release(config->sem);
    }
}

static void iccard_thread_entry(void *parameter)
{
    struct hnthinker_iccard_config *config = (struct hnthinker_iccard_config *)parameter;

    while(1)
    {
        if (rt_sem_take(config->sem, RT_WAITING_FOREVER) == RT_EOK)
        {
            if (config->is_insert > 0)
            {
                LOG_D("ic card in");
                iccard_power_set(1);
                iccard_rst();
                uint32_t id = iccard_read_id();
                if (id != 0)
                {
                    LOG_I("id : 0x%x", id);
                    config->dev = fal_dev_mtd_nor_device_create(config->faldev);
                    if (config->dev == NULL)
                    {
                        LOG_E("Failed to creat nor %s!", config->faldev->name);
                    }
                }
                else
                {
                    LOG_E("Unrecognized ic card model");
                }
            }
            else
            {
                LOG_D("ic card out");
                iccard_power_set(0);
                /* unregister device */
                if (rt_device_unregister(config->dev) == RT_EOK)
                {
                    LOG_I("The block device (%s) unregister successfully", config->dev->parent.name);
                }
                else
                {
                    LOG_E("The block device (%s) unregister failed!", config->dev->parent.name);
                }

                config->dev = NULL;
                config->faldev = NULL;
            }
        }
        else
        {
            LOG_E("take sem error!");
        }
    }
}

static int rt_hw_iccard_init(void)
{
    rt_err_t ret = RT_EOK;

    /* 初始化spi */
    rt_uint8_t dev_num = 0;
    char dev_name[RT_NAME_MAX];
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", BSP_ICCARD_FLASH_SPI_BUS, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

//    rt_hw_soft_spi_device_attach(BSP_ICCARD_FLASH_SPI_BUS, dev_name, BSP_ICCARD_FLASH_SPI_CS_PIN);
    rt_hw_spi_device_attach(BSP_ICCARD_FLASH_SPI_BUS, dev_name, rt_pin_get(BSP_ICCARD_FLASH_SPI_CS_PIN));
    iccard_config.spidev = (struct rt_spi_device *)rt_device_find(dev_name);
    if (iccard_config.spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }
    struct rt_spi_configuration cfg = {0};
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = BSP_ICCARD_FLASH_SPI_SPEED;
    ret = rt_spi_configure(iccard_config.spidev, &cfg);
    if (ret != RT_EOK)
    {
        LOG_E("device %s configure error %d!", dev_name, ret);
        return -RT_EIO;
    }
    /* 初始化控制引脚 */
    iccard_config.power = rt_pin_get(BSP_ICCARD_FLASH_POWER_PIN);
    rt_pin_mode(iccard_config.power, PIN_MODE_OUTPUT);
    iccard_power_set(0);

    iccard_config.busy = rt_pin_get(BSP_ICCARD_FLASH_BUSY_PIN);
    rt_pin_mode(iccard_config.busy, PIN_MODE_INPUT);

    iccard_config.rst = rt_pin_get(BSP_ICCARD_FLASH_RST_PIN);
    rt_pin_mode(iccard_config.rst, PIN_MODE_OUTPUT);

    iccard_config.isr = rt_pin_get(BSP_ICCARD_FLASH_ISR_PIN);
    rt_pin_mode(iccard_config.isr, PIN_MODE_INPUT_PULLDOWN);
    /* 绑定中断，不插卡时低，插卡了变高 */
    rt_pin_attach_irq(iccard_config.isr, PIN_IRQ_MODE_RISING, iccard_isr_callback, &iccard_config);
    /* 使能中断 */
    rt_pin_irq_enable(iccard_config.isr, PIN_IRQ_ENABLE);

    /* 创建信号量 */
    iccard_config.sem = rt_sem_create("iccard", 0, RT_IPC_FLAG_PRIO);
    if (iccard_config.sem == RT_NULL)
    {
        LOG_E("semaphore creat failed!");
        return -RT_ENOMEM;
    }
    /* 创建热插拔host线程 */
    iccard_config.thread = rt_thread_create( "iccard",
                                            iccard_thread_entry,
                                            &iccard_config,
                                            2048,
                                            15,
                                            10);
    if ( iccard_config.thread != RT_NULL)
    {
        rt_thread_startup(iccard_config.thread);
    }
    else
    {
        LOG_E("thread creat error!");
        return -RT_ENOMEM;
    }

    /* 初始化时检查第一次卡插拔状态 */
    iccard_isr_callback(&iccard_config);

    return ret;
}
/* 导出到自动初始化 */
INIT_ENV_EXPORT(rt_hw_iccard_init);

#endif

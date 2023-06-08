/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-06     lvhan       the first version
 */
#include "board.h"

#include "fal.h"
#include "fal_cfg.h"
#include "drv_spi.h"
#include "spi_flash.h"
#include "spi_flash_sfud.h"

static int init(void);
static int read(long offset, uint8_t *buf, size_t size);
static int write(long offset, const uint8_t *buf, size_t size);
static int erase(long offset, size_t size);

static sfud_flash_t sfud_dev = NULL;
struct fal_flash_dev w25qxx_flash =
{
    .name       = SPI_FLASH_DEV_NAME,
    .addr       = SPI_FLASH_START_ADRESS,
    .len        = SPI_FLASH_SIZE_GRANULARITY_TOTAL,
    .blk_size   = SPI_FLASH_BLK_SIZE,
    .ops        = {init, read, write, erase},
    .write_gran = 1
};

static int init(void)
{
    rt_uint8_t dev_num = 0;
    char dev_name[RT_NAME_MAX];
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", BSP_SPI_FLASH_SPI_BUS, dev_num++);
        if (dev_num == 255)
        {
            return;
        }
    } while (rt_device_find(dev_name));

    rt_hw_spi_device_attach(BSP_SPI_FLASH_SPI_BUS, dev_name, rt_pin_get(BSP_SPI_FLASH_SPI_CS_PIN));

    if (RT_NULL == rt_sfud_flash_probe("w25qxx", dev_name))
    {
        return -RT_ERROR;
    }
    sfud_dev = rt_sfud_flash_find_by_dev_name("w25qxx");
    if (RT_NULL == sfud_dev)
    {
        return -1;
    }

    /* update the flash chip information */
    w25qxx_flash.blk_size = sfud_dev->chip.erase_gran;
    w25qxx_flash.len = sfud_dev->chip.capacity;

    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    sfud_read(sfud_dev, w25qxx_flash.addr + offset, size, buf);

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    if (sfud_write(sfud_dev, w25qxx_flash.addr + offset, size, buf) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}

static int erase(long offset, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    if (sfud_erase(sfud_dev, w25qxx_flash.addr + offset, size) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}

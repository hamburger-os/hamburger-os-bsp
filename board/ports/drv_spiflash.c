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

#ifdef BSP_USING_SPI_FLASH

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
struct fal_flash_dev spiflash =
{
    .name       = SPI_FLASH_DEV_NAME,
    .addr       = SPI_FLASH_START_ADRESS,
    .len        = SPI_FLASH_SIZE_GRANULARITY_TOTAL,
    .blk_size   = SPI_FLASH_BLK_SIZE,
    .ops        = {init, read, write, erase},
    .write_gran = 1
};

static char dev_name[RT_NAME_MAX];
static int sfud_spi_device_init(void)
{
    rt_uint8_t dev_num = 0;
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", BSP_SPI_FLASH_SPI_BUS, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

//    rt_hw_soft_spi_device_attach(BSP_SPI_FLASH_SPI_BUS, dev_name, BSP_SPI_FLASH_SPI_CS_PIN);
    rt_hw_spi_device_attach(BSP_SPI_FLASH_SPI_BUS, dev_name, rt_pin_get(BSP_SPI_FLASH_SPI_CS_PIN));
    struct rt_spi_device *spidev = (struct rt_spi_device *)rt_device_find(dev_name);
    if (spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }
    struct rt_spi_configuration cfg = {0};
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = BSP_SPI_FLASH_SPI_SPEED;
    if (rt_spi_configure(spidev, &cfg) != RT_EOK)
    {
        LOG_E("device %s configure error!", dev_name);
        return -RT_EIO;
    }

    return RT_EOK;
}
INIT_PREV_EXPORT(sfud_spi_device_init);

static int init(void)
{
    if (RT_NULL == rt_sfud_flash_probe("sfud", dev_name))
    {
        return -RT_ERROR;
    }
    sfud_dev = rt_sfud_flash_find_by_dev_name("sfud");
    if (RT_NULL == sfud_dev)
    {
        return -RT_ERROR;
    }

    /* update the flash chip information */
    if (spiflash.blk_size != sfud_dev->chip.erase_gran || spiflash.len != sfud_dev->chip.capacity)
    {
        LOG_W("blk_size (%d %d) or len(%d %d) set warning!"
                , spiflash.blk_size, sfud_dev->chip.erase_gran
                , spiflash.len, sfud_dev->chip.capacity);
        spiflash.blk_size = sfud_dev->chip.erase_gran;
        spiflash.len = sfud_dev->chip.capacity;
    }

    LOG_I("init succeed %d MB.", spiflash.len/1024/1024);
    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);

    sfud_err result = SFUD_SUCCESS;
    uint32_t addr = spiflash.addr + offset;
    if (size < 1)
    {
//        LOG_W("read size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    result = sfud_read(sfud_dev, addr, size, buf);
    if (result != SFUD_SUCCESS)
    {
        LOG_E("read data error %d!", result);
        return -RT_EIO;
    }

    LOG_HEX("read", 16, buf, (size > 128)?(128):(size));
    LOG_D("read (0x%p) %d", (void*)(addr), size);
    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);

    sfud_err result = SFUD_SUCCESS;
    uint32_t addr = spiflash.addr + offset;
    if (size < 1)
    {
//        LOG_W("write size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    result = sfud_write(sfud_dev, addr, size, buf);
    if (result != SFUD_SUCCESS)
    {
        LOG_E("write data error %d!", result);
        return -RT_EIO;
    }

    LOG_HEX("write", 16, (uint8_t *)buf, (size > 128)?(128):(size));
    LOG_D("write (0x%p) %d", (void*)(addr), size);
    return size;
}

static int erase(long offset, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);

    sfud_err result = SFUD_SUCCESS;
    rt_uint32_t addr = spiflash.addr + offset;
    if (size < 1)
    {
//        LOG_W("erase size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    rt_tick_t tick = rt_tick_get();
    result = sfud_erase(sfud_dev, addr, size);
    if (result != SFUD_SUCCESS)
    {
        LOG_E("erase data error %d!", result);
        return -RT_EIO;
    }

    LOG_D("erase (0x%p) %d used %d ms", (void*)(addr), size, rt_tick_get() - tick);
    return size;
}

#ifdef SPI_FLASH_ENABLE_TEST
static int spiflash_test(void)
{
    rt_err_t ret = RT_EOK;
    uint32_t testsize = spiflash.blk_size;
    LOG_I("spiflash read write test start...");

    char *tbuf = rt_malloc(testsize);
    char *rbuf = rt_malloc(testsize);
    if (tbuf == NULL || rbuf == NULL)
    {
        rt_free(tbuf);
        rt_free(rbuf);
        LOG_W("Not enough memory to request a block.");

        testsize = 1024;
        tbuf = rt_malloc(testsize);
        rbuf = rt_malloc(testsize);
        if (tbuf == NULL || rbuf == NULL)
        {
            rt_free(tbuf);
            rt_free(rbuf);
            LOG_E("Not enough memory to complete the test.");
            return -RT_ERROR;
        }
    }

    for(uint32_t i = 0; i<testsize; i+=4)
    {
        uint32_t *data32 = (uint32_t *)&tbuf[i];
        *data32 = i;
    }

    rt_tick_t tick1, tick2, tick3, tick4;
    for (uint32_t offset = 0; offset <= spiflash.len; offset += spiflash.blk_size * 64)
    {
        if (offset == spiflash.len)
        {
            offset -= spiflash.blk_size;
        }
        tick1 = rt_tick_get_millisecond();
        erase(offset, testsize);
        tick2 = rt_tick_get_millisecond();
        write(offset, (uint8_t *)tbuf, testsize);
        tick3 = rt_tick_get_millisecond();
        read (offset, (uint8_t *)rbuf, testsize);
        tick4 = rt_tick_get_millisecond();

        LOG_I("erase %08p %d use %u ms", offset, testsize, tick2 - tick1);
        LOG_I("write %08p %d use %u ms: %x %x", offset, testsize, tick3 - tick2, *(uint32_t *)&tbuf[0], *(uint32_t *)&tbuf[testsize-4]);
        LOG_I("read  %08p %d use %u ms: %x %x", offset, testsize, tick4 - tick3, *(uint32_t *)&rbuf[0], *(uint32_t *)&rbuf[testsize-4]);

        if (rt_memcmp(tbuf, rbuf, testsize) != 0)
        {
            LOG_E("test %08p %d failed!", offset, testsize);
            ret = -RT_ERROR;
        }
        rt_memset(rbuf, 0, testsize);
    }

    if (ret == RT_EOK)
    {
        LOG_I("spiflash read write test succsess.");
    }
    else
    {
        LOG_E("spiflash read write test failed!");
    }

    rt_free(tbuf);
    rt_free(rbuf);

    return ret;
}
MSH_CMD_EXPORT(spiflash_test, sfud spiflash test);
#endif

#endif

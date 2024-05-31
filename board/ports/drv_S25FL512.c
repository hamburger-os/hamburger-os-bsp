/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-08     lvhan       the first version
 */

#include "board.h"

#include "fal_cfg.h"

#define DBG_TAG "s25fl512"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#include "drv_spi.h"
#include "drv_soft_spi.h"

#if defined(RT_USING_FAL)
#include "fal.h"

static struct rt_spi_device *s25fl512_spidev;

static int fal_s25fl512_init(void);
static int fal_s25fl512_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_s25fl512_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_s25fl512_erase(long offset, size_t size);

/* ===================== Flash device Configuration ========================= */
const struct fal_flash_dev s25fl512_flash =
{
    .name = S25FL512_DEV_NAME,
    .addr = S25FL512_START_ADRESS,
    .len = S25FL512_SIZE_GRANULARITY_TOTAL,
    .blk_size = S25FL512_BLK_SIZE,
    .ops = {fal_s25fl512_init, fal_s25fl512_read, fal_s25fl512_write, fal_s25fl512_erase},
    .write_gran = 0,
};

static rt_err_t s25fl512_readDeviceID(uint16_t *id)
{
    rt_err_t ret = RT_EOK;

    uint8_t cmd[4] = {0x90, 0, 0, 0};
    ret = rt_spi_send_then_recv(s25fl512_spidev, cmd, 4, id, 2);
    if (ret != RT_EOK)
    {
        LOG_E("read id error %d!", ret);
        ret = -RT_EIO;
    }
    if (*id == 0x1901)
    {
        LOG_I("init succeed 0x%x.", *id);
    }
    else
    {
        LOG_E("init failed 0x%x != 0x1901.", *id);
    }

    return ret;
}

static rt_err_t s25fl512_writeEnable(void)
{
    rt_err_t ret = RT_EOK;

    uint8_t cmd[] = {0x06};
    rt_size_t size = rt_spi_send(s25fl512_spidev, cmd, sizeof(cmd));
    if (size != sizeof(cmd))
    {
        LOG_E("write enable error %d!", ret);
        ret = -RT_EIO;
    }

    return ret;
}

static rt_err_t s25fl512_writeDisable(void)
{
    rt_err_t ret = RT_EOK;

    uint8_t cmd[] = {0x04};
    rt_size_t size = rt_spi_send(s25fl512_spidev, cmd, sizeof(cmd));
    if (size != sizeof(cmd))
    {
        LOG_E("write disable error %d!", ret);
        ret = -RT_EIO;
    }

    return ret;
}

static void s25fl512_wait_busy(void)
{
    uint8_t value = 0x01;
    /* 发送读命令 */
    uint8_t cmd[] = {0x05};

    rt_tick_t tick = rt_tick_get();
    while((value & 0x01) == 0x01)
    {
        /* 读数据 */
        rt_err_t ret = rt_spi_send_then_recv(s25fl512_spidev, cmd, sizeof(cmd), &value, 1);
        if (ret != RT_EOK)
        {
            LOG_E("wait busy error %d!", ret);
            break;
        }
        rt_thread_delay(1);   // 等待BUSY位清空
        if (rt_tick_get() - tick > 1000)
        {
            LOG_E("wait busy timeout! wait 0x%x %d ms", value, rt_tick_get() - tick);
            break;
        }
    }
    LOG_D("wait 0x%x %d ms", value, rt_tick_get() - tick);
}

static int s25fl512_spi_device_init(void)
{
    char dev_name[RT_NAME_MAX];
    rt_uint8_t dev_num = 0;
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", BSP_S25FL512_SPI_BUS, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

//    rt_hw_soft_spi_device_attach(BSP_S25FL512_SPI_BUS, dev_name, rt_pin_get(BSP_S25FL512_SPI_CS_PIN));
    rt_hw_spi_device_attach(BSP_S25FL512_SPI_BUS, dev_name, rt_pin_get(BSP_S25FL512_SPI_CS_PIN));
    s25fl512_spidev = (struct rt_spi_device *)rt_device_find(dev_name);
    if (s25fl512_spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }
    struct rt_spi_configuration cfg = {0};
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = BSP_S25FL512_SPI_SPEED;
    if (rt_spi_configure(s25fl512_spidev, &cfg) != RT_EOK)
    {
        LOG_E("device %s configure error!", dev_name);
        return -RT_EIO;
    }

    return RT_EOK;
}
INIT_PREV_EXPORT(s25fl512_spi_device_init);

static int fal_s25fl512_init(void)
{
    rt_err_t ret = RT_EOK;

    uint16_t id = {0};
    ret = s25fl512_readDeviceID(&id);

    return ret;
}

static int fal_s25fl512_read(long offset, rt_uint8_t *buf, size_t size)
{
    uint32_t addr = s25fl512_flash.addr + offset;
    if (addr + size > s25fl512_flash.addr + s25fl512_flash.len)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("read size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    /* 发送读命令 */
    uint8_t cmd[] = { 0x13, (uint8_t) ((addr) >> 24U), (uint8_t) ((addr) >> 16U), (uint8_t) ((addr) >> 8U), (uint8_t) ((addr)) };

    /* 读数据 */
    rt_err_t ret = rt_spi_send_then_recv(s25fl512_spidev, cmd, sizeof(cmd), buf, size);
    if (ret != RT_EOK)
    {
        LOG_E("read data error %d!", ret);
        return -RT_EIO;
    }

    LOG_HEX("read", 16, buf, (size > 64)?(64):(size));
    LOG_D("read (0x%p) %d", (void*)(addr), size);

    return size;
}

#define PAGE_SIZE 512
static int fal_s25fl512_write(long offset, const rt_uint8_t *buf, size_t size)
{
    uint32_t addr = s25fl512_flash.addr + offset;
    if (addr + size > s25fl512_flash.addr + s25fl512_flash.len)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("write size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    uint32_t addr_page = addr;
    uint8_t *buf_page = (uint8_t *)buf;
    size_t size_less = size;
    size_t size_page = PAGE_SIZE;
    size_t countmax = (size%PAGE_SIZE == 0)?(size/PAGE_SIZE):(size/PAGE_SIZE + 1);
    for (size_t count = 0; count < countmax; count++)
    {
        s25fl512_writeEnable();

        /* 计算页长度 */
        if (size_less >= PAGE_SIZE)
        {
            size_page = PAGE_SIZE;
        }
        else
        {
            size_page = size_less % PAGE_SIZE;
        }

        /* 发送写命令 */
        uint8_t cmd[] = {0x12, (uint8_t)((addr_page) >> 24U), (uint8_t)((addr_page) >> 16U), (uint8_t)((addr_page) >> 8U), (uint8_t)((addr_page))};

        /* 写数据 */
        rt_err_t ret = rt_spi_send_then_send(s25fl512_spidev, cmd, sizeof(cmd), buf_page, size_page);
        if (ret != RT_EOK)
        {
            LOG_E("write data error %d!", ret);
            return -RT_EIO;
        }
        addr_page += size_page;
        buf_page += size_page;
        size_less -= size_page;

        s25fl512_writeDisable();
        s25fl512_wait_busy();
    }

    LOG_HEX("write", 16, (uint8_t *)buf, (size > 64)?(64):(size));
    LOG_D("write (0x%p) %d", (void*)(addr), size);

    return size;
}

static int fal_s25fl512_erase(long offset, size_t size)
{
    rt_uint32_t addr = s25fl512_flash.addr + offset;
    if ((addr + size) > s25fl512_flash.addr + s25fl512_flash.len)
    {
        LOG_E("erase outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("erase size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    uint32_t addr_blk = addr;
    size_t countmax = (size%s25fl512_flash.blk_size == 0)?(size/s25fl512_flash.blk_size):(size/s25fl512_flash.blk_size + 1);
    for (size_t count = 0; count < countmax; count++)
    {
        s25fl512_writeEnable();

        /* 发送擦命令 */
        uint8_t cmd[] = {0xDC, (uint8_t)((addr_blk) >> 24U), (uint8_t)((addr_blk) >> 16U), (uint8_t)((addr_blk) >> 8U), (uint8_t)((addr_blk))};

        /* 擦数据 */
        rt_err_t ret = rt_spi_send(s25fl512_spidev, cmd, sizeof(cmd));
        if (ret != sizeof(cmd))
        {
            LOG_E("erase error %d!", ret);
            return -RT_EIO;
        }
        addr_blk += s25fl512_flash.blk_size;

        s25fl512_writeDisable();
        s25fl512_wait_busy();
    }

    LOG_D("erase (0x%p) %d", (void*)(addr), size);

    return size;
}

#endif

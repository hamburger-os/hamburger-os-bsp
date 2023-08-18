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

#define DBG_TAG "at45db321e"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#include "drv_spi.h"
#include "drv_soft_spi.h"

#if defined(RT_USING_FAL)
#include "fal.h"

static struct rt_spi_device *at45db321e_spidev;

static int fal_at45db321e_init(void);
static int fal_at45db321e_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_at45db321e_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_at45db321e_erase(long offset, size_t size);

/* ===================== Flash device Configuration ========================= */
const struct fal_flash_dev at45db321e_flash =
{
    .name = AT45DB321E_DEV_NAME,
    .addr = AT45DB321E_START_ADRESS,
    .len = AT45DB321E_SIZE_GRANULARITY_TOTAL,
    .blk_size = AT45DB321E_BLK_SIZE,
    .ops = {fal_at45db321e_init, fal_at45db321e_read, fal_at45db321e_write, fal_at45db321e_erase},
    .write_gran = 0,
};

static rt_err_t at45db321e_readDeviceID(uint8_t *id)
{
    rt_err_t ret = RT_EOK;

    uint8_t cmd[] = {0x9F};
    ret = rt_spi_send_then_recv(at45db321e_spidev, cmd, sizeof(cmd), id, 5);
    if (ret != RT_EOK)
    {
        LOG_E("read id error %d!", ret);
        ret = -RT_EIO;
    }
    uint8_t check[] = {0x1f, 0x27, 0x01, 0x01, 0x00};
    if (rt_memcmp(id, check, 5) == 0)
    {
        LOG_I("init succeed 0x%02x %02x %02x %02x %02x.", id[0], id[1], id[2], id[3], id[4]);
    }
    else
    {
        LOG_E("init failed 0x%02x %02x %02x %02x %02x.", id[0], id[1], id[2], id[3], id[4]);
    }

    return ret;
}


static void at45db321e_wait_busy(void)
{
    uint8_t value[2] = {0};
    /* 发送读命令 */
    uint8_t cmd[] = {0xD7};

    rt_tick_t tick = rt_tick_get();
    while((value[0] & 0x80) == 0x00)
    {
        /* 读数据 */
        rt_err_t ret = rt_spi_send_then_recv(at45db321e_spidev, cmd, sizeof(cmd), value, 2);
        if (ret != RT_EOK)
        {
            LOG_E("wait busy error %d!", ret);
            break;
        }
        rt_thread_delay(1);   // 等待BUSY位清空
    }

    LOG_HEX("wait", 16, value, 2);
    LOG_D("wait 0x%x %d ms", value[0], rt_tick_get() - tick);
}

static int at45db321e_spi_device_init(void)
{
    char dev_name[RT_NAME_MAX];
    rt_uint8_t dev_num = 0;
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", BSP_AT45DB321E_SPI_BUS, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

//    rt_hw_soft_spi_device_attach(BSP_AT45DB321E_SPI_BUS, dev_name, BSP_AT45DB321E_SPI_CS_PIN);
    rt_hw_spi_device_attach(BSP_AT45DB321E_SPI_BUS, dev_name, rt_pin_get(BSP_AT45DB321E_SPI_CS_PIN));
    at45db321e_spidev = (struct rt_spi_device *)rt_device_find(dev_name);
    if (at45db321e_spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }
    struct rt_spi_configuration cfg = {0};
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = BSP_AT45DB321E_SPI_SPEED;
    if (rt_spi_configure(at45db321e_spidev, &cfg) != RT_EOK)
    {
        LOG_E("device %s configure error!", dev_name);
        return -RT_EIO;
    }

    return RT_EOK;
}
INIT_PREV_EXPORT(at45db321e_spi_device_init);

static int fal_at45db321e_init(void)
{
    rt_err_t ret = RT_EOK;

    uint8_t id[5];
    ret = at45db321e_readDeviceID(id);

    // 软件复位
    uint8_t cmd_rst[] = {0xF0, 0x00, 0x00, 0x00};
    rt_size_t size = rt_spi_send(at45db321e_spidev, cmd_rst, sizeof(cmd_rst));
    if (size != sizeof(cmd_rst))
    {
        LOG_E("Reset error!");
        ret = -RT_EIO;
    }

    // 设置页大小512字节
    uint8_t cmd[] = {0x3D, 0x2A, 0x80, 0xA6};
    size = rt_spi_send(at45db321e_spidev, cmd, sizeof(cmd));
    if (size != sizeof(cmd))
    {
        LOG_E("Configure \"Power of 2\" (Binary) Page Size error!");
        ret = -RT_EIO;
    }

    return ret;
}

#define PAGE_SIZE 512

static int fal_at45db321e_read(long offset, rt_uint8_t *buf, size_t size)
{
    uint32_t addr = at45db321e_flash.addr + offset;
    if (addr + size > at45db321e_flash.addr + at45db321e_flash.len)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("read size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    uint32_t addr_page = addr;
    uint8_t *buf_page = buf;
    size_t size_less = size;
    size_t size_page = PAGE_SIZE;
    size_t countmax = (size%PAGE_SIZE == 0)?(size/PAGE_SIZE):(size/PAGE_SIZE + 1);
    for (size_t count = 0; count < countmax; count++)
    {
        /* 计算页长度 */
        if (size_less >= PAGE_SIZE)
        {
            size_page = PAGE_SIZE;
        }
        else
        {
            size_page = size_less % PAGE_SIZE;
        }

        /* 发送读命令 */
        uint32_t page = (addr_page / PAGE_SIZE) << 9;
        uint8_t cmd[] = {0xD2, (uint8_t)((page) >> 16U), (uint8_t)((page) >> 8U), (uint8_t)((page)),
                         (uint8_t)((0)), (uint8_t)((0)), (uint8_t)((0)), (uint8_t)((0))};
        /* 读数据 */
        rt_err_t ret = rt_spi_send_then_recv(at45db321e_spidev, cmd, sizeof(cmd), buf_page, size_page);
        if (ret != RT_EOK)
        {
            LOG_E("read data error %d!", ret);
            return -RT_EIO;
        }

        addr_page += PAGE_SIZE;
        buf_page += PAGE_SIZE;
        size_less -= PAGE_SIZE;
    }

    LOG_HEX("read", 16, buf, (size > 64)?(64):(size));
    LOG_D("read (0x%p) %d", (void*)(addr), size);

    return size;
}

static int fal_at45db321e_write(long offset, const rt_uint8_t *buf, size_t size)
{
    uint32_t addr = at45db321e_flash.addr + offset;
    if (addr + size > at45db321e_flash.addr + at45db321e_flash.len)
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
        uint32_t page = (addr_page / PAGE_SIZE) << 9;
        uint8_t cmd[] = {0x87, (uint8_t)((page) >> 16U), (uint8_t)((page) >> 8U), (uint8_t)((page))};

        /* 向buffer(sram)中写数据 */
        rt_err_t ret = rt_spi_send_then_send(at45db321e_spidev, cmd, sizeof(cmd), buf_page, size_page);
        if (ret != RT_EOK)
        {
            LOG_E("write buffer error!");
            return -RT_EIO;
        }

        /* 将buffer(sram)中的数据写入到mem中 */
        cmd[0] = 0x89;
        ret = rt_spi_send(at45db321e_spidev, cmd, sizeof(cmd));
        if (ret != sizeof(cmd))
        {
            LOG_E("buffer to mem error!");
            return -RT_EIO;
        }
        at45db321e_wait_busy();

        addr_page += PAGE_SIZE;
        buf_page += PAGE_SIZE;
        size_less -= PAGE_SIZE;
    }

    LOG_HEX("write", 16, (uint8_t *)buf, (size > 64)?(64):(size));
    LOG_D("write (0x%p) %d", (void*)(addr), size);

    return size;
}

static int fal_at45db321e_erase(long offset, size_t size)
{
    rt_uint32_t addr = at45db321e_flash.addr + offset;
    if ((addr + size) > at45db321e_flash.addr + at45db321e_flash.len)
    {
        LOG_E("erase outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
        // LOG_W("erase size %d! addr is (0x%p)", size, (void *)(addr + size));
        return 0;
    }

    uint32_t addr_blk = addr;
    size_t countmax = (size % at45db321e_flash.blk_size == 0) ? (size / at45db321e_flash.blk_size) : (size / at45db321e_flash.blk_size + 1);
    for (size_t count = 0; count < countmax; count++)
    {
        /* 发送擦命令 */
        uint32_t page = (addr_blk / PAGE_SIZE) << 9;
        uint8_t cmd[] = {0x81, (uint8_t)((page) >> 16U), (uint8_t)((page) >> 8U), (uint8_t)((page))};

        /* 擦数据 */
        rt_err_t ret = rt_spi_send(at45db321e_spidev, cmd, sizeof(cmd));
        if (ret != sizeof(cmd))
        {
            LOG_E("erase error %d!", ret);
            return -RT_EIO;
        }
        addr_blk += at45db321e_flash.blk_size;

        at45db321e_wait_busy();
    }

    LOG_D("erase (0x%p) %d", (void *)(addr), size);

    return size;
}

#endif

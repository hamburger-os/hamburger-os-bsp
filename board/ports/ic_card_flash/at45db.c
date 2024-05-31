/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-31     lvhan       the first version
 */
#include "board.h"

#ifdef BSP_USING_ICCARD_FLASH

#include "fal_cfg.h"

#define DBG_TAG "at45db"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "drv_spi.h"
#include "drv_soft_spi.h"

#if defined(RT_USING_FAL)
#include "fal.h"

#define CMD_STATUS_READ_D7 ((uint8_t)0xD7)
#define CMD_READ_DEVICE_ID ((uint8_t)0x9F)
#define CMD_READ_SEC_REG ((uint8_t)0x77)
#define CMD_READ_PAGE ((uint8_t)0xD2)
#define CMD_WR_BUF1 ((uint8_t)0x84)
#define CMD_PRG_BUF1TOPAG ((uint8_t)0x83)
#define CMD_PRGNE_BUF1TOPAG ((uint8_t)0x88) /*  无内建擦除的页编程命令 */
#define CMD_BLK_ERASE ((uint8_t)0x50)       /*  块擦除命令 */
#define CMD_ANYDATA ((uint8_t)0x00)

#define IC4M_PAGES_PER_BLK (8)     /*  4M卡每块页数 */
#define IC4M_EXP_PAGES_PER_BLK (3) /*  4M卡每块页数对2的指数 */

#define DEV_AT45DB161_STATUS 0xac
#define DEV_AT45DB321 0x06
#define DEV_AT45DB321_STATUS 0xb4
#define DEV_AT45DB642 0x07
#define DEV_AT45DB642_STATUS 0xbc

#define AT45DB161D_DEVICE_ID (0x0000261FU)
#define AT45DB321D_DEVICE_ID (0x0001271FU)
#define AT45DB321E_DEVICE_ID (0x0101271FU)
#define AT45DB642D_DEVICE_ID (0x0000281FU)
#define SECURITY_REG_LEN (128)

static int fal_at45db_init(void);
static int fal_at45db_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_at45db_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_at45db_erase(long offset, size_t size);

/* ===================== Flash device Configuration ========================= */
static struct rt_spi_device *at45db_spidev = NULL;
struct fal_flash_dev at45db_faldev =
{
    .name = "at45db",
    .addr = 0,
    .len = 0x400000,
    .blk_size = 512,
    .ops = {fal_at45db_init, fal_at45db_read, fal_at45db_write, fal_at45db_erase},
    .write_gran = 0,
};

uint32_t at45db_read_id(struct rt_spi_device *device)
{
    rt_err_t ret = RT_EOK;
    at45db_spidev = device;

    uint32_t id = 0;
    uint8_t command[] = {CMD_READ_DEVICE_ID};
    ret = rt_spi_send_then_recv(at45db_spidev, command, 1, &id, 4);
    if (ret == RT_EOK)
    {
        //根据卡型号重新配置容量和块大小
        if (id == AT45DB161D_DEVICE_ID)
        {
            return id;
        }
        else if (id == AT45DB321D_DEVICE_ID)
        {
            return id;
        }
        else if (id == AT45DB321E_DEVICE_ID)
        {
            at45db_faldev.len = 4*1024*1024;
            at45db_faldev.blk_size = 512;
            return id;
        }
        else if (id == AT45DB642D_DEVICE_ID)
        {
            return id;
        }
        else
        {
            LOG_D("id : 0x%x", id);
        }
    }
    else
    {
        LOG_E("read id error 0x%x!", id);
    }

    return 0;
}

static int fal_at45db_init(void)
{
    LOG_I("init succeed.");
    return RT_EOK;
}

static int fal_at45db_read(long offset, rt_uint8_t *buf, size_t size)
{
    uint32_t addr = at45db_faldev.addr + offset;
    if (addr + size > at45db_faldev.addr + at45db_faldev.len)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
        LOG_W("read size %d! addr is (0x%p)", size, (void*)(addr + size));
        return -RT_EINVAL;
    }

    rt_err_t ret = RT_EOK;
    /* 发送读命令 */
    uint8_t cmd[] = {CMD_ANYDATA, (uint8_t)((addr) >> 16U), (uint8_t)((addr) >> 8U), (uint8_t)((addr))};

    /* 读数据 */
    ret = rt_spi_send_then_recv(at45db_spidev, cmd, sizeof(cmd), buf, size);
    if (ret != RT_EOK)
    {
        LOG_E("read data error %d!", ret);
        return -RT_EIO;
    }

    LOG_HEX("read", 16, buf, (size > 64)?(64):(size));
    LOG_D("read (0x%p) %d", (void*)(addr), size);

    return size;
}

static int fal_at45db_write(long offset, const rt_uint8_t *buf, size_t size)
{
    uint32_t addr = at45db_faldev.addr + offset;
    if (addr + size > at45db_faldev.addr + at45db_faldev.len)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
        LOG_W("write size %d! addr is (0x%p)", size, (void*)(addr + size));
        return -RT_EINVAL;
    }



    LOG_HEX("write", 16, (uint8_t *)buf, (size > 64)?(64):(size));
    LOG_D("write (0x%p) %d", (void*)(addr), size);

    return size;
}

static int fal_at45db_erase(long offset, size_t size)
{
    rt_uint32_t addr = at45db_faldev.addr + offset;
    if ((addr + size) > at45db_faldev.addr + at45db_faldev.len)
    {
        LOG_E("erase outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
        LOG_W("erase size %d! addr is (0x%p)", size, (void*)(addr + size));
        return -RT_EINVAL;
    }



    LOG_D("erase (0x%p) %d", (void*)(addr), size);

    return size;
}

#endif
#endif

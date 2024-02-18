/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-02-09     lvhan       the first version
 */

/*
 * 此文件的fal框架与内核中的不同
 * 内核中的fal框架最大支持4GB的存储设备
 * 此修改版本无需事先定义fal设备列表
 * 更加适合存储大小在一开始不能确定的存储设备
 */

#include <board.h>
#include "drv_fal.h"

#define DBG_TAG "drv.fal"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

struct fal_mtd_nor_device
{
    struct rt_mtd_nor_device       parent;
    struct fal_flash64_dev         *fal_dev;
};

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_ssize_t mtd_nor_dev_read(struct rt_mtd_nor_device* device, rt_off_t offset, rt_uint8_t* data, rt_size_t length)
#else
static rt_size_t mtd_nor_dev_read(struct rt_mtd_nor_device* device, rt_off_t offset, rt_uint8_t* data, rt_uint32_t length)
#endif
{
    int ret = 0;
    struct fal_mtd_nor_device *dev = (struct fal_mtd_nor_device*) device;

    uint64_t phy_offset;
    uint32_t phy_length;

    /* change the block device's logic address to physical address */
    phy_offset = offset;
    phy_length = length;

    ret = dev->fal_dev->ops.read(phy_offset, data, phy_length);

    if (ret != (int)phy_length)
    {
        ret = 0;
    }
    else
    {
        ret = length;
    }

    return ret;
}

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_ssize_t mtd_nor_dev_write(struct rt_mtd_nor_device* device, rt_off_t offset, const rt_uint8_t* data, rt_size_t length)
#else
static rt_size_t mtd_nor_dev_write(struct rt_mtd_nor_device* device, rt_off_t offset, const rt_uint8_t* data, rt_uint32_t length)
#endif
{
    int ret = 0;
    struct fal_mtd_nor_device *dev = (struct fal_mtd_nor_device*) device;

    uint64_t phy_offset;
    uint32_t phy_length;

    /* change the block device's logic address to physical address */
    phy_offset = offset;
    phy_length = length;

    ret = dev->fal_dev->ops.write(phy_offset, data, phy_length);

    if (ret != (int) phy_length)
    {
        ret = 0;
    }
    else
    {
        ret = length;
    }

    return ret;
}

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_err_t mtd_nor_dev_erase(struct rt_mtd_nor_device* device, rt_off_t offset, rt_size_t length)
#else
static rt_err_t mtd_nor_dev_erase(struct rt_mtd_nor_device* device, rt_off_t offset, rt_uint32_t length)
#endif
{
    int ret = 0;
    struct fal_mtd_nor_device *dev = (struct fal_mtd_nor_device*) device;

    uint64_t phy_offset;
    uint32_t phy_length;

    /* change the block device's logic address to physical address */
    phy_offset = offset;
    phy_length = length;

    ret = dev->fal_dev->ops.erase(phy_offset, phy_length);

    if ((rt_uint32_t)ret != phy_length || ret < 0)
    {
        return -RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }
}

static const struct rt_mtd_nor_driver_ops _ops =
{
    RT_NULL,
    mtd_nor_dev_read,
    mtd_nor_dev_write,
    mtd_nor_dev_erase,
};

struct rt_device *fal_dev_mtd_nor_device_create(struct fal_flash64_dev *fal_dev)
{
    fal_dev->ops.init();

    struct fal_mtd_nor_device *mtd_nor_dev;
    mtd_nor_dev = (struct fal_mtd_nor_device*) rt_malloc(sizeof(struct fal_mtd_nor_device));
    if (mtd_nor_dev)
    {
        mtd_nor_dev->fal_dev = fal_dev;

        mtd_nor_dev->parent.block_start = 0;
        //nor模式最大支持到4GB
        if (fal_dev->len > 0xffffffff)
        {
            mtd_nor_dev->parent.block_end = 0xffffffff/fal_dev->blk_size;
            fal_dev->len = 0xffffffff;
        }
        else
        {
            mtd_nor_dev->parent.block_end = fal_dev->len/fal_dev->blk_size;
        }
        mtd_nor_dev->parent.block_size = fal_dev->blk_size;

        /* set ops */
        mtd_nor_dev->parent.ops = &_ops;

        LOG_I("The FAL MTD NOR device (%s) created successfully %d MB [ %d block ]"
                , fal_dev->name
                , mtd_nor_dev->parent.block_end / 1024 * mtd_nor_dev->parent.block_size / 1024
                , mtd_nor_dev->parent.block_size);
        rt_mtd_nor_register_device(fal_dev->name, &mtd_nor_dev->parent);
    }
    else
    {
        LOG_E("Error: no memory for create FAL MTD NOR device");
    }

    return RT_DEVICE(&mtd_nor_dev->parent);
}

struct fal_blk_device
{
    struct rt_device                parent;
    struct rt_device_blk_geometry   geometry;
    struct fal_flash64_dev          *fal_dev;
};

#if RTTHREAD_VERSION >= 30000
static rt_err_t blk_dev_control(rt_device_t dev, int cmd, void *args)
#else
static rt_err_t blk_dev_control(rt_device_t dev, rt_uint8_t cmd, void *args)
#endif
{
    struct fal_blk_device *fal_dev = (struct fal_blk_device*) dev;

    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct rt_device_blk_geometry *geometry;

        geometry = (struct rt_device_blk_geometry *) args;
        if (geometry == RT_NULL)
        {
            return -RT_ERROR;
        }

        rt_memcpy(geometry, &fal_dev->geometry, sizeof(struct rt_device_blk_geometry));
        LOG_D("geometry '%s': len %u MB, block %u"
                    , fal_dev->fal_dev->name
                    , (uint32_t)(geometry->sector_count / 1024 * geometry->block_size / 1024)
                    , geometry->block_size);
    }
    else if (cmd == RT_DEVICE_CTRL_BLK_ERASE)
    {
        uint64_t *addrs = (uint64_t *) args, start_addr = addrs[0], end_addr = addrs[1], phy_start_addr;
        uint32_t phy_size;

        if (addrs == RT_NULL || start_addr > end_addr)
        {
            return -RT_ERROR;
        }

        if (end_addr == start_addr)
        {
            end_addr++;
        }

        phy_start_addr = start_addr * fal_dev->fal_dev->blk_size;
        phy_size = (end_addr - start_addr) * fal_dev->fal_dev->blk_size;

        if (fal_dev->fal_dev->ops.erase(phy_start_addr, phy_size) < 0)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_ssize_t blk_dev_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
#else
static rt_size_t blk_dev_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
#endif
{
    int ret = 0;
    struct fal_blk_device *fal_dev = (struct fal_blk_device*) dev;

    uint64_t phy_pos;
    uint32_t phy_size;

    /* change the block device's logic address to physical address */
    phy_pos = pos * fal_dev->fal_dev->blk_size;
    phy_size = size * fal_dev->fal_dev->blk_size;

    ret = fal_dev->fal_dev->ops.read(phy_pos, buffer, phy_size);

    if (ret != (int)(phy_size))
    {
        ret = 0;
    }
    else
    {
        ret = size;
    }

    return ret;
}

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_ssize_t blk_dev_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
#else
static rt_size_t blk_dev_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
#endif
{
    int ret = 0;
    struct fal_blk_device *fal_dev = (struct fal_blk_device*) dev;

    uint64_t phy_pos;
    uint32_t phy_size;

    /* change the block device's logic address to physical address */
    phy_pos = pos * fal_dev->fal_dev->blk_size;
    phy_size = size * fal_dev->fal_dev->blk_size;

    ret = fal_dev->fal_dev->ops.erase(phy_pos, phy_size);

    if (ret == (int) phy_size)
    {
        ret = fal_dev->fal_dev->ops.write(phy_pos, buffer, phy_size);
    }

    if (ret != (int) phy_size)
    {
        ret = 0;
    }
    else
    {
        ret = size;
    }

    return ret;
}

struct rt_device *fal_dev_blk_device_create(struct fal_flash64_dev *fal_dev)
{
    fal_dev->ops.init();

    struct fal_blk_device *blk_dev;
    blk_dev = (struct fal_blk_device*) rt_malloc(sizeof(struct fal_blk_device));
    if (blk_dev)
    {
        blk_dev->fal_dev = fal_dev;
        blk_dev->geometry.bytes_per_sector = fal_dev->blk_size;
        blk_dev->geometry.block_size = fal_dev->blk_size;
        blk_dev->geometry.sector_count = fal_dev->len/fal_dev->blk_size;

        /* register device */
        blk_dev->parent.type = RT_Device_Class_Block;

#ifdef RT_USING_DEVICE_OPS
        blk_dev->parent.ops  = &blk_dev_ops;
#else
        blk_dev->parent.init = NULL;
        blk_dev->parent.open = NULL;
        blk_dev->parent.close = NULL;
        blk_dev->parent.read = blk_dev_read;
        blk_dev->parent.write = blk_dev_write;
        blk_dev->parent.control = blk_dev_control;
#endif

        /* no private */
        blk_dev->parent.user_data = RT_NULL;

        LOG_I("The FAL block device (%s) created successfully", fal_dev->name);
        rt_device_register(RT_DEVICE(blk_dev), fal_dev->name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
    }
    else
    {
        LOG_E("Error: no memory for create FAL block device");
    }

    return RT_DEVICE(blk_dev);
}

/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-16     lvhan       the first version
 */
#include "board.h"
#include "fal_cfg.h"

#ifdef BSP_USING_USBH_CHERRY
#include <unistd.h>
#include <dfs_fs.h>
#include "usbh_hub.h"
#include "usbh_msc.h"

#define DBG_TAG "drv.cherry"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define MAX_PARTITION_COUNT 4

struct udisk_part
{
    struct usbh_msc *msc_class;
    struct dfs_partition part;
    struct rt_device dev;
};
static struct udisk_part udisk_part[MAX_PARTITION_COUNT] = { 0 };

static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t msc_buffer[CONFIG_USBDEV_MSC_BLOCK_SIZE];

#ifdef HAL_HCD_MODULE_ENABLED
static HCD_HandleTypeDef hhcd_USB_OTG_HS;

void usb_hc_low_level_init(void)
{
#if defined (STM32F405xx) || defined (STM32F415xx) || defined (STM32F407xx) || defined (STM32F417xx) || \
    defined (STM32F427xx) || defined (STM32F437xx) || defined (STM32F429xx) || defined (STM32F439xx) || \
    defined (STM32F401xC) || defined (STM32F401xE) || defined (STM32F410Tx) || defined (STM32F410Cx) || \
    defined (STM32F410Rx) || defined (STM32F411xE) || defined (STM32F446xx) || defined (STM32F469xx) || \
    defined (STM32F479xx) || defined (STM32F412Cx) || defined (STM32F412Rx) || defined (STM32F412Vx) || \
    defined (STM32F412Zx) || defined (STM32F413xx) || defined (STM32F423xx)

    hhcd_USB_OTG_HS.Instance = USB_OTG_HS;
    hhcd_USB_OTG_HS.Init.Host_channels = 12;
    hhcd_USB_OTG_HS.Init.speed = HCD_SPEED_FULL;
    hhcd_USB_OTG_HS.Init.dma_enable = ENABLE;
    hhcd_USB_OTG_HS.Init.phy_itface = USB_OTG_EMBEDDED_PHY;
    hhcd_USB_OTG_HS.Init.Sof_enable = DISABLE;
    hhcd_USB_OTG_HS.Init.low_power_enable = DISABLE;
    hhcd_USB_OTG_HS.Init.vbus_sensing_enable = DISABLE;
    hhcd_USB_OTG_HS.Init.use_external_vbus = DISABLE;
    if (HAL_HCD_Init(&hhcd_USB_OTG_HS) != HAL_OK)
    {
        Error_Handler();
    }
#endif

#if defined (STM32H743xx) || defined (STM32H753xx)  || defined (STM32H750xx) || defined (STM32H742xx)  || \
    defined (STM32H745xx) || defined (STM32H745xG)  || defined (STM32H755xx) || defined (STM32H747xx)  || defined (STM32H747xG) || defined (STM32H757xx)  || \
    defined (STM32H7A3xx) || defined (STM32H7A3xxQ) || defined (STM32H7B3xx) || defined (STM32H7B3xxQ) || defined (STM32H7B0xx) || defined (STM32H7B0xxQ) || \
    defined (STM32H735xx) || defined (STM32H733xx)  || defined (STM32H730xx) || defined (STM32H730xxQ) || defined (STM32H725xx) || defined (STM32H723xx)

    hhcd_USB_OTG_HS.Instance = USB_OTG_HS;
    hhcd_USB_OTG_HS.Init.Host_channels = 16;
    hhcd_USB_OTG_HS.Init.speed = HCD_SPEED_FULL;
    hhcd_USB_OTG_HS.Init.dma_enable = ENABLE;
    hhcd_USB_OTG_HS.Init.phy_itface = USB_OTG_EMBEDDED_PHY;
    hhcd_USB_OTG_HS.Init.Sof_enable = DISABLE;
    hhcd_USB_OTG_HS.Init.low_power_enable = DISABLE;
    hhcd_USB_OTG_HS.Init.use_external_vbus = DISABLE;
    if (HAL_HCD_Init(&hhcd_USB_OTG_HS) != HAL_OK)
    {
        Error_Handler();
    }
#endif
}
#endif

#ifdef HAL_PCD_MODULE_ENABLED
static PCD_HandleTypeDef hpcd_USB_OTG_HS;

void usb_dc_low_level_init(void)
{
    hpcd_USB_OTG_HS.Instance = USB_OTG_HS;
    hpcd_USB_OTG_HS.Init.dev_endpoints = 6;
    hpcd_USB_OTG_HS.Init.speed = PCD_SPEED_FULL;
    hpcd_USB_OTG_HS.Init.dma_enable = DISABLE;
    hpcd_USB_OTG_HS.Init.phy_itface = USB_OTG_EMBEDDED_PHY;
    hpcd_USB_OTG_HS.Init.Sof_enable = DISABLE;
    hpcd_USB_OTG_HS.Init.low_power_enable = DISABLE;
    hpcd_USB_OTG_HS.Init.lpm_enable = DISABLE;
    hpcd_USB_OTG_HS.Init.vbus_sensing_enable = DISABLE;
    hpcd_USB_OTG_HS.Init.use_dedicated_ep1 = DISABLE;
    hpcd_USB_OTG_HS.Init.use_external_vbus = DISABLE;
    if (HAL_PCD_Init(&hpcd_USB_OTG_HS) != HAL_OK)
    {
        Error_Handler();
    }
}
#endif

static int rt_cherryusb_init(void)
{
    usbh_initialize();

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_ENV_EXPORT(rt_cherryusb_init);

static rt_size_t blk_dev_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    int ret;
    RT_ASSERT(dev != RT_NULL);

    struct udisk_part *part = dev->user_data;

    if (pos + size > part->part.offset + part->part.size)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(pos + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
        LOG_W("read size %d! block is %d", size, pos);
        return 0;
    }
    LOG_D("read (0x%p) %d", pos, size);

#ifdef SOC_SERIES_STM32H7
    rt_size_t nblk = size;
    while (nblk > 0)
    {
        ret = usbh_msc_scsi_read10(part->msc_class, pos, msc_buffer, 1);
        if (ret != 0)
        {
            LOG_E("ReadBlocks Error (0x%p) %d %d/%d", pos, ret, nblk, size);
            return -RT_EINVAL;
        }
        rt_memcpy(buffer, msc_buffer, CONFIG_USBDEV_MSC_BLOCK_SIZE);

        nblk--;
        pos++;
        buffer += CONFIG_USBDEV_MSC_BLOCK_SIZE;
    }
#else
    ret = usbh_msc_scsi_read10(part->msc_class, pos, buffer, size);
    if (ret != 0)
    {
        LOG_E("ReadBlocks Error (0x%p) %d %d", pos, ret, size);
        return -RT_EINVAL;
    }
#endif

    return size;
}

static rt_size_t blk_dev_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    int ret;
    RT_ASSERT(dev != RT_NULL);

    struct udisk_part *part = dev->user_data;

    if (pos + size > part->part.offset + part->part.size)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void*)(pos + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
        LOG_W("write size %d! block is %d", size, pos);
        return 0;
    }
    LOG_D("write (0x%p) %d", pos, size);

#ifdef SOC_SERIES_STM32H7
    rt_size_t nblk = size;
    while (nblk > 0)
    {
        rt_memcpy(msc_buffer, buffer, CONFIG_USBDEV_MSC_BLOCK_SIZE);
        ret = usbh_msc_scsi_write10(part->msc_class, pos, msc_buffer, 1);
        if (ret != 0)
        {
            LOG_E("WriteBlocks Error (0x%p) %d %d/%d", pos, ret, nblk, size);
            return -RT_EINVAL;
        }

        nblk--;
        pos++;
        buffer += CONFIG_USBDEV_MSC_BLOCK_SIZE;
    }
#else
    ret = usbh_msc_scsi_write10(part->msc_class, pos, buffer, size);
    if (ret != 0)
    {
        LOG_E("WriteBlocks Error (0x%p) %d %d", pos, ret, size);
        return -RT_EINVAL;
    }
#endif

    return size;
}

static rt_err_t blk_dev_control(rt_device_t dev, int cmd, void *args)
{
    RT_ASSERT(dev != RT_NULL);

    struct udisk_part *part = dev->user_data;

    LOG_D("blk_dev_control 0x%x", cmd);
    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct rt_device_blk_geometry *geometry;

        geometry = (struct rt_device_blk_geometry *) args;
        if (geometry == RT_NULL)
        {
            return -RT_ERROR;
        }

        geometry->block_size = part->msc_class->blocksize;
        geometry->bytes_per_sector = part->msc_class->blocksize;
        geometry->sector_count = part->part.size;
        LOG_I("geometry 0x%x 0x%x : len %d MB, block %d", part->msc_class->intf, part->msc_class->sdchar,
                geometry->sector_count / 1024 * geometry->block_size / 1024, geometry->block_size);
    }

    return RT_EOK;
}

void usbh_msc_run(struct usbh_msc *msc_class)
{
    int i = 0;
    rt_err_t ret;
    char dname[8];
    char sname[8];
    RT_ASSERT(msc_class != RT_NULL);

    /* get the partition table */
    ret = usbh_msc_scsi_read10(msc_class, 0, msc_buffer, 1);
    if (ret != RT_EOK)
    {
        LOG_E("read parition table error");
        return;
    }

#ifdef BSP_USING_ROOTFS
    /* Imitating Linux's proc */
    int sn_len = 64;
    if (msc_class->hport->iSerialNumber != 0U)
    {
        /* User callback for Serial number string */
        LOG_I("Serial Number : %s", (char * )(void * )msc_class->hport->iSerialNumber);
    }
    else
    {
        LOG_I("Serial Number : N/A");
        sn_len = 0;
    }
    mkdir("/proc/udisk", 0);
    int fd = open("/proc/udisk/SerialNumber", O_WRONLY | O_CREAT | O_TRUNC, 0);
    write(fd, msc_class->hport->iSerialNumber, sn_len);
    close(fd);
#endif

    char blk_dir[32] = {0};
    for (i = 0; i < MAX_PARTITION_COUNT; i++)
    {
        struct udisk_part *data = &udisk_part[i];
        rt_memset(data, 0, sizeof(struct udisk_part));
        /* get the first partition */
        ret = dfs_filesystem_get_partition(&data->part, msc_buffer, i);
        if (ret == RT_EOK)
        {
            data->msc_class = msc_class;
            rt_snprintf(dname, 6, "ud0p%d", i);
            rt_snprintf(sname, 8, "sem_ud%d", i);
            rt_sprintf(blk_dir, "/mnt/%s/%s", BLK_USBH_UDISK, dname);
            data->part.lock = rt_sem_create(sname, 1, RT_IPC_FLAG_FIFO);

            /* register sdcard device */
            data->dev.type = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
            data->dev.ops = &udisk_device_ops;
#else
            data->dev.read = blk_dev_read;
            data->dev.write = blk_dev_write;
            data->dev.control = blk_dev_control;
#endif
            data->dev.user_data = (void *) data;

            if (rt_device_register(&data->dev, dname, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE) == RT_EOK)
            {
                LOG_I("The block device (%s) register successfully", dname);
            }
            else
            {
                LOG_E("The block device (%s) register failed!", dname);
            }

            mkdir(blk_dir, 0);
            if (dfs_mount(dname, blk_dir, "elm", 0, 0) == 0)
            {
                LOG_I("Udisk part (%s) mount %s successfully", dname, blk_dir);
            }
            else
            {
                LOG_E("Udisk part (%s) mount %s failed", dname, blk_dir);
            }
        }
        else
        {
            if (i == 0)
            {
                /* there is no partition table */
                data->part.offset = 0;
                data->part.size = msc_class->blocknum;
                data->msc_class = msc_class;
                data->part.lock = rt_sem_create("sem_ud", 1, RT_IPC_FLAG_FIFO);

                rt_snprintf(dname, 7, "ud0p0");
                rt_sprintf(blk_dir, "/mnt/%s/%s", BLK_USBH_UDISK, dname);

                /* register sdcard device */
                data->dev.type = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
                stor->dev[i].ops = &udisk_device_ops;
#else
                data->dev.read = blk_dev_read;
                data->dev.write = blk_dev_write;
                data->dev.control = blk_dev_control;
#endif
                data->dev.user_data = (void *) data;

                if (rt_device_register(&data->dev, dname, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE) == RT_EOK)
                {
                    LOG_I("The block device (%s) register successfully", dname);
                }
                else
                {
                    LOG_E("The block device (%s) register failed!", dname);
                }

                mkdir(blk_dir, 0);
                if (dfs_mount(dname, blk_dir, "elm", 0, 0) == 0)
                {
                    LOG_I("Udisk dev (%s) mount %s successful.", dname, blk_dir);
                }
                else
                {
                    LOG_E("Udisk dev (%s) mount %s failed.", dname, blk_dir);
                }
            }
            break;
        }
    }
}

void usbh_msc_stop(struct usbh_msc *msc_class)
{
    int i;
    /* check parameter */
    RT_ASSERT(msc_class != RT_NULL);
#ifdef BSP_USING_ROOTFS
    rm("/proc/udisk");
#endif

    for (i = 0; i < MAX_PARTITION_COUNT; i++)
    {
        struct udisk_part *data = &udisk_part[i];
        if (data->msc_class == NULL)
            break;

        /* unmount file system */
        const char *blk_dir = dfs_filesystem_get_mounted_path(&(data->dev));
        if (blk_dir)
        {
            if (dfs_unmount(blk_dir) == 0)
            {
                LOG_I("dev %s unmount succeed!", data->dev.parent.name);
            }
            else
            {
                LOG_E("dev %s unmount failed!", data->dev.parent.name);
            }
            rmdir(blk_dir);
        }

        /* delete semaphore */
        rt_sem_delete(data->part.lock);

        /* unregister device */
        if (rt_device_unregister(&data->dev) == RT_EOK)
        {
            LOG_I("The block device (%s) unregister successfully", data->dev.parent.name);
        }
        else
        {
            LOG_E("The block device (%s) unregister failed!", data->dev.parent.name);
        }

        rt_memset(data, 0, sizeof(struct udisk_part));
    }
}

#endif

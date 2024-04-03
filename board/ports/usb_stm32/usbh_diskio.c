/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usbh_diskio.c (based on usbh_diskio_dma_template.c v2.0.2)
  * @brief   USB Host Disk I/O driver
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbh_diskio.h"

#ifdef BSP_USING_USBH_STM32
#include <unistd.h>
#include <dfs_file.h>
#include <dfs_fs.h>

#define DBG_TAG "udisk"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#if defined(RT_USING_FAL)
#include "fal.h"
#include "fal_cfg.h"

/* Private define ------------------------------------------------------------*/
#define USB_DEFAULT_BLOCK_SIZE 512
#define _MIN_SS    512  /* 512, 1024, 2048 or 4096 */
#define _MAX_SS    512  /* 512, 1024, 2048 or 4096 */

/* Private variables ---------------------------------------------------------*/
extern USBH_HandleTypeDef hUsbHost;
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint32_t scratch[_MAX_SS / 4];

/* Private functions ---------------------------------------------------------*/
#define _USE_WRITE 1 /* 1: Enable disk_write function */
#define _USE_IOCTL 1 /* 1: Enable disk_ioctl function */

#define MAX_PARTITION_COUNT 4

struct usbh_diskio
{
    MSC_HandleTypeDef *handle;
    struct dfs_partition part;
    struct rt_device dev;
    rt_mutex_t ready;
};

static struct usbh_diskio udisk_part[MAX_PARTITION_COUNT] = { 0 };

/* Private function prototypes -----------------------------------------------*/
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_ssize_t USBH_read(rt_device_t dev, rt_off_t sector, void* buff, rt_size_t count);
#else
static rt_size_t USBH_read(rt_device_t dev, rt_off_t sector, void* buff, rt_size_t count);
#endif

#if _USE_WRITE == 1
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_ssize_t USBH_write(rt_device_t dev, rt_off_t sector, const void* buff, rt_size_t count);
#else
static rt_size_t USBH_write(rt_device_t dev, rt_off_t sector, const void* buff, rt_size_t count);
#endif
#endif /* _USE_WRITE == 1 */

#if _USE_IOCTL == 1
static rt_err_t USBH_ioctl(rt_device_t dev, int cmd, void *buff);
#endif /* _USE_IOCTL == 1 */
/**
 * @brief  Initializes a Drive
 * @param  lun : lun id
 * @retval DSTATUS: Operation status
 */
rt_err_t USBH_diskio_initialize(MSC_HandleTypeDef *handle)
{
    int i = 0;
    rt_err_t ret;
    char dname[8];
    char mname[8];
    MSC_LUNTypeDef info;
    USBH_HandleTypeDef *phost = &hUsbHost;

    /* get lun info */
    if (USBH_MSC_GetLUNInfo(phost, handle->current_lun, &info) != USBH_OK)
    {
        LOG_E("get lun info failed");
        return -RT_ERROR;
    }

    /* get the partition table */
    ret = USBH_MSC_Read(phost, handle->current_lun, 0, (uint8_t *)scratch, 1);
    if (ret != USBH_OK)
    {
        LOG_E("read parition table error");
        return -RT_ERROR;
    }

#ifdef BSP_USING_ROOTFS
    /* Imitating Linux's proc */
    int sn_len = 64;
    if (phost->device.DevDesc.iSerialNumber != 0U)
    {
        /* User callback for Serial number string */
        LOG_D("Serial Number : %s", (char * )(void * )phost->device.Data);
    }
    else
    {
        LOG_D("Serial Number : N/A");
        sn_len = 0;
    }
    mkdir("/proc/udisk", 0);
    int fd = open("/proc/udisk/SerialNumber", O_WRONLY | O_CREAT | O_TRUNC, 0);
    write(fd, phost->device.Data, sn_len);
    close(fd);
#endif

    char blk_dir[32] = {0};
    for (i = 0; i < MAX_PARTITION_COUNT; i++)
    {
        struct usbh_diskio *data = &udisk_part[i];

        rt_memset(data, 0, sizeof(struct usbh_diskio));
        /* get the first partition */
        ret = dfs_filesystem_get_partition(&data->part, (uint8_t *)scratch, i);
        if (ret == RT_EOK)
        {
            data->handle = handle;
            rt_snprintf(dname, 6, "ud0p%d", i);
            rt_snprintf(mname, 8, "mut_ud%d", i);
            rt_sprintf(blk_dir, "/mnt/%s/%s", BLK_USBH_UDISK, dname);
            data->ready = rt_mutex_create(mname, RT_IPC_FLAG_PRIO);

            /* register udisk device */
            data->dev.type = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
            data->dev.ops = &udisk_device_ops;
#else
            data->dev.read = USBH_read;
            data->dev.write = USBH_write;
            data->dev.control = USBH_ioctl;
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
            if (dfs_mount(data->dev.parent.name, blk_dir, "elm", 0, 0) == 0)
            {
                LOG_I("Udisk part %d mount '%s' successfully", i, blk_dir);
            }
            else
            {
                LOG_E("Udisk part %d mount '%s' failed", i, blk_dir);
            }
        }
        else
        {
            if (i == 0)
            {
                /* there is no partition table */
                data->part.offset = 0;
                data->part.size = info.capacity.block_nbr;
                data->handle = handle;
                data->ready = rt_mutex_create("mut_ud", RT_IPC_FLAG_PRIO);

                rt_snprintf(dname, 7, "ud0p0");
                rt_sprintf(blk_dir, "/mnt/%s/%s", BLK_USBH_UDISK, dname);

                /* register sdcard device */
                data->dev.type = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
                stor->dev[i].ops = &udisk_device_ops;
#else
                data->dev.read = USBH_read;
                data->dev.write = USBH_write;
                data->dev.control = USBH_ioctl;
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
                if (dfs_mount(data->dev.parent.name, blk_dir, "elm", 0, 0) == 0)
                {
                    LOG_I("Udisk mount '%s' successfully", blk_dir);
                }
                else
                {
                    LOG_E("Udisk mount '%s' failed", blk_dir);
                }
            }
            break;
        }
    }

    return RT_EOK;
}

rt_err_t USBH_diskio_uninitialize()
{
    int i;
    rt_err_t ret = RT_EOK;
#ifdef BSP_USING_ROOTFS
    dfs_file_unlink("/proc/udisk");
#endif

    for (i = 0; i < MAX_PARTITION_COUNT; i++)
    {
        struct usbh_diskio *data = &udisk_part[i];

        if (data->handle == NULL)
            break;

        /* unmount file system */
        const char *blk_dir = dfs_filesystem_get_mounted_path(&(data->dev));
        if (blk_dir)
        {
            rt_mutex_take(data->ready, RT_WAITING_FOREVER);
            if (dfs_unmount(blk_dir) == 0)
            {
                LOG_I("Udisk unmount '%s' successfully", blk_dir);
            }
            else
            {
                LOG_E("Udisk unmount '%s' failed", blk_dir);
                ret = -RT_ERROR;
            }
            rmdir(blk_dir);
        }

        if (rt_device_find(data->dev.parent.name) != NULL)
        {
            /* unregister device */
            if (rt_device_unregister(&data->dev) == RT_EOK)
            {
                LOG_I("The block device (%s) unregister successfully", data->dev.parent.name);
            }
            else
            {
                LOG_E("The block device (%s) unregister failed!", data->dev.parent.name);
                ret = -RT_ERROR;
            }

            /* clean */
            rt_mutex_delete(data->ready);
        }
    }

    return ret;
}

#ifdef USBH_ENABLE_STATUS
/**
 * @brief  Gets Disk Status
 * @param  lun : lun id
 * @retval DSTATUS: Operation status
 */
static rt_err_t USBH_status(rt_device_t dev)
{
    struct rt_device *part = (struct rt_device *)dev;
    struct usbh_diskio *msc_class = part->user_data;

    RT_ASSERT(part != RT_NULL);

    rt_err_t res = -RT_ERROR;

    if (USBH_MSC_UnitIsReady(&hUsbHost, msc_class->handle->current_lun))
    {
        res = RT_EOK;
    }

    return res;
}
#endif

/**
 * @brief  Reads Sector(s)
 * @param  lun : lun id
 * @param  *buff: Data buffer to store read data
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to read (1..128)
 * @retval DRESULT: Operation result
 */
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_ssize_t USBH_read(rt_device_t dev, rt_off_t sector, void* buff, rt_size_t count)
#else
static rt_size_t USBH_read(rt_device_t dev, rt_off_t sector, void* buff, rt_size_t count)
#endif
{
    struct rt_device *part = (struct rt_device *)dev;
    struct usbh_diskio *msc_class = part->user_data;

    RT_ASSERT(part != RT_NULL);
    rt_mutex_take(msc_class->ready, RT_WAITING_FOREVER);

    rt_size_t res = 0;
    MSC_LUNTypeDef info;
    USBH_StatusTypeDef status = USBH_OK;

    if (!((uint32_t)buff & 3) && (((HCD_HandleTypeDef *)hUsbHost.pData)->Init.dma_enable))
    {
        USBH_DbgLog("read dma : %d %d", sector, count);
        while (res < count)
        {
            status = USBH_MSC_Read(&hUsbHost, msc_class->handle->current_lun, sector + res, (uint8_t *)scratch, 1);

            if (status == USBH_OK)
            {
                rt_memcpy(&((uint8_t *)buff)[res * _MAX_SS], scratch, _MAX_SS);
            }
            else
            {
                break;
            }
            res ++;
        }
    }
    else
    {
        USBH_DbgLog("read : %d %d", sector, count);
        status = USBH_MSC_Read(&hUsbHost, msc_class->handle->current_lun, sector, buff, count);
    }

    if (status == USBH_OK)
    {
        res = count;
    }
    else
    {
        USBH_MSC_GetLUNInfo(&hUsbHost, msc_class->handle->current_lun, &info);

        switch (info.sense.asc)
        {
        case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
        case SCSI_ASC_MEDIUM_NOT_PRESENT:
        case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
            USBH_ErrLog("USB Disk is not ready!");
            res = 0;
            break;

        default:
            USBH_ErrLog("USB Disk read error %d/%d %d!", res, count, status);
            res = 0;
            break;
        }
    }

    rt_mutex_release(msc_class->ready);
    return res;
}

/**
 * @brief  Writes Sector(s)
 * @param  lun : lun id
 * @param  *buff: Data to be written
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to write (1..128)
 * @retval DRESULT: Operation result
 */
#if _USE_WRITE == 1
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_ssize_t USBH_write(rt_device_t dev, rt_off_t sector, const void* buff, rt_size_t count)
#else
static rt_size_t USBH_write(rt_device_t dev, rt_off_t sector, const void* buff, rt_size_t count)
#endif
{
    struct rt_device *part = (struct rt_device *)dev;
    struct usbh_diskio *msc_class = part->user_data;

    RT_ASSERT(part != RT_NULL);
    rt_mutex_take(msc_class->ready, RT_WAITING_FOREVER);

    rt_size_t res = 0;
    MSC_LUNTypeDef info;
    USBH_StatusTypeDef status = USBH_OK;

    if (!((uint32_t)buff & 3) && (((HCD_HandleTypeDef *)hUsbHost.pData)->Init.dma_enable))
    {
        USBH_DbgLog("write dma : %d %d", sector, count);
        while (res < count)
        {
            rt_memcpy(scratch, &((uint8_t *)buff)[res * _MAX_SS], _MAX_SS);

            status = USBH_MSC_Write(&hUsbHost, msc_class->handle->current_lun, sector + res, (uint8_t *)scratch, 1);
            if (status != USBH_OK)
            {
                break;
            }
            res ++;
        }
    }
    else
    {
        USBH_DbgLog("write : %d %d", sector, count);
        status = USBH_MSC_Write(&hUsbHost, msc_class->handle->current_lun, sector, (uint8_t *)buff, count);
    }

    if (status == USBH_OK)
    {
        res = count;
    }
    else
    {
        USBH_MSC_GetLUNInfo(&hUsbHost, msc_class->handle->current_lun, &info);

        switch (info.sense.asc)
        {
        case SCSI_ASC_WRITE_PROTECTED:
            USBH_ErrLog("USB Disk is Write protected!");
            res = 0;
            break;

        case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
        case SCSI_ASC_MEDIUM_NOT_PRESENT:
        case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
            USBH_ErrLog("USB Disk is not ready!");
            res = 0;
            break;

        default:
            USBH_ErrLog("USB Disk write error %d/%d %d!", res, count, status);
            res = 0;
            break;
        }
    }

    rt_mutex_release(msc_class->ready);
    return res;
}
#endif /* _USE_WRITE == 1 */

/**
 * @brief  I/O control operation
 * @param  lun : lun id
 * @param  cmd: Control code
 * @param  *buff: Buffer to send/receive control data
 * @retval DRESULT: Operation result
 */
#if _USE_IOCTL == 1
static rt_err_t USBH_ioctl(rt_device_t dev, int cmd, void *buff)
{
    struct rt_device *part = (struct rt_device *)dev;
    struct usbh_diskio *msc_class = part->user_data;
    USBH_HandleTypeDef *phost = &hUsbHost;

    RT_ASSERT(part != RT_NULL);
    rt_mutex_take(msc_class->ready, RT_WAITING_FOREVER);

    rt_err_t res = RT_EOK;
    MSC_LUNTypeDef info;

    switch (cmd)
    {
    /* Make sure that no pending write process */
    case RT_DEVICE_CTRL_BLK_SYNC:
        res = RT_EOK;
        break;

    case RT_DEVICE_CTRL_BLK_GETGEOME:
        if (USBH_MSC_GetLUNInfo(phost, msc_class->handle->current_lun, &info) == USBH_OK)
        {
            struct rt_device_blk_geometry *geometry;

            geometry = (struct rt_device_blk_geometry *) buff;
            if (geometry == RT_NULL)
            {
                rt_mutex_release(msc_class->ready);
                return -RT_ERROR;
            }

            geometry->block_size = info.capacity.block_size;
            geometry->bytes_per_sector = info.capacity.block_size;
            geometry->sector_count = msc_class->part.size;
            USBH_UsrLog("geometry : len %d MB, block %d"
                    , geometry->sector_count / 1024 * geometry->block_size / 1024
                    , geometry->block_size);
            res = RT_EOK;
        }
        else
        {
            res = -RT_ERROR;
        }
        break;
    }

    rt_mutex_release(msc_class->ready);
    return res;
}
#endif /* _USE_IOCTL == 1 */

#endif
#endif

/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-12     lvhan       the first version
 */
#include "board.h"

#include <rtthread.h>
#include <fal.h>
#include <dfs_file.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#include "crc32.h"
#include "ota_from_file.h"

#define DBG_TAG "ota.file"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define OTA_ALGO_CRYPT_NONE         0
#define OTA_ALGO_CRYPT_XOR          1
#define OTA_ALGO_CRYPT_AES          2
#define OTA_ALGO_CRYPT_MASK         0x0F

#define OTA_ALGO_CMPRS_NONE         (0 << 8)
#define OTA_ALGO_CMPRS_GZIP         (1 << 8)
#define OTA_ALGO_CMPRS_QUICKLZ      (2 << 8)
#define OTA_ALGO_CMPRS_FASTLZ       (3 << 8)
#define OTA_ALGO_CMPRS_MASK         (0x0F << 8)

#define OTA_ALGO2_VERIFY_NONE       0
#define OTA_ALGO2_VERIFY_CRC        1
#define OTA_ALGO2_VERIFY_MASK       0x0F

#define OTA_CMPRS_BUF_SIZE          4096
#define OTA_FILE_BUF_LEN            1024

enum
{
    ReadyToUpdate,
    FileNotExist,
    FileNotCheck,
    FileIsSame,
    PartNotExist,
};

enum
{
    MODE_DOWNLOAD = 0,
    MODE_FACTORY,
    MODE_TFTP,
};

struct ota_from_file_ops
{
    int fd;
    uint8_t devsta;//存储设备连接状态
    rt_device_t dev;//存储设备
    char* devname;

    const char* part_name[2];
    const struct fal_partition * part[2];
    const char *path[3];

    uint8_t updatesta;//升级状态
    uint8_t* cmprs_buf;
    uint8_t* file_buf;
    size_t update_file_total_size;
    size_t update_file_cur_size;
};

static struct ota_from_file_ops ota_from_file = {
    .devname = OTA_FROM_FILE_DEVNAME,
    .part_name = {OTA_FROM_FILE_DOWNLOAD_PART_NAME, OTA_FROM_FILE_FACTORY_PART_NAME},
    .path = {OTA_FROM_FILE_DOWNLOAD_PATH, OTA_FROM_FILE_FACTORY_PATH, OTA_FROM_FILE_TFTP_PATH},
};

typedef struct {
    uint8_t  type[4];
    uint16_t algo;
    uint16_t algo2;
    uint32_t time_stamp;
    uint8_t  part_name[16];
    uint8_t  fw_ver[24];
    uint8_t  prod_code[24];
    uint32_t pkg_crc;
    uint32_t raw_crc;
    uint32_t raw_size;
    uint32_t pkg_size;
    uint32_t hdr_crc;
}fw_info_t;

static int ota_fd_fw_info_read(int fd, fw_info_t *fw_info)
{
    lseek(fd, 0, SEEK_SET);
    if (read(fd, fw_info, sizeof(fw_info_t)) != sizeof(fw_info_t))
    {
        return(0);
    }
    return(1);
}

static int ota_fd_fw_info_check(fw_info_t *fw_info)
{
    if (rt_strcmp((const char *)(fw_info->type), "RBL") != 0)
    {
        return(0);
    }

    return (crc32_cal((uint8_t *)fw_info, (sizeof(fw_info_t) - sizeof(uint32_t))) == fw_info->hdr_crc);
}

static int ota_fw_info_show(fw_info_t *fw_info)
{
    char str[20];

    rt_memset(str, 0x0, sizeof(str));
    switch(fw_info->algo & OTA_ALGO_CRYPT_MASK)
    {
    case OTA_ALGO_CRYPT_NONE:
        rt_strcpy(str, "NONE");
        break;
    case OTA_ALGO_CRYPT_XOR:
        rt_strcpy(str, "XOR");
        break;
    case OTA_ALGO_CRYPT_AES:
        rt_strcpy(str, "AES");
        break;
    default:
        rt_strcpy(str, "UNKNOW");
        break;
    }
    switch(fw_info->algo & OTA_ALGO_CMPRS_MASK)
    {
    case OTA_ALGO_CMPRS_NONE:
        rt_strcpy(str + rt_strlen(str), " && NONE");
        break;
    case OTA_ALGO_CMPRS_GZIP:
        rt_strcpy(str + rt_strlen(str), " && GZIP");
        break;
    case OTA_ALGO_CMPRS_QUICKLZ:
        rt_strcpy(str + rt_strlen(str), " && QUCKLZ");
        break;
    case OTA_ALGO_CMPRS_FASTLZ:
        rt_strcpy(str + rt_strlen(str), " && FASTLZ");
        break;
    default:
        rt_strcpy(str + rt_strlen(str), " && UNKNOW");
        break;
    }

    rt_kprintf("| Product code          | %*.s |\n", 20, fw_info->prod_code);
    rt_kprintf("| Algorithm mode        | %*.s |\n", 20, str);
    rt_kprintf("| Destition partition   | %*.s |\n", 20, fw_info->part_name);
    rt_kprintf("| Version               | %*.s |\n", 20, fw_info->fw_ver);
    rt_kprintf("| Package size          | %20d |\n", fw_info->pkg_size);
    rt_kprintf("| Raw code size         | %20d |\n", fw_info->raw_size);
    rt_kprintf("| Package crc           | %20X |\n", fw_info->pkg_crc);
    rt_kprintf("| Raw code verify       | %20X |\n", fw_info->raw_crc);
    rt_kprintf("| Header crc            | %20X |\n", fw_info->hdr_crc);
    rt_kprintf("| Build timestamp       | %20d |\n", fw_info->time_stamp);

    return (1);
}

static int ota_fd_fw_crc_check(int fd, uint32_t addr, uint32_t size, uint32_t crc)
{
    uint32_t pos = 0;
    uint32_t crc32 = 0xFFFFFFFF;

    lseek(fd, addr, SEEK_SET);
    while (pos < size)
    {
        int read_len = OTA_FILE_BUF_LEN;
        int gzip_remain_len = size - pos;
        if (read_len > gzip_remain_len)
        {
            read_len = gzip_remain_len;
        }
        if (read(fd, ota_from_file.cmprs_buf, read_len) != read_len)
        {
            LOG_E("read firmware datas fail. addr = %08X, length = %d", pos, read_len);
            return(0);
        }
        crc32 = crc32_cyc_cal(crc32, ota_from_file.cmprs_buf, read_len);
        pos += read_len;
    }
    crc32 ^= 0xFFFFFFFF;

    if (crc32 != crc)
    {
        LOG_E("verify CRC32 error, cal.crc: %08X != body.crc: %08X", crc32, crc);
        return(0);
    }

    return(1);
}

static int ota_fd_fw_check(int fd, fw_info_t *fw_info)
{
    if ( ! ota_fd_fw_info_read(fd, fw_info))
    {
        LOG_E("firmware check fail. read firmware infomation fail.");
        return(0);
    }

    if ( ! ota_fd_fw_info_check(fw_info))
    {
        LOG_E("firmware check fail. firmware infomation check fail.");
        return(0);
    }

    if ( ! ota_fd_fw_crc_check(fd, sizeof(fw_info_t), fw_info->pkg_size, fw_info->pkg_crc))
    {
        LOG_E("firmware check fail. firmware body check fail.");
        return(0);
    }

    LOG_D("firmware check success.");

    return(1);
}

static int ota_fw_info_read(const struct fal_partition *part, fw_info_t *fw_info)
{
    if (fal_partition_read(part, 0, (uint8_t *)fw_info, sizeof(fw_info_t)) < 0)
    {
        return(0);
    }
    return(1);
}

static int ota_fw_sign_check(const struct fal_partition *part, fw_info_t *fw_info)
{
    uint32_t release_sign = 0;
    uint32_t pos = (((sizeof(fw_info_t) + fw_info->pkg_size) + 0x1F) & ~0x1F);

    if (fal_partition_read(part, pos, (uint8_t *)&release_sign, sizeof(uint32_t)) < 0)
    {
        LOG_E("read release sign fail from %s partition.", part->name);
        return(0);
    }

    return(release_sign == 0);
}

static int ota_fw_info_cmp(fw_info_t *fw_info, fw_info_t *part_fw_info)
{
    if (fw_info->hdr_crc == part_fw_info->hdr_crc
            && fw_info->pkg_crc == part_fw_info->pkg_crc
            && fw_info->raw_crc == part_fw_info->raw_crc
            && fw_info->time_stamp == part_fw_info->time_stamp)
    {
        return(1);
    }
    return(0);
}

static int ota_on_begin(struct ota_from_file_ops *ops, uint8_t mode)
{
    struct stat buf;
    fstat(ops->fd, &buf);

    /* calculate and store file size */
    ops->update_file_total_size = buf.st_size;
    LOG_D("firmware file size: %d KB", ops->update_file_total_size/1024);
    ops->update_file_cur_size = 0;

    /* check firmware */
    fw_info_t fw_info;
    if ( ! ota_fd_fw_check(ops->fd, &fw_info))
    {
        ops->updatesta = FileNotCheck;
        return -RT_ERROR;
    }

    /* Get partition information and erase partition data */
    if ((ops->part[mode] = fal_partition_find(ops->part_name[mode])) == RT_NULL)
    {
        LOG_E("Partition (%s) find error!", ops->part_name[mode]);
        return -RT_ERROR;
    }
    if (ops->update_file_total_size > ops->part[mode]->len)
    {
        LOG_E("Firmware is too large! File size (%d), '%s' partition size (%d)", ops->update_file_total_size, ops->part_name[mode], ops->part[mode]->len);
        return -RT_ERROR;
    }

    fw_info_t part_fw_info;
    ota_fw_info_read(ops->part[mode], &part_fw_info);
    if (ota_fw_sign_check(ops->part[mode], &part_fw_info))
    {
        ota_from_file_handle(OTA_HANDLE_LOADED);
        LOG_D("Last firmware has been loaded.");
    }
    else
    {
        ota_from_file_handle(OTA_HANDLE_FAILED);
        LOG_W("Last firmware may fail to load.");
    }

    if ( ! ota_fw_info_cmp(&fw_info, &part_fw_info))
    {
        ota_fw_info_show(&fw_info);
        LOG_I("Start erase. Size (%d)", ops->update_file_total_size);

        /* erase section */
        if (fal_partition_erase(ops->part[mode], 0, ops->update_file_total_size) < 0)
        {
            LOG_E("Partition (%s) erase failed!", ops->part[mode]->name);
            return -RT_ERROR;
        }

        return RT_EOK;
    }
    else
    {
        LOG_D("upgrade firmware same as last.");
        ops->updatesta = FileIsSame;
    }

    return -RT_ERROR;
}

static int ota_on_data(struct ota_from_file_ops *ops, uint32_t len, uint8_t mode)
{
    /* write data of application to partition  */
    if (fal_partition_write(ops->part[mode], ops->update_file_cur_size, ops->file_buf, len) < 0)
    {
        rt_kprintf("\n");
        LOG_E("Firmware program failed! Partition (%s) write data 0x%x error!", ops->part[mode]->name, ops->update_file_cur_size);
        return -RT_ERROR;
    }
    if (fal_partition_read(ops->part[mode], ops->update_file_cur_size, ops->cmprs_buf, len) < 0)
    {
        rt_kprintf("\n");
        LOG_E("Firmware program failed! Partition (%s) read data 0x%x error!", ops->part[mode]->name, ops->update_file_cur_size);
        return -RT_ERROR;
    }
    if (rt_memcmp(ops->file_buf, ops->cmprs_buf, len) != 0)
    {
        rt_kprintf("\n");
        LOG_HEX("write", 16, ops->file_buf, len);
        LOG_HEX("read ", 16, ops->cmprs_buf, len);
        LOG_E("Firmware program failed! Partition (%s) check data 0x%x %d error!", ops->part[mode]->name, ops->update_file_cur_size, len);
        return -RT_ERROR;
    }

    ops->update_file_cur_size += len;

    return RT_EOK;
}

static void ota_thread_entry(void* parameter)
{
    struct ota_from_file_ops *ops = (struct ota_from_file_ops *)parameter;

    while(1)
    {
        rt_thread_delay(1000);

        ops->dev = rt_device_find(ops->devname);
        if (ops->devsta == 0 && ops->dev != NULL)
        {
            LOG_D("dev in.");
            ops->devsta = 1;
            ops->updatesta = ReadyToUpdate;
            rt_thread_delay(500);//确保文件系统挂载
        }
        if (ops->devsta == 1 && ops->dev == NULL)
        {
            LOG_D("dev out.");
            ops->devsta = 0;
        }

        if (ops->devsta == 1 && ops->updatesta == ReadyToUpdate)
        {
            uint8_t is_reboot = 0;
            for(uint8_t mode = MODE_DOWNLOAD; mode <= MODE_FACTORY; mode ++)
            {
                if (access(ops->path[mode], 0) == 0)
                {
                    LOG_D("firmware '%s' is exisit", ops->path[mode]);
                    ops->fd = open(ops->path[mode], O_RDONLY);
                    if (ota_on_begin(ops, mode) == RT_EOK)
                    {
                        ota_from_file_handle(OTA_HANDLE_START);
                        LOG_I("Start programming ...");
                        rt_size_t length = 1;
                        lseek(ops->fd, 0, SEEK_SET);
                        while (length > 0)
                        {
                            rt_memset(ops->file_buf, 0, OTA_FILE_BUF_LEN);
                            length = read(ops->fd, ops->file_buf, OTA_FILE_BUF_LEN);
                            if (length < 1)
                            {
                                break;
                            }
                            if (ota_on_data(ops, length, mode) != RT_EOK)
                            {
                                break;
                            }
                            rt_kprintf(".");
                        }
                        if (length < 1)
                            rt_kprintf("\n");

                        if (ops->update_file_total_size == ops->update_file_cur_size)
                        {
                            ota_from_file_handle(OTA_HANDLE_FINISH);
                            LOG_D("Download firmware to flash success.");

                            is_reboot ++;
                        }
                        else
                        {
                            /* wait some time for terminal response finish */
                            rt_thread_mdelay(1000);
                            LOG_E("Update firmware fail %d/%d.", ops->update_file_cur_size, ops->update_file_total_size);
                        }
                    }
                    else
                    {
                        ops->updatesta = PartNotExist;
                    }
                    close(ops->fd);
                }
                else
                {
                    LOG_D("firmware '%s' not exisit", ops->path[mode]);
                    ops->updatesta = FileNotExist;
                }
            }
            if (is_reboot > 0)
            {
                LOG_I("System now will restart...");

                /* wait some time for terminal response finish */
                rt_thread_mdelay(200);

                /* Reset the device, Start new firmware */
                rt_hw_cpu_reset();
                /* wait some time for terminal response finish */
                rt_thread_mdelay(200);
            }
        }

        if (access(ops->path[MODE_TFTP], 0) == 0)
        {
            LOG_D("firmware '%s' is exisit", ops->path[MODE_TFTP]);
            rt_thread_mdelay(5000);
            ops->fd = open(ops->path[MODE_TFTP], O_RDONLY);
            if (ota_on_begin(ops, MODE_DOWNLOAD) == RT_EOK)
            {
                ota_from_file_handle(OTA_HANDLE_START);
                LOG_I("Start programming ...");
                rt_size_t length = 1;
                lseek(ops->fd, 0, SEEK_SET);
                while (length > 0)
                {
                    rt_memset(ops->file_buf, 0, OTA_FILE_BUF_LEN);
                    length = read(ops->fd, ops->file_buf, OTA_FILE_BUF_LEN);
                    if (length < 1)
                    {
                        break;
                    }
                    if (ota_on_data(ops, length, MODE_DOWNLOAD) != RT_EOK)
                    {
                        break;
                    }
                    rt_kprintf(".");
                }
                if (length < 1)
                    rt_kprintf("\n");

                if (ops->update_file_total_size == ops->update_file_cur_size)
                {
                    ota_from_file_handle(OTA_HANDLE_FINISH);
                    LOG_D("Download firmware to flash success.");
                    close(ops->fd);
                    dfs_file_unlink(ops->path[MODE_TFTP]);

                    LOG_I("System now will restart...");

                    /* wait some time for terminal response finish */
                    rt_thread_mdelay(200);

                    /* Reset the device, Start new firmware */
                    rt_hw_cpu_reset();
                    /* wait some time for terminal response finish */
                    rt_thread_mdelay(200);
                }
                else
                {
                    /* wait some time for terminal response finish */
                    rt_thread_mdelay(1000);
                    LOG_E("Update firmware fail %d/%d.", ops->update_file_cur_size, ops->update_file_total_size);
                }
            }
            close(ops->fd);
            dfs_file_unlink(ops->path[MODE_TFTP]);
        }
    }
}

static int ota_from_file_init(void)
{
    ota_from_file.file_buf = rt_malloc(OTA_FILE_BUF_LEN);
    if (ota_from_file.file_buf == NULL)
    {
        LOG_E("file_buf malloc error!");
    }
    ota_from_file.cmprs_buf = rt_malloc(OTA_CMPRS_BUF_SIZE);
    if (ota_from_file.cmprs_buf == NULL)
    {
        LOG_E("cmprs_buf malloc error!");
    }
    /* 创建线程 */
    rt_thread_t thread = rt_thread_create("ota_file", ota_thread_entry, &ota_from_file, 3072, 28, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }

    return RT_EOK;
}
INIT_SERVICE_EXPORT(ota_from_file_init);

RT_WEAK void ota_from_file_handle(OtaHandleTypeDef type)
{
    switch(type)
    {
    case OTA_HANDLE_START:
        break;
    case OTA_HANDLE_FINISH:
        break;
    case OTA_HANDLE_LOADED:
        break;
    case OTA_HANDLE_FAILED:
        break;
    default:
        break;
    }
}

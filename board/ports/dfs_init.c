/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-09     lvhan       the first version
 */
#include "board.h"

#include <dfs_fs.h>

#ifdef RT_USING_DFS_ROMFS
#include <dfs_romfs.h>
#endif

#include "fal_cfg.h"

#define DBG_TAG "dfs"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static const struct romfs_dirent _romfs_root_mnt[] =
{
#ifdef RT_USING_DFS_NFS
    {ROMFS_DIRENT_DIR, "nfs"        , RT_NULL, 0},      //nfs
#endif

#ifdef BSP_USING_FM25xx
    {ROMFS_DIRENT_DIR, BLK_FRAM     , RT_NULL, 0},      //fram
#endif

#ifdef BSP_USING_SPI_FLASH
    {ROMFS_DIRENT_DIR, BLK_SPI_FLASH, RT_NULL, 0},      //spiflash
#endif

#ifdef BSP_USING_S25FL512
    {ROMFS_DIRENT_DIR, BLK_S25FL512, RT_NULL, 0},       //S25FL512
#endif

#ifdef BSP_USING_AT45DB321E
    {ROMFS_DIRENT_DIR, BLK_AT45DB321E, RT_NULL, 0},       //AT45DB321E
#endif

#ifdef BSP_FMCSRAM_ENABLE_FS
    {ROMFS_DIRENT_DIR, BLK_FMCSRAM  , RT_NULL, 0},      //sram
#endif

#ifdef BSP_SDRAM_ENABLE_FS
    {ROMFS_DIRENT_DIR, BLK_SDRAM  , RT_NULL, 0},        //sdram
#endif

#ifdef NORFLASH_ENABLE_FS
    {ROMFS_DIRENT_DIR, BLK_NOR      , RT_NULL, 0},      //nor
#endif

#ifdef BSP_USING_EMMC
    {ROMFS_DIRENT_DIR, BLK_EMMC     , RT_NULL, 0},      //emmc
#endif

#ifdef BSP_USING_SDCARD
    {ROMFS_DIRENT_DIR, BLK_SDCARD   , RT_NULL, 0},      //sd卡
#endif

#ifdef BSP_USING_USB
    {ROMFS_DIRENT_DIR,BLK_USBH_UDISK, RT_NULL, 0},      //u盘
#endif
};

static const struct romfs_dirent _romfs_root[] =
{
#ifdef BSP_USING_ROOTFS
    {ROMFS_DIRENT_DIR, "bin"        , RT_NULL, 0},      //二进制可执行命令
#endif
#ifdef RT_USING_DFS_DEVFS
    {ROMFS_DIRENT_DIR, "dev"        , RT_NULL, 0},      //设备驱动程序
#endif
#ifdef BSP_USING_ROOTFS
    {ROMFS_DIRENT_DIR, "etc"        , RT_NULL, 0},      //系统管理和配置文件
    {ROMFS_DIRENT_DIR, "lib"        , RT_NULL, 0},      //标准程序设计库
#endif
    //文件系统挂载
    {ROMFS_DIRENT_DIR, "mnt"        , (rt_uint8_t *)_romfs_root_mnt, sizeof(_romfs_root_mnt)/sizeof(struct romfs_dirent)},
#ifdef BSP_USING_ROOTFS
    {ROMFS_DIRENT_DIR, "proc"       , RT_NULL, 0},      //系统内存映射
    {ROMFS_DIRENT_DIR, "usr"        , RT_NULL, 0},      //应用目录
#endif
};

//根文件系统
const struct romfs_dirent romfs_root =
{
    ROMFS_DIRENT_DIR , "/"          , (rt_uint8_t *)_romfs_root    , sizeof(_romfs_root) / sizeof(struct romfs_dirent)
};

struct mount_fs
{
    char *dev_name;
    char *blk_name;
    char *fs;
};

static const struct mount_fs _mount_fs[] =
{
#ifdef BSP_USING_SDCARD
    {RT_NULL            , BLK_SDCARD        , "tmp"             },
#endif

#ifdef BSP_USING_USB
    {RT_NULL            , BLK_USBH_UDISK    , "tmp"             },
#endif

#ifdef BSP_USING_FM25xx
    {BLK_FRAM           , BLK_FRAM          , FM25xx_FS         },
#endif

#ifdef BSP_USING_SPI_FLASH
    {BLK_SPI_FLASH      , BLK_SPI_FLASH     , SPI_FLASH_FS      },
#endif

#ifdef BSP_USING_S25FL512
    {BLK_S25FL512       , BLK_S25FL512      , S25FL512_FS       },
#endif

#ifdef BSP_USING_AT45DB321E
    {BLK_AT45DB321E     , BLK_AT45DB321E    , AT45DB321E_FS     },
#endif

#ifdef BSP_FMCSRAM_ENABLE_FS
    {BLK_FMCSRAM        , BLK_FMCSRAM       , BSP_FMCSRAM_FS    },
#endif

#ifdef BSP_SDRAM_ENABLE_FS
    {BLK_SDRAM          , BLK_SDRAM         , BSP_SDRAM_FS      },
#endif

#ifdef NORFLASH_ENABLE_FS
    {BLK_NOR            , BLK_NOR           , NORFLASH_FS       },
#endif

#ifdef BSP_USING_EMMC
    {EMMC_DEV_NAME      , BLK_EMMC          , EMMC_FS           },
#endif
};

#ifdef BSP_USING_ROOTFS
static const struct mount_fs _mount_rootfs[] =
{
    {"bin"           , "bin"          , "lfs"         },
    {"etc"           , "etc"          , "lfs"         },
    {"lib"           , "lib"          , "lfs"         },
    {"usr"           , "usr"          , "lfs"         },
};
#endif

static int rt_dfs_init(void)
{
    int ret = 0;
    struct mount_fs node;
    char blk_dir[32] = {0};

#ifdef RT_USING_DFS_ROMFS
    /* 挂载 romfs */
    ret = dfs_mount(RT_NULL, "/", "rom", 0, &(romfs_root));
    if (ret != 0)
    {
        LOG_E("rom mount to '/' failed!");
        return -RT_ERROR;
    }
#endif

#ifdef BSP_USING_ROOTFS
    /* 挂载 tmpfs */
    ret = dfs_mount(RT_NULL, "/proc", "tmp", 0, NULL);
    if (ret != 0)
    {
        LOG_E("tmp mount to '/proc' failed!");
        return -RT_ERROR;
    }

    for (int i = 0; i < (sizeof(_mount_rootfs)/sizeof(struct mount_fs)); i++)
    {
        node = _mount_rootfs[i];

        /* 挂载文件系统 */
        rt_sprintf(blk_dir, "/%s", node.blk_name);
        if (dfs_mount(node.dev_name, blk_dir, node.fs, 0, 0) == 0)
        {
            LOG_I("dev %s mount to %s succeed!", node.dev_name, blk_dir);
        }
        else
        {
            /* 格式化文件系统 */
            ret = dfs_mkfs(node.fs, node.dev_name);
            if (ret != 0)
            {
                LOG_E("dev %s mkfs %s failed %d!", node.dev_name, node.fs, ret);
                ret = -RT_ERROR;
            }
            else
            {
                /* 挂载文件系统 */
                ret = dfs_mount(node.dev_name, blk_dir, node.fs, 0, 0);
                if (ret == 0)
                {
                    LOG_W("dev %s mkfs %s mount to %s succeed!", node.dev_name, node.fs, blk_dir);
                }
                else
                {
                    LOG_E("dev %s mkfs %s mount to %s failed %d!", node.dev_name, node.fs, blk_dir, ret);
                    ret = -RT_ERROR;
                }
            }
        }
    }
#endif

    for (int i = 0; i < (sizeof(_mount_fs)/sizeof(struct mount_fs)); i++)
    {
        node = _mount_fs[i];

        /* 挂载文件系统 */
        rt_sprintf(blk_dir, "/mnt/%s", node.blk_name);
        if (dfs_mount(node.dev_name, blk_dir, node.fs, 0, 0) == 0)
        {
            LOG_I("dev %s mount to %s succeed!", node.dev_name, blk_dir);
        }
        else
        {
            /* 格式化文件系统 */
            ret = dfs_mkfs(node.fs, node.dev_name);
            if (ret != 0)
            {
                LOG_E("dev %s mkfs %s failed %d!", node.dev_name, node.fs, ret);
                ret = -RT_ERROR;
            }
            else
            {
                /* 挂载文件系统 */
                ret = dfs_mount(node.dev_name, blk_dir, node.fs, 0, 0);
                if (ret == 0)
                {
                    LOG_W("dev %s mkfs %s mount to %s succeed!", node.dev_name, node.fs, blk_dir);
                }
                else
                {
                    LOG_E("dev %s mkfs %s mount to %s failed %d!", node.dev_name, node.fs, blk_dir, ret);
                    ret = -RT_ERROR;
                }
            }
        }
    }

    return ret;
}
/* 导出到自动初始化 */
INIT_COMPONENT_EXPORT(rt_dfs_init);

/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-16     lvhan       the first version
 */
#include "board.h"

#if defined(RT_USING_FAL)
#include "fal.h"
#include "fal_cfg.h"

#define DBG_TAG "fal"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

struct fal_dev_init
{
    char *dev_name;
    char *fs;
    void *p;
};

#ifdef BSP_USING_EMMC
    extern struct fal_flash_dev emmc_flash;
#endif

static const struct fal_dev_init _fal_dev[] =
{
#ifdef BSP_USING_FM25xx
    {BLK_FRAM, FM25xx_FS, NULL},
#endif

#ifdef BSP_USING_SPI_FLASH
    {BLK_SPI_FLASH, SPI_FLASH_FS, NULL},
#endif

#ifdef BSP_USING_S25FL512
    {BLK_S25FL512, S25FL512_FS, NULL},
#endif

#ifdef BSP_USING_AT45DB321E
    {BLK_AT45DB321E, AT45DB321E_FS, NULL},
#endif

#ifdef BSP_FMCSRAM_ENABLE_BLK
    {BLK_FMCSRAM, BSP_FMCSRAM_FS, NULL},
#endif

#ifdef BSP_SDRAM_ENABLE_BLK
    {BLK_SDRAM, BSP_SDRAM_FS, NULL},
#endif

#ifdef BSP_USING_NORFLASH
    {BLK_NOR, NORFLASH_FS, NULL},
#endif

#ifdef BSP_USING_EEPROM_24Cxx
    {BLK_EEPROM, "", NULL},
#endif

#ifdef BSP_USING_MAX31826
    {BLK_MAX31826, "", NULL},
#endif

#ifdef BSP_USING_ROOTFS
    {"bin", "lfs", NULL},
    {"etc", "lfs", NULL},
    {"lib", "lfs", NULL},
    {"usr", "lfs", NULL},
#endif

#ifdef BSP_USING_EMMC
    {EMMC_DEV_NAME, EMMC_FS, &emmc_flash},
#endif
};

static int rt_fal_init(void)
{
    fal_init();

    struct fal_dev_init node;
    for (int i = 0; i < (sizeof(_fal_dev)/sizeof(struct fal_dev_init)); i++)
    {
        node = _fal_dev[i];

        if (rt_strcmp(node.fs, "lfs") == 0)
        {
            if (node.p != NULL)
            {
                if (fal_dev_mtd_nor_device_create(node.p) == NULL)
                {
                    LOG_E("Failed to creat nor %s!", node.dev_name);
                }
            }
            else
            {
                if (fal_mtd_nor_device_create(node.dev_name) == NULL)
                {
                    LOG_E("Failed to creat nor %s!", node.dev_name);
                }
            }
        }
        else if (rt_strcmp(node.fs, "elm") == 0 || rt_strcmp(node.fs, "ext") == 0)
        {
            if (node.p != NULL)
            {
                if (fal_dev_blk_device_create(node.p) == NULL)
                {
                    LOG_E("Failed to creat blk %s!", node.dev_name);
                }
            }
            else
            {
                if (fal_blk_device_create(node.dev_name) == NULL)
                {
                    LOG_E("Failed to creat blk %s!", node.dev_name);
                }
            }
        }
        else
        {
            if (node.p != NULL)
            {
                if (fal_dev_blk_device_create(node.p) == NULL)
                {
                    LOG_E("Failed to creat blk %s!", node.dev_name);
                }
            }
            else
            {
                if (fal_blk_device_create(node.dev_name) == NULL)
                {
                    LOG_E("Failed to creat blk %s!", node.dev_name);
                }
            }
        }
    }

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_fal_init);

#endif //defined(RT_USING_FAL)

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

#ifdef BSP_USING_EMMC
    extern struct fal_flash_dev emmc_flash;
#endif

#ifdef BSP_USING_ROOTFS
static const char *fal_rootfs[] = {
    "bin", "etc", "lib", "usr",
};
#endif

static int rt_fal_init(void)
{
    fal_init();

#ifdef BSP_USING_FM25xx
    if (fal_mtd_nor_device_create(BLK_FRAM) == NULL)
//    if (fal_blk_device_create(BLK_FRAM) == NULL)
    {
        LOG_E("Failed to creat nor %s!", BLK_FRAM);
//        LOG_E("Failed to creat blk %s!", BLK_FRAM);
    }
#endif

#ifdef BSP_USING_SPI_FLASH
    if (fal_mtd_nor_device_create(BLK_SPI_FLASH) == NULL)
    {
        LOG_E("Failed to creat nor %s!", BLK_SPI_FLASH);
    }
#endif

#ifdef BSP_USING_S25FL512
    if (fal_mtd_nor_device_create(BLK_S25FL512) == NULL)
    {
        LOG_E("Failed to creat nor %s!", BLK_S25FL512);
    }
#endif

#ifdef BSP_FMCSRAM_ENABLE_BLK
    if (fal_blk_device_create(BLK_FMCSRAM) == NULL)
    {
        LOG_E("Failed to creat blk %s!", BLK_FMCSRAM);
    }
#endif

#ifdef BSP_SDRAM_ENABLE_BLK
    if (fal_blk_device_create(BLK_SDRAM) == NULL)
    {
        LOG_E("Failed to creat blk %s!", BLK_SDRAM);
    }
#endif

#ifdef NORFLASH_ENABLE_FS
    if (fal_mtd_nor_device_create(BLK_NOR) == NULL)
//    if (fal_blk_device_create(BLK_NOR) == NULL)
    {
        LOG_E("Failed to creat nor %s!", BLK_NOR);
//        LOG_E("Failed to creat blk %s!", BLK_NOR);
    }
#endif

#ifdef BSP_USING_EEPROM_24Cxx
    if (fal_blk_device_create(BLK_EEPROM) == NULL)
    {
        LOG_E("Failed to creat blk %s!", BLK_EEPROM);
    }
#endif

#ifdef BSP_USING_ROOTFS
    for (int i = 0; i < (sizeof(fal_rootfs)/sizeof(char *)); i++)
    {
        if (fal_mtd_nor_device_create(fal_rootfs[i]) == NULL)
        {
            LOG_E("Failed to creat nor %s!", fal_rootfs[i]);
        }
    }
#endif

#ifdef BSP_USING_EMMC
    if (rt_strcmp(EMMC_FS, "lfs") == 0)
    {
        if (fal_dev_mtd_nor_device_create(&emmc_flash) == NULL)
        {
            LOG_E("Failed to creat nor %s!", EMMC_DEV_NAME);
        }
    }
    else if (rt_strcmp(EMMC_FS, "elm") == 0 || rt_strcmp(EMMC_FS, "ext") == 0)
    {
        if (fal_dev_blk_device_create(&emmc_flash) == NULL)
        {
            LOG_E("Failed to creat blk %s!", EMMC_DEV_NAME);
        }
    }
#endif

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_fal_init);

#endif //defined(RT_USING_FAL)

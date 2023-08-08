/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-15     lvhan       the first version
 */
#ifndef APPLICATIONS_FAL_CFG_H_
#define APPLICATIONS_FAL_CFG_H_

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BSP_USING_ON_CHIP_FLASH
//内部flash定义,无需修改
#define FLASH_SIZE_TOTAL                    (STM32_FLASH_SIZE)
#define FLASH_OFFSET_BOOTLOADER             (0)
#define FLASH_OFFSET_APP                    (FLASH_OFFSET_BOOTLOADER + FLASH_SIZE_BOOTLOADER)
#define FLASH_OFFSET_DOWNLOAD               (FLASH_OFFSET_APP + FLASH_SIZE_APP)
#define FLASH_OFFSET_FACTORY                (FLASH_OFFSET_DOWNLOAD + FLASH_SIZE_DOWNLOAD)
#define FLASH_START_ADRESS                  (STM32_FLASH_START_ADRESS)
#define FLASH_BLK_SIZE                      (128 * 1024)
#define FLASH_DEV_NAME                      "onchip128"

#define RT_APP_PART_ADDR                    (FLASH_START_ADRESS + FLASH_OFFSET_APP)

#ifdef FLASH_USING_BLK256
#define FLASH_BLK256_START_ADRESS           (STM32_FLASH_START_ADRESS + FLASH_SIZE_TOTAL)
#define FLASH_BLK256_SIZE                   (256 * 1024)
#define FLASH_BLK256_NAME                   "onchip256"
#endif
#endif

#ifdef BSP_USING_FM25xx
//fm25xx定义,无需修改
#define FM25xx_START_ADRESS            (0)
#define FM25xx_DEV_NAME                "fm25xx"
#define BLK_FRAM                       "fram"
#endif

#ifdef BSP_USING_SPI_FLASH
//spiflash定义,无需修改
#define SPI_FLASH_START_ADRESS          (0)
#define SPI_FLASH_DEV_NAME              "sfud"
#define BLK_SPI_FLASH                   "spiflash"
#endif

#ifdef BSP_FMCSRAM_ENABLE_BLK
//sram定义,无需修改
#define FMCSRAM_DEV_NAME                "fmc_sram"
#define BLK_FMCSRAM                     "sram"
#define FMCSRAM_BLK_SIZE                (512)
#endif

#ifdef BSP_SDRAM_ENABLE_BLK
//sdram定义,无需修改
#define SDRAM_DEV_NAME                  "sdram"
#define BLK_SDRAM                       "sdram"
#define SDRAM_BLK_SIZE                  (512)
#endif
#ifndef BSP_SDRAM_BLK_SIZE
#define BSP_SDRAM_BLK_SIZE              (0)
#endif

#ifdef BSP_USING_NORFLASH
//norflash定义,无需修改
#define NORFLASH_DEV_NAME               "fmc_nor"
#define BLK_NOR                         "nor"
#define NORFLASH_OFFSET_APP             (0)
#define NORFLASH_OFFSET_DOWNLOAD        (NORFLASH_OFFSET_APP + NORFLASH_SIZE_APP)
#define NORFLASH_OFFSET_FACTORY         (NORFLASH_OFFSET_DOWNLOAD + NORFLASH_SIZE_DOWNLOAD)
#define NORFLASH_OFFSET_BIN             (NORFLASH_OFFSET_FACTORY + NORFLASH_SIZE_FACTORY)
#define NORFLASH_OFFSET_ETC             (NORFLASH_OFFSET_BIN + NORFLASH_SIZE_BIN)
#define NORFLASH_OFFSET_LIB             (NORFLASH_OFFSET_ETC + NORFLASH_SIZE_ETC)
#define NORFLASH_OFFSET_USR             (NORFLASH_OFFSET_LIB + NORFLASH_SIZE_LIB)
#define NORFLASH_OFFSET_KVDB            (NORFLASH_OFFSET_USR + NORFLASH_SIZE_USR)
#define NORFLASH_OFFSET_TSDB            (NORFLASH_OFFSET_KVDB + NORFLASH_SIZE_KVDB)
#define NORFLASH_OFFSET_FS              (NORFLASH_OFFSET_TSDB + NORFLASH_SIZE_TSDB)
#define NORFLASH_SIZE_FS                (NORFLASH_SIZE_GRANULARITY_TOTAL - NORFLASH_OFFSET_FS)
#endif

#ifdef BSP_USING_EMMC
//emmc定义,无需修改
#define EMMC_START_ADRESS               (0)
#define EMMC_DEV_NAME                   "sdmmc"
#define BLK_EMMC                        "emmc"
#define EMMC_SIZE_GRANULARITY_TOTAL     (2048)
#define EMMC_BLK_SIZE                   (512)
#endif

#ifdef BSP_USING_EEPROM_24Cxx
//24cxx定义,无需修改
#define EEPROM_24Cxx_START_ADRESS      (0)
#define EEPROM_24Cxx_DEV_NAME          "24cxx"
#define BLK_EEPROM                     "eeprom"
#endif

#ifdef BSP_USING_SDCARD
//sdcard定义,无需修改
#define BLK_SDCARD                      "sdcard"
#define BLK_SDCARD0                     "sd0"
#define BLK_SDCARDp0                    "sd0p0"
#define BLK_SDCARDp1                    "sd0p1"
#define BLK_SDCARDp2                    "sd0p2"
#define BLK_SDCARDp3                    "sd0p3"
#endif

#ifdef BSP_USING_USB
//msc定义,无需修改
#define UDISK_BLK_SIZE                  (512)
#define BLK_USBH_UDISK                  "udisk"
#define BLK_USBH_UDISK0                 "ud0"
#define BLK_USBH_UDISK0p0               "ud0p0"
#define BLK_USBH_UDISK0p1               "ud0p1"
#define BLK_USBH_UDISK0p2               "ud0p2"
#define BLK_USBH_UDISK0p3               "ud0p3"
#endif

#ifdef RT_USING_FAL
#include "fal_def.h"

/* flash device table */
#define FAL_FLASH_DEV_TABLE
extern const struct fal_flash_dev *const device_table[];
extern const size_t device_table_len;

/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE
extern const struct fal_partition partition_table_def[];
extern const size_t partition_table_def_len;

#define PART_FLASH_MAX 32
#endif /* FAL_PART_HAS_TABLE_CFG */
#endif

struct rt_device *fal_dev_mtd_nor_device_create(struct fal_flash_dev *fal_dev);
struct rt_device *fal_dev_blk_device_create(struct fal_flash_dev *fal_dev);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATIONS_FAL_CFG_H_ */

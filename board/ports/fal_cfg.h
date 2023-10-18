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
#define FLASH_OFFSET_KVDB                   (FLASH_OFFSET_FACTORY + FLASH_SIZE_FACTORY)
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

#ifdef BSP_USING_FM25xx
//fm25xx定义,无需修改
#define FM25xx_START_ADRESS            (0)
#define FM25xx_DEV_NAME                "fm25xx"
#define BLK_FRAM                       FM25xx_PARTNAME
#ifndef FM25xx_ENABLE_FS
#define FM25xx_FS                      ""
#endif
#endif

#ifdef BSP_USING_SPI_FLASH
//spiflash定义,无需修改
#define SPI_FLASH_START_ADRESS          (0)
#define SPI_FLASH_DEV_NAME              "sfud"
#define BLK_SPI_FLASH                   "spiflash"
#define SPI_FLASH_OFFSET_DOWNLOAD        (0)
#define SPI_FLASH_OFFSET_FACTORY         (SPI_FLASH_OFFSET_DOWNLOAD + SPI_FLASH_SIZE_DOWNLOAD)
#define SPI_FLASH_OFFSET_BIN             (SPI_FLASH_OFFSET_FACTORY + SPI_FLASH_SIZE_FACTORY)
#define SPI_FLASH_OFFSET_ETC             (SPI_FLASH_OFFSET_BIN + SPI_FLASH_SIZE_BIN)
#define SPI_FLASH_OFFSET_LIB             (SPI_FLASH_OFFSET_ETC + SPI_FLASH_SIZE_ETC)
#define SPI_FLASH_OFFSET_USR             (SPI_FLASH_OFFSET_LIB + SPI_FLASH_SIZE_LIB)
#define SPI_FLASH_OFFSET_KVDB            (SPI_FLASH_OFFSET_USR + SPI_FLASH_SIZE_USR)
#define SPI_FLASH_OFFSET_TSDB            (SPI_FLASH_OFFSET_KVDB + SPI_FLASH_SIZE_KVDB)
#define SPI_FLASH_OFFSET_FS              (SPI_FLASH_OFFSET_TSDB + SPI_FLASH_SIZE_TSDB)
#define SPI_FLASH_SIZE_FS                (SPI_FLASH_SIZE_GRANULARITY_TOTAL - SPI_FLASH_OFFSET_FS)
#ifndef SPI_FLASH_ENABLE_FS
#define SPI_FLASH_FS                     ""
#endif
#endif

#ifdef BSP_USING_S25FL512
//s25fl512定义,无需修改
#define S25FL512_START_ADRESS           (0)
#define S25FL512_DEV_NAME               "s25fl512"
#define BLK_S25FL512                    "spinor64"
#define S25FL512_OFFSET_DOWNLOAD        (0)
#define S25FL512_OFFSET_FACTORY         (S25FL512_OFFSET_DOWNLOAD + S25FL512_SIZE_DOWNLOAD)
#define S25FL512_OFFSET_BIN             (S25FL512_OFFSET_FACTORY + S25FL512_SIZE_FACTORY)
#define S25FL512_OFFSET_ETC             (S25FL512_OFFSET_BIN + S25FL512_SIZE_BIN)
#define S25FL512_OFFSET_LIB             (S25FL512_OFFSET_ETC + S25FL512_SIZE_ETC)
#define S25FL512_OFFSET_USR             (S25FL512_OFFSET_LIB + S25FL512_SIZE_LIB)
#define S25FL512_OFFSET_KVDB            (S25FL512_OFFSET_USR + S25FL512_SIZE_USR)
#define S25FL512_OFFSET_TSDB            (S25FL512_OFFSET_KVDB + S25FL512_SIZE_KVDB)
#define S25FL512_OFFSET_FS              (S25FL512_OFFSET_TSDB + S25FL512_SIZE_TSDB)
#define S25FL512_SIZE_FS                (S25FL512_SIZE_GRANULARITY_TOTAL - S25FL512_OFFSET_FS)
#ifndef S25FL512_ENABLE_FS
#define S25FL512_FS                     ""
#endif
#endif

#ifdef BSP_USING_AT45DB321E
//at45db321e定义,无需修改
#define AT45DB321E_START_ADRESS         (0)
#define AT45DB321E_DEV_NAME             "at45db321e"
#define BLK_AT45DB321E                  "spinor4"
#ifndef AT45DB321E_ENABLE_FS
#define AT45DB321E_FS                   ""
#endif
#endif

#ifdef BSP_USING_NORFLASH
//norflash定义,无需修改
#define NORFLASH_DEV_NAME               "fmc_nor"
#define BLK_NOR                         "nor"
#define NORFLASH_OFFSET_DOWNLOAD        (0)
#define NORFLASH_OFFSET_FACTORY         (NORFLASH_OFFSET_DOWNLOAD + NORFLASH_SIZE_DOWNLOAD)
#define NORFLASH_OFFSET_BIN             (NORFLASH_OFFSET_FACTORY + NORFLASH_SIZE_FACTORY)
#define NORFLASH_OFFSET_ETC             (NORFLASH_OFFSET_BIN + NORFLASH_SIZE_BIN)
#define NORFLASH_OFFSET_LIB             (NORFLASH_OFFSET_ETC + NORFLASH_SIZE_ETC)
#define NORFLASH_OFFSET_USR             (NORFLASH_OFFSET_LIB + NORFLASH_SIZE_LIB)
#define NORFLASH_OFFSET_KVDB            (NORFLASH_OFFSET_USR + NORFLASH_SIZE_USR)
#define NORFLASH_OFFSET_TSDB            (NORFLASH_OFFSET_KVDB + NORFLASH_SIZE_KVDB)
#define NORFLASH_OFFSET_FS              (NORFLASH_OFFSET_TSDB + NORFLASH_SIZE_TSDB)
#define NORFLASH_SIZE_FS                (NORFLASH_SIZE_GRANULARITY_TOTAL - NORFLASH_OFFSET_FS)
#ifndef NORFLASH_ENABLE_FS
#define NORFLASH_FS                     ""
#endif
#endif

#ifdef BSP_USING_EMMC
//emmc定义,无需修改
#define EMMC_DEV_NAME                   "sdmmc"
#define BLK_EMMC                        "emmc"
#define EMMC_START_ADRESS               (0)
#define EMMC_BLK_SIZE                   (512)
#define EMMC_OFFSET_DOWNLOAD            (0)
#define EMMC_OFFSET_FACTORY             (EMMC_OFFSET_DOWNLOAD + EMMC_SIZE_DOWNLOAD)
#define EMMC_OFFSET_BIN                 (EMMC_OFFSET_FACTORY + EMMC_SIZE_FACTORY)
#define EMMC_OFFSET_ETC                 (EMMC_OFFSET_BIN + EMMC_SIZE_BIN)
#define EMMC_OFFSET_LIB                 (EMMC_OFFSET_ETC + EMMC_SIZE_ETC)
#define EMMC_OFFSET_USR                 (EMMC_OFFSET_LIB + EMMC_SIZE_LIB)
#define EMMC_OFFSET_KVDB                (EMMC_OFFSET_USR + EMMC_SIZE_USR)
#define EMMC_OFFSET_TSDB                (EMMC_OFFSET_KVDB + EMMC_SIZE_KVDB)
#define EMMC_OFFSET_FS                  (EMMC_OFFSET_TSDB + EMMC_SIZE_TSDB)
#ifndef EMMC_ENABLE_FS
#define EMMC_FS                         ""
#endif
#endif

#ifdef BSP_USING_EEPROM_24Cxx
//24cxx定义,无需修改
#define EEPROM_24Cxx_DEV_NAME          "24cxx"
#define BLK_EEPROM                     "eeprom"
#define EEPROM_24Cxx_START_ADRESS      (0)
#endif

#ifdef BSP_USING_MAX31826
//max31826定义,无需修改
#define MAX31826_DEV_NAME               "max31826"
#define BLK_MAX31826                    "max31826"
#define MAX31826_START_ADRESS           (0)
#endif

#ifdef BSP_USING_SDCARD
//sdcard定义,无需修改
#define BLK_SDCARD                      "sdcard"
#endif

#ifdef BSP_USING_USB
//msc定义,无需修改
#define BLK_USBH_UDISK                  "udisk"
#define UDISK_BLK_SIZE                  (512)
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

#include "drv_fal.h"

struct rt_device *fal_dev_mtd_nor_device_create(struct fal_flash64_dev *fal_dev);
struct rt_device *fal_dev_blk_device_create(struct fal_flash64_dev *fal_dev);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATIONS_FAL_CFG_H_ */

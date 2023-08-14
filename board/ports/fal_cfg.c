/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-11-22     lvhan       the first version
 */
#include <fal.h>

#ifdef BSP_USING_ON_CHIP_FLASH
    extern const struct fal_flash_dev onchip128_flash;
#ifdef FLASH_USING_BLK256
    extern const struct fal_flash_dev onchip256_flash;
#endif
#endif

#ifdef BSP_USING_FM25xx
    extern const struct fal_flash_dev fm25xx_fram;
#endif

#ifdef BSP_USING_SPI_FLASH
    extern struct fal_flash_dev spiflash;
#endif

#ifdef BSP_USING_S25FL512
    extern struct fal_flash_dev s25fl512_flash;
#endif

#ifdef BSP_FMCSRAM_ENABLE_BLK
    extern const struct fal_flash_dev sram_flash;
#endif

#ifdef BSP_SDRAM_ENABLE_BLK
    extern const struct fal_flash_dev sdram_flash;
#endif

#ifdef BSP_USING_NORFLASH
    extern const struct fal_flash_dev nor_flash;
#endif

#ifdef BSP_USING_EEPROM_24Cxx
    extern const struct fal_flash_dev hg24cxx_eeprom;
#endif

const struct fal_flash_dev * const device_table[] =
{
#ifdef BSP_USING_ON_CHIP_FLASH
    &onchip128_flash,
#ifdef FLASH_USING_BLK256
    &onchip256_flash,
#endif
#endif

#ifdef BSP_USING_FM25xx
    &fm25xx_fram,
#endif

#ifdef BSP_USING_SPI_FLASH
    &spiflash,
#endif

#ifdef BSP_USING_S25FL512
    &s25fl512_flash,
#endif

#ifdef BSP_FMCSRAM_ENABLE_BLK
    &sram_flash,
#endif

#ifdef BSP_SDRAM_ENABLE_BLK
    &sdram_flash,
#endif

#ifdef BSP_USING_NORFLASH
    &nor_flash,
#endif

#ifdef BSP_USING_EEPROM_24Cxx
    &hg24cxx_eeprom,
#endif
};
const size_t device_table_len = sizeof(device_table) / sizeof(struct fal_flash_dev *);

const struct fal_partition partition_table_def[] =
{
#ifdef BSP_USING_ON_CHIP_FLASH
    {FAL_PART_MAGIC_WORD,"bootloader",        FLASH_DEV_NAME,       FLASH_OFFSET_BOOTLOADER,               FLASH_SIZE_BOOTLOADER, 0},
#if FLASH_SIZE_APP > 0
    {FAL_PART_MAGIC_WORD,       "app",        FLASH_DEV_NAME,              FLASH_OFFSET_APP,                      FLASH_SIZE_APP, 0},
#endif
#if FLASH_SIZE_DOWNLOAD > 0
    {FAL_PART_MAGIC_WORD,  "download",        FLASH_DEV_NAME,         FLASH_OFFSET_DOWNLOAD,                 FLASH_SIZE_DOWNLOAD, 0},
#endif
#if FLASH_SIZE_FACTORY > 0
    {FAL_PART_MAGIC_WORD,   "factory",        FLASH_DEV_NAME,          FLASH_OFFSET_FACTORY,                  FLASH_SIZE_FACTORY, 0},
#endif
#ifdef FLASH_USING_BLK256
    {FAL_PART_MAGIC_WORD,    "onchip",     FLASH_BLK256_NAME,                             0,                   FLASH_BLK256_SIZE, 0},
#endif
#endif

#ifdef BSP_USING_FM25xx
    {FAL_PART_MAGIC_WORD,    BLK_FRAM,       FM25xx_DEV_NAME,                             0,       FM25xx_SIZE_GRANULARITY_TOTAL, 0},
#endif

#ifdef BSP_USING_SPI_FLASH
    {FAL_PART_MAGIC_WORD,BLK_SPI_FLASH,   SPI_FLASH_DEV_NAME,                             0,    SPI_FLASH_SIZE_GRANULARITY_TOTAL, 0},
#endif

#ifdef BSP_USING_S25FL512
    {FAL_PART_MAGIC_WORD, BLK_S25FL512,    S25FL512_DEV_NAME,                             0,     S25FL512_SIZE_GRANULARITY_TOTAL, 0},
#endif

#ifdef BSP_FMCSRAM_ENABLE_BLK
    {FAL_PART_MAGIC_WORD,  BLK_FMCSRAM,     FMCSRAM_DEV_NAME,                             0,                    BSP_FMCSRAM_SIZE, 0},
#endif

#ifdef BSP_SDRAM_ENABLE_BLK
    {FAL_PART_MAGIC_WORD,  BLK_SDRAM,     SDRAM_DEV_NAME,                             0,         SDRAM_SIZE - BSP_SDRAM_BLK_SIZE, 0},
#endif

#ifdef BSP_USING_NORFLASH
#if NORFLASH_SIZE_APP > 0
    {FAL_PART_MAGIC_WORD,       "app",     NORFLASH_DEV_NAME,           NORFLASH_OFFSET_APP,                   NORFLASH_SIZE_APP, 0},
#endif
#if NORFLASH_SIZE_DOWNLOAD > 0
    {FAL_PART_MAGIC_WORD,  "download",     NORFLASH_DEV_NAME,      NORFLASH_OFFSET_DOWNLOAD,              NORFLASH_SIZE_DOWNLOAD, 0},
#endif
#if NORFLASH_SIZE_FACTORY > 0
    {FAL_PART_MAGIC_WORD,   "factory",     NORFLASH_DEV_NAME,       NORFLASH_OFFSET_FACTORY,               NORFLASH_SIZE_FACTORY, 0},
#endif
#if NORFLASH_SIZE_BIN > 0
    {FAL_PART_MAGIC_WORD,       "bin",     NORFLASH_DEV_NAME,           NORFLASH_OFFSET_BIN,                   NORFLASH_SIZE_BIN, 0},
#endif
#if NORFLASH_SIZE_ETC > 0
    {FAL_PART_MAGIC_WORD,       "etc",     NORFLASH_DEV_NAME,           NORFLASH_OFFSET_ETC,                   NORFLASH_SIZE_ETC, 0},
#endif
#if NORFLASH_SIZE_LIB > 0
    {FAL_PART_MAGIC_WORD,       "lib",     NORFLASH_DEV_NAME,           NORFLASH_OFFSET_LIB,                   NORFLASH_SIZE_LIB, 0},
#endif
#if NORFLASH_SIZE_USR > 0
    {FAL_PART_MAGIC_WORD,       "usr",     NORFLASH_DEV_NAME,           NORFLASH_OFFSET_USR,                   NORFLASH_SIZE_USR, 0},
#endif
#if NORFLASH_SIZE_KVDB > 0
    {FAL_PART_MAGIC_WORD,      "kvdb",     NORFLASH_DEV_NAME,          NORFLASH_OFFSET_KVDB,                  NORFLASH_SIZE_KVDB, 0},
#endif
#if NORFLASH_SIZE_TSDB > 0
    {FAL_PART_MAGIC_WORD,      "tsdb",     NORFLASH_DEV_NAME,          NORFLASH_OFFSET_TSDB,                  NORFLASH_SIZE_TSDB, 0},
#endif
#if NORFLASH_SIZE_FS > 0 && defined(NORFLASH_ENABLE_FS)
    {FAL_PART_MAGIC_WORD,     BLK_NOR,     NORFLASH_DEV_NAME,            NORFLASH_OFFSET_FS,                    NORFLASH_SIZE_FS, 0},
#endif
#endif

#ifdef BSP_USING_EEPROM_24Cxx
    {FAL_PART_MAGIC_WORD,  BLK_EEPROM, EEPROM_24Cxx_DEV_NAME,                             0, EEPROM_24Cxx_SIZE_GRANULARITY_TOTAL, 0},
#endif
};
const size_t partition_table_def_len = sizeof(partition_table_def) / sizeof(struct fal_partition);

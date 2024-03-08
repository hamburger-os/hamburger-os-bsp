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


enum
{
#if MAX31826_SEN_ALL > 0
    MAX31826_SEN1,
#endif

#if MAX31826_SEN_ALL > 1
    MAX31826_SEN2,
#endif

#if MAX31826_SEN_ALL > 2
    MAX31826_SEN3,
#endif

#if MAX31826_SEN_ALL > 3
    MAX31826_SEN4,
#endif
};

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

#ifdef BSP_USING_AT45DB321E
    extern struct fal_flash_dev at45db321e_flash;
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

#if EMMC_OFFSET_FS > 0
    extern const struct fal_flash_dev emmc_fal_flash;
#endif

#ifdef BSP_USING_EEPROM_24Cxx
    extern const struct fal_flash_dev hg24cxx_eeprom;
#endif

#ifdef BSP_USING_MAX31826
    extern const struct fal_flash_dev max31826_flash;
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

#ifdef BSP_USING_AT45DB321E
    &at45db321e_flash,
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

#if EMMC_OFFSET_FS > 0
    &emmc_fal_flash,
#endif

#ifdef BSP_USING_EEPROM_24Cxx
    &hg24cxx_eeprom,
#endif

#ifdef BSP_USING_MAX31826
    &max31826_flash,
#endif
};
const size_t device_table_len = sizeof(device_table) / sizeof(struct fal_flash_dev *);

const struct fal_partition partition_table_def[] =
{
#ifdef BSP_USING_ON_CHIP_FLASH
#if FLASH_SIZE_BOOTLOADER > 0
    {FAL_PART_MAGIC_WORD,"bootloader",        FLASH_DEV_NAME,       FLASH_OFFSET_BOOTLOADER,               FLASH_SIZE_BOOTLOADER, 0},
#endif
#if FLASH_SIZE_APP > 0
    {FAL_PART_MAGIC_WORD,       "app",        FLASH_DEV_NAME,              FLASH_OFFSET_APP,                      FLASH_SIZE_APP, 0},
#endif
#if FLASH_SIZE_DOWNLOAD > 0
    {FAL_PART_MAGIC_WORD,  "download",        FLASH_DEV_NAME,         FLASH_OFFSET_DOWNLOAD,                 FLASH_SIZE_DOWNLOAD, 0},
#endif
#if FLASH_SIZE_FACTORY > 0
    {FAL_PART_MAGIC_WORD,   "factory",        FLASH_DEV_NAME,          FLASH_OFFSET_FACTORY,                  FLASH_SIZE_FACTORY, 0},
#endif
#if FLASH_SIZE_KVDB > 0
    {FAL_PART_MAGIC_WORD,      "kvdb",        FLASH_DEV_NAME,             FLASH_OFFSET_KVDB,                     FLASH_SIZE_KVDB, 0},
#endif
#ifdef FLASH_USING_BLK256
    {FAL_PART_MAGIC_WORD,    "onchip",     FLASH_BLK256_NAME,                             0,                   FLASH_BLK256_SIZE, 0},
#endif
#endif

#ifdef BSP_USING_MAX31826

#ifdef MAX31826_USING_IO
    {FAL_PART_MAGIC_WORD,BLK_MAX31826,     MAX31826_DEV_NAME,                             0,     MAX31826_SIZE_GRANULARITY_TOTAL, 0},
#endif

#ifdef MAX31826_USING_I2C_DS2484
#if MAX31826_SEN_ALL > 0
    {FAL_PART_MAGIC_WORD,BLK_MAX31826_1,     MAX31826_DEV_NAME,          MAX31826_SIZE_GRANULARITY_TOTAL*MAX31826_SEN1,     MAX31826_SIZE_GRANULARITY_TOTAL, 0},
#endif
#if MAX31826_SEN_ALL > 1
    {FAL_PART_MAGIC_WORD,BLK_MAX31826_2,     MAX31826_DEV_NAME,          MAX31826_SIZE_GRANULARITY_TOTAL*MAX31826_SEN2,     MAX31826_SIZE_GRANULARITY_TOTAL, 0},
#endif
#if MAX31826_SEN_ALL > 2
    {FAL_PART_MAGIC_WORD,BLK_MAX31826_3,     MAX31826_DEV_NAME,          MAX31826_SIZE_GRANULARITY_TOTAL*MAX31826_SEN3,     MAX31826_SIZE_GRANULARITY_TOTAL, 0},
#endif
#if MAX31826_SEN_ALL > 3
    {FAL_PART_MAGIC_WORD,BLK_MAX31826_4,     MAX31826_DEV_NAME,          MAX31826_SIZE_GRANULARITY_TOTAL*MAX31826_SEN4,     MAX31826_SIZE_GRANULARITY_TOTAL, 0},
#endif
#endif

#endif

#ifdef BSP_USING_FM25xx
    {FAL_PART_MAGIC_WORD,    BLK_FRAM,       FM25xx_DEV_NAME,                             0,       FM25xx_SIZE_GRANULARITY_TOTAL, 0},
#endif

#ifdef BSP_USING_SPI_FLASH
#if SPI_FLASH_SIZE_DOWNLOAD > 0
    {FAL_PART_MAGIC_WORD,  "download",     SPI_FLASH_DEV_NAME,      SPI_FLASH_OFFSET_DOWNLOAD,              SPI_FLASH_SIZE_DOWNLOAD, 0},
#endif
#if SPI_FLASH_SIZE_FACTORY > 0
    {FAL_PART_MAGIC_WORD,   "factory",     SPI_FLASH_DEV_NAME,       SPI_FLASH_OFFSET_FACTORY,               SPI_FLASH_SIZE_FACTORY, 0},
#endif
#if SPI_FLASH_SIZE_BIN > 0
    {FAL_PART_MAGIC_WORD,       "bin",     SPI_FLASH_DEV_NAME,           SPI_FLASH_OFFSET_BIN,                   SPI_FLASH_SIZE_BIN, 0},
#endif
#if SPI_FLASH_SIZE_ETC > 0
    {FAL_PART_MAGIC_WORD,       "etc",     SPI_FLASH_DEV_NAME,           SPI_FLASH_OFFSET_ETC,                   SPI_FLASH_SIZE_ETC, 0},
#endif
#if SPI_FLASH_SIZE_LIB > 0
    {FAL_PART_MAGIC_WORD,       "lib",     SPI_FLASH_DEV_NAME,           SPI_FLASH_OFFSET_LIB,                   SPI_FLASH_SIZE_LIB, 0},
#endif
#if SPI_FLASH_SIZE_USR > 0
    {FAL_PART_MAGIC_WORD,       "usr",     SPI_FLASH_DEV_NAME,           SPI_FLASH_OFFSET_USR,                   SPI_FLASH_SIZE_USR, 0},
#endif
#if SPI_FLASH_SIZE_KVDB > 0
    {FAL_PART_MAGIC_WORD,      "kvdb",     SPI_FLASH_DEV_NAME,          SPI_FLASH_OFFSET_KVDB,                  SPI_FLASH_SIZE_KVDB, 0},
#endif
#if SPI_FLASH_SIZE_TSDB > 0
    {FAL_PART_MAGIC_WORD,      "tsdb",     SPI_FLASH_DEV_NAME,          SPI_FLASH_OFFSET_TSDB,                  SPI_FLASH_SIZE_TSDB, 0},
#endif
#if SPI_FLASH_SIZE_FS > 0
    {FAL_PART_MAGIC_WORD,BLK_SPI_FLASH,     SPI_FLASH_DEV_NAME,            SPI_FLASH_OFFSET_FS,                    SPI_FLASH_SIZE_FS, 0},
#endif
#endif

#ifdef BSP_USING_S25FL512
#if S25FL512_SIZE_DOWNLOAD > 0
    {FAL_PART_MAGIC_WORD,  "download",     S25FL512_DEV_NAME,      S25FL512_OFFSET_DOWNLOAD,              S25FL512_SIZE_DOWNLOAD, 0},
#endif
#if S25FL512_SIZE_FACTORY > 0
    {FAL_PART_MAGIC_WORD,   "factory",     S25FL512_DEV_NAME,       S25FL512_OFFSET_FACTORY,               S25FL512_SIZE_FACTORY, 0},
#endif
#if S25FL512_SIZE_BIN > 0
    {FAL_PART_MAGIC_WORD,       "bin",     S25FL512_DEV_NAME,           S25FL512_OFFSET_BIN,                   S25FL512_SIZE_BIN, 0},
#endif
#if S25FL512_SIZE_ETC > 0
    {FAL_PART_MAGIC_WORD,       "etc",     S25FL512_DEV_NAME,           S25FL512_OFFSET_ETC,                   S25FL512_SIZE_ETC, 0},
#endif
#if S25FL512_SIZE_LIB > 0
    {FAL_PART_MAGIC_WORD,       "lib",     S25FL512_DEV_NAME,           S25FL512_OFFSET_LIB,                   S25FL512_SIZE_LIB, 0},
#endif
#if S25FL512_SIZE_USR > 0
    {FAL_PART_MAGIC_WORD,       "usr",     S25FL512_DEV_NAME,           S25FL512_OFFSET_USR,                   S25FL512_SIZE_USR, 0},
#endif
#if S25FL512_SIZE_KVDB > 0
    {FAL_PART_MAGIC_WORD,      "kvdb",     S25FL512_DEV_NAME,          S25FL512_OFFSET_KVDB,                  S25FL512_SIZE_KVDB, 0},
#endif
#if S25FL512_SIZE_TSDB > 0
    {FAL_PART_MAGIC_WORD,      "tsdb",     S25FL512_DEV_NAME,          S25FL512_OFFSET_TSDB,                  S25FL512_SIZE_TSDB, 0},
#endif
#if S25FL512_SIZE_FS > 0
    {FAL_PART_MAGIC_WORD,BLK_S25FL512,     S25FL512_DEV_NAME,            S25FL512_OFFSET_FS,                    S25FL512_SIZE_FS, 0},
#endif
#endif

#ifdef BSP_USING_AT45DB321E
    {FAL_PART_MAGIC_WORD, BLK_AT45DB321E, AT45DB321E_DEV_NAME,                            0,   AT45DB321E_SIZE_GRANULARITY_TOTAL, 0},
#endif

#ifdef BSP_FMCSRAM_ENABLE_BLK
    {FAL_PART_MAGIC_WORD,  BLK_FMCSRAM,     FMCSRAM_DEV_NAME,                             0,                    BSP_FMCSRAM_SIZE, 0},
#endif

#ifdef BSP_SDRAM_ENABLE_BLK
    {FAL_PART_MAGIC_WORD,  BLK_SDRAM,     SDRAM_DEV_NAME,                             0,         SDRAM_SIZE - BSP_SDRAM_BLK_SIZE, 0},
#endif

#ifdef BSP_USING_NORFLASH
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
#if NORFLASH_SIZE_FS > 0
    {FAL_PART_MAGIC_WORD,     BLK_NOR,     NORFLASH_DEV_NAME,            NORFLASH_OFFSET_FS,                    NORFLASH_SIZE_FS, 0},
#endif
#endif

#ifdef BSP_USING_EMMC
#if EMMC_SIZE_DOWNLOAD > 0
    {FAL_PART_MAGIC_WORD,  "download",     EMMC_DEV_NAME,      EMMC_OFFSET_DOWNLOAD,              EMMC_SIZE_DOWNLOAD, 0},
#endif
#if EMMC_SIZE_FACTORY > 0
    {FAL_PART_MAGIC_WORD,   "factory",     EMMC_DEV_NAME,       EMMC_OFFSET_FACTORY,               EMMC_SIZE_FACTORY, 0},
#endif
#if EMMC_SIZE_BIN > 0
    {FAL_PART_MAGIC_WORD,       "bin",     EMMC_DEV_NAME,           EMMC_OFFSET_BIN,                   EMMC_SIZE_BIN, 0},
#endif
#if EMMC_SIZE_ETC > 0
    {FAL_PART_MAGIC_WORD,       "etc",     EMMC_DEV_NAME,           EMMC_OFFSET_ETC,                   EMMC_SIZE_ETC, 0},
#endif
#if EMMC_SIZE_LIB > 0
    {FAL_PART_MAGIC_WORD,       "lib",     EMMC_DEV_NAME,           EMMC_OFFSET_LIB,                   EMMC_SIZE_LIB, 0},
#endif
#if EMMC_SIZE_USR > 0
    {FAL_PART_MAGIC_WORD,       "usr",     EMMC_DEV_NAME,           EMMC_OFFSET_USR,                   EMMC_SIZE_USR, 0},
#endif
#if EMMC_SIZE_KVDB > 0
    {FAL_PART_MAGIC_WORD,      "kvdb",     EMMC_DEV_NAME,          EMMC_OFFSET_KVDB,                  EMMC_SIZE_KVDB, 0},
#endif
#if EMMC_SIZE_TSDB > 0
    {FAL_PART_MAGIC_WORD,      "tsdb",     EMMC_DEV_NAME,          EMMC_OFFSET_TSDB,                  EMMC_SIZE_TSDB, 0},
#endif
#endif

#ifdef BSP_USING_EEPROM_24Cxx
    {FAL_PART_MAGIC_WORD,  BLK_EEPROM, EEPROM_24Cxx_DEV_NAME,                             0, EEPROM_24Cxx_SIZE_GRANULARITY_TOTAL, 0},
#endif
};
const size_t partition_table_def_len = sizeof(partition_table_def) / sizeof(struct fal_partition);

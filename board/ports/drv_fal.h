/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-18     lvhan       the first version
 */
#ifndef BOARD_PORTS_DRV_FAL_H_
#define BOARD_PORTS_DRV_FAL_H_

struct fal_flash64_dev
{
    char name[RT_NAME_MAX];

    /* flash device start address and len  */
    uint64_t addr;
    uint64_t len;
    /* the block size in the flash for erase minimum granularity */
    size_t blk_size;

    struct
    {
        int (*init)(void);
        uint32_t (*read)(uint64_t offset, uint8_t *buf, uint32_t size);
        uint32_t (*write)(uint64_t offset, const uint8_t *buf, uint32_t size);
        uint32_t (*erase)(uint64_t offset, uint32_t size);
    } ops;

    /* write minimum granularity, unit: bit.
       1(nor flash)/ 8(stm32f2/f4)/ 32(stm32f1)/ 64(stm32l4)
       0 will not take effect. */
    size_t write_gran;
};

#endif /* BOARD_PORTS_DRV_FAL_H_ */

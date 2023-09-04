/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-01     lvhan       the first version
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

int8_t get_sys_version(uint8_t *version)
{
    return 0;
}

int8_t get_cpu_temp(int32_t *temp)
{
    return 0;
}

int8_t get_chip_id_temp(uint8_t *id, int32_t *temp)
{
    return 0;
}

int8_t get_times_count(uint32_t *times, uint16_t *count)
{
    return 0;
}

int16_t sysinfofix_blk_read(uint8_t *buf, size_t size)
{
    return size;
}

int16_t sysinfofix_blk_write(const uint8_t *buf, size_t size)
{
    return size;
}

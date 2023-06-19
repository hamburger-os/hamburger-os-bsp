/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-06     lvhan       the first version
 */
#ifndef PACKAGES_SYSINFO_H_
#define PACKAGES_SYSINFO_H_

struct __attribute__ ((packed)) SysInfoDef
{
    uint32_t    cpu_id[3];
    float       cpu_temp;
    uint32_t    flash_size;//KB
    uint32_t    sram_size;//KB
    uint32_t    sys_clock;//M
    uint32_t    mem_size;//KB
    uint8_t     chip_id[8];
    float       chip_temp;
    uint32_t    times;
    uint16_t    count;
};

void sysinfo_get(struct SysInfoDef *info);

#endif /* PACKAGES_SYSINFO_H_ */

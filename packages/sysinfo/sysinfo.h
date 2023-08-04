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
    uint16_t    version;
    uint32_t    cpu_id[3];
    float       cpu_temp;
    uint8_t     chip_id[8];
    float       chip_temp;
    uint32_t    times;
    uint16_t    count;
    uint8_t     mac[3][6];
};

void sysinfo_get(struct SysInfoDef *info);

#endif /* PACKAGES_SYSINFO_H_ */

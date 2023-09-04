/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-01     lvhan       the first version
 */
#ifndef PACKAGES_SYSINFO_PLATFORM_PUBLIC_H_
#define PACKAGES_SYSINFO_PLATFORM_PUBLIC_H_

#ifdef __cplusplus
extern "C" {
#endif

int8_t get_sys_version(uint8_t *version);
int8_t get_cpu_temp(int32_t *temp);
int8_t get_chip_id_temp(uint8_t *id, int32_t *temp);
int8_t get_times_count(uint32_t *times, uint16_t *count);

int16_t sysinfofix_blk_read(uint8_t *buf, size_t size);
int16_t sysinfofix_blk_write(const uint8_t *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* PACKAGES_SYSINFO_PLATFORM_PUBLIC_H_ */

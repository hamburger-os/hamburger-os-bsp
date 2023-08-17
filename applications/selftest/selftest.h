/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-31     lvhan       the first version
 */
#ifndef APPLICATIONS_SELFTEST_SELFTEST_H_
#define APPLICATIONS_SELFTEST_SELFTEST_H_

typedef struct
{
    char *gpio_devname[6][2];
    char *key_devname;
    char *fs_path[6];

    rt_base_t gpio_pin[6][2];
    rt_base_t key_pin;
} SelftestlUserData;

void selftest_gpio_init(SelftestlUserData *puserdata);
void selftest_gpio_test(SelftestlUserData *puserdata);

void selftest_fs_test(SelftestlUserData *puserdata);

#endif /* APPLICATIONS_SELFTEST_SELFTEST_H_ */

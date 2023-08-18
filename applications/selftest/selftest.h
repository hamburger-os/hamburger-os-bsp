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
    char *i2c_devname;
    char *wav_path;
    char *uart_devname[3][2];
    char *can_devname[2][2];

    rt_base_t gpio_pin[6][2];
    rt_base_t key_pin;
    struct fal_partition *i2c_dev;
    rt_device_t uart_dev[3][2];
    rt_device_t can_dev[2][2];
} SelftestlUserData;

void selftest_gpio_test(SelftestlUserData *puserdata);

void selftest_fs_test(SelftestlUserData *puserdata);

void selftest_i2c_test(SelftestlUserData *puserdata);

void selftest_i2s_test(SelftestlUserData *puserdata);

void selftest_uart_test(SelftestlUserData *puserdata);

void selftest_can_test(SelftestlUserData *puserdata);

#endif /* APPLICATIONS_SELFTEST_SELFTEST_H_ */

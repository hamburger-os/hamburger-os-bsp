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

enum {
    RESULT_MAX31826 = 0,
    RESULT_DS1682,
    RESULT_GPIO_LOW_F,
    RESULT_GPIO_HIGH_F,
    RESULT_GPIO_LOW_R,
    RESULT_GPIO_HIGH_R,
    RESULT_FRAM,
    RESULT_SPINOR64,
    RESULT_NOR,
    RESULT_EMMC,
    RESULT_UDISK,
    RESULT_SPINOR4,
    RESULT_EEPROM,
    RESULT_UART2_UART2,
    RESULT_UART3_UART4,
    RESULT_UART4_UART3,
    RESULT_CAN1_CAN2,
    RESULT_CAN2_CAN1,
    RESULT_ETH1_ETH2,
    RESULT_ETH2_ETH1,
};

typedef struct
{
    char *name;
    uint8_t result;
} SelftestResult;

typedef struct
{
    char *gpio_devname[6][2];
    char *key_devname;
    char *fs_path[5];
    char *spi_devname;
    char *spi_devname_cs;
    char *i2c_devname;
    char *wav_path;
    char *uart_devname[3][2];
    char *can_devname[2][2];
    char *eth_devname[2][2];

    rt_base_t gpio_pin[6][2];
    rt_base_t key_pin;
    struct fal_partition *i2c_dev;
    rt_device_t uart_dev[3][2];
    rt_device_t can_dev[2][2];
    rt_device_t eth_dev[2][2];
    int sock[2][2];

    SelftestResult result[20];
} SelftestUserData;

void selftest_gpio_test(SelftestUserData *puserdata);
void selftest_key_test(SelftestUserData *puserdata);
void selftest_fs_test(SelftestUserData *puserdata);
void selftest_spi_test(SelftestUserData *puserdata);
void selftest_i2c_test(SelftestUserData *puserdata);
void selftest_i2s_test(SelftestUserData *puserdata);
void selftest_uart_test(SelftestUserData *puserdata);
void selftest_can_test(SelftestUserData *puserdata);
void selftest_eth_test(SelftestUserData *puserdata);
void selftest_tcpip_test(SelftestUserData *puserdata);

#endif /* APPLICATIONS_SELFTEST_SELFTEST_H_ */

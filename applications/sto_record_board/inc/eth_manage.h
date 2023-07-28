/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-30     zm       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_ETH_MANAGE_H_
#define APPLICATIONS_STO_RECORD_BOARD_ETH_MANAGE_H_

#include <rtthread.h>

#define ETH_MANAGE_CHANNEL_MAX 0x03U

typedef enum {
    ETHManageChannel1 = 0x00U,
    ETHManageChannel2 = 0x01U,
    ETHManageChannel3 = 0x02U,
    ETHManageChannelALL
} ETHManageCHannel;

void ETHManageTX(ETHManageCHannel channel, const void *buffer, rt_uint16_t size);
void ETHManageSetRXCallback(ETHManageCHannel ch, rt_err_t (*rx_ind)(rt_device_t dev,rt_size_t size));
void ETHManageInit(void);

void ETHManageTestThreadInit(void);

#endif /* APPLICATIONS_STO_RECORD_BOARD_ETH_MANAGE_H_ */

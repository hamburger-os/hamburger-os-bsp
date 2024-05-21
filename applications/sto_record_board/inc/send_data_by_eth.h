/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-29     zm       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_INC_SEND_DATA_BY_ETH_H_
#define APPLICATIONS_STO_RECORD_BOARD_INC_SEND_DATA_BY_ETH_H_

#include <rtthread.h>

void Check_exp_2_mainctl_SendFlag(void);
void linklayer_sendFrame( uint8_t chl, uint8_t pri, uint8_t no, uint8_t *pdata, uint8_t dataLen );

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_SEND_DATA_BY_ETH_H_ */

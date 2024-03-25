/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-20     zm       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_INC_ETH_THREAD_H_
#define APPLICATIONS_STO_RECORD_BOARD_INC_ETH_THREAD_H_

#include "data_handle.h"

#define ETH_RX_BUF_MAX_NUM  (1500U)

typedef enum {
    ETH_CH_INEX_1 = 1,  /* I系网口 */
    ETH_CH_INEX_2 = 2,  /* II系网口 */
    ETH_CH_INEX_3 = 3,  /* 前面板 */
    ETH_CH_INEX_MAX
} ETH_CH_INEX;

/* 单字节对齐 */
#pragma pack(1)

typedef struct
{
    uint16_t len;
    uint8_t data_u8[ETH_RX_BUF_MAX_NUM];
} S_ETH_BUF;

#pragma pack()

extern uint8_t record_mac_i[];
extern uint8_t record_mac_ii[];
extern uint8_t zk_mac_i[];
extern uint8_t zk_mac_ii[];

rt_err_t ETHInit(void);
int ETHThreadInit(void);
int linke_eth_send(ETH_CH_INEX ch, uint8_t *pdata, uint16_t len);

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_ETH_THREAD_H_ */

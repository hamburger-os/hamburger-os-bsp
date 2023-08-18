/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-21     zm       the first version
 */
#ifndef BOARD_PORTS_KSZ8851_KSZ8851_LEP_H_
#define BOARD_PORTS_KSZ8851_KSZ8851_LEP_H_

#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>

#define LEP_MAC_PKT_MAX_LEN   (1536U)              /* 链路层最大数据包长度 */

#define LEP_RX_BUF_NUM    (50)

#define LEP_RBF_TV        (1U)
#define LEP_RBF_RV        (2U)

typedef struct tagLEP_BUF /* 接收缓冲区 */
{
    struct tagLEP_BUF *pnext; /* 下一个 */
    uint16_t len; /* 长度 */
    uint16_t flag; /* 收发标志位 */
    uint8_t buf[LEP_MAC_PKT_MAX_LEN]; /* 1536 */
} S_LEP_BUF;

typedef struct eth_interface /* 接收环形缓冲区，应用程序维护 */
{
    S_LEP_BUF *prx_rptr; /* 接收缓冲区读位置指针 */
    S_LEP_BUF *prx_wptr; /* 接收缓冲区写位置指针 */
    S_LEP_BUF *rx_buf;
    S_LEP_BUF tx_buf;
} S_ETH_IF;

/* Exported function prototypes ----------------------------------------------*/
rt_err_t lep_eth_if_init(S_ETH_IF *ps_eth_if);
S_LEP_BUF *lep_get_free_buf(S_ETH_IF *ps_eth_if);
S_LEP_BUF *lep_if_is_received(const S_ETH_IF *ps_eth_if);
void lep_if_release_rptr(S_ETH_IF *ps_eth_if);

#endif /* BOARD_PORTS_KSZ8851_KSZ8851_LEP_H_ */

/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-25     zylx         first version
 */

#ifndef __DRV_ETH_H__
#define __DRV_ETH_H__

#include <board.h>
#include <netif/ethernetif.h>

#define MAC_ADDR_LEN 6

struct rt_stm32_eth
{
    /* inherit from ethernet device */
    struct eth_device parent;
#ifndef PHY_USING_INTERRUPT_MODE
    rt_timer_t poll_link_timer;
#endif

    ETH_HandleTypeDef heth;
    ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT], DMATxDscrTab[ETH_RX_DESC_CNT];
    ETH_TxPacketConfig TxConfig;

    struct rt_completion TxPkt_completion;

    uint8_t phy_addr;
    /* interface address info, hw address */
    uint8_t mac[MAC_ADDR_LEN];
};

#endif /* __DRV_ETH_H__ */

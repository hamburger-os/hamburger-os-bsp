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
#include "drv_link_layer_list.h"

#define MAC_ADDR_LEN 6

struct rt_stm32_eth
{
    /* inherit from ethernet device */
    struct eth_device parent;
#ifndef PHY_USING_INTERRUPT_MODE
    rt_timer_t poll_link_timer;
#endif

    ETH_HandleTypeDef heth;
    ETH_TxPacketConfig TxConfig;

    struct rt_completion TxPkt_completion;

    char *dev_name;
    uint8_t phy_addr;
    /* interface address info, hw address */
    uint8_t mac[MAC_ADDR_LEN];

    struct rt_mutex eth_mux;
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
    S_ETH_IF link_layer_buf;
    uint32_t rx_num;
#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */
};

#endif /* __DRV_ETH_H__ */

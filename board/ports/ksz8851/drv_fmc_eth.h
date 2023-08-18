/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-22     lvhan       the first version
 */
#ifndef APPLICATIONS_DRV_FMC_ETH_H_
#define APPLICATIONS_DRV_FMC_ETH_H_

#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "drv_config.h"
#include <netif/ethernetif.h>
#include <lwipopts.h>

#include "ksz8851_lep.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ADDR_LEN 6

#define ETH_RXBUF_SIZE 2000
#define ETH_TXBUF_SIZE 1516

#define MAX_MCAST_LST (32)     /** 组播地址列表数 */
#define HW_MCAST_SIZE (8)      /** Hash表大小 */
#define ETH_ALEN (6)           /** 以太网MAC地址字节数 */
#define KSZ_MAX_RFRM_THD (255) /** 接收帧最大缓冲数阈值 */

/** 数据包头控制信息结构 */
typedef struct tagFRAME_HEAD
{
    uint16_t sts_u16; /** 帧状态 */
    uint16_t len_u16; /** 帧字节数 */
} FRAME_HEAD;

struct rt_fmc_eth
{
    struct rt_fmc_eth_port *port;
};

struct rt_fmc_eth_port
{
    /* inherit from ethernet device */
    struct eth_device parent;

    /* interface address info, hw address */
    uint8_t dev_addr[MAX_ADDR_LEN];
    char *dev_name;
    struct rt_mutex eth_mux;    /** 接收发送互斥信号量 */

    /* 网卡通信与引脚定义 */
    volatile void *hw_addr;     /** 芯片数据端口地址 */
    volatile void *hw_addr_cmd; /** 芯片命令端口地址 */
    rt_base_t rst_pin;          /** 复位引脚 */
    rt_base_t isr_pin;          /** 中断引脚 */
    uint32_t NE;                /** NE 片选号 */

    rt_bool_t b_enabled; /** 芯片使能标志 */
    uint16_t txw[2];     /** 发送包时附加的4字节头信息 */

    uint16_t cmd_reg_cache;
    uint16_t cmd_reg_cache_int;
    uint16_t rc_ier;        /** 使能的中断控制字 */
    uint16_t sharedbus_u16; /** 数据、地址总线共享标志 */
    uint16_t bus_width_u16; /** 总线宽度 */
    uint16_t rc_rxqcr_u16;  /** 接收控制 */
    uint16_t all_mcast_u16;
    uint16_t mcast_lst_size_u16;
    uint16_t promiscuous_u16; /** 混杂模式 */
    uint8_t extra_byte_u8;    /** 额外数据字节数 */

    uint8_t mcast_lst[MAX_MCAST_LST][ETH_ALEN];
    uint8_t mcast_bits[HW_MCAST_SIZE];

    uint8_t frm_cnt_u8; /** QMU接收缓冲区中接收到的帧数 */
    FRAME_HEAD frame_head[KSZ_MAX_RFRM_THD];

#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
    rt_bool_t link_layer_enable;  /** 链路层使能标志 **/
    S_ETH_IF link_layer_buf;     /** 链路层缓冲区 **/
#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */
};

void fmc_eth_memcpy(void *DstAddress, void *SrcAddress, uint32_t DataLength);
void fmc_eth_inblk (void *DstAddress, volatile void *SrcAddress, uint32_t DataLength);
void fmc_eth_outblk(volatile void *DstAddress, void *SrcAddress, uint32_t DataLength);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATIONS_DRV_FMC_ETH_H_ */

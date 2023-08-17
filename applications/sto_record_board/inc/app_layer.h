/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-21     zm       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_INC_APP_LAYER_H_
#define APPLICATIONS_STO_RECORD_BOARD_INC_APP_LAYER_H_

#include <rtthread.h>

#include "data_handle.h"

/*应用层报文类型*/
//#define ROUND_CIRCLE_TYPE      3 /*轮循模式  请求方*/
//#define ROUND_CIRCLE_ACK_TYPE  5 /*轮循模式  应答方*/
#define ROUND_PULSE_MODE       6 /*周期模式  有应答模式*/

/*应用层帧子类型*/
//#define TIME_SET_LOCAL     3   /*轮循模式时 时钟同步，主控下发时钟通信插件同步 业务数据为4个字节 有应答*/
//#define IAP_PAKAGE         5   /*周期模式时 IAP数据包  业务数据为IAP数据包  无应答*/
//#define APPLY_TIME_SET     6   /*轮循模式时 向主控申请时钟同步  有应答*/
//#define SET_CHANNL_INFO    23  /*轮循模式时 设置通道配置信息，即为配置信息 通道数目+通道1参数+通道2参数+N参数 有应答 2字节OK或ERR*/
//#define GET_CHANNL_INFO    24  /*轮循模式时 查询通道配置信息，通道数目+通道号+通道号+N 有应答 通道数目+通道1参数+通道2参数+N参数*/
//#define SET_CONTIUE_INFO   27  /*轮循模式时 设置数据转发路由表，路由表数目1字节+路由表1(4字节)+N 有应答 2字节OK或ERR*/
//#define GET_CONTIUE_INFO   29  /*轮循模式时 查询数据转发路由表，2字节保留 有应答  路由表数目1字节+路由表1(4字节)+N*/
//#define COMM_PLUG_INFO     30  /*轮循模式时 主控获取插件信息，周期+平态工作状态 有应答*/
#define RX_MAINCTLDATA_EXP 33  /*周期模式时 收到主控数据发送到外部通道  通道数目+通道数据(通道编号1+时间戳4+长度2+数据N)+N  无应答*/
//#define RX_EXPDATA_MAINCTL 34  /*周期模式时 收到外部数据发送给主控     通道数目+通道数据(通道编号1+时间戳4+长度2+数据N)+N 无应答*/

/* 数据流通道 */

/* TX1插件CAN通道信息 */
/* TX1插件CAN通道号  */
#define DATA_CHANNEL_TX1CAN1 ( 0x10U )      /* 通信1的CAN1通道 */
#define DATA_CHANNEL_TX1CAN2 ( 0x11U )      /* 通信1的CAN2通道 */
#define DATA_CHANNEL_TX1CAN3 ( 0x12U )      /* 通信1的CAN3通道 */
#define DATA_CHANNEL_TX1CAN4 ( 0x13U )      /* 通信1的CAN4通道 */
#define DATA_CHANNEL_TX1CAN5 ( 0x14U )      /* 通信1的CAN5通道 */
//#define DATA_CHANNEL_TX1VMCAN1 ( 0x15U )     /* 通信1的VMCAN1通道 */
//#define DATA_CHANNEL_TX1VMCAN2 ( 0x16U )     /* 通信1的VMCAN2通道 */

/* TX2插件CAN通道号 */
#define DATA_CHANNEL_TX2CAN1 ( 0x17U )      /* 通信2的CAN1通道 */
#define DATA_CHANNEL_TX2CAN2 ( 0x18U )      /* 通信2的CAN2通道 */
#define DATA_CHANNEL_TX2CAN3 ( 0x19U )      /* 通信2的CAN3通道 */
#define DATA_CHANNEL_TX2CAN4 ( 0x1AU )      /* 通信2的CAN4通道 */
#define DATA_CHANNEL_TX2CAN5 ( 0x1BU )      /* 通信2的CAN5通道 */
//#define DATA_CHANNEL_TX2VMCAN1 ( 0x1CU )     /* 通信2的VMCAN1通道 */
//#define DATA_CHANNEL_TX2VMCAN2 ( 0x1DU )     /* 通信2的VMCAN2通道 */

/*********************应用层**************************/
/*应用层报文做为安全层的负载区传输*/
#pragma pack(1)
typedef struct eth_rx_app_layer
{
    uint8_t msg_type; /*报文类型*/
    uint8_t msg_sub_type; /*报文子类型*/
    uint16_t serial_num; /*报文序列号*/
} r_app_layer;

/*外部通道头数据格式*/
typedef struct exp_chanl_head {
        uint8_t  channl_index; /*通道索引号*/
        uint32_t time_print;   /*时间戳*/
        uint16_t data_len;     /*数据长度*/
  } h_exp_chanl;
#pragma pack()

rt_err_t app_layer_check(S_DATA_HANDLE * data_handle, uint8_t *pBuf, uint8_t *p_safe_layer);

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_APP_LAYER_H_ */

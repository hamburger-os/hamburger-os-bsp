/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-20     zm       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_INC_DATA_HANDLE_H_
#define APPLICATIONS_STO_RECORD_BOARD_INC_DATA_HANDLE_H_

#include <rtthread.h>
#include "can_common_def.h"

/* 定义通道编号 */
/*对系内通道*/
#define IN_ETH_DEV              (0x01U)              /* 内网 */

#define MAX_ETH_CAN_LEN         (1465U) /*应用层负载区数据最大为1476-exp_head(8)-pack_head(3)*/
#define CAN_DATA_MQ_MAX_NUM     (MAX_ETH_CAN_LEN + 100)

#define SET_RTC_CYCLE_MS        (600000UL)

extern S_CAN_PACKE_Grade s_packet_gradeInfo;

#pragma pack(push, 1)
/* 链路层通信的数据头 */
typedef struct _eth_frame
{
    uint8_t to_addr[6];
    uint8_t from_addr[6];
    uint16_t type;
} eth_frame_t;
#pragma pack(pop)

/******************************** 按字节对齐 ********************************/
#pragma pack (1)
/* CNA数据在ETH包中单帧格式 */
typedef struct
{
    uint32_t ID;
    uint8_t len;
    uint8_t Data[64];
} S_ETH_CAN_FRAME;

/* CAN业务层数据在ETH中格式 */
typedef struct
{
    uint32_t frameAllNum; /* 从上电开始到目前发送的总包数 */
    uint8_t frameNum; /* 总帧数 */
    uint16_t datalen;
    uint8_t data[MAX_ETH_CAN_LEN];
} S_APP_INETH_PACK;


/******************************** 按字节对齐 ********************************/

typedef struct
{
    /*******************************************
     **  11bit ID=优先级(8bit)+帧号(3bit)
     ********************************************/
    uint8_t priority_u8;        //优先级
    uint8_t no_u8;              //帧号
    uint8_t length_u8;          //长度
    uint8_t data_u8[64];         //数据
} CAN_FRAME;


typedef struct
{
    rt_mq_t can_data_mq;  /* 存放以太网转换为CAN格式的消息队列 */
} S_DATA_HANDLE;

#pragma pack ()

rt_err_t DataHandleInit(S_DATA_HANDLE *p_data_handle);
void ETHToCanDataHandle(S_DATA_HANDLE *p_data_handle, uint8_t *pbuf, uint16_t data_len);
rt_err_t CanDataHandle(S_DATA_HANDLE *p_data_handle);

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_DATA_HANDLE_H_ */

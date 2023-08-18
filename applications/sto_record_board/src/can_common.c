/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-17     zm       the first version
 */
#include "can_common_def.h"

#define DBG_TAG "cancomm"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <string.h>

/* 显示器拷贝程序 20201027 by dyp
 * can_buf_init - 接收CAN数据缓冲区初始化
 */
rt_err_t can_buf_init(S_CAN_PACKE_Grade *can_pkt_grade)
{
    if(NULL == can_pkt_grade)
    {
        return -RT_EEMPTY;
    }

    memset(can_pkt_grade, 0, sizeof(S_CAN_PACKE_Grade));

    can_pkt_grade->received_data_u8 = rt_malloc(sizeof(uint8_t) * MAX_LENGTH_PACKET);   /* 已接收完成的数据缓冲区 */
    if(NULL == can_pkt_grade->received_data_u8)
    {
        LOG_E("malloc size %d error", sizeof(uint8_t) * MAX_LENGTH_PACKET);
        return -RT_EEMPTY;
    }
    else
    {
        memset(can_pkt_grade->received_data_u8, 0, sizeof(uint8_t) * MAX_LENGTH_PACKET);
    }

    can_pkt_grade->receiving_data_u8 = rt_malloc(sizeof(uint8_t) * PACKET_LEN); /* 正在接收的数据缓冲区 */
    if(NULL == can_pkt_grade->receiving_data_u8)
    {
        LOG_E("malloc size %d error", sizeof(uint8_t) * PACKET_LEN);
        return -RT_EEMPTY;
    }
    else
    {
        memset(can_pkt_grade->receiving_data_u8, 0, sizeof(uint8_t) * PACKET_LEN);
    }

    can_pkt_grade->e_state = E_IDLE;                                  /* 接收状态 */
    can_pkt_grade->frame_id_u8 = 0;                                   /* 帧号 */
    can_pkt_grade->total_length_u16 = 0;                              /* 总长度 */
    can_pkt_grade->receiving_length_u16 = 0;                          /* 正在接收的数据长度 */
    can_pkt_grade->received_length_u16 = 0;                           /* 已接收完成的数据长度 */
    return RT_EOK;
}

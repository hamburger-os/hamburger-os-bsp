/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-01     zm       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_INC_POWER_MSG_H_
#define APPLICATIONS_STO_RECORD_BOARD_INC_POWER_MSG_H_

#include <rtthread.h>

#define POWER_MSG_CAN_DEV_NAME            "can1"
#define POWER_MSG_CAN_FRAME_NUM            (8)
#define POWER_MSG_CAN_DATA_NUM             (8)
//#define POWER_MSG_CAN_DATA_MQ_MAX_NUM     (10)

#define PWOER_MSG_DEV_CODE                 (0xA1)

typedef struct _S_POWER_CAN_FRAME S_POWER_CAN_FRAME;

extern S_POWER_CAN_FRAME *power_can_frame;

#define POWER_CAN_TX_MSG                 (0xF9U)
#define POWER_CAN_0xF9(N)                (power_can_frame[N])

/* 序号：上电初始为0，之后1-255循环 */
#define POWER_CAN_0_INDEX                (POWER_CAN_0xF9(0).data_u8[0])          /* 序号 1-255 */
#define POWER_CAN_0_24V_V                (POWER_CAN_0xF9(0).data_u8[1])          /* 24V电压 */
#define POWER_CAN_0_24V_A                (POWER_CAN_0xF9(0).data_u8[3])          /* 24V电流 */
#define POWER_CAN_0_CH_MSG               (POWER_CAN_0xF9(0).data_u8[5])          /* 通道信息 */
#define POWER_CAN_0_CRC                  (POWER_CAN_0xF9(0).data_u8[6])

#define POWER_CAN_1_INDEX                (POWER_CAN_0xF9(1).data_u8[0])          /* 序号 1-255 */
#define POWER_CAN_1_3V3_V                (POWER_CAN_0xF9(1).data_u8[1])          /* 3.3V电压 */
#define POWER_CAN_1_RESERVE              (POWER_CAN_0xF9(1).data_u8[3])          /* 保留 */
#define POWER_CAN_1_CRC                  (POWER_CAN_0xF9(1).data_u8[6])

#define POWER_CAN_2_INDEX                (POWER_CAN_0xF9(2).data_u8[0])          /* 序号 1-255 */
#define POWER_CAN_2_POWER_NUM            (POWER_CAN_0xF9(2).data_u8[1])          /* 上电次数 0-65535 */
#define POWER_CAN_2_TEMP                 (POWER_CAN_0xF9(2).data_u8[3])          /* 温度 */
#define POWER_CAN_2_POWER_CTL_STATE      (POWER_CAN_0xF9(2).data_u8[5])          /* 电源控制状态 open: 0x55, close: 0xaa */
#define POWER_CAN_2_CRC                  (POWER_CAN_0xF9(2).data_u8[6])

#define POWER_CAN_3_INDEX                (POWER_CAN_0xF9(3).data_u8[0])          /* 序号 1-255 */
#define POWER_CAN_3_POWER_TIME           (POWER_CAN_0xF9(3).data_u8[1])          /* 上电时间 0-0x3fffffffs */
#define POWER_CAN_3_RESERVE              (POWER_CAN_0xF9(3).data_u8[5])          /* 保留 */
#define POWER_CAN_3_CRC                  (POWER_CAN_0xF9(3).data_u8[6])

#define POWER_CAN_4_INDEX                (POWER_CAN_0xF9(4).data_u8[0])          /* 序号 1-255 */
#define POWER_CAN_4_DEVICE_ID            (POWER_CAN_0xF9(4).data_u8[1])          /* 设备ID A模：DYBAM B模：DYBBM */
#define POWER_CAN_4_CRC                  (POWER_CAN_0xF9(4).data_u8[6])

#define POWER_CAN_5_INDEX                (POWER_CAN_0xF9(5).data_u8[0])          /* 序号 1-255 */
#define POWER_CAN_5_SOFT_ID              (POWER_CAN_0xF9(5).data_u8[1])          /* 软件版本号 */
#define POWER_CAN_5_RESERVE              (POWER_CAN_0xF9(5).data_u8[5])          /* 保留 */
#define POWER_CAN_5_CRC                  (POWER_CAN_0xF9(5).data_u8[6])

#define POWER_CAN_6_INDEX                (POWER_CAN_0xF9(6).data_u8[0])          /* 序号 1-255 */
#define POWER_CAN_6_110V_V               (POWER_CAN_0xF9(6).data_u8[1])          /* 110V电压 */
#define POWER_CAN_6_110V_A               (POWER_CAN_0xF9(6).data_u8[3])          /* 110A电流 */
#define POWER_CAN_6_RESERVE              (POWER_CAN_0xF9(6).data_u8[5])          /* 保留 */
#define POWER_CAN_6_CRC                  (POWER_CAN_0xF9(6).data_u8[6])

/******************************** 按字节对齐 ********************************/
#pragma pack (1)

struct _S_POWER_CAN_FRAME
{
    /*******************************************
     **  11bit ID=优先级(8bit)+帧号(3bit)
     ********************************************/
    uint8_t priority_u8;        //优先级
    uint8_t no_u8;              //帧号
    uint8_t length_u8;          //长度
    uint8_t data_u8[POWER_MSG_CAN_DATA_NUM];         //数据
};

#pragma pack ()

typedef struct {
    struct rt_semaphore rx_sem;
    rt_device_t can_dev;
//    rt_mq_t data_mq;
    rt_mutex_t  mutex;
} S_POWER_MSG;

rt_err_t PowerMsgThreadInit(void);

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_POWER_MSG_H_ */

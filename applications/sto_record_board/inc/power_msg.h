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
#define POWER_MSG_CAN_FRAME_NUM            (7)
#define POWER_MSG_CAN_DATA_NUM             (8)

#define PWOER_MSG_DEV_CODE                 (0xA1)

typedef struct _S_POWER_CAN_FRAME S_POWER_CAN_FRAME;

extern S_POWER_CAN_FRAME *power_can_frame;


//POWER_MSG_CAN_FRAME_NUM 同步
#define POWER_MSG_CAN_0X7C1                  (0x7C1)        /* 电压电流 */
#define POWER_MSG_CAN_0x7C2                  (0x7C2)        /* 状态 */
#define POWER_MSG_CAN_0x7D1                  (0x7D1)        /* 历时 */
#define POWER_MSG_CAN_0x7D2                  (0x7D2)        /* 上电次数 */
#define POWER_MSG_CAN_0x7D5                  (0x7D5)        /* 温度 */
#define POWER_MSG_CAN_0x7D6                  (0x7D6)        /* 软件版本 */
#define POWER_MSG_CAN_0x7E1                  (0x7E1)        /* 输出控制 */

#define POWER_MSG_CAN_0X7C1_DATA                  (power_can_frame[0])
#define POWER_MSG_CAN_0X7C2_DATA                  (power_can_frame[1])
#define POWER_MSG_CAN_0X7D1_DATA                  (power_can_frame[2])
#define POWER_MSG_CAN_0X7D2_DATA                  (power_can_frame[3])
#define POWER_MSG_CAN_0X7D5_DATA                  (power_can_frame[4])
#define POWER_MSG_CAN_0X7D6_DATA                  (power_can_frame[5])
#define POWER_MSG_CAN_0X7E1_DATA                  (power_can_frame[6])

#define POWER_MSG_CAN_0X7C1_DATA_CH               (power_can_frame[0].data_u8[1])
#define POWER_MSG_CAN_0X7C1_DATA_V                (power_can_frame[0].data_u8[2])
#define POWER_MSG_CAN_0X7C1_DATA_A                (power_can_frame[0].data_u8[4])

#define POWER_MSG_CAN_0X7C2_DATA_STATE_TYEP       (power_can_frame[1].data_u8[1])
#define POWER_MSG_CAN_0X7C2_DATA_STATE_INFO       (power_can_frame[1].data_u8[2])

#define POWER_MSG_CAN_0X7D1_DATA_POWER_ON_TIME    (power_can_frame[2].data_u8[2])

#define POWER_MSG_CAN_0X7D2_DATA_POWER_ON_NUM     (power_can_frame[3].data_u8[2])

#define POWER_MSG_CAN_0X7D5_DATA_TEMP1            (power_can_frame[4].data_u8[2])
#define POWER_MSG_CAN_0X7D5_DATA_TEMP2            (power_can_frame[4].data_u8[4])

#define POWER_MSG_CAN_0X7D6_DATA_SOFT             (power_can_frame[5].data_u8[2])

#define POWER_MSG_CAN_0X7E1_DATA_CH               (power_can_frame[6].data_u8[1])
#define POWER_MSG_CAN_0X7E1_DATA_CTL              (power_can_frame[6].data_u8[2])

/******************************** 按字节对齐 ********************************/
#pragma pack (1)

struct _S_POWER_CAN_FRAME
{
    uint16_t priority_u8;        //帧号
    uint8_t length_u8;          //长度
    uint8_t data_u8[POWER_MSG_CAN_DATA_NUM];         //数据
};

#pragma pack ()

typedef struct {
    struct rt_semaphore rx_sem;
    rt_device_t can_dev;
    rt_mutex_t  mutex;
} S_POWER_MSG;

rt_err_t PowerMsgThreadInit(void);
void RecordingPowerPlugMessage(void);

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_POWER_MSG_H_ */

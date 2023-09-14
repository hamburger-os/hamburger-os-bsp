/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-01     zm       the first version
 */
#include "power_msg.h"
#include "board.h"

#define DBG_TAG "power msg"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <string.h>
#include <rtthread.h>

#include "crc.h"
#include "Record_FileCreate.h"

#define POWER_MSG_THREAD_PRIORITY         12
#define POWER_MSG_THREAD_STACK_SIZE       (1024 * 6)
#define POWER_MSG_THREAD_TIMESLICE        5

S_POWER_CAN_FRAME *power_can_frame;
static S_POWER_MSG power_msg;

/* 接收数据回调函数 */
static rt_err_t PowerMsgCanRxCall(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&power_msg.rx_sem);

    return RT_EOK;
}

static rt_err_t PowerMsgInit(S_POWER_MSG *msg)
{
    if(RT_NULL == msg)
    {
        return -RT_ERROR;
    }

    rt_memset(msg, 0, sizeof(S_POWER_MSG));

    /* 查找 CAN 设备 */
    msg->can_dev = rt_device_find(POWER_MSG_CAN_DEV_NAME);
    if (msg->can_dev != NULL)
    {
        /* 设置 CAN 通信的默认波特率 */
        if (rt_device_control(msg->can_dev, RT_CAN_CMD_SET_BAUD, (void *)CAN500kBaud) != RT_EOK)
        {
            LOG_E("set baud error!");
            return -RT_ERROR;
        }
        /* 设置 CAN 的工作模式为正常工作模式 */
        if (rt_device_control(msg->can_dev, RT_CAN_CMD_SET_MODE, (void *)RT_CAN_MODE_NORMAL) != RT_EOK)
        {
            LOG_E("set mode error!");
            return -RT_ERROR;
        }

        /* 以中断接收及发送方式打开 CAN 设备 */
        if (rt_device_open(msg->can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX) != RT_EOK)
        {
            LOG_E("open %s error", POWER_MSG_CAN_DEV_NAME);
            return -RT_ERROR;
        }

        /* 设置接收回调函数 */
        rt_device_set_rx_indicate(msg->can_dev, PowerMsgCanRxCall);

        /* 初始化 CAN 接收信号量 */
        rt_sem_init(&power_msg.rx_sem, "can_rx_sem", 0, RT_IPC_FLAG_FIFO);

        msg->mutex = rt_mutex_create("power mutex", RT_IPC_FLAG_PRIO);
        if(NULL == msg->mutex)
        {
            LOG_E("mutex creat error");
            return -RT_ERROR;
        }

        return RT_EOK;

    }
    else
    {
        LOG_E("find %s error", POWER_MSG_CAN_DEV_NAME);
        return -RT_EIO;
    }
}

static void RecordingPowerVoltageCurrentOneSelfCheckMessage(uint8_t record_num2, uint8_t ch, uint16_t v_new, uint16_t a_new, uint8_t *last_value)
{
    uint16_t v_old = 0;
    uint16_t a_old = 0;

    v_old =  ((uint16_t) (last_value[2]) << 8u) + (uint16_t)last_value[1];
    a_old =  ((uint16_t) (last_value[4]) << 8u) + (uint16_t)last_value[3];

    if((POWER_MSG_VOLTAGE_DIFF <= abs(v_new - v_old)) || (POWER_MSG_CURRENT_DIFF <= abs(a_new - a_old)))
    {
        last_value[0] = ch;
        last_value[1] = (uint8_t)v_new;
        last_value[2] = (uint8_t)(v_new >> 8);
        last_value[3] = (uint8_t)a_new;
        last_value[4] = (uint8_t)(a_new >> 8);

        WriteFileContantPkt(0xA6, record_num2, PWOER_MSG_DEV_CODE, last_value, 4u);
//        LOG_I("power ch %x, v %d, a %d", ch, v_new, a_new);
    }
}

static void RecordingPowerVoltageCurrentSelfCheckMessage(void)
{
    static uint8_t last_value_ch0[5] = {0x00,0x00,0x00,0x00,0x00};
    static uint8_t last_value_ch1[5] = {0x00,0x00,0x00,0x00,0x00};
    static uint8_t last_value_ch2[5] = {0x00,0x00,0x00,0x00,0x00};
    static uint8_t last_value_ch3[5] = {0x00,0x00,0x00,0x00,0x00};
    static uint8_t last_value_ch4[5] = {0x00,0x00,0x00,0x00,0x00};
    static uint8_t last_value_ch5[5] = {0x00,0x00,0x00,0x00,0x00};

    uint8_t ch = 0;
    uint16_t v_new = 0;
    uint16_t a_new = 0;

    rt_mutex_take(power_msg.mutex, RT_WAITING_FOREVER);

    ch = POWER_MSG_CAN_0X7C1_DATA_CH;
    v_new =  ((uint16_t) (*(&POWER_MSG_CAN_0X7C1_DATA_V + 1)) << 8u) + (uint16_t)POWER_MSG_CAN_0X7C1_DATA_V;
    a_new =  ((uint16_t) (*(&POWER_MSG_CAN_0X7C1_DATA_A + 1)) << 8u) + (uint16_t)POWER_MSG_CAN_0X7C1_DATA_A;

    rt_mutex_release(power_msg.mutex);

    switch(ch)
    {
    case POWER_MSG_VOLTAGE_CURRENT_CH1:
        RecordingPowerVoltageCurrentOneSelfCheckMessage(0x64, ch, v_new, a_new, last_value_ch0);
        break;
    case POWER_MSG_VOLTAGE_CURRENT_CH2:
        RecordingPowerVoltageCurrentOneSelfCheckMessage(0x65, ch, v_new, a_new, last_value_ch1);
        break;
    case POWER_MSG_VOLTAGE_CURRENT_CH3:
        RecordingPowerVoltageCurrentOneSelfCheckMessage(0x66, ch, v_new, a_new, last_value_ch2);
        break;
    case POWER_MSG_VOLTAGE_CURRENT_CH4:
        RecordingPowerVoltageCurrentOneSelfCheckMessage(0x67, ch, v_new, a_new, last_value_ch3);
        break;
    case POWER_MSG_VOLTAGE_CURRENT_CH5:
        RecordingPowerVoltageCurrentOneSelfCheckMessage(0x68, ch, v_new, a_new, last_value_ch4);
        break;
    case POWER_MSG_VOLTAGE_CURRENT_CH6:
        RecordingPowerVoltageCurrentOneSelfCheckMessage(0x69, ch, v_new, a_new, last_value_ch5);
        break;
    default:
        LOG_E("power ch error");
        break;
    }
}

static void RecordingPowerPowerOnCntSelfCheckMessage(void)
{
    static uint8_t last_value[2] = {0x00,0x00};
    uint16_t power_on_cnt_new = 0, power_on_cnt_old = 0;

    rt_mutex_take(power_msg.mutex, RT_WAITING_FOREVER);

    power_on_cnt_new = ((uint16_t) (*(&POWER_MSG_CAN_0X7D2_DATA_POWER_ON_NUM + 1)) << 8u) + (uint16_t)POWER_MSG_CAN_0X7D2_DATA_POWER_ON_NUM;

    rt_mutex_release(power_msg.mutex);

    power_on_cnt_old =  ((uint16_t) (last_value[1]) << 8u) + (uint16_t)last_value[0];

    if(power_on_cnt_old != power_on_cnt_new)
    {
        last_value[0] = (uint8_t)power_on_cnt_new;
        last_value[1] = (uint8_t)(power_on_cnt_new >> 8);

        WriteFileContantPkt(0xA6, 0x6a, PWOER_MSG_DEV_CODE, last_value, 4u);
//        LOG_I("power temp1 %d temp 2 %d", temp1_new, temp2_new);
    }
}

static void RecordingPowerTempSelfCheckMessage(void)
{
    static uint8_t last_value[4] = {0x00,0x00,0x00,0x00};
    uint16_t temp1_new = 0, temp1_old = 0;
    uint16_t temp2_new = 0, temp2_old = 0;

    rt_mutex_take(power_msg.mutex, RT_WAITING_FOREVER);

    temp1_new = ((uint16_t) (*(&POWER_MSG_CAN_0X7D5_DATA_TEMP1 + 1)) << 8u) + (uint16_t)POWER_MSG_CAN_0X7D5_DATA_TEMP1;
    temp2_new = ((uint16_t) (*(&POWER_MSG_CAN_0X7D5_DATA_TEMP2 + 1)) << 8u) + (uint16_t)POWER_MSG_CAN_0X7D5_DATA_TEMP2;

    rt_mutex_release(power_msg.mutex);

    temp1_old =  ((uint16_t) (last_value[1]) << 8u) + (uint16_t)last_value[0];
    temp2_old =  ((uint16_t) (last_value[3]) << 8u) + (uint16_t)last_value[2];

    if((POWER_MSG_TEMP_DIFF <= abs(temp1_new - temp1_old)) || (POWER_MSG_TEMP_DIFF <= abs(temp2_new - temp2_old)))
    {
        last_value[0] = (uint8_t)temp1_new;
        last_value[1] = (uint8_t)(temp1_new >> 8);
        last_value[2] = (uint8_t)temp2_new;
        last_value[3] = (uint8_t)(temp2_new >> 8);

        WriteFileContantPkt(0xA6, 0x6b, PWOER_MSG_DEV_CODE, last_value, 4u);
//        LOG_I("power temp1 %d temp 2 %d", temp1_new, temp2_new);
    }
}

static void RecordingPowerSoftInfoMessage(void)
{
    static uint8_t last_value[4] = {0x00,0x00,0x00,0x00};
    uint32_t soft_new = 0, soft_old = 0;

    rt_mutex_take(power_msg.mutex, RT_WAITING_FOREVER);

    soft_new = ((uint32_t) (*(&POWER_MSG_CAN_0X7D6_DATA_SOFT + 3)) << 24u) + ((uint32_t) (*(&POWER_MSG_CAN_0X7D6_DATA_SOFT + 2)) << 16u) +
                ((uint32_t) (*(&POWER_MSG_CAN_0X7D6_DATA_SOFT + 1)) << 8u) + (uint32_t)POWER_MSG_CAN_0X7D6_DATA_SOFT;

    rt_mutex_release(power_msg.mutex);

    soft_old = ((uint32_t) (last_value[3]) << 24u) + ((uint32_t) (last_value[2]) << 16u) +
            ((uint32_t) (last_value[1]) << 8u) + (uint32_t)last_value[0];

    if(soft_old != soft_new)
    {
        last_value[0] = (uint8_t)soft_new;
        last_value[1] = (uint8_t)(soft_new >> 8);
        last_value[2] = (uint8_t)(soft_new >> 16);
        last_value[3] = (uint8_t)(soft_new >> 24);

        WriteFileContantPkt(0xA5, 0x08, PWOER_MSG_DEV_CODE, last_value, 4u);
//        LOG_I("soft new %x, old %x", soft_new, soft_old);
    }
}

void RecordingPowerPlugMessage(void)
{
    RecordingPowerSoftInfoMessage();

    RecordingPowerVoltageCurrentSelfCheckMessage();
    RecordingPowerPowerOnCntSelfCheckMessage();
    RecordingPowerTempSelfCheckMessage();
}

static void PowerMsgCanDeal(struct rt_can_msg *can_msg)
{
    if(NULL == can_msg)
    {
        return;
    }

    rt_mutex_take(power_msg.mutex, RT_WAITING_FOREVER);
    switch(can_msg->id)
    {
    case POWER_MSG_CAN_0X7C1:
        rt_memcpy (POWER_MSG_CAN_0X7C1_DATA.data_u8, can_msg->data, can_msg->len);
        break;
    case POWER_MSG_CAN_0x7D2:
        rt_memcpy (POWER_MSG_CAN_0X7D2_DATA.data_u8, can_msg->data, can_msg->len);
        break;
    case POWER_MSG_CAN_0x7D5:
        rt_memcpy (POWER_MSG_CAN_0X7D5_DATA.data_u8, can_msg->data, can_msg->len);
        break;
    case POWER_MSG_CAN_0x7D6:
        rt_memcpy (POWER_MSG_CAN_0X7D6_DATA.data_u8, can_msg->data, can_msg->len);
        break;
    default:
        break;
    }
    rt_mutex_release(power_msg.mutex);
}

static void *PowerMsgThreadEntry(void *parameter)
{
    rt_err_t result = RT_EOK;
    struct rt_can_msg rxmsg = {0};
    uint16_t read_crc = 0, msg_crc = 0;

    /* 申请can帧缓冲区 */
    power_can_frame = rt_malloc((sizeof(S_POWER_CAN_FRAME) * POWER_MSG_CAN_FRAME_NUM));
    if(NULL == power_can_frame)
    {
        LOG_E("can frame malloc size %d error", (sizeof(S_POWER_CAN_FRAME) * POWER_MSG_CAN_FRAME_NUM));
    }

    rt_memset(power_can_frame, 0, sizeof((sizeof(S_POWER_CAN_FRAME) * POWER_MSG_CAN_FRAME_NUM)));

    if(PowerMsgInit(&power_msg) != RT_EOK)
    {
        LOG_E("power msg init error");
        return;
    }

    while(1)
    {
        result = rt_sem_take(&power_msg.rx_sem, 1000);
        /* 从 CAN 读取一帧数据 */
        if (rt_device_read(power_msg.can_dev, 0, &rxmsg, sizeof(rxmsg)) == sizeof(rxmsg))
        {
            /* crc校验 */
            msg_crc = Crc16TabCCITT(rxmsg.data, (POWER_MSG_CAN_DATA_NUM - 2));
            read_crc = ((uint16_t) (rxmsg.data[7]) << 8u) + (uint16_t)rxmsg.data[6];
            if(msg_crc == read_crc)
            {
                PowerMsgCanDeal(&rxmsg);
            }
            else
            {
                LOG_E("%d crc error", rxmsg.id);
            }
        }
        rt_thread_mdelay(10);
    }
}

rt_err_t PowerMsgThreadInit(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("power msg",
                            PowerMsgThreadEntry, RT_NULL,
                            POWER_MSG_THREAD_STACK_SIZE,
                            POWER_MSG_THREAD_PRIORITY, POWER_MSG_THREAD_TIMESLICE);

    if(tid != NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    return -RT_ERROR;
}


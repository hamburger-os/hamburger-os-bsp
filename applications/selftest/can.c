/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-18     lvhan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "selftest.h"

#define DBG_TAG "can "
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static char * error_log[] = {
    "CAN1      ----> CAN2     ",
    "CAN2      ----> CAN1     ",
};

void selftest_can_test(SelftestlUserData *puserdata)
{
    struct rt_can_msg txmsg = {0};
    txmsg.id = 0x78;              /* ID 为 0x78 */
    txmsg.ide = RT_CAN_STDID;     /* 标准格式 */
    txmsg.rtr = RT_CAN_DTR;       /* 数据帧 */
    txmsg.len = 8;                /* 数据长度为 8 */
    /* 待发送的 8 字节数据 */
    txmsg.data[0] = 0x00;
    txmsg.data[1] = 0x11;
    txmsg.data[2] = 0x22;
    txmsg.data[3] = 0x33;
    txmsg.data[4] = 0x44;
    txmsg.data[5] = 0x55;
    txmsg.data[6] = 0x66;
    txmsg.data[7] = 0x77;
    struct rt_can_msg rxmsg = {0};

    for (int i = 0; i<2; i++)
    {
        /* step1：查找串口设备 */
        puserdata->can_dev[i][0] = rt_device_find(puserdata->can_devname[i][0]);
        puserdata->can_dev[i][1] = rt_device_find(puserdata->can_devname[i][1]);

        /* 设置 CAN 通信的默认波特率 */
        if (rt_device_control(puserdata->can_dev[i][0], RT_CAN_CMD_SET_BAUD, (void *)CAN500kBaud) != RT_EOK)
        {
            LOG_E("set '%s' baud error!", puserdata->can_devname[i][0]);
        }
        /* 设置 CAN 的工作模式为正常工作模式 */
        if (rt_device_control(puserdata->can_dev[i][0], RT_CAN_CMD_SET_MODE, (void *)RT_CAN_MODE_NORMAL) != RT_EOK)
        {
            LOG_E("set '%s' mode error!", puserdata->can_devname[i][0]);
        }
        /* 以中断接收及发送方式打开 CAN 设备 */
        if (rt_device_open(puserdata->can_dev[i][0], RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX) != RT_EOK)
        {
            LOG_E("open '%s' error!", puserdata->can_devname[i][0]);
        }
    }

    for (int i = 0; i<2; i++)
    {
        rt_thread_delay(10);
        rt_device_write(puserdata->can_dev[i][0], 0, &txmsg, sizeof(txmsg));
        rt_thread_delay(100);

        rt_memset(&rxmsg, 0, sizeof(rxmsg));
        rxmsg.hdr = -1;
        if (rt_device_read(puserdata->can_dev[i][1], 0, &rxmsg, sizeof(rxmsg)) != sizeof(rxmsg))
        {
            LOG_E("%s read failed", error_log[i]);
        }
        if (rt_memcmp(txmsg.data, rxmsg.data, txmsg.len) == 0)
        {
            LOG_D("%s pass", error_log[i]);
        }
        else
        {
            LOG_E("%s error!", error_log[i]);
            LOG_HEX("rd", 16, rxmsg.data, txmsg.len);
        }
    }

    for (int i = 0; i<2; i++)
    {
        rt_device_close(puserdata->can_dev[i][0]);
    }
}

/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-26     zm       the first version
 */

#include "record_ota.h"

#define DBG_TAG "RecordOTA"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#include "yw_manage.h"
#include "yw_packet.h"
#include "yw_config.h"
#include "yw_interf.h"
#include "yw_udp_interf.h"
#include "udp_comm.h"

#define ENABLE_XING_SHI_SHI_YAN 0

typedef struct {
    yw_ota_t ota;
    RecordOTAMode current_mode;
    struct rt_mutex mux;
} RecordOTA;

static RecordOTA record_ota;


void RecordOTAInit(void)
{
    memset(&record_ota, 0, sizeof(record_ota));

    record_ota.current_mode = RecordOTAModeNormal;

    rt_mutex_init(&record_ota.mux, "ota mutex", RT_IPC_FLAG_PRIO);
}

RecordOTAMode RecordOTAGetMode(void)
{
    RecordOTAMode mode = 0;

    rt_mutex_take(&record_ota.mux, RT_WAITING_FOREVER);

    mode = record_ota.current_mode;

    rt_mutex_release(&record_ota.mux);

    return mode;
}

void RecordOTASetMode(RecordOTAMode mode)
{
    rt_mutex_take(&record_ota.mux, RT_WAITING_FOREVER);

    record_ota.current_mode = mode;

    rt_mutex_release(&record_ota.mux);
}

void RecordOTAThread(void *parameter)
{
    sint32_t ret = 0;
    RecordOTAMode current_ota_mode = RecordOTAModeNormal;

    print("***************************\r\n");
    print("* hello, ota !\r\n");
    print("***************************\r\n");

    dev_eth0 = rt_device_find("e0");
    if (dev_eth0 == RT_NULL)
    {
        LOG_E("e0 find NULL.");
        while(1);
    }

    if(rt_device_open(dev_eth0, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("e0 open fail.");
        while(1);
    }
    LOG_I("e0 open successful");
    dev_eth1 = rt_device_find("e1");
    if (dev_eth1 == RT_NULL)
    {
        LOG_E("e1 find NULL.");
        while(1);
    }

    if(rt_device_open(dev_eth1, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("e1 open fail.");
        while(1);
    }
    LOG_I("e1 open successful");
    ret = yw_process_init(&record_ota.ota);
    if (ret < 0)
    {
        print("yw_process_init error, ret=%d. \r\n", ret);
    }
    while (1)
    {
        current_ota_mode = RecordOTAGetMode();
        UDPServerRcvIPData((char *)record_ota.ota.connect_info.ip_str, sizeof(record_ota.ota.connect_info.ip_str), &record_ota.ota.connect_info.port);

        if(RecordOTAModeUpdata == current_ota_mode || RecordOTAModeUpdataAgain == current_ota_mode)
        {
#if ENABLE_XING_SHI_SHI_YAN
            usleep(1000);
#else
            if(RecordOTAModeUpdataAgain == current_ota_mode)
            {
                print("update again disconnect \r\n");
                record_ota.ota.disconnect();
                RecordOTASetMode(RecordOTAModeUpdata);
                current_ota_mode = RecordOTAGetMode();

                print("current mode %d\r\n",  current_ota_mode);
            }
            ret = yw_process(&record_ota.ota);
            if (ret < 0) /* 处于空闲状态, 可以休眠 */
            {
                usleep(2 * 1000);
            }
#endif
        }
        else
        {
            if(record_ota.ota.get_connect_state() != CON_STAT_DISCONNECT)
            {
                print("record_ota.ota.disconnect \r\n");
                record_ota.ota.disconnect();
            }
            rt_thread_mdelay(2 * 1000);
        }
    }
    ret = yw_process_exit(&record_ota.ota);
    if (ret < 0)
    {
        print("yw_process_exit error, ret=%d. \r\n", ret);
    }
}

int RecordOTAInitThreadInit(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("ota",
                            RecordOTAThread, RT_NULL,
                            (1024 * 50),//25
                            23, 5);//15, 5);

    if(tid != NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    return -RT_ERROR;
}


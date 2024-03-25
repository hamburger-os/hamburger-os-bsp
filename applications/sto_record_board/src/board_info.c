/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     zm       the first version
 */
#include "board_info.h"

#define DBG_TAG "BoardInfo"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#include "Common.h"
#include "eth_thread.h"
#include "send_data_by_eth.h"
#include "app_layer.h"
#include "safe_layer.h"

#define BOARD_INFO_THREAD_PRIORITY         23
#define BOARD_INFO_THREAD_STACK_SIZE       1024 * 10
#define BOARD_INFO_THREAD_TIMESLICE        5

#define BOARD_INFO_READ          1
#define BOARD_INFO_READ_PERIOD   (60 * 1000 * 20U) //20 minute

struct __attribute__((packed)) DS1682DataDef
{
    rt_uint32_t time_min;
    rt_uint16_t count;
};

typedef struct {
    rt_device_t ds1682dev;
    struct DS1682DataDef ds1682_data;

    rt_device_t ltc2991dev;
    rt_uint16_t ltc2991data[4];

    rt_device_t max31826dev;
    rt_uint8_t max_ID[8];
    struct rt_sensor_data max31826data;

    uint32_t send_board_version_time;
    uint32_t apply_time_ajust_5s_cmpTime;
    uint32_t board_info_time;

    uint32_t system_run_cnt;

} BoardInfo;

static BoardInfo board_info;

rt_err_t BoardInfoInit(void)
{
    memset(&board_info, 0, sizeof(BoardInfo));

    /* 查找历时芯片设备 */
    board_info.ds1682dev = rt_device_find("ds1682");
    if (RT_NULL == board_info.ds1682dev)
    {
        LOG_E("ds1682dev find NULL.");
        return -RT_EIO;
    }

    /* 查找测5V电压电流设备 */
    board_info.ltc2991dev = rt_device_find("ltc2991");
    if (RT_NULL == board_info.ltc2991dev)
    {
        LOG_E("ltc2991dev find NULL.");
        return -RT_EIO;
    }

    /* 查找单总线MAX31826设备 */
    board_info.max31826dev = rt_device_find("temp_max31826_1");
    if (board_info.max31826dev == RT_NULL)
    {
        LOG_E("max31826dev find NULL.");
        return -RT_EIO;
    }

    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(board_info.ds1682dev, RT_DEVICE_FLAG_RDONLY);

    rt_device_open(board_info.ltc2991dev, RT_DEVICE_FLAG_RDONLY);

    rt_device_open(board_info.max31826dev, RT_DEVICE_FLAG_RDONLY);
#if 1
    rt_thread_mdelay(2000);
    if (rt_device_control(board_info.max31826dev, RT_SENSOR_CTRL_GET_ID, board_info.max_ID) == RT_EOK)
    {
        LOG_D("max31826 id: %x %x %x %x %x %x %x %x", board_info.max_ID[0], board_info.max_ID[1], board_info.max_ID[2], board_info.max_ID[3],
                board_info.max_ID[4], board_info.max_ID[5], board_info.max_ID[6], board_info.max_ID[7]);
        return RT_EOK;
    }
    else
    {
        LOG_E("max31826 read id error");
        return -RT_ERROR;
    }
#else
    return RT_EOK;
#endif
}

void get_comm_plug_info(uint8_t *pSafe , r_app_layer *pApp_layer)
{

}

void BoardInfoRead(void)
{
    board_info.system_run_cnt++;
    LOG_I("sys run %d", board_info.system_run_cnt);
    if (rt_device_read(board_info.max31826dev, 0, &board_info.max31826data, sizeof(board_info.max31826data)) == 1)   //扩大了100
    {
        LOG_I("temp: %d", board_info.max31826data.data.temp);
    }
    /* 读取历时芯片设备 */
    if (rt_device_read(board_info.ds1682dev, 0, &board_info.ds1682_data, sizeof(board_info.ds1682_data)) == 0)
    {
        board_info.ds1682_data.time_min = board_info.ds1682_data.time_min / 60;
        LOG_I("time: %d min count: %d", board_info.ds1682_data.time_min, board_info.ds1682_data.count);
    }
    /* 读取ltc2991设备 */
    if (rt_device_read(board_info.ltc2991dev, 4, &board_info.ltc2991data, sizeof(board_info.ltc2991data)) == 0)   //扩大了1000
    {
        board_info.ltc2991data[0] = board_info.ltc2991data[0] << 1;    // * 2
        board_info.ltc2991data[1] = board_info.ltc2991data[1] << 1;    // * 2
        board_info.ltc2991data[2] = board_info.ltc2991data[2] << 1;    // * 2
        board_info.ltc2991data[3] = board_info.ltc2991data[3] >> 1;    // / 2

        /* v v v a */
        LOG_I("va %d %d %d %d",
                board_info.ltc2991data[0], board_info.ltc2991data[1],
                board_info.ltc2991data[2], board_info.ltc2991data[3]);
    }
}

static void BoardInfoThreadEntry(void *parameter)
{
    uint8_t sndBuf[8U] = { 0U };
    uint16_t error_flag = 0;
    BoardInfo *p_board_info = &board_info;

    p_board_info->apply_time_ajust_5s_cmpTime = rt_tick_get();
    p_board_info->send_board_version_time = rt_tick_get();
    p_board_info->board_info_time = rt_tick_get();

    while (1)
    {
        Check_exp_2_mainctl_SendFlag();
        if (Common_BeTimeOutMN(&p_board_info->send_board_version_time, 1000u))
        {
            p_board_info->send_board_version_time = rt_tick_get();
            memset(&sndBuf[0U], 0U, 8U );

            memcpy( &sndBuf[0U], &error_flag, 2u );
            memcpy( &sndBuf[2U], Version, 4u );
            memcpy( &sndBuf[6], &Verdate, 2u );

            linklayer_sendFrame(DATA_CHANNEL_TX2ZK, 0xa0, 0, &sndBuf[0u], 8);

        }
        if (Common_BeTimeOutMN(&p_board_info->apply_time_ajust_5s_cmpTime, 5000u))
        {
            p_board_info->apply_time_ajust_5s_cmpTime = rt_tick_get();
            if(InETH0_diff_flag == 0)
            {
                ApplyDiffToSto(ETH_CH_INEX_1);
            }
            if(InETH1_diff_flag == 0)
            {
                ApplyDiffToSto(ETH_CH_INEX_2);
            }
        }
#if BOARD_INFO_READ
        if(Common_BeTimeOutMN(&p_board_info->board_info_time, BOARD_INFO_READ_PERIOD))
        {
            BoardInfoRead();
            p_board_info->board_info_time = rt_tick_get();
        }
#endif /* BOARD_INFO_READ */
        rt_thread_mdelay(1);
    }
}

int BoardInfoThreadInit(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("board info",
            BoardInfoThreadEntry, RT_NULL,
            BOARD_INFO_THREAD_STACK_SIZE,
            BOARD_INFO_THREAD_PRIORITY, BOARD_INFO_THREAD_TIMESLICE);

    if(tid != NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    return -RT_ERROR;
}


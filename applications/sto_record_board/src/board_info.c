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

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#define DBG_TAG "BoardInfo"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define BOARD_INFO_THREAD_PRIORITY         20
#define BOARD_INFO_THREAD_STACK_SIZE       1024
#define BOARD_INFO_THREAD_TIMESLICE        5

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

} BoardInfo;

static BoardInfo board_info;

static void *BoardInfoThreadEntry(void *parameter)
{
    memset(&board_info, 0, sizeof(BoardInfo));

    /* 查找历时芯片设备 */
    board_info.ds1682dev = rt_device_find("ds1682");
    if (RT_NULL == board_info.ds1682dev)
    {
        LOG_E("ds1682dev find NULL.");
    }

    /* 查找测5V电压电流设备 */
    board_info.ltc2991dev = rt_device_find("ltc2991");
    if (RT_NULL == board_info.ltc2991dev)
    {
        LOG_E("ltc2991dev find NULL.");
    }

    /* 查找单总线MAX31826设备 */
    board_info.max31826dev = rt_device_find("temp_max31826");
    if (board_info.max31826dev == RT_NULL)
    {
        LOG_E("max31826dev find NULL.");
    }

    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(board_info.ds1682dev, RT_DEVICE_FLAG_RDONLY);

    rt_device_open(board_info.ltc2991dev, RT_DEVICE_FLAG_RDONLY);

    rt_device_open(board_info.max31826dev, RT_DEVICE_FLAG_RDONLY);
    rt_thread_mdelay(2000);
    if (rt_device_control(board_info.max31826dev, RT_SENSOR_CTRL_GET_ID, board_info.max_ID) == 0)
    {
        LOG_D("id   : %x %x %x %x %x %x %x %x", board_info.max_ID[0], board_info.max_ID[1], board_info.max_ID[2], board_info.max_ID[3],
                board_info.max_ID[4], board_info.max_ID[5], board_info.max_ID[6], board_info.max_ID[7]);
    }
    while (1)
    {
        rt_thread_mdelay(2000);
        if (rt_device_read(board_info.max31826dev, 0, &board_info.max31826data, sizeof(board_info.max31826data)) == 1)   //扩大了100
        {
            LOG_D("temp: %d ℃", board_info.max31826data.data.temp);
        }
        rt_thread_mdelay(2000);
        /* 读取历时芯片设备 */
        if (rt_device_read(board_info.ds1682dev, 0, &board_info.ds1682_data, sizeof(board_info.ds1682_data)) == 0)
        {
            board_info.ds1682_data.time_min = board_info.ds1682_data.time_min / 60;
            LOG_D("time : %d min, count : %d", board_info.ds1682_data.time_min, board_info.ds1682_data.count);
        }
        rt_thread_mdelay(2000);
        /* 读取ltc2991设备 */
        if (rt_device_read(board_info.ltc2991dev, 4, &board_info.ltc2991data, sizeof(board_info.ltc2991data)) == 0)   //扩大了1000
        {
            board_info.ltc2991data[0] = board_info.ltc2991data[0] << 1;    // * 2
            board_info.ltc2991data[1] = board_info.ltc2991data[1] << 1;    // * 2
            board_info.ltc2991data[2] = board_info.ltc2991data[2] << 1;    // * 2
            board_info.ltc2991data[3] = board_info.ltc2991data[3] >> 1;    // / 2

            LOG_D("ch1 voltage = %d V", board_info.ltc2991data[0]);
            LOG_D("ch2 voltage = %d V", board_info.ltc2991data[1]);
            LOG_D("ch3 voltage = %d V", board_info.ltc2991data[2]);
            LOG_D("ch4 current = %d A", board_info.ltc2991data[3]);
        }
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


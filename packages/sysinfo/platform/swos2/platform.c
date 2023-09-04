/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-01     lvhan       the first version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"

int8_t get_sys_version(uint8_t *version)
{
    strncpy((char *)version, SYS_VERSION, 22);

    return 0;
}

int8_t get_cpu_temp(int32_t *temp)
{
    rt_device_t dev = NULL;
    struct rt_sensor_data sensor_data = {0};

    /* 查找 cpu temp 设备 */
    dev = rt_device_find(SYSINFO_CPU_TEMP_NAME);
    if (dev == RT_NULL)
    {
        return -1;
    }
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(dev, RT_DEVICE_FLAG_RDONLY);
    if (rt_device_read(dev, 0, &sensor_data, sizeof(sensor_data)) == 1)
    {
        *temp = sensor_data.data.temp;
    }
    else
    {
        return -1;
    }

    return 0;
}

int8_t get_chip_id_temp(uint8_t *id, int32_t *temp)
{
    rt_device_t dev = NULL;
    struct rt_sensor_data sensor_data = {0};

    /* 查找单总线MAX31826设备 */
    dev = rt_device_find(SYSINFO_CHIP_TEMP_NAME);
    if (dev == RT_NULL)
    {
        return -1;
    }
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(dev, RT_DEVICE_FLAG_RDONLY);
    if (rt_device_control(dev, RT_SENSOR_CTRL_GET_ID, id) != 0)
    {
        return -1;
    }
    if (rt_device_read(dev, 0, &sensor_data, sizeof(sensor_data)) == 1)
    {
        *temp = sensor_data.data.temp;
    }
    else
    {
        return -1;
    }

    return 0;
}

struct __attribute__((packed)) DS1682DataDef
{
    uint32_t times;
    uint16_t count;
};
int8_t get_times_count(uint32_t *times, uint16_t *count)
{
    rt_device_t dev = NULL;
    struct DS1682DataDef ds1682data = {0};

    /* 查找历时芯片设备 */
    dev = rt_device_find(SYSINFO_TIMES_DEV_NAME);
    if (dev == RT_NULL)
    {
        return -1;
    }
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(dev, RT_DEVICE_FLAG_RDONLY);
    if (rt_device_read(dev, 0, &ds1682data, sizeof(ds1682data)) == 0)
    {
        *times = ds1682data.times;
        *count = ds1682data.count;
    }
    else
    {
        return -1;
    }

    return 0;
}

int16_t sysinfofix_blk_read(uint8_t *buf, size_t size)
{
    struct fal_partition * part = (struct fal_partition *)fal_partition_find(SYSINFO_PARTNAME);
    if (part == NULL)
    {
        pLOG_E("part find error!");
        return -1;
    }
    if (fal_partition_read(part, 0, buf, size) != size)
    {
        pLOG_E("part read error!");
        return -1;
    }

    return size;
}

int16_t sysinfofix_blk_write(const uint8_t *buf, size_t size)
{
    struct fal_partition * part = (struct fal_partition *)fal_partition_find(SYSINFO_PARTNAME);
    if (part == NULL)
    {
        pLOG_E("part find error!");
        return -1;
    }
    if (fal_partition_write(part, 0, buf, size) != size)
    {
        pLOG_E("part write error!");
        return -1;
    }

    return size;
}

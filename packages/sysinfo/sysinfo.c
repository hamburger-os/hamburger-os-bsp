/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-12     lvhan       the first version
 */
#include "board.h"

#include "sysinfo.h"

#define DBG_TAG "sysinfo"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

struct __attribute__((packed)) DS1682DataDef
{
    uint32_t times;
    uint16_t count;
};

void sysinfo_get(struct SysInfoDef *info)
{
    rt_device_t dev = NULL;
    struct rt_sensor_data sensor_data = {0};

    info->cpu_id[0] = HAL_GetUIDw0();
    info->cpu_id[1] = HAL_GetUIDw1();
    info->cpu_id[2] = HAL_GetUIDw2();

    /* 查找 cpu temp 设备 */
    dev = rt_device_find("temp_cpu");
    if (dev == RT_NULL)
    {
        LOG_E("cpu temp not find!");
    }
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(dev, RT_DEVICE_FLAG_RDONLY);
    if (rt_device_read(dev, 0, &sensor_data, sizeof(sensor_data)) == 1)
    {
        info->cpu_temp = sensor_data.data.temp / 100.0f;
    }
    else
    {
        LOG_E("sensor read error!");
    }

    /* 查找单总线MAX31826设备 */
    dev = rt_device_find("temp_max31826");
    if (dev == RT_NULL)
    {
        LOG_E("max31826 not find!");
    }
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(dev, RT_DEVICE_FLAG_RDONLY);
    rt_memset((void *)info->chip_id, 0, sizeof(info->chip_id));
    if (rt_device_control(dev, RT_SENSOR_CTRL_GET_ID, info->chip_id) != 0)
    {
        LOG_E("max31826 id error!");
    }
    if (rt_device_read(dev, 0, &sensor_data, sizeof(sensor_data)) == 1)
    {
        info->chip_temp = sensor_data.data.temp / 100.0f;
    }
    else
    {
        LOG_E("max31826 read error!");
    }

    /* 查找历时芯片设备 */
    dev = rt_device_find("ds1682");
    if (dev == RT_NULL)
    {
        LOG_E("ds1682 not find!");
    }
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(dev, RT_DEVICE_FLAG_RDONLY);
    struct DS1682DataDef ds1682data = {0};
    if (rt_device_read(dev, 0, &ds1682data, sizeof(ds1682data)) == 0)
    {
        info->times = ds1682data.times;
        info->count = ds1682data.count;
    }
    else
    {
        LOG_E("ds1682 read error!");
    }
}

void sysinfo_show(void)
{
    struct SysInfoDef info = {0};
    sysinfo_get(&info);

    LOG_D("- systerm info:");
    LOG_D("----------------------------------------------------------------");
    LOG_D("- version     : %d", info.version);
    LOG_HEX("     - SN          ", 8, info.SN, sizeof(info.SN));
    LOG_D("----------------------------------------------------------------");
    LOG_D("- cpu id      : 0x%08X %08X %08X", info.cpu_id[0], info.cpu_id[1], info.cpu_id[2]);
    LOG_D("- cpu temp    : %d.%02d (℃) ", (int32_t)info.cpu_temp, abs((int32_t)((info.cpu_temp - (int32_t)info.cpu_temp) * 100)));
    LOG_D("----------------------------------------------------------------");
    LOG_D("- chip id     : 0x%02X %02X %02X %02X %02X %02X %02X %02X", info.chip_id[0], info.chip_id[1], info.chip_id[2], info.chip_id[3], info.chip_id[4], info.chip_id[5], info.chip_id[6], info.chip_id[7]);
    LOG_D("- chip temp   : %d.%02d (℃) ", (int32_t)info.chip_temp, abs((int32_t)((info.chip_temp - (int32_t)info.chip_temp) * 100)));
    LOG_D("- times       : %d s", info.times);
    LOG_D("- count       : %d", info.count);
    LOG_D("----------------------------------------------------------------");
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(sysinfo_show, sysinfo, Show system info.);
#endif

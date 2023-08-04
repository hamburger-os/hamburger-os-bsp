/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-04     lvhan       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include "coupler_controller.h"

#define DBG_TAG "gnss"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static void gnss_thread_entry(void *parameter)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)parameter;
    /* 查找gnss设备 */
    puserdata->gnss_dev = rt_device_find(puserdata->gnss_devname);
    /* 打开gnss设备 */
    rt_device_open(puserdata->gnss_dev, RT_DEVICE_FLAG_RDONLY);

    LOG_I("gnss thread startup...");
    while(puserdata->isThreadRun)
    {
        rt_thread_delay(1000);
        rt_device_read(puserdata->gnss_dev, 0, &puserdata->hgps, sizeof(lwgps_t));
        LOG_D("[%d %d] %d/%d/%d %d:%d:%d %d.%06d %d.%06d %d.%03d %d.%03d %d.%03d"
            , puserdata->hgps.is_valid, puserdata->hgps.sats_in_use
            , puserdata->hgps.year, puserdata->hgps.month, puserdata->hgps.date
            , puserdata->hgps.hours, puserdata->hgps.minutes, puserdata->hgps.seconds
            , (int)puserdata->hgps.latitude, (int)((puserdata->hgps.latitude - (int)puserdata->hgps.latitude)*1000000)
            , (int)puserdata->hgps.longitude, (int)((puserdata->hgps.longitude - (int)puserdata->hgps.longitude)*1000000)
            , (int)puserdata->hgps.altitude, (int)((puserdata->hgps.altitude - (int)puserdata->hgps.altitude)*1000)
            , (int)puserdata->hgps.speed, (int)((puserdata->hgps.speed - (int)puserdata->hgps.speed)*1000)
            , (int)puserdata->hgps.variation, (int)((puserdata->hgps.variation - (int)puserdata->hgps.variation)*1000));
    }
}

void coupler_controller_gnssinit(void)
{
    /* 创建 gnss 线程 */
    rt_thread_t thread = rt_thread_create("gnss_ctrl", gnss_thread_entry, &coupler_controller_userdata, 2048, 22, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E("thread startup error!");
    }
}

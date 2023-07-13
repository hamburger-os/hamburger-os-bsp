/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-26     lvhan       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include "coupler_controller.h"

#define DBG_TAG "ctrl"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

enum {
    CTRL_DO1 = 0,
    CTRL_DO2,
    CTRL_DI1,
    CTRL_DI2,
};

static void ctrl_thread_entry(void *parameter)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)parameter;

    puserdata->ctrl_pin[CTRL_DO1] = rt_pin_get(puserdata->ctrl_devname[CTRL_DO1]);
    rt_pin_mode(puserdata->ctrl_pin[CTRL_DO1], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->ctrl_pin[CTRL_DO1], PIN_LOW);

    puserdata->ctrl_pin[CTRL_DO2] = rt_pin_get(puserdata->ctrl_devname[CTRL_DO2]);
    rt_pin_mode(puserdata->ctrl_pin[CTRL_DO2], PIN_MODE_OUTPUT);
    rt_pin_write(puserdata->ctrl_pin[CTRL_DO2], PIN_LOW);

    puserdata->ctrl_pin[CTRL_DI1] = rt_pin_get(puserdata->ctrl_devname[CTRL_DI1]);
    rt_pin_mode(puserdata->ctrl_pin[CTRL_DI1], PIN_MODE_INPUT);

    puserdata->ctrl_pin[CTRL_DI2] = rt_pin_get(puserdata->ctrl_devname[CTRL_DI2]);
    rt_pin_mode(puserdata->ctrl_pin[CTRL_DI2], PIN_MODE_INPUT);

    while(puserdata->isThreadRun)
    {
        rt_thread_delay(1000);
    }
}

void ctrl_air_pressure(uint8_t onoff)
{
    rt_pin_write(coupler_controller_userdata.ctrl_pin[CTRL_DO1], onoff);
    LOG_D("air pressure %d", onoff);
}
static void air_test(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("Usage: airtest [onoff]\n");
        rt_kprintf("       example : airtest 1\n");
    }
    else
    {
        uint8_t onoff = strtoul(argv[1], NULL, 10);
        ctrl_air_pressure(onoff);
    }
}
MSH_CMD_EXPORT_ALIAS(air_test, airtest, air_pressure test);

void coupler_controller_ctrlinit(void)
{
    /* 创建 ctrl 线程 */
    rt_thread_t thread = rt_thread_create("ctrl", ctrl_thread_entry, &coupler_controller_userdata, 2048, 26, 10);
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

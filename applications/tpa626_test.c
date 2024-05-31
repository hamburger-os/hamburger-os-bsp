#include <drv_tpa626.h>
/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-25     Administrator       the first version
 */
#include "board.h"
#include <rtthread.h>

#define DBG_TAG "tpa626"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

rt_device_t tpa626dev;

static void tpa626_test(int argc, char *argv[])
{
    if (argc != 1)
    {
        rt_kprintf("Usage: tpa626_test\n");
        return;
    }

    uint16_t cur_vol_val;

    tpa626dev = rt_device_find("tpa626");
    if (tpa626dev == RT_NULL)
    {
        LOG_E("tpa626 find NULL.");
    }
    LOG_D("tpa626 test start...");
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(tpa626dev, RT_DEVICE_FLAG_RDONLY);

    if (rt_device_read(tpa626dev, TPA626_CURRENT_CHANL, &cur_vol_val, sizeof(cur_vol_val)) != 0)
    {
        LOG_D("tpa626 sample vol is %d mA", cur_vol_val);
    }

    if (rt_device_read(tpa626dev, TPA626_BUS_VOL_CHANL, &cur_vol_val, sizeof(cur_vol_val)) != 0)
    {
        LOG_D("tpa626 sample vol is %d mV", cur_vol_val);
    }
    rt_device_close(tpa626dev);
}
MSH_CMD_EXPORT_ALIAS(tpa626_test, tpa626_test, tpa626 read test.);

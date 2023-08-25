/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-24     zm       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#define DBG_TAG "macp2517fd test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static rt_device_t p_mcp2517fd_dev[2];
const char mcp2517fd_device_name[2][11] = {{"macp2517fd1"}, {"macp2517fd2"}};



static void MCP2517FDTestInit(rt_device_t *device, const char *device_name)
{
    device = rt_device_find(device_name);
    if (RT_NULL == device)
    {
        LOG_E("%s find NULL", device_name);
        return;
    }

    if(rt_device_init(device) != RT_EOK)
    {
        LOG_E("%s %d init fail", device_name);
        return;
    }

    if(rt_device_open(device, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("%s %d open fail", device_name);
        return;
    }

    LOG_I("%s open successful", device_name);
}

static void MCP2517FDTest(int argc, char **argv)
{
    uint8_t i = 0;

    if (argc != 2 && argc != 3)
    {
        rt_kprintf("Usage: mcp2517fdtest [cmd]\n");
        rt_kprintf("       mcp2517fdtest --init\n");
        rt_kprintf("       mcp2517fdtest --start\n");
        rt_kprintf("       mcp2517fdtest --stop\n");
    }
    else
    {
        if (rt_strcmp(argv[1], "--init") == 0)
        {
            for(i = 0; i < sizeof(p_mcp2517fd_dev) / sizeof(rt_device_t); i++)
            {
                MCP2517FDTestInit(p_mcp2517fd_dev[i], mcp2517fd_device_name[i][0]);
            }
        }
        else if(rt_strcmp(argv[1], "--start") == 0)
        {

        }
        else if(rt_strcmp(argv[1], "--stop") == 0)
        {

        }
        else
        {
            rt_kprintf("Usage: mcp2517fdtest [cmd]\n");
            rt_kprintf("       mcp2517fdtest --init\n");
            rt_kprintf("       mcp2517fdtest --start\n");
            rt_kprintf("       mcp2517fdtest --stop\n");
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
    MSH_CMD_EXPORT_ALIAS(MCP2517FDTest, mcp2517fdtest, macp2517fd test);
#endif /* RT_USING_FINSH */

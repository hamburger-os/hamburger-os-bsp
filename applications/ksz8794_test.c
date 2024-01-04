/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-23     zm       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#define DBG_TAG "ksz8794 test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static rt_device_t p_ksz8794_dev = NULL;

static void Ksz8794TestInit(void)
{
    p_ksz8794_dev = rt_device_find("ksz8794");
    if (RT_NULL == p_ksz8794_dev)
    {
        LOG_E("ksz8794 find NULL.");
        return;
    }

    if(rt_device_open(p_ksz8794_dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("ksz8794 open fail.");
        return;
    }

    LOG_I("ksz8794 open successful");
}

static void Ksz8794TestRun(void)
{
    uint8_t recievebuffer[8] = {0U};

    rt_device_read(p_ksz8794_dev, 0, recievebuffer, 1);
    LOG_I("1read %x", recievebuffer[0]);
    rt_device_read(p_ksz8794_dev, 1, recievebuffer, 1);
    LOG_I("2read %x", recievebuffer[0]);
}

#define KSZ8794_Port1_Status1           0x19
#define KSZ8794_Port1_Status2           0x1E
#define KSZ8794_Port1_Status3           0x1F
static void Ksz8794TestStatus(void)
{
    uint8_t ksz8794_StateBuf[3];

    rt_device_read(p_ksz8794_dev, KSZ8794_Port1_Status1, &ksz8794_StateBuf[0], 1);
    rt_device_read(p_ksz8794_dev, KSZ8794_Port1_Status2, &ksz8794_StateBuf[1], 1);
    rt_device_read(p_ksz8794_dev, KSZ8794_Port1_Status3, &ksz8794_StateBuf[2], 1);
    LOG_I("KSZ8794_Port1_Status: Status1=%2x,  Status2=%2x, Status3=%2x", \
                      ksz8794_StateBuf[0], ksz8794_StateBuf[1], ksz8794_StateBuf[2]);
}

static void KSZ8794Test(int argc, char **argv)
{

    if (argc != 2 && argc != 3)
    {
        rt_kprintf("Usage: ksz8794test [cmd]\n");
        rt_kprintf("       ksz8794test --init\n");
        rt_kprintf("       ksz8794test --start\n");
        rt_kprintf("       ksz8794test --close\n");
        rt_kprintf("       ksz8794test --status\n");
    }
    else
    {
        if (rt_strcmp(argv[1], "--init") == 0)
        {
            Ksz8794TestInit();
        }
        else if(rt_strcmp(argv[1], "--start") == 0)
        {
            Ksz8794TestRun();
        }
        else if(rt_strcmp(argv[1], "--close") == 0)
        {
            rt_device_close(p_ksz8794_dev);
        }
        else if(rt_strcmp(argv[1], "--status") == 0)
        {
            Ksz8794TestStatus();
        }
        else
        {
            rt_kprintf("Usage: ksz8794test [cmd]\n");
            rt_kprintf("       ksz8794test --init\n");
            rt_kprintf("       ksz8794test --start\n");
            rt_kprintf("       ksz8794test --close\n");
            rt_kprintf("       ksz8794test --status\n");
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
    MSH_CMD_EXPORT_ALIAS(KSZ8794Test, ksz8794test, ksz8794 test);
#endif /* RT_USING_FINSH */


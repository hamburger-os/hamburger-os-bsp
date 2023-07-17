/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-02-22     lvhan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <stdlib.h>

#define DBG_TAG "nor_test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static rt_thread_t norrw_thread = RT_NULL;

struct testDef
{
    char device[128];
    uint32_t len;
    uint32_t count;
};
static struct testDef test_def;

static void norrw_thread_entry(void* parameter)
{
    uint32_t index,length;
    uint32_t round;
    uint32_t tick_start,tick_end,read_speed,write_speed,erase_speed;
    struct testDef* ptest = parameter;

    /* find device */
    struct rt_mtd_nor_device* device = (struct rt_mtd_nor_device *)rt_device_find(ptest->device);
    if (device == NULL)
    {
        norrw_thread = RT_NULL;
        LOG_E("norrw find device '%s' failed", ptest->device);
        return;
    }

    LOG_D("norrw test start device %s %d %d", ptest->device, ptest->len, ptest->count);

    uint8_t *write_data = rt_malloc(ptest->len);
    uint8_t *read_data = rt_malloc(ptest->len);
    if (write_data == NULL || read_data == NULL)
    {
        rt_free(write_data);
        rt_free(read_data);
        LOG_W("Not enough memory to request a block.");

        ptest->len = 512;
        write_data = rt_malloc(ptest->len);
        read_data = rt_malloc(ptest->len);
        if (write_data == NULL || read_data == NULL)
        {
            norrw_thread = RT_NULL;
            rt_free(write_data);
            rt_free(read_data);
            LOG_E("Not enough memory to complete the test.");
            return;
        }
    }
    /* plan write data */
    for (index = 0; index < ptest->len; index ++)
    {
        write_data[index] = index;
    }

    round = 0;
    while(round < 4)
    {
        /* erase area */
        tick_start = rt_tick_get_millisecond();
        if (rt_mtd_nor_erase_block(device, 0, ptest->len * ptest->count) != RT_EOK)
        {
            LOG_E("norrw erase data%d failed %d", 0, ptest->len * ptest->count);
            device = NULL;
            goto end;
        }
        tick_end = rt_tick_get_millisecond();
        erase_speed = ptest->len * ptest->count / (tick_end-tick_start) * RT_TICK_PER_SECOND;

        /* write times */
        tick_start = rt_tick_get_millisecond();
        for(index = 0; index < ptest->count; index ++)
        {
            length = rt_mtd_nor_write(device, index * ptest->len, write_data, ptest->len);
            if (length != ptest->len)
            {
                LOG_E("norrw write data%d failed %d/%d", index, length, ptest->len);
                device = NULL;
                goto end;
            }
        }
        tick_end = rt_tick_get_millisecond();
        write_speed = ptest->len * ptest->count / (tick_end-tick_start) * RT_TICK_PER_SECOND;

        /* verify data */
        tick_start = rt_tick_get_millisecond();
        for(index=0; index<ptest->count; index++)
        {
            length = rt_mtd_nor_read(device, index * ptest->len, read_data, ptest->len);
            if (length != ptest->len)
            {
                LOG_E("norrw read data%d failed %d/%d", index, length, ptest->len);
                device = NULL;
                goto end;
            }
            if (rt_memcmp(write_data, read_data, ptest->len) != 0)
            {
                LOG_HEX("wr", 16, write_data, (ptest->len > 64)?(64):(ptest->len));
                LOG_HEX("rd", 16, read_data, (ptest->len > 64)?(64):(ptest->len));
                LOG_E("norrw data error!");
                device = NULL;
                goto end;
            }
        }
        tick_end = rt_tick_get_millisecond();
        read_speed = ptest->len * ptest->count / (tick_end-tick_start) * RT_TICK_PER_SECOND;

        LOG_D("thread norrw round %d size : %d KB, rd : %d KB/s, wr : %d KB/s, erase : %d KB/s"
                , round++, ptest->len * ptest->count / 1024, read_speed/1024, write_speed/1024, erase_speed/1024);
    }

end:
    rt_free(write_data);
    rt_free(read_data);
    LOG_D("norrw test end.");
    norrw_thread = RT_NULL;
}

/** \brief startup mtd nor read/write test(multi thread).
 * \return void
 *
 */
static void norrw_test(int argc, char *argv[])
{
    if (argc != 4)
    {
        rt_kprintf("Usage: norrw_test [device] [len] [count]\n");
    }
    else
    {
        if( norrw_thread != RT_NULL )
        {
            LOG_W("norrw thread already exists!");
        }
        else
        {
            rt_strcpy(test_def.device, argv[1]);
            test_def.len = atoi(argv[2]);
            test_def.count = atoi(argv[3]);
            norrw_thread = rt_thread_create( "norrw",
                                             norrw_thread_entry,
                                             &test_def,
                                             2048,
                                             RT_THREAD_PRIORITY_MAX-2,
                                             10);
            if ( norrw_thread != RT_NULL)
            {
                rt_thread_startup(norrw_thread);
            }
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(norrw_test, norrw_test, mtd nor R/W test.);
#endif

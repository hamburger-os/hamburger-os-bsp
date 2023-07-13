/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-01-02     aozima       the first version.
 * 2011-03-17     aozima       fix some bug.
 * 2011-03-18     aozima       to dynamic thread.
 */

#include <rtthread.h>
#include <dfs_file.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#define DBG_TAG "fs_test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static rt_thread_t fsrw_thread = RT_NULL;

#define fsrw_fn                   "/test.dat"
struct testDef
{
    char path[128];
    uint32_t len;
    uint32_t count;
};
static struct testDef test_def;

static void fsrw_thread_entry(void* parameter)
{
    int fd;
    uint32_t index, length;
    uint32_t round;
    uint32_t tick_start, tick_end, run_start, run_end;
    uint32_t read_speed, read_run, write_speed, write_run;

    struct testDef* ptest = parameter;

    rt_strncpy(&ptest->path[rt_strlen(ptest->path)], fsrw_fn, 10);
    LOG_D("fsrw test start file %s %d %d", ptest->path, ptest->len, ptest->count);

    char *write_data = rt_malloc(ptest->len);
    char *read_data = rt_malloc(ptest->len);
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
            fsrw_thread = RT_NULL;
            rt_free(write_data);
            rt_free(read_data);
            LOG_E("Not enough memory to complete the test.");
            return;
        }
    }
    /* plan write data */
    for (index = 0; index < ptest->len; index++)
    {
        write_data[index] = index;
    }

    round = 0;
    while (round < 4)
    {
        /* creat file */
        run_start = rt_tick_get_millisecond();
        fd = open(ptest->path, O_WRONLY | O_CREAT | O_TRUNC, 0);
        if (fd < 0)
        {
            LOG_E("fsrw open file '%s' for write failed %d", ptest->path, errno);
            goto end;
        }

        /* write times */
        tick_start = rt_tick_get_millisecond();
        for (index = 0; index < ptest->count; index++)
        {
            length = write(fd, write_data, ptest->len);
            if (length != ptest->len)
            {
                LOG_E("fsrw write data%d failed %d/%d", index, length, ptest->len);
                close(fd);
                goto end;
            }
        }
        tick_end = rt_tick_get_millisecond();
        write_speed = ptest->len * ptest->count / (tick_end - tick_start) * RT_TICK_PER_SECOND;

        /* close file */
        if (close(fd) != 0)
        {
            LOG_E("fsrw close file '%s' for write failed %d", ptest->path, errno);
            goto end;
        }
        run_end = rt_tick_get_millisecond();
        write_run = ptest->len * ptest->count / (run_end - run_start) * RT_TICK_PER_SECOND;
        rt_thread_delay(10);

        /* open file read only */
        run_start = rt_tick_get_millisecond();
        fd = open(ptest->path, O_RDONLY, 0);
        if (fd < 0)
        {
            LOG_E("fsrw open file '%s' for read failed %d", ptest->path, errno);
            goto end;
        }

        /* verify data */
        tick_start = rt_tick_get_millisecond();
        for (index = 0; index < ptest->count; index++)
        {
            rt_memset(read_data, 0, ptest->len);
            length = read(fd, read_data, ptest->len);
            if (length != ptest->len)
            {
                LOG_E("fsrw read data%d failed %d/%d", index, length, ptest->len);
                close(fd);
                goto end;
            }
            if (rt_memcmp(write_data, read_data, ptest->len) != 0)
            {
                LOG_E("fsrw check data%d failed %d/%d", index, length, ptest->len);
//                LOG_HEX("wr", 16, (uint8_t *)write_data, ptest->len);
//                LOG_HEX("rd", 16, (uint8_t *)read_data, ptest->len);
            }
        }
        tick_end = rt_tick_get_millisecond();
        read_speed = ptest->len * ptest->count / (tick_end - tick_start) * RT_TICK_PER_SECOND;

        /* close file */
        if (close(fd) != 0)
        {
            LOG_E("fsrw close file '%s' for read failed %d", ptest->path, errno);
            goto end;
        }
        run_end = rt_tick_get_millisecond();
        read_run = ptest->len * ptest->count / (run_end - run_start) * RT_TICK_PER_SECOND;
        rt_thread_delay(10);

        LOG_D("thread fsrw round %d size : %d KB, rd : %d - %d KB/s, wr : %d - %d KB/s"
                , round++, ptest->len * ptest->count / 1024
                , read_run / 1024, read_speed / 1024
                , write_run / 1024, write_speed / 1024);
    }

    end: fsrw_thread = RT_NULL;
    rt_free(write_data);
    rt_free(read_data);
    LOG_D("fsrw test end.");
}

/** \brief startup filesystem read/write test(multi thread).
 * \return void
 *
 */
static void fsrw_test(int argc, char *argv[])
{
    if (argc != 4)
    {
        rt_kprintf("Usage: fs_test [file_path] [len] [count]\n");
    }
    else
    {
        if (fsrw_thread != RT_NULL)
        {
            rt_kprintf("fsrw thread already exists!\n");
        }
        else
        {
            rt_memset(test_def.path, 0, 128);
            rt_strcpy(test_def.path, argv[1]);
            test_def.len = strtoul(argv[2], NULL, 10);
            test_def.count = strtoul(argv[3], NULL, 10);
            fsrw_thread = rt_thread_create("fsrw", fsrw_thread_entry, &test_def, 2 * 1024,
                                            RT_THREAD_PRIORITY_MAX - 2, 10);
            if (fsrw_thread != RT_NULL)
            {
                rt_thread_startup(fsrw_thread);
            }
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(fsrw_test, fsrw_test, file system R/W test.);
#endif

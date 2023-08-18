/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-17     lvhan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "selftest.h"

#include <dfs_file.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#define DBG_TAG "file"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define TEST_LEN    4096
#define TEST_COUNT  8

void selftest_fs_test(SelftestlUserData *puserdata)
{
    int fd;
    char path[128];
    uint32_t index, length;
    uint32_t tick_start, tick_end, run_start, run_end;
    uint32_t read_speed, read_run, write_speed, write_run;

    char *write_data = rt_malloc(TEST_LEN);
    char *read_data = rt_malloc(TEST_LEN);
    if (write_data == NULL || read_data == NULL)
    {
        rt_free(write_data);
        rt_free(read_data);
        LOG_W("Not enough memory to request a block.");
    }
    /* plan write data */
    for (index = 0; index < TEST_LEN; index++)
    {
        write_data[index] = index;
    }

    for (int x = 0; x<6; x++)
    {
        /* creat file path */
        rt_memset(path, 0, 128);
        rt_strncpy(path, puserdata->fs_path[x], rt_strlen(puserdata->fs_path[x]));
        rt_strncpy(&path[rt_strlen(puserdata->fs_path[x])], "/selftest.dat", 14);

        /* creat file */
        run_start = rt_tick_get_millisecond();
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0);
        if (fd < 0)
        {
            LOG_E("open file '%s' for write failed %d", path, errno);
            continue;
        }

        /* write times */
        tick_start = rt_tick_get_millisecond();
        for (index = 0; index < TEST_COUNT; index++)
        {
            length = write(fd, write_data, TEST_LEN);
            if (length != TEST_LEN)
            {
                LOG_E("write data%d failed %d/%d", index, length, TEST_LEN);
                close(fd);
                continue;
            }
        }
        tick_end = rt_tick_get_millisecond();
        write_speed = TEST_LEN * TEST_COUNT / (tick_end - tick_start) * RT_TICK_PER_SECOND;

        /* close file */
        if (close(fd) != 0)
        {
            LOG_E("close file '%s' for write failed %d", path, errno);
            continue;
        }
        run_end = rt_tick_get_millisecond();
        write_run = TEST_LEN * TEST_COUNT / (run_end - run_start) * RT_TICK_PER_SECOND;
        rt_thread_delay(10);

        /* open file read only */
        run_start = rt_tick_get_millisecond();
        fd = open(path, O_RDONLY, 0);
        if (fd < 0)
        {
            LOG_E("open file '%s' for read failed %d", path, errno);
            continue;
        }

        /* verify data */
        tick_start = rt_tick_get_millisecond();
        for (index = 0; index < TEST_COUNT; index++)
        {
            rt_memset(read_data, 0, TEST_LEN);
            length = read(fd, read_data, TEST_LEN);
            if (length != TEST_LEN)
            {
                LOG_E("read data%d failed %d/%d", index, length, TEST_LEN);
                close(fd);
                continue;
            }
            if (rt_memcmp(write_data, read_data, TEST_LEN) != 0)
            {
                LOG_E("check data%d failed %d/%d", index, length, TEST_LEN);
//                LOG_HEX("wr", 16, (uint8_t *)write_data, TEST_LEN);
//                LOG_HEX("rd", 16, (uint8_t *)read_data, TEST_LEN);
                continue;
            }
        }
        tick_end = rt_tick_get_millisecond();
        read_speed = TEST_LEN * TEST_COUNT / (tick_end - tick_start) * RT_TICK_PER_SECOND;

        /* close file */
        if (close(fd) != 0)
        {
            LOG_E("close file '%s' for read failed %d", path, errno);
            continue;
        }
        run_end = rt_tick_get_millisecond();
        read_run = TEST_LEN * TEST_COUNT / (run_end - run_start) * RT_TICK_PER_SECOND;
        rt_thread_delay(10);

        LOG_D("'%32s' size : %d KB, rd : %4d - %4d KB/s, wr : %4d - %4d KB/s"
                , path, TEST_LEN * TEST_COUNT / 1024
                , read_run / 1024, read_speed / 1024
                , write_run / 1024, write_speed / 1024);
    }

    rt_free(write_data);
    rt_free(read_data);
}

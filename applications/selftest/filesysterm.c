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

static char * error_log[] = {
    "fram       ",
    "spinor64   ",
    "nor        ",
    "emmc       ",
    "udisk      ",
};

void selftest_fs_test(SelftestlUserData *puserdata)
{
    int fd;
    char path[128];
    uint32_t index, length;

    char *write_data = rt_malloc(TEST_LEN);
    char *read_data = rt_malloc(TEST_LEN);
    if (write_data == NULL || read_data == NULL)
    {
        rt_free(write_data);
        rt_free(read_data);
        LOG_W("Not enough memory to request a block.");
        return;
    }
    /* plan write data */
    for (index = 0; index < TEST_LEN; index++)
    {
        write_data[index] = index;
    }

    for (int x = 0; x<5; x++)
    {
        uint8_t is_error = 0;

        /* creat file path */
        rt_memset(path, 0, 128);
        rt_strncpy(path, puserdata->fs_path[x], rt_strlen(puserdata->fs_path[x]));
        rt_strncpy(&path[rt_strlen(puserdata->fs_path[x])], "/selftest.dat", 14);

        /* creat file */
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0);
        if (fd < 0)
        {
            is_error ++;
        }

        /* write times */
        for (index = 0; index < TEST_COUNT; index++)
        {
            length = write(fd, write_data, TEST_LEN);
            if (length != TEST_LEN)
            {
                close(fd);
                is_error ++;
            }
        }

        /* close file */
        if (close(fd) != 0)
        {
            is_error ++;
        }
        rt_thread_delay(10);

        /* open file read only */
        fd = open(path, O_RDONLY, 0);
        if (fd < 0)
        {
            is_error ++;
        }

        /* verify data */
        for (index = 0; index < TEST_COUNT; index++)
        {
            rt_memset(read_data, 0, TEST_LEN);
            length = read(fd, read_data, TEST_LEN);
            if (length != TEST_LEN)
            {
                close(fd);
                is_error ++;
            }
            if (rt_memcmp(write_data, read_data, TEST_LEN) != 0)
            {
                is_error ++;
            }
        }

        /* close file */
        if (close(fd) != 0)
        {
            is_error ++;
        }
        rt_thread_delay(10);

        if (is_error == 0)
        {
            LOG_D("%s pass", error_log[x]);
        }
        else
        {
            LOG_E("%s error!", error_log[x]);
        }
    }

    rt_free(write_data);
    rt_free(read_data);
}

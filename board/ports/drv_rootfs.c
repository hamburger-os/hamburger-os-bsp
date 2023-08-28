/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     lvhan       the first version
 */
#include "board.h"
#include "msh.h"

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#define DBG_TAG "rootfs"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

enum
{
    ReadyToUpdate,
    FileNotExist,
    FileIsSame,
};

#ifdef ROOTFS_USING_UPDATE
struct rootfs_ops
{
    uint8_t devsta;//存储设备连接状态
    rt_device_t dev;//存储设备
    uint8_t updatesta;//升级状态

    char* devname;
    char *path;
};

static struct rootfs_ops rootfs_config = {
    .devname = ROOTFS_UPDATE_DEVNAME,
    .path = ROOTFS_UPDATE_PATH,
};

static const char *rootfs_dir[] = {
    "/bin", "/etc", "/lib", "/usr",
};

extern void copydir(const char *src, const char *dst);
static void rootfs_thread_entry(void* parameter)
{
    char path_dir[256] = {0};
    struct rootfs_ops *ops = (struct rootfs_ops *)parameter;

    while(1)
    {
        rt_thread_delay(1000);

        ops->dev = rt_device_find(ops->devname);
        if (ops->devsta == 0 && ops->dev != NULL)
        {
            LOG_D("dev in.");
            ops->devsta = 1;
            ops->updatesta = ReadyToUpdate;
            rt_thread_delay(200);//确保文件系统挂载
        }
        if (ops->devsta == 1 && ops->dev == NULL)
        {
            LOG_D("dev out.");
            ops->devsta = 0;
        }

        if (ops->devsta == 1 && ops->updatesta == ReadyToUpdate)
        {
            if (access(ops->path, 0) == 0)
            {
                LOG_D("'%s' is exisit", ops->path);

                for (int i = 0; i < (sizeof(rootfs_dir)/sizeof(char *)); i++)
                {
                    rt_sprintf(path_dir, "%s%s", ops->path, rootfs_dir[i]);
                    if (access(path_dir, 0) == 0)
                    {
                        LOG_D("'%s' is exisit", path_dir);

                        copydir(path_dir, rootfs_dir[i]);
                    }
                }

                ops->updatesta = FileIsSame;
            }
            else
            {
                LOG_D("dir '%s' not exisit", ops->path);
                ops->updatesta = FileNotExist;
            }
        }
    }
}
#endif

void delete_line_feed(char *line, char len)
{
    for (int i = 0; i<len; i++)
    {
        if (line[i] == '\n' || line[i] == '\r')
        {
            line[i] = 0;
        }
    }
}

static int rootfs_init(void)
{
#ifdef ROOTFS_USING_SELF_START
    char line_str[256] = {0};
    char line_len = 0;

    if (access(ROOTFS_SELF_START_PATH, 0) == 0)
    {
        LOG_D("scripts '%s' is exisit", ROOTFS_SELF_START_PATH);

        /* open file read only */
        FILE *file= fopen(ROOTFS_SELF_START_PATH, "r");
        if (file == NULL)
        {
            LOG_E("open scripts '%s' for read failed %d", ROOTFS_SELF_START_PATH, errno);
        }
        else
        {
            while (fgets(line_str, sizeof(line_str), file) != NULL)
            {
                line_len = rt_strlen(line_str);
                if (line_len > 0 && line_str[0] == '/')
                {
                    delete_line_feed(line_str, line_len);
                    LOG_D("scripts line : '%s'", line_str);

                    msh_exec(line_str, rt_strlen(line_str));
                }
            }

            fclose(file);
        }
    }
    else
    {
        LOG_W("scripts '%s' not exisit", ROOTFS_SELF_START_PATH);
    }
#endif

#ifdef ROOTFS_USING_UPDATE
    /* 创建线程 */
    rt_thread_t thread = rt_thread_create("rootfs", rootfs_thread_entry, &rootfs_config, 3072, 27, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
#endif

    return RT_EOK;
}
INIT_APP_EXPORT(rootfs_init);

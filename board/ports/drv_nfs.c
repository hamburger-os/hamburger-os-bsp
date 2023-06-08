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

#include <dfs_fs.h>

#define DBG_TAG "nfs"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static void nfs(int argc, char *argv[])
{
    int ret = 0;

    if (argc != 3)
    {
        rt_kprintf("Usage: nfs [server_path] [local_path]\n");
        rt_kprintf("       example: nfs 192.168.1.8:/c/work /nfs\n");
    }
    else
    {
        LOG_D("'%s' mount to '%s' ...", argv[1], argv[2]);
        /* 挂载文件系统 */
        ret = dfs_mount(argv[1], argv[2], "nfs", 0, 0);
        if (ret == 0)
        {
            LOG_I("mount succeed!");
        }
        else
        {
            LOG_E("mount failed %d!", ret);
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(nfs, nfs, nfs mount command.);
#endif

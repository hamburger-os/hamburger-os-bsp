/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-03     lvhan       the first version
 */
#include "board.h"

#ifdef PKG_USING_ULOG_BACK
#include <rtthread.h>

#define DBG_TAG "logfile"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <ulog_be.h>

#ifdef ULOG_BACK_USING_FILE
static struct ulog_file_be file_be;
#endif

static int rt_ulog_back_init(void)
{
#ifdef ULOG_BACK_USING_FILE
    ulog_file_backend_init(&file_be, "file"
            , ULOG_BACK_DIR_PATH, ULOG_BACK_FILE_MAX_NUM, ULOG_BACK_FILE_MAX_SIZE, ULOG_BACK_FILE_BUF_SIZE);
    ulog_file_backend_enable(&file_be);
#endif

#ifdef ULOG_BACK_USING_TSDB

#endif

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_APP_EXPORT(rt_ulog_back_init);

#endif

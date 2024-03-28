/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-31     zm       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include "linklib/inc/if_app.h"

static int STOComBoardInit(void)
{
    app_archInit();
    return RT_EOK;
}

INIT_APP_EXPORT(STOComBoardInit);

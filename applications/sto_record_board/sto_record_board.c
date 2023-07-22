/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-27     zm       the first version
 */
#include "sto_record_board.h"
#include "led.h"
#include "eth_manage.h"
#include "board_info.h"

#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG "STORecordBoard"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static void STORecordBoardInit(void)
{
    LedCtrlInit();
    ETHThreadInit();
//    ETHManageTestThreadInit();
    FileThreadInit();
}

INIT_APP_EXPORT(STORecordBoardInit);

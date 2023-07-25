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
#include "file_manager.h"
#include "board_info.h"

#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG "STORecordBoard"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

S_DATA_HANDLE eth_can_data_handle;
S_FILE_MANAGER file_manager;

static void STORecordBoardInit(void)
{
    DataHandleInit(&eth_can_data_handle);
    FMInit(&file_manager);
    LedCtrlInit();
//    ETHThreadInit();
//    ETHManageTestThreadInit();
    FileThreadInit();
}

INIT_APP_EXPORT(STORecordBoardInit);

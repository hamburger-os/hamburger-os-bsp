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
#include "board_info.h"
#include "usb.h"
#include "eth_thread.h"
#include "file_handle_thread.h"
#include "board_info.h"

#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG "STORecordBoard"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

S_DATA_HANDLE eth0_can_data_handle;
S_DATA_HANDLE eth1_can_data_handle;
S_FILE_MANAGER file_manager;

static void STORecordBoardInit(void)
{
    DataHandleInit(&eth0_can_data_handle);
    DataHandleInit(&eth1_can_data_handle);
    FMInit(&file_manager);
    LedCtrlInit();

    /* thread */
    ETHThreadInit();
    FileThreadInit();
    usb_init();
}

INIT_APP_EXPORT(STORecordBoardInit);

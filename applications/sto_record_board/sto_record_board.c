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

#include <rtthread.h>
#include <rtdevice.h>

#include "led.h"
#include "board_info.h"
#include "usb.h"
#include "eth_thread.h"
#include "file_handle_thread.h"
#include "board_info.h"
#include "power_msg.h"

#define DBG_TAG "STORecordBoard"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define BOARD_INIT_THREAD_PRIORITY         24
#define BOARD_INIT_THREAD_STACK_SIZE       (1024 * 3)
#define BOARD_INIT_THREAD_TIMESLICE        5

S_DATA_HANDLE eth0_can_data_handle;
S_DATA_HANDLE eth1_can_data_handle;
S_FILE_MANAGER file_manager;

static void *BoardInitThreadEntry(void *parameter)
{
    DataHandleInit(&eth0_can_data_handle);
    DataHandleInit(&eth1_can_data_handle);
    FMInit(&file_manager);
    LedCtrlInit();

    /* thread */
    ETHThreadInit();
    FileThreadInit();
//    PowerMsgThreadInit();
    usb_init();
    LOG_I("init ok");
}

static int BoardInitThreadInit(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("board_init",
                            BoardInitThreadEntry, RT_NULL,
                            BOARD_INIT_THREAD_STACK_SIZE,
                            BOARD_INIT_THREAD_PRIORITY, BOARD_INIT_THREAD_TIMESLICE);

    if(tid != NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    return -RT_ERROR;
}

static void STORecordBoardInit(void)
{
    BoardInitThreadInit();
}

INIT_APP_EXPORT(STORecordBoardInit);

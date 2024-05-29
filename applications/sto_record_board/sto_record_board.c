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

#define DBG_TAG "STORecordBoard"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include "led.h"
#include "board_info.h"
#include "usb.h"
#include "eth_thread.h"
#include "file_handle_thread.h"
#include "board_info.h"
#include "power_msg.h"
#include "udp_comm.h"
#include "Record_FileDown.h"
#include "record_ota.h"
#include "swos2_cfg_init.h"

#define BOARD_INIT_THREAD_PRIORITY         (24)
#define BOARD_INIT_THREAD_STACK_SIZE       (1024 * 4)
#define BOARD_INIT_THREAD_TIMESLICE        5

S_DATA_HANDLE eth0_can_data_handle;
S_DATA_HANDLE eth1_can_data_handle;
S_FILE_MANAGER file_manager;

static void BoardInitThreadEntry(void *parameter)
{
    if(BoardInfoInit() !=RT_EOK)
    {
        LOG_E("board info init error");
        return;
    }

    if(DataHandleInit(&eth0_can_data_handle) !=RT_EOK)
    {
        LOG_E("eth0 can data init error");
        return;
    }

    if(DataHandleInit(&eth1_can_data_handle) !=RT_EOK)
    {
        LOG_E("eth1 can data init error");
        return;
    }

    if(FMInit(&file_manager) != 0)
    {
        LOG_E("FMInit error");
        return;
    }

    if(UDPServerInit() !=RT_EOK)
    {
        LOG_E("UDPServerInit error");
        return;
    }

    if(ETHInit() !=RT_EOK)
    {
        LOG_E("ETHInit error");
        return;
    }

    if(PowerMsgInit() != RT_EOK)
    {
        LOG_E("power msg init error");
        return;
    }

    LedCtrlInit();
    DownloadFileInit();
    RecordOTAInit();

    /* thread */
    ETHThreadInit();
    FileThreadInit();
    PowerMsgThreadInit();
    UDPServerRcvThreadInit();
    RecordOTAInitThreadInit();
    BoardInfoThreadInit();
    usb_init();
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

static int STORecordBoardInit(void)
{
    if(SWOS2CfgETHIP() != RT_EOK)
    {
        LOG_E("swos2 cfg kvdb net error!");
        return -RT_ERROR;
    }
    BoardInitThreadInit();
    return RT_EOK;
}

INIT_APP_EXPORT(STORecordBoardInit);

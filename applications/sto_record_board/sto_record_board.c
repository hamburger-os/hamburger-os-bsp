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
#include "led_ctrl.h"

static void STORecordBoardInit(void)
{
  LedCtrlInit();
}

INIT_APP_EXPORT(STORecordBoardInit);

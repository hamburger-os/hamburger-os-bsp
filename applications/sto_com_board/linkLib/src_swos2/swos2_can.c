/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_can.h"

#define DBG_TAG "if_can"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

BOOL if_can_init( void )
{

    return TRUE;
}

BOOL if_can_send( E_CAN_CH ch, S_CAN_MSG *pMsg )
{

    return TRUE;
}

BOOL if_can_get( E_CAN_CH ch, S_CAN_MSG *pMsg )
{

    return TRUE;
}

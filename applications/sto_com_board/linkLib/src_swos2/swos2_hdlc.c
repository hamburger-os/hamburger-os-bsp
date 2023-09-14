/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_hdlc.h"

#define DBG_TAG "if_hdlc"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

BOOL if_hdlc_init( void )
{
    return TRUE;
}

BOOL if_hdlc_send( E_HDLC_CH ch, uint8 *pdata, uint16 len )
{
    return TRUE;
}

uint16 if_hdlc_get( E_HDLC_CH ch, uint8 *pdata, uint16 len )
{
    return 0;
}


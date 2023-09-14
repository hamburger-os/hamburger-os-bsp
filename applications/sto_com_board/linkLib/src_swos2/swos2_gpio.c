/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_gpio.h"

#define DBG_TAG "if_gpio"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

void if_gpio_init( void )
{

}

void if_gpio_set( E_IO_ID id )
{

}

void if_gpio_reset( E_IO_ID id )
{

}

void if_gpio_toggle( E_IO_ID id )
{

}

BOOL if_gpio_get( E_IO_ID id )
{
    return TRUE;
}

E_SLOT_ID if_gpio_getSlotId( void )
{
    static E_SLOT_ID slot_id = E_SLOT_ID_MAX;

    if(E_SLOT_ID_MAX != slot_id)
    {
        return slot_id;
    }
    else
    {

    }


    return slot_id;
}

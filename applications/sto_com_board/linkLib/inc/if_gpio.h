/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : if_gpio.h
**@author: Created By Chengt
**@date  : 2023.09.06
**@brief : Implement the function interfaces of if_gpio
********************************************************************************************/
#ifndef _IF_GPIO_H
#define _IF_GPIO_H

#include "common.h"

/* 槽位类型 */
typedef enum
{
    E_SLOT_ID_1 = 0U,
    E_SLOT_ID_2,
    E_SLOT_ID_3,
    E_SLOT_ID_4,
    E_SLOT_ID_5,
    E_SLOT_ID_6,
    E_SLOT_ID_MAX
}E_SLOT_ID;

/* IO类型 */
typedef enum
{
    E_IO_ID_1 = 0U,
    E_IO_ID_2,
    E_IO_ID_3,
    E_IO_ID_4,
    E_IO_ID_MAX
}E_IO_ID;


extern void if_gpio_init( void );
extern void if_gpio_set( E_IO_ID id );
extern void if_gpio_reset( E_IO_ID id );
extern void if_gpio_toggle( E_IO_ID id );
extern BOOL if_gpio_get( E_IO_ID id );
extern E_SLOT_ID if_gpio_getSlotId( void );

#endif
/*******************************************end file*******************************************************/

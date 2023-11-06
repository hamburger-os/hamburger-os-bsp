/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_gpio.c
 **@author: Created By Chengt
 **@date  : 2023.09.06
 **@brief : Manage support layer gpio
 ********************************************************************************************/
#include "if_gpio.h"
#include "support_gpio.h"

/*******************************************************************************************
 *        Local definitions
 *******************************************************************************************/

/*******************************************************************************************
 *        Local variables
 *******************************************************************************************/

/*******************************************************************************************
 *        Local functions
 *******************************************************************************************/

/*******************************************************************************************
 ** @brief: get_ctl_id
 ** @param: id
 *******************************************************************************************/
static E_IO_ID get_ctl_id(E_GPIO_ID id)
{
    E_IO_ID io_id = E_IO_ID_MAX;

    switch (id)
    {
    case LED1_ID:
        io_id = E_IO_ID_1;
        break;
    case LED2_ID:
        io_id = E_IO_ID_2;
        break;
    case LED3_ID:
        io_id = E_IO_ID_3;
        break;
    case LED4_ID:
        io_id = E_IO_ID_4;
        break;
    default:
        io_id = E_IO_ID_MAX;
        break;
    }

    return io_id;
}

/*******************************************************************************************
 ** @brief: support_gpio_init
 ** @param: null
 *******************************************************************************************/
extern E_GPIO_RET support_gpio_init(E_GPIO_ID id, E_GPIO_STATE state)
{
    if_gpio_init();

    return IO_OK;
}

/*******************************************************************************************
 ** @brief: support_gpio_set
 ** @param: null
 *******************************************************************************************/
extern E_GPIO_RET support_gpio_set(E_GPIO_ID id, E_GPIO_STATE state)
{
    E_IO_ID ctl_id = E_IO_ID_MAX;
    /* 1.参数检查 */
    if (LED_NONE == id)
    {
        return IO_ERR;
    }

    ctl_id = get_ctl_id(id);
    if (E_IO_ID_MAX == ctl_id)
    {
        return IO_ERR;
    }

    /* 2.LED控制状态转换 */
    switch (state)
    {
    case IO_LOW:
        if_gpio_reset(ctl_id);
        break;
    case IO_HIGH:
        if_gpio_set(ctl_id);
        break;
    case IO_TOGGLE:
        if_gpio_toggle(ctl_id);
        break;
    default:
        break;
    }

    return IO_OK;
}

/*******************************************************************************************
 ** @brief: support_gpio_get
 ** @param: null
 *******************************************************************************************/
extern E_GPIO_RET support_gpio_get(E_GPIO_ID id, E_GPIO_STATE *state)
{
    E_IO_ID ctl_id = E_IO_ID_MAX;
    /* 1.参数检查 */
    if (LED_NONE == id)
    {
        return IO_ERR;
    }

    ctl_id = get_ctl_id(id);
    if (E_IO_ID_MAX == ctl_id)
    {
        return IO_ERR;
    }

    /* 2.获取LED控制状态 */
    if ( TRUE == if_gpio_get(ctl_id))
    {
        *state = IO_HIGH;
    }
    else
    {
        *state = IO_HIGH;
    }

    return IO_OK;
}

/*******************************************************************************************
 ** @brief: support_gpio_toggle
 ** @param: null
 *******************************************************************************************/
extern E_GPIO_RET support_gpio_toggle(E_GPIO_ID id)
{
    E_IO_ID ctl_id = E_IO_ID_MAX;

    /* 1.参数检查 */
    if (LED_NONE == id)
    {
        return IO_ERR;
    }

    ctl_id = get_ctl_id(id);
    if (E_IO_ID_MAX == ctl_id)
    {
        return IO_ERR;
    }

    /* 2.获取LED控制状态 */
    if_gpio_toggle(ctl_id);

    return IO_OK;
}

/*******************************************************************************************
 ** @brief: support_gpio_toggle
 ** @param: null
 *******************************************************************************************/
extern E_BOARD_ID support_gpio_getBoardId(void)
{
    E_BOARD_ID boardId = ID_NONE;
    E_SLOT_ID slot_id = E_SLOT_ID_MAX;

    /* 1.获取平台状态信息 */
    slot_id = if_gpio_getSlotId();

    /* 2.获取板子位置信息 */
    switch (slot_id)
    {
    case E_SLOT_ID_1:
    case E_SLOT_ID_5:
        boardId = ID_TX1_Load;
        break;
    case E_SLOT_ID_2:
    case E_SLOT_ID_6:
        boardId = ID_TX1_Child;
        break;
    case E_SLOT_ID_3:
    case E_SLOT_ID_7:
        boardId = ID_TX2_Load;
        break;
    case E_SLOT_ID_4:
    case E_SLOT_ID_8:
        boardId = ID_TX2_Child;
        break;
    case E_SLOT_ID_9:
        boardId = ID_RD;
        break;
    case E_SLOT_ID_10:
        boardId = ID_WX;
        break;
    default:
        break;
    }
    boardId = ID_TX2_Load;
    /* 3.返回boardId */
    return boardId;
}
/**************************************end file*********************************************/

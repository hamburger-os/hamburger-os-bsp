/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_gpio.h
 **@author: Created By Chengt
 **@date  : 2019.12.13
 **@brief : Manage support GPIO
 ********************************************************************************************/
#ifndef _SUPPORT_GPIO_H
#define _SUPPORT_GPIO_H

/* LED_ID */
typedef enum
{
    LED_NONE = 0U,
    LED1_ID,
    LED2_ID,
    LED3_ID,
    LED4_ID
} E_GPIO_ID;

/* GPIO_STATE */
typedef enum
{
    IO_LOW = 0U,
    IO_HIGH,
    IO_TOGGLE
} E_GPIO_STATE;

/* GPIO_RET */
typedef enum
{
    IO_OK = 0U,
    IO_ERR
} E_GPIO_RET;

/* BOARD_ID */
typedef enum
{
    ID_NONE = 0U, /* 未定义 */
    ID_TX1_Load, /* 通信1底板 */
    ID_TX1_Child, /* 通信1子板 */
    ID_TX2_Load, /* 通信2底板 */
    ID_TX2_Child, /* 通信2子板 */
    ID_RD, /* 记录板 */
    ID_WX /* 无线板 */
} E_BOARD_ID;

extern E_GPIO_RET support_gpio_init(E_GPIO_ID id, E_GPIO_STATE state);
extern E_GPIO_RET support_gpio_set(E_GPIO_ID id, E_GPIO_STATE state);
extern E_GPIO_RET support_gpio_get(E_GPIO_ID id, E_GPIO_STATE *state);
extern E_GPIO_RET support_gpio_toggle(E_GPIO_ID id);

extern E_BOARD_ID support_gpio_getBoardId(void);
#endif
/**************************************end file*********************************************/

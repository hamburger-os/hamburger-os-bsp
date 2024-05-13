/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : devlib_can.c
 **@author: Created By jiaqx
 **@date  : 2023.11.02
 **@brief : app_main function
 ********************************************************************************************/

#include "support_gpio.h"
#include "devlib_rs485.h"
#include "support_libPub.h"

/*******************************************************************************************
 ** @brief: rs485_resource_map_TX1_Load
 ** @param: e_rs485_cls
 *******************************************************************************************/
static E_RS485_ID rs485_resource_map_TX1_Load( E_RS485_CLASS e_rs485_cls )
{
    E_RS485_ID e_rs485_id = E_RS485_ID_MAX ;

    /* 1.校验通道与板卡是否匹配 */
    if ((E_RS485_EX1 != e_rs485_cls))
    {
        MY_Printf("rs485_resource_map_TX1_Load e_rs485_cls parameter is illegal !!!\r\n");
        return e_rs485_id ;
    }

    /* 2.依据资源表配置CANID */
    switch (e_rs485_cls)
    {
        case E_RS485_EX1:e_rs485_id = E_RS485_ID_1;break;
        default: e_rs485_id = E_RS485_ID_MAX;break;
    }

    /* 2.依据资源表配置网络ID*/
    return e_rs485_id;
}

/*******************************************************************************************
 ** @brief: rs485_resource_map_TX1_Child
 ** @param: e_rs485_cls
 *******************************************************************************************/
static E_RS485_ID rs485_resource_map_TX1_Child( E_RS485_CLASS e_rs485_cls )
{
    E_RS485_ID e_rs485_id = E_RS485_ID_MAX ;

    /* 1.校验通道与板卡是否匹配 */
    if ((E_RS485_EX1 != e_rs485_cls))
    {
        MY_Printf("rs485_resource_map_TX1_Child e_rs485_cls parameter is illegal !!!\r\n");
        return e_rs485_id ;
    }

    /* 2.依据资源表配置CANID */
    switch (e_rs485_cls)
    {
        case E_RS485_EX1:e_rs485_id = E_RS485_ID_1;break;
        default: e_rs485_id = E_RS485_ID_MAX;break;
    }

    /* 2.依据资源表配置网络ID*/
    return e_rs485_id;
}

/*******************************************************************************************
 ** @brief: rs485_resource_map_TX2_Load
 ** @param: e_rs485_cls
 *******************************************************************************************/
static E_RS485_ID rs485_resource_map_TX2_Load( E_RS485_CLASS e_rs485_cls )
{
    E_RS485_ID e_rs485_id = E_RS485_ID_MAX ;

    /* 1.校验通道与板卡是否匹配 */
    if ((E_RS485_MAX != e_rs485_cls))
    {
        MY_Printf("rs485_resource_map_TX2_Load e_rs485_cls parameter is illegal !!!\r\n");
        return e_rs485_id ;
    }

    /* 2.依据资源表配置CANID */
    switch (e_rs485_cls)
    {
        case E_RS485_EX1:e_rs485_id = E_RS485_ID_1;break;
        default: e_rs485_id = E_RS485_ID_MAX;break;
    }

    /* 2.依据资源表配置网络ID*/
    return e_rs485_id;
}

/*******************************************************************************************
 ** @brief: rs485_resource_map_TX2_Child
 ** @param: e_rs485_cls
 *******************************************************************************************/
static E_RS485_ID rs485_resource_map_TX2_Child( E_RS485_CLASS e_rs485_cls )
{
    E_RS485_ID e_rs485_id = E_RS485_ID_MAX ;

    /* 1.校验通道与板卡是否匹配 */
    if ((E_RS485_EX1 != e_rs485_cls))
    {
        MY_Printf("rs485_resource_map_TX2_Child e_rs485_cls parameter is illegal !!!\r\n");
        return e_rs485_id ;
    }

    /* 2.依据资源表配置CANID */
    switch (e_rs485_cls)
    {
        case E_RS485_EX1:e_rs485_id = E_RS485_ID_1;break;
        default: e_rs485_id = E_RS485_ID_MAX;break;
    }

    /* 2.依据资源表配置网络ID*/
    return e_rs485_id;
}

/*******************************************************************************************
 ** @brief: rs485_get_resource
 ** @param: e_rs485_cls
 *******************************************************************************************/
static E_RS485_ID rs485_get_resource( E_RS485_CLASS e_rs485_cls )
{
    E_RS485_ID e_rs485_id = E_RS485_ID_MAX ;
    E_BOARD_ID BoardId = ID_NONE;

    /* 1.获取板载ID */
    BoardId = support_gpio_getBoardId();

    /* 2. 获取支持层网络ID*/
    switch( BoardId )
    {
    case ID_TX1_Load:
        e_rs485_id = rs485_resource_map_TX1_Load( e_rs485_cls );
        break;
    case ID_TX1_Child:
        e_rs485_id = rs485_resource_map_TX1_Child( e_rs485_cls );
        break;
    case ID_TX2_Load:
        e_rs485_id = rs485_resource_map_TX2_Load( e_rs485_cls );
        break;
    case ID_TX2_Child:
        e_rs485_id = rs485_resource_map_TX2_Child( e_rs485_cls );
        break;
    default:
        MY_Printf("rs485_get_resource BoardId is NONE\r\n");
        e_rs485_id = E_RS485_ID_MAX;
        break;
    }
    return e_rs485_id;
}


/*******************************************************************************************
 ** @brief: devLib_rs485_sendData
 ** @param: e_rs485_cls,*pframe
 *******************************************************************************************/
extern BOOL devLib_rs485_sendData( E_RS485_CLASS e_rs485_cls, S_RS485_FRAME *pframe )
{
    BOOL retFlg = FALSE;
    E_RS485_ID e_rs485_id = E_RS485_ID_MAX;

    /* 1.参数合法性检查 */
    if(( NULL == pframe ) || ( 0U == pframe->len ) )
    {
        //MY_Printf("devLib_rs485_sendData formal parameter is illegal !!!");
        return FALSE;
    }

    /* 2.获取硬件资源 */
    e_rs485_id = rs485_get_resource( e_rs485_cls );

    /* 5.发送数据 */
    if( E_RS485_OK == support_rs485_sendData( e_rs485_id, pframe ))
    {
        retFlg = TRUE;
    }

    return retFlg;
}

/*******************************************************************************************
 ** @brief: devLib_rs485_getData
 ** @param: e_rs485_cls,*pframe
 *******************************************************************************************/
extern BOOL devLib_rs485_getData( E_RS485_CLASS e_rs485_cls, S_RS485_FRAME *pframe )
{
    BOOL retFlg = FALSE;
    E_RS485_ID e_rs485_id = E_RS485_ID_MAX;

    /* 1.参数合法性检查 */
    if( NULL == pframe )
    {
        //MY_Printf("devLib_rs485_getData formal parameter is illegal !!!\r\n");
        return retFlg;
    }

    /* 2.获取硬件资源 */
    e_rs485_id = rs485_get_resource( e_rs485_cls );

    /* 5.发送数据 */
    if( E_RS485_OK == support_rs485_sendData( e_rs485_id, pframe ))
    {
        retFlg = TRUE;
    }

    return retFlg;
}

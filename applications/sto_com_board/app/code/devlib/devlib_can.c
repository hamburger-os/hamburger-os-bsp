/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : devlib_can.c
 **@author: Created By jiaqx
 **@date  : 2023.11.02
 **@brief : app_main function
 ********************************************************************************************/

#include "support_gpio.h"
#include "devlib_can.h"
#include "support_libPub.h"

/*******************************************************************************************
 ** @brief: eth_resource_map_TX1_Load
 ** @param: eth_cls
 *******************************************************************************************/
static E_CAN_ID can_resource_map_TX1_Load( E_CAN_CLASS can_cls )
{
    E_CAN_ID e_can_id = E_CAN_ID_MAX;

    /* 1.校验通道与板卡是否匹配 */
    if (can_cls < E_CAN_ZK1 || can_cls > E_CAN_EX2)
    {
        MY_Printf("can_resource_map_TX1_Load can_cls parameter is illegal !!!\r\n");
        return e_can_id ;
    }

    /* 2.依据资源表配置CANID*/
    switch (can_cls)
    {
        case E_CAN_ZK1:e_can_id = E_CAN_ID_1;break;
        case E_CAN_ZK2:e_can_id = E_CAN_ID_2;break;
        case E_CAN_EX1:e_can_id = E_CAN_ID_3;break;
        case E_CAN_EX2:e_can_id = E_CAN_ID_4;break;
        default: e_can_id = E_CAN_ID_MAX;break;
    }

    return e_can_id;
}

/*******************************************************************************************
 ** @brief: eth_resource_map_TX1_Child
 ** @param: eth_cls
 *******************************************************************************************/
static E_CAN_ID can_resource_map_TX1_Child( E_CAN_CLASS can_cls )
{
    E_CAN_ID e_can_id = E_CAN_ID_MAX;

    /* 1.校验通道与板卡是否匹配 */
    if (can_cls < E_CAN_ZK1 || can_cls > E_CAN_EX3)
    {
        MY_Printf("can_resource_map_TX1_Child can_cls parameter is illegal !!!\r\n");
        return e_can_id ;
    }

    /* 2.依据资源表配置CANID*/
    switch (can_cls)
    {
        case E_CAN_ZK1:e_can_id = E_CAN_ID_1;break;
        case E_CAN_ZK2:e_can_id = E_CAN_ID_2;break;
        case E_CAN_EX1:e_can_id = E_CAN_ID_3;break;
        case E_CAN_EX2:e_can_id = E_CAN_ID_4;break;
        case E_CAN_EX3:e_can_id = E_CAN_ID_5;break;
        default: e_can_id = E_CAN_ID_MAX;break;
    }

    return e_can_id;
}

/*******************************************************************************************
 ** @brief: eth_resource_map_TX2_Load
 ** @param: eth_cls
 *******************************************************************************************/
static E_CAN_ID can_resource_map_TX2_Load( E_CAN_CLASS can_cls )
{
    E_CAN_ID e_can_id = E_CAN_ID_MAX;

    /* 1.校验通道与板卡是否匹配 */
    if (can_cls < E_CAN_ZK1 || can_cls > E_CAN_EX3)
    {
        MY_Printf("can_resource_map_TX2_Load can_cls parameter is illegal !!!\r\n");
        return e_can_id ;
    }

    /* 2.依据资源表配置CANID */
    switch (can_cls)
    {
        case E_CAN_ZK1:e_can_id = E_CAN_ID_1;break;
        case E_CAN_ZK2:e_can_id = E_CAN_ID_2;break;
        case E_CAN_EX1:e_can_id = E_CAN_ID_3;break;
        case E_CAN_EX2:e_can_id = E_CAN_ID_4;break;
        default: e_can_id = E_CAN_ID_MAX;break;
    }

    return e_can_id;
}

/*******************************************************************************************
 ** @brief: eth_resource_map_TX2_Child
 ** @param: eth_cls
 *******************************************************************************************/
static E_CAN_ID can_resource_map_TX2_Child( E_CAN_CLASS can_cls )
{
    E_CAN_ID e_can_id = E_CAN_ID_MAX;

    /* 1.校验通道与板卡是否匹配 */
    if (can_cls < E_CAN_ZK1 || can_cls > E_CAN_MAX)
    {
        MY_Printf("can_resource_map_TX2_Child can_cls parameter is illegal !!!\r\n");
        return e_can_id ;
    }

    /* 2.依据资源表配置CANID */
    switch (can_cls)
    {
        case E_CAN_ZK1:e_can_id = E_CAN_ID_1;break;
        case E_CAN_ZK2:e_can_id = E_CAN_ID_2;break;
        case E_CAN_EX1:e_can_id = E_CAN_ID_3;break;
        case E_CAN_EX2:e_can_id = E_CAN_ID_4;break;
        case E_CAN_EX3:e_can_id = E_CAN_ID_5;break;
        default: e_can_id = E_CAN_ID_MAX;break;
    }

    /* 2.依据资源表配置网络ID*/
    return e_can_id;
}

/*******************************************************************************************
 ** @brief: eth_get_resource
 ** @param: eth_cls
 *******************************************************************************************/
static E_CAN_ID can_get_resource( E_CAN_CLASS can_cls )
{
    E_CAN_ID e_can_id = E_CAN_ID_MAX ;
    E_BOARD_ID BoardId = ID_NONE;

    /* 1.获取板载ID */
    BoardId = support_gpio_getBoardId();

    /* 2. 获取支持层网络ID*/
    switch( BoardId )
    {
    case ID_TX1_Load:
        e_can_id = can_resource_map_TX1_Load( can_cls );
        break;
    case ID_TX1_Child:
        e_can_id = can_resource_map_TX1_Child( can_cls );
        break;
    case ID_TX2_Load:
        e_can_id = can_resource_map_TX2_Load( can_cls );
        break;
    case ID_TX2_Child:
        e_can_id = can_resource_map_TX2_Child( can_cls );
        break;
    default:
        MY_Printf("can_get_resource BoardId is NONE\r\n");
        e_can_id = E_CAN_ID_MAX;
        break;
    }
    return e_can_id;
}


/*******************************************************************************************
 ** @brief: eth_get_resource
 ** @param: eth_cls
 *******************************************************************************************/
static E_CAN_ID can_get_mode( E_CAN_CLASS can_cls )
{
    E_CAN_MODE e_can_mode = E_CAN_MODE_MAX;

    /* 依据资源表配置配置网络模式*/
    switch (can_cls)
    {
        case E_CAN_ZK1:e_can_mode = E_CAN_FD_MODE;break;
        case E_CAN_ZK2:e_can_mode = E_CAN_FD_MODE;break;
        case E_CAN_EX1:e_can_mode = E_CAN_NORMAL_MODE;break;
        case E_CAN_EX2:e_can_mode = E_CAN_NORMAL_MODE;break;
        case E_CAN_EX3:e_can_mode = E_CAN_NORMAL_MODE;break;
        default: e_can_mode = E_CAN_MODE_MAX;break;
    }

    return e_can_mode;
}

/*******************************************************************************************
 ** @brief: devLib_eth_sendData
 ** @param: eth_cls,*pdata,len
 *******************************************************************************************/
extern BOOL devLib_can_sendData( E_CAN_CLASS can_cls, S_CAN_FRAME_DATA *p_can_frame_data )
{
    BOOL retFlg = FALSE;
    E_CAN_ID e_can_id = E_CAN_ID_MAX;
    S_CAN_FRAME s_can_frame = {0};

    /* 1.参数合法性检查 */
    if(( NULL == p_can_frame_data ) || ( 0U == p_can_frame_data->length_u8 ) )
    {
        //MY_Printf("devLib_can_sendData formal parameter is illegal !!!");
        return FALSE;
    }

    /* 2.获取硬件资源 */
    e_can_id = can_get_resource( can_cls );

    /* 3.获取CAN通信方式，FDCAN or CAN2.0 */
    s_can_frame.can_mode =  can_get_mode( can_cls );

    /* 4.组织CAN帧数据 */
    support_memcpy(&s_can_frame.priority_u8, p_can_frame_data, sizeof(S_CAN_FRAME_DATA));

    /* 5.发送数据 */
    if( E_CAN_OK == support_can_sendData( e_can_id, &s_can_frame ))
    {
        retFlg = TRUE;
    }

    return retFlg;
}

/*******************************************************************************************
 ** @brief: devLib_eth_getData
 ** @param: eth_cls,*pframe
 *******************************************************************************************/
extern BOOL devLib_can_getData( E_CAN_CLASS can_cls, S_CAN_FRAME_DATA *p_can_frame_data )
{
    BOOL retFlg = FALSE;
    E_CAN_ID e_can_id = E_CAN_ID_MAX;

    S_CAN_FRAME s_can_frame;

    /* 1.参数合法性检查 */
    if( NULL == p_can_frame_data )
    {
        //MY_Printf("devLib_can_getData formal parameter is illegal !!!\r\n");
        return FALSE;
    }

    /* 2.获取硬件资源 */
    e_can_id = can_get_resource( can_cls );

    /* 5.发送数据 */
    if( E_CAN_OK == supprot_can_getData( e_can_id, &s_can_frame ))
    {
        //MY_Printf("s_can_frame.can_mode is %d !!!\r\n",s_can_frame.can_mode);
        support_memcpy(p_can_frame_data, &s_can_frame.priority_u8, sizeof(S_CAN_FRAME_DATA));
        retFlg = TRUE;
    }
    else
    {
        retFlg = FALSE;
    }

    return retFlg;
}

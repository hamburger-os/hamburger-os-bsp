/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : devlib_can.c
 **@author: Created By jiaqx
 **@date  : 2023.11.02
 **@brief : app_main function
 ********************************************************************************************/

#include "support_gpio.h"
#include "devlib_mvb.h"

/*******************************************************************************************
 ** @brief: eth_resource_map_TX1_Load
 ** @param: eth_cls
 *******************************************************************************************/
static E_MVB_ID mvb_resource_map_TX1_Load( E_MVB_CLASS e_mvb_cls )
{
    E_MVB_ID e_mvb_id = E_MVB_ID_MAX;
    /* 1.校验通道与板卡是否匹配 */
    if ( e_mvb_cls <= E_MVB_MAX )
    {
        MY_Printf("mvb_resource_map_TX1_Load can_cls parameter is illegal !!!\r\n");
        return e_mvb_id ;
    }

    /* 2.依据资源表配置网络ID*/
    return e_mvb_id;
}

/*******************************************************************************************
 ** @brief: mvb_resource_map_TX1_Child
 ** @param: eth_cls
 *******************************************************************************************/
static E_MVB_ID mvb_resource_map_TX1_Child( E_MVB_CLASS e_mvb_cls )
{
    E_MVB_ID e_mvb_id = E_MVB_ID_MAX;
    /* 1.校验通道与板卡是否匹配 */
    if ( e_mvb_cls <= E_MVB_MAX )
    {
        MY_Printf("mvb_resource_map_TX1_Child can_cls parameter is illegal !!!\r\n");
        return e_mvb_id ;
    }

    /* 2.依据资源表配置网络ID*/
    return e_mvb_id;
}

/*******************************************************************************************
 ** @brief: eth_resource_map_TX2_Load
 ** @param: eth_cls
 *******************************************************************************************/
static E_MVB_ID mvb_resource_map_TX2_Load( E_MVB_CLASS e_mvb_cls )
{
    E_MVB_ID e_mvb_id = E_MVB_ID_MAX;
    /* 1.校验通道与板卡是否匹配 */
    if ( e_mvb_cls != E_MVB_EX1 )
    {
        MY_Printf("mvb_resource_map_TX2_Load e_mvb_cls parameter is illegal !!!\r\n");
        return e_mvb_id ;
    }

    /* 2.依据资源表配置CANID */
    switch (e_mvb_cls)
    {
        case E_MVB_EX1:e_mvb_id = E_MVB_ID_1;break;
        default: e_mvb_id = E_MVB_ID_MAX;break;
    }

    return e_mvb_id;
}

/*******************************************************************************************
 ** @brief: eth_resource_map_TX2_Child
 ** @param: eth_cls
 *******************************************************************************************/
static E_MVB_ID mvb_resource_map_TX2_Child( E_MVB_CLASS e_mvb_cls )
{
    E_MVB_ID e_mvb_id = E_MVB_ID_MAX;
    /* 1.校验通道与板卡是否匹配 */
    if ( e_mvb_cls <= E_MVB_MAX )
    {
        MY_Printf("mvb_resource_map_TX2_Child can_cls parameter is illegal !!!\r\n");
        return e_mvb_id ;
    }

    /* 2.依据资源表配置网络ID*/
    return e_mvb_id;
}

/*******************************************************************************************
 ** @brief: eth_get_resource
 ** @param: eth_cls
 *******************************************************************************************/
static E_MVB_ID mvb_get_resource(E_MVB_CLASS e_mvb_cls)
{
    E_MVB_ID e_mvb_id = E_MVB_ID_MAX;
    E_BOARD_ID BoardId = ID_NONE;

    /* 1.获取板载ID */
    BoardId = support_gpio_getBoardId();

    /* 2. 获取支持层网络ID*/
    switch( BoardId )
    {
    case ID_TX1_Load:
        e_mvb_id = mvb_resource_map_TX1_Load( e_mvb_cls );
        break;
    case ID_TX1_Child:
        e_mvb_id = mvb_resource_map_TX1_Child( e_mvb_cls );
        break;
    case ID_TX2_Load:
        e_mvb_id = mvb_resource_map_TX2_Load( e_mvb_cls );
        break;
    case ID_TX2_Child:
        e_mvb_id = mvb_resource_map_TX2_Child( e_mvb_cls );
        break;
    default:
        MY_Printf("mvb_get_resource BoardId is NONE\r\n");
        e_mvb_id = E_MVB_ID_MAX;
        break;
    }
    return e_mvb_id;
}


/*******************************************************************************************
 ** @brief: devLib_rs485_sendData
 ** @param: e_rs485_cls,*pframe
 *******************************************************************************************/
extern BOOL devlib_mvb_sendData(E_MVB_CLASS e_mvb_cls, S_MVB_FRAME *pframe)
{
    BOOL retFlg = FALSE;
    E_MVB_ID e_mvb_id = E_MVB_ID_MAX;

    /* 1.参数合法性检查 */
    if(( NULL == pframe ) || ( 0U == pframe->len ) )
    {
        //MY_Printf("devlib_mvb_sendData formal parameter is illegal !!!");
        return FALSE;
    }

    /* 2.获取硬件资源 */
    e_mvb_id = mvb_get_resource( e_mvb_cls );

    /* 5.发送数据 */
    if( E_MVB_OK == support_mvb_sendData( e_mvb_id, pframe ))
    {
        retFlg = TRUE;
    }

    return retFlg;
}

/*******************************************************************************************
 ** @brief: devLib_rs485_getData
 ** @param: e_rs485_cls,*pframe
 *******************************************************************************************/
extern BOOL devlib_mvb_getData(E_MVB_CLASS e_mvb_cls, S_MVB_FRAME *pframe)
{
    BOOL retFlg = FALSE;
    E_MVB_ID e_mvb_id = E_MVB_ID_MAX;

    /* 1.参数合法性检查 */
    if( NULL == pframe )
    {
        //MY_Printf("devLib_rs485_getData formal parameter is illegal !!!\r\n");
        return retFlg;
    }

    /* 2.获取硬件资源 */
    e_mvb_id = mvb_get_resource( e_mvb_cls );

    /* 5.发送数据 */
    if( E_MVB_OK == supprot_mvb_getData( e_mvb_id, pframe ))
    {
        retFlg = TRUE;
    }

    return retFlg;
}

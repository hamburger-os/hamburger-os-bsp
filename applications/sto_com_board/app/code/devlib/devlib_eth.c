/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : app_main.c
 **@author: Created By jiaqx
 **@date  : 2023.11.02
 **@brief : app_main function
 ********************************************************************************************/

#include "support_gpio.h"
#include "devlib_eth.h"

/*******************************************************************************************
 ** @brief: eth_resource_map_TX1_Load
 ** @param: eth_cls
 *******************************************************************************************/
static E_ETH_ID eth_resource_map_TX1_Load(E_ETH_CLASS eth_cls )
{
    E_ETH_ID e_eth_id = E_ETH_ID_MAX;

    /* 依据资源表配置网络ID*/
    switch (eth_cls)
    {
        case E_ETH_ZK :e_eth_id = E_ETH_ID_3;break;
        case E_ETH_EX1 :e_eth_id = E_ETH_ID_1;break;
        case E_ETH_EX2 :e_eth_id = E_ETH_ID_2;break;
        default: e_eth_id = E_ETH_ID_MAX;break;
    }

    return e_eth_id;
}

/*******************************************************************************************
 ** @brief: eth_resource_map_TX1_Child
 ** @param: eth_cls
 *******************************************************************************************/
static E_ETH_ID eth_resource_map_TX1_Child(E_ETH_CLASS eth_cls )
{
    E_ETH_ID e_eth_id = E_ETH_ID_MAX;

    /* 依据资源表配置网络ID*/
    switch (eth_cls)
    {
        case E_ETH_ZK :e_eth_id = E_ETH_ID_3;break;
        case E_ETH_EX1 :e_eth_id = E_ETH_ID_1;break;
        case E_ETH_EX2 :e_eth_id = E_ETH_ID_2;break;
        default: e_eth_id = E_ETH_ID_MAX;break;
    }

    return e_eth_id;
}

/*******************************************************************************************
 ** @brief: eth_resource_map_TX2_Load
 ** @param: eth_cls
 *******************************************************************************************/
static E_ETH_ID eth_resource_map_TX2_Load(E_ETH_CLASS eth_cls )
{
    E_ETH_ID e_eth_id;

    /* 依据资源表配置网络ID*/
    switch (eth_cls)
    {
        case E_ETH_ZK :e_eth_id = E_ETH_ID_2;break;
        case E_ETH_EX1 :e_eth_id = E_ETH_ID_1;break;
        default: e_eth_id = E_ETH_ID_MAX;break;
    }

    return e_eth_id;
}

/*******************************************************************************************
 ** @brief: eth_resource_map_TX2_Child
 ** @param: eth_cls
 *******************************************************************************************/
static E_ETH_ID eth_resource_map_TX2_Child(E_ETH_CLASS eth_cls )
{
    E_ETH_ID e_eth_id;

    /* 依据资源表配置网络ID*/
    switch (eth_cls)
    {
    case E_ETH_ZK :e_eth_id = E_ETH_ID_3;break;
    case E_ETH_EX1 :e_eth_id = E_ETH_ID_1;break;
    case E_ETH_EX2 :e_eth_id = E_ETH_ID_2;break;
    default: e_eth_id = E_ETH_ID_MAX;break;
    }

    return e_eth_id;
}

/*******************************************************************************************
 ** @brief: eth_get_resource
 ** @param: eth_cls
 *******************************************************************************************/
static E_ETH_ID eth_get_resource( E_ETH_CLASS eth_cls )
{
    E_ETH_ID e_eth_id = E_ETH_ID_MAX ;
    E_BOARD_ID BoardId = ID_NONE;

    /* 1.获取板载ID */
    BoardId = support_gpio_getBoardId();

    /* 2. 获取支持层网络ID*/
    switch( BoardId )
    {
    case ID_TX1_Load:
        e_eth_id = eth_resource_map_TX1_Load( eth_cls );
        break;
    case ID_TX1_Child:
        e_eth_id = eth_resource_map_TX1_Child( eth_cls );
        break;
    case ID_TX2_Load:
        e_eth_id = eth_resource_map_TX2_Load( eth_cls );
        break;
    case ID_TX2_Child:
        e_eth_id = eth_resource_map_TX2_Child( eth_cls );
        break;
    default:
        MY_Printf("eth_get_resource BoardId is NONE\r\n");
        e_eth_id = E_ETH_ID_MAX;
        break;
    }
    return e_eth_id;
}

/*******************************************************************************************
 ** @brief: devLib_eth_sendData
 ** @param: eth_cls,*pdata,len
 *******************************************************************************************/
extern BOOL devLib_eth_sendData( E_ETH_CLASS eth_cls, uint8 *pdata, uint16 len )
{
    BOOL retFlg = FALSE;
    E_ETH_ID e_eth_id;

    /* 1.参数合法性检查 */
    if(( NULL == pdata ) || ( 0U == len ) || ( len > ETH_FRAME_SIZE_MAX))
    {
        MY_Printf("devLib_eth_sendData formal parameter is illegal !!!");
        return FALSE;
    }

    /* 2.获取硬件资源 */
    e_eth_id = eth_get_resource( eth_cls );

    //MY_Printf("e_eth_id %d !!!",e_eth_id);
    /* 3.发送数据 */
    if( E_ETH_OK == support_eth_sendData( e_eth_id, pdata, len ))
    {
        retFlg = TRUE;
    }

    return retFlg;
}

/*******************************************************************************************
 ** @brief: devLib_eth_getData
 ** @param: eth_cls,*pframe
 *******************************************************************************************/
extern BOOL devLib_eth_getData( E_ETH_CLASS eth_cls, S_ETH_FRAME *pframe )
{
    BOOL retFlg = FALSE;
    E_ETH_ID e_eth_id = E_ETH_ID_MAX;

    /* 1.参数合法性检查 */
    if (NULL == pframe)
    {
        MY_Printf("devLib_eth_getData formal parameter is illegal !!!");
        return FALSE;
    }

    /* 2.获取硬件资源 */
    e_eth_id = eth_get_resource( eth_cls );

    /* 3.发送数据 */
    if( E_ETH_OK == supprot_eth_getData( e_eth_id, pframe ))
    {
        retFlg = TRUE;
    }
    else
    {
        retFlg = FALSE;
    }

    return retFlg;
}

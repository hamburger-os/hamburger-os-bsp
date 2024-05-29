/***************************************************************************
#include <text_app/protocol/include/rec_mag_process_EX.h>
文件名：vcp_time_manage.c
模  块：解析外部通道数据，放置相应队列中，主要隔离通道与通道ID的关系，利用映射表进行隔离。
详  述：
***************************************************************************/

#include "support_libList.h"
#include "support_libPub.h"
#include "support_gpio.h"
#include "list_manage.h"
#include "receive_external_protocol.h"

/*******************************************************************************************
 ** @brief: get_channelId
 ** @param: e_ex_cls
 *******************************************************************************************/
static uint8 get_channelId( E_EX_CLASS e_ex_cls )
{
    const uint8 channeliD_MapTable[][4] =
    {
            /*通信1底板                                       通信1子版                                                 通信2底板                                      通信2子版                   */
            /* E_EX_CAN_EX1 */
            {DATA_CHANNEL_TX1CAN3  , DATA_CHANNEL_TX1CAN1  , DATA_CHANNEL_TX2CAN3, DATA_CHANNEL_TX2CAN1  },
            /* E_EX_CAN_EX2 */
            {DATA_CHANNEL_TX1CAN4  , DATA_CHANNEL_TX1CAN2  , DATA_CHANNEL_TX2CAN4, DATA_CHANNEL_TX2CAN2  },
            /* E_EX_CAN_EX3 */
            {0U                    , DATA_CHANNEL_TX1CAN5  , 0U                  , DATA_CHANNEL_TX2CAN5  },
            /* E_EX_ETH_EX1 */
            {DATA_CHANNEL_TX1ETH1  , DATA_CHANNEL_TX1ETH2  , DATA_CHANNEL_TX2ETH1, DATA_CHANNEL_TX2ETH2  },
            /* E_EX_ETH_EX2 */
            {DATA_CHANNEL_TX1ETH3  , DATA_CHANNEL_TX1ETH4  , 0U                  , 0U                    },
            /* E_EX_RS485_EX1 */
            {DATA_CHANNEL_TX1RS485b, DATA_CHANNEL_TX1RS485a, 0U                  , DATA_CHANNEL_TX2RS485a},
            /* E_EX_HDLC_EX1 */
            {0U                    , 0U                    , DATA_CHANNEL_TX2HDLC, 0U                    },
            /* E_EX_MVB_EX1 */
            {0U                    , 0U                    , DATA_CHANNEL_TX2MVB , 0U                    }
    };

    uint8 channel_id = 0U;
    E_BOARD_ID BoardId = ID_NONE;

    /* 1.获取板载ID */
    BoardId = support_gpio_getBoardId() ;

    /* 2.检查资源映射表参数合法性 */
    if((BoardId - 1) > ID_TX2_Child)
    {
        MY_Printf("channeliD_MapTable BoardId-1 is illegal !!!\r\n");
        return channel_id;
    }

    if(e_ex_cls >= E_EX_MAX)
    {
        MY_Printf("channeliD_MapTable e_ex_cls is illegal !!!\r\n");
        return channel_id;
    }

    /* 3.获取通道ID*/
    channel_id = channeliD_MapTable[e_ex_cls][BoardId-1];

    return channel_id;
}

/*******************************************************************************************
 ** @brief: rec_msg_external_ETH_proc
 ** @param: e_ex_cls *pdata
 *******************************************************************************************/
static void rec_msg_external_CAN_proc( E_EX_CLASS e_ex_cls, uint8 *pdata )
{
    uint8 channel_id = 0U;
    S_ArrayList *pList = NULL;

    /* 1.检查参数合法性 */
    if( NULL == pdata )
    {
        MY_Printf("rec_msg_EX_CAN_proc pApp is NULL !!!\r\n");
        return ;
    }

    if( e_ex_cls > E_EX_CAN_EX3 )
    {
        MY_Printf("rec_msg_EX_CAN_proc e_ex_cls is err e_ex_cls: %d !!!\r\n",e_ex_cls);
        return ;
    }

    /* 2.数据处理 */
    /* 2.1 获取通道号 */
    channel_id = get_channelId( e_ex_cls );

    /* 2.2 获取队列 */
    pList = get_EX_list( channel_id );

    /* 2.3 数据加入队列 */
    if( NULL != pList )
    {
        support_arraylistAdd( pList, pdata, sizeof( S_CAN_FRAME_DATA ));

    }
    else
    {
        MY_Printf("rec_msg_external_CAN_proc pList is NULL !!!\r\n");
        MY_Printf("channel_id: 0x%.2x !!!\r\n",channel_id);
    }

}

/*******************************************************************************************
 ** @brief: rec_msg_external_ETH_proc
 ** @param: e_ex_cls *pdata
 *******************************************************************************************/
static void rec_msg_external_ETH_proc( E_EX_CLASS e_ex_cls, uint8 *pdata )
{
    uint8 channel_id = 0U;
    S_ArrayList *pList = NULL;

    /* 1.检查参数合法性 */
    if( NULL == pdata )
    {
        MY_Printf("rec_msg_EX_proc pApp is NULL !!!\r\n");
        return ;
    }

    if( e_ex_cls >  E_EX_CAN_EX3 )
    {
        MY_Printf("rec_msg_EX_proc e_ex_cls is err !!!\r\n");
        return ;
    }

    /* 2.数据处理 */
    /* 2.1 获取通道号 */
    channel_id = get_channelId( e_ex_cls );

    /* 2.2 获取队列 */
    pList = get_EX_list( channel_id );

    /* 2.3 数据加入队列 */
    if( NULL != pList )
    {
        support_arraylistAdd( pList, pdata, sizeof( S_ETH_FRAME ));
    }
    else
    {
        MY_Printf("rec_msg_external_ETH_proc pList is NULL !!!\r\n");
        MY_Printf("channel_id: 0x%.2x !!!\r\n",channel_id);
    }

}

/*******************************************************************************************
 ** @brief: rec_msg_external_ETH_proc
 ** @param: e_ex_cls *pdata
 *******************************************************************************************/
extern void rec_msg_external_proc( E_EX_CLASS e_ex_cls, uint8 *pdata )
{
    /* 1.检查参数合法性 */
    if( NULL == pdata )
    {
        MY_Printf("rec_msg_EX_proc pApp is NULL !!!\r\n");
        return ;
    }

    /* 2.数据处理 */
    if( e_ex_cls <=  E_EX_CAN_EX3 )
    {
        rec_msg_external_CAN_proc(e_ex_cls, pdata);
    }

    else
    {
        rec_msg_external_ETH_proc(e_ex_cls, pdata);
    }

}

/*******************************************************************************************
 ** @brief: getListData_CANtype_external
 ** @param: type, *p_can_frame
 *******************************************************************************************/
extern BOOL getListData_CANtype_external( E_CAN_TYPE type, S_CAN_FRAME_DATA *p_can_frame )
{
    uint8 channel_id = 0U;
    S_ArrayList *p_can_list = NULL;
    BOOL retState = FALSE;
    uint16 len = 0;

    /* 1.获取CAN数据队列 */
    /* 1.1.获取通道ID */
    channel_id = get_channelIdByCanType( type );

    /* 1.2.获取队列 */
    p_can_list = get_EX_list( channel_id );

    /* 2.获取CAN数据 */
    if( support_arraylistGet( p_can_list, (uint8 *)p_can_frame, &len, sizeof( S_CAN_FRAME_DATA )) > 0 )
    {
        retState = TRUE;
    }

    return retState;
}

/*******************************************************************************************
 ** @brief: getListData_ETHtype_external
 ** @param: type, p_eth_frame
 *******************************************************************************************/
extern BOOL getListData_ETHtype_external( E_ETH_TYPE type, S_ETH_FRAME *p_eth_frame )
{
    uint8 channel_id = 0U;
    S_ArrayList *p_eth_list = NULL;
    BOOL retState = FALSE;
    uint16 len = 0U;

    /* 1.获取ETH数据队列 */
    /* 1.1.获取通道ID */
    channel_id = get_channelIdByEthType( type );

    /* 1.2.获取队列 */
    p_eth_list = get_EX_list( channel_id );

    /* 2.获取ETH数据 */
    if( support_arraylistGet( p_eth_list, (uint8 *)p_eth_frame, &len, sizeof( S_ETH_FRAME )) > 0 )
    {
        retState = TRUE;
    }

    return retState;
}


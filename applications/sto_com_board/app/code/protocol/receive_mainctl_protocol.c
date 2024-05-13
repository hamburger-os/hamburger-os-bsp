/***************************************************************************
#include <text_app/protocol/include/rec_mag_process_ZK.h>
文件名：vcp_time_manage.c
模  块：解析主控发送的周期数据，放置相应队列中，隔离主控下发通信板卡程序。
详  述：
***************************************************************************/
#include "support_libPub.h"
#include "list_manage.h"
#include "receive_mainctl_protocol.h"

/*******************************************************************************************
 ** @brief: canData_proc
 ** @param: pdata
 *******************************************************************************************/
static void canData_proc( uint8 channel_id, S_CHANNEL_CAN *pdata )
{
    S_CAN_FRAME_DATA canData = { 0U };
    S_ArrayList *pList = NULL;

    /* 1.参数合法性检查 */
    /* 1.1.CAN帧pdata检查 */
    if( NULL == pdata )
    {
        MY_Printf("canData_proc error==> pdata is NULL!!!\r\n");
        return;
    }
    /* 1.2.CAN通道号channel_id检查 */
    if(( channel_id < DATA_CHANNEL_TX1CAN1 ) || ( channel_id > DATA_CHANNEL_TX2VMCAN2 ))
    {
        MY_Printf("canData_proc error==> channel_id is invalid!!!\r\n");
        return;
    }

    /* 2.CAN帧数据解析 */
    if( pdata->len_u8 <= 8U )
    {
        /* 2.1.组织CAN_DATA数据帧 */
        canData.priority_u8 = ( uint8 )(( pdata->id_u32 >> 3U ) & 0xFF );
        canData.no_u8 = ( uint8 )(( pdata->id_u32 & 0x07 ) & 0xFF );
        canData.length_u8 =  pdata->len_u8;
        support_memcpy( &canData.data_u8[0U], &pdata->pdata_u8[0U], pdata->len_u8 );

#if 0
        /* 调试信息 */
        if ((canData.priority_u8 == 0x8B)||(canData.priority_u8 == 0x8C) || (canData.priority_u8 == 0x2E) )
        {
            MY_Printf("canData.priority_u8: %x !!!\r\n",canData.priority_u8);
            for(uint8 i = 0; i<8;i++)
            {
                MY_Printf("%.2x ",canData.data_u8[i]);
            }
            MY_Printf("\r\n");
        }
#endif

        /* 2.2.根据通道ID将数据插入到对应队列 */
        pList = get_ZK_list( channel_id );
        if( NULL != pList )
        {
            support_arraylistAdd( pList, (uint8 *)&canData, sizeof( S_CAN_FRAME_DATA ));
            //MY_Printf("channel_id: 0x%.2x !!!\r\n",channel_id);
            //MY_Printf("pList is ok !!!\r\n");
        }
        else
        {
            MY_Printf("canData_proc pList is NULL !!!\r\n");
            MY_Printf("channel_id: 0x%.2x !!!\r\n",channel_id);
        }

    }
}

/*******************************************************************************************
 ** @brief: canData_proc
 ** @param: channel_id, pdata
 *******************************************************************************************/
static void channel_canframe_proc( uint8 channel_id, uint8 *pdata, uint16 len )
{
    S_CHANNEL_FRAME *p_channelFrame = NULL;
    S_CHANNEL_CAN *p_canFrame = NULL;
    uint16 index = 0U;
    uint8 num = 0U;
    //uint32 *p_recNum = NULL;

    /* 1.参数合法性检查 */
    /* 1.1.CAN帧pdata检查 */
    if( NULL == pdata )
    {
        MY_Printf("channel_canframe_proc error==> pdata is NULL!!!\r\n");
        return;
    }
    /* 1.2.CAN通道号channel_id检查 */
    if(( channel_id < DATA_CHANNEL_TX1CAN1 ) || ( channel_id > DATA_CHANNEL_TX2VMCAN2 ))
    {
        MY_Printf("channel_canframe_proc error==> channel_id is invalid!!!\r\n");
        return;
    }

    /* 2.获取CAN通道数据帧 */
    p_channelFrame = ( S_CHANNEL_FRAME *)pdata;

    /* 3.输出CAN通道数据帧的基本信息 */
#if 0
    p_recNum = get_recTotalNum( channel_id );
    *p_recNum += p_channelFrame->totalFrameNum_u8;

    MY_Printf("channel_canframe_proc==> channel_id:0x%02x totalNum:%d allNum:%d\r\n", channel_id, p_channelFrame->totalFrameNum_u8, p_channelFrame->frameAllNum_u32 );
    MY_Printf("ch_can==>id:0x%02x totalNum:%d SallNum:%d RallNum:%d err:%d\r\n", channel_id, p_channelFrame->totalFrameNum_u8, p_channelFrame->frameAllNum_u32, *p_recNum, p_channelFrame->frameAllNum_u32 - *p_recNum );
#endif

    /* 4.解析CAN通道数据帧的详细CAN帧内容 */
    for( num = 0U; num < p_channelFrame->totalFrameNum_u8; num++ )
    {
        /* 4.1.获取CAN帧数据块 */
        p_canFrame = ( S_CHANNEL_CAN *)&p_channelFrame->pdata_u8[index];

        /* 4.2.处理CAN帧数据块 */
        canData_proc( channel_id, p_canFrame );

        /* 4.3.下一个CAN帧数据位置索引 */
        index += p_canFrame->len_u8 + 5U;
    }
}

/*******************************************************************************************
 ** @brief: channel_ethframe_proc
 ** @param: channel_id, pdata, len
 *******************************************************************************************/
static void channel_ethframe_proc( uint8 channel_id, uint8 *pdata, uint16 len )
{
    S_CHANNEL_FRAME *p_channelFrame = NULL;
    S_CHANNEL_ETH *p_ethFrame = NULL;
    S_ArrayList *pList = NULL;
    S_ETH_FRAME s_eth_frame = { 0U };
    uint16 index = 0U;
    uint8 num = 0U;
    //uint32 *p_recNum = NULL;

    /* 1.参数合法性检查 */
    /* 1.1.ETH帧内容检查 */
    if( NULL == pdata )
    {
        MY_Printf("channel_ethframe_proc error==> pdata is NULL!!!\r\n");
        return;
    }

    /* 1.2.ETH通道号channel_id检查 */
    if(( channel_id > DATA_CHANNEL_TX1ETH1 )\
        &&( channel_id != DATA_CHANNEL_TX1ETH2 )\
        &&( channel_id != DATA_CHANNEL_TX1ETH3 )\
        &&( channel_id != DATA_CHANNEL_TX1ETH4 )\
        &&( channel_id != DATA_CHANNEL_TX2ETH1 )\
        &&( channel_id != DATA_CHANNEL_TX2ETH2 )\
        &&( channel_id != DATA_CHANNEL_TX2HDLC )\
        &&( channel_id != DATA_CHANNEL_TX2MVB )\
        &&( channel_id != DATA_CHANNEL_TX1RS485a )\
        &&( channel_id != DATA_CHANNEL_TX1RS485b )\
        &&( channel_id != DATA_CHANNEL_TX2RS485a ))
    {
        MY_Printf("channel_ethframe_proc error==> channel_id is invalid ID:0x%.2x !!!\r\n",channel_id);
        return;
    }

    /* 2.获取ETH通道数据帧 */
    p_channelFrame = ( S_CHANNEL_FRAME *)pdata;

    /* 3.输出ETH通道数据帧的基本信息 */
#if 0
    /* 后续添加 */
    p_recNum = get_recTotalNum( channel_id );
    *p_recNum += p_channelFrame->totalFrameNum_u8;
    MY_Printf("channel_ethframe_proc==> channel_id:0x%02x totalNum:%d allNum:%d\r\n", channel_id, p_channelFrame->totalFrameNum_u8, p_channelFrame->frameAllNum_u32 );
    MY_Printf("ch_eth==>id:0x%02x totalNum:%d SallNum:%d RallNum:%d err:%d\r\n", channel_id, p_channelFrame->totalFrameNum_u8, p_channelFrame->frameAllNum_u32, *p_recNum, p_channelFrame->frameAllNum_u32 - *p_recNum );
#endif

    /* 4.解析ETH数据帧*/
    for( num = 0U; num < p_channelFrame->totalFrameNum_u8; num++ )
    {
        /* 4.1.获取ETH帧数据块 */
        p_ethFrame = ( S_CHANNEL_ETH *)&p_channelFrame->pdata_u8[index];

        /* 4.2.处理ETH帧数据块 */
        s_eth_frame.len += p_ethFrame->len_u16;
        support_memcpy(&s_eth_frame.data_u8[0], p_ethFrame, p_ethFrame->len_u16);

        /* 4.3.下一个ETH帧数据位置索引 */
        index += p_ethFrame->len_u16 + 2U;
    }

    /* 5.解析完成的数据相应队列中 */
    pList = get_ZK_list( channel_id );
    if( NULL != pList )
    {
        support_arraylistAdd( pList, (uint8 *)&s_eth_frame, sizeof( S_ETH_FRAME ));
    }
    else
    {
        MY_Printf("channel_ethframe_proc pList is NULL !!!\r\n");
        MY_Printf("channel_id: 0x%.2x !!!\r\n",channel_id);
    }


}

/*******************************************************************************************
 ** @brief: channel_data_proc
 ** @param: pdata
 *******************************************************************************************/
static void channel_data_proc( S_CHANNEL_DATA *pdata )
{
    /* 1.参数合法性检查 */
    if( NULL == pdata )
    {
        MY_Printf("channel_data_proc error==> pdata is NULL!!!\r\n");
        return;
    }

    /* 2.通道数据处理 */
    switch( pdata->channel_id_u8 )
    {
    /* 2.1.CAN通道数据处理 */
    case DATA_CHANNEL_TX1CAN1:
    case DATA_CHANNEL_TX1CAN2:
    case DATA_CHANNEL_TX1CAN3:
    case DATA_CHANNEL_TX1CAN4:
    case DATA_CHANNEL_TX1CAN5:
    case DATA_CHANNEL_TX1VMCAN1:
    case DATA_CHANNEL_TX1VMCAN2:
    case DATA_CHANNEL_TX2CAN1:
    case DATA_CHANNEL_TX2CAN2:
    case DATA_CHANNEL_TX2CAN3:
    case DATA_CHANNEL_TX2CAN4:
    case DATA_CHANNEL_TX2CAN5:
    case DATA_CHANNEL_TX2VMCAN1:
    case DATA_CHANNEL_TX2VMCAN2:
         channel_canframe_proc( pdata->channel_id_u8, pdata->pdata_u8, pdata->len_u16 );
         break;

    /* 2.2.ETH通道数据处理 */
    case DATA_CHANNEL_TX1ETH1:
    case DATA_CHANNEL_TX1ETH2:
    case DATA_CHANNEL_TX1ETH3:
    case DATA_CHANNEL_TX1ETH4:
    case DATA_CHANNEL_TX2ETH1:
    case DATA_CHANNEL_TX2ETH2:
    case DATA_CHANNEL_TX2HDLC:
    case DATA_CHANNEL_TX2MVB:
    case DATA_CHANNEL_TX1RS485a:
    case DATA_CHANNEL_TX1RS485b:
    case DATA_CHANNEL_TX2RS485a:
        channel_ethframe_proc( pdata->channel_id_u8, pdata->pdata_u8, pdata->len_u16 );
        break;
    default:
        MY_Printf("channel_data_proc channel_id_u8 is err !!!\r\n");
        break;
    }
}

/*******************************************************************************************
 ** @brief: msg_data_proc
 ** @param: pdata
 *******************************************************************************************/
extern void rec_msg_mainctl_proc( uint8 *pdata )
{
    S_CHANNEL_DATA *p_channelData = NULL;
    uint16 index = 0U;
    uint8 num = 0U;
    S_MSG_DATA *p_msg_data = NULL;
    /* 1.参数合法性检查 */
    if( NULL == pdata )
    {
        MY_Printf("msg_data_proc error==> pdata is NULL!!!\r\n");
        return;
    }

    p_msg_data = (S_MSG_DATA *)pdata;

    /* 2.报文数据内容解析 */
    for( num = 0U; num < p_msg_data->channel_num_u8; num++ )
    {
        /* 2.1.获取通道数据块 */
        p_channelData = ( S_CHANNEL_DATA *)&p_msg_data->pdata_u8[index];

        /* 2.2.通道数据块处理 */
        channel_data_proc( p_channelData );

        /* 2.3.下一个通道数据块位置索引 */
        index += p_channelData->len_u16 + 7U;
    }
}


/*******************************************************************************************
 ** @brief: comProtocol_getCanData
 ** @param: type, id, p_can_frame
 *******************************************************************************************/
extern BOOL getListData_CANtype_mainctl( E_CAN_TYPE type, S_CAN_FRAME_DATA *p_can_frame )
{
    uint8 channel_id = 0U;
    S_ArrayList *p_can_list = NULL;
    BOOL retState = FALSE;
    uint16 len = 0;

    /* 1.获取CAN数据队列 */
    /* 1.1.获取通道ID */
    channel_id = get_channelIdByCanType( type );

    /* 1.2.获取队列 */
    p_can_list = get_ZK_list( channel_id );

    /* 2.获取CAN数据 */
    if( support_arraylistGet( p_can_list, (uint8 *)p_can_frame, &len, sizeof( S_CAN_FRAME_DATA )) > 0 )
    {
        retState = TRUE;
    }

    return retState;
}

/*******************************************************************************************
 ** @brief: comProtocol_getETHData
 ** @param: type, p_eth_frame
 *******************************************************************************************/
extern BOOL getListData_ETHtype_mainctl( E_ETH_TYPE e_eth_type, S_ETH_FRAME *p_eth_frame )
{
    uint8 channel_id = 0U;
    S_ArrayList *p_eth_list = NULL;
    BOOL retState = FALSE;
    uint16 len = 0U;

    /* 1.获取ETH数据队列 */
    /* 1.1.获取通道ID */
    channel_id = get_channelIdByEthType( e_eth_type );

    /* 1.2.获取队列 */
    p_eth_list = get_ZK_list( channel_id );

    /* 2.获取ETH数据 */
    if( support_arraylistGet( p_eth_list, (uint8 *)p_eth_frame, &len, sizeof( S_ETH_FRAME )) > 0 )
    {
        retState = TRUE;
    }

    return retState;
}


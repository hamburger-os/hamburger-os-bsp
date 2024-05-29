/***************************************************************************
#include <text_app/protocol/include/rec_mag_process_ZK.h>
文件名：vcp_time_manage.c
模  块：解析主控发送的周期数据，放置相应队列中，封装
详  述：
***************************************************************************/

#include "support_libList.h"
#include "support_can.h"
#include "support_eth.h"
#include "support_libPub.h"

#include "receive_external_protocol.h"


/*******************************************************************************************
 ** @brief: channel_canframe_pakage
 ** @param: pdata e_can_type
 *******************************************************************************************/
static void channel_canframe_pakage(uint8* pdata, E_CAN_TYPE e_can_type)
{
    S_CHANNEL_FRAME* p_channel_frame = NULL;
    uint16 p_channel_frame_index = 0U;
    S_CAN_FRAME_DATA s_can_frame_data = {0};
    S_CHANNEL_CAN s_channel_can = {0};

    /* 1.参数合法性检查 */
    if(pdata == NULL)
    {
        MY_Printf("channel_data_pakage pdata is NULL !!!\r\n");
        return ;
    }

    p_channel_frame = (S_CHANNEL_FRAME*) pdata;

    /* 2.组织单通道数据*/
    while( TRUE == getListData_CANtype_external(e_can_type, &s_can_frame_data) )
    {

        /* 2.1 通道通道数据帧总数(统计使用) */
        p_channel_frame->frameAllNum_u32 = 0U;

        /* 2.2 组织通道数据帧数 */
        p_channel_frame->totalFrameNum_u8 ++;

        /* 2.3 组织通道数据帧长度 */
        p_channel_frame->len_u16 += (sizeof(S_CAN_FRAME_DATA) + 2U);

        /* 2.3 CAN帧格式转网络CAN帧格式 */
        s_channel_can.id_u32 = (s_can_frame_data.priority_u8 << 3U ) + s_can_frame_data.no_u8;
        s_channel_can.len_u8 = s_can_frame_data.length_u8;
        support_memcpy( (uint8*)&s_channel_can.pdata_u8[0U], (uint8*)&s_can_frame_data.data_u8[0U], s_can_frame_data.length_u8 );

        /* 2.4 组织通道数据*/
        support_memcpy((uint8*)&p_channel_frame->pdata_u8[p_channel_frame_index], (uint8*)&s_channel_can, (sizeof(S_CAN_FRAME_DATA) + 2U) );

        /* 2.5 维护通道数据末尾下标*/
        p_channel_frame_index = p_channel_frame->len_u16 ;

    }

}

/*******************************************************************************************
 ** @brief: channel_ethframe_pakage
 ** @param: pdata e_eth_type
 *******************************************************************************************/
static void channel_ethframe_pakage(uint8* pdata, E_ETH_TYPE e_eth_type)
{
    S_CHANNEL_FRAME* p_channel_frame = NULL;
    uint16 p_channel_frame_index = 0U;
    S_ETH_FRAME s_eth_frame = {0};

    /* 1.参数合法性检查 */
    if(pdata == NULL)
    {
        MY_Printf("channel_data_pakage pdata is NULL !!!\r\n");
        return ;
    }

    p_channel_frame = (S_CHANNEL_FRAME*) pdata;

    /* 2.组织单通道数据*/
    while( TRUE == getListData_ETHtype_external(e_eth_type, &s_eth_frame) )
    {
        /* 2.1 通道通道数据帧总数(统计使用)*/
        p_channel_frame->totalFrameNum_u8 = 0U;

        /* 2.2 组织通道数据帧数*/
        p_channel_frame->totalFrameNum_u8 ++;

        /* 2.3 组织通道数据帧总长 */
        p_channel_frame->len_u16 += s_eth_frame.len + sizeof(s_eth_frame.len);

        /* 2.4 组织通道数据*/
        support_memcpy((uint8*)&p_channel_frame->pdata_u8[p_channel_frame_index], (uint8*)&s_eth_frame, s_eth_frame.len + sizeof(s_eth_frame.len) );

        /* 2.5 维护通道数据末尾下标*/
        p_channel_frame_index = p_channel_frame->len_u16;

    }

}


/*******************************************************************************************
 ** @brief: channel_frame_pakage
 ** @param: pdata channel_type_num e_channel_type
 *******************************************************************************************/
static void channel_frame_pakage( uint8* pdata, uint8 channel_type_num, E_CHANNEL_TYPE e_channel_type)
{
    S_CHANNEL_DATA* p_channel_data = NULL;
    S_CHANNEL_FRAME* p_channel_frame = NULL;
    uint8 channel_id = 0U;

    /* 1.参数合法性检查 */
    if(pdata == NULL)
    {
        MY_Printf("channel_data_pakage pdata is NULL !!!\r\n");
        return ;
    }

    p_channel_data = (S_CHANNEL_DATA*) pdata;
    p_channel_frame = (S_CHANNEL_FRAME*)p_channel_data->pdata_u8;

    /* 2.组织通道数据(通道总数-最后) */
    if (E_CHANNEL_CAN == e_channel_type)
    {
        channel_canframe_pakage(p_channel_data->pdata_u8, channel_type_num);
    }
    else if ((E_CHANNEL_ETH == e_channel_type))
    {
        channel_ethframe_pakage(p_channel_data->pdata_u8, channel_type_num);
    }
    else
    {
        MY_Printf("channel_data_pakage channel_type is err !!!\r\n");
    }

    /* 3.组织通道负载长度 */
    /* 3.1 通道数据无效 */
    if ( 0U == p_channel_frame->len_u16 )
    {
        p_channel_data->len_u16 = 0U;
    }
    /* 3.2 通道数据有效 */
    else
    {
        p_channel_data->len_u16 = p_channel_frame->len_u16 + 7U ;
    }

    /* 4.组织通道时间戳 */
    p_channel_data->timestemp_u32 = 0U;

    /* 5.组织通道ID */
    /* 5.1 获取通道ID */
    if (E_CHANNEL_CAN == e_channel_type)
    {
        channel_id = get_channelIdByCanType( channel_type_num );
    }
    else if ((E_CHANNEL_ETH == e_channel_type))
    {
        channel_id = get_channelIdByEthType( channel_type_num );
    }
    else
    {
        MY_Printf("channel_data_pakage channel_type is err !!!\r\n");
    }

    /* 5.2 通道ID赋值 */
    p_channel_data->channel_id_u8 = channel_id;
}



/*******************************************************************************************
 ** @brief: channel_data_pakage
 ** @param: pdata p_msgdatalen channel_type_num channel_type
 *******************************************************************************************/
static void channel_data_pakage (uint8* pdata, uint16* p_msgdatalen,uint8 channel_type_num, uint8 channel_type)
{
    S_CHANNEL_DATA* p_channel_data = NULL;

    /* 1.参数合法性检查 */
    if(pdata == NULL)
    {
        MY_Printf("channel_data_pakage pdata is NULL !!!\r\n");
        return ;
    }

    /* 2. 组织单通道数据*/
    channel_frame_pakage(pdata, channel_type_num, channel_type);
    p_channel_data = (S_CHANNEL_DATA*)pdata;

    /* 3.维护单通道数据长度(不包括通道ID数量)*/
    /* 3.1 单通道数据无效 */
    if ( 0U == p_channel_data->len_u16)
    {
        *p_msgdatalen += 0U;
    }
    /* 3.2 单通道数据有效 */
    else
    {
        *p_msgdatalen += (p_channel_data->len_u16 + 7U);
    }

    MY_Printf("msgdatalen :%d !!!\r\n",*p_msgdatalen);

}

/*******************************************************************************************
 ** @brief: mainctl_msg_data_pakage
 ** @param: pdata p_msglen channel_type_num channel_type
 *******************************************************************************************/
extern void mainctl_msg_data_pakage(uint8* pdata, uint16* p_msglen, uint8 channel_type_num, E_CHANNEL_TYPE e_channel_type)
{
    S_MSG_DATA *p_msg_data = NULL;
    S_CHANNEL_DATA  *p_channel_data = NULL;
    static uint16 channel_data_index_last = 0U;
    uint16 msgdatalen = 0U;
    uint16 channel_data_index = 0U;

    /* 1.参数合法性检查 */
    if(pdata == NULL)
    {
        MY_Printf("msg_data_pakage pdata is NULL !!!\r\n");
        return ;
    }
    p_msg_data = (S_MSG_DATA *)pdata;
    p_channel_data = (S_CHANNEL_DATA *)p_msg_data->pdata_u8;

    /* 2.取得通道数据首地址 */
    for(uint8 i = 0U; i < p_msg_data->channel_num_u8; i++)
    {
        p_channel_data = (S_CHANNEL_DATA *)&p_msg_data->pdata_u8[channel_data_index];
        channel_data_index += (p_channel_data->len_u16 + 7U);
        p_channel_data = (S_CHANNEL_DATA *)&p_msg_data->pdata_u8[channel_data_index];
    }

    /* 3.组织数据并获得周期数据内容长度 */
    channel_frame_pakage((uint8*)p_channel_data, channel_type_num, e_channel_type);

    /* 4.更新通道ID数量 */
    if( (0U != p_channel_data->len_u16) &&( (0U ==  channel_data_index) ||  channel_data_index != channel_data_index_last ))
    {
        p_msg_data->channel_num_u8++;
    }

    channel_data_index_last = channel_data_index;


    /* 5.计算周期数据总长度 */
    channel_data_index = 0U;
    for(uint8 i = 0U; i < p_msg_data->channel_num_u8; i++)
    {
        p_channel_data = (S_CHANNEL_DATA *)&p_msg_data->pdata_u8[channel_data_index];
        channel_data_index += (p_channel_data->len_u16 + 7U);
        /* 5.1 通道ID无效 */
        if( 0U == channel_data_index)
        {
            *p_msglen = 0U;
        }

        /* 5.2 通道ID数据有效 */
        else
        {
            *p_msglen = channel_data_index + 1U;
        }
    }

}


/*******************************************************************************************
 ** @brief: mainctl_msg_data_clean
 ** @param: pdata p_msglen channel_type_num channel_type
 *******************************************************************************************/
extern void mainctl_msg_data_clean( void )
{
    /* 1.开辟空间 */
    uint8 temp_data[1500] = {0};

    /* 2.清空主控消息队列 */
    /* 2.1 清空CAN队列数据 */
    for( E_CAN_TYPE channel_num = 0; channel_num < E_CAN_TYPE_MAX; channel_num++ )
    {
        channel_canframe_pakage(&temp_data[0], channel_num);
    }
    /* 2.2 清空ETH队列数据 */
    for( E_ETH_TYPE channel_num = 0; channel_num < E_ETH_TYPE_MAX; channel_num++ )
    {
        channel_ethframe_pakage(&temp_data[0], channel_num);
    }

}


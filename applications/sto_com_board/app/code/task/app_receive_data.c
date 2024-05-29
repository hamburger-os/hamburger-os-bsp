/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : app_main.c
 **@author: Created By jiaqx
 **@date  : 2023.11.02
 **@brief : app_main function
 ********************************************************************************************/

#include "support_timer.h"
#include "devlib_can.h"
#include "support_libPub.h"

#include "devlib_eth.h"
#include "vcp_safe_layer.h"
#include "vcp_app_layer.h"
#include "receive_mainctl_protocol.h"
#include "receive_external_protocol.h"

//#define PRINT_DEBUG
/*******************************************************************************************
 ** @brief: dataCheck_safe_layer
 ** @param: p_eth_frame
 *******************************************************************************************/
static BOOL dataCheck_safe_layer(S_ETH_FRAME * p_eth_frame)
{
    BOOL ret = FALSE;

    S_SAFE_LAYER_HEADER* p_safe_layer_header = NULL;

    /* 1.参数合法性检查 */
    if( p_eth_frame == NULL )
    {
        MY_Printf("mainctl_dataCheck p_eth_frame is NULL !!!\r\n");
        return ret;
    }

    if( 0U == p_eth_frame->len || p_eth_frame->len > ETH_FRAME_SIZE_MAX)
    {
        //MY_Printf("eth data len is illegal, len:%d !!!\r\n",s_eth_frame.len);
        return ret;
    }


    /* 2.VCP安全层校验 */
    if (ERROR == vcp_safe_layer_check( &p_eth_frame->data_u8[0], IN_ETH_DEV) )
    {
        return ret;
    }

    p_safe_layer_header = (S_SAFE_LAYER_HEADER* )&p_eth_frame->data_u8[0];

    /* 3.VCP应用数据处理(暂时仅考虑内部以太网接收) */
    vcp_app_layer_process( (uint8*)&p_eth_frame->data_u8[sizeof(S_SAFE_LAYER_HEADER)], p_safe_layer_header->lenth - sizeof(S_SAFE_LAYER_HEADER));

    ret = TRUE;

    return ret;

}
/*******************************************************************************************
 ** @brief: mainctl_dataCheck
 ** @param: p_eth_frame
 *******************************************************************************************/
static BOOL mainctl_dataCheck(S_ETH_FRAME * p_eth_frame)
{
    BOOL ret = FALSE;

    /* 1.参数合法性检查 */
    if( p_eth_frame == NULL )
    {
        MY_Printf("mainctl_dataCheck p_eth_frame is NULL !!!\r\n");
        return ret;
    }

    if( (0U == p_eth_frame->len) || (p_eth_frame->len > ETH_FRAME_SIZE_MAX) )
    {
        return ret;
    }

    //MY_Printf(" mainctl_dataCheck eth len:%d !!!\r\n",p_eth_frame->len);

#ifdef PRINT_DEBUG
    if( 0U != p_eth_frame->len )
    {

        MY_Printf("mainctl eth data len is legal, len:%d !!!\r\n",p_eth_frame->len);
        MY_Printf("data:");
        for(uint16 i=0;i<p_eth_frame->len;i++)
        {
            MY_Printf("%.2x ",p_eth_frame->data_u8[i]);
        }
        MY_Printf("\r\n");
    }
#endif

    /* 2.安全层检查与处理 */
    if( FALSE == dataCheck_safe_layer(p_eth_frame)  )
    {
        MY_Printf("dataCheck_safe_layer is err !!!\r\n");
        return ret;
    }

    ret = TRUE;
    return ret;

}


/*******************************************************************************************
 ** @brief: receive_mainctl_data_proc
 ** @param: null
 *******************************************************************************************/
static void receive_mainctl_data_proc( void )
{
    S_ETH_FRAME s_eth_frame = {0};

    /* 1.获取数据 */
    if ( FALSE == devLib_eth_getData( E_ETH_ZK, &s_eth_frame))
    {
        //MY_Printf("devLib_eth_getData is FALSE\r\n");
        return;
    }

    /* 2.数据合法性检查 */
    if ( FALSE == mainctl_dataCheck( &s_eth_frame ) )
    {
        //MY_Printf("mainctl_dataCheck is FALSE\r\n");
        return;
    }

    /* 3.周期数据处理 */
    /* 暂定在应用层周期处理函数中 */

}


/*******************************************************************************************
 ** @brief: receive_ZKdata_proc
 ** @param: null
 *******************************************************************************************/
static void receive_external_data_proc( void )
{
    /* 1.开辟空间 */
    S_ETH_FRAME s_eth_frame;
    S_CAN_FRAME_DATA s_can_frame_data;

    uint8* pdatatemp = NULL;
    /* 2.接收外部CAN数据 */
    /* 2.1 获取外部CAN1数据 */
    if ( TRUE == devLib_can_getData( E_CAN_EX1, &s_can_frame_data))
    {
#if 0
        pdatatemp = (uint8*)&s_can_frame_data;
        MY_Printf("receive_external_data_proc1 data len is legal, len:%d !!!\r\n",sizeof(S_CAN_FRAME_DATA));
        MY_Printf("data:");
        for(uint16 i=0;i<sizeof(S_CAN_FRAME_DATA);i++)
        {

            MY_Printf("%.2x ",pdatatemp[i]);
        }
        MY_Printf("\r\n");
#endif

        if ( 0U != s_can_frame_data.length_u8)
        {
            rec_msg_external_proc(E_EX_CAN_EX1, (uint8*)&s_can_frame_data);
            support_memset(&s_can_frame_data, 0U, sizeof(S_CAN_FRAME_DATA));
        }
    }

    /* 2.2 获取外部CAN2数据 */
    if ( TRUE == devLib_can_getData( E_CAN_EX2, &s_can_frame_data))
    {
#if 0
        pdatatemp = (uint8*)&s_can_frame_data;
        MY_Printf("receive_external_data_proc2 data len is legal, len:%d !!!\r\n",sizeof(S_CAN_FRAME_DATA));
        MY_Printf("data:");
        for(uint16 i=0;i<sizeof(S_CAN_FRAME_DATA);i++)
        {

            MY_Printf("%.2x ",pdatatemp[i]);
        }
        MY_Printf("\r\n");
#endif
        if ( 0U != s_can_frame_data.length_u8)
        {
            rec_msg_external_proc(E_EX_CAN_EX2, (uint8*)&s_can_frame_data);
            support_memset(&s_can_frame_data, 0U, sizeof(S_CAN_FRAME_DATA));
        }
    }

    /* 2.3 获取外部CAN3数据 */
    if ( TRUE == devLib_can_getData( E_CAN_EX3, &s_can_frame_data))
    {
#if 0
        pdatatemp = (uint8*)&s_can_frame_data;
        MY_Printf("receive_external_data_proc3 data len is legal, len:%d !!!\r\n",sizeof(S_CAN_FRAME_DATA));
        MY_Printf("data:");
        for(uint16 i=0;i<sizeof(S_CAN_FRAME_DATA);i++)
        {

            MY_Printf("%.2x ",pdatatemp[i]);
        }
        MY_Printf("\r\n");
#endif
        if ( 0U != s_can_frame_data.length_u8)
        {
            rec_msg_external_proc(E_EX_CAN_EX3, (uint8*)&s_can_frame_data);
            support_memset(&s_can_frame_data, 0U, sizeof(S_CAN_FRAME_DATA));
        }
    }
#if 0
    /* 3.接收外部ETH数据 */
    /* 3.1 获取外部ETH1数据 */
    if ( TRUE == devLib_eth_getData( E_ETH_EX1, &s_eth_frame))
    {
        if ( 0U != s_eth_frame.len)
        {
            rec_msg_external_proc(E_EX_ETH_EX1, (uint8*)&s_eth_frame);
            support_memset(&s_eth_frame, 0U, sizeof(S_ETH_FRAME));
        }
    }

    /* 3.2 获取外部ETH2数据 */
    if ( TRUE == devLib_eth_getData( E_ETH_EX2, &s_eth_frame))
    {
        if ( 0U != s_eth_frame.len)
        {
            rec_msg_external_proc(E_EX_ETH_EX2, (uint8*)&s_eth_frame);
            support_memset(&s_eth_frame, 0U, sizeof(S_ETH_FRAME));

        }
    }
#endif

}


/*******************************************************************************************
 ** @brief: app_receive_data
 ** @param: null
 *******************************************************************************************/
extern void app_receive_data( void )
{
    /* 1.接收数据来自于主控 */
    receive_mainctl_data_proc();

    /* 2.接收外部通道数据 */
    receive_external_data_proc();

}

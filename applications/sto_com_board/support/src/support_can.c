/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : support_can.c
**@author: Created By Chengt
**@date  : 2023.09.06
**@brief : Manage support layer can 
********************************************************************************************/
#include "if_can.h"
#include "support_libPub.h"
#include "support_can.h"
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
 ** @brief: get_can_channelId
 ** @param: id
 *******************************************************************************************/
static E_CAN_CH get_can_channelId( E_CAN_ID id )
{
	E_CAN_CH ch = E_CAN_CH_MAX;

	switch( id )
	{
	case E_CAN_ID_1: ch = E_CAN_CH_1; break;
	case E_CAN_ID_2: ch = E_CAN_CH_2; break;
	case E_CAN_ID_3: ch = E_CAN_CH_3; break;
	case E_CAN_ID_4: ch = E_CAN_CH_4; break;
	case E_CAN_ID_5: ch = E_CAN_CH_5; break;
	default:break;
	}

	return ch;
}

/*******************************************************************************************
 ** @brief: support_can_init
 ** @param: id
 *******************************************************************************************/
extern E_CAN_STATE support_can_init( E_CAN_ID id )
{
    if(if_can_init() == TRUE)
    {
        return E_CAN_OK;
    }
    else
    {
        MY_Printf("if_can_init error\r\n");
        return FALSE;
    }
}

/*******************************************************************************************
 ** @brief: support_can_sendData
 ** @param: id, p_can_frame
 *******************************************************************************************/
extern E_CAN_STATE support_can_sendData( E_CAN_ID id, S_CAN_FRAME *p_can_frame )
{
	E_CAN_STATE valid_ret = E_CAN_OK;
	E_CAN_CH ch = E_CAN_CH_MAX;
	S_CAN_MSG msg = { 0U };
	
	ch = get_can_channelId( id );
	
	/* 1.参数合法性检测 */
	if(( E_CAN_CH_MAX == ch ) || ( NULL == p_can_frame ))
	{
		return E_CAN_ERR;
	}

	/* 2.组织数据 */
	msg.id_u32 = (( uint32 )p_can_frame->priority_u8 << 3U ) + p_can_frame->no_u8;
	msg.len_u8 = p_can_frame->length_u8;
	support_memcpy( msg.data_u8, p_can_frame->data_u8, p_can_frame->length_u8 );

	/* 3.发送数据 */
	if( TRUE == if_can_send( ch, &msg ))
	{
		valid_ret = E_CAN_OK;
	}
	else
	{
		valid_ret = E_CAN_ERR;
	}

	return valid_ret;
}

/*******************************************************************************************
 ** @brief: supprot_can_getData
 ** @param: id, p_can_frame
 *******************************************************************************************/
extern E_CAN_STATE supprot_can_getData( E_CAN_ID id, S_CAN_FRAME *p_can_frame )
{
	E_CAN_STATE valid_ret = E_CAN_OK;
	E_CAN_CH ch = E_CAN_CH_MAX;
	S_CAN_MSG msg = { 0U };
	
	ch = get_can_channelId( id );
	
	/* 1.参数合法性检测 */
	if(( E_CAN_CH_MAX == ch ) || ( NULL == p_can_frame ))
	{
		return E_CAN_ERR;
	}

	/* 3.发送数据 */
	if( TRUE == if_can_get( ch, &msg ))
	{
		p_can_frame->priority_u8 = ( uint8 )(( msg.id_u32 >> 3U ) & 0xFF );
		p_can_frame->no_u8 = ( uint8 )(( msg.id_u32 & 0x07 ) & 0xFF );
		p_can_frame->length_u8 =  msg.len_u8;
		support_memcpy( &p_can_frame->data_u8[0U], &msg.data_u8[0U], msg.len_u8 );
		valid_ret = E_CAN_OK;
	}
	else
	{
		valid_ret = E_CAN_ERR;
	}

	return valid_ret;
}
/**************************************end file*********************************************/



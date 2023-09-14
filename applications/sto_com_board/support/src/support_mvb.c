/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : support_mvb.c
**@author: Created By Chengt
**@date  : 2023.09.06
**@brief : Manage support layer mvb 
********************************************************************************************/
#include "if_mvb.h"
#include "support_mvb.h"
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
 ** @brief: get_mvb_channelId
 ** @param: id
 *******************************************************************************************/ 
static E_MVB_CH get_mvb_channelId( E_MVB_ID id )
{
	E_MVB_CH ch = E_MVB_CH_MAX;

	switch( id )
	{
	case E_MVB_ID_1: ch = E_MVB_CH_1; break;
	default:break;
	}

	return ch;
}
/*******************************************************************************************
 ** @brief: support_hdlc_init
 ** @param: id
 *******************************************************************************************/
extern E_MVB_STATE support_mvb_init( E_MVB_ID id )
{
	return E_MVB_OK;
}

/*******************************************************************************************
 ** @brief: support_mvb_sendData
 ** @param: id, pframe
 *******************************************************************************************/
extern E_MVB_STATE support_mvb_sendData( E_MVB_ID id, S_MVB_FRAME *pframe )
{
	E_MVB_STATE valid_ret = E_MVB_OK;
	E_MVB_CH ch = E_MVB_CH_MAX;

	ch = get_mvb_channelId( id );

	/* 1.参数合法性检查 */
	if(( E_MVB_CH_MAX == ch ) || ( NULL == pframe ))
	{
		return E_MVB_ERR;
	}
	
	/* 2.发送数据 */
	if( TRUE == if_mvb_send( ch, pframe->data_u8, pframe->len ))
	{
		valid_ret = E_MVB_OK;
	}
	else
	{
		valid_ret = E_MVB_OK;
	}

	return valid_ret;
}

/*******************************************************************************************
 ** @brief: supprot_mvb_getData
 ** @param: id, p_mvb_frame
 *******************************************************************************************/
extern E_MVB_STATE supprot_mvb_getData( E_MVB_ID id, S_MVB_FRAME *pframe )
{
	E_MVB_CH ch = E_MVB_CH_MAX;
	uint16 len = 0U;

	ch = get_hdlc_channelId( id );

	/* 1.参数合法性检查 */
	if(( E_MVB_CH_MAX == ch ) || ( NULL == pframe ))
	{
		return E_MVB_ERR;
	}

	/* 2.获取一帧数据 */
	len = if_mvb_get( ch, pframe->data_u8, MVB_FRAME_SIZE_MAX );
	pframe->len = len;

	return E_MVB_OK;
}
/**************************************end file*********************************************/

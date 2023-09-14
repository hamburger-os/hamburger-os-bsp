/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : support_eth.c
**@author: Created By Chengt
**@date  : 2023.09.06
**@brief : Manage support layer eth 
********************************************************************************************/
#include "if_eth.h"
#include "support_libPub.h"
#include "support_eth.h"
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
 ** @brief: support_can_init
 ** @param: id
 *******************************************************************************************/
static E_ETH_CH get_eth_channelId( E_ETH_ID id )
{
	E_ETH_CH ch = E_ETH_CH_MAX;

	switch( id )
	{
	case E_ETH_ID_1: ch = E_ETH_CH_1; break;
	case E_ETH_ID_2: ch = E_ETH_CH_2; break;
	case E_ETH_ID_3: ch = E_ETH_CH_3; break;
	default:break;
	}

	return ch;
}
/*******************************************************************************************
 ** @brief: support_can_init
 ** @param: id
 *******************************************************************************************/
extern E_ETH_STATE support_eth_init( E_ETH_ID id )
{
	return E_ETH_OK;
}
/*******************************************************************************************
** @brief: support_eth_sendData
** @param: eth_frame
 *******************************************************************************************/
extern E_ETH_STATE support_eth_sendData( E_ETH_ID id, uint8 *pbuf, uint16 len )
{
	E_ETH_STATE eth_status = E_ETH_OK;
	E_ETH_CH ch = E_ETH_CH_MAX;

	ch = get_eth_channelId( id );

	/* 1.参数合法性检查 */
	if(( E_ETH_CH_MAX == ch ) || ( NULL == pbuf ))
	{
		return E_ETH_ERR;
	}
	
	/* 2.发送数据 */
	if( TRUE == if_eth_send( ch, pbuf, len ))
	{
		eth_status = E_ETH_OK;
	}
	else
	{
		eth_status = E_ETH_ERR;
	}

	return eth_status;
}
/*******************************************************************************************
** @brief: supprot_eth_getData
** @param: *pframe
 *******************************************************************************************/
extern E_ETH_STATE supprot_eth_getData( E_ETH_ID id, S_ETH_FRAME *pframe )
{
	E_ETH_CH ch = E_ETH_CH_MAX;
	uint16 len = 0U;

	ch = get_eth_channelId( id );

	/* 1.参数合法性检查 */
	if(( E_ETH_CH_MAX == ch ) || ( NULL == pframe ))
	{
		return E_ETH_ERR;
	}
	
	/* 2.获取一帧数据 */
	len =  if_eth_get( ch, pframe->data_u8, ETH_FRAME_SIZE_MAX );
	pframe->len = len;

	return E_ETH_OK;
}

/*******************************************************************************************
** @brief: supprot_eth_proc
** @param: type
 *******************************************************************************************/
extern void supprot_eth_proc( E_ETH_ID id )
{


}
/**************************************end file*********************************************/

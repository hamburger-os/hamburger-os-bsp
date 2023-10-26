/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_rs485.c
 **@author: Created By Chengt
 **@date  : 2023.09.06
 **@brief : Manage support layer rs485
 ********************************************************************************************/
#include "if_rs485.h"
#include "support_rs485.h"
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
 ** @brief: get_rs485_channelId
 ** @param: id
 *******************************************************************************************/
static E_RS485_CH get_rs485_channelId(E_RS485_ID id)
{
    E_RS485_CH ch = E_RS485_CH_MAX;

    switch (id)
    {
    case E_RS485_ID_1:
        ch = E_RS485_CH_1;
        break;
    default:
        break;
    }

    return ch;
}
/*******************************************************************************************
 ** @brief: support_hdlc_init
 ** @param: id
 *******************************************************************************************/
extern E_RS485_STATE support_rs485_init(E_RS485_ID id)
{
    return E_RS485_OK;
}

/*******************************************************************************************
 ** @brief: support_mvb_sendData
 ** @param: id, pframe
 *******************************************************************************************/
extern E_RS485_STATE support_rs485_sendData(E_RS485_ID id, S_RS485_FRAME *pframe)
{
    E_RS485_STATE valid_ret = E_RS485_OK;
    E_RS485_CH ch = E_RS485_CH_MAX;

    ch = get_mvb_channelId(id);

    /* 1.参数合法性检查 */
    if ((E_RS485_CH_MAX == ch) || (NULL == pframe))
    {
        return E_RS485_ERR;
    }

    /* 2.发送数据 */
    if ( TRUE == if_mvb_send(ch, pframe->data_u8, pframe->len))
    {
        valid_ret = E_RS485_OK;
    }
    else
    {
        valid_ret = E_RS485_ERR;
    }

    return valid_ret;
}

/*******************************************************************************************
 ** @brief: supprot_mvb_getData
 ** @param: id, p_mvb_frame
 *******************************************************************************************/
extern E_RS485_STATE supprot_rs485_getData(E_RS485_ID id, S_RS485_FRAME *pframe)
{
    E_RS485_STATE ch = E_RS485_CH_MAX;
    uint16 len = 0U;

    ch = get_hdlc_channelId(id);

    /* 1.参数合法性检查 */
    if ((E_RS485_CH_MAX == ch) || (NULL == pframe))
    {
        return E_RS485_ERR;
    }

    /* 2.获取一帧数据 */
    len = if_mvb_get(ch, pframe->data_u8, RS485_FRAME_SIZE_MAX);
    pframe->len = len;

    return E_RS485_OK;
}
/**************************************end file*********************************************/

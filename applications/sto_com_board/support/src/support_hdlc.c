/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_hdlc.c
 **@author: Created By Chengt
 **@date  : 2023.09.06
 **@brief : Manage support layer hdlc
 ********************************************************************************************/
#include "if_hdlc.h"
#include "support_hdlc.h"
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
 ** @brief: get_hdlc_channelId
 ** @param: id
 *******************************************************************************************/
static E_HDLC_CH get_hdlc_channelId(E_HDLC_ID id)
{
    E_HDLC_CH ch = E_HDLC_CH_MAX;

    switch (id)
    {
    case E_HDLC_ID_1:
        ch = E_HDLC_CH_1;
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
extern E_HDLC_STATE support_hdlc_init(E_HDLC_ID id)
{
    return E_HDLC_OK;
}

/*******************************************************************************************
 ** @brief: support_hdlc_sendData
 ** @param: id, pframe
 *******************************************************************************************/
extern E_HDLC_STATE support_hdlc_sendData(E_HDLC_ID id, S_HDLC_FRAME *pframe)
{
    E_HDLC_STATE valid_ret = E_HDLC_OK;
    E_HDLC_CH ch = E_HDLC_CH_MAX;

    ch = get_hdlc_channelId(id);

    /* 1.参数合法性检查 */
    if ((E_HDLC_CH_MAX == ch) || (NULL == pframe))
    {
        return E_HDLC_ERR;
    }

    /* 2.发送数据 */
    if ( TRUE == if_hdlc_send(ch, pframe->data_u8, pframe->len))
    {
        valid_ret = E_HDLC_OK;
    }
    else
    {
        valid_ret = E_HDLC_ERR;
    }

    return valid_ret;
}

/*******************************************************************************************
 ** @brief: supprot_hdlc_getData
 ** @param: id, pframe
 *******************************************************************************************/
extern E_HDLC_STATE supprot_hdlc_getData(E_HDLC_ID id, S_HDLC_FRAME *pframe)
{
    E_HDLC_CH ch = E_HDLC_CH_MAX;
    uint16 len = 0U;

    ch = get_hdlc_channelId(id);

    /* 1.参数合法性检查 */
    if ((E_HDLC_CH_MAX == ch) || (NULL == pframe))
    {
        return E_HDLC_ERR;
    }

    /* 2.获取一帧数据 */
    len = if_hdlc_get(ch, pframe->data_u8, HDLC_FRAME_SIZE_MAX);
    pframe->len = len;

    return E_HDLC_OK;
}
/**************************************end file*********************************************/


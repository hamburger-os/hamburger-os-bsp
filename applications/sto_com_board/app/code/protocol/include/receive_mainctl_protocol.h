/***************************************************************************
#include <text_app/protocol/include/app_list_manage.h>
文件名：app_list_manage.h
模    块：初始化应用程序所需要的消息队列
详    述：无
作    者：jiaqx,20231206
***************************************************************************/

#include "common.h"
#include "support_libList.h"
#include "devlib_can.h"
#include "devlib_eth.h"
#include "list_manage.h"
#ifndef RECEIVE_MAINCTL_PROTOCOL_H
#define RECEIVE_MAINCTL_PROTOCOL_H



extern void rec_msg_mainctl_proc( uint8 *pdata );
extern BOOL getListData_CANtype_mainctl( E_CAN_TYPE type, S_CAN_FRAME_DATA *p_can_frame );
extern BOOL getListData_ETHtype_mainctl( E_ETH_TYPE type, S_ETH_FRAME *p_eth_frame );

#endif

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
#ifndef SEND_EXTERNAL_PROTOCOL_H
#define SEND_EXTERNAL_PROTOCOL_H


extern void send_external_list_data( uint8 channel_type_num, E_CHANNEL_TYPE e_channel_type);
#endif

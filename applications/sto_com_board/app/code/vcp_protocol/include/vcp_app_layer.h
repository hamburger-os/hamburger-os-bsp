/***************************************************************************
文件名：vcp_app_layer.h
模    块：
详    述：
作    者：jiaqx 20231128
***************************************************************************/

#include "comm_proc_ctl.h"
#include "common.h"

#ifndef VCP_APP_LAYER_H
#define VCP_APP_LAYER_H

#pragma pack(1)
/* 安全层帧头解析结构 */
typedef struct {
     uint8 msg_type;                /*报文类型*/
     uint8 msg_sub_type;            /*报文子类型*/
     uint16 serial_num;             /*报文序列号*/
}S_APP_LAYER_HEADER;

#pragma pack()

extern void vcp_app_layer_process(uint8 *pBuf , uint16 applayer_datalen );

extern void vcp_app_layer_pakage( uint8 *pApp, uint8 *pbuf, uint16 app_datalen,
                                  uint8 msg_type, uint8 msg_sub_type, uint8 send_chl);

#endif

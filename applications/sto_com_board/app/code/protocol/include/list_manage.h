/***************************************************************************
文件名：app_list_manage.h
模    块：初始化应用程序所需要的消息队列
详    述：无
作    者：jiaqx,20231206
***************************************************************************/

#include "common.h"
#include "support_libList.h"

#ifndef APP_LIST_MANAGE_H
#define APP_LIST_MANAGE_H


/****************can type*********************/
typedef enum  {
 E_TX1CAN1 = 0U,
 E_TX1CAN2,
 E_TX1CAN3,
 E_TX1CAN4,
 E_TX1CAN5,
 E_TX1VMCAN1,
 E_TX1VMCAN2,

 E_TX2CAN1,
 E_TX2CAN2,
 E_TX2CAN3,
 E_TX2CAN4,
 E_TX2CAN5,
 E_TX2VMCAN1,
 E_TX2VMCAN2,
 E_CAN_TYPE_MAX
}E_CAN_TYPE;

typedef enum
{
    E_TX1_ETH1 = 0U,
    E_TX1_ETH2,
    E_TX1_ETH3,
    E_TX1_ETH4,
    E_TX2_ETH1,
    E_TX2_ETH2,
    E_TX2_HDLC,
    E_TX2_MVB,
    E_TX1_RS485A,
    E_TX1_RS485B,
    E_TX2_RS485A,
    E_ETH_TYPE_MAX
}E_ETH_TYPE;


typedef enum
{
    E_CHANNEL_CAN = 0U,
    E_CHANNEL_ETH
}E_CHANNEL_TYPE;

/* 数据流宏定义 */
/*******************************************************************************************
 *        Local variables
 *******************************************************************************************/
/* TX插件CAN通道信息  */
#define TX_CAN_CHANNEL_NUM  ( 14U )         /* 通信对外CAN通道数目 */

/* TX1插件CAN通道号  */
#define DATA_CHANNEL_TX1CAN1 ( 0x10U )      /* 通信1的CAN1通道 */
#define DATA_CHANNEL_TX1CAN2 ( 0x11U )      /* 通信1的CAN2通道 */
#define DATA_CHANNEL_TX1CAN3 ( 0x12U )      /* 通信1的CAN3通道 */
#define DATA_CHANNEL_TX1CAN4 ( 0x13U )      /* 通信1的CAN4通道 */
#define DATA_CHANNEL_TX1CAN5 ( 0x14U )      /* 通信1的CAN5通道 */
#define DATA_CHANNEL_TX1VMCAN1 ( 0x15U )    /* 通信1的VMCAN1通道 */
#define DATA_CHANNEL_TX1VMCAN2 ( 0x16U )    /* 通信1的VMCAN2通道 */

/* TX2插件CAN通道号 */
#define DATA_CHANNEL_TX2CAN1 ( 0x17U )      /* 通信2的CAN1通道 */
#define DATA_CHANNEL_TX2CAN2 ( 0x18U )      /* 通信2的CAN2通道 */
#define DATA_CHANNEL_TX2CAN3 ( 0x19U )      /* 通信2的CAN3通道 */
#define DATA_CHANNEL_TX2CAN4 ( 0x1AU )      /* 通信2的CAN4通道 */
#define DATA_CHANNEL_TX2CAN5 ( 0x1BU )      /* 通信2的CAN5通道 */
#define DATA_CHANNEL_TX2VMCAN1 ( 0x1CU )    /* 通信2的VMCAN1通道 */
#define DATA_CHANNEL_TX2VMCAN2 ( 0x1DU )    /* 通信2的VMCAN2通道 */


/* TX插件ETH通道信息  */
#define TX_ETH_CHANNEL_NUM   ( 6U )         /* 通信对外ETH通道数目 */

#define DATA_CHANNEL_TX1ETH1 ( 0x30U )      /* 通信1的ETH1通道 */
#define DATA_CHANNEL_TX1ETH2 ( 0x31U )      /* 通信1的ETH2通道 */
#define DATA_CHANNEL_TX1ETH3 ( 0x32U )      /* 通信1的ETH3通道 */
#define DATA_CHANNEL_TX1ETH4 ( 0x33U )      /* 通信1的ETH4通道 */

#define DATA_CHANNEL_TX2ETH1 ( 0x34U )      /* 通信2的ETH1通道 */
#define DATA_CHANNEL_TX2ETH2 ( 0x35U )      /* 通信2的ETH2通道 */

/* TX插件HDLC通道信息  */
#define TX_HDLC_CHANNEL_NUM  ( 1U )         /* 通信对外HDLC通道数目 */
#define DATA_CHANNEL_TX2HDLC ( 0x50U )      /* 通信2的HDLC通道 */

/* TX插件MVB通道号 */
#define TX_MVB_CHANNEL_NUM   ( 1U )
#define DATA_CHANNEL_TX2MVB  ( 0x60U )      /* 通信2对外MVB通道 */

/* TX插件RS485通道号 */
#define TX_RS485_CHANNEL_NUM   ( 3U )       /* 通信对外RS485通道数目 */
#define DATA_CHANNEL_TX1RS485a ( 0x80U )    /* 通信1对外RS485a通道 */
#define DATA_CHANNEL_TX1RS485b ( 0x81U )    /* 通信1对外RS485b通道 */
#define DATA_CHANNEL_TX2RS485a ( 0x82U )    /* 通信2对外RS485a通道 */

/* 单字节对齐 */
#pragma pack(1)

/* 应用层报文协议格式 */
typedef struct
{
    uint8  type_u8;                    /* 报文类型 */
    uint8  sub_type_u8;                /* 报文子类型 */
    uint16 seq_no_u16;                 /* 报文序号 */
    uint8  pdata_u8[1U];               /* 报文数据内容 */
}S_APP_MSG;

/* 数据内容格式 */
typedef struct
{
    uint8  channel_num_u8;             /* 通道数目 */
    uint8  pdata_u8[1U];               /* 数据内容 */
}S_MSG_DATA;

/* 通道数据内容格式 */
typedef struct
{
    uint8  channel_id_u8;              /* 通道id */
    uint32 timestemp_u32;              /* 时间戳 */
    uint16 len_u16;                    /* 通道数据长度 */
    uint8  pdata_u8[1U];               /* 数据内容 */
}S_CHANNEL_DATA;

/* 传输通道（CAN）数据内容格式 */
typedef struct
{
    uint32 frameAllNum_u32;            /* 通道接收到的所有帧数 */
    uint8  totalFrameNum_u8;           /* 通道本次接收到的帧数 */
    uint16 len_u16;                    /* 数据长度：frameAllNum_u32--->*pdata_u8 */
    uint8  pdata_u8[1U];               /* 数据内容 */

}S_CHANNEL_FRAME;

/* 应用CAN数据格式 */
typedef struct
{
    uint32 id_u32;                     /* 标识符id */
    uint8  len_u8;                     /* 本帧数据长度:仅指数据区长度 */
    uint8  pdata_u8[1U];               /* 数据内容 */
}S_CHANNEL_CAN;

/* 应用ETH数据格式 */
typedef struct
{
    uint16 len_u16;                    /* 本帧数据长度:仅指数据区长度 */
    uint8  pdata_u8[1U];               /* 数据内容 */
}S_CHANNEL_ETH;

/* 应用HDLC数据格式 */
typedef struct
{
    uint16 len_u16;                    /* 本帧数据长度:仅指数据区长度 */
    uint8  pdata_u8[1U];               /* 数据内容 */
}S_CHANNEL_HDLC;

/* 应用MVB数据格式 */
typedef struct
{
    uint16 len_u16;                    /* 本帧数据长度:仅指数据区长度 */
    uint8  pdata_u8[1U];               /* 数据内容 */
}S_CHANNEL_MVB;

/* 应用RS485数据格式 */
typedef struct
{
    uint16 len_u16;                    /* 本帧数据长度:仅指数据区长度 */
    uint8  pdata_u8[1U];               /* 数据内容 */
}S_CHANNEL_RS485;



#pragma pack()

extern void list_manage_init( void );
extern S_ArrayList *get_ZK_list( uint8 channelId );

extern S_ArrayList *get_EX_list( uint8 channelId );

extern uint8 get_channelIdByCanType( E_CAN_TYPE type );
extern uint8 get_channelIdByEthType( E_ETH_TYPE type );

#endif

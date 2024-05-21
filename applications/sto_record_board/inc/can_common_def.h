/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : CAN_Manage.h
**@author: Created By Chengt
**@date  : 2015.09.23
**@brief : Manage CAN Device
**@Change Logs:
**Date           Author       Notes
**2023-07-21      zm           add swos2
********************************************************************************************/
#ifndef CAN_COMMONDEF_H
#define CAN_COMMONDEF_H

#include <rtthread.h>

/* public macro definition --------------------------------------------------------------------- */
/* 通信板->主控板 ============================================================================== */
/* 自检信息 */
#define CANPRI_TXZJ      ( 0x50U )
/* 显示器1信息 */
#define CANPRI_TXCMD1    ( 0x51U )
/* 显示器2信息 */
#define CANPRI_TXCMD2    ( 0x52U )

/* 通信板->显示器 ============================================================================== */
/* 强实时 */
#define CANPRI_TXQSS     ( 0x60U )
/* 弱实时 */
#define CANPRI_TXRSS     ( 0x61U )
/* 自检信息1一系 */
#define CANPRI_TXZJ1X1   ( 0x62U )
/* 自检信息1二系 */
#define CANPRI_TXZJ1X2   ( 0x63U )
/* 自检信息2一系 */
#define CANPRI_TXZJ2X1   ( 0x64U )
/* 自检信息2二系 */
#define CANPRI_TXZJ2X2   ( 0x65U )
/* 打包信息 */
#define CANPRI_TXDB      ( 0x68U )

/* 主控->通信板 ================================================================================ */
/* 强实时A模 */
#define CANPRI_ZKQSSA    ( 0x60U )
/* 强实时B模 */
#define CANPRI_ZKQSSB    ( 0x70U )
/* 弱实时A模 */
#define CANPRI_ZKRSSA    ( 0x61U )
/* 弱实时B模 */
#define CANPRI_ZKRSSB    ( 0x71U )
/* 自检信息A模 */
#define CANPRI_ZKZJA     ( 0x63U )
/* 自检信息B模 */
#define CANPRI_ZKZJB     ( 0x73U )
/* 打包信息A模 */
#define CANPRI_ZKDBA     ( 0xb0U )
/* 打包信息B模 */
#define CANPRI_ZKDBB     ( 0xb8U )

/* 控车->机车接口 ============================================================================== */
/* 控车信息A模 */
#define CANPRI_KONGCHEA  ( 0x62U )
/* 控车信息B模 */
#define CANPRI_KONGCHEB  ( 0x72U )
/* 12-June-2018, by Liang Zhen. */
/* EBV controlling by host main board CPU A. */
#define EBV_CTRL_A       ( 0x64U )
/* EBV controlling by host main board CPU B. */
#define EBV_CTRL_B       ( 0x74U )
/* 13-June-2018, by Liang Zhen. */
/* EBV monitor and handle working condition. */
#define EBV_WC_ABV       ( 0x75U )
/* EBV monitor and handle working condition. */
#define EBV_WC_IBV       ( 0x76U )
/* 14-June-2018, by Liang Zhen. */
/* LKJ low level real-time message. */
#define LKJ_LLRT_MSG     ( 0x68U )

/* 显示器->通信板 ============================================================================== */
/* 显示器1命令信息 */
#define CANPRI_XSQCMD1   ( 0x45U )
/* 显示器2命令信息 */
#define CANPRI_XSQCMD2   ( 0x46U )

/* 接车接口板->通信板 ========================================================================== */
/* 机车接口板自检 */
#define CANPRI_JCJKZJ    ( 0x90U )
/* 28-September-2018, by Liang Zhen. */
#define CAN_PRI_HIB_B    ( 0x92U )
/* 机车接口状态 */
#define CANPRI_JCJKSTAT  ( 0x91U )

/* 记录板->通信板 ============================================================================== */
/* 记录板自检 */
#define CANPRI_JLZJ      ( 0xa0U )

/* CEU/司控器转接盒 ============================================================================ */
#define CAN_PRI_CEU      ( 0x28U )

/* 06-July-2020, by DuYanPo. */
/* 主机发送线路数据（打包） */
#define Host_SlopeInfo   ( 0x8CU )
/* LKJ强实时信息透传 */
#define CANPRI_ZKQSSTC   ( 0x66U )
/* LKJ弱实时信息1透传 */
#define CANPRI_ZKRSSTC1  ( 0x67U )
/* DMI显示查询状态信息 */
#define CANPRI_DMIXSCX1   ( 0x6AU )
#define CANPRI_DMIXSCX2   ( 0x7AU )
/* 微机数据 */
#define CANPRI_WJSJ      ( 0x81U )
/* 微机接口板自检信息 */
#define CANPRI_WJJKZJ    ( 0x82U )

/* 07-10-2020, by DuYanPo. */
/* 主控->微机接口板 ================================================================================ */
#define CANPRI_ZKWJSJ1   ( 0x30U )
#define CANPRI_ZKWJSJ2   ( 0x31U )
/* 微机接口板->主控 ================================================================================ */
#define CANPRI_WJZKSJ1   ( 0x32U )
#define CANPRI_WJZKSJ2   ( 0x33U )

/* 通过通信板获取 */
#define CANPRI_LKJDATA   ( 0x15U )

#define CANPRI_RSS_2_DATA   ( 0x2DU )  /* 主控发送扩展实时数据 */

/* 06-July-2020, by DuYanPo. */
#define PACKET_LEN (12495)                  /* 单包数据的长度 */
#define MAX_LENGTH_PACKET (PACKET_LEN * 8)  /* 打包数据接收数据缓存区最大长度 */
/* 06-July-2020, by DuYanPo. */
#define Guide_Curve_Type         (0x03)             /* 指导曲线类型 */
#define LimitSpeed_Curve_Type    (0x02)             /* 限速曲线类型 */
#define Grade_Curve_Type         (0x04)             /* 线路数据类型 */
#define ForecastSpeed_Type       (0x01)             /* 速度预测曲线 */
#define Uncontrollable_Type      (0x09)             /* 应用程序升级 */
#define Reminder_Type            (0x0a)			        /* 提示信息数据 */
#define LocomotiveInfo_Type      (0x0b)			        /* 机车信息数据 */
 
/* 06-July-2020, by DuYanPo. */
/* CAN接收状态 */
typedef enum
{
	E_IDLE = 0x00,                    /* 空闲状态 */
	E_REQUESTING = 0x01,              /* 正在请求 */
	E_RECEIVING = 0x02,               /* 正在接收 */
	E_RECEIVED_OK = 0x03,             /* 接收完成 */
	E_RECEIVING_FAILURE = 0x04        /* 接收错误 */

}E_CAN_RECEIVE_STATE;
/* 06-July-2020, by DuYanPo. */
/* CAN打包信息包描述 */
typedef struct
{
	uint32_t end_frm_len   : 3;   								/* 本包末组末帧有效字节数 */
	uint32_t end_frm       : 3;  								  /* 本包末组末帧号 */
	uint32_t pack_id       : 13; 								  /* 本包号 */
	uint32_t total_pack    : 13;  								/* 总包数 */
}S_PACK_DESCRIBE; 

/* 06-July-2020, by DuYanPo. */
typedef struct 
{
	E_CAN_RECEIVE_STATE e_state;          		    /* 接收状态 */
	uint8_t group_id_u8;                   		      /* 组号 */
	uint8_t frame_id_u8;                   		      /* 帧号 */
	uint16_t total_length_u16;            		      /* 包信息中的总长度  */
	uint16_t locomotiveinfo_total_length_u16;       /* 指导包信息中的机车信息总长度 */
	uint16_t receiving_length_u16;          	      /* 正在接收的数据长度 */
	uint8_t *receiving_data_u8;  			  		        /* 正在接收的数据缓存区 */
	uint16_t received_length_u16;           		    /* 已接收完成的数据的长度 */
	uint16_t locomotiveinfo_received_length_u16; 	  /* 已接收完成的机车信息总长度 */
	uint8_t *received_data_u8;             		      /* 已接收完成的数据缓冲区 */
}S_CAN_PACKE_Grade;


rt_err_t can_buf_init(S_CAN_PACKE_Grade *can_pkt_grade);
#endif
/**************************************end file*********************************************/


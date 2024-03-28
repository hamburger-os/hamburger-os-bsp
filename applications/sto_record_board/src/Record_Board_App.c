/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : RecordBoard_Board_App.c
**@author: Created By Sunzq
**@date  : 2015.11.18
**@brief : None
**@Change Logs:
**Date           Author       Notes
**2023-07-21      zm           add swos2
********************************************************************************************/
#include "Record_Board_App.h"

#define DBG_TAG "record_app"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "Common.h"
#include "can_common_def.h"
#include "RecordErrorCode.h"
#include "crc.h"
#include "Record_FileCreate.h"

/* 06-July-2020, by DuYanPo. */
#define MAX_VAILD_PER_FRM (7)                           		/* CAN打包发送每帧最大有效字节数 */
#define CAN_PACK_GROUP_MAX_LEN (49)                          	/* CAN打包发送每组最大字节数 */
#define MAX_LEN_PER_PACKAGE (255 * CAN_PACK_GROUP_MAX_LEN)  	/* CAN每包最大字节数 */
/* 计算传输文件的字节数，-2*(P1)应为每包前两个字节为曲线条数，而非有效的曲线数据 */
#define CAL_PACKAGE_SIZE(P1, P2, P3, P4) (((((P1)-1) * MAX_LEN_PER_PACKAGE) + ((P2-1) * CAN_PACK_GROUP_MAX_LEN) + ((P3-1) * MAX_VAILD_PER_FRM) + (P4))-2*(P1))
/* Define something usefull */
#define TO_UINT16(X,Y)            ( ( ( uint16_t )( X ) << 8u ) + ( uint16_t )( Y ) )

uint8_t GradePacket_Start_Flag = 0;      /* 坡道信息开始接收标志 */
uint8_t GradePacket_Finish_Flag = 0;     /* 坡道信息接收完成标志 */
uint8_t LocomotiveInfo_Start_Flag = 0;   /* 机车信息开始接收标志 */
uint8_t LocomotiveInfo_Finish_Flag = 0;  /* 机车信息接收完成标志 */


#if ENABLE_RECORD_BOARD_APP  //TODO(mingzhao)
/* "Iϵ", "IIϵ" */
static const char DPRT_I[]  = { 0x49U, 0xCFU, 0xB5U, '\0' };
static const char DPRT_II[] = { 0x49U, 0x49U, 0xCFU, 0xB5U, '\0' };

/*******************************************************************************************
** @brief: RecordBoard_DeviceInit
** @param: null
*******************************************************************************************/
static void RecordBoard_DeviceInit( void )
{
  UART1_Init();
  
	ETH_Manage_Init();
  
	GPIO_Device_Init();
  
	CAN_Device_Init();
  
	S25FL256_Manage_Init();
  
	FM25V05_Manage_Init();
  
	/* 20171111---add KSZ8895 init */
  /* 16-August-2018, by Liang Zhen. */
  #if 0
	KSZ8895_Device_Init();
  #endif
  
  /* 8-May-2018, by Liang Zhen. */  
  switch ( Get_CPU_Type() )
  {
    case CPU_B :			
      Set_IAP_Device_ID( HRB_IAP_DEV_ID_II );
      /* 28-September-2018, by Liang Zhen. */
      ErrorCodeInit( DPRT_II );
      break;
    
    case CPU_A :
      Set_IAP_Device_ID( HRB_IAP_DEV_ID_I );
      /* 28-September-2018, by Liang Zhen. */
      ErrorCodeInit( DPRT_I );
      break;
    
    default : 
      Set_IAP_Device_ID( 0x00U );
      /* 28-September-2018, by Liang Zhen. */
      ErrorCodeInit( "ERR " );
      break;
  } /* end switch */
  
  /* 28-Septmeber-2018, by Liang Zhen. */
  CAN0_time = GetTicks();
  CAN1_time = GetTicks();
}

/******************************************************************************************
** @brief: RecordBoard_Operation_Init
** @param: null
*******************************************************************************************/
void RecordBoard_Operation_Init( void )
{
	/* Common Operation ------Initlize */
	Board_Operation.Board_DataInit      = RecordBoard_DataInit;
	Board_Operation.Board_InterruptInit = RecordBoard_InterruptInit;
	Board_Operation.Board_Device_Init   = RecordBoard_DeviceInit;
	Board_Operation.Board_MainApp       = RecordBoard_MainApp;
}
#endif
/**************************************************************************************************
(^_^) Function Name : Processing_EBV_Controlling.
(^_^) Brief         : Processing EBV controlling message from host main board.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-June-2018.
(^_^) Parameter     : low3bit --> low 3-bit of EBV CAN priority.
(^_^)                 data    --> CAN data array.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Processing_EBV_Controlling( uint8_t low3bit, uint8_t data[] )
{
  switch ( low3bit )
  {
    /* ABV. */
    case 0x01U : 
      Update_ABV_ControllingMessage( data );
      break;
    
    /* IBV */
    case 0x02U : 
      Update_IBV_ControllingMessage( data );
      break;
    
    /* Unknown. */
    default : 
      break;
  } /* end switch */
} /* end function Processing_EBV_Controlling */


/**************************************************************************************************
(^_^) Function Name : Processing_IAP_Message.
(^_^) Brief         : Processing IAP message.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-June-2018.
(^_^) Parameter     : low3bit  --> low 3-bit of EBV CAN priority.
(^_^)                 high8bit --> high 8-bit of EBV CAN priority.
(^_^)                 data     --> CAN data array.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Processing_IAP_Message( uint8_t low3bit, uint8_t high8bit, uint8_t data[] )
{
#if ENABLE_RECORD_BOARD_APP  //TODO(mingzhao)
  uint16_t prio = ( uint16_t )( ( uint32_t )high8bit << 3U )\
                            + ( uint16_t )( low3bit & 0x07U );
  switch ( low3bit )
  {
    case 0x00U : 
      ProcessDatagramDescriptionFrame( data, prio );
      break;
    case 0x01U : 
      ProcessDatagramDataFrame_1( data, prio );
      break;
    case 0x02U : 
    case 0x03U : 
    case 0x04U : 
    case 0x05U : 
    case 0x06U : 
    case 0x07U : 
      ProcessDatagramDataFrame_2To7( data, prio );
      break;
    default    : break;
  } /* end switch */

#endif
} /* end function Processing_IAP_Message */


/**************************************************************************************************
(^_^) Function Name : Processing_CEU_Message.
(^_^) Brief         : Processing CEU message.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-June-2018.
(^_^) Parameter     : low3bit --> low 3-bit of EBV CAN priority.
(^_^)                 data    --> CAN data array.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Processing_CEU_Message( uint8_t low3bit, uint8_t data[] )
{

  switch ( ( uint32_t )low3bit )
  {
    case 0x01U : 
      UpdateErrorFlag_CEU( DEV_SIDE_I, &data[4] );
      break;
    case 0x02U : 
      SK1ZJ_time = rt_tick_get();
      break;
    
    case 0x05U : 
      UpdateErrorFlag_CEU( DEV_SIDE_II, &data[4] );
      break;
    case 0x06U : 
      SK2ZJ_time = rt_tick_get();
      break;
    
    default : 
      break;
  } /* end switch */
} /* end function Processing_CEU_Message */


/**************************************************************************************************
(^_^) Function Name : Processing_ABV_WorkingCondition.
(^_^) Brief         : Processing automatic brake valve working condition.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-June-2018.
(^_^) Parameter     : low3bit --> low 3-bit of EBV CAN priority.
(^_^)                 data    --> CAN data array.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Processing_ABV_WorkingCondition( uint8_t low3bit, uint8_t data[] )
{
  switch ( low3bit )
  {
    /* Side I. */
    case 0x01U : 
      Update_ABV_MonitorMessage( data, 0x01U );
      /* 28-September-2018, by Liang Zhen. */
      UpdateErrorFlag_EBV( DEV_SIDE_I, data + 3U );
      break;
    
    case 0x02U : 
      Update_ABV_HandleMessage( data, 0x02U );
      break;
    
    /* Side II. */
    case 0x05U : 
      Update_ABV_MonitorMessage( data, 0x05U );
      /* 28-September-2018, by Liang Zhen. */
      UpdateErrorFlag_EBV( DEV_SIDE_II, data + 3U );
      break;
    
    case 0x06U : 
      Update_ABV_HandleMessage( data, 0x06U );
      break;
    
    default : 
      break;
  } /* end switch */
} /* end function Processing_ABV_WorkingCondition */


/**************************************************************************************************
(^_^) Function Name : Processing_IBV_WorkingCondition.
(^_^) Brief         : Processing independent brake valve working condition.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-June-2018.
(^_^) Parameter     : low3bit --> low 3-bit of EBV CAN priority.
(^_^)                 data    --> CAN data array.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Processing_IBV_WorkingCondition( uint8_t low3bit, uint8_t data[] )
{
  switch ( low3bit )
  {
    /* Side I. */
    case 0x01U : 
      Update_IBV_MonitorMessage( data, 0x01U );
      /* 28-September-2018, by Liang Zhen. */
      UpdateErrorFlag_EBV( DEV_SIDE_I, data + 3U );
      break;
    
    case 0x02U : 
      Update_IBV_HandleMessage( data, 0x02U );
      break;
    
    /* Side II. */
    case 0x05U : 
      Update_IBV_MonitorMessage( data, 0x05U );
      /* 28-September-2018, by Liang Zhen. */
      UpdateErrorFlag_EBV( DEV_SIDE_II, data + 3U );
      break;
    
    case 0x06U : 
      Update_IBV_HandleMessage( data, 0x06U );
      break;
    
    default : 
      break;
  } /* end switch */
} /* end function Processing_IBV_WorkingCondition */


/**************************************************************************************************
(^_^) Function Name : Processing_HMB_HLRT_Message.
(^_^) Brief         : Processing high level real-time message of host main board.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 14-June-2018.
(^_^) Parameter     : low3bit --> low 3-bit of EBV CAN priority.
(^_^)                 data    --> CAN data array.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Processing_HMB_HLRT_Message( uint8_t low3bit, uint8_t data[] )
{
  switch ( ( uint32_t )low3bit )
  {
    case 0x00U : 
      
      break;
    
    case 0x01U : 
      break;
    
    case 0x02U : 
      Update_HLRT_SpeechTipsMessage( data + 2U );
      break;
    
    /* 28-September-2018, by Liang Zhen. */
    case 0x04U : 
      Update_LKJ_HLRT_Message( data );
      break;
    
    default : 
      break;
  } /* end switch */
} /* end function Processing_HMB_HLRT_Message */


/**************************************************************************************************
(^_^) Function Name : Processing_LKJ_LLRT_Message.
(^_^) Brief         : Processing low level real-time message of LKJ.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 14-June-2018.
(^_^) Parameter     : low3bit --> low 3-bit of EBV CAN priority.
(^_^)                 data    --> CAN data array.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Processing_LKJ_LLRT_Message( uint8_t low3bit, uint8_t data[] )
{
  switch ( low3bit )
  {
    case 0x00U : 
      Update_LKJ_LLRT_Message( data );
      break;
    
    case 0x01U : 
      break;
    
    default : 
      break;
  } /* end switch */
} /* end function Processing_LKJ_LLRT_Message */

/* 显示器拷贝程序 20201027 by dyp
 * can_receive_gradepacket - 接收坡度打包数据
 * @ps_can_frame: 指向CAN帧结构体的指针
 */
void Processing_Gradepacket_Message(CAN_FRAME *ps_can_frame)
{
    uint16_t i, j;
    uint16_t crc_check1, crc_check2;
    uint32_t length;
    static S_PACK_DESCRIBE s_pack_describe;
    static uint8_t curve_type, end_group, end_frame;
    static uint16_t total_packet, pack_id;

    /* 记录一包数据传输过程中出错的情况 */
    static uint8_t file_error_count = 0;

    /* 获取描述帧的信息 */
    if (ps_can_frame->no_u8 == 0)
    {
        /* 解析包描述帧 */
        rt_memcpy(&s_pack_describe, &ps_can_frame->data_u8[2], 4);
        curve_type = ps_can_frame->data_u8[0] & 0x0f;
        end_group = ps_can_frame->data_u8[1];
        end_frame = (uint8_t) s_pack_describe.end_frm;
        total_packet = (uint16_t) s_pack_describe.total_pack;
        pack_id = (uint16_t) s_pack_describe.pack_id;
        /*
         * 接收到第一包第一帧的内容，开始为接收区分配内存
         * 20170628---解决多包数据接收问题：在第一包数据的描述帧把对应的接收数据长度置零
         * CAL_PACKAGE_SIZE:用于计算接收文件的总长度
         */
        if (pack_id)
        {
            /* 线路数据类型 */
            if (Grade_Curve_Type == curve_type)
            {
#if 0
                s_packet_gradeInfo.received_data_u8 = Grade_Received_data;
                s_packet_gradeInfo.total_length_u16 = CAL_PACKAGE_SIZE(pack_id,end_group,end_frame,s_pack_describe.end_frm_len);

                if (1 == pack_id)
                {
                    s_packet_gradeInfo.received_length_u16 = 0;
                }
                s_packet_gradeInfo.e_state = E_RECEIVING; /* 置状态为正在接收 */
                GradePacket_Start_Flag = 1;
#endif
            }
            /* 机车信息数据类型 */
            else if (LocomotiveInfo_Type == curve_type)
            {
//				printf("LocomotiveInfo_Type!\n");
//				s_packet_gradeInfo.received_data_u8 = LocomotiveInfo_Received_data;
                s_packet_gradeInfo.locomotiveinfo_total_length_u16 = CAL_PACKAGE_SIZE(pack_id, end_group, end_frame,
                        s_pack_describe.end_frm_len);

                if (1 == pack_id)
                {
                    s_packet_gradeInfo.locomotiveinfo_received_length_u16 = 0;
//					printf("locomotiveinfo_received_length_u16 is zero\n");
                }
                s_packet_gradeInfo.e_state = E_RECEIVING; /* 置状态为正在接收 */
                LocomotiveInfo_Start_Flag = 1;
            }
            else
            {
                s_packet_gradeInfo.e_state = E_IDLE;
            }
        }
        else
        {
            s_packet_gradeInfo.e_state = E_IDLE;
            if (0 == total_packet)
            {
                if (Grade_Curve_Type == curve_type)
                {
#if 0
                    /* 设置发送完成标志位 */
                    s_packet_gradeInfo.total_length_u16 = 0; /* 接收长度 */
                    s_packet_gradeInfo.received_length_u16 = 0;
                    GradePacket_Finish_Flag = 1;
                    GradePacket_Start_Flag = 0;
                    printf("grade len is Zero\n");
                    printf("s_packet_gradeInfo.received_length_u16 = 0,total_packet = 0\n");
#endif
                }
                else if (LocomotiveInfo_Type == curve_type)
                {
                    /* 设置发送完成标志位 */
                    s_packet_gradeInfo.locomotiveinfo_total_length_u16 = 0; /* 接收长度 */
                    s_packet_gradeInfo.locomotiveinfo_received_length_u16 = 0;
                    LocomotiveInfo_Finish_Flag = 1;
                    LocomotiveInfo_Start_Flag = 0;
                    LOG_I("LocomotiveInfo len is Zero");
                    LOG_I("s_packet_gradeInfo.locomotiveinfo_received_length_u16 = 0,total_packet = 0");
                }
            }
        }

        file_error_count = 0;
        s_packet_gradeInfo.receiving_length_u16 = 0; /* 接收长度 */
        s_packet_gradeInfo.group_id_u8 = 1; /* 组号 */
        s_packet_gradeInfo.frame_id_u8 = 1; /* 帧号 */
//		printf("grade: end_group = %d end_frm = %d\n",end_group,end_frame);
//		printf("grade: packet_id = %d total_packet = %d total_length = %d\n",pack_id,total_packet,s_packet_gradeInfo.total_length_u16);

#if 0
        /* 接收描述帧之后发送返回帧 */
        can_send_ack(0x08,0x60,0x8b,0,0);
#endif
    }
    /* 接收打包数据的真实内容 */
    else if ((s_packet_gradeInfo.e_state == E_RECEIVING) && (s_packet_gradeInfo.receiving_data_u8 != NULL))
    {
        /* 判断组号和帧号是否匹配 */
        if ((s_packet_gradeInfo.group_id_u8 == ps_can_frame->data_u8[0])
                && (s_packet_gradeInfo.frame_id_u8 == ps_can_frame->no_u8))
        {
            for (i = 1; i < 8; i++)
            {
                s_packet_gradeInfo.receiving_data_u8[s_packet_gradeInfo.receiving_length_u16++] =
                        ps_can_frame->data_u8[i];
            }
            /* 判断本包数据是否接收完成(依据末组、末帧) */
            if ((s_packet_gradeInfo.group_id_u8 == end_group) && (s_packet_gradeInfo.frame_id_u8 == end_frame))
            {
                /* 计算校验和,数据开始的两个字节是文件校验和,length为接收的本数据包的长度 */
                length = (end_group - 1) * 49 + (end_frame - 1) * 7 + s_pack_describe.end_frm_len - 2;
                crc_check1 = Common_CRC16(s_packet_gradeInfo.receiving_data_u8 + 2, length);
                crc_check2 = TO_UINT16(s_packet_gradeInfo.receiving_data_u8[1],
                        s_packet_gradeInfo.receiving_data_u8[0]);

                if (crc_check1 != crc_check2)
                {
                    /* 发送错误返回帧 */
                    LOG_E("grade: CRC error");
#if 0
                    can_send_ack(0x02,0x60,0x8b,0,(pack_id << 3));
#endif
                    /* 校验错误，设置接收缓冲区的状态为空闲 */
                    s_packet_gradeInfo.e_state = E_IDLE;
                    return;
                }

//				printf("can_receive_gradepacket length:%d\n",length);

                if (Grade_Curve_Type == curve_type)
                {
#if 0
                    /* 开始拷贝本包的数据内容 */
                    for (j = 0;j < length;j++)
                    {
                        s_packet_gradeInfo.received_data_u8[s_packet_gradeInfo.received_length_u16 + j] = s_packet_gradeInfo.receiving_data_u8[j + 2];
                    }
                    s_packet_gradeInfo.received_length_u16 += length;
#endif
                }
                else if (LocomotiveInfo_Type == curve_type)
                {
                    /* 开始拷贝本包的数据内容 */
                    for (j = 0; j < length; j++)
                    {
                        s_packet_gradeInfo.received_data_u8[s_packet_gradeInfo.locomotiveinfo_received_length_u16 + j] =
                                s_packet_gradeInfo.receiving_data_u8[j + 2];
                    }
                    s_packet_gradeInfo.locomotiveinfo_received_length_u16 += length;
                }
//				printf("received_length_u16:%d,length:%d\n",s_packet_gradeInfo.received_length_u16,length);

                if (pack_id != total_packet)
                {
#if 0
                    /* 发送本包接收完成返回帧 */
                    can_send_ack(0x01,0x60,0x8b,0,(pack_id << 3));
#endif
                    LOG_I("grade: send packet ack");
                    s_packet_gradeInfo.e_state = E_IDLE;
                }
                else
                {
#if 0
                    /* 发送接收完成返回帧 */
                    can_send_ack(0x01,0x60,0x8b,0,(pack_id << 3));
#endif
                    s_packet_gradeInfo.e_state = E_IDLE;

                    if (Grade_Curve_Type == curve_type)
                    {
#if 0
                        /* 文件数据包接收完成 */
//						printf("grade_file finished\n");
                        /* 设置发送完成标志位 */
                        GradePacket_Finish_Flag = 1;
                        GradePacket_Start_Flag = 0;
#endif
                    }
                    else if (LocomotiveInfo_Type == curve_type)
                    {
                        /* 机车信息接收完成 */
//						printf("locomotiveinfo_file finished\n");
                        /* 设置发送完成标志位 */
                        LocomotiveInfo_Finish_Flag = 1;
                        LocomotiveInfo_Start_Flag = 0;
//						s_packet_gradeInfo.locomotiveinfo_received_length_u16 = s_packet_gradeInfo.received_length_u16;
                    }
                }
            }
            else
            {
                s_packet_gradeInfo.frame_id_u8++;
                if (s_packet_gradeInfo.frame_id_u8 > 7)
                {
                    s_packet_gradeInfo.frame_id_u8 = 1;
                    s_packet_gradeInfo.group_id_u8++;
                }
            }
        }
        else
        {
            /* 判断是否出现丢帧的情况 */
            if ((end_group == ps_can_frame->data_u8[0]) && ((uint8_t) s_pack_describe.end_frm == ps_can_frame->no_u8))
            {
#if 0
                /* 发送错误的返回帧 */
                can_send_ack(0x04,0x60,0x8b,s_packet_gradeInfo.group_id_u8,(pack_id << 3) | s_packet_gradeInfo.frame_id_u8);
                printf("grade: file interrrup group_id=%d  frame_id=%d\n",s_packet_gradeInfo.group_id_u8,s_packet_gradeInfo.frame_id_u8);
#endif
                file_error_count++;
                if (file_error_count >= 3)
                {
                    LOG_E("grade: file error count_out");
                    file_error_count = 0;
                    s_packet_gradeInfo.e_state = E_IDLE;
                }
            }
        }
    }
}


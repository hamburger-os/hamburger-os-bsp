/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : Record_DataProc.h
**@author: Created By Sunzq
**@date  : 2015.11.19
**@brief : None
**@Change Logs:
**Date           Author       Notes
**2023-07-21      zm           add swos2
********************************************************************************************/

#ifndef __RECORD_
#define __RECORD_

#include "common.h"
#include "can_common_def.h"
#include "type.h"
#include "file_manager.h"

extern CAN_FRAME  Record_CanBuffer[150];

extern uint8_t g_ZK_DevCode;
extern uint8_t g_XSQ_DevCode;
extern uint8_t g_TX_DevCode;
extern uint8_t g_JK_DevCode;
extern uint8_t g_CEU_DevCode;
extern uint8_t g_ECU_DevCode;

#define SFILE_DIR_SIZE      (128U)  /* 结构体SFile_Directory的大小 */

/* Bus Data. ============================================================================== */
/* 0x60/0x70 */
#define QSS_(N)        Record_CanBuffer[N]
/* 0x61/0x71 */
#define RSS_(N)        Record_CanBuffer[8 + N]
/* 0x45显示器1自检 */
#define CAN_0X45_(N)   Record_CanBuffer[16 + N]
/* 0x46显示器2自检 */
#define CAN_0X46_(N)   Record_CanBuffer[20 + 2 + N]
/* 主控1自检 */
#define CAN_0X63_(N)   Record_CanBuffer[24 + 4 + N]
/* 主控2自检 */
#define CAN_0X73_(N)   Record_CanBuffer[28 + 4 + N]
/* 通信自检 */
#define CAN_0X50_(N)   Record_CanBuffer[32 + 4 + N]
/* 机车接口自检 */
#define CAN_0X90_(N)   Record_CanBuffer[34 + 4 + N]
/* 主控控车信息 */
#define CAN_KONGCHE(N) Record_CanBuffer[40 + 4 + N]
/* LKJ强实时信息透传 */
#define CAN_0x66(N)    Record_CanBuffer[41 + 4 + N]     //增加CAN协议内容
/* LKJ弱实时信息1透传 */
#define CAN_0x67(N)    Record_CanBuffer[46 + 4 + N]     //增加CAN协议内容
/* LKJ弱实时信息2透传 */
#define CAN_0x68(N)    Record_CanBuffer[54 + 4 + N]     //增加CAN协议内容
/* 主控板发送DMI查询状态信息 */
#define CAN_0x6A(N)    Record_CanBuffer[56 + 4 + N]     //增加CAN协议内容
/* 微机接口板自检信息 */
#define CAN_0x82(N)    Record_CanBuffer[64 + 2 + N]     //增加CAN协议内容
/* 司控盒发送信息 */
#define CAN_0x28(N)    Record_CanBuffer[65 + 2 + N]     //增加CAN协议内容
/* 微机数据信息 */
#define CAN_0x81(N)    Record_CanBuffer[69 + 2 + N]     //增加CAN协议内容
/* 大闸监控信息 */
#define CAN_0x75(N)    Record_CanBuffer[75 + 2 + N]     //增加CAN协议内容
/* 小闸监控信息 */
#define CAN_0x76(N)    Record_CanBuffer[79 + 2 + N]     //增加CAN协议内容
/* 主控发送微机信息1 */
#define CAN_0x30(N)    Record_CanBuffer[83 + 2 + N]     //增加CAN协议内容
/* 主控发送微机信息2 */
#define CAN_0x31(N)    Record_CanBuffer[91 + 2 + N]     //增加CAN协议内容
/* 微机发送主控信息1 */
#define CAN_0x32(N)    Record_CanBuffer[99 + 2 + N]     //增加CAN协议内容
/* 微机发送主控信息2 */
#define CAN_0x33(N)    Record_CanBuffer[107 + 2 + N]     //增加CAN协议内容
/* 机车接口板发送状态信息 */
#define CAN_0x91(N)    Record_CanBuffer[115 + 2 + N]     //增加CAN协议内容


/* 协议解析文件头 */
#if 0
#define TIME_SFM  			          ( CAN_0x66( 0u ).data_u8[4] )
#define TIME_NYR                      ( RSS_( 2u ).data_u8[4] )
#else
#define TIME_NYR                      ( RSS_( 2u ).data_u8[4] )
#define TIME_N                        ( RSS_( 2u ).data_u8[4] )
#define TIME_Y                        ( RSS_( 2u ).data_u8[5] )
#define TIME_R                        ( RSS_( 2u ).data_u8[6] )
#define TIME_SFM                      ( QSS_( 0u ).data_u8[4] )
#define TIME_S                        ( QSS_( 0u ).data_u8[4] )
#define TIME_F                        ( QSS_( 0u ).data_u8[5] )
#define TIME_M                        ( QSS_( 0u ).data_u8[6] )
#endif
#define CHECIKUOCHONG  	              ( RSS_( 3u ).data_u8[3] )
#define CHECI  					      ( RSS_( 3u ).data_u8[0] )
#define SIJI1  					      ( RSS_( 0u ).data_u8[0] )
#define SIJI2  					      ( RSS_( 0u ).data_u8[3] )
#define SHUJUJIAOLU  		          ( RSS_( 4u ).data_u8[1] )
#define JIANKONGJIAOLU 	              ( RSS_( 4u ).data_u8[0] )
#define CHEZHANHAO  		          ( RSS_( 4u ).data_u8[2] )
#define ZONGZHONG 	 		          ( RSS_( 1u ).data_u8[0] )
#define HUANCHANG  			          ( RSS_( 1u ).data_u8[2] )
#define LIANGSHU  			          ( RSS_( 1u ).data_u8[4] )
#define CHESUDENGJI  		          ( RSS_( 0u ).data_u8[6] )
#define JICHEXINGHAO  	              ( RSS_( 2u ).data_u8[2] )
#define JICHEHAO 	 			      ( RSS_( 2u ).data_u8[0] )
#define JICHEZHONGLEI		          ( RSS_( 3u ).data_u8[7] )

/* 运行状态信息 */
/* 新协议标准 */
#define JIANKONGSTATE		          ( QSS_( 4u ).data_u8[7] )

/* TCMS采集信息 */

#define ato_jiwei_T54                 ( CAN_0x32( 6u ).data_u8[5] )
#define ato_gk_T55                    ( CAN_0x32( 6u ).data_u8[6] )
#define TCMSJIWEI				      ato_jiwei_T54
#define TCMSGONGKUANG		          ( ato_gk_T55 & 0x07 )

/* 指导信息 */
#if 0  //TODO(mingzhao)
#define Sto_CurSpeed_S13              ( CAN_0x30( 1u ).data_u8[3] )
#else
#define Sto_CurSpeed_S13              ( QSS_( 5u ).data_u8[0] )
#endif
#define ZHIDAOJIWEI			          ( QSS_( 5u ).data_u8[2] )
#define ZHIDAOSUDU			          Sto_CurSpeed_S13
#define ZHIDAOGONGKUANG               ( QSS_( 5u ).data_u8[7] )
#define STOXIANSU                     ( QSS_( 7u ).data_u8[0] )      //增加CAN协议内容

/* 控车信息 */
#define KONGZHIMOSHI			    ( QSS_( 2u ).data_u8[6] )
#define YOUHUAMOSHI				    ( QSS_( 3u ).data_u8[7] )
#define YOUHUADENGXINHAO	        ( QSS_( 5u ).data_u8[6] )
#define SKHYOUQUANDUAN		        ( QSS_( 3u ).data_u8[6] )
#if 0 //TODO(mingzhao)
#define	KONGZHIGONGKUANG	        ( CAN_KONGCHE( 0u ).data_u8[1] )
#define KONGZHIJIWEI			    ( CAN_KONGCHE( 0u ).data_u8[2] )
#else
#define KONGZHIGONGKUANG            ( QSS_( 2u ).data_u8[7] )
#define KONGZHIJIWEI                ( QSS_( 2u ).data_u8[5] )
#endif
#define SHOUBINGHUILING		        ( QSS_( 6u ).data_u8[ 6u ] )
/* 12-June-2018, by Liang Zhen. */
#define ABV_CTRL_MSG                ( CAN_KONGCHE( 0u ).data_u8[1] )

/* 公共信息 */
#define SHISU                 ( CAN_0x66( 0u ).data_u8[0] )
#define LKJXIANSU             ( CAN_0x66( 0u ).data_u8[2] ) 
//#define JICHEXINHAODAIMA      ( QSS_( 0u ).data_u8[7] )
#define JICHEXINHAODAIMA      ( CAN_0x66( 1u ).data_u8[2] )
#define JUlI                  ( CAN_0x66( 2u ).data_u8[3] )
#if 0   //TODO(mingzhao)
#define LIECHEGUANYA          ( CAN_0x32( 0u ).data_u8[0] )
#define ZHAGANGYALI           ( CAN_0x32( 0u ).data_u8[6] )
#define JUNGANG1YALI          ( CAN_0x32( 0u ).data_u8[4] )
#else
#define LIECHEGUANYA          ( CAN_0x68( 0u ).data_u8[0] )
#define ZHAGANGYALI           ( CAN_0x68( 0u ).data_u8[2] )
#define JUNGANG1YALI          ( CAN_0x68( 0u ).data_u8[4] )
#endif
#define CHAIYOUJIZHUANSU      ( CAN_0x68( 1u ).data_u8[6] )
#define GUOJIZHUNDIANJULI     ( QSS_( 1u ).data_u8[0] )
#define GONGLIBIAO            ( QSS_( 1u ).data_u8[4] )
#define JINGGAOBIAOZHI        ( QSS_( 6u ).data_u8[0] )

/* 板间自检信息 */
#define ZK1_ZJ	              ( CAN_0X63_( 0u ).data_u8[0] )
#define ZK2_ZJ	              ( CAN_0X73_( 0u ).data_u8[0] )
#define	JCTX_ZJ	              ( CAN_0X50_( 0u ).data_u8[0] )
#define JCJK_ZJ	              ( CAN_0X90_( 0u ).data_u8[0] )
#define XSQ1_ZJ               ( CAN_0X45_( 0u ).data_u8[2] )
#define XSQ2_ZJ               ( CAN_0X46_( 0u ).data_u8[2] )

/* 版本信息 */
#define ZK1_BB	              ( CAN_0X63_( 0u ).data_u8[2] )
#define ZK2_BB	              ( CAN_0X73_( 0u ).data_u8[2] )
#define JCTX_BB	              ( CAN_0X50_( 0u ).data_u8[2] )
#define JCJK_BB	              ( CAN_0X90_( 0u ).data_u8[2] )

/********************************************************************************************/

/* 协议解析文件头 */
#define LIEWEICHANGJIA            ( CAN_0x6A( 0u ).data_u8[4] )
#define ZHIDONGJICHANGJIA         ( CAN_0x6A( 0u ).data_u8[5] )
#define TCMSCHANGJIA              ( CAN_0x6A( 0u ).data_u8[6] )
#define LKJCHANGJIA               ( CAN_0x6A( 0u ).data_u8[7] )
//#define TIME_NYR                                      //CAN协议已有，名称一致
//#define TIME_SFM                                      //CAN协议已有，名称一致
#define YUNXINGJINGLUHAO	       ( SHUJUJIAOLU<<8 + JIANKONGJIAOLU )
#define LKJFACHEFANGXIANG	       CHEZHANHAO
#define QIANGZHIBENGFENG           (s_packet_gradeInfo.received_data_u8[23] & 0x10)
#define CHEZHANMING                ETH_DAT[1]           //CAN协议没有定义
//#define JICHEXINGHAO                                  //CAN协议已有，名称一致
#define XITONGKONGZHI              ( CAN_0x67( 5u ).data_u8[4] )
#define JICHELEIXING               ( (*(&XITONGKONGZHI + 1u)) & 0x01 )
//#define JICHEHAO	                                    //CAN协议已有，名称一致
#define JUDUANHAO                  ETH_DAT[2]           //CAN协议没有定义
#define CHEZHONGBIAOSHI            CHECIKUOCHONG
#define CHECIHAO	               CHECI
#define LIECHESHUXING              JICHEZHONGLEI
#define SIJIHAO1                   SIJI1
#define SIJIHAO2                   SIJI2
#define BENXIZHUANGTAI             ( CAN_0x91( 1u ).data_u8[1] )
#define GONGZUOZHUANGTAI	       KONGZHIMOSHI

#define AJIKONGZHIRUANJIANBANBEN	    ( CAN_0X63_( 0u ).data_u8[2] )
#define BJIKONGZHIRUANJIANBANBEN	    ( CAN_0X73_( 0u ).data_u8[2] )
#define AJISTOJICHUSHUJUBANBENRIQI      ( CAN_0X63_( 1u ).data_u8[0] )
#define BJISTOJICHUSHUJUBANBENRIQI      ( CAN_0X73_( 1u ).data_u8[0] )
#define AJISTOJICHUSHUJUBIANYIRIQI      ( CAN_0X63_( 1u ).data_u8[2] )
#define BJISTOJICHUSHUJUBIANYIRIQI      ( CAN_0X73_( 1u ).data_u8[2] )
#define AJISTOKONGZHICANSHUBANBENRIQI   ( CAN_0X63_( 1u ).data_u8[4] )
#define BJISTOKONGZHICANSHUBANBENRIQI   ( CAN_0X73_( 1u ).data_u8[4] )
#define AJISTOKONGZHICANSHUBIANYIRIQI   ( CAN_0X63_( 1u ).data_u8[6] )
#define BJISTOKONGZHICANSHUBIANYIRIQI   ( CAN_0X73_( 1u ).data_u8[6] )
#define AJILKJSHUJUBANBEN               ETH_DAT[4]           //CAN协议没有定义
#define BJILKJSHUJUBANBEN               ETH_DAT[4]           //CAN协议没有定义
#define AJILKJSHUJUSHIJIAN              ETH_DAT[4]           //CAN协议没有定义
#define BJILKJSHUJUSHIJIAN	            ETH_DAT[4]           //CAN协议没有定义

/* 公共信息 */
#define loco_T34                   ( CAN_0x32( 4u ).data_u8[1] )
#define CHONGLIANCHE               ( (loco_T34 & 0x30) >> 4u )
#define axle1_jicheli_T37          ( CAN_0x32( 4u ).data_u8[4] )
#define axle2_jicheli_T38          ( CAN_0x32( 4u ).data_u8[5] )
#define axle3_jicheli_T39          ( CAN_0x32( 4u ).data_u8[6] )
#define axle4_jicheli_T40          ( CAN_0x32( 4u ).data_u8[7] )
#define axle5_jicheli_T41          ( CAN_0x32( 5u ).data_u8[0] )
#define axle6_jicheli_T42          ( CAN_0x32( 5u ).data_u8[1] )
#define axle1_CLjicheli_T75        ( CAN_0x33( 1u ).data_u8[2] )
#define axle2_CLjicheli_T76        ( CAN_0x33( 1u ).data_u8[3] )
#define axle3_CLjicheli_T77        ( CAN_0x33( 1u ).data_u8[4] )
#define axle4_CLjicheli_T78        ( CAN_0x33( 1u ).data_u8[5] )
#define axle5_CLjicheli_T79        ( CAN_0x33( 1u ).data_u8[6] )
#define axle6_CLjicheli_T80        ( CAN_0x33( 1u ).data_u8[7] )

#define JULI                       JUlI          //CAN协议增加LKJ强实时信息透传
#define LICHENG                    GONGLIBIAO    //公里标即为里程
#define JICHEXINHAO                JICHEXINHAODAIMA
#define XIANSU                     LKJXIANSU
#define LKJSUDU                    SHISU
//#define GONGZUOZHUANGTAI           KONGZHIMOSHI  //文件头信息已定义
#define GONGZUOMOSHI               YOUHUAMOSHI
#define JICHEFAHUIGONGKUANG        TCMSGONGKUANG 
#define JICHEFAHUISHOUBINGJIWEI    TCMSJIWEI
#define CCUSUDU                    ( CAN_0x32( 4u ).data_u8[2] )
#define LIECHEGUANYALI             LIECHEGUANYA 
#define ZHIDONGGANGYALI            ZHAGANGYALI  
#define JUNFENGGANGYALI            JUNGANG1YALI   
#define CHAIZHUANDIANLIU           CHAIYOUJIZHUANSU 


/* 板间自检信息 */
#define ZK_ZJ	                 ZK1_ZJ
#define	TX1_ZJ	                 JCTX_ZJ
#define JL_ZJ                    ERROR_FLAG
#define WXTX_ZJ                  ETH_DAT[1]      //CAN协议没有定义
#define	WJJK_ZJ	                 ( CAN_0x82( 0u ).data_u8[0] )
#define CEU_ZJ                   ( CAN_0x28( 1u ).data_u8[4] )     
#define ECU_ZJ                   ETH_DAT[1]      //CAN协议没有定义

/* 主机与外部设备通信信息 */
#define Ato_Enter_Auto_S96        ( CAN_0x31( 3u ).data_u8[7] )
#define LKJ_COMMUNICATION	      ( Ato_Enter_Auto_S96 & 0x01 )
#define	CCU_COMMUNICATION	      ( (CAN_0x6A( 0u ).data_u8[0] & 0x01) >> 0u )
#define BCU_COMMUNICATION	      ( (CAN_0x6A( 0u ).data_u8[0] & 0x02) >> 1u )
#define CIR_COMMUNICATION         ( s_packet_gradeInfo.received_data_u8[22] )
#define CEU_COMMUNICATION         ETH_DAT[1]      //CAN协议没有定义
#define	ECU_COMMUNICATION	      ETH_DAT[1]      //CAN协议没有定义

/* 司机操作信息 */
#define MINGLINGHAOI              ( CAN_0X45_( 1u ).data_u8[2] )    
#define MINGLINGNEIRONGI          ( CAN_0X45_( 1u ).data_u8[3] )
#define MINGLINGHAOII             ( CAN_0X46_( 1u ).data_u8[2] )
#define MINGLINGNEIRONGII         ( CAN_0X46_( 1u ).data_u8[3] ) 
#define DDUCAOZUO                 ETH_DAT[1]      //CAN协议没有定义
#define LDENGJI                   ( CAN_0x32( 7u ).data_u8[7] )
#define SUDUXIAFUZHI              ( CAN_0x32( 7u ).data_u8[6] )
#define GUANMENCHE                ( CAN_0x6A( 1u ).data_u8[1] )
#define HUANSUANZHAWAYALI         ( CAN_0x6A( 1u ).data_u8[2] )
#define GELISTO                   ((Enter_Autopilot_S1 & 0x20) >> 5u )


/* STO行程规划信息 */
//#define ZHIDAOSUDU	                //CAN协议已有，名称一致
//#define	ZHIDAOGONGKUANG	            //CAN协议已有，名称一致
//#define ZHIDAOJIWEI	                //CAN协议已有，名称一致
#define ZHIDAOXIANSU                  STOXIANSU
#define YOUHUAJIEGUO                  ( CAN_0x6A( 5u ).data_u8[4] )
#define YOUHUAHAOSHI                  ( CAN_0x6A( 5u ).data_u8[6] )

#define ZAOWANDIANBIAOZHI             ( QSS_( 4u ).data_u8[3] )
#define XIANSHIKONGZHI                ( CAN_0x66( 3u ).data_u8[0] )
#define GUOZHANZHONGXIN               ( (XIANSHIKONGZHI & 0x02) >> 1u )
#define DAOZHANBIAOZHUNSHIJIAN        ( QSS_( 3u ).data_u8[3] )
#define YUJIDAOZHANSHIJIAN            ( QSS_( 4u ).data_u8[4] )

/* STO控车信息 */
#define Enter_Autopilot_S1            ( CAN_0x30( 0u ).data_u8[0] )
#define Sto_GKCom_S10                 ( CAN_0x30( 1u ).data_u8[1] ) 
#define YUNXUFUZHUJIASHI	          ((Enter_Autopilot_S1 & 0x08) >> 3u)
#define	ZHENGCHEJINRUFUZHUJIASHI	  (Enter_Autopilot_S1 & 0x03)
#define JINRUFUZHUJIASHI	          ((Enter_Autopilot_S1 & 0x10) >> 4u )
#define ZHENGCHETUICHUFUZHUJIASHI     ((Enter_Autopilot_S1 & 0x04) >> 2u )
#define TUICHUFUZHUJIASHI             ( CAN_0x6A( 5u ).data_u8[0] )
#define YUNXUQIDONG	                  ( CAN_0x6A( 0u ).data_u8[3] )
#define	KONGZHILIECHEQIDONG	          ( (*(&WENBENTISHI + 1U)) & 0x01) 
#define KONGCHEGONGKUANG	          KONGZHIGONGKUANG
#define KONGCHEJIWEI                  KONGZHIJIWEI
#define KEKONGBIAOZHI                 ( ((*(&WENBENTISHI + 1U)) & 0x10) >> 4U )
#define FENXIANGHUILING               ( QSS_( 6u ).data_u8[6] )
#define WENBENTISHI2                  ( QSS_( 2u ).data_u8[2] )
#define GUANTONGJIEGUO                ( CAN_0x6A( 4u ).data_u8[6] )
#define KONGZHIZHUANGTAI              ( QSS_( 7u ).data_u8[4] )
#define TINGCHEBAOYA                   ( (*(&WENBENTISHI2 + 1)) & 0x01 )
#define WENBENTISHI                    ( RSS_( 7u ).data_u8[0] )
#define YUYINDAIMA                     ( CAN_0x31( 1u ).data_u8[4] )
//#define GONGZUOZHUANGTAI             //公共信息已定义
//#define GONGZUOMOSHI                 //公共信息已定义
#define JINGTAICESHIZHUANGTAI          ( CAN_0x6A( 2u ).data_u8[2] )
#define KONGZHISASHA                   ( (Sto_GKCom_S10 & 0x08) >> 3u )
#define QIANGZHIBENGFENG               ( (Sto_GKCom_S10 & 0x10) >> 4u )

/* 机车制动系统信息 */
//#define LIECHEGUANYALI               s_packet_gradeInfo.received_data_u8[26]  //公共信息已定义
//#define ZHIDONGGANGYALI              s_packet_gradeInfo.received_data_u8[28]  //公共信息已定义
#define ZONGFENGGANGYALI               ( CAN_0x32( 0u ).data_u8[2] )
//#define JUNGANGYALI                    JUNFENGGANGYALI  
#define ZIZHIDONGSHOUBING              ( CAN_0x32( 2u ).data_u8[2] )
#define DANDUZHIDONGSHOUBING           ( CAN_0x32( 2u ).data_u8[3] ) 
#define XIAOZHACEHUAN                  ( CAN_0x75( 2u ).data_u8[4] )  
#define BCUCHONGFENGLIULIANG           ( CAN_0x32( 1u ).data_u8[2] )
#define BCUZHUANGTAI                   ( (CAN_0x32( 2u ).data_u8[1] & 0x03) >> 0u )
#define BCUCHENGFAZHIDONG              ( (CAN_0x32( 2u ).data_u8[0] & 0x20) >> 5u )
#define BCUFUZHUJIASHIZHUANGTAI        ( CAN_0x32( 2u ).data_u8[6] )
#define BCUYUNXUTIAOJIAN               ( CAN_0x32( 3u ).data_u8[4] )
#define BCUCHANGJIA	                   ( CAN_0x32( 1u ).data_u8[4] )
#define BCUGUZHANGDAIMA	               ( CAN_0x32( 3u ).data_u8[0] )

/* 机车牵引系统信息 */
#define axle_cut_T47                   ( CAN_0x32( 5u ).data_u8[6] )
#define ctrl_stat_T48                  ( CAN_0x32( 5u ).data_u8[7] )
#define tanTing_saSha_T49              ( CAN_0x32( 6u ).data_u8[0] )
#define skq_stat_T50                   ( CAN_0x32( 6u ).data_u8[1] )
#define jicheli_per_T51                ( CAN_0x32( 6u ).data_u8[2] )
#define CLjicheli_per_T82              ( CAN_0x33( 2u ).data_u8[1] )
#define skq_jiwei_T52                  ( CAN_0x32( 6u ).data_u8[3] )
#define auto_stat_T53                  ( CAN_0x32( 6u ).data_u8[4] )
#define CESHIZHUANGTAIJIJIEGUO         ( CAN_0x6A( 2u ).data_u8[2] )

#define WULISHOUBINGJIWEI              skq_jiwei_T52
#define WULIJICHEGONGKUANG             ( (skq_stat_T50 & 0x70) >> 4u )
#define DIANJIGELIZHUANGTAI            (axle_cut_T47 & 0x3F)
#define JICHELIKEYONGBILI              jicheli_per_T51
#define CLJICHELIKEYONGBILI            CLjicheli_per_T82
#define ZHOU1LI                        axle1_jicheli_T37
#define ZHOU2LI                        axle2_jicheli_T38
#define ZHOU3LI                        axle3_jicheli_T39
#define ZHOU4LI                        axle4_jicheli_T40
#define ZHOU5LI                        axle5_jicheli_T41
#define ZHOU6LI                        axle6_jicheli_T42
#define JICHEKONGZHUAN                 ( (ctrl_stat_T48 & 0x01) >> 0u )
#define JICHEHUAXING                   ( (ctrl_stat_T48 & 0x02) >> 1u )
#define SHOUDIANGONGZHUANGTAI          ( (ctrl_stat_T48 & 0x04) >> 2u )
#define ZHUDUANZHUANGTAI               ( (ctrl_stat_T48 & 0x08) >> 3u )
#define QIANYINFENGSUO                 ( (ctrl_stat_T48 & 0x20) >> 5u )
#define DIANZHIFEGNSUO                 ( (ctrl_stat_T48 & 0x10) >> 4u )
#define YUDUANYOUXIAO                  ( (ctrl_stat_T48 & 0x40) >> 6u )
#define QIANGDUANHOUXIAO               ( (ctrl_stat_T48 & 0x80) >> 7u )
#define TINGFANGZHIDONG                ( (tanTing_saSha_T49 & 0x01) >> 0u )
#define SASHAZHUANGTAI                 ( (tanTing_saSha_T49 & 0x02) >> 1u )
#define KONGDIANLIANHEZHUANGTAI        ( (tanTing_saSha_T49 & 0x04) >> 2u ) 
#define CCUFUZHUZHUANGTAI              auto_stat_T53
#define CCUYUNXUCESHI                  ( (CESHIZHUANGTAIJIJIEGUO & 0x20) >> 5u )

#define prinary_Vol_T44                ( CAN_0x32( 5u ).data_u8[2] )
#define prinary_Al_T46                 ( CAN_0x32( 5u ).data_u8[4] )
#define YUANBIANDIANYA                 prinary_Vol_T44
#define YUANBIANDIANLIU                prinary_Al_T46
#define DIANYAOSHIZHUANGTAI            ( (skq_stat_T50 & 0x03) >> 0u )

/* LKJ系统信息 */
#define HUOQUJIESHITIAOSHU             ( RSS_( 1u ).data_u8[5] )
#define SHENQINGJIESHI                 ETH_DAT[1]      //CAN协议没有定义
#define JIESHIXUHAO                    ETH_DAT[1]      //CAN协议没有定义
#define JIESHINEIRONG                  ETH_DAT[1]      //CAN协议没有定义
//#define SIJIHAO1                                     //文件头已有定义，名称一致
//#define SIJIHAO2                                     //文件头已有定义，名称一致
#define YUNXINGJINGLU                  YUNXINGJINGLUHAO
//#define FACHEFANGXIANG                               //文件头已有定义，名称一致
//#define ZONGZHONG                                    //文件头已有定义，名称一致
#define JICHANG                        HUANCHANG
//#define LIANGSHU                                     //文件头已有定义，名称一致
#define ZAIZHONG                       ( CAN_0x67( 2u ).data_u8[0] )    //增加CAN协议LKJ弱实时信息1透传
#define KECHE                          ( CAN_0x67( 2u ).data_u8[2] )    //增加CAN协议LKJ弱实时信息1透传
#define ZHONGCHE                       ( CAN_0x67( 2u ).data_u8[3] )    //增加CAN协议LKJ弱实时信息1透传
#define KONGCHE                        ( CAN_0x67( 2u ).data_u8[4] )    //增加CAN协议LKJ弱实时信息1透传
#define FEIYUNYONGCHE                  ( CAN_0x67( 2u ).data_u8[5] )    //增加CAN协议LKJ弱实时信息1透传
#define DAIKECHE                       ( CAN_0x67( 2u ).data_u8[6] )    //增加CAN协议LKJ弱实时信息1透传
#define SHOUCHE                        ( CAN_0x67( 2u ).data_u8[7] )    //增加CAN协议LKJ弱实时信息1透传
//#define CHESUDENGJI                                  //文件头已有定义，名称一致

/* 列车运行信息 */
//#define FACHEFANGXIANG                               //文件头已定义
//#define CHEZHANMING                                  //文件头已定义
#define JIANKONGZHUANGTAI               ( CAN_0x66( 3u ).data_u8[6] )
#define LKJGONGZUOMOSHI                 JIANKONGZHUANGTAI
#define CAOZUOYUNXU                     ( CAN_0x66( 3u ).data_u8[2] )
#define KAICHEDUIBIAO                   ( (CAOZUOYUNXU & 0x02) >> 1u )
#define ZHIXIANHAO                      ( CAN_0x66( 4u ).data_u8[2] )    
#define CEXIANHAO                       ( CAN_0x66( 4u ).data_u8[3] )   
#define ZHIDONGSHUCHU                   ( CAN_0x66( 1u ).data_u8[6] )
#define QIANFANGCHEZHANHAO              ( QSS_( 4u ).data_u8[0] )
#define XINHAOJILEIXING                 ( CAN_0x66( 2u ).data_u8[7] )
#define XINHAOJIBIANHAOZIFUTOU           ETH_DAT[1]      //CAN协议没有定义
#define XINHAOJIBIANHAO                 ( CAN_0x66( 2u ).data_u8[5] )
#define XINHAOJIZHUANGTAI                JICHEXINHAODAIMA
//#define lLKJSUDU                       //公共信息已定义
//#define XIANSU                         //公共信息已定义
#define GUOFENXIANG                     ( RSS_( 1u ).data_u8[7] )
#define XIANLUSHUJUZHONGZHI              ETH_DAT[1]      //CAN协议没有定义
#define ATOYUNXUFUZHUTIAOJIAN           ( CAN_0x6A( 3u ).data_u8[1] )
#define SHUJUGUZHANG                    ( ATOYUNXUFUZHUTIAOJIAN & 0x01 )


/* 版本信息 */
#define ZK_I_A_BB           ZK1_BB
#define ZK_I_B_BB           ZK2_BB
#define ZK_II_A_BB          ZK1_BB
#define ZK_II_B_BB          ZK2_BB
#define XSQ_I_BB            ( CAN_0X45_( 5u ).data_u8[2] )
#define XSQ_II_BB           ( CAN_0X46_( 5u ).data_u8[2] )
#define TX_I_BB             JCTX_BB
#define TX_II_BB            JCTX_BB

#define WJJK_I_BB           ( CAN_0x82( 0u ).data_u8[2] )
#define WJJK_II_BB          ( CAN_0x82( 0u ).data_u8[2] )
#define JL_BB               Version[0]      
#define WXTX_BB             ETH_DAT[1]      //CAN协议没有定义
#define CEU_I_BB            ETH_DAT[1]      //CAN协议没有定义
#define CEU_II_BB           ETH_DAT[1]      //CAN协议没有定义
#define ECU_I_BB            ETH_DAT[1]      //CAN协议没有定义
#define ECU_II_BB           ETH_DAT[1]      //CAN协议没有定义
#define CCU_BB              ETH_DAT[1]      //CAN协议没有定义
#define BCU_BB              s_packet_gradeInfo.received_data_u8[7]

#define RUANJIANBANBENBUYIZHI    ( WENBENTISHI & 0x01 )

#define GANG_YA_PRESSURE_DVALUE  (160)  //压力差值，单位kPa  10 * 16  比例系数为16

/* public type definition ---------------------------------------------------------------------- */

/* 文件目录结构体 128 字节
 * 该结构体修改时需要修改 宏定义：SFILE_DIR_SIZE
 * */
typedef struct __attribute__((packed)) _SFile_Directory /* 按照字节对齐*/ //TODO(mingzhao)
{
    char ch_checi[4]; /* 车次 */
    char ch_checikuochong[4]; /* 车次扩充 */
    char ch_siji[4]; /* 司机号 */
    char ch_date[4]; /* 日期 */
    char ch_time[4]; /* 时间 */
    uint32_t u32_over_flag; /* 结束标志 */
    uint32_t u32_file_size; /* 文件实际大小 */
    char ch_file_name[FILE_NAME_MAX_NUM];     /* 记录文件名 */
    char ch_dir_name[FILE_NAME_MAX_NUM];     /* 目录文件名 */
    char ch_benbuzhuangtai[1]; /* 本补状态 */
    char ch_reserve[3]; /* 预留 */
    uint8_t file_id;   /* 文件ID */  //TODO(mingzhao) 对接到文件链表中使用
    uint8_t is_save;     /* 是否转存过  1：转存 0：未转存 */
} SFile_Directory;


/* 包头信息 */
typedef struct __attribute__((packed))
{
    uint8_t file_num;
    uint8_t file_count;
    uint8_t lenth;
    uint32_t total_package;
    uint32_t current_package;
    uint16_t crc;
    uint8_t file_name[FILE_NAME_MAX_NUM];
    uint8_t reserve;
} PACKAGE_HEAD;

/* 公共信息事件包结构体 34字节 */
typedef struct __attribute__((packed))
{
  char ch_time[3];        /* 时间 */
  char ch_juli[4];        /* 距离 */
  char ch_licheng[4];     /* 里程 */
  char ch_jichexinhao[1];    /* 机车信号 */
  char ch_xiansu[2];      /* 限速 */
  char ch_LKJsudu[2];     /* LKJ速度 */
  char ch_gongzuozhuangtai[1];     /* 工作状态 */
  char ch_gongzuomoshi[1];         /* 工作模式 */
  char ch_xitongzhuangtai[1];      /* 系统状态 */
  char ch_yuliu[1];                /* 预留 */
  char ch_jichefahuigongkuang[1];  /* 机车发挥工况 */
  char ch_jichefahuishoubingjiwei[1];   /* 机车发挥手柄级位 */
  char ch_CCUsudu[2];           /* CCU速度 */
  char ch_qianyinzhidong[2];    /* 牵引/制动力 */
  char ch_liecheguanyali[2];    /* 列车管压力 */
  char ch_zhidonggangyali[2];   /* 制动缸压力 */
  char ch_junfenggangyali[2];   /* 均风缸压力 */
  char ch_chaizhuandianliu[2];   /* 柴转/电流 */
} SFile_Public;

/* 文件头结构体 128字节 */
typedef struct __attribute__((packed))
{
    char ch_head_flag[2];                 /* 文件头标志 */
    char ch_yuliu1[2];                    /* 预留 */
    char ch_jilugeshibanbenhao[4];        /* 记录板格式 */
    char ch_waisheleixing[2];             /* 外设类型 */
    char ch_create_time[6];               /* 建立文件时间  设定内容变化造成文件头变化时不修改该时间，实际计算 byte0 + 2000  年月日时分秒 */
    char ch_yunxingjiaoluhao[4];          /* 运行径路号 */
    char ch_LKJfachefangxiang[4];         /* LKJ发车方向 */
    char ch_chezhan[12];                  /* 车站 */
    char ch_jichexinghao[2];              /* 机车型号 */
    char ch_jicheleixing[1];              /* 机车类型 */
    char ch_yuliu2[1];                    /* 预留 */
    char ch_jichehao[4];                  /* 机车号 */
    char ch_juduanhao[2];                 /* 机车配属局段号 */
    char ch_chezhongbiaoshi[4];           /* 车种标识 */
    char ch_checihao[4];                  /* 车次号码 */
    char ch_liecheshuxing[1];             /* 列车属性 */
    char ch_yuliu3[1];                    /* 预留 */
    char ch_siji1[4];                     /* 司机1 */
    char ch_siji2[4];                     /* 司机2 */
    char ch_zongzhong[2];                 /* 总重 */
    char ch_jichang[2];                   /* 计长 */
    char ch_liangshu[1];                  /* 量数 */
    char ch_yuliu4[9];                    /* 预留 */
    char ch_shebeizhuangtai[1];           /* 设备状态 */
    char ch_gongzuozhuangtai[1];          /* 工作状态 */
    char ch_Ajikongzhiruanjianbanben[4];  /* A机控制软件版本 */
    char ch_Bjikongzhiruanjianbanben[4];  /* B机控制软件版本 */
    char ch_AjiSTOjichushujubanben[4];    /* A机STO基础数据版本日期 */
    char ch_BjiSTOjichushujubanben[4];    /* B机STO基础数据版本日期 */
    char ch_AjiSTOkongzhicanshu[4];       /* A机STO控制参数版本日期 */
    char ch_BjiSTOkongzhicanshu[4];       /* B机STO控制参数版本日期 */
    char ch_AjiLKJshujubanben[4];         /* A机LKJ数据版本日期 */
    char ch_BjiLKJshujubanben[4];         /* B机LKJ数据版本日期 */
    char ch_AjiLKJshujushijian[4];        /* A机LKJ数据时间日期 */
    char ch_BjiLKJshujushijian[4];        /* B机LKJ数据时间日期 */
    char ch_wenjianneirongCRC[4];         /* 文件内容CRC32 */
    uint32_t u32_CRC32;                   /* CRC32 */
} SFile_Head;

/* 文件体 20 字节*/
typedef struct __attribute__((packed))
{
    uint16_t shijiandaima;
    char gonggongxinxi[7];
    char shijianchangdu[1];
    char jilunneirong[9];
    char xiaoyanma;
} FILE_CONTANT;

/* FRAM写缓冲区 257 */
typedef struct __attribute__((packed))
{
    uint8_t pos;
    uint8_t buf[256];
} WRITE_BUF;

/* 插件自检状态 */
typedef enum
{
    SelfChackNORMAL = 0U,
    SelfChackFAULT
} ESelfChack_State;

/* 外部设备通信状态 */
typedef enum
{
    SUSPEND = 0U,
    RESUME = 1U,
//  ABNORMAL
} ECommunicetion_State;

typedef struct __attribute__((packed)) _S_CURRENT_FILE_INFO /* 按照字节对齐*/
{
    SFile_Directory *file_dir;
    SFile_Head      *file_head;
    WRITE_BUF       *write_buf;
    off_t new_record_head_offset;      /* 最新记录文件的文件头偏移量 */
} S_CURRENT_FILE_INFO;

extern WRITE_BUF write_buf;
//extern FLASH_STATE Flash_State;
extern uint8_t SoftWare_Cycle_Flag;
extern uint8_t u8_Gonggongxinxi_Flag;
extern SFile_Head s_file_head;
extern SFile_Directory s_File_Directory;

/* public function declaration ----------------------------------------------------------------- */
//void RecordBoard_FileCreate(void);
//void Init_FlashState(void);
void RecordBoard_FileCreate(void);

/* 12-June-2018, by Liang Zhen. */
void Update_ABV_ControllingMessage( uint8_t msg[] );
void Update_IBV_ControllingMessage( uint8_t msg[] );

/* 13-June-2018, by Liang Zhen. */
void Update_ABV_MonitorMessage( uint8_t msg[], uint8_t low3bit );
void Update_IBV_MonitorMessage( uint8_t msg[], uint8_t low3bit );

void Update_ABV_HandleMessage( uint8_t msg[], uint8_t low3bit );
void Update_IBV_HandleMessage( uint8_t msg[], uint8_t low3bit );

/* 14-June-2018, by Liang Zhen. */
void Update_HLRT_SpeechTipsMessage( uint8_t msg[] );

void Update_LKJ_LLRT_Message( uint8_t msg[] );

/* 28-September-2018, by Liang Zhen. */
void Update_LKJ_HLRT_Message( uint8_t msg[] );

void RecordingPowerOnMessage(void);
void RecordingPowerOffMessage(void);
void RecordingLLevelMessage( void );
void RecordingSpeedDownMessage( void );

void WriteFileContantPkt(uint8_t num1, uint8_t num2, uint8_t device_code, uint8_t *contant, uint8_t lenth);
uint16_t FFFEEncode(uint8_t *u8p_SrcData, uint16_t u16_SrcLen, uint8_t *u8p_DstData);
#endif




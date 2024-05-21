/* ***************************************************
 * 文件名： app.h
 * 模  块： 应用层解析模块
 *
 *
 ****************************************************/

#ifndef APP_H
#define APP_H

#include "comm_proc_ctl.h"

/** 类POSIX系统返回值 */
#if !defined(__cplusplus)
  #undef  ER
  #define ER -1
#endif

#define  nBOARD_SLOT_UNKNOWN      0
#define  nBOARD_SLOT_I_1          0x01			/* I系通信插件1 */
#define  nBOARD_SLOT_I_2          0x02			/* I系通信插件2 */
#define  nBOARD_SLOT_II_1         0x05			/* II系通信插件1 */
#define  nBOARD_SLOT_II_2         0x06			/* II系通信插件2 */


/** 板上硬件通道定义 */
typedef enum  
{
	Invalid_DEV = 0,
  IN_FDCAN_1_DEV,
	IN_FDCAN_2_DEV,
	EXP_FDCAN_3_DEV,
  EXP_FDCAN_4_DEV,
  EXP_FDCAN_5_DEV,	
  EXP_KSZ8851_I_DEV,
  EXP_KSZ8851_II_DEV,
  IN_MII_ETH_DEV,
	RS422_DEV,
  HDLC_DEV,
  MVB_DEV
}E_DEV;

/**********************************************************
**		内部安全层地址定义
**********************************************************/
//#ifdef	STO_V2		//二代STO
//#define  Safe_ZK_I_ADR						3					/* I系主控地址 */
//#define  Safe_TX1_I_L_ADR					71				/* I系通信1底板地址 */
//#define  Safe_TX1_I_C_ADR					6					/* I系通信1子板地址 */
//#define  Safe_TX2_I_L_ADR					57				/* I系通信2底板地址 */
//#define  Safe_TX2_I_C_ADR					72				/* I系通信2子板地址 */
//#define  Safe_ZK_II_ADR						3					/* II系主控地址 */
//#define  Safe_TX1_II_L_ADR				71			  /* II系通信1底板地址 */
//#define  Safe_TX1_II_C_ADR				6					/* II系通信1子板地址 */
//#define  Safe_TX2_II_L_ADR				57				/* II系通信2底板地址 */
//#define  Safe_TX2_II_C_ADR				72				/* II系通信2子板地址 */
//
//#else							//三合一
//#define  Safe_ZK_I_ADR						9					/* I系主控地址 */
//#define  Safe_TX1_I_L_ADR					4					/* I系通信1底板地址 */
//#define  Safe_TX1_I_C_ADR					5					/* I系通信1子板地址 */
//#define  Safe_TX2_I_L_ADR					6					/* I系通信2底板地址 */
//#define  Safe_TX2_I_C_ADR					7					/* I系通信2子板地址 */
//#define  Safe_ZK_II_ADR						10				/* II系主控地址 */
//#define  Safe_TX1_II_L_ADR				11				/* II系通信1底板地址 */
//#define  Safe_TX1_II_C_ADR				12				/* II系通信1子板地址 */
//#define  Safe_TX2_II_L_ADR				13				/* II系通信2底板地址 */
//#define  Safe_TX2_II_C_ADR				14				/* II系通信2子板地址 */
//
//#endif
/**********************************************************/
/**********************************************************/

extern uint8  InCAN1_diff_flag, InCAN2_diff_flag, InETH_diff_flag;
extern uint8  InCAN1_2_MainCtl_en, InCAN2_2_MainCtl_en, InETH_2_MainCtl_en;
extern uint32 InCAN1_SendDelay_timer, InCAN2_SendDelay_timer, InETH_SendDelay_timer;

sint8 rx_safe_layer_check(uint8 *pBuf , uint8 from_chl);

#endif /** APP_H */


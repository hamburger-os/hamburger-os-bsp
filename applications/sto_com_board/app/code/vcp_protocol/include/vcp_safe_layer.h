/***************************************************************************
文件名：vcp_app_layer.h
模    块：
详    述：
作    者：jiaqx 20231128
***************************************************************************/

#include "comm_proc_ctl.h"
#include "common.h"

#ifndef VCP_SAFE_LAYER_H
#define VCP_SAFE_LAYER_H

/* 内部安全层地址定义 */
#define  Safe_ZK_I_ADR       3      /* I系主控地址 */
#define  Safe_TX1_I_L_ADR    71     /* I系通信1底板地址 */
#define  Safe_TX1_I_C_ADR    6      /* I系通信1子板地址 */
#define  Safe_TX2_I_L_ADR    57     /* I系通信2底板地址 */
#define  Safe_TX2_I_C_ADR    72     /* I系通信2子板地址 */
#define  Safe_ZK_II_ADR      3      /* II系主控地址 */
#define  Safe_TX1_II_L_ADR   71     /* II系通信1底板地址 */
#define  Safe_TX1_II_C_ADR   6      /* II系通信1子板地址 */
#define  Safe_TX2_II_L_ADR   57     /* II系通信2底板地址 */
#define  Safe_TX2_II_C_ADR   72     /* II系通信2子板地址 */


#pragma pack(1)

/*安全层帧头解析结构*/
typedef struct {
    uint8 des_adr;                  /* 目的地址 */
    uint8 src_adr;                  /* 源地址 */
    uint8 sig_pos;                  /* 标识位 */
    uint8 res;                      /* 预留 */
    uint32 serial_num;              /* 序列号 */
    uint32 time_print;              /* 时间戳 */
    uint16 lenth;                   /* 安全层整体长度 */
}S_SAFE_LAYER_HEADER;

#pragma pack()


extern sint8 vcp_safe_layer_check(uint8 *pBuf , uint8 from_chl);
extern void vcp_safe_layer_pakage(uint8 *pSafe , uint8 *pApp , uint16 app_len, uint8 send_chl);

#endif

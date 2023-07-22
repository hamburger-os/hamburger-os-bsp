/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-21     zm       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_INC_SAFE_LAYER_H_
#define APPLICATIONS_STO_RECORD_BOARD_INC_SAFE_LAYER_H_

#include <rtthread.h>

#include "data_handle.h"

#define STO_V2

/**********************************************************
 **      内部安全层地址定义
 **********************************************************/
#ifdef  STO_V2      //二代STO
#define  Safe_ZK_I_ADR                      3                   /* I系主控地址 */
#define  Safe_TX1_I_L_ADR                   71              /* I系通信1底板地址 */
#define  Safe_TX1_I_C_ADR                   6                   /* I系通信1子板地址 */
#define  Safe_TX2_I_L_ADR                   57              /* I系通信2底板地址 */
#define  Safe_TX2_I_C_ADR                   72              /* I系通信2子板地址 */
#define  Safe_ZK_II_ADR                     3                   /* II系主控地址 */
#define  Safe_TX1_II_L_ADR              71            /* II系通信1底板地址 */
#define  Safe_TX1_II_C_ADR              6                   /* II系通信1子板地址 */
#define  Safe_TX2_II_L_ADR              57              /* II系通信2底板地址 */
#define  Safe_TX2_II_C_ADR              72              /* II系通信2子板地址 */

#else                           //三合一
#define  Safe_ZK_I_ADR                      9                   /* I系主控地址 */
#define  Safe_TX1_I_L_ADR                   4                   /* I系通信1底板地址 */
#define  Safe_TX1_I_C_ADR                   5                   /* I系通信1子板地址 */
#define  Safe_TX2_I_L_ADR                   6                   /* I系通信2底板地址 */
#define  Safe_TX2_I_C_ADR                   7                   /* I系通信2子板地址 */
#define  Safe_ZK_II_ADR                     10              /* II系主控地址 */
#define  Safe_TX1_II_L_ADR              11              /* II系通信1底板地址 */
#define  Safe_TX1_II_C_ADR              12              /* II系通信1子板地址 */
#define  Safe_TX2_II_L_ADR              13              /* II系通信2底板地址 */
#define  Safe_TX2_II_C_ADR              14              /* II系通信2子板地址 */

#endif

#define SAFE_LAYER_PLOADLEN  (1480) /*安全层负载数据区最大为1480*/

/*安全层帧头解析结构*/
typedef struct eth_rx_safe_layer
{
    uint8_t des_adr;/*目的地址*/
    uint8_t src_adr;/*源地址*/
    uint8_t sig_pos;/*标识位*/
    uint8_t res; /*预留*/
    uint32_t serial_num;/*序列号*/
    uint32_t time_print;/*时间戳*/
    uint16_t lenth;
/*安全层解析到此处即可，不用全部解析，*/
} r_safe_layer;

rt_err_t rx_safe_layer_check(S_DATA_HANDLE * data_handle, uint8_t *pBuf, uint8_t from_chl);

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_SAFE_LAYER_H_ */

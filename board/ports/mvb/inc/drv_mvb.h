#ifndef _DRV_MVB_H
#define _DRV_MVB_H

#include <stdio.h>
#include "string.h"
#include "tcn_def.h"
#include "mue_def.h"
#include "mue_pd_full.h"
/***************************用户设置地址****************************************/
/*设置地址   bit11-bit0(12个bit)*/
#define  MVB_DEV_ADR  (BITSET16)(0x0010 & 0x0FFF)


#define RX_PORT_ADR_NUB   2
/*接收端口  bit11-bit0(12个bit)*/
#define  RX_PORT_ADR1 (BITSET16)(0x0140 & 0x0FFF)
/*接收端口  bit11-bit0(12个bit)*/
#define  RX_PORT_ADR2 (BITSET16)(0x0142 & 0x0FFF)
/*接收端口  bit11-bit0(12个bit)*/
#define  RX_PORT_ADR3 (BITSET16)(0x0022 & 0x0FFF)
/*接收端口  bit11-bit0(12个bit)*/
#define  RX_PORT_ADR4 (BITSET16)(0x0024 & 0x0FFF)
/*接收端口  bit11-bit0(12个bit)*/
#define  RX_PORT_ADR5 (BITSET16)(0x0032 & 0x0FFF)
/*接收端口  bit11-bit0(12个bit)*/
#define  RX_PORT_ADR6 (BITSET16)(0x0033 & 0x0FFF)


/*测试得出同一个MVB总线上 多个从设备， 从设备中不能存在相同的发送端口号 整个总线上发送端口号唯一*/
#define TX_PORT_ADR_NUB   2
/*发送端口   bit11-bit0(12个bit)*/
#define  TX_PORT_ADR1 (BITSET16)(0x0141 & 0x0FFF)
/*发送端口   bit11-bit0(12个bit)*/
#define  TX_PORT_ADR2 (BITSET16)(0x0143 & 0x0FFF)
/*发送端口   bit11-bit0(12个bit)*/
#define  TX_PORT_ADR3 (BITSET16)(0x0025 & 0x0FFF)
/*发送端口   bit11-bit0(12个bit)*/
#define  TX_PORT_ADR4 (BITSET16)(0x0026 & 0x0FFF)
/*发送端口   bit11-bit0(12个bit)*/
#define  TX_PORT_ADR5 (BITSET16)(0x0027 & 0x0FFF)
/*发送端口   bit11-bit0(12个bit)*/
#define  TX_PORT_ADR6 (BITSET16)(0x0028 & 0x0FFF)

#endif 



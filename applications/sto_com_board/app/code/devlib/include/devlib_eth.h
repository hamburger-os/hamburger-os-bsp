/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : devlib_eth.h
 **@author: Created By jiaqx
 **@date  : 2023.11.07
 **@brief : Manage devlib eth
 ********************************************************************************************/
#ifndef DEVLIB_ETH_H
#define DEVLIB_ETH_H

#include "support_eth.h"
typedef enum
{
    E_ETH_ZK,
    E_ETH_EX1,
    E_ETH_EX2,
    E_ETH_MAX
}E_ETH_CLASS;

extern BOOL devLib_eth_sendData( E_ETH_CLASS eth_cls, uint8 *pdata, uint16 len );
extern BOOL devLib_eth_getData( E_ETH_CLASS eth_cls, S_ETH_FRAME *pframe );

#endif
/**************************************end file*********************************************/


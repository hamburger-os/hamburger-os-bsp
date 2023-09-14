/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : if_eth.h
**@author: Created By Chengt
**@date  : 2023.09.06
**@brief : Implement the function interfaces of if_eth
********************************************************************************************/
#ifndef _IF_ETH_H
#define _IF_ETH_H

#include "common.h"

/***************eth channel*****************/
typedef enum 
{
  E_ETH_CH_1 = 0U,
  E_ETH_CH_2,
  E_ETH_CH_3,
  E_ETH_CH_MAX
}E_ETH_CH;

/***************eth interface***************/
	
extern BOOL if_eth_init( void );
extern BOOL if_eth_send( E_ETH_CH ch, uint8 *pdata, uint16 len );
extern uint16 if_eth_get( E_ETH_CH ch, uint8 *pdata, uint16 len );

#endif
/*******************************************end file****************************************/

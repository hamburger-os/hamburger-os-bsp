/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_libCRC.h
 **@author: Created By Chengt
 **@date  : 2022.04.07
 **@brief : Implement the function interfaces of support_libCRC
 ********************************************************************************************/
#ifndef _SUPPORT_LIBCRC_H
#define _SUPPORT_LIBCRC_H

#include "common.h"

typedef enum
{
    CRC16_TYPE_00000 = 0,
    CRC16_TYPE_16125,
    CRC16_TYPE_16152,
    CRC16_TYPE_16125_Tabf0,
    CRC16_TYPE_XMODEM
} E_CRC16_TYPE;

extern uint8 support_crc8(const uint8* pData, uint32 nLength);
extern uint16 support_crc16(const uint8* pData, uint32 nLength);
extern uint32 support_crc32(const uint8* pData, uint32 nLength);
extern uint16 support_crc16Type(E_CRC16_TYPE crcType, uint8 *pData, uint32 nLength, uint16 crc_init);

#endif
/**************************************end file*********************************************/


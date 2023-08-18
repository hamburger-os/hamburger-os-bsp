/**************************************************************************************************
(^_^) File Name  : RecordErrorCode.h
(^_^) Brief      : It is the error code recording header file of project.
(^_^) Author     : Liang Zhen.
(^_^) Date       : 4-September-2018.
(^_^) Version    : V1.0.0.1
(^_^) Revision   : 6-September-2018, by Liang Zhen.
(^_^) Company    : 北京思维鑫科信息技术有限公司.
(^_^) Copyright  : All rights reserved.
(^_^) Platform   : ATSAME70Q21.
(^_^) Disclaimer : none.
(^_^) Note       : Before using the file, please read comments carefully.
**************************************************************************************************/


#ifndef RECORDERRORCODE_H
#define RECORDERRORCODE_H


/* include ------------------------------------------------------------------------------------- */
#include "Common.h"
#include <stdint.h>


/* public macro definition --------------------------------------------------------------------- */


/* public type definition ---------------------------------------------------------------------- */
/* Locker. */
typedef enum
{
  UNLOCK = 0U,
  
  LOCK
} LOCKER;

/* The method of time. */
typedef enum
{
  TIME_OLDEST = 0U,
  
  TIME_NEWEST
} TIME_METHOD;

/* The side of device. */
typedef enum
{
  /* Unknown. */
  DEV_SIDE_UNKNOWN = 0U,
  
  /* Side I. */
  DEV_SIDE_I,
  
  /* Side II. */
  DEV_SIDE_II
} DEVICE_SIDE;


//TODO(mingzhao)
//原来在 sto 一代eth_manage.h
/* 10-November-2018, by Liang Zhen. */
/* public macro definition --------------------------------------------------------------------- */
/* Unit sign. */
#define HMB_SIGN           ( 'M' )
#define HMB_SIGN_I_A       ( 0x11U )
#define HMB_SIGN_I_B       ( 0x12U )
#define HMB_SIGN_II_A      ( 0x21U )
#define HMB_SIGN_II_B      ( 0x22U )
#define HCB_SIGN           ( 'C' )
#define HCB_SIGN_I         ( 0x10U )
#define HCB_SIGN_II        ( 0x20U )
#define HRB_SIGN           ( 'R' )
#define HRB_SIGN_I         ( 0x10U )
#define HRB_SIGN_II        ( 0x20U )
#define HIB_SIGN           ( 'I' )
#define HIB_SIGN_I         ( 0x10U )
#define HIB_SIGN_II        ( 0x20U )
#define CEU_SIGN           ( 'E' )
#define CEU_SIGN_I         ( 0x10U )
#define CEU_SIGN_II        ( 0x20U )
#define ECU_SIGN           ( 'B' )
#define ECU_SIGN_I         ( 0x10U )
#define ECU_SIGN_II        ( 0x20U )
#define uCOMP_SIGN         ( 'U' )
#define uCOMP_DUMMY        ( 0x00U )
#define DMI_SIGN           ( 'D' )
#define DMI_SIGN_I         ( 0x10U )
#define DMI_SIGN_II        ( 0x20U )
#define WLUP_SIGN          ( 'W' )
#define WLUP_DUMMY         ( 0x00U )

/* Handshake sign. */
#define HS_2KDATA          ( "2K" )
#define HS_DP              ( "DP" )
#define HS_ZMB             ( "ZM" )
#define HS_DMI             ( "XS" )
#define HS_EC              ( "EC" )
#define HS_APP             ( "AP" )
#define HS_RB              ( "RB" )
#define HS_RF              ( "RF" )

/* Acknowledgement sign. */
#define ACK_CF             ( "CF" )
#define ACK_NS             ( "NS" )
#define ACK_RS             ( "RS" )
#define ACK_DENY           ( "AD" )
#define ACK_RT             ( "RT" )
/****************************/

/* public function declaration ----------------------------------------------------------------- */
void ThreadErrorCode( void );

void ErrorCodeInit( const char *dprt );

uint32_t UpdateErrorCodeDate( uint8_t date[] );
uint32_t UpdateErrorCodeTime( uint8_t time[] );

uint32_t UpdateErrorFlag_HMB( CPU_TYPE cpu, uint8_t err[] );
void UpdateErrorFlag_HRB( uint16_t err );
uint32_t UpdateErrorFlag_HCB( uint8_t err[] );
uint32_t UpdateErrorFlag_HIB( uint8_t err[] );
uint32_t UpdateErrorFlag_DMI( DEVICE_SIDE side, uint8_t err[] );
uint32_t UpdateErrorFlag_CEU( DEVICE_SIDE side, uint8_t err[] );
uint32_t UpdateErrorFlag_EBV( DEVICE_SIDE side, uint8_t err[] );

uint32_t GetNewDatagram( uint8_t dgm[], uint32_t len );


#endif /* end RECORDERRORCODE_H */



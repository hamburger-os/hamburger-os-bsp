/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : common.h
**@author: Created By Chengt
**@date  : 2019.12.13
**@brief : null 
********************************************************************************************/
#ifndef _COMMON_H
#define _COMMON_H

typedef signed char           sint8;          /** 8位有符号 */
typedef unsigned char         uint8;          /** 8位无符号 */
typedef signed short          sint16;         /** 16位有符号 */
typedef unsigned short        uint16;         /** 16位无符号 */
typedef signed   int          sint32;         /** 32位有符号 */
typedef unsigned int          uint32;         /** 32位无符号*/
typedef unsigned long         ulong;          /** 无符号long */
typedef long                  long32;         /** 32位long */
typedef unsigned long long    uint64;         /** 64位无符号 */
typedef signed long long      sint64;         /** 64位有符号 */


typedef float                 real32;         /** 32位浮点 */
typedef double                real64;         /** 64位浮点 */

typedef unsigned int          size_t;  /** 32位无符号 */

typedef unsigned char   BOOL;

#undef true
#define  true   ((sint32)1)          /** 布尔TRUE */

#undef false
#define  false  ((sint32)!true)      /** 布尔FALSE */

#undef FALSE
#define FALSE       ( 0 )

#undef TRUE
#define TRUE        ( 1 )

#undef  ERROR
#define ERROR       ((sint32)-1)         /** 错误 */

#undef  OK
#define OK          ((sint32)0)          /** 正确 */

#ifndef NULL
#define NULL  ((void *)0)          /* 空指针定义 */
#endif                


#undef U8_CONV_U16
#define U8_CONV_U16(p_u8) (*((uint16*)(p_u8)))

#undef U8_CONV_U32
#define U8_CONV_U32(p_u8) (*((uint32*)(p_u8)))


#undef ABS
#define ABS(a,b)  (((a) > (b)) ? (a-b) : (b-a))


#undef MIN
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

#undef MAX
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))

#undef STO_HTONS
#define STO_HTONS(n)  (((n & 0xff) << 8) | ((n & 0xff00) >> 8))

#undef STO_HTONL
#define STO_HTONL(n)  (((n & 0xff) << 24) |((n & 0xff00) << 8) |((n & 0xff0000UL) >> 8) |((n & 0xff000000UL) >> 24))

 /******** 四字节转化为无符合的32位整形 ******/
#undef FOUR_TO_UINT32
#define FOUR_TO_UINT32(X,Y,Z,K)    ((uint32)(X) * 0x1000000L + (uint32)(Y) * 0x10000L + (uint32)(Z) * 0x100 + (uint32)(K))

/**********两字节转化为无符合的16位整形*****/
#undef  TWO_TO_UINT16
#define TWO_TO_UINT16(X,Y)          ((uint16)(X) * 0x100 + (uint16)(Y))

/****************打印函数******************/
extern sint32 MY_Printf( const char *fmt,... );
extern sint32 MY_PrintfLog( const char *fmt,... );

extern sint8  int8_abs( sint8 a );
extern sint16 int16_abs( sint16 a );
extern sint32 int32_abs( sint32 a );
extern sint64 int64_abs( sint64 a );
extern real32 f32_abs( real32 a );
extern real64 d64_abs( real64 a );
extern real32 f32_sqrt( real32 a );
#endif

/**************************************end file*********************************************/

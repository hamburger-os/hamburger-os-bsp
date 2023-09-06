/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-05     zm       the first version
 */
#ifndef APPLICATIONS_STO_COM_BOARD_HAL_OS_COMMON_H_
#define APPLICATIONS_STO_COM_BOARD_HAL_OS_COMMON_H_

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

#endif /* APPLICATIONS_STO_COM_BOARD_HAL_OS_COMMON_H_ */

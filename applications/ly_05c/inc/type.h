/*******************************************************
 *
 * @FileName: type.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 类型定义的头文件,定义的常见的数据类型.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef _TYPE_H_
#define _TYPE_H_

/*******************************************************
 * 头文件
 *******************************************************/

#include "config.h"
#include "log.h"

/*******************************************************
 * 重定义
 *******************************************************/

typedef signed char sint8_t;
typedef unsigned char uint8_t;

typedef signed short sint16_t;
typedef unsigned short uint16_t;

typedef signed int sint32_t;
/*在RT_Thread系统中, 其他模块定义了uint32_t,故在此处不定义. */
/*typedef unsigned int uint32_t; */

typedef float real32_t;
typedef float real64_t;

#endif

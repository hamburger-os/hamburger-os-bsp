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

#ifndef TYPE_H_
#define TYPE_H_

/*******************************************************
 * 头文件
 *******************************************************/

/*******************************************************
 * 重定义
 *******************************************************/

typedef signed char sint8_t; /* 重定义类型  */
typedef unsigned char uint8_t; /* 重定义类型  */

typedef signed short sint16_t; /* 重定义类型  */
typedef unsigned short uint16_t; /* 重定义类型  */

typedef signed int sint32_t; /* 重定义类型  */
/* 在RT_Thread系统中, 其他模块定义了uint32_t,故在此处不定义. */
/* typedef unsigned int uint32_t; */

typedef float real32_t; /* 重定义类型  */
typedef double real64_t; /* 重定义类型  */

#endif

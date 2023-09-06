/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-05     zm       the first version
 */
#ifndef APPLICATIONS_STO_COM_BOARD_HAL_OS_HAL_OS_H_
#define APPLICATIONS_STO_COM_BOARD_HAL_OS_HAL_OS_H_

#include "common.h"

#define HAL_OS_WAITING_FOREVER  (-1)

#ifdef HAL_OS_USE_RT_THREAD_LOG
#include <rtdef.h>
/* rtthread 线程安全日志输出 */
#define HAL_OS_LOG_E(...)                      ulog_e(LOG_TAG, __VA_ARGS__)
#define HAL_OS_LOG_W(...)                      ulog_w(LOG_TAG, __VA_ARGS__)
#define HAL_OS_LOG_I(...)                      ulog_i(LOG_TAG, __VA_ARGS__)
#define HAL_OS_LOG_D(...)                      ulog_d(LOG_TAG, __VA_ARGS__)
#define HAL_OS_LOG_RAW(...)                    ulog_raw(__VA_ARGS__)
#else
#define HAL_OS_LOG_E(...)
#define HAL_OS_LOG_W(...)
#define HAL_OS_LOG_I(...)
#define HAL_OS_LOG_D(...)
#define HAL_OS_LOG_RAW(...)
#endif

typedef struct {
    void *sem;
} S_HLA_OS_SEM;

typedef struct {
    void *mq;
} S_HAL_OS_MQ;

/**@brief 创建线程
* @param[in]  *task_name        线程名
* @param[in]  *entry            线程入口函数
* @param[in]  *param            线程入口函数参数
* @param[in]  stack_size        线程栈大小
* @param[in]  priority          线程优先级
* @return  函数执行结果
* - > 0      创建成功
* - < 0      创建失败
*/
sint8 hal_os_creat_task(const char *task_name,
                        void (*entry)(void *param),
                        void *param,
                        uint32 stack_size,
                        uint8 priority);

/**@brief 创建信号量
* @param[in]  *sem        信号量句柄
* @param[in]  *name       信号量名称
* @param[in]  value       信号量初始值
* @return  函数执行结果
* - > 0      创建成功
* - < 0      创建失败
* @note 等待线程队列将按照优先级进行排队，优先级高的等待线程将先获得等待的信号量
*/
sint8 hal_os_sem_creat(S_HLA_OS_SEM *sem, const char *name, uint32 value);

/**@brief 删除信号量
* @param[in]  *sem        信号量句柄
* @return  函数执行结果
* - > 0      删除成功
* - < 0      删除失败
*/
sint8 hal_os_sem_delete(S_HLA_OS_SEM *sem);

/**@brief 获取信号量
* @param[in]  *sem        信号量句柄
* @param[in]  timeout     超时时间  单位 ms   阻塞等待时入参为：HAL_OS_WAITING_FOREVER
* @return  函数执行结果
* - > 0      获取成功
* - < 0      获取失败
*/
sint8 hal_os_sem_take(S_HLA_OS_SEM *sem, sint32 timeout);

/**@brief 释放信号量
* @param[in]  *sem        信号量句柄
* @return  函数执行结果
* - > 0      释放成功
* - < 0      释放失败
*/
sint8 hal_os_sem_release(S_HLA_OS_SEM *sem);

/**@brief 创建消息队列
* @param[in]  *mq         消息队列句柄
* @param[in]  *name       消息队列名称
* @param[in]  mq_size     消息队列中一条消息的最大长度，单位字节
* @param[in]  max_mq_num  消息队列的最大个数
* @return  函数执行结果
* - > 0      释放成功
* - < 0      释放失败
* @note 等待线程队列将按照优先级进行排队，优先级高的等待线程将先获得等待的消息队列
*/
sint8 hal_os_mq_creat(S_HAL_OS_MQ *mq, const char *name, uint32 mq_size, uint32 max_mq_num);

/**@brief 删除消息队列
* @param[in]  *mq        消息队列句柄
* @return  函数执行结果
* - > 0      删除成功
* - < 0      删除失败
*/
sint8 hal_os_mq_delete(S_HAL_OS_MQ *mq);

/**@brief 发送消息队列
* @param[in]  *sem        消息队列句柄
* @param[in]  *buffer     消息内容
* @param[in]  size        消息大小
* @param[in]  timeout     超时时间  单位 ms   0：不等待；>0：发送线程将进行超时等待
* @return  函数执行结果
* - > 0      发送成功
* - < 0      发送失败
*/
sint8 hal_os_mq_send(S_HAL_OS_MQ *mq, const void *buffer, uint32 size, uint32 timeout);

/**@brief 接收消息队列
* @param[in]  *sem        消息队列句柄
* @param[in]  *buffer     消息内容
* @param[in]  size        消息大小
* @param[in]  timeout     超时时间  单位 ms   阻塞等待时入参为：HAL_OS_WAITING_FOREVER
* @return  函数执行结果
* - > 0      发送成功
* - < 0      发送失败
*/
sint8 hal_os_mq_recv(S_HAL_OS_MQ *mq, void *buffer, uint32 size, uint32 timeout);

/**@brief 延时
* @param[in]  ms      延时时间 单位ms
* @return  无返回值
*/
void hal_os_delay_ms(uint32 ms);

/**@brief 串口打印输出
* @param[in]  fmt      打印信息
* @return  无返回值
* @note 非线程安全
*/
void hal_os_printf(const char *fmt, ...);

#endif /* APPLICATIONS_STO_COM_BOARD_HAL_OS_HAL_OS_H_ */

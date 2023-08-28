
/*******************************************************
 *
 * @FileName: ly_05c.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: USB接口测试程序.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <stdbool.h>
#include <pthread.h>
#include <delay.h>

#ifndef F_OK
#define F_OK 0
#endif

#define UDISK_PATH "/mnt/udisk/ud0p0"

static bool udisk_test_runinig_state = false;

/*******************************************************
 *
 * @brief  判断文件是否存在
 *
 * @param  file: 文件名
 * @retval none
 *
 *******************************************************/

bool f_exist(const char *file)
{
    return (access(file, F_OK) == 0);
}
/*******************************************************
 *
 * @brief  USB接口测试程序
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
int ly_05c1_udisk_test(void)
{
    int ret = 0;

    if (f_exist(UDISK_PATH))
        rt_kprintf("USB接口正常!\n");
    else
        rt_kprintf("USB接口故障!\n");
    return RT_EOK;
}

MSH_CMD_EXPORT_ALIAS(ly_05c1_udisk_test, UdiskDaemon, ly - 05c udisk test.);

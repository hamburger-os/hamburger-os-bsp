/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-21     lvhan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include <stdio.h>
#include <stdlib.h>

#if !defined(SAL_USING_POSIX)
#error "Please enable SAL_USING_POSIX!"
#else
#include <sys/time.h>
#include <sys/select.h>
#endif
#include <sys/socket.h>
#include <sys/errno.h>
#include <netdb.h>

#include "selftest.h"

#define DBG_TAG "tcp "
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

void selftest_tcpip_test(SelftestlUserData *puserdata)
{

}

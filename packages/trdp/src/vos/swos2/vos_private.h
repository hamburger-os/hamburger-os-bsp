/**********************************************************************************************************************/
/**
 * @file            esp/vos_private.h
 *
 * @brief           Private definitions for the OS abstraction layer
 *
 * @details
 *
 * @note            Project: TCNOpen TRDP prototype stack
 *
 * @author          Bernd Loehr
 *
 * @remarks This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 *          If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *          Copyright 2018. All rights reserved.
 */
 /*
 * $Id: vos_private.h 2288 2021-08-11 14:52:06Z ahweiss $
 *
 *      BL 2018-06-20: Ticket #184: Building with VS 2015: WIN64 and Windows threads (SOCKET instead of INT32)
 *
 */

#ifndef VOS_PRIVATE_H
#define VOS_PRIVATE_H

/***********************************************************************************************************************
 * INCLUDES
 */

#include <string.h>
#include <pthread.h>
#include "sys/socket.h"
#include "netdb.h"
#include "vos_types.h"
#include "vos_thread.h"

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 * DEFINES
 */

/* The VOS version can be predefined as CFLAG   */
#ifndef VOS_VERSION
#define VOS_VERSION            2u
#define VOS_RELEASE            1u  /* 322: interface change in vos_sockReceiveUDP() */
#define VOS_UPDATE             0u
#define VOS_EVOLUTION          0u
#endif

struct VOS_MUTEX
{
    UINT32          magicNo;
    pthread_mutex_t mutexId;
};

struct VOS_SEMA
{
    rt_sem_t    semHandle;
};

#ifdef __PLATFORM_SWOS2__
#define STRING_ERR(pStrBuf)  (void)strerror_r(errno, pStrBuf, VOS_MAX_ERR_STR_SIZE);
#else
#define STRING_ERR(pStrBuf)  (void)strerror_r(errno, pStrBuf, VOS_MAX_ERR_STR_SIZE);
#endif

#ifdef __cplusplus
}
#endif

#endif /* VOS_UTILS_H */

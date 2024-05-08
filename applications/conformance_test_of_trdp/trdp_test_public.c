/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-09     zm       the first version
 */
#include "trdp_test_public.h"
#include "vos_utils.h"

#include <rtthread.h>
#include <string.h>

/* --- convert trdp error code to string -------------------------------------*/

const char * trdp_test_public_get_result_string(int ret)
{
    static char buf[128];

    switch (ret)
    {
    case TRDP_NO_ERR:
        return "TRDP_NO_ERR (no error)";
    case TRDP_PARAM_ERR:
        return "TRDP_PARAM_ERR (parameter missing or out of range)";
    case TRDP_INIT_ERR:
        return "TRDP_INIT_ERR (call without valid initialization)";
    case TRDP_NOINIT_ERR:
        return "TRDP_NOINIT_ERR (call with invalid handle)";
    case TRDP_TIMEOUT_ERR:
        return "TRDP_TIMEOUT_ERR (timeout)";
    case TRDP_NODATA_ERR:
        return "TRDP_NODATA_ERR (non blocking mode: no data received)";
    case TRDP_SOCK_ERR:
        return "TRDP_SOCK_ERR (socket error / option not supported)";
    case TRDP_IO_ERR:
        return "TRDP_IO_ERR (socket IO error, data can't be received/sent)";
    case TRDP_MEM_ERR:
        return "TRDP_MEM_ERR (no more memory available)";
    case TRDP_SEMA_ERR:
        return "TRDP_SEMA_ERR semaphore not available)";
    case TRDP_QUEUE_ERR:
        return "TRDP_QUEUE_ERR (queue empty)";
    case TRDP_QUEUE_FULL_ERR:
        return "TRDP_QUEUE_FULL_ERR (queue full)";
    case TRDP_MUTEX_ERR:
        return "TRDP_MUTEX_ERR (mutex not available)";
    case TRDP_NOSESSION_ERR:
        return "TRDP_NOSESSION_ERR (no such session)";
    case TRDP_SESSION_ABORT_ERR:
        return "TRDP_SESSION_ABORT_ERR (Session aborted)";
    case TRDP_NOSUB_ERR:
        return "TRDP_NOSUB_ERR (no subscriber)";
    case TRDP_NOPUB_ERR:
        return "TRDP_NOPUB_ERR (no publisher)";
    case TRDP_NOLIST_ERR:
        return "TRDP_NOLIST_ERR (no listener)";
    case TRDP_CRC_ERR:
        return "TRDP_CRC_ERR (wrong CRC)";
    case TRDP_WIRE_ERR:
        return "TRDP_WIRE_ERR (wire error)";
    case TRDP_TOPO_ERR:
        return "TRDP_TOPO_ERR (invalid topo count)";
    case TRDP_COMID_ERR:
        return "TRDP_COMID_ERR (unknown comid)";
    case TRDP_STATE_ERR:
        return "TRDP_STATE_ERR (call in wrong state)";
    case TRDP_APP_TIMEOUT_ERR:
        return "TRDP_APPTIMEOUT_ERR (application timeout)";
    case TRDP_UNKNOWN_ERR:
        return "TRDP_UNKNOWN_ERR (unspecified error)";
    }
    sprintf(buf, "unknown error (%d)", ret);
    return buf;
}

void trdp_test_public_printLog(
    void        *pRefCon,
    VOS_LOG_T   category,
    const CHAR8 *pTime,
    const CHAR8 *pFile,
    UINT16      line,
    const CHAR8 *pMsgStr)
{
    (void) pRefCon;

    const char *catStr[] = {"Error:", "Warning:", "Info:", "Debug:", "User:"};
    CHAR8       *pF = strrchr(pFile, VOS_DIR_SEP);

    rt_kprintf("%s %s:%d %s",
           catStr[category],
           (pF == NULL)? "" : pF + 1,
           line,
           pMsgStr);
}

rt_bool_t trdp_test_timeout_ms(uint32_t *current_ms, uint32_t timeout_ms)
{
    uint32_t curtv = 0U;
    rt_bool_t status = RT_FALSE;

    /* get the tick */
    curtv = rt_tick_get();
    /* init the timer */

    /* check the conditon about the timeout */
    if ((curtv - *current_ms) >= timeout_ms)
    {
        *current_ms = curtv;
        status = RT_TRUE;
    }
    else
    {
        status = RT_FALSE;
    }
    return status;
}


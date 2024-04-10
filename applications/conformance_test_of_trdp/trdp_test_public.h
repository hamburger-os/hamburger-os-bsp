/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-09     zm       the first version
 */
#ifndef APPLICATIONS_CONFORMANCE_TEST_OF_TRDP_TRDP_TEST_PUBLIC_H_
#define APPLICATIONS_CONFORMANCE_TEST_OF_TRDP_TRDP_TEST_PUBLIC_H_

#include "trdp_types.h"

const char * trdp_test_public_get_result_string(int ret);
void trdp_test_public_printLog(
    void        *pRefCon,
    VOS_LOG_T   category,
    const CHAR8 *pTime,
    const CHAR8 *pFile,
    UINT16      line,
    const CHAR8 *pMsgStr);
rt_bool_t trdp_test_timeout_ms(uint32_t *current_ms, uint32_t timeout_ms);

#endif /* APPLICATIONS_CONFORMANCE_TEST_OF_TRDP_TRDP_TEST_PUBLIC_H_ */

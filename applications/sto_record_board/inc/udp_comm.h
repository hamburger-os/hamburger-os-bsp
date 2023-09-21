/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-18     zm       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_INC_UDP_COMM_H_
#define APPLICATIONS_STO_RECORD_BOARD_INC_UDP_COMM_H_

#include <rtthread.h>

typedef enum
{
  UDP_RECV_EMPTY = 0U,
  UDP_RECV_NOTEMPTY
} UDP_RECV_STATUS;

int UDPServerRcvThreadInit(void);
rt_err_t UDPServerRcvMQData(void);

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_UDP_COMM_H_ */

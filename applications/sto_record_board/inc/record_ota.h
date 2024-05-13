/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-26     zm       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_INC_RECORD_OTA_H_
#define APPLICATIONS_STO_RECORD_BOARD_INC_RECORD_OTA_H_

typedef enum {
    RecordOTAModeNormal = 1,  /* 正常模式 */
    RecordOTAModeUpdata = 2,  /* 升级模式 */
    RecordOTAModeUpdataAgain = 3  /* 再次连接  之前的ota未断开连接，显示器再次发送连接命令 */
} RecordOTAMode;

void RecordOTAInit(void);

RecordOTAMode RecordOTAGetMode(void);

int RecordOTAInitThreadInit(void);
void RecordOTASetMode(RecordOTAMode mode);

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_RECORD_OTA_H_ */

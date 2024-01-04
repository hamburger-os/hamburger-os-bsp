/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-06     lvhan       the first version
 */
#ifndef PACKAGES_OTA_FROM_FILE_OTA_FROM_FILE_H_
#define PACKAGES_OTA_FROM_FILE_OTA_FROM_FILE_H_

typedef enum
{
    OTA_HANDLE_START = 0,
    OTA_HANDLE_FINISH,
    OTA_HANDLE_LOADED,
    OTA_HANDLE_FAILED,
}OtaHandleTypeDef;

void ota_from_file_handle(OtaHandleTypeDef type);

#endif /* PACKAGES_OTA_FROM_FILE_OTA_FROM_FILE_H_ */

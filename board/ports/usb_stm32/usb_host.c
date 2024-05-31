/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file            : usb_host.c
  * @version         : v1.0_Cube
  * @brief           : This file implements the USB Host
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_audio.h"
#include "usbh_cdc.h"
#include "usbh_msc.h"
#include "usbh_hid.h"
//#include "usbh_mtp.h"

#include "usbh_diskio.h"

#include "board.h"

#define DBG_TAG "usbh.host"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* USB Host core handle declaration */
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX USBH_HandleTypeDef hUsbHost;

/*
 * user callback declaration
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/*
 * -- Insert your external function declaration here --
 */

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
void MX_USB_HOST_Init(void)
{
    /* Init host Library, add supported class and start the library. */
#ifdef BSP_USING_USBH_STM32_FS
    if (USBH_Init(&hUsbHost, USBH_UserProcess, HOST_FS) != USBH_OK)
#endif
#ifdef BSP_USING_USBH_STM32_HS
    if (USBH_Init(&hUsbHost, USBH_UserProcess, HOST_HS) != USBH_OK)
#endif
    {
        Error_Handler();
    }
    if (USBH_RegisterClass(&hUsbHost, USBH_AUDIO_CLASS) != USBH_OK)
    {
        Error_Handler();
    }
    if (USBH_RegisterClass(&hUsbHost, USBH_CDC_CLASS) != USBH_OK)
    {
        Error_Handler();
    }
    if (USBH_RegisterClass(&hUsbHost, USBH_MSC_CLASS) != USBH_OK)
    {
        Error_Handler();
    }
    if (USBH_RegisterClass(&hUsbHost, USBH_HID_CLASS) != USBH_OK)
    {
        Error_Handler();
    }
//    if (USBH_RegisterClass(&hUsbHost, USBH_MTP_CLASS) != USBH_OK)
//    {
//        Error_Handler();
//    }
    if (USBH_Start(&hUsbHost) != USBH_OK)
    {
        Error_Handler();
    }

    USBH_UsrLog("Init successfully");
}

/*
 * user callback definition
 */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{
    switch (id)
    {
    case HOST_USER_SELECT_CONFIGURATION:
    {
        USBH_DbgLog("SELECT_CONFIGURATION");
    }
        break;

    case HOST_USER_CLASS_ACTIVE:
    {
        USBH_ClassTypeDef *pActiveClass = phost->pActiveClass;
        USBH_DbgLog("CLASS_ACTIVE '%s'", pActiveClass->Name);
        if (rt_strcmp(pActiveClass->Name, "MSC") == 0)
        {
            MSC_HandleTypeDef *MSC_Handle = (MSC_HandleTypeDef *) pActiveClass->pData;
            if (MSC_Handle->unit[MSC_Handle->current_lun].error == MSC_OK)
            {
                USBH_DbgLog("MSC_OK %d %d", MSC_Handle->state, MSC_Handle->current_lun);
                USBH_diskio_initialize(MSC_Handle);
            }
            else if (MSC_Handle->unit[MSC_Handle->current_lun].error == MSC_NOT_READY)
            {
                USBH_ErrLog("MSC_NOT_READY %d %d", MSC_Handle->state, MSC_Handle->current_lun);
            }
            else if (MSC_Handle->unit[MSC_Handle->current_lun].error == MSC_ERROR)
            {
                USBH_ErrLog("MSC_ERROR %d %d", MSC_Handle->state, MSC_Handle->current_lun);
            }
        }
    }
        break;

    case HOST_USER_CLASS_SELECTED:
    {
        USBH_DbgLog("CLASS_SELECTED");
    }
        break;

    case HOST_USER_CONNECTION:
    {
        USBH_DbgLog("CONNECTION");
    }
        break;

    case HOST_USER_DISCONNECTION:
    {
        USBH_DbgLog("DISCONNECTION");
        USBH_diskio_uninitialize();
    }
        break;

    case HOST_USER_UNRECOVERED_ERROR:
    {
        USBH_ErrLog("UNRECOVERED_ERROR");
    }
        break;

    default:
    {
        USBH_DbgLog("USBH_UserProcess 0x%x", id);
    }
        break;
    }
}

/**
  * @}
  */

/**
  * @}
  */


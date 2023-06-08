/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usbh_diskio.h (based on usbh_diskio_dma_template.h v2.0.2)
  * @brief   Header for usbh_diskio.c module
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBH_DISKIO_H
#define __USBH_DISKIO_H

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_conf.h"
#include "rtdef.h"

/* Private typedef -----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
rt_err_t USBH_diskio_initialize(MSC_HandleTypeDef *handle);
rt_err_t USBH_diskio_uninitialize(void);

#endif /* __USBH_DISKIO_H */


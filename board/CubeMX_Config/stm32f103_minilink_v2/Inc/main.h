/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TGT_SWCLK_Pin GPIO_PIN_0
#define TGT_SWCLK_GPIO_Port GPIOB
#define TGT_SWOUT_Pin GPIO_PIN_1
#define TGT_SWOUT_GPIO_Port GPIOB
#define TGT_SWDIO_IN_Pin GPIO_PIN_2
#define TGT_SWDIO_IN_GPIO_Port GPIOB
#define TGT_nRESET_Pin GPIO_PIN_12
#define TGT_nRESET_GPIO_Port GPIOB
#define LED_COM_Pin GPIO_PIN_5
#define LED_COM_GPIO_Port GPIOB
#define LED_DAP_Pin GPIO_PIN_6
#define LED_DAP_GPIO_Port GPIOB
#define LED_PWR_Pin GPIO_PIN_7
#define LED_PWR_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

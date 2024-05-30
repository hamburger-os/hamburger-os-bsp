/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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
#define WDI_LED_Pin GPIO_PIN_2
#define WDI_LED_GPIO_Port GPIOE
#define CPU_SWRST_Pin GPIO_PIN_3
#define CPU_SWRST_GPIO_Port GPIOA
#define BOAD0_Pin GPIO_PIN_7
#define BOAD0_GPIO_Port GPIOE
#define BOAD1_Pin GPIO_PIN_8
#define BOAD1_GPIO_Port GPIOE
#define BOAD2_Pin GPIO_PIN_9
#define BOAD2_GPIO_Port GPIOE
#define BOAD3_Pin GPIO_PIN_10
#define BOAD3_GPIO_Port GPIOE
#define ID0_Pin GPIO_PIN_12
#define ID0_GPIO_Port GPIOE
#define ID1_Pin GPIO_PIN_13
#define ID1_GPIO_Port GPIOE
#define ID2_Pin GPIO_PIN_14
#define ID2_GPIO_Port GPIOE
#define ID3_Pin GPIO_PIN_15
#define ID3_GPIO_Port GPIOE
#define CANA_INT_Pin GPIO_PIN_14
#define CANA_INT_GPIO_Port GPIOB
#define LED_RUN_Pin GPIO_PIN_8
#define LED_RUN_GPIO_Port GPIOD
#define LED_BY1_Pin GPIO_PIN_9
#define LED_BY1_GPIO_Port GPIOD
#define LED_BY2_Pin GPIO_PIN_10
#define LED_BY2_GPIO_Port GPIOD
#define LED_BY3_Pin GPIO_PIN_11
#define LED_BY3_GPIO_Port GPIOD
#define LED_BY4_Pin GPIO_PIN_12
#define LED_BY4_GPIO_Port GPIOD
#define CANB_INT_Pin GPIO_PIN_6
#define CANB_INT_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

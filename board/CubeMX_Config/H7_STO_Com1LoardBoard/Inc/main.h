/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

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
#define B_SW2_IRQ_Pin GPIO_PIN_8
#define B_SW2_IRQ_GPIO_Port GPIOI
#define B_SW3_RST_Pin GPIO_PIN_9
#define B_SW3_RST_GPIO_Port GPIOI
#define B_ETH3_RST_Pin GPIO_PIN_11
#define B_ETH3_RST_GPIO_Port GPIOI
#define B_SPI5_CS1_Pin GPIO_PIN_6
#define B_SPI5_CS1_GPIO_Port GPIOF
#define B_SPI6_CS_Pin GPIO_PIN_9
#define B_SPI6_CS_GPIO_Port GPIOH
#define B_SPI2_CS_Pin GPIO_PIN_10
#define B_SPI2_CS_GPIO_Port GPIOH
#define B_SPI2_INT_Pin GPIO_PIN_11
#define B_SPI2_INT_GPIO_Port GPIOH
#define B_SPI6_INT_Pin GPIO_PIN_12
#define B_SPI6_INT_GPIO_Port GPIOH
#define B_SPI2_MOSI_Pin GPIO_PIN_14
#define B_SPI2_MOSI_GPIO_Port GPIOB
#define B_SPI2_MOSIB15_Pin GPIO_PIN_15
#define B_SPI2_MOSIB15_GPIO_Port GPIOB
#define B_ETH1_RST_Pin GPIO_PIN_7
#define B_ETH1_RST_GPIO_Port GPIOG
#define B_BAD0IN_Pin GPIO_PIN_2
#define B_BAD0IN_GPIO_Port GPIOI
#define B_BAD1IN_Pin GPIO_PIN_3
#define B_BAD1IN_GPIO_Port GPIOI
#define B_ETH2_RST_Pin GPIO_PIN_6
#define B_ETH2_RST_GPIO_Port GPIOD
#define B_ETH2_IRQ_Pin GPIO_PIN_7
#define B_ETH2_IRQ_GPIO_Port GPIOD
#define B_ETH1_IRQ_Pin GPIO_PIN_10
#define B_ETH1_IRQ_GPIO_Port GPIOG
#define B_SW1_IRQ_Pin GPIO_PIN_4
#define B_SW1_IRQ_GPIO_Port GPIOI
#define B_LED_WORK_Pin GPIO_PIN_5
#define B_LED_WORK_GPIO_Port GPIOI
#define B_LED_ERR_Pin GPIO_PIN_6
#define B_LED_ERR_GPIO_Port GPIOI
#define B_SW1_RST_Pin GPIO_PIN_7
#define B_SW1_RST_GPIO_Port GPIOI

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

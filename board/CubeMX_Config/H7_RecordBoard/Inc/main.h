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
#define USB_OC_Pin GPIO_PIN_8
#define USB_OC_GPIO_Port GPIOI
#define USB_PEN_Pin GPIO_PIN_13
#define USB_PEN_GPIO_Port GPIOC
#define SPI5_CS1_Pin GPIO_PIN_6
#define SPI5_CS1_GPIO_Port GPIOF
#define NET3_RST_Pin GPIO_PIN_0
#define NET3_RST_GPIO_Port GPIOA
#define NET3_IRQ_Pin GPIO_PIN_1
#define NET3_IRQ_GPIO_Port GPIOA
#define USB_RST_Pin GPIO_PIN_2
#define USB_RST_GPIO_Port GPIOA
#define SPI6_INT2_Pin GPIO_PIN_9
#define SPI6_INT2_GPIO_Port GPIOH
#define SPI6_INT2_EXTI_IRQn EXTI9_5_IRQn
#define SPI6_CS2_Pin GPIO_PIN_10
#define SPI6_CS2_GPIO_Port GPIOH
#define SPI6_CS1_Pin GPIO_PIN_11
#define SPI6_CS1_GPIO_Port GPIOH
#define SPI6_INT1_Pin GPIO_PIN_12
#define SPI6_INT1_GPIO_Port GPIOH
#define SPI6_INT1_EXTI_IRQn EXTI15_10_IRQn
#define NET1_RST_Pin GPIO_PIN_7
#define NET1_RST_GPIO_Port GPIOG
#define SPI2_INT2_Pin GPIO_PIN_13
#define SPI2_INT2_GPIO_Port GPIOH
#define SPI2_INT2_EXTI_IRQn EXTI15_10_IRQn
#define SPI2_CS2_Pin GPIO_PIN_14
#define SPI2_CS2_GPIO_Port GPIOH
#define SPI2_CS1_Pin GPIO_PIN_15
#define SPI2_CS1_GPIO_Port GPIOH
#define SPI2_INT1_Pin GPIO_PIN_0
#define SPI2_INT1_GPIO_Port GPIOI
#define SPI2_INT1_EXTI_IRQn EXTI0_IRQn
#define NET2_RST_Pin GPIO_PIN_6
#define NET2_RST_GPIO_Port GPIOD
#define NET2_IRQ_Pin GPIO_PIN_7
#define NET2_IRQ_GPIO_Port GPIOD
#define NET1_IRQ_Pin GPIO_PIN_10
#define NET1_IRQ_GPIO_Port GPIOG
#define SW1_IRQ_Pin GPIO_PIN_4
#define SW1_IRQ_GPIO_Port GPIOI
#define SW1_RST_Pin GPIO_PIN_7
#define SW1_RST_GPIO_Port GPIOI

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

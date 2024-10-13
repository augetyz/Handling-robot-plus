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
#define fatfs_sd_Pin GPIO_PIN_5
#define fatfs_sd_GPIO_Port GPIOF
#define B0_Pin GPIO_PIN_6
#define B0_GPIO_Port GPIOF
#define B1_Pin GPIO_PIN_7
#define B1_GPIO_Port GPIOF
#define B2_Pin GPIO_PIN_8
#define B2_GPIO_Port GPIOF
#define LCD_BL_Pin GPIO_PIN_10
#define LCD_BL_GPIO_Port GPIOF
#define T_CLK_Pin GPIO_PIN_5
#define T_CLK_GPIO_Port GPIOA
#define T_CS_Pin GPIO_PIN_0
#define T_CS_GPIO_Port GPIOB
#define T_MOSI_Pin GPIO_PIN_1
#define T_MOSI_GPIO_Port GPIOB
#define T_MISO_Pin GPIO_PIN_2
#define T_MISO_GPIO_Port GPIOB
#define T_PEN_Pin GPIO_PIN_11
#define T_PEN_GPIO_Port GPIOF
#define CH2_A_Pin GPIO_PIN_3
#define CH2_A_GPIO_Port GPIOG
#define CH2_B_Pin GPIO_PIN_4
#define CH2_B_GPIO_Port GPIOG
#define CH3_A_Pin GPIO_PIN_5
#define CH3_A_GPIO_Port GPIOG
#define CH3_B_Pin GPIO_PIN_6
#define CH3_B_GPIO_Port GPIOG
#define CH4_A_Pin GPIO_PIN_7
#define CH4_A_GPIO_Port GPIOG
#define CH4_B_Pin GPIO_PIN_8
#define CH4_B_GPIO_Port GPIOG
#define CH1_A_Pin GPIO_PIN_11
#define CH1_A_GPIO_Port GPIOA
#define CH1_B_Pin GPIO_PIN_12
#define CH1_B_GPIO_Port GPIOA
#define LED0_Pin GPIO_PIN_13
#define LED0_GPIO_Port GPIOG
#define LED1_Pin GPIO_PIN_14
#define LED1_GPIO_Port GPIOG
#define MotorStep_Pin GPIO_PIN_8
#define MotorStep_GPIO_Port GPIOB
#define MotorDir_Pin GPIO_PIN_3
#define MotorDir_GPIO_Port GPIOE
#define StepEn_Pin GPIO_PIN_0
#define StepEn_GPIO_Port GPIOE
#define BEEP_Pin GPIO_PIN_1
#define BEEP_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

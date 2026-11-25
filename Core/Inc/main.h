/* USER CODE BEGIN Header */

/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* USER CODE BEGIN EM */

/* USER CODE END EM */

void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#define LED0_Pin GPIO_PIN_13
#define LED0_GPIO_Port GPIOC
#define KEY_ADD_Pin GPIO_PIN_1
#define KEY_ADD_GPIO_Port GPIOA
#define KEY_SUB_Pin GPIO_PIN_2
#define KEY_SUB_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif

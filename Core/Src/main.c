/* USER CODE BEGIN Header */

/* USER CODE END Header */

#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "Servo.h"
#include "OLED.h"

#include <stdio.h>

/* USER CODE BEGIN PD */
#define SEQ_POINT_NUM        5U
#define SEQ_HOLD_MS          1000U
#define KEY_SAMPLE_MS        5U     /* 按键采样周期 */
#define KEY_DEBOUNCE_MS      20U    /* 电平变化后需保持 20ms 才确认 */
#define KEY_STEP_DEG         15.0f
#define KEY_RESUME_MS        5000U
#define HEARTBEAT_MS         500U
#define DISPLAY_REFRESH_MS   250U   /* 周期整屏自愈刷新，防止干扰残留 */
#define UART_TIMEOUT_MS      100U
/* USER CODE END PD */

/* USER CODE BEGIN PV */
static const float s_seq[SEQ_POINT_NUM] = {0.0f, 45.0f, 90.0f, 135.0f, 180.0f};

static uint8_t  s_seqIndex    = 0U;
static uint32_t s_seqTick     = 0U;
static uint8_t  s_seqPaused   = 0U;
static uint32_t s_lastKeyTick = 0U;

static uint32_t s_displayTick  = 0U;
static uint8_t  s_displayDirty = 0U;
static uint32_t s_heartTick    = 0U;

static uint8_t  s_keyStable[2]    = {1U, 1U};
static uint8_t  s_keyCandidate[2] = {1U, 1U};
static uint32_t s_keyCandTick[2]  = {0U, 0U};
static uint32_t s_keyTick         = 0U;
static int      s_lastPrintDeg    = -999;
/* USER CODE END PV */

/* USER CODE BEGIN PFP */
void SystemClock_Config(void);
static void Display_Update(float angle);
static void Uart_SendBanner(void);
static uint8_t Key_Scan(float *delta);
static void Heartbeat_Task(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

int _write(int fd, char *ptr, int len)
{
  (void)fd;
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, (uint16_t)len, UART_TIMEOUT_MS);
  return len;
}

/* 上电自检：不经 stdio 缓冲，直接用 HAL 发一串固定字符。
   串口助手里看得到这一行 → 链路（接线/波特率/共地）没问题，问题在 printf 重定向；
   看不到 → 问题在硬件链路，与代码无关。 */
static void Uart_SendBanner(void)
{
  const char msg[] = "\r\n[BOOT] RM-2027 Servo, USART1 115200 8N1\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, (uint16_t)(sizeof(msg) - 1U), UART_TIMEOUT_MS);
}

static void Display_Update(float angle)
{
  char     line[17];
  uint16_t ccr = Servo_GetCCR();
  int      deg = (int)(angle + 0.5f);

  OLED_BufClear();

  snprintf(line, sizeof(line), "Target:%03ddeg", deg);
  OLED_BufString(1, 1, line);

  snprintf(line, sizeof(line), "CCR   :%4u", (unsigned)ccr);
  OLED_BufString(2, 1, line);

  snprintf(line, sizeof(line), "Pulse :%u.%03ums", (unsigned)(ccr / 1000U), (unsigned)(ccr % 1000U));
  OLED_BufString(3, 1, line);

  OLED_Flush();

  if (deg != s_lastPrintDeg)
  {
    s_lastPrintDeg = deg;
    printf("Target=%03d  CCR=%4u  Pulse=%u.%03ums\r\n",
           deg, (unsigned)ccr, (unsigned)(ccr / 1000U), (unsigned)(ccr % 1000U));
  }
}

static uint8_t Key_Scan(float *delta)
{
  uint32_t now = HAL_GetTick();
  uint8_t  raw[2];
  uint8_t  pressed = 0U;
  uint8_t  i;

  if ((now - s_keyTick) < KEY_SAMPLE_MS)
  {
    return 0U;
  }
  s_keyTick = now;

  raw[0] = (HAL_GPIO_ReadPin(KEY_ADD_GPIO_Port, KEY_ADD_Pin) == GPIO_PIN_RESET) ? 0U : 1U;
  raw[1] = (HAL_GPIO_ReadPin(KEY_SUB_GPIO_Port, KEY_SUB_Pin) == GPIO_PIN_RESET) ? 0U : 1U;

  for (i = 0U; i < 2U; i++)
  {
    if (raw[i] != s_keyCandidate[i])
    {
      s_keyCandidate[i] = raw[i];
      s_keyCandTick[i]  = now;
    }
    else if ((s_keyCandidate[i] != s_keyStable[i]) &&
             ((now - s_keyCandTick[i]) >= KEY_DEBOUNCE_MS))
    {
      s_keyStable[i] = s_keyCandidate[i];

      if (s_keyStable[i] == 0U)
      {
        *delta += (i == 0U) ? KEY_STEP_DEG : -KEY_STEP_DEG;
        pressed = 1U;
      }
    }
  }

  return pressed;
}

static void Heartbeat_Task(void)
{
  uint32_t now = HAL_GetTick();

  if ((now - s_heartTick) >= HEARTBEAT_MS)
  {
    s_heartTick = now;
    HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
  setvbuf(stdout, NULL, _IONBF, 0);   /* printf 不缓冲，立即经 _write 发出 */
  Uart_SendBanner();

  OLED_Init();
  OLED_Clear();

  Servo_Init(&htim2, TIM_CHANNEL_1);
  Servo_SetAngle(s_seq[0]);

  s_seqTick     = HAL_GetTick();
  s_displayTick = s_seqTick;
  s_heartTick   = s_seqTick;

  Display_Update(Servo_GetAngle());

  printf("\r\nRM-2027 servo ready: TIM2_CH1 on PA0, 50Hz, 1us/count\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint32_t now   = HAL_GetTick();
    float    delta = 0.0f;

    if (Key_Scan(&delta) != 0U)
    {
      s_seqPaused    = 1U;
      s_lastKeyTick  = now;
      s_displayDirty = 1U;

      Servo_SetAngle(Servo_GetAngle() + delta);
    }

    if ((s_seqPaused != 0U) && ((now - s_lastKeyTick) >= KEY_RESUME_MS))
    {
      s_seqPaused = 0U;
      s_seqTick   = now;
    }

    if ((s_seqPaused == 0U) && ((now - s_seqTick) >= SEQ_HOLD_MS))
    {
      s_seqTick  = now;
      s_seqIndex = (uint8_t)((s_seqIndex + 1U) % SEQ_POINT_NUM);

      Servo_SetAngle(s_seq[s_seqIndex]);
      s_displayDirty = 1U;
    }

    if ((s_displayDirty != 0U) || ((now - s_displayTick) >= DISPLAY_REFRESH_MS))
    {
      s_displayDirty = 0U;
      s_displayTick  = now;
      Display_Update(Servo_GetAngle());
    }

    Heartbeat_Task();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  (void)file;
  (void)line;
  /* USER CODE END 6 */
}
#endif

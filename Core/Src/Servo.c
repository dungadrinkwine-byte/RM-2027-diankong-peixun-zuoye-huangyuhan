#include "Servo.h"

static TIM_HandleTypeDef *s_htim    = 0;
static uint32_t           s_channel = 0U;
static float              s_angle   = SERVO_ANGLE_MIN;
static uint16_t           s_ccr     = SERVO_PULSE_MIN_US;

uint16_t Servo_AngleToCCR(float angle)
{
  float eff;
  float pulse;

  if (angle < SERVO_ANGLE_MIN)
  {
    angle = SERVO_ANGLE_MIN;
  }
  if (angle > SERVO_ANGLE_MAX)
  {
    angle = SERVO_ANGLE_MAX;
  }

  eff   = angle - SERVO_ZERO_OFFSET_DEG;
  pulse = SERVO_PULSE_MIN_US
        + eff * (float)(SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US) / SERVO_ANGLE_MAX;

  if (pulse < (float)SERVO_PULSE_ABS_MIN_US)
  {
    pulse = (float)SERVO_PULSE_ABS_MIN_US;
  }
  if (pulse > (float)SERVO_PULSE_ABS_MAX_US)
  {
    pulse = (float)SERVO_PULSE_ABS_MAX_US;
  }

  return (uint16_t)(pulse + 0.5f);
}

void Servo_Init(TIM_HandleTypeDef *htim, uint32_t channel)
{
  s_htim    = htim;
  s_channel = channel;
  s_angle   = SERVO_ANGLE_MIN;
  s_ccr     = Servo_AngleToCCR(s_angle);

  __HAL_TIM_SET_COMPARE(s_htim, s_channel, s_ccr);
  HAL_TIM_PWM_Start(s_htim, s_channel);
}

void Servo_SetAngle(float angle)
{
  if (angle < SERVO_ANGLE_MIN)
  {
    angle = SERVO_ANGLE_MIN;
  }
  if (angle > SERVO_ANGLE_MAX)
  {
    angle = SERVO_ANGLE_MAX;
  }

  s_angle = angle;
  s_ccr   = Servo_AngleToCCR(angle);

  if (s_htim != 0)
  {
    __HAL_TIM_SET_COMPARE(s_htim, s_channel, s_ccr);
  }
}

uint16_t Servo_GetCCR(void)
{
  return s_ccr;
}

float Servo_GetAngle(void)
{
  return s_angle;
}

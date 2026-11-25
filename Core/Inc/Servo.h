#ifndef __SERVO_H
#define __SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define SERVO_ANGLE_MIN      0.0f
#define SERVO_ANGLE_MAX      180.0f

/* 两端各收一截，避开机械限位（要强行榨行程可改为 500 / 2500，但两端会顶限位堵转） */
#define SERVO_PULSE_MIN_US   550U
#define SERVO_PULSE_MAX_US   2450U

/* 零点校准：实际角度 = 显示角度 + SERVO_ZERO_OFFSET_DEG
   显示 90° 时若手臂朝 180° 方向多转了 X 度，填 +X；朝 0° 方向差 X 度填 -X */
#define SERVO_ZERO_OFFSET_DEG    0.0f

/* 加零点偏移后允许达到的绝对脉宽界限（避免超出舵机机械极限） */
#define SERVO_PULSE_ABS_MIN_US   500U
#define SERVO_PULSE_ABS_MAX_US   2500U

void     Servo_Init(TIM_HandleTypeDef *htim, uint32_t channel);
void     Servo_SetAngle(float angle);
uint16_t Servo_AngleToCCR(float angle);
uint16_t Servo_GetCCR(void);
float    Servo_GetAngle(void);

#ifdef __cplusplus
}
#endif

#endif

/**
 * @file servo.c
 * @brief 舵机控制实现 (TIM3_CH1/PA6 PWM)
 */

#include "servo.h"
#include "tim.h"

/* 舵机参数 */
#define SERVO_MIN_PULSE_US 500  // 最小脉宽 (-90°)
#define SERVO_MAX_PULSE_US 2500 // 最大脉宽 (+90°)
#define SERVO_PERIOD_US 20000   // PWM周期 (50Hz)

/* 速率限制 */
#define SERVO_MAX_DELTA_DEG 4.0f // 每次最大变化 (度) - 与原代码一致

/* 静态状态 */
static float servo_angle_f = 0.0f; // 当前角度

/**
 * @brief 初始化舵机
 */
void servo_init(void)
{
    /* 启动TIM3_CH1 PWM */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

    /* 设置初始角度到中间位置 */
    servo_set_angle_deg(0.0f);
}

/**
 * @brief 设置舵机角度 (带速率限制)
 */
void servo_set_angle_deg(float angle_deg)
{
    /* 角度限幅 */
    if (angle_deg > 90.0f)
        angle_deg = 90.0f;
    if (angle_deg < -90.0f)
        angle_deg = -90.0f;

    /* 速率限制: 平滑过渡 */
    float delta = angle_deg - servo_angle_f;
    if (delta > SERVO_MAX_DELTA_DEG)
        delta = SERVO_MAX_DELTA_DEG;
    if (delta < -SERVO_MAX_DELTA_DEG)
        delta = -SERVO_MAX_DELTA_DEG;

    servo_angle_f += delta;

    /* 角度 → 脉宽 (us) */
    float pulse_us = SERVO_MIN_PULSE_US +
                     (servo_angle_f + 90.0f) * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US) / 180.0f;

    /* 限制脉宽范围 */
    if (pulse_us < SERVO_MIN_PULSE_US)
        pulse_us = SERVO_MIN_PULSE_US;
    if (pulse_us > SERVO_MAX_PULSE_US)
        pulse_us = SERVO_MAX_PULSE_US;

    /* 脉宽 → CCR值 (基于ARR的精确计算) */
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim3);
    uint32_t period_counts = arr + 1u;
    // 50Hz => 20000us, 使用四舍五入
    uint32_t ccr = (uint32_t)(((uint32_t)pulse_us * period_counts + 10000u) / 20000u);
    if (ccr > arr)
        ccr = arr;

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr);
}

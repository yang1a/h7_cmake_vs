/**
 * @file servo.h
 * @brief 舵机控制接口
 */

#ifndef SERVO_H
#define SERVO_H

/**
 * @brief 初始化舵机 (TIM3_CH1/PA6)
 */
void servo_init(void);

/**
 * @brief 设置舵机角度 (带速率限制)
 * @param angle_deg 目标角度 (度), 范围 [-90, +90]
 */
void servo_set_angle_deg(float angle_deg);

#endif // SERVO_H

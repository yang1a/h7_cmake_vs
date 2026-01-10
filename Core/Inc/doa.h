/**
 * @file doa.h
 * @brief DOA角度计算接口
 */

#ifndef DOA_H
#define DOA_H

/**
 * @brief 从时延计算角度 (deg)
 * @param lag 时延 (采样点), 正值表示左声道在前
 * @return 角度 (度), 范围 [-90, +90], 正值表示声源偏左
 */
float doa_lag_to_angle_deg(float lag);

#endif // DOA_H

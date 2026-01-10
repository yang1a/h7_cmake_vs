/**
 * @file doa.c
 * @brief DOA角度计算实现
 */

#include "doa.h"
#include <math.h>

/* 物理参数 */
#define MIC_SPACING_M 0.12f  // 麦克风间距 (米)
#define SOUND_SPEED_M 343.0f // 声速 (米/秒)
#define FS_HZ 48000.0f       // 采样率 (Hz)

/* 弧度转角度 */
#define RAD_TO_DEG (180.0f / 3.14159265358979323846f)

/**
 * @brief 从时延计算角度
 */
float doa_lag_to_angle_deg(float lag)
{
    /* 时延 → 距离差 */
    float delta_t = lag / FS_HZ;
    float delta_d = delta_t * SOUND_SPEED_M;

    /* asin输入限幅 */
    float ratio = delta_d / MIC_SPACING_M;
    if (ratio > 1.0f)
        ratio = 1.0f;
    if (ratio < -1.0f)
        ratio = -1.0f;

    /* 角度 (弧度 → 度) */
    float angle_rad = asinf(ratio);
    float angle_deg = angle_rad * RAD_TO_DEG;

    return angle_deg;
}

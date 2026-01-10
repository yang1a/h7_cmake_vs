/**
 * @file tdoa.h
 * @brief TDOA时延估计接口
 */

#ifndef TDOA_H
#define TDOA_H

#include <stdint.h>

/**
 * @brief TDOA计算结果
 */
typedef struct
{
    float lag;  ///< 时延 (采样点), 正值表示L在前
    float corr; ///< 相关峰值 (归一化)
} tdoa_result_t;

/**
 * @brief 使用NCC算法计算TDOA
 * @param L 左声道信号 (n个采样点)
 * @param R 右声道信号 (n个采样点)
 * @param n 采样点数量
 * @param max_lag 最大搜索滞后 (±max_lag 范围)
 * @return TDOA结果 (lag和相关峰值)
 */
tdoa_result_t tdoa_ncc_compute(const int16_t *L, const int16_t *R, int n, int max_lag);

#endif // TDOA_H

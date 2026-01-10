/**
 * @file tdoa_ncc.c
 * @brief 归一化互相关 (NCC) 实现的TDOA算法
 */

#include "tdoa.h"
#include <math.h>

/* NCC平滑系数 */
#define LAG_ALPHA 0.75f

/* NCC决策阈值 (与原代码一致) */
#define NONZERO_MARGIN_MIL 10    // (bestNonZeroCorr - corr0) > 10/1000
#define NONZERO_CORR_MIN_MIL 600 // bestNonZeroCorr > 0.600

/* 静态状态 */
static float lag_f = 0.0f; // 平滑后的滞后

/**
 * @brief 使用NCC算法计算TDOA (完整决策逻辑)
 */
tdoa_result_t tdoa_ncc_compute(const int16_t *L, const int16_t *R, int n, int max_lag)
{
    tdoa_result_t result = {0};

    /* 计算L和R的能量 */
    double eL = 0.0, eR = 0.0;
    for (int i = 0; i < n; i++)
    {
        double a = (double)L[i];
        double b = (double)R[i];
        eL += a * a;
        eR += b * b;
    }

    double denom = sqrt(eL * eR);
    if (denom < 1e-9)
    {
        result.lag = lag_f;
        result.corr = 0.0f;
        return result;
    }

    /* 先单独计算lag=0的相关值 */
    double corr0 = 0.0;
    for (int i = 0; i < n; i++)
    {
        corr0 += (double)L[i] * R[i];
    }
    corr0 /= denom;

    /* 搜索非零lag的最大峰值 */
    int bestNZLag = 0;
    double bestNZ = -1e9;

    for (int lag = -max_lag; lag <= max_lag; lag++)
    {
        if (lag == 0)
            continue; // 跳过lag=0

        double dot = 0.0;
        int start = (lag < 0) ? -lag : 0;
        int end = (lag > 0) ? (n - lag) : n;

        for (int i = start; i < end; i++)
        {
            dot += (double)L[i] * (double)R[i + lag];
        }

        double corr = dot / denom;

        if (corr > bestNZ)
        {
            bestNZ = corr;
            bestNZLag = lag;
        }
    }

    /* 决策: 只有当非零峰明显强于corr0才使用,否则返回0(正前方) */
    int final_lag = 0;
    float final_corr = (float)corr0;

    float d = (float)(bestNZ - corr0);
    if ((bestNZ > (NONZERO_CORR_MIN_MIL / 1000.0f)) &&
        (d > (NONZERO_MARGIN_MIL / 1000.0f)))
    {
        final_lag = bestNZLag;
        final_corr = (float)bestNZ;
    }

    /* 低通平滑 */
    lag_f = LAG_ALPHA * lag_f + (1.0f - LAG_ALPHA) * final_lag;

    result.lag = lag_f;
    result.corr = final_corr;
    return result;
}

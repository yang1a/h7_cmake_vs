/**
 * @file app.c
 * @brief 应用层主控逻辑 - 编排音频处理流程
 */

#include "app.h"
#include "audio_io.h"
#include "tdoa.h"
#include "doa.h"
#include "servo.h"
#include "usart.h"
#include <stdio.h>
#include <math.h>

/* 帧参数 */
#define FRAME_SIZE 1024
#define MAX_LAG 17

/* 噪声学习参数 */
#define NOISE_LEARN_FRAMES 200
#define NOISE_ALPHA 0.98f        // 慢速更新(与原代码一致)
#define NOISE_UPDATE_LIMIT 1.20f // 只在功率<1.2倍当前底噪时更新

/* 功率门限 */
#define GATE_ADD_M 3000u // 门限 = 底噪 + 3000 (单位: pwr/1e6)

/* 静态缓冲区 */
static int16_t frameL[FRAME_SIZE];
static int16_t frameR[FRAME_SIZE];

/* 噪声学习状态 */
static float noiseM_f = 0.0f;
static int learn_cnt = 0;

/**
 * @brief 计算帧功率 (L+R平方和)
 */
static uint64_t frame_power_u64(const int16_t *L, const int16_t *R, int n)
{
    uint64_t sum = 0;
    for (int i = 0; i < n; i++)
    {
        int32_t a = L[i];
        int32_t b = R[i];
        sum += (uint64_t)(a * a + b * b);
    }
    return sum;
}

/**
 * @brief 初始化应用层
 */
void app_init(void)
{
    /* 初始化各模块 */
    audio_io_init();
    servo_init();

    /* 串口问候 */
    printf("\r\n=== Sound Localization System ===\r\n");
    printf("Learning noise floor...\r\n");
}

/**
 * @brief 运行一次主控流程
 */
void app_run_once(void)
{
    /* 1. 获取音频帧 (已做预处理: 解交织、去直流、高通滤波) */
    if (!audio_io_get_frame(frameL, frameR, FRAME_SIZE))
    {
        return; // 无新数据
    }

    /* 2. 计算双声道功率 (单位: /1e6) */
    uint64_t pwr = frame_power_u64(frameL, frameR, FRAME_SIZE);
    uint32_t pwrM_u = (uint32_t)(pwr / 1000000ULL);

    /* 3. 噪声学习: 前200帧建立底噪模型 */
    if (learn_cnt < NOISE_LEARN_FRAMES)
    {
        float pwrM = (float)pwrM_u;

        if (noiseM_f < 1e-3f)
            noiseM_f = pwrM;

        // 只在功率 < 1.2倍当前底噪时更新(避免响声污染)
        if (pwrM < noiseM_f * NOISE_UPDATE_LIMIT)
        {
            noiseM_f = NOISE_ALPHA * noiseM_f + (1.0f - NOISE_ALPHA) * pwrM;
        }
        learn_cnt++;

        if (learn_cnt == NOISE_LEARN_FRAMES)
        {
            printf("Noise floor: %lu (ready)\r\n", (unsigned long)noiseM_f);
        }
        return;
    }

    /* 4. 功率门限: 信号低于底噪+3000则跳过处理 */
    uint32_t noiseM_u = (uint32_t)(noiseM_f + 0.5f);
    uint32_t gateM_u = noiseM_u + GATE_ADD_M;

    if (pwrM_u < gateM_u)
    {
        return;
    }

    /* 5. TDOA计算 */
#ifdef USE_GCC_PHAT
    tdoa_result_t tdoa = tdoa_gcc_phat_compute(frameL, frameR, FRAME_SIZE, MAX_LAG);
#else
    tdoa_result_t tdoa = tdoa_ncc_compute(frameL, frameR, FRAME_SIZE, MAX_LAG);
#endif

    /* 6. DOA角度计算 */
    float angle_deg = doa_lag_to_angle_deg(tdoa.lag);

    /* 7. 舵机控制 */
    servo_set_angle_deg(angle_deg);

    /* 8. 串口输出 */
    printf("pow=%lu lag=%d/100 corr=%d/1000 ang=%d/10\r\n",
           (unsigned long)pwrM_u,
           (int)(tdoa.lag * 100.0f),
           (int)(tdoa.corr * 1000.0f),
           (int)(angle_deg * 10.0f));
}

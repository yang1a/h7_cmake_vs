/**
 * @file audio_io.c
 * @brief 音频采样实现 - ADC2双通道DMA (PC4左, PB1右)
 */

#include "audio_io.h"
#include "adc.h"
#include "tim.h"
#include "main.h"
#include <string.h>

/* 配置参数 */
#define FRAME_SAMPLES_PER_CH 1024
#define DMA_WORDS_PER_FRAME (FRAME_SAMPLES_PER_CH * 2) // L0 R0 L1 R1...
#define DMA_TOTAL_WORDS (DMA_WORDS_PER_FRAME * 2)      // 双缓冲

/* DMA缓冲区 */
__attribute__((section(".dma_buffer"), aligned(32))) static uint16_t adc_dma_buf[DMA_TOTAL_WORDS];

/* 帧就绪标志和指针 */
static volatile uint8_t frame_ready = 0;
static const uint16_t *frame_ptr = NULL;

/* 预处理状态 */
typedef struct
{
    float prev_x;
    float prev_y;
} HP1State;

static HP1State hpL = {0}, hpR = {0};

#define HP_A 0.995f // 高通滤波系数

/* DCache失效 */
static inline void cache_invalidate(const uint16_t *p, size_t bytes)
{
    SCB_InvalidateDCache_by_Addr((uint32_t *)p, (int32_t)bytes);
}

/* 解交织: uint16[L0R0L1R1...] -> int16[L...] int16[R...] */
static void deinterleave_to_i16(const uint16_t *src, int16_t *left, int16_t *right, int n)
{
    for (int i = 0; i < n; i++)
    {
        left[i] = (int16_t)((int32_t)src[2 * i + 0] - 32768);
        right[i] = (int16_t)((int32_t)src[2 * i + 1] - 32768);
    }
}

/* 去直流 */
static void remove_mean_i16(int16_t *x, int n)
{
    int64_t sum = 0;
    for (int i = 0; i < n; i++)
        sum += x[i];
    int32_t mean = (int32_t)(sum / n);
    for (int i = 0; i < n; i++)
        x[i] = (int16_t)(x[i] - mean);
}

/* 高通滤波 (一阶IIR) */
static inline int16_t hp1_iir_i16(int16_t x, HP1State *st)
{
    float y = (float)x - st->prev_x + HP_A * st->prev_y;
    st->prev_x = (float)x;
    st->prev_y = y;

    if (y > 32767.0f)
        y = 32767.0f;
    if (y < -32768.0f)
        y = -32768.0f;
    return (int16_t)y;
}

/* DMA半完成回调 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC2)
    {
        frame_ptr = &adc_dma_buf[0];
        frame_ready = 1;
    }
}

/* DMA完成回调 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC2)
    {
        frame_ptr = &adc_dma_buf[DMA_WORDS_PER_FRAME];
        frame_ready = 1;
    }
}

/**
 * @brief 初始化音频采样
 */
void audio_io_init(void)
{
    /* ADC校准 */
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);

    /* 清空DCache */
    SCB_CleanDCache_by_Addr((uint32_t *)adc_dma_buf, sizeof(adc_dma_buf));

    /* 启动ADC DMA */
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc_dma_buf, DMA_TOTAL_WORDS);

    /* 启动TIM2触发 (48kHz) */
    HAL_TIM_Base_Start(&htim2);
}

/**
 * @brief 获取一帧音频数据
 * @return 1=获取成功, 0=无新数据
 */
int audio_io_get_frame(int16_t *L, int16_t *R, int n)
{
    if (n != FRAME_SAMPLES_PER_CH)
        return 0;

    if (!frame_ready)
        return 0;

    const uint16_t *src = frame_ptr;
    frame_ready = 0;

    /* 使DCache失效 */
    cache_invalidate(src, DMA_WORDS_PER_FRAME * sizeof(uint16_t));

    /* 解交织 */
    deinterleave_to_i16(src, L, R, n);

    /* 去直流 */
    remove_mean_i16(L, n);
    remove_mean_i16(R, n);

    /* 高通滤波 */
    for (int i = 0; i < n; i++)
    {
        L[i] = hp1_iir_i16(L[i], &hpL);
        R[i] = hp1_iir_i16(R[i], &hpR);
    }

    return 1;
}

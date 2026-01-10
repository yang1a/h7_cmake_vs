/**
 * @file audio_io.h
 * @brief 音频采样接口 - 当前使用ADC2双通道DMA
 * @note 未来升级AD7606时只需修改audio_io.c实现
 */

#ifndef __AUDIO_IO_H
#define __AUDIO_IO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 初始化音频采样
     * @note 启动ADC2 DMA + TIM2触发
     */
    void audio_io_init(void);

    /**
     * @brief 获取一帧音频数据
     * @param L 左声道输出缓冲区
     * @param R 右声道输出缓冲区
     * @param n 每通道采样点数
     * @return 1=成功获取新帧, 0=无新数据
     */
    int audio_io_get_frame(int16_t *L, int16_t *R, int n);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_IO_H */

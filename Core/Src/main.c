/* USER CODE BEGIN Header */
/**
 * @file main.c
 * @brief STM32H7 声源定位系统 - 精简入口
 */
/* USER CODE END Header */

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "app.h"

extern void SystemClock_Config(void);
extern void MPU_Config(void);

/**
 * @brief 主函数入口
 */
int main(void)
{
  /* 系统初始化 */
  MPU_Config();
  SCB_EnableICache();
  SCB_EnableDCache();
  
  HAL_Init();
  SystemClock_Config();
  
  /* 外设初始化 */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  
  /* 应用层初始化 */
  app_init();
  
  /* 主循环 */
  while (1)
  {
    app_run_once();
  }
}


void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif

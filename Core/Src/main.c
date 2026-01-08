#include "main.h"
#include "adc.h"
#include "dma.h"
#include "dma2d.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "system/system_config.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/

#define ADC_BUFFER_SIZE 100
__attribute__((section(".dma_buffer"))) uint16_t adc_buffer[ADC_BUFFER_SIZE];
uint8_t uart_tx_buffer[256];
volatile uint8_t adc_conversion_complete = 0;

/* Private function prototypes -----------------------------------------------*/
int _write(int file, char *ptr, int len);
void Print_ADC_Data(void);

int main(void)
{
  SCB_EnableICache();
  SCB_EnableDCache();
  HAL_Init();
  MPU_Config();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_DMA2D_Init();
  MX_ADC2_Init();
  MX_TIM2_Init();
  MX_RTC_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();

  printf("\r\n=== STM32H7 DMA ADC Test ===\r\n");
  printf("ADC Buffer Address: 0x%08lX\r\n", (uint32_t)adc_buffer);

  HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);

  SCB_CleanDCache_by_Addr((uint32_t *)adc_buffer, sizeof(adc_buffer));

  if (HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc_buffer, ADC_BUFFER_SIZE) != HAL_OK)
  {
    printf("Error: ADC DMA start failed!\r\n");
    Error_Handler();
  }

  HAL_TIM_Base_Start(&htim2);
  printf("ADC DMA started, waiting for data...\r\n");

  while (1)
  {
    if (adc_conversion_complete)
    {
      adc_conversion_complete = 0;
      Print_ADC_Data();
    }
    HAL_Delay(100);
  }
}



void Print_ADC_Data(void)
{
  static uint32_t count = 0;
  uint32_t sum_ch1 = 0, sum_ch2 = 0;

  SCB_InvalidateDCache_by_Addr((uint32_t *)adc_buffer, sizeof(adc_buffer));

  for (int i = 0; i < ADC_BUFFER_SIZE; i += 2)
  {
    sum_ch1 += adc_buffer[i];
    sum_ch2 += adc_buffer[i + 1];
  }

  uint16_t avg_ch1 = sum_ch1 / (ADC_BUFFER_SIZE / 2);
  uint16_t avg_ch2 = sum_ch2 / (ADC_BUFFER_SIZE / 2);

  printf("[%lu] ADC CH4: %u (%.2fV), CH5: %u (%.2fV)\r\n",
         count++, avg_ch1, avg_ch1 * 3.3f / 65536.0f,
         avg_ch2, avg_ch2 * 3.3f / 65536.0f);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC2)
  {
    adc_conversion_complete = 1;
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
}
#endif

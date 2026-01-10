
#include "dma2d.h"

DMA2D_HandleTypeDef hdma2d;


void MX_DMA2D_Init(void)
{

  

  

  

  
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_R2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = 0;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  

  

}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* dma2dHandle)
{

  if(dma2dHandle->Instance==DMA2D)
  {
  

  
    
    __HAL_RCC_DMA2D_CLK_ENABLE();
  

  
  }
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* dma2dHandle)
{

  if(dma2dHandle->Instance==DMA2D)
  {
  

  
    
    __HAL_RCC_DMA2D_CLK_DISABLE();
  

  
  }
}


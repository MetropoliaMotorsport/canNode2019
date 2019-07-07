#include "main.h"

//external variables
extern DMA_HandleTypeDef hdma_adc1;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;
extern CAN_HandleTypeDef hcan1;

void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(hadc->Instance==hadc1.Instance)
	{
		__HAL_RCC_ADC1_CLK_ENABLE();
		//__HAL_RCC_GPIOA_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		hdma_adc1.Instance = DMA1_Channel1;
		hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
		hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		hdma_adc1.Init.Mode = DMA_CIRCULAR;
		hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
		if(HAL_DMA_Init(&hdma_adc1) != HAL_OK)
		{
			Error_Handler();
		}

		__HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);
		HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
	}
	else
	{
		Error_Handler();
	}
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
	if(hadc->Instance==hadc1.Instance)
	{
		__HAL_RCC_ADC1_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
		HAL_DMA_DeInit(hadc->DMA_Handle);
		HAL_NVIC_DisableIRQ(ADC1_2_IRQn);
	}
	else
	{
		Error_Handler();
	}
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	if(htim_base->Instance==htim3.Instance)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
	else
	{
		Error_Handler();
	}
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
	if(htim_base->Instance==htim3.Instance)
	{
		__HAL_RCC_TIM3_CLK_DISABLE();

		HAL_NVIC_DisableIRQ(TIM3_IRQn);
	}
	else
	{
		Error_Handler();
	}
}

void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(hcan->Instance==hcan1.Instance)
	{
		__HAL_RCC_CAN1_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_8;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		__HAL_AFIO_REMAP_CAN1_2();

		HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	}
	else
	{
		Error_Handler();
	}
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan)
{
	if(hcan->Instance==hcan1.Instance)
	{
		__HAL_RCC_CAN1_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);
		HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
	}
	else
	{
		Error_Handler();
	}
}

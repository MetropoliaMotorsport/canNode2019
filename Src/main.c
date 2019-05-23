#include "main.h"

//definitions and comments //maybe we should also have lookup tables, and in that case maybe the configuring over flash should be dropped completely
//this stuff should be moved to flash, but I don't feel like messing with that right now
#define fiveVoltDivider 6.40656410285 //22k/39k, also includes conversion to mV
#define twelveVoltDivider 15.3600000016 //33k/12k, also includes conversion to mV

//this should definitely be moved to flash in the future
//more complex equations should be used anyway, this way is kind of dumb
uint32_t sensorAdjustment[8] =
{
	(200/fiveVoltDivider),	//brake temperature sensor in C or K
	(200/fiveVoltDivider),	//brake temperature sensor in C or K
	(6.25/twelveVoltDivider),	//rear suspension sensor when powered by 12V in mm
	(6.25/twelveVoltDivider),	//rear suspension sensor when powered by 12V in mm
	(66.6017642561/fiveVoltDivider),	//front suspension sensor in mm
	(66.6017642561/fiveVoltDivider),	//front suspension sensor in mm
	1,	//unused
	1	//unused
};

int32_t sensorAdjustmentB[8] =
{
		-100,		//brake temperature sensor in C
		-100,		//brake temperature sensor in C
		0,
		0,
		0,
		0,
		0,
		0
};

//this corresponds to above things and should also be moved to flash in the future definitely
//this should probably be determined programatically in the future, maybe, especially if in the flash
uint32_t significantByteShift[8] =
{
	4,
	4,
	4,
	4,
	0,	//we want least significant bits for these because the rotation thing I think
	0,	//same as above
	4,
	4
};

/* ID 7FF, F6 FF 0 1A 0 64 */ //this is for six sensors on PA1, PA2, PA4, PA5, PA6, and PA7; and it sends every 10 mS on ID 1A

//constants
const int adc_channels[9] = {ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_3, ADC_CHANNEL_4, ADC_CHANNEL_5, ADC_CHANNEL_6, ADC_CHANNEL_7, ADC_CHANNEL_8};
const int adc_ranks[9] = {ADC_REGULAR_RANK_1, ADC_REGULAR_RANK_2, ADC_REGULAR_RANK_3, ADC_REGULAR_RANK_4, ADC_REGULAR_RANK_5, ADC_REGULAR_RANK_6, ADC_REGULAR_RANK_7, ADC_REGULAR_RANK_8, ADC_REGULAR_RANK_9};

//global variables
int adc1Values[ADC1_INPUTS_NUMB_MAX], adc1Buffer[ADC1_INPUTS_NUMB_MAX];

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
TIM_HandleTypeDef htim3;
CAN_HandleTypeDef hcan1;
CAN_TxHeaderTypeDef txHeader1;
CAN_RxHeaderTypeDef rxHeader1;
uint8_t txData1[8];
uint8_t rxData1[8];
uint32_t txMailbox1;

#if WATCHDOG
	IWDG_HandleTypeDef hiwdg;
#endif

//function prototypes
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_CAN_Init(void);

void Flash_Write(uint32_t, uint32_t[256], int);
uint32_t Flash_Read(uint32_t);

//external functions
extern void FLASH_PageErase(uint32_t PageAddress);

#if WATCHDOG
	static void MX_IWDG_Init(void);
#endif

volatile uint x=0;

int main(void)
{
	HAL_Init();

	SystemClock_Config();

	if(Flash_Read(FLASH_PAGE_63)==0xFFFFFFFF) //initialize the flash to avoid errors
	{
		uint32_t data[256] = {0};

		data[ADC_ENABLES_POS]=0x03;
		data[ADC1_INPUTS_NUMB_POS]=2;
		data[ADC_SAMPLE_RATE_POS] = ADC_SAMPLETIME_239CYCLES_5;
		data[CAN_ID_POS]=0x01;
		data[CAN_DLC_POS]=4;
		data[MESSAGE_TIM_PERIOD_POS]=1000;

		Flash_Write(FLASH_PAGE_63, data, 6);
	}

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	MX_TIM3_Init();
	MX_CAN_Init();

	if(HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}

	if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc1Buffer, ADC1_INPUTS_NUMB) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_TIM_Base_Start_IT(&htim3);
	if(HAL_CAN_Start(&hcan1) != HAL_OK)
	{
		Error_Handler();
	}
	if(HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		Error_Handler();
	}

	txHeader1.StdId = CAN_ID;
	txHeader1.ExtId = 0x01;
	txHeader1.RTR = CAN_RTR_DATA;
	txHeader1.IDE = CAN_ID_STD;
	txHeader1.DLC = CAN_DLC;
	txHeader1.TransmitGlobalTime = DISABLE;



#if WATCHDOG
	MX_IWDG_Init();
#endif

	while (1)
	{
#if WATCHDOG
		if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK)
		{
			Error_Handler();
		}
#endif

		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_Delay(Flash_Read(FLASH_PAGE_63+(0x4*4)));
	}
}

void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
	  Error_Handler();
	}

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
	if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
}

static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	//enable GPIO Port Clocks
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	//reset pins
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);

	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

static void MX_DMA_Init(void)
{
	__HAL_RCC_DMA1_CLK_ENABLE();

	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

static void MX_ADC1_Init(void)
{
	ADC_ChannelConfTypeDef sConfig = {0};

	hadc1.Instance = ADC1;
	hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
	hadc1.Init.ContinuousConvMode = ENABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = ADC1_INPUTS_NUMB;
	if(HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}

	sConfig.SamplingTime = ADC_SAMPLE_RATE;

	int j=0;
	for(int i=0; i<ADC1_INPUTS_NUMB_MAX; i++)
	{
		if((ADC_ENABLES&(1<<i)))
		{
			sConfig.Channel=adc_channels[i];
			sConfig.Rank=adc_ranks[j];
			if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
			{
				Error_Handler();
			}
			j++;
		}
	}
}

static void MX_TIM3_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 6399;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = MESSAGE_TIM_PERIOD;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if(HAL_TIM_Base_Init(&htim3) != HAL_OK)
	{
		Error_Handler();
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if(HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if(HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
}

static void MX_CAN_Init(void)
{
	CAN_FilterTypeDef sFilterConfig;

	hcan1.Instance = CAN1;
	hcan1.Init.Prescaler = 2;
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
	hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
	hcan1.Init.Mode = CAN_MODE_NORMAL;
	hcan1.Init.TimeTriggeredMode = DISABLE;
	hcan1.Init.AutoBusOff = DISABLE;
	hcan1.Init.AutoWakeUp = DISABLE;
	hcan1.Init.AutoRetransmission = DISABLE;
	hcan1.Init.TransmitFifoPriority = DISABLE;
	if(HAL_CAN_Init(&hcan1) != HAL_OK)
	{
		Error_Handler();
	}

	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0xFFE0; //0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0xFFE0; //0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;
	if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
	{
		Error_Handler();
	}
}

#if WATCHDOG
	static void MX_IWDG_Init(void)
	{
		hiwdg.Instance = IWDG;
		hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
		hiwdg.Init.Reload = 999; //~780 ms for some reason?
		if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
		{
			Error_Handler();
		}
	}
#endif



void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *AdcHandle)
{
	if(AdcHandle->Instance==hadc1.Instance)
	{
		for(int i=0; i<ADC1_INPUTS_NUMB; i++)
		{
			adc1Values[i]=adc1Buffer[i];
		}
	}
	else
	{
		Error_Handler();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == htim3.Instance)
	{
		if(ADC_ENABLES)
		{
			txData1[0]=0x00; txData1[1]=0x00; txData1[2]=0x00; txData1[3]=0x00; txData1[4]=0x00; txData1[5]=0x00; txData1[6]=0x00; txData1[7]=0x00;

			for(int i=0; i<ADC1_INPUTS_NUMB; i++) //this won't work if ADC1_INPUTS_NUMB > 8
			{
				uint32_t data = adc1Values[i]*sensorAdjustment[i]+sensorAdjustmentB[i];
				if(ADC1_INPUTS_NUMB<=0/*4*/) //this should be fixed in the case that 4 or fewer sensors are used
				{
					txData1[i*2]=adc1Values[i]>>8;
					txData1[i*2+1]=adc1Values[i];
				}
				else
				{
					txData1[i]=data>>significantByteShift[i];
				}
			}

			HAL_CAN_AddTxMessage(&hcan1, &txHeader1, txData1, &txMailbox1);
		}
	}
	else
	{
		Error_Handler();
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *CanHandle)
{
	if(HAL_CAN_GetRxMessage(CanHandle, CAN_RX_FIFO0, &rxHeader1, rxData1) != HAL_OK)
	{
		Error_Handler();
	}

	if(CanHandle->Instance==hcan1.Instance)
	{
		if(rxHeader1.StdId==0x7ff)
		{
			uint32_t data[256] = {0};

			data[ADC_ENABLES_POS]=rxData1[0];
			data[ADC1_INPUTS_NUMB_POS]=0;
			for(int i=0; i<8; i++) //8 is because we are sending a byte, even though max inputs is 9 everywhere else that will never be reached setting values this way
			{
				if(rxData1[0]&(1<<i))
				{
					data[ADC1_INPUTS_NUMB_POS]+=1;
				}
			}
			if(rxData1[1]>((239-79)/2)+79) { data[ADC_SAMPLE_RATE_POS] = ADC_SAMPLETIME_239CYCLES_5; }
			else if(rxData1[1]>((79-55)/2)+55) { data[ADC_SAMPLE_RATE_POS] = ADC_SAMPLETIME_71CYCLES_5; }
			else if(rxData1[1]>((55-41)/2)+41) { data[ADC_SAMPLE_RATE_POS] = ADC_SAMPLETIME_55CYCLES_5; }
			else if(rxData1[1]>((41-28)/2)+28) { data[ADC_SAMPLE_RATE_POS] = ADC_SAMPLETIME_41CYCLES_5; }
			else if(rxData1[1]>((28-13)/2)+13) { data[ADC_SAMPLE_RATE_POS] = ADC_SAMPLETIME_28CYCLES_5; }
			else if(rxData1[1]>((13-7)/2)+7) { data[ADC_SAMPLE_RATE_POS] = ADC_SAMPLETIME_13CYCLES_5; }
			else if(rxData1[1]>((7-1)/2)+1) { data[ADC_SAMPLE_RATE_POS] = ADC_SAMPLETIME_7CYCLES_5; }
			else { data[ADC_SAMPLE_RATE_POS] = ADC_SAMPLETIME_1CYCLE_5; }

			data[CAN_ID_POS]=((rxData1[2]<<8)+rxData1[3]);
			if(data[ADC1_INPUTS_NUMB_POS]<=4) {data[CAN_DLC_POS]=data[ADC1_INPUTS_NUMB_POS]*2;}
			else if(data[ADC1_INPUTS_NUMB_POS]<=8) {data[CAN_DLC_POS]=data[ADC1_INPUTS_NUMB_POS];}
			else {data[CAN_DLC_POS]=8;} //this shouldn't actually happen in this section

			data[MESSAGE_TIM_PERIOD_POS]=(rxData1[4]<<8)+rxData1[5];

			Flash_Write(FLASH_PAGE_63, data, 6);
		}
	}
	else
	{
		Error_Handler();
	}
}



void Flash_Write(uint32_t Flash_Address, uint32_t Flash_Data[256], int Data_Words)
{
	FLASH_EraseInitTypeDef pEraseInit;
	uint32_t pError = 0;

	pEraseInit.PageAddress = Flash_Address;
	pEraseInit.NbPages = 1;
	pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;

	__disable_irq();
	if(HAL_FLASH_Unlock() != HAL_OK)
	{
		__enable_irq();
		Error_Handler();
	}

	while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != 0) { }

	HAL_FLASHEx_Erase(&pEraseInit, &pError);
	for(int i=0; i<Data_Words; i++)
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Flash_Address+i*0x04, Flash_Data[i]) != HAL_OK)
		{
			__enable_irq();
			Error_Handler();
		}
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != 0) { }
	}

	if(HAL_FLASH_Lock() != HAL_OK)
	{
		__enable_irq();
		Error_Handler();
	}
	__enable_irq();
}

uint32_t Flash_Read(uint32_t Flash_Address)
{
	return *(uint32_t*)Flash_Address;
}



void Error_Handler(void)
{
	while(1)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_Delay(33);
	}
}



#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif

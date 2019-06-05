#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>



//includes
#include "stm32f1xx_hal.h"

//definitions
#define WATCHDOG 0

#define ADC_CHANNELS 6
#define ADC_SAMPLE_RATE ADC_SAMPLETIME_239CYCLES_5

#define CAN_ID 0x1A
#define CAN_DLC 8

#define MESSAGE_TIM_PERIOD 100 //this is in 100s of uSs

#define ROLLAVGCNT 10


//exported function prototypes
void Error_Handler(void);



#ifdef __cplusplus
}
#endif

#endif

#ifndef SENSORFUNCTIONS_H_
#define SENSORFUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "main.h"



//function prototypes
uint32_t average_adcs(uint32_t adc_values[ROLLAVGCNT]);

uint16_t INFKL800(uint32_t adc_value);
int8_t linearPot750mm12V(uint32_t adc_value);
int8_t RSC28xxx3621x_frontSuspension(uint32_t adc_value); //to be split and changed

uint8_t frontLeftSuspension(uint32_t adc_value);
uint8_t frontRightSuspension(uint32_t adc_value);
uint8_t rearRightSuspension(uint32_t adc_value);
uint8_t rearLeftSuspension(uint32_t adc_value);



#ifdef __cplusplus
}
#endif

#endif

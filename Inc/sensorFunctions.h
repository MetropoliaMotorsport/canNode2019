#ifndef SENSORFUNCTIONS_H_
#define SENSORFUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>



//function prototypes
uint16_t INFKL800(int adc_value);
int8_t linearPot750mm12V(int adc_value);
int8_t RSC28xxx3621x_frontSuspension(int adc_value, int offsetAngle);



#ifdef __cplusplus
}
#endif

#endif

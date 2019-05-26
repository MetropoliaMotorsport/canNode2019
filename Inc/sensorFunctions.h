#ifndef SENSORFUNCTIONS_H_
#define SENSORFUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif



//function prototypes
int INFKL800(int adc_value);
int linearPot750mm12V(int adc_value);
int RSC28xxx3621x_frontSuspension(int adc_value, int offsetAngle);



#ifdef __cplusplus
}
#endif

#endif

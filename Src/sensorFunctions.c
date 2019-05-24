#include "sensorFunctions.h"



//definitions
#define fiveVoltMultiplier (330000/(2^12))*(39/(39+22)) //22k/39k, also includes conversion to 10's of uVs
#define twelveVoltMultiplier (330000/(2^12))*(12/(12+33)) //33k/12k, also includes conversion to 10's of uVs



int INFKL800(int adc_value)
{
	return (((adc_value*fiveVoltMultiplier)/500)-100);
}

int linearPot750mm12V(int adc_value)
{
	return (adc_value*fiveVoltMultiplier)*(750/12);
}

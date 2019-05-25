#include "sensorFunctions.h"



//definitions
#define fiveVoltMultiplier (33000*(22+39))/(39*(1<<12)) //22k/39k, also includes conversion to 10s of uVs
#define twelveVoltMultiplier (33000/(1<<12))*((12+33)/12) //33k/12k, also includes conversion to 10s of uVs



int INFKL800(int adc_value)
{
	return ((adc_value*fiveVoltMultiplier)/50)-100;
}

int linearPot750mm12V(int adc_value)
{
	return (adc_value*fiveVoltMultiplier)*(750/12);
}

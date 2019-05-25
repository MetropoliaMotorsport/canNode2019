#include "sensorFunctions.h"



//definitions
#define fiveVoltMultiplier (33000*(22+39))/(39*(1<<12)) //22k/39k, also includes conversion to 10s of uVs
#define twelveVoltMultiplier (33000*(12+33))/(12*(1<<12)) //33k/12k, also includes conversion to 10s of uVs



int INFKL800(int adc_value)
{
	return ((adc_value*fiveVoltMultiplier)/50)-100;
}

int linearPot750mm12V(int adc_value)
{
	return ((adc_value*twelveVoltMultiplier*750)/120000);
}

//.25 to 4.75v radiometric to 5V supply

int RSC28xxx3621x_frontSuspension(int adc_value)
{
	int theta=((adc_value*fiveVoltMultiplier)/1250)-20;
	return theta; //we don't actually care about the angle, but with this we can try and check that angle is correct
}

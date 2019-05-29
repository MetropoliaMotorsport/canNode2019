#include "sensorFunctions.h"



//definitions
#define fiveVoltMultiplier ((33000*(22+39))/(39*(1<<12))) //22k/39k, also includes conversion to 10s of uVs
#define twelveVoltMultiplier ((33000*(12+33))/(12*(1<<12))) //33k/12k, also includes conversion to 10s of uVs



uint16_t INFKL800(int adc_value)
{
	return ((adc_value*fiveVoltMultiplier)/50)-100;
}

volatile int x;

int8_t linearPot750mm12V(int adc_value)
{
	x=((adc_value*twelveVoltMultiplier*750)/120000);
	return ((adc_value*twelveVoltMultiplier*750)/120000);
}

int8_t RSC28xxx3621x_frontSuspension(int adc_value, int offsetAngle) //offset angle isn't proper way to do this, but it will work
{
	int theta=((adc_value*fiveVoltMultiplier)/125)-20;
	x=(theta+offsetAngle)*335/360;
	return (theta+offsetAngle)*335/360; //almost 1 mm/degree, should be improved with lookup table, maybe even straight from raw values
}

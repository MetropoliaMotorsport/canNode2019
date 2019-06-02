#include "sensorFunctions.h"



//definitions
#define fiveVoltMultiplier ((33000*(22+39))/(39*(1<<12))) //22k/39k, also includes conversion to 10s of uVs
#define twelveVoltMultiplier ((33000*(12+33))/(12*(1<<12))) //33k/12k, also includes conversion to 10s of uVs



uint32_t average_adcs(uint32_t adc_values[ROLLAVGCNT])
{
	uint32_t total=0;
	for(uint32_t i=0; i<ROLLAVGCNT; i++)
	{
		total+=adc_values[i];
	}
	//return total/ROLLAVGCNT;
	return adc_values[0];
}

uint16_t INFKL800(uint32_t adc_value)
{
	return ((adc_value*fiveVoltMultiplier)/5)-1000;
}

int8_t linearPot750mm12V(uint32_t adc_value) //this should also be tried to be fixed and probably split into two sometime
{
	return ((adc_value*twelveVoltMultiplier*75)/120000);
}

//this should be changed 36 to 12, and lookup table should be made sometime, or at least redo the equations for that
int8_t RSC28xxx3621x_frontSuspension(uint32_t adc_value) //120 degrees, split to left and right as well becuase they go different ways
{
	int theta=((adc_value*fiveVoltMultiplier)/375)-20;
	return (theta)*335/360; //almost 1 mm/degree, should be improved with lookup table, maybe even straight from raw values
}

volatile int x, y;

uint8_t frontLeftSuspension(uint32_t adc_value)
{
	return 248-(adc_value/32);
}

uint8_t frontRightSuspension(uint32_t adc_value)
{
	return (adc_value/32)+100;
}

uint8_t rearLeftSuspension(uint32_t adc_value)
{
	x=adc_value;
	y=208-(adc_value/51);
	return 208-(adc_value/51);
}

uint8_t rearRightSuspension(uint32_t adc_value)
{
	return 208-(adc_value/53);
}

#include <cassert>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <wiringPi.h>
#include "StepperMotor.h"

using namespace std;


// Default constructor
StepperMotor::StepperMotor() {
	// http://wiringpi.com/pins/
	pulse = 0;
	pinMode(pulse, OUTPUT);
	direction = 1;
	pinMode(direction, OUTPUT);
	enable = 2;
	pinMode(enable, OUTPUT);

	digitalWrite(pulse, HIGH);
	digitalWrite(direction, LOW);
	digitalWrite(enable, LOW);
}


// Runs the stepper motor.
// * direction: 1 to go clockwise, -1 to go counterclockwise
// * angle: can assume values from 0 to 360 degrees
// * speed: from 20% (minimum speed) to 100% (maximum speed)
void StepperMotor::run(int direction, unsigned angle)
{
    float td;
    unsigned nsteps, count, ndegrees;
    unsigned steps = angle * 4;//Umrechnung Winkel  zu Anzahl steps
    //set enable HIGH
    digitalWrite(enable, HIGH);
    usleep(5);
    if(direction == 1)
    {
    	digitalWrite(direction, HIGH);
    }
    else if(direction == -1)
    {
    	digitalWrite(direction, LOW);
    }
    usleep(5);

    unsigned toggle = 0;
    for(unsigned step = 0; step < steps; step++)
    {
    	if(toggle == 1)
    	{
    		digitalWrite(pulse, HIGH);
    		toggle = 0;
			sleep(5);
    	}
    	else if(toggle == 0)
    	{
    		digitalWrite(pulse, LOW);
    		toggle = 1;
			sleep(5);
    	}
    	usleep(3);
    }
    digitalWrite(enable, LOW);
}


// Sends to sleep the stepper motor for a certain amount of time (in milliseconds)
void StepperMotor::wait(unsigned milliseconds) const {
    delay(milliseconds);
}

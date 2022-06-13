// Copyright (c) 2016, Cristiano Urban (http://crish4cks.net)
//
// A simple C++ class created to provide an easily interaction with 
// the geared stepper motor 28BYJ48 through the ULN2003APG driver.
//

#ifndef STEPPER_MOTOR_HPP
#define STEPPER_MOTOR_HPP

using namespace std;

class StepperMotor {
    public:
        StepperMotor();
        void run(int direction, unsigned angle);
        void wait(unsigned milliseconds) const;

    private:
        unsigned pulse, direction, enable;           // stepper motor driver inputs
};

#endif

#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <FastAccelStepper.h>

const int DIR_G = 4;
const int STEP_G = 13;
const int DIR_D = 0;
const int STEP_D = 15;

class MotorDriver {
  private:
    FastAccelStepperEngine engine;
    FastAccelStepper *motorG;
    FastAccelStepper *motorD;

    int maxSpeed;
    float currentLeftSpeed;
    float currentRightSpeed;

  public:
    MotorDriver();

    void begin(int stepG, int dirG, int stepD, int dirD, int maxSpd);
    void setSpeed(float left, float right);
    float leftSpeed() const;
    float rightSpeed() const;
};

#endif

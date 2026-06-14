#include "MotorDriver.h"

MotorDriver::MotorDriver() {
  motorG = nullptr;
  motorD = nullptr;
  currentLeftSpeed = 0.0;
  currentRightSpeed = 0.0;
}

void MotorDriver::begin(int stepG, int dirG, int stepD, int dirD, int maxSpd) {

  maxSpeed = maxSpd;

  engine.init();

  motorG = engine.stepperConnectToPin(stepG);
  motorD = engine.stepperConnectToPin(stepD);

  if (motorG) {
    motorG->setDirectionPin(dirG);
    motorG->setAutoEnable(true);
    motorG->setAcceleration(5000); // Réduit de 10000 à 5000 pour éviter le décrochage
  }

  if (motorD) {
    motorD->setDirectionPin(dirD);
    motorD->setAutoEnable(true);
    motorD->setAcceleration(5000); // Réduit de 10000 à 5000
  }
}

void MotorDriver::setSpeed(float left, float right) {

  left  = constrain(left,  -maxSpeed, maxSpeed);
  right = constrain(right, -maxSpeed, maxSpeed);

  currentLeftSpeed = left;
  currentRightSpeed = right;

  if (motorG) {
    uint32_t absLeft = abs((int)left);
    motorG->setSpeedInHz(absLeft);
    if (left > 0) {
       motorG->runBackward();
      //motorG->runForward();
    } else if (left < 0) {
      //motorG->runBackward();
      motorG->runForward();
    } else {
      motorG->stopMove();
    }
  }

  if (motorD) {
    uint32_t absRight = abs((int)right);
    motorD->setSpeedInHz(absRight);
    if (right > 0) {
      motorD->runForward();
    } else if (right < 0) {
      motorD->runBackward();
    } else {
      motorD->stopMove();
    }
  }
}

float MotorDriver::leftSpeed() const {
  return currentLeftSpeed;
}

float MotorDriver::rightSpeed() const {
  return currentRightSpeed;
}

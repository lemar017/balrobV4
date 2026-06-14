#ifndef IMU_TYPES_H
#define IMU_TYPES_H

#include <Arduino.h>

struct ImuSample
{
  int16_t accX = 0;
  int16_t accY = 0;
  int16_t accZ = 0;
  int16_t gyroX = 0;
  int16_t gyroY = 0;
  int16_t gyroZ = 0;
};

struct FilteredImuSample
{
  float accX = 0.0f;
  float accY = 0.0f;
  float accZ = 0.0f;
  float gyroX = 0.0f;
  float gyroY = 0.0f;
  float gyroZ = 0.0f;
};

struct PidSettings
{
  float targetAngleBase = 0.0f;
  double kp = 0.0;
  double ki = 0.0;
  double kd = 0.0;
};

struct JoystickState
{
  float x = 0.0f;
  float y = 0.0f;
};

struct BalanceTelemetry
{
  float accAngle = 0.0f;
  float gyroRate = 0.0f;
  float gyroAngle = 0.0f;
  float currentAngle = 0.0f;
  float targetAngle = 0.0f;
  float error = 0.0f;
  float errorSum = 0.0f;
  float p = 0.0f;
  float i = 0.0f;
  float d = 0.0f;
  float motorPid = 0.0f;
  float tiltRequest = 0.0f;
};

#endif

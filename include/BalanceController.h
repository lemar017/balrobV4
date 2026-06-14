#ifndef BALANCE_CONTROLLER_H
#define BALANCE_CONTROLLER_H

#include <ImuTypes.h>

class BalanceController
{
public:
  float compute(float dt, const FilteredImuSample &imu, const JoystickState &joystick, const PidSettings &settings);
  const BalanceTelemetry &telemetry() const;

private:
  float previousAngle_ = 0.0f;
  float angleErrorSum_ = 0.0f;
  float speedErrorSum_ = 0.0f;
  float currentSpeed_ = 0.0f;
  BalanceTelemetry telemetry_;
};

#endif

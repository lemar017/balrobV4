#include <Arduino.h>
#include <BalanceController.h>
#include <ModuleVAR-GLOB.h>

float BalanceController::compute(float dt, const FilteredImuSample &imu, const JoystickState &joystick, const PidSettings &settings)
{
  telemetry_.accAngle = atan2(imu.accZ, imu.accX) * RAD_TO_DEG;
  telemetry_.gyroRate = imu.gyroY / GYRO_SENS;
  telemetry_.gyroAngle = telemetry_.gyroRate * dt;
  telemetry_.currentAngle = 0.98f * (previousAngle_ + telemetry_.gyroAngle) + 0.02f * telemetry_.accAngle;

  float joystickY = joystick.y;
  if (abs(joystickY) < 5.0f)
  {
    joystickY = 0.0f;
  }

  const float targetSpeed = (joystickY / 100.0f) * 800.0f;
  currentSpeed_ = 0.8f * currentSpeed_ + 0.2f * (telemetry_.motorPid * 10.0f);

  const float speedError = targetSpeed - currentSpeed_;
  if (joystickY == 0.0f && abs(currentSpeed_) < 10.0f)
  {
    speedErrorSum_ *= 0.9f;
  }
  else
  {
    speedErrorSum_ += speedError * dt;
  }
  speedErrorSum_ = constrain(speedErrorSum_, -4000.0f, 4000.0f);

  const float kpSpeed = 0.0025f;
  const float kiSpeed = 0.00025f;
  telemetry_.tiltRequest = kpSpeed * speedError + kiSpeed * speedErrorSum_;

  const float maxAngle = 4.0f;
  float speedNorm = abs(telemetry_.motorPid) / 400.0f;
  if (speedNorm > 1.0f)
  {
    speedNorm = 1.0f;
  }

  const float dynamicMax = maxAngle * (1.0f - 0.5f * speedNorm);
  telemetry_.tiltRequest = constrain(telemetry_.tiltRequest, -dynamicMax, dynamicMax);
  telemetry_.targetAngle = settings.targetAngleBase + telemetry_.tiltRequest;

  telemetry_.error = telemetry_.currentAngle - telemetry_.targetAngle;
  angleErrorSum_ += telemetry_.error * dt;
  angleErrorSum_ = constrain(angleErrorSum_, -10.0f, 10.0f);
  telemetry_.errorSum = angleErrorSum_;

  telemetry_.p = settings.kp * telemetry_.error;
  telemetry_.i = settings.ki * angleErrorSum_;
  telemetry_.d = settings.kd * telemetry_.gyroRate;
  telemetry_.motorPid = telemetry_.p + telemetry_.i + telemetry_.d;

  previousAngle_ = telemetry_.currentAngle;
  return telemetry_.motorPid;
}

const BalanceTelemetry &BalanceController::telemetry() const
{
  return telemetry_;
}

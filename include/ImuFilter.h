#ifndef IMU_FILTER_H
#define IMU_FILTER_H

#include <ModuleVAR-GLOB.h>
#include <ImuTypes.h>

class ImuFilter
{
public:
  explicit ImuFilter(FiltreType type = MOYENNE, int intervalMs = 10);

  void setType(FiltreType type);
  void setIntervalMs(int intervalMs);
  FilteredImuSample update(const ImuSample &sample);

private:
  static const int kWindowSize = 10;

  FiltreType type_;
  int intervalMs_;
  int bufferIndex_;
  float accXBuffer_[kWindowSize];
  float accYBuffer_[kWindowSize];
  float accZBuffer_[kWindowSize];
  float gyroXBuffer_[kWindowSize];
  float gyroYBuffer_[kWindowSize];
  float gyroZBuffer_[kWindowSize];
  FilteredImuSample filtered_;
};

#endif

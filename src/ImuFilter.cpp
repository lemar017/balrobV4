#include <ImuFilter.h>

ImuFilter::ImuFilter(FiltreType type, int intervalMs)
    : type_(type), intervalMs_(intervalMs), bufferIndex_(0)
{
  for (int i = 0; i < kWindowSize; i++)
  {
    accXBuffer_[i] = 0.0f;
    accYBuffer_[i] = 0.0f;
    accZBuffer_[i] = 0.0f;
    gyroXBuffer_[i] = 0.0f;
    gyroYBuffer_[i] = 0.0f;
    gyroZBuffer_[i] = 0.0f;
  }
}

void ImuFilter::setType(FiltreType type)
{
  type_ = type;
}

void ImuFilter::setIntervalMs(int intervalMs)
{
  intervalMs_ = intervalMs;
}

FilteredImuSample ImuFilter::update(const ImuSample &sample)
{
  const float accXRaw = static_cast<float>(sample.accX);
  const float accYRaw = static_cast<float>(sample.accY);
  const float accZRaw = static_cast<float>(sample.accZ);
  const float gyroXRaw = static_cast<float>(sample.gyroX);
  const float gyroYRaw = static_cast<float>(sample.gyroY);
  const float gyroZRaw = static_cast<float>(sample.gyroZ);

  switch (type_)
  {
  case MOYENNE:
  {
    accXBuffer_[bufferIndex_] = accXRaw;
    accYBuffer_[bufferIndex_] = accYRaw;
    accZBuffer_[bufferIndex_] = accZRaw;
    gyroXBuffer_[bufferIndex_] = gyroXRaw;
    gyroYBuffer_[bufferIndex_] = gyroYRaw;
    gyroZBuffer_[bufferIndex_] = gyroZRaw;

    filtered_ = FilteredImuSample();
    for (int i = 0; i < kWindowSize; i++)
    {
      filtered_.accX += accXBuffer_[i];
      filtered_.accY += accYBuffer_[i];
      filtered_.accZ += accZBuffer_[i];
      filtered_.gyroX += gyroXBuffer_[i];
      filtered_.gyroY += gyroYBuffer_[i];
      filtered_.gyroZ += gyroZBuffer_[i];
    }

    filtered_.accX /= kWindowSize;
    filtered_.accY /= kWindowSize;
    filtered_.accZ /= kWindowSize;
    filtered_.gyroX /= kWindowSize;
    filtered_.gyroY /= kWindowSize;
    filtered_.gyroZ /= kWindowSize;
    bufferIndex_ = (bufferIndex_ + 1) % kWindowSize;
    break;
  }

  case EMA:
  {
    const float alpha = 0.2f;
    filtered_.accX = alpha * accXRaw + (1.0f - alpha) * filtered_.accX;
    filtered_.accY = alpha * accYRaw + (1.0f - alpha) * filtered_.accY;
    filtered_.accZ = alpha * accZRaw + (1.0f - alpha) * filtered_.accZ;
    filtered_.gyroX = alpha * gyroXRaw + (1.0f - alpha) * filtered_.gyroX;
    filtered_.gyroY = alpha * gyroYRaw + (1.0f - alpha) * filtered_.gyroY;
    filtered_.gyroZ = alpha * gyroZRaw + (1.0f - alpha) * filtered_.gyroZ;
    break;
  }

  case COMPLEMENTAIRE:
  {
    const float dt = intervalMs_ / 1000.0f;
    filtered_.accX = 0.98f * (filtered_.accX + accXRaw * dt) + 0.02f * accXRaw;
    filtered_.accY = 0.98f * (filtered_.accY + accYRaw * dt) + 0.02f * accYRaw;
    filtered_.accZ = 0.98f * (filtered_.accZ + accZRaw * dt) + 0.02f * accZRaw;
    filtered_.gyroX = 0.98f * (filtered_.gyroX + gyroXRaw * dt) + 0.02f * gyroXRaw;
    filtered_.gyroY = 0.98f * (filtered_.gyroY + gyroYRaw * dt) + 0.02f * gyroYRaw;
    filtered_.gyroZ = 0.98f * (filtered_.gyroZ + gyroZRaw * dt) + 0.02f * gyroZRaw;
    break;
  }

  case KALMAN:
    break;
  }

  return filtered_;
}

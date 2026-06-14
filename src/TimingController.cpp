#include "TimingController.h"

/**
 * @file TimingController.cpp
 * @brief Implémentation de la gestion du timing
 */

TimingController::TimingController(
    unsigned long pidIntervalUs,
    unsigned long displayIntervalUs,
    unsigned long ledIntervalUs)
    : pidIntervalUs_(pidIntervalUs),
      displayIntervalUs_(displayIntervalUs),
      ledIntervalUs_(ledIntervalUs),
      telemetryIntervalUs_(1000000), // 1 seconde
      lastPIDTime_(0),
      lastDisplayTime_(0),
      lastLEDTime_(0),
      lastTelemetryTime_(0),
      lastLoopTime_(0),
      startTime_(micros()),
      iterationCount_(0),
      loopFrequencyHz_(0.0f),
      averageLoopTimeMs_(0.0f),
      accumulatedLoopTime_(0),
      currentTime_(0),
      deltaTimeMicros_(0)
{
        // Initialisation
}

// ===== MISE À JOUR DES TIMERS =====
void TimingController::update()
{
        currentTime_ = micros();

        // Calculer le delta time
        if (lastLoopTime_ > 0)
        {
                deltaTimeMicros_ = currentTime_ - lastLoopTime_;
        }
        else
        {
                deltaTimeMicros_ = pidIntervalUs_; // Valeur par défaut au premier appel
        }

        lastLoopTime_ = currentTime_;

        // Mettre à jour les statistiques tous les 1000ms
        if ((currentTime_ - lastTelemetryTime_) >= 1000000)
        {
                updateLoopStatistics_();
                lastTelemetryTime_ = currentTime_;
        }

        iterationCount_++;
        accumulatedLoopTime_ += deltaTimeMicros_;
}

// ===== VÉRIFICATIONS DE TEMPS =====
bool TimingController::shouldExecutePID()
{
        if ((currentTime_ - lastPIDTime_) >= pidIntervalUs_)
        {
                lastPIDTime_ = currentTime_;
                return true;
        }
        return false;
}

bool TimingController::shouldDisplayData()
{
        if ((currentTime_ - lastDisplayTime_) >= displayIntervalUs_)
        {
                lastDisplayTime_ = currentTime_;
                return true;
        }
        return false;
}

bool TimingController::shouldToggleLED()
{
        if ((currentTime_ - lastLEDTime_) >= ledIntervalUs_)
        {
                lastLEDTime_ = currentTime_;
                return true;
        }
        return false;
}

bool TimingController::shouldUpdateTelemetry()
{
        if ((currentTime_ - lastTelemetryTime_) >= telemetryIntervalUs_)
        {
                lastTelemetryTime_ = currentTime_;
                return true;
        }
        return false;
}

// ===== DELTA TIME =====
float TimingController::getDeltaTimeSeconds()
{
        return deltaTimeMicros_ / 1000000.0f;
}

float TimingController::getDeltaTimeMillis()
{
        return deltaTimeMicros_ / 1000.0f;
}

unsigned long TimingController::getDeltaTimeMicros()
{
        return deltaTimeMicros_;
}

// ===== STATISTIQUES PRIVÉES =====
void TimingController::updateLoopStatistics_()
{
        // Calculer la fréquence moyenne
        unsigned long elapsedTime = currentTime_ - startTime_;
        if (elapsedTime > 0)
        {
                loopFrequencyHz_ = (iterationCount_ * 1000000.0f) / elapsedTime;
        }

        // Calculer le temps moyen d'une itération
        if (iterationCount_ > 0)
        {
                averageLoopTimeMs_ = accumulatedLoopTime_ / (iterationCount_ * 1000.0f);
        }

        // Debug
        Serial.printf("[TIMING] Freq: %.1f Hz, AvgTime: %.2f ms\n",
                      loopFrequencyHz_, averageLoopTimeMs_);
}

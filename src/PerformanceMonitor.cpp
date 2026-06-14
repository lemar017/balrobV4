#include "PerformanceMonitor.h"
#include <ArduinoJson.h>

/**
 * @file PerformanceMonitor.cpp
 * @brief Implémentation de la télémétrie et affichage unifié
 */

PerformanceMonitor::PerformanceMonitor(OutputFormat format, bool autoFlush)
    : currentFormat_(format),
      enabled_(true),
      verbosity_(2),
      logCount_(0),
      errorCount_(0),
      lastStatisticsTime_(0),
      averageLogsPerSecond_(0.0f)
{
}

// ===== LOGGING PRINCIPAL =====
void PerformanceMonitor::logData(const RobotState &state)
{
        if (!enabled_)
                return;

        switch (currentFormat_)
        {
        case OutputFormat::RAW:
                logRaw_(state);
                break;
        case OutputFormat::FILTERED:
                logFiltered_(state);
                break;
        case OutputFormat::PID_ONLY:
                logPIDOnly_(state);
                break;
        case OutputFormat::JOYSTICK:
                logJoystick_(state);
                break;
        case OutputFormat::MOTOR_CMD:
                logMotorCommands_(state);
                break;
        case OutputFormat::CSV:
                logCSV_(state);
                break;
        case OutputFormat::CSV_EXTENDED:
                logCSVExtended_(state);
                break;
        case OutputFormat::BINARY:
                logBinary_(state);
                break;
        case OutputFormat::DEBUG:
                logDebug_(state);
                break;
        case OutputFormat::TELEMETRY:
                logTelemetry_(state);
                break;
        }

        logCount_++;
        updateStatistics_();
}

void PerformanceMonitor::logIMUData(const RobotState &state)
{
        if (!enabled_)
                return;
        logRaw_(state);
}

void PerformanceMonitor::logPIDData(const RobotState &state)
{
        if (!enabled_)
                return;
        logPIDOnly_(state);
}

void PerformanceMonitor::logMotorData(const RobotState &state)
{
        if (!enabled_)
                return;
        logMotorCommands_(state);
}

void PerformanceMonitor::logSystemTelemetry(const RobotState &state)
{
        if (!enabled_)
                return;
        logTelemetry_(state);
}

// ===== LOGGING FORMATÉ =====
void PerformanceMonitor::logMessage(const String &message, bool includeTimestamp)
{
        if (!shouldLog_(1))
                return;

        if (includeTimestamp)
        {
                Serial.print(getTimestamp_());
                Serial.print(" ");
        }
        Serial.println(message);
}

void PerformanceMonitor::logValue(const String &label, float value, int precision)
{
        if (!shouldLog_(2))
                return;

        Serial.print(label);
        Serial.print(": ");
        Serial.println(value, precision);
}

void PerformanceMonitor::logError(const String &error)
{
        errorCount_++;
        Serial.print("[ERREUR] ");
        Serial.println(error);
}

void PerformanceMonitor::logWarning(const String &warning)
{
        if (!shouldLog_(1))
                return;
        Serial.print("[AVERTISSEMENT] ");
        Serial.println(warning);
}

// ===== MÉTHODES DE FORMATAGE PRIVÉES =====
void PerformanceMonitor::logRaw_(const RobotState &state)
{
        // Format: accX,accY,accZ,gyroX,gyroY,gyroZ (brut)
        Serial.printf("%d,%d,%d,%d,%d,%d\n",
                      state.imu.raw.accX, state.imu.raw.accY, state.imu.raw.accZ,
                      state.imu.raw.gyroX, state.imu.raw.gyroY, state.imu.raw.gyroZ);
}

void PerformanceMonitor::logFiltered_(const RobotState &state)
{
        // Format: accX,accY,accZ,gyroX,gyroY,gyroZ (filtré)
        Serial.printf("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                      state.imu.filtered.accX, state.imu.filtered.accY, state.imu.filtered.accZ,
                      state.imu.filtered.gyroX, state.imu.filtered.gyroY, state.imu.filtered.gyroZ);
}

void PerformanceMonitor::logPIDOnly_(const RobotState &state)
{
        // Format: accAngle,gyroRate,gyroAngle,currentAngle,error,errorSum,targetAngle,P,I,D
        Serial.printf("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                      state.imu.accAngle, state.imu.gyroRate, state.imu.gyroAngle,
                      state.imu.currentAngle, state.pid.state.error, state.pid.state.errorSum,
                      state.pid.params.targetAngle,
                      state.pid.state.P, state.pid.state.I, state.pid.state.D);
}

void PerformanceMonitor::logJoystick_(const RobotState &state)
{
        Serial.printf("JOY: X=%d, Y=%d, Active=%d\n",
                      state.joystick.current.x, state.joystick.current.y,
                      state.joystick.current.isActive);
}

void PerformanceMonitor::logMotorCommands_(const RobotState &state)
{
        Serial.printf("MOTORS: L=%.0f, R=%.0f\n",
                      state.motors.current.speedLeft, state.motors.current.speedRight);
}

void PerformanceMonitor::logCSV_(const RobotState &state)
{
        // Format complet CSV
        Serial.printf("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.0f,%.0f,%.2f\n",
                      state.imu.accAngle, state.imu.gyroRate, state.imu.gyroAngle,
                      state.imu.currentAngle, state.pid.state.error, state.pid.state.errorSum,
                      state.pid.params.targetAngle,
                      state.pid.state.P, state.pid.state.I, state.pid.state.D,
                      state.motors.current.speedLeft, state.motors.current.speedRight,
                      state.pid.motorOutput);
}

void PerformanceMonitor::logCSVExtended_(const RobotState &state)
{
        // CSV avec toutes les données
        logCSV_(state);
        Serial.printf(",%.0f,%.0f,%.2f,%.2f\n",
                      state.imu.raw.accX, state.imu.raw.accY,
                      state.timing.deltaTimeS, state.telemetry.loopFrequencyHz);
}

void PerformanceMonitor::logBinary_(const RobotState &state)
{
        // Format binaire compact
        int16_t values[] = {
            state.imu.raw.accX, state.imu.raw.accY, state.imu.raw.accZ,
            state.imu.raw.gyroX, state.imu.raw.gyroY, state.imu.raw.gyroZ};

        for (int i = 0; i < 6; i++)
        {
                Serial.write((uint8_t)(values[i] >> 8));
                Serial.write((uint8_t)(values[i] & 0xFF));
        }
}

void PerformanceMonitor::logDebug_(const RobotState &state)
{
        // Format lisible pour debug
        Serial.println("=== DEBUG STATE ===");
        Serial.printf("Angle: %.2f° (cible: %.2f°)\n",
                      state.imu.currentAngle, state.pid.params.targetAngle);
        Serial.printf("Erreur: %.2f (somme: %.2f)\n",
                      state.pid.state.error, state.pid.state.errorSum);
        Serial.printf("PID: P=%.2f, I=%.2f, D=%.2f → %.2f\n",
                      state.pid.state.P, state.pid.state.I, state.pid.state.D,
                      state.pid.motorOutput);
        Serial.printf("Moteurs: G=%.0f, D=%.0f\n",
                      state.motors.current.speedLeft, state.motors.current.speedRight);
        Serial.printf("Joystick: X=%d, Y=%d\n",
                      state.joystick.current.x, state.joystick.current.y);
}

void PerformanceMonitor::logTelemetry_(const RobotState &state)
{
        Serial.printf("TELEMETRY: Heap=%lu bytes, Freq=%.1f Hz, CPU=%.1f%%\n",
                      state.telemetry.freeHeap,
                      state.telemetry.loopFrequencyHz,
                      state.telemetry.cpuUsagePercent);
}

// ===== UTILITAIRES PRIVÉS =====
String PerformanceMonitor::getTimestamp_() const
{
        unsigned long ms = millis();
        unsigned long sec = ms / 1000;
        unsigned long ms_part = ms % 1000;

        static char buf[16];
        snprintf(buf, sizeof(buf), "[%02lu:%03lu]", sec, ms_part);
        return String(buf);
}

void PerformanceMonitor::updateStatistics_()
{
        unsigned long currentTime = millis();
        unsigned long elapsed = currentTime - lastStatisticsTime_;

        if (elapsed >= 1000)
        { // Tous les 1000ms
                averageLogsPerSecond_ = (logCount_ * 1000.0f) / (elapsed + 1);
                lastStatisticsTime_ = currentTime;
        }
}

String PerformanceMonitor::getStatisticsJSON() const
{
        StaticJsonDocument<256> doc;
        doc["logCount"] = logCount_;
        doc["errorCount"] = errorCount_;
        doc["logsPerSecond"] = averageLogsPerSecond_;
        doc["format"] = static_cast<int>(currentFormat_);
        doc["enabled"] = enabled_;

        String output;
        serializeJson(doc, output);
        return output;
}

void PerformanceMonitor::resetCounters()
{
        logCount_ = 0;
        errorCount_ = 0;
        averageLogsPerSecond_ = 0.0f;
}

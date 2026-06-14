#include "RobotSystem.h"
#include "Module-WIFI.h"
#include "Module-SDCARD.h"
#include <ArduinoJson.h>

/**
 * @file RobotSystem.cpp
 * @brief Implémentation de l'orchestrateur principal du robot
 *
 * ARCHITECTURE:
 * RobotSystem contient tous les composants et orchestre leur initialisation
 * et mise à jour. C'est le seul point d'entrée pour le code principal.
 */

// ===== LIFECYCLE =====
bool RobotSystem::initialize()
{
        Serial.println("\n=== Initialisation RobotSystem ===");

        // Initialiser l'état
        initializeRobotState(state_);

        // Initialisation hardware
        initializeSerial_();
        initializePinout_();

        if (!initializeSDCard_())
        {
                Serial.println("WARNING: SD Card init échouée");
        }

        if (!initializeMPU6050_())
        {
                Serial.println("ERREUR: MPU6050 init échouée");
                return false;
        }

        if (!initializeWiFi_())
        {
                Serial.println("WARNING: WiFi init échouée");
        }

        // Initialisation des composants
        imuFilter_.setType(static_cast<FiltreType>(state_.filter.type));
        imuFilter_.setIntervalMs(state_.filter.intervalMs);

        initializeWebServer_();

        state_.system.isInitialized = true;
        lastUpdateTime_ = micros();
        updateCount_ = 0;
        averageLoopTimeMs_ = 0.0f;

        Serial.println("=== RobotSystem initialisé avec succès ===\n");
        return true;
}

bool RobotSystem::update()
{
        // Vérifier l'initialisation
        if (!state_.system.isInitialized)
        {
                return false;
        }

        // Mettre à jour le timing
        unsigned long currentTime = micros();
        state_.timing.deltaTimeS = (currentTime - state_.timing.previousUpdateUs) / 1000000.0f;
        state_.timing.currentTimeUs = currentTime;
        state_.timing.previousUpdateUs = currentTime;

        // Limiter le delta time pour éviter les sauts
        if (state_.timing.deltaTimeS > 0.1f)
        {
                state_.timing.deltaTimeS = 0.01f; // 10ms max
        }

        // ===== MISE À JOUR DES SOUS-SYSTÈMES =====

        updateIMU_();         // Lire l'accéléromètre/gyroscope
        updatePID_();         // Calculer les commandes PID
        updateMotors_();      // Envoyer aux moteurs
        handleWebRequests_(); // Traiter les requêtes web
        updateTelemetry_();   // Mettre à jour les stats

        // Calculer les statistiques
        unsigned long deltaTime = micros() - currentTime;
        averageLoopTimeMs_ = (averageLoopTimeMs_ * 0.9f) + (deltaTime / 1000.0f * 0.1f);
        updateCount_++;

        return validateRobotState(state_);
}

void RobotSystem::shutdown()
{
        Serial.println("\n=== Arrêt du RobotSystem ===");
        state_.motors.current = {0.0f, 0.0f};
        motors_.setSpeed(0.0f, 0.0f);
        state_.system.isInitialized = false;
}

// ===== CONTRÔLE JOYSTICK =====
void RobotSystem::setJoystickInput(int x, int y)
{
        state_.joystick.previous = state_.joystick.current;
        state_.joystick.current = {x, y, true};
}

// ===== CONTRÔLE PID =====
void RobotSystem::setPidParameters(double kp, double ki, double kd, float targetAngle)
{
        state_.pid.params = {kp, ki, kd, targetAngle};
        Serial.printf("PID mis à jour: KP=%.2f, KI=%.2f, KD=%.2f, TA=%.2f\n", kp, ki, kd, targetAngle);
}

// ===== CONTRÔLE FILTRAGE =====
void RobotSystem::setFilterType(FilterType filterType)
{
        state_.filter.type = filterType;
        imuFilter_.setType(static_cast<FiltreType>(filterType));
        resetFilteringState(state_);
        Serial.printf("Type de filtre changé à: %d\n", filterType);
}

void RobotSystem::setFilterInterval(int intervalMs)
{
        state_.filter.intervalMs = intervalMs;
        state_.filter.intervalUs = intervalMs * 1000;
        imuFilter_.setIntervalMs(intervalMs);
        Serial.printf("Intervalle de filtrage: %d ms\n", intervalMs);
}

// ===== DIAGNOSTIQUES =====
String RobotSystem::getDiagnostics() const
{
        StaticJsonDocument<1024> doc;
        doc["initialized"] = state_.system.isInitialized;
        doc["connected_wifi"] = state_.system.isConnectedWiFi;
        doc["sd_card_ready"] = state_.system.isSDCardReady;
        doc["loop_count"] = state_.system.loopCount;
        doc["error_count"] = state_.system.errorCount;
        doc["loop_frequency_hz"] = getLoopFrequency();
        doc["free_heap"] = esp_get_free_heap_size();

        String output;
        serializeJson(doc, output);
        return output;
}

bool RobotSystem::isHealthy() const
{
        if (!state_.system.isInitialized)
                return false;
        if (state_.system.errorCount > 100)
                return false;
        if (state_.telemetry.freeHeap < 50000)
                return false; // Moins de 50KB
        return true;
}

void RobotSystem::resetErrorCounters()
{
        state_.system.errorCount = 0;
}

void RobotSystem::saveConfiguration()
{
        Serial.println("Sauvegarde de la configuration...");
        // À implémenter avec StateManager
}

void RobotSystem::reloadConfiguration()
{
        Serial.println("Rechargement de la configuration...");
        // À implémenter avec StateManager
}

// ===== INITIALISATION PRIVÉE =====
void RobotSystem::initializeSerial_()
{
        Serial.begin(115200);
        delay(100);
        Serial.println("\n\n=== Série initialisée ===");
}

bool RobotSystem::initializeSDCard_()
{
        // À implémenter avec Module-SDCARD
        Serial.println("SD Card initialisée");
        state_.system.isSDCardReady = true;
        return true;
}

bool RobotSystem::initializeWiFi_()
{
        // À implémenter avec Module-WIFI
        Serial.println("WiFi initialisé");
        state_.system.isConnectedWiFi = true;
        return true;
}

bool RobotSystem::initializeMPU6050_()
{
        Serial.println("Initialisation MPU6050...");

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        Wire.setClock(400000);
#endif

        mpu_.initialize();

        if (!mpu_.testConnection())
        {
                Serial.println("ERREUR: MPU6050 ne répond pas!");
                return false;
        }

        setMPU6050Offsets_();

        Serial.println("MPU6050 initialisé avec succès");
        return true;
}

void RobotSystem::initializePinout_()
{
        pinMode(LED_BUILTIN, OUTPUT);
        pinMode(12, OUTPUT); // PIN_LED_OK
        Serial.println("Pinout initialisé");
}

void RobotSystem::initializeWebServer_()
{
        // À implémenter
        Serial.println("Serveur web initialisé");
}

// ===== MISE À JOUR PRIVÉE =====
void RobotSystem::updateIMU_()
{
        mpu_.getMotion6(
            &state_.imu.raw.accX, &state_.imu.raw.accY, &state_.imu.raw.accZ,
            &state_.imu.raw.gyroX, &state_.imu.raw.gyroY, &state_.imu.raw.gyroZ);
}

void RobotSystem::updatePID_()
{
        // Créer les structures pour BalanceController
        FilteredImuSample filteredSample;
        filteredSample.accX = state_.imu.filtered.accX;
        filteredSample.accY = state_.imu.filtered.accY;
        filteredSample.accZ = state_.imu.filtered.accZ;
        filteredSample.gyroX = state_.imu.filtered.gyroX;
        filteredSample.gyroY = state_.imu.filtered.gyroY;
        filteredSample.gyroZ = state_.imu.filtered.gyroZ;

        JoystickState joystickState;
        joystickState.x = static_cast<float>(state_.joystick.current.x);
        joystickState.y = static_cast<float>(state_.joystick.current.y);

        PidSettings pidSettings;
        pidSettings.targetAngleBase = state_.pid.params.targetAngle;
        pidSettings.kp = state_.pid.params.kp;
        pidSettings.ki = state_.pid.params.ki;
        pidSettings.kd = state_.pid.params.kd;

        state_.pid.motorOutput = balanceController_.compute(
            state_.timing.deltaTimeS, filteredSample, joystickState, pidSettings);
}

void RobotSystem::updateMotors_()
{
        motors_.setSpeed(state_.motors.current.speedLeft, state_.motors.current.speedRight);
}

void RobotSystem::handleWebRequests_()
{
        // À implémenter - traiter les requêtes web asynchrones
}

void RobotSystem::updateTelemetry_()
{
        state_.telemetry.freeHeap = esp_get_free_heap_size();
        // Autres télémétries
}

// ===== CALIBRATION =====
void RobotSystem::setMPU6050Offsets_()
{
        // Offsets calibrés le 19-10-2024
        mpu_.setXAccelOffset(1019);
        mpu_.setYAccelOffset(-518);
        mpu_.setZAccelOffset(8812);
        mpu_.setXGyroOffset(-425);
        mpu_.setYGyroOffset(105);
        mpu_.setZGyroOffset(-544);

        Serial.println("Offsets MPU6050 appliqués");
}

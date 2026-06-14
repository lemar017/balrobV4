#include "RobotState.h"

/**
 * @file RobotState.cpp
 * @brief Implémentation des utilitaires de RobotState
 */

// ===== INITIALISATION PAR DÉFAUT =====
void initializeRobotState(RobotState &state)
{
        // IMU
        state.imu.raw = {0, 0, 0, 0, 0, 0};
        state.imu.filtered = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        state.imu.accAngle = 0.0f;
        state.imu.gyroRate = 0.0f;
        state.imu.gyroAngle = 0.0f;
        state.imu.currentAngle = 0.0f;

        // PID
        state.pid.params = {0.1, 0.05, 0.05, 1.5f}; // Valeurs par défaut
        state.pid.state = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        state.pid.motorOutput = 0.0f;

        // Joystick
        state.joystick.current = {0, 0, false};
        state.joystick.previous = {0, 0, false};

        // Moteurs
        state.motors.current = {0.0f, 0.0f};
        state.motors.previous = {0.0f, 0.0f};

        // Filtrage
        state.filter = {FILTER_EMA, 20, 20000};

        // Timing
        state.timing.currentTimeUs = 0;
        state.timing.previousUpdateUs = 0;
        state.timing.deltaTimeS = 0.0f;

        // Système
        state.system = {false, false, false, 0, 0};

        // Télémétrie
        state.telemetry = {0.0f, 0, 0.0f};
}

// ===== RÉINITIALISATION DU FILTRAGE =====
void resetFilteringState(RobotState &state)
{
        state.imu.filtered = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        state.imu.accAngle = 0.0f;
        state.imu.gyroRate = 0.0f;
        state.imu.gyroAngle = 0.0f;
        state.imu.currentAngle = 0.0f;
}

// ===== VALIDATION D'ÉTAT =====
bool validateRobotState(const RobotState &state)
{
        // Vérifier que les angles ne sont pas NaN ou infinis
        if (isnan(state.imu.currentAngle) || isinf(state.imu.currentAngle))
        {
                Serial.println("ERREUR: Angle du robot invalide!");
                return false;
        }

        // Vérifier que les vitesses moteurs sont raisonnables
        if (abs(state.motors.current.speedLeft) > 2000.0f ||
            abs(state.motors.current.speedRight) > 2000.0f)
        {
                Serial.println("ERREUR: Vitesse moteur hors limites!");
                return false;
        }

        // Vérifier que les paramètres PID sont positifs
        if (state.pid.params.kp < 0.0 || state.pid.params.ki < 0.0 ||
            state.pid.params.kd < 0.0)
        {
                Serial.println("ERREUR: Paramètres PID invalides!");
                return false;
        }

        // Vérifier que le delta time est positif
        if (state.timing.deltaTimeS < 0.0f)
        {
                Serial.println("ERREUR: Delta time invalide!");
                return false;
        }

        return true;
}

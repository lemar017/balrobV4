#ifndef ROBOT_STATE_H
#define ROBOT_STATE_H

#include <Arduino.h>

/**
 * @file RobotState.h
 * @brief Structure centralisée pour tout l'état du robot
 * @details Remplace ModuleVAR-GLOB.h - une seule source de vérité
 * 
 * AVANTAGES:
 * - Pas de variables extern éparpillées
 * - État cohérent et prévisible
 * - Facile à sérialiser/désérialiser
 * - Facilite le debugging
 */

// ==================== TYPES IMU ====================
struct ImuRaw {
    int16_t accX, accY, accZ;
    int16_t gyroX, gyroY, gyroZ;
};

struct ImuFiltered {
    float accX, accY, accZ;
    float gyroX, gyroY, gyroZ;
};

// ==================== TYPES PID ====================
struct PidParameters {
    double kp;
    double ki;
    double kd;
    float targetAngle;  // TA
};

struct PidState {
    float error;
    float errorSum;
    float prevAngle;
    float P, I, D;
};

// ==================== TYPES JOYSTICK ====================
struct JoystickInput {
    int x;           // -100 à +100
    int y;           // -100 à +100
    bool isActive;
};

// ==================== TYPES MOTEURS ====================
struct MotorCommand {
    float speedLeft;
    float speedRight;
};

// ==================== TYPES FILTRAGE ====================
enum FilterType {
    FILTER_MOYENNE,
    FILTER_EMA,
    FILTER_COMPLEMENTAIRE,
    FILTER_KALMAN
};

struct FilterConfig {
    FilterType type;
    int intervalMs;
    int intervalUs;
};

// ==================== ÉTAT PRINCIPAL DU ROBOT ====================
/**
 * @struct RobotState
 * @brief Conteneur unique pour tout l'état du système
 * 
 * PRINCIPE: Cette structure est passée par référence aux modules
 * qui en ont besoin, remplaçant les variables extern.
 * 
 * Exemple d'accès:
 * @code
 * RobotState state;
 * float angle = state.imu.filtered.accX;
 * state.motors.current = {100.0f, -100.0f};
 * @endcode
 */
struct RobotState {
    // ===== CAPTEUR IMU =====
    struct {
        ImuRaw raw;
        ImuFiltered filtered;
        float accAngle;     // Angle calculé depuis accel
        float gyroRate;     // Taux de rotation
        float gyroAngle;    // Angle intégré du gyro
        float currentAngle; // Angle filtré final
    } imu;

    // ===== CORRECTEUR PID =====
    struct {
        PidParameters params;  // KP, KI, KD, TA
        PidState state;        // Error, Sum, P/I/D
        float motorOutput;     // Sortie PID en vitesse moteur
    } pid;

    // ===== JOYSTICK =====
    struct {
        JoystickInput current;
        JoystickInput previous;
    } joystick;

    // ===== MOTEURS =====
    struct {
        MotorCommand current;
        MotorCommand previous;
    } motors;

    // ===== CONFIGURATION FILTRAGE =====
    FilterConfig filter;

    // ===== TEMPS =====
    struct {
        unsigned long currentTimeUs;
        unsigned long previousUpdateUs;
        float deltaTimeS;  // Delta time en secondes
    } timing;

    // ===== INDICATEURS SYSTÈME =====
    struct {
        bool isInitialized;
        bool isConnectedWiFi;
        bool isSDCardReady;
        uint16_t loopCount;
        uint16_t errorCount;
    } system;

    // ===== TÉLÉMÉTRIE =====
    struct {
        float cpuUsagePercent;
        uint32_t freeHeap;
        float loopFrequencyHz;
    } telemetry;
};

// ==================== FONCTIONS UTILITAIRES =====
/**
 * @brief Initialise l'état du robot avec valeurs par défaut
 */
void initializeRobotState(RobotState& state);

/**
 * @brief Réinitialise les valeurs de filtrage
 */
void resetFilteringState(RobotState& state);

/**
 * @brief Valide l'état du robot (vérifications de cohérence)
 */
bool validateRobotState(const RobotState& state);

#endif // ROBOT_STATE_H

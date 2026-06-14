#ifndef ROBOT_SYSTEM_H
#define ROBOT_SYSTEM_H

#include "RobotState.h"
#include "BalanceController.h"
#include "motorDriver.h"
#include "ImuFilter.h"
#include <MPU6050.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

/**
 * @file RobotSystem.h
 * @brief Orchestrateur principal du robot - la classe qui gouverne tout
 * 
 * RÔLE:
 * - Initialisation complète du système
 * - Boucle principale (update)
 * - Interface pour les modules externes (Web, SD card, etc.)
 * - Gestion d'état globale
 * 
 * PATTERN UTILISÉ: Façade + Singleton
 * Une seule instance du robot en global, interface simple pour main.cpp
 * 
 * AVANT (main.cpp chaotique):
 * @code
 * void setup() {
 *     initSDCard();
 *     initWiFi();
 *     initMPU6050();
 *     // ... 20 lignes d'initialisation
 * }
 * void loop() {
 *     // ... 100 lignes de logique
 * }
 * @endcode
 * 
 * APRÈS (main.cpp propre):
 * @code
 * RobotSystem robot;
 * void setup() { robot.initialize(); }
 * void loop() { robot.update(); }
 * @endcode
 */
class RobotSystem {
public:
    // ===== LIFECYCLE =====
    
    /**
     * @brief Initialise tous les sous-systèmes du robot
     * @details Appelé une seule fois dans setup()
     * - Initialisation série
     * - Carte SD
     * - WiFi
     * - MPU6050
     * - Serveur web
     * @return true si succès, false si erreur critique
     */
    bool initialize();

    /**
     * @brief Boucle principale - appelée chaque cycle
     * @details Ré-entrant et non-bloquant
     * @return false si erreur critique
     */
    bool update();

    /**
     * @brief Arrête proprement le robot
     */
    void shutdown();

    // ===== ACCÈS À L'ÉTAT =====

    /**
     * @brief Accès en lecture à l'état actuel du robot
     */
    const RobotState& getState() const { return state_; }

    /**
     * @brief Accès en écriture (pour les modules externes)
     */
    RobotState& getMutableState() { return state_; }

    // ===== CONTRÔLE JOYSTICK =====

    /**
     * @brief Définir les valeurs du joystick depuis l'interface web
     * @param x Valeur X (-100 à +100)
     * @param y Valeur Y (-100 à +100)
     */
    void setJoystickInput(int x, int y);

    // ===== CONTRÔLE PID =====

    /**
     * @brief Mettre à jour les paramètres PID à chaud
     * @param kp Constante proportionnelle
     * @param ki Constante intégrale
     * @param kd Constante dérivée
     * @param targetAngle Angle cible (en degrés)
     */
    void setPidParameters(double kp, double ki, double kd, float targetAngle);

    /**
     * @brief Récupérer les paramètres PID actuels
     */
    PidParameters getPidParameters() const { return state_.pid.params; }

    // ===== CONTRÔLE FILTRAGE =====

    /**
     * @brief Changer le type de filtre
     * @param filterType Type de filtre (MOYENNE, EMA, COMPLEMENTAIRE, KALMAN)
     */
    void setFilterType(FilterType filterType);

    /**
     * @brief Définir l'intervalle de filtrage
     * @param intervalMs Intervalle en millisecondes
     */
    void setFilterInterval(int intervalMs);

    // ===== DIAGNOSTIQUES =====

    /**
     * @brief Obtenir des infos de diagnostic
     * @return String JSON avec infos système
     */
    String getDiagnostics() const;

    /**
     * @brief Vérifier la santé du système
     */
    bool isHealthy() const;

    /**
     * @brief Réinitialiser les compteurs d'erreur
     */
    void resetErrorCounters();

    // ===== UTILITAIRES =====

    /**
     * @brief Obtenir la fréquence de la boucle principale
     */
    float getLoopFrequency() const { return state_.telemetry.loopFrequencyHz; }

    /**
     * @brief Forcer une sauvegarde de configuration
     */
    void saveConfiguration();

    /**
     * @brief Recharger la configuration depuis SD
     */
    void reloadConfiguration();

private:
    // ===== ÉTAT CENTRAL =====
    RobotState state_;

    // ===== COMPOSANTS =====
    MPU6050 mpu_;
    ImuFilter imuFilter_;
    BalanceController balanceController_;
    MotorDriver motors_;

    // ===== TIMING ET DIAGNOSTIC =====
    unsigned long lastUpdateTime_;
    uint32_t updateCount_;
    float averageLoopTimeMs_;

    // ===== MÉTHODES PRIVÉES D'INITIALISATION =====
    void initializeSerial_();
    bool initializeSDCard_();
    bool initializeWiFi_();
    bool initializeMPU6050_();
    void initializeWebServer_();
    void initializePinout_();

    // ===== MÉTHODES PRIVÉES DE MISE À JOUR =====
    void updateIMU_();
    void updatePID_();
    void updateMotors_();
    void updateTelemetry_();
    void handleWebRequests_();

    // ===== MÉTHODES PRIVÉES DE CALIBRATION =====
    void calibrateMPU6050_();
    void setMPU6050Offsets_();

    // ===== HANDLERS WEB =====
    void setupWebHandlers_();
    void handlePidParametersUpdate_(AsyncWebServerRequest* request);
    void handleJoystickUpdate_(AsyncWebServerRequest* request);
    void handleFilterUpdate_(AsyncWebServerRequest* request);

    // ===== UTILITAIRES PRIVÉS =====
    void logStatus_();
    void checkSystemHealth_();
};

#endif // ROBOT_SYSTEM_H

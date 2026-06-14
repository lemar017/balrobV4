#ifndef TIMING_CONTROLLER_H
#define TIMING_CONTROLLER_H

#include <Arduino.h>

/**
 * @file TimingController.h
 * @brief Gestion centralisée des timers et du timing
 *
 * PROBLÈME RÉSOLU:
 * Avant: Variables globales éparpillées
 * @code
 * unsigned long dernierPID = 0;
 * unsigned long dernierLED = 0;
 * unsigned long dernierAffichage = 0;
 * const unsigned long INTERVALLE_PID = 10000;
 * const unsigned long INTERVALLE_AFFICHAGE = 20000;
 * const unsigned long INTERVALLE_LED = 500000;
 * @endcode
 *
 * Après: Classe cohésive
 * @code
 * TimingController timing;
 * if (timing.shouldExecutePID()) { // ... }
 * @endcode
 */
class TimingController
{
public:
        /**
         * @brief Initialiser le contrôleur de timing
         * @param pidIntervalUs   Intervalle PID en microsecondes (défaut: 10000 = 10ms)
         * @param displayIntervalUs Intervalle affichage (défaut: 20000 = 20ms)
         * @param ledIntervalUs   Intervalle LED (défaut: 500000 = 500ms)
         */
        TimingController(
            unsigned long pidIntervalUs = 10000,
            unsigned long displayIntervalUs = 20000,
            unsigned long ledIntervalUs = 500000);

        /**
         * @brief Mettre à jour les timers (à appeler une fois par loop)
         */
        void update();

        // ===== VÉRIFICATIONS DE TEMPS =====

        /**
         * @brief Vérifier si le PID doit s'exécuter
         * @return true si l'intervalle PID est écoulé
         */
        bool shouldExecutePID();

        /**
         * @brief Vérifier si les données doivent être affichées
         * @return true si l'intervalle d'affichage est écoulé
         */
        bool shouldDisplayData();

        /**
         * @brief Vérifier si la LED doit être mise à jour
         * @return true si l'intervalle LED est écoulé
         */
        bool shouldToggleLED();

        /**
         * @brief Vérifier si on doit mettre à jour la télémétrie
         * @return true si assez de temps s'est écoulé
         */
        bool shouldUpdateTelemetry();

        // ===== DELTA TIME =====

        /**
         * @brief Obtenir le delta time depuis la dernière mise à jour (en secondes)
         * @return float Delta time en secondes
         */
        float getDeltaTimeSeconds();

        /**
         * @brief Obtenir le delta time en millisecondes
         */
        float getDeltaTimeMillis();

        /**
         * @brief Obtenir le delta time en microsecondes
         */
        unsigned long getDeltaTimeMicros();

        // ===== CONFIGURATION =====

        /**
         * @brief Changer l'intervalle d'exécution PID
         */
        void setPIDInterval(unsigned long intervalUs) { pidIntervalUs_ = intervalUs; }

        /**
         * @brief Changer l'intervalle d'affichage
         */
        void setDisplayInterval(unsigned long intervalUs) { displayIntervalUs_ = intervalUs; }

        /**
         * @brief Changer l'intervalle LED
         */
        void setLEDInterval(unsigned long intervalUs) { ledIntervalUs_ = intervalUs; }

        // ===== DIAGNOSTIQUES =====

        /**
         * @brief Obtenir la fréquence de boucle actuelle (Hz)
         */
        float getLoopFrequencyHz() const { return loopFrequencyHz_; }

        /**
         * @brief Obtenir le temps moyen d'une itération (ms)
         */
        float getAverageLoopTimeMs() const { return averageLoopTimeMs_; }

        /**
         * @brief Obtenir le nombre total d'itérations
         */
        uint32_t getIterationCount() const { return iterationCount_; }

private:
        // ===== TIMERS =====
        unsigned long lastPIDTime_;
        unsigned long lastDisplayTime_;
        unsigned long lastLEDTime_;
        unsigned long lastTelemetryTime_;
        unsigned long lastLoopTime_;
        unsigned long startTime_;

        // ===== INTERVALLES (en microsecondes) =====
        unsigned long pidIntervalUs_;
        unsigned long displayIntervalUs_;
        unsigned long ledIntervalUs_;
        unsigned long telemetryIntervalUs_;

        // ===== STATISTIQUES =====
        uint32_t iterationCount_;
        float loopFrequencyHz_;
        float averageLoopTimeMs_;
        unsigned long accumulatedLoopTime_;

        // ===== DELTA TIME =====
        unsigned long currentTime_;
        unsigned long deltaTimeMicros_;

        /**
         * @brief Mettre à jour les statistiques de fréquence
         */
        void updateLoopStatistics_();
};

#endif // TIMING_CONTROLLER_H

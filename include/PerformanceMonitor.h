#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include "RobotState.h"
#include <Arduino.h>

/**
 * @file PerformanceMonitor.h
 * @brief Télémétrie et affichage unifié du système
 *
 * PROBLÈME RÉSOLU:
 * Avant: Multiples fonctions printData*(), code dupliqué
 * @code
 * void printData() { // ... }
 * void printDataOnTimer() { // ... }
 * void printDataBinaire() { // ... }
 * void printDataFiltree() { // ... }
 * void efficaciteFiltre() { // ... }
 * void printConsignesPID() { // ... }
 * void printValeursPID() { // ... }
 * @endcode
 *
 * Après: Une seule classe cohésive
 * @code
 * PerformanceMonitor monitor(OutputFormat::CSV);
 * monitor.logData(state);
 * @endcode
 */
class PerformanceMonitor
{
public:
        /**
         * @brief Formats de sortie disponibles
         */
        enum class OutputFormat
        {
                RAW,          // Données brutes du MPU6050
                FILTERED,     // Données après filtrage
                PID_ONLY,     // Valeurs PID uniquement
                JOYSTICK,     // État du joystick
                MOTOR_CMD,    // Commandes moteurs
                CSV,          // Format CSV complet (tous les capteurs)
                CSV_EXTENDED, // CSV + PID + moteurs
                BINARY,       // Format binaire (compact)
                DEBUG,        // Format lisible pour debug
                TELEMETRY     // Infos système (CPU, heap, etc.)
        };

        /**
         * @brief Initialiser le moniteur
         * @param format Format de sortie initial
         * @param autoFlush true pour forcer un flush après chaque log
         */
        PerformanceMonitor(OutputFormat format = OutputFormat::CSV, bool autoFlush = true);

        /**
         * @brief Changer le format de sortie
         */
        void setOutputFormat(OutputFormat format) { currentFormat_ = format; }

        /**
         * @brief Obtenir le format actuel
         */
        OutputFormat getOutputFormat() const { return currentFormat_; }

        // ===== LOGGING PRINCIPAL =====

        /**
         * @brief Logger l'état complet du robot
         * @param state État du robot à logger
         * @details Utilise le format configuré
         */
        void logData(const RobotState &state);

        /**
         * @brief Logger uniquement les données IMU
         */
        void logIMUData(const RobotState &state);

        /**
         * @brief Logger uniquement les données PID
         */
        void logPIDData(const RobotState &state);

        /**
         * @brief Logger uniquement les commandes moteurs
         */
        void logMotorData(const RobotState &state);

        /**
         * @brief Logger les infos de système
         */
        void logSystemTelemetry(const RobotState &state);

        // ===== LOGGING FORMATÉ =====

        /**
         * @brief Logger un message avec timestamp
         */
        void logMessage(const String &message, bool includeTimestamp = true);

        /**
         * @brief Logger une valeur étiquetée
         */
        void logValue(const String &label, float value, int precision = 2);

        /**
         * @brief Logger une erreur
         */
        void logError(const String &error);

        /**
         * @brief Logger un avertissement
         */
        void logWarning(const String &warning);

        // ===== CONTRÔLE D'AFFICHAGE =====

        /**
         * @brief Activer/désactiver l'affichage
         */
        void setEnabled(bool enabled) { enabled_ = enabled; }

        /**
         * @brief Vérifier si l'affichage est actif
         */
        bool isEnabled() const { return enabled_; }

        /**
         * @brief Définir le niveau de verbosité (0-3)
         * @param level 0=silence, 1=erreurs, 2=normal, 3=verbose
         */
        void setVerbosity(int level) { verbosity_ = level; }

        /**
         * @brief Vider le buffer série
         */
        void flush() { Serial.flush(); }

        // ===== DIAGNOSTIQUES =====

        /**
         * @brief Obtenir les statistiques de logging
         */
        String getStatisticsJSON() const;

        /**
         * @brief Réinitialiser les compteurs
         */
        void resetCounters();

        /**
         * @brief Obtenir le nombre d'items loggés
         */
        uint32_t getLogCount() const { return logCount_; }

private:
        OutputFormat currentFormat_;
        bool enabled_;
        int verbosity_;
        uint32_t logCount_;
        uint32_t errorCount_;

        // ===== STATISTIQUES =====
        unsigned long lastStatisticsTime_;
        float averageLogsPerSecond_;

        // ===== MÉTHODES DE FORMATAGE PRIVÉES =====

        /**
         * @brief Logger au format brut
         */
        void logRaw_(const RobotState &state);

        /**
         * @brief Logger au format filtré
         */
        void logFiltered_(const RobotState &state);

        /**
         * @brief Logger au format PID uniquement
         */
        void logPIDOnly_(const RobotState &state);

        /**
         * @brief Logger au format joystick
         */
        void logJoystick_(const RobotState &state);

        /**
         * @brief Logger au format commandes moteurs
         */
        void logMotorCommands_(const RobotState &state);

        /**
         * @brief Logger au format CSV
         */
        void logCSV_(const RobotState &state);

        /**
         * @brief Logger au format CSV étendu
         */
        void logCSVExtended_(const RobotState &state);

        /**
         * @brief Logger au format binaire
         */
        void logBinary_(const RobotState &state);

        /**
         * @brief Logger au format debug lisible
         */
        void logDebug_(const RobotState &state);

        /**
         * @brief Logger au format télémétrie
         */
        void logTelemetry_(const RobotState &state);

        // ===== UTILITAIRES =====

        /**
         * @brief Obtenir un timestamp formaté
         */
        String getTimestamp_() const;

        /**
         * @brief Vérifier si on doit logger selon la verbosité
         */
        bool shouldLog_(int minimumVerbosity) const { return verbosity_ >= minimumVerbosity; }

        /**
         * @brief Incrémenter les statistiques
         */
        void updateStatistics_();
};

#endif // PERFORMANCE_MONITOR_H

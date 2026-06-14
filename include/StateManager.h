#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "RobotState.h"
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

/**
 * @file StateManager.h
 * @brief Gestion de la configuration persistante (SD card + JSON)
 * 
 * RESPONSABILITÉS:
 * - Charger/sauvegarder la configuration depuis/vers SD
 * - Gérer les fichiers JSON (pid.json, wifi.json, filter.json)
 * - Valider les paramètres
 * - Interface web pour modifications temps réel
 * 
 * FICHIERS SD ATTENDUS:
 * /balrob/
 *   ├── pid.json           (KP, KI, KD, TA)
 *   ├── wifi.json          (SSID, password)
 *   └── filter.json        (type filtre, intervalle)
 */
class StateManager {
public:
    /**
     * @brief Initialiser le gestionnaire d'état
     * @param sdCardMounted true si la carte SD est déjà initialisée
     */
    bool initialize(bool sdCardMounted = false);

    // ===== CHARGEMENT DEPUIS SD =====

    /**
     * @brief Charger TOUS les paramètres depuis SD
     * @param state État du robot à remplir
     * @return true si succès
     */
    bool loadAllConfiguration(RobotState& state);

    /**
     * @brief Charger les paramètres PID depuis pid.json
     */
    bool loadPIDConfiguration(RobotState& state);

    /**
     * @brief Charger la configuration WiFi depuis wifi.json
     */
    bool loadWiFiConfiguration(String& ssid, String& password);

    /**
     * @brief Charger la configuration du filtre depuis filter.json
     */
    bool loadFilterConfiguration(RobotState& state);

    // ===== SAUVEGARDE VERS SD =====

    /**
     * @brief Sauvegarder TOUS les paramètres vers SD
     * @param state État du robot à sauvegarder
     * @return true si succès
     */
    bool saveAllConfiguration(const RobotState& state);

    /**
     * @brief Sauvegarder les paramètres PID vers pid.json
     */
    bool savePIDConfiguration(const RobotState& state);

    /**
     * @brief Sauvegarder la configuration WiFi vers wifi.json
     */
    bool saveWiFiConfiguration(const String& ssid, const String& password);

    /**
     * @brief Sauvegarder la configuration du filtre vers filter.json
     */
    bool saveFilterConfiguration(const RobotState& state);

    // ===== VALIDATION =====

    /**
     * @brief Valider les paramètres PID
     * @return true si les valeurs sont acceptables
     */
    bool validatePIDParameters(const PidParameters& params) const;

    /**
     * @brief Valider les paramètres du filtre
     */
    bool validateFilterConfiguration(const FilterConfig& filter) const;

    /**
     * @brief Obtenir les limites de paramètres
     */
    struct ParameterLimits {
        double kpMin = 0.0, kpMax = 100.0;
        double kiMin = 0.0, kiMax = 50.0;
        double kdMin = 0.0, kdMax = 50.0;
        float taMin = -45.0f, taMax = 45.0f;
    } const limits;

    // ===== INTERFACE WEB =====

    /**
     * @brief Configurer les handlers web pour les mise à jour
     * @param server Pointeur vers le serveur AsyncWebServer
     */
    void setupWebHandlers(AsyncWebServer* server);

    /**
     * @brief Récupérer la configuration actuelle en JSON
     * @return String JSON avec la configuration
     */
    String getCurrentConfigurationJSON(const RobotState& state) const;

    /**
     * @brief Récupérer l'historique des erreurs
     * @return String JSON avec les erreurs
     */
    String getErrorHistoryJSON() const;

    // ===== UTILITAIRES =====

    /**
     * @brief Effectuer une sauvegarde en arrière-plan
     * @note Non-bloquant
     */
    void scheduleBackgroundSave(const RobotState& state);

    /**
     * @brief Vérifier si la SD card est disponible
     */
    bool isSDCardAvailable() const { return sdCardReady_; }

    /**
     * @brief Réinitialiser la configuration aux valeurs par défaut
     */
    bool resetToDefaults(RobotState& state);

    /**
     * @brief Afficher la structure des fichiers de configuration
     */
    void printConfigurationStructure();

private:
    bool sdCardReady_;
    unsigned long lastSaveTime_;
    uint32_t errorCount_;

    // ===== CHEMINS FICHIERS =====
    static constexpr const char* CONFIG_DIR = "/balrob";
    static constexpr const char* PID_CONFIG_FILE = "/balrob/pid.json";
    static constexpr const char* WIFI_CONFIG_FILE = "/balrob/wifi.json";
    static constexpr const char* FILTER_CONFIG_FILE = "/balrob/filter.json";

    // ===== PARSING JSON =====
    bool parseJSONFile_(const char* filepath, JsonDocument& doc);
    bool writeJSONFile_(const char* filepath, const JsonDocument& doc);

    // ===== HANDLERS WEB PRIVÉS =====
    void handlePIDUpdate_(AsyncWebServerRequest* request);
    void handleWiFiUpdate_(AsyncWebServerRequest* request);
    void handleFilterUpdate_(AsyncWebServerRequest* request);
    void handleConfigurationGet_(AsyncWebServerRequest* request);

    // ===== CONFIGURATION PAR DÉFAUT =====
    void setDefaultPIDParameters_(RobotState& state);
    void setDefaultFilterConfiguration_(RobotState& state);

    // ===== UTILITAIRES PRIVÉS =====
    bool ensureConfigDirectory_();
    bool createBackupFile_(const char* originalFile);
    void recordError_(const String& errorMessage);
};

#endif // STATE_MANAGER_H

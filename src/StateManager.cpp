#include "StateManager.h"
#include "FS.h"
#include "SD.h"

/**
 * @file StateManager.cpp
 * @brief Implémentation de la gestion de la configuration et de l'état persistant
 */

bool StateManager::initialize(bool sdCardMounted)
{
        Serial.println("Initialisation StateManager...");
        sdCardReady_ = sdCardMounted;
        lastSaveTime_ = millis();
        errorCount_ = 0;

        if (sdCardReady_)
        {
                ensureConfigDirectory_();
                Serial.println("StateManager initialisé avec SD Card");
        }
        else
        {
                Serial.println("StateManager initialisé (SD Card non disponible)");
        }

        return true;
}

// ===== CHARGEMENT DEPUIS SD =====
bool StateManager::loadAllConfiguration(RobotState &state)
{
        Serial.println("Chargement de TOUTE la configuration...");

        bool pidOk = loadPIDConfiguration(state);
        bool filterOk = loadFilterConfiguration(state);

        if (!pidOk || !filterOk)
        {
                Serial.println("Avertissement: Certains fichiers config manquent");
                return false;
        }

        return true;
}

bool StateManager::loadPIDConfiguration(RobotState &state)
{
        Serial.println("Chargement pid.json...");

        if (!sdCardReady_)
        {
                Serial.println("  → SD Card non disponible, utilisation des valeurs par défaut");
                setDefaultPIDParameters_(state);
                return false;
        }

        JsonDocument doc;
        if (!parseJSONFile_(PID_CONFIG_FILE, doc))
        {
                Serial.println("  → Fichier non trouvé, valeurs par défaut");
                setDefaultPIDParameters_(state);
                return false;
        }

        // Parser les valeurs
        state.pid.params.kp = doc["kp"] | 0.1;
        state.pid.params.ki = doc["ki"] | 0.05;
        state.pid.params.kd = doc["kd"] | 0.05;
        state.pid.params.targetAngle = doc["targetAngle"] | 1.5f;

        Serial.printf("  ✓ PID chargé: KP=%.2f, KI=%.2f, KD=%.2f, TA=%.2f\n",
                      state.pid.params.kp, state.pid.params.ki,
                      state.pid.params.kd, state.pid.params.targetAngle);

        return true;
}

bool StateManager::loadWiFiConfiguration(String &ssid, String &password)
{
        Serial.println("Chargement wifi.json...");

        if (!sdCardReady_)
        {
                Serial.println("  → SD Card non disponible");
                return false;
        }

        JsonDocument doc;
        if (!parseJSONFile_(WIFI_CONFIG_FILE, doc))
        {
                Serial.println("  → Fichier non trouvé");
                return false;
        }

        ssid = doc["ssid"].as<String>();
        password = doc["password"].as<String>();

        Serial.printf("  ✓ WiFi chargé: SSID=%s\n", ssid.c_str());
        return true;
}

bool StateManager::loadFilterConfiguration(RobotState &state)
{
        Serial.println("Chargement filter.json...");

        if (!sdCardReady_)
        {
                Serial.println("  → SD Card non disponible, valeurs par défaut");
                setDefaultFilterConfiguration_(state);
                return false;
        }

        JsonDocument doc;
        if (!parseJSONFile_(FILTER_CONFIG_FILE, doc))
        {
                Serial.println("  → Fichier non trouvé, valeurs par défaut");
                setDefaultFilterConfiguration_(state);
                return false;
        }

        state.filter.type = static_cast<FilterType>(doc["type"] | FILTER_EMA);
        state.filter.intervalMs = doc["intervalMs"] | 20;

        Serial.printf("  ✓ Filtre chargé: Type=%d, Intervalle=%d ms\n",
                      state.filter.type, state.filter.intervalMs);

        return true;
}

// ===== SAUVEGARDE VERS SD =====
bool StateManager::saveAllConfiguration(const RobotState &state)
{
        Serial.println("Sauvegarde de TOUTE la configuration...");

        bool pidOk = savePIDConfiguration(state);
        bool filterOk = saveFilterConfiguration(state);

        if (pidOk && filterOk)
        {
                Serial.println("  ✓ Configuration sauvegardée avec succès");
                return true;
        }
        else
        {
                Serial.println("  ✗ Erreur lors de la sauvegarde");
                return false;
        }
}

bool StateManager::savePIDConfiguration(const RobotState &state)
{
        Serial.println("Sauvegarde pid.json...");

        if (!sdCardReady_)
        {
                Serial.println("  → SD Card non disponible");
                return false;
        }

        JsonDocument doc;
        doc["kp"] = state.pid.params.kp;
        doc["ki"] = state.pid.params.ki;
        doc["kd"] = state.pid.params.kd;
        doc["targetAngle"] = state.pid.params.targetAngle;

        return writeJSONFile_(PID_CONFIG_FILE, doc);
}

bool StateManager::saveWiFiConfiguration(const String &ssid, const String &password)
{
        Serial.println("Sauvegarde wifi.json...");

        if (!sdCardReady_)
        {
                Serial.println("  → SD Card non disponible");
                return false;
        }

        JsonDocument doc;
        doc["ssid"] = ssid;
        doc["password"] = password;

        return writeJSONFile_(WIFI_CONFIG_FILE, doc);
}

bool StateManager::saveFilterConfiguration(const RobotState &state)
{
        Serial.println("Sauvegarde filter.json...");

        if (!sdCardReady_)
        {
                Serial.println("  → SD Card non disponible");
                return false;
        }

        JsonDocument doc;
        doc["type"] = static_cast<int>(state.filter.type);
        doc["intervalMs"] = state.filter.intervalMs;

        return writeJSONFile_(FILTER_CONFIG_FILE, doc);
}

// ===== VALIDATION =====
bool StateManager::validatePIDParameters(const PidParameters &params) const
{
        if (params.kp < limits.kpMin || params.kp > limits.kpMax)
                return false;
        if (params.ki < limits.kiMin || params.ki > limits.kiMax)
                return false;
        if (params.kd < limits.kdMin || params.kd > limits.kdMax)
                return false;
        if (params.targetAngle < limits.taMin || params.targetAngle > limits.taMax)
                return false;
        return true;
}

bool StateManager::validateFilterConfiguration(const FilterConfig &filter) const
{
        if (filter.intervalMs < 1 || filter.intervalMs > 1000)
                return false;
        return true;
}

// ===== UTILITAIRES PRIVÉS =====
bool StateManager::parseJSONFile_(const char *filepath, JsonDocument &doc)
{
        File file = SD.open(filepath, FILE_READ);
        if (!file)
        {
                return false;
        }

        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error)
        {
                Serial.printf("  Erreur JSON: %s\n", error.c_str());
                return false;
        }

        return true;
}

bool StateManager::writeJSONFile_(const char *filepath, const JsonDocument &doc)
{
        // Créer une sauvegarde
        createBackupFile_(filepath);

        File file = SD.open(filepath, FILE_WRITE);
        if (!file)
        {
                Serial.printf("  Impossible d'ouvrir %s\n", filepath);
                return false;
        }

        serializeJson(doc, file);
        file.close();

        Serial.printf("  ✓ %s écrit avec succès\n", filepath);
        return true;
}

bool StateManager::ensureConfigDirectory_()
{
        if (!SD.exists(CONFIG_DIR))
        {
                SD.mkdir(CONFIG_DIR);
                Serial.printf("Répertoire créé: %s\n", CONFIG_DIR);
        }
        return true;
}

bool StateManager::createBackupFile_(const char *originalFile)
{
        // Implémenter si nécessaire
        return true;
}

void StateManager::recordError_(const String &errorMessage)
{
        errorCount_++;
        Serial.printf("ERROR[%d]: %s\n", errorCount_, errorMessage.c_str());
}

void StateManager::setDefaultPIDParameters_(RobotState &state)
{
        state.pid.params = {0.1, 0.05, 0.05, 1.5f};
}

void StateManager::setDefaultFilterConfiguration_(RobotState &state)
{
        state.filter = {FILTER_EMA, 20, 20000};
}

// ===== DIAGNOSTIQUES =====
String StateManager::getCurrentConfigurationJSON(const RobotState &state) const
{
        StaticJsonDocument<512> doc;
        doc["pid"]["kp"] = state.pid.params.kp;
        doc["pid"]["ki"] = state.pid.params.ki;
        doc["pid"]["kd"] = state.pid.params.kd;
        doc["pid"]["targetAngle"] = state.pid.params.targetAngle;
        doc["filter"]["type"] = static_cast<int>(state.filter.type);
        doc["filter"]["intervalMs"] = state.filter.intervalMs;

        String output;
        serializeJson(doc, output);
        return output;
}

String StateManager::getErrorHistoryJSON() const
{
        StaticJsonDocument<256> doc;
        doc["errorCount"] = errorCount_;
        doc["sdCardReady"] = sdCardReady_;

        String output;
        serializeJson(doc, output);
        return output;
}

void StateManager::printConfigurationStructure()
{
        Serial.println("\n=== Structure des fichiers de configuration ===");
        Serial.println("SD:/balrob/");
        Serial.println("  ├── pid.json");
        Serial.println("  │   ├── kp (double)");
        Serial.println("  │   ├── ki (double)");
        Serial.println("  │   ├── kd (double)");
        Serial.println("  │   └── targetAngle (float)");
        Serial.println("  ├── wifi.json");
        Serial.println("  │   ├── ssid (string)");
        Serial.println("  │   └── password (string)");
        Serial.println("  └── filter.json");
        Serial.println("      ├── type (int)");
        Serial.println("      └── intervalMs (int)");
        Serial.println();
}

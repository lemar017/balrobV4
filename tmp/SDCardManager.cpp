#include "SDCardManager.h"

SDCardManager::SDCardManager(uint8_t csPin) : _csPin(csPin), _initialized(false) {}

bool SDCardManager::begin() {
    if (!SD.begin(_csPin)) {
        Serial.println("[SD] Erreur : échec du montage");
        return false;
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[SD] Aucune carte détectée");
        return false;
    }
    Serial.print("[SD] Type de carte : ");
    if (cardType == CARD_MMC) Serial.println("MMC");
    else if (cardType == CARD_SD) Serial.println("SDSC");
    else if (cardType == CARD_SDHC) Serial.println("SDHC");
    else Serial.println("Inconnu");

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("[SD] Taille : %llu MB\n", cardSize);

    _initialized = true;
    return true;
}

bool SDCardManager::_checkInit() {
    if (!_initialized) {
        Serial.println("[SD] Erreur : carte non initialisée");
        return false;
    }
    return true;
}

String SDCardManager::readFile(const char* path) {
    if (!_checkInit()) return "";
    File file = SD.open(path, FILE_READ);
    if (!file) {
        Serial.printf("[SD] Erreur : impossible d’ouvrir %s\n", path);
        return "";
    }
    String content;
    while (file.available()) content += (char)file.read();
    file.close();
    Serial.printf("[SD] Lecture terminée : %s\n", path);
    return content;
}

bool SDCardManager::_createParentDirectories(const char* filepath) {
    // Extrait le chemin dossier et crée tous les dossiers parents nécessaires
    String path(filepath);
    int lastSlash = path.lastIndexOf('/');
    if (lastSlash <= 0) return true; // Pas de dossier parent, fichier à la racine

    String dirPath = path.substring(0, lastSlash);
    if (SD.exists(dirPath.c_str())) return true; // Dossier existe déjà

    // Création récursive : on crée d’abord le parent du parent
    if (!_createParentDirectories(dirPath.c_str())) return false;

    if (SD.mkdir(dirPath.c_str())) {
        Serial.printf("[SD] Dossier créé : %s\n", dirPath.c_str());
        return true;
    } else {
        Serial.printf("[SD] Échec création dossier : %s\n", dirPath.c_str());
        return false;
    }
}

bool SDCardManager::writeFile(const char* path, const String& data) {
    if (!_checkInit()) return false;

    // Création des dossiers parents si besoin
    if (!_createParentDirectories(path)) {
        Serial.println("[SD] Erreur : impossible de créer les dossiers parents");
        return false;
    }

    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        Serial.printf("[SD] Erreur : impossible d’écrire %s\n", path);
        return false;
    }

    file.print(data);
    file.close();
    Serial.printf("[SD] Écriture terminée : %s\n", path);
    return true;
}

bool SDCardManager::deleteFile(const char* path) {
    if (!_checkInit()) return false;

    if (SD.exists(path)) {
        if (SD.remove(path)) {
            Serial.printf("[SD] Fichier supprimé : %s\n", path);
            return true;
        } else {
            Serial.printf("[SD] Erreur : suppression impossible %s\n", path);
            return false;
        }
    } else {
        Serial.printf("[SD] Fichier inexistant : %s\n", path);
        return false;
    }
}

bool SDCardManager::createDirectory(const char* path) {
    if (!_checkInit()) return false;

    if (SD.exists(path)) {
        Serial.printf("[SD] Dossier déjà existant : %s\n", path);
        return true;
    }

    // Création récursive des parents
    if (!_createParentDirectories(path)) {
        Serial.println("[SD] Erreur : impossible de créer les dossiers parents");
        return false;
    }

    if (SD.mkdir(path)) {
        Serial.printf("[SD] Dossier créé : %s\n", path);
        return true;
    } else {
        Serial.printf("[SD] Échec création dossier : %s\n", path);
        return false;
    }
}

bool SDCardManager::deleteDirectory(const char* path, bool recursive) {
    if (!_checkInit()) return false;

    if (!SD.exists(path)) {
        Serial.printf("[SD] Dossier inexistant : %s\n", path);
        return false;
    }

    if (!recursive) {
        Serial.printf("[SD] Suppression dossier non supportée (Arduino SD ne supprime pas les dossiers)\n");
        return false;
    } else {
        return _deleteDirectoryRecursive(path);
    }
}

bool SDCardManager::_deleteDirectoryRecursive(const char* path) {
    File dir = SD.open(path);
    if (!dir) {
        Serial.printf("[SD] Erreur : impossible d’ouvrir dossier %s\n", path);
        return false;
    }
    if (!dir.isDirectory()) {
        dir.close();
        Serial.printf("[SD] Ce n’est pas un dossier : %s\n", path);
        return false;
    }

    File entry;
    while ((entry = dir.openNextFile())) {
        String entryName = String(path) + "/" + entry.name();
        if (entry.isDirectory()) {
            entry.close();
            if (!_deleteDirectoryRecursive(entryName.c_str())) {
                dir.close();
                return false;
            }
        } else {
            if (!SD.remove(entryName.c_str())) {
                Serial.printf("[SD] Impossible de supprimer fichier : %s\n", entryName.c_str());
                entry.close();
                dir.close();
                return false;
            }
            entry.close();
        }
    }
    dir.close();

    Serial.printf("[SD] Contenu du dossier supprimé : %s\n", path);
    // Note : la librairie SD Arduino ne supprime pas le dossier lui-même
    return true;
}

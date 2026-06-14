

// ── Variables filtre globales ─────────────────────────────
String filterType     = "MOYENNE";
int    filterInterval = 500;

const char* FILTER_PATH = "/balrob/filter.json";

// ─────────────────────────────────────────────────────────
// Lecture filter.json depuis la SD
// ─────────────────────────────────────────────────────────
bool loadFilterFromSD() {
  if (!SD.exists(FILTER_PATH)) {
    Serial.println("[SD] filter.json introuvable");
    return false;
  }
  File f = SD.open(FILTER_PATH, FILE_READ);
  if (!f) return false;

  StaticJsonDocument<128> doc;
  if (deserializeJson(doc, f) != DeserializationError::Ok) {
    f.close(); return false;
  }
  f.close();

  filterType     = doc["filter"]   | "MOYENNE";
  filterInterval = doc["interval"] | 500;

  Serial.printf("[SD] Filtre: %s · %dms\n",
                filterType.c_str(), filterInterval);
  return true;
}

// ─────────────────────────────────────────────────────────
// Sauvegarde filter.json sur la SD
// ─────────────────────────────────────────────────────────
bool saveFilterToSD(const char* filter, int interval) {
  if (!SD.exists("/balrob")) SD.mkdir("/balrob");
  File f = SD.open(FILTER_PATH, FILE_WRITE);
  if (!f) return false;

  StaticJsonDocument<128> doc;
  doc["filter"]   = filter;
  doc["interval"] = interval;

  serializeJsonPretty(doc, f);
  f.close();
  Serial.println("[SD] filter.json sauvegardé");
  return true;
}

// ─────────────────────────────────────────────────────────
// Application du filtre (à adapter à ton algo)
// ─────────────────────────────────────────────────────────
void applyFilter() {
  if      (filterType == "MOYENNE")   { /* moyenne glissante sur N échantillons */ }
  else if (filterType == "MEDIAN")    { /* filtre médian */                        }
  else if (filterType == "PASSE_BAS") { /* filtre passe-bas (RC numérique) */      }
  // filterInterval → période d'échantillonnage en ms
}
// Sauvegarder config sur SD
void saveWifiConfig() {
    File f = SD.open("/wifi/json/wifi.json", FILE_WRITE);
    if (f) {
        StaticJsonDocument<256> doc;
        doc["mode"] = mode;
        doc["ssid"] = ssid;
        doc["pwd"]  = pwd;
        doc["ip"]   = ip;
        serializeJson(doc, f);
        f.close();
    }
}
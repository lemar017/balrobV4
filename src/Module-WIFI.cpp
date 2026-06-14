#include "Module-WIFI.h"
#include <cstdarg>
#include <cstdio>

WiFiConfig currentConfig;
bool sdCardInitialized = false;
AsyncWebServer server(80);
// WebSocket for live logs
AsyncWebSocket ws("/log");
bool shouldConnectWiFi = false;

// Implementation of functions
void setupAccessPoint()
{
        WiFi.mode(WIFI_AP_STA);

        WiFi.softAP("ESP32-BALROB-V6", "12345678");
        
        Serial.println("Point d'accès créé");
        Serial.print("IP: ");
        Serial.println(WiFi.softAPIP());
}

void setupWebServer()
{
        Serial.println("=== setupWebServer() START ===");
        server.serveStatic("/", SD, "/balrob/").setDefaultFile("index.html");

        // register websocket handler
        server.addHandler(&ws);
        Serial.println("WebSocket handler registered at /log");

        // optional: log websocket connect/disconnect events
        ws.onEvent([](AsyncWebSocket *serverWS, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                   {
                        if (type == WS_EVT_CONNECT) {
                                Serial.print("WS_EVT_CONNECT: client ID ");
                                Serial.println(client->id());
                                String msg = "Client connected: ";
                                msg += client->id();
                                broadcastLog(msg);
                        } else if (type == WS_EVT_DISCONNECT) {
                                Serial.print("WS_EVT_DISCONNECT: client ID ");
                                Serial.println(client->id());
                                String msg = "Client disconnected: ";
                                msg += client->id();
                                broadcastLog(msg);
                        } else if (type == WS_EVT_ERROR) {
                                Serial.print("WS_EVT_ERROR: client ID ");
                                Serial.print(client->id());
                                Serial.print(", error code: ");
                                Serial.println((uint32_t)arg);
                        } else if (type == WS_EVT_PONG) {
                                Serial.print("WS_EVT_PONG: client ID ");
                                Serial.println(client->id());
                        }
                   });

        server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request)
                  { handleWiFiScan(request); });

        server.on("/wificonfig", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                  { handleWiFiConfig(request, data, len, index, total); });

        server.on("/wificurrent", HTTP_GET, [](AsyncWebServerRequest *request)
                  { handleGetCurrentConfig(request); });

        // Compatibility: older client pages requested /get-wifi-config
        server.on("/get-wifi-config", HTTP_GET, [](AsyncWebServerRequest *request)
                  { handleGetCurrentConfig(request); });

        server.onNotFound([](AsyncWebServerRequest *request)
                          { request->send(404, "text/plain", "Page non trouvée"); });
        
        Serial.println("=== setupWebServer() END ===");
}

// Broadcast a log message to all connected websocket clients
void broadcastLog(const String &msg)
{
        // send to websocket clients
        if (ws.count() > 0)
        {
                ws.textAll(msg);
        }
}

// Formatted log: send to Serial and to websocket
void logf(const char *fmt, ...)
{
        char buf[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        // print to serial
        Serial.println(buf);

        // broadcast to web clients
        broadcastLog(String(buf));
}

void handleWiFiScan(AsyncWebServerRequest *request)
{
        Serial.println("Scan des réseaux WiFi...");

        int n = WiFi.scanNetworks();

        JsonDocument doc;
        JsonArray networks = doc.to<JsonArray>();

        for (int i = 0; i < n; i++)
        {
                JsonObject network = networks.add<JsonObject>();
                network["ssid"] = WiFi.SSID(i);
                network["rssi"] = WiFi.RSSI(i);
                network["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "open" : "encrypted";
        }

        String response;
        serializeJson(doc, response);

        request->send(200, "application/json", response);
        Serial.printf("Scan terminé: %d réseaux trouvés\n", n);
}

void handleWiFiConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
        static String jsonBuffer;

        if (index == 0)
        {
                jsonBuffer = "";
        }

        for (size_t i = 0; i < len; i++)
        {
                jsonBuffer += (char)data[i];
        }

        if (index + len == total)
        {
                Serial.println("Configuration WiFi reçue:");
                Serial.println(jsonBuffer);

                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, jsonBuffer);

                if (error)
                {
                        Serial.println("Erreur de parsing JSON");
                        request->send(400, "application/json", "{\"error\":\"JSON invalide\"}");
                        return;
                }

                currentConfig.mode = doc["mode"].as<String>();
                currentConfig.ssid = doc["ssid"].as<String>();
                currentConfig.password = doc["password"].as<String>();

                if (currentConfig.ssid.length() == 0)
                {
                        request->send(400, "application/json", "{\"error\":\"SSID requis\"}");
                        return;
                }

                bool saveSuccess = saveWiFiConfig();

                if (saveSuccess)
                {
                        request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Configuration sauvegardée\"}");
                        shouldConnectWiFi = true;
                }
                else
                {
                        request->send(500, "application/json", "{\"error\":\"Erreur de sauvegarde\"}");
                }
        }
}

void handleGetCurrentConfig(AsyncWebServerRequest *request)
{
        JsonDocument doc;
        doc["mode"] = currentConfig.mode;
        doc["ssid"] = currentConfig.ssid;
        doc["connected"] = WiFi.status() == WL_CONNECTED;
        doc["ip"] = WiFi.localIP().toString();

        String response;
        serializeJson(doc, response);

        request->send(200, "application/json", response);
}

bool saveWiFiConfig()
{
        if (!sdCardInitialized)
        {
                Serial.println("Erreur: Carte SD non initialisée");
                return false;
        }

        JsonDocument doc;
        doc["mode"] = currentConfig.mode;
        doc["ssid"] = currentConfig.ssid;
        doc["password"] = currentConfig.password;
        doc["timestamp"] = millis();

        if (SD.exists("/json/wifi.json"))
        {
                SD.remove("/json/wifi.json");
        }

        File file = SD.open("/json/wifi.json", FILE_WRITE);
        if (!file)
        {
                Serial.println("Erreur: Impossible d'ouvrir wifi.json en écriture");
                return false;
        }

        if (serializeJson(doc, file) == 0)
        {
                Serial.println("Erreur: Échec de l'écriture JSON");
                file.close();
                return false;
        }

        file.close();
        Serial.println("Configuration WiFi sauvegardée dans /json/wifi.json");

        return true;
}
//
bool loadWiFiConfig()
{
        if (!sdCardInitialized)
        {
                Serial.println("Carte SD non disponible pour le chargement");
                return false;
        }

        if (!SD.exists("/json/wifi.json"))
        {
                Serial.println("Fichier wifi.json non trouvé");
                return false;
        }

        File file = SD.open("/json/wifi.json", FILE_READ);
        if (!file)
        {
                Serial.println("Erreur: Impossible d'ouvrir wifi.json en lecture");
                return false;
        }

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error)
        {
                Serial.println("Erreur de parsing du fichier wifi.json");
                return false;
        }

        currentConfig.mode = doc["mode"].as<String>();
        currentConfig.ssid = doc["ssid"].as<String>();
        currentConfig.password = doc["password"].as<String>();

        Serial.println("Configuration WiFi chargée:");
        Serial.println("Mode: " + currentConfig.mode);
        Serial.println("SSID: " + currentConfig.ssid);

        return true;
}

void connectToConfiguredNetwork()
{
        if (currentConfig.ssid.length() == 0)
        {
                Serial.println("Aucun SSID configuré");
                return;
        }

        Serial.println("Tentative de connexion à: " + currentConfig.ssid);

        WiFi.mode(WIFI_AP_STA); // On garde l'AP actif pour ne pas couper le smartphone
        WiFi.begin(currentConfig.ssid.c_str(), currentConfig.password.c_str());

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20)
        {
                delay(500);
                Serial.print(".");
                attempts++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
                Serial.println();
                Serial.println("Connexion WiFi réussie!");
                Serial.print("IP: ");
                Serial.println(WiFi.localIP());
        }
        else
        {
                Serial.println();
                Serial.println("Échec de la connexion WiFi");
                setupAccessPoint();
        }
}

void printWiFiStatus()
{
        Serial.println("=== État WiFi ===");
        Serial.println("Mode: " + String(WiFi.getMode()));
        Serial.println("Statut: " + String(WiFi.status()));
        if (WiFi.status() == WL_CONNECTED)
        {
                Serial.println("SSID: " + WiFi.SSID());
                Serial.println("IP: " + WiFi.localIP().toString());
                Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
        }
        Serial.println("AP IP: " + WiFi.softAPIP().toString());
        Serial.println("================");
}

void resetWiFiConfig()
{
        currentConfig.mode = "";
        currentConfig.ssid = "";
        currentConfig.password = "";

        if (sdCardInitialized && SD.exists("/json/wifi.json"))
        {
                SD.remove("/json/wifi.json");
                Serial.println("Configuration WiFi réinitialisée");
        }
}

void loopWiFi()
{
        if (shouldConnectWiFi)
        {
                shouldConnectWiFi = false;
                connectToConfiguredNetwork();
        }
}

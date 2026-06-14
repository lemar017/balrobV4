#ifndef MODULEWIFI_H
#define MODULEWIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include "SD.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Variables globales - Declarations only
extern const char *ssid;
extern const char *password;
extern const char *fileJSON;
extern bool shouldConnectWiFi;


// Structure pour stocker la configuration WiFi
struct WiFiConfig
{
  String mode;
  String ssid;
  String password;
};

extern WiFiConfig currentConfig;
extern bool sdCardInitialized;
extern AsyncWebServer server;
extern AsyncWebSocket ws;

// Logging helpers: broadcast messages to connected web clients and to Serial
void broadcastLog(const String &msg);
void logf(const char *fmt, ...);

// Convenience macro: use LOG("format %d", val);
#define LOG(...) logf(__VA_ARGS__)

// Function declarations
void handleWiFiScan(AsyncWebServerRequest *request);
void handleGetCurrentConfig(AsyncWebServerRequest *request);
void handleWiFiConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
bool saveWiFiConfig();
void connectToConfiguredNetwork();
void setupAccessPoint();
void setupWebServer();
bool loadWiFiConfig();
void printWiFiStatus();
void resetWiFiConfig();
void loopWiFi();

#endif

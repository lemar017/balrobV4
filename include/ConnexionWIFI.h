#ifndef MODULECONNECTIONWIFI_H
#define MODULECONNECTIONWIFI_H

#include <Arduino.h>
#include <ArduinoJson.h>    
#include <ModuleVAR-GLOB.h>
#include <Module-WIFI.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

//--------------------------------------------------------------------
// Variable definitions
extern const char *ssid;
extern const char *password;
extern const char *fileJSON;

//--------------------------------------------------------------------
void initWiFi();
void initServerWeb();
void JSON2Client();
void JSON2Server(); 
void receivePIDenTR();
void sendXYJoystick();   
void initVarPIDfromSD();   




#endif
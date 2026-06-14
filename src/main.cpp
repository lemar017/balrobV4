/**
 * @file main.cpp (REFACTORISÉ)
 * @brief Exemple de main.cpp après refactorisation
 *
 * COMPARAISON:
 *
 * AVANT (chaotique):
 *   - 50+ variables globales
 *   - 20+ #define de debug
 *   - setup() avec 30 lignes d'initialisation
 *   - loop() avec 100+ lignes de logique
 *   - Difficile à déboguer
 *
 * APRÈS (propre):
 *   - 2 variables globales (robot, monitor)
 *   - setup() avec 5 lignes
 *   - loop() avec 10 lignes
 *   - Facile à maintenir et déboguer
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include "RobotSystem.h"
#include "TimingController.h"
#include "PerformanceMonitor.h"

// ===== INSTANCES GLOBALES =====
RobotSystem robot;                                                 // Orchestrateur principal
TimingController timing(10000, 20000, 500000);                     // 10ms PID, 20ms display, 500ms LED
PerformanceMonitor monitor(PerformanceMonitor::OutputFormat::CSV); // Télémétrie

// ===== SETUP =====
void setup()
{
  // Initialisation complète - simple et propre
  robot.initialize();

  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║   ROBOT D'ÉQUILIBRE - VERSION 3.0   ║");
  Serial.println("╚════════════════════════════════════╝\n");
}

// ===== LOOP PRINCIPALE =====
void loop()
{
  // Mettre à jour les timers
  timing.update();

  // Mettre à jour le robot (tous les composants)
  if (!robot.update())
  {
    Serial.println("ERREUR CRITIQUE: Le robot ne répond plus!");
    delay(1000);
    return;
  }

  // ===== EXÉCUTION CONDITIONNELLE (basée sur les timers) =====

  // Traiter le PID à intervalle régulier (10ms)
  if (timing.shouldExecutePID())
  {
    // Le PID est déjà calculé dans robot.update()
  }

  // Afficher les données à intervalle régulier (20ms)
  if (timing.shouldDisplayData())
  {
    monitor.logData(robot.getState());
  }

  // Mettre à jour la LED à intervalle régulier (500ms)
  if (timing.shouldToggleLED())
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  // Mettre à jour la télémétrie (1s)
  if (timing.shouldUpdateTelemetry())
  {
    RobotState &state = robot.getMutableState();
    state.telemetry.loopFrequencyHz = timing.getLoopFrequencyHz();
    state.telemetry.freeHeap = esp_get_free_heap_size();
  }
}

// ===== HANDLERS WEB (pour ESP32 + AsyncWebServer) =====

/**
 * Handler pour recevoir les paramètres PID depuis l'interface web
 * URL: /api/pid?kp=0.5&ki=0.1&kd=0.05&ta=2.0
 */
void handlePIDUpdate(AsyncWebServerRequest *request)
{
  if (request->hasParam("kp") && request->hasParam("ki") &&
      request->hasParam("kd") && request->hasParam("ta"))
  {

    double kp = request->getParam("kp")->value().toDouble();
    double ki = request->getParam("ki")->value().toDouble();
    double kd = request->getParam("kd")->value().toDouble();
    float ta = request->getParam("ta")->value().toFloat();

    robot.setPidParameters(kp, ki, kd, ta);

    request->send(200, "application/json", "{\"status\":\"ok\"}");
  }
  else
  {
    request->send(400, "application/json", "{\"error\":\"paramètres manquants\"}");
  }
}

/**
 * Handler pour recevoir les commandes du joystick
 * URL: /api/joystick?x=-50&y=100
 */
void handleJoystickUpdate(AsyncWebServerRequest *request)
{
  if (request->hasParam("x") && request->hasParam("y"))
  {
    int x = request->getParam("x")->value().toInt();
    int y = request->getParam("y")->value().toInt();

    robot.setJoystickInput(x, y);

    request->send(200, "application/json", "{\"status\":\"ok\"}");
  }
  else
  {
    request->send(400, "application/json", "{\"error\":\"paramètres manquants\"}");
  }
}

/**
 * Handler pour changer le type de filtre
 * URL: /api/filter/type?type=2 (0=MOYENNE, 1=EMA, 2=COMPLEMENTAIRE, 3=KALMAN)
 */
void handleFilterTypeUpdate(AsyncWebServerRequest *request)
{
  if (request->hasParam("type"))
  {
    int type = request->getParam("type")->value().toInt();
    if (type >= 0 && type <= 3)
    {
      robot.setFilterType(static_cast<FilterType>(type));
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    }
    else
    {
      request->send(400, "application/json", "{\"error\":\"type invalide\"}");
    }
  }
  else
  {
    request->send(400, "application/json", "{\"error\":\"type manquant\"}");
  }
}

/**
 * Handler pour obtenir les diagnostiques
 * URL: /api/diagnostics
 */
void handleDiagnostics(AsyncWebServerRequest *request)
{
  request->send(200, "application/json", robot.getDiagnostics());
}

/**
 * Handler pour obtenir l'état courant
 * URL: /api/state
 */
void handleGetState(AsyncWebServerRequest *request)
{
  const RobotState &state = robot.getState();

  StaticJsonDocument<1024> doc;
  doc["imu"]["angle"] = state.imu.currentAngle;
  doc["imu"]["gyroRate"] = state.imu.gyroRate;
  doc["pid"]["kp"] = state.pid.params.kp;
  doc["pid"]["ki"] = state.pid.params.ki;
  doc["pid"]["kd"] = state.pid.params.kd;
  doc["pid"]["targetAngle"] = state.pid.params.targetAngle;
  doc["motors"]["left"] = state.motors.current.speedLeft;
  doc["motors"]["right"] = state.motors.current.speedRight;
  doc["telemetry"]["freeHeap"] = state.telemetry.freeHeap;
  doc["telemetry"]["frequency"] = state.telemetry.loopFrequencyHz;

  String output;
  serializeJson(doc, output);
  request->send(200, "application/json", output);
}

/**
 * Handler pour changer le mode d'affichage
 * URL: /api/monitor/format?format=2 (0=RAW, 1=FILTERED, 2=PID, 3=CSV, ...)
 */
void handleMonitorFormatUpdate(AsyncWebServerRequest *request)
{
  if (request->hasParam("format"))
  {
    int format = request->getParam("format")->value().toInt();
    if (format >= 0 && format <= 8)
    {
      monitor.setOutputFormat(static_cast<PerformanceMonitor::OutputFormat>(format));
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    }
    else
    {
      request->send(400, "application/json", "{\"error\":\"format invalide\"}");
    }
  }
  else
  {
    request->send(400, "application/json", "{\"error\":\"format manquant\"}");
  }
}

// ===== À APPELER LORS DE L'INITIALISATION DU SERVEUR WEB =====
void setupWebAPIHandlers(AsyncWebServer *server)
{
  // Mettre à jour les paramètres
  server->on("/api/pid", HTTP_GET, handlePIDUpdate);
  server->on("/api/joystick", HTTP_GET, handleJoystickUpdate);
  server->on("/api/filter/type", HTTP_GET, handleFilterTypeUpdate);
  server->on("/api/monitor/format", HTTP_GET, handleMonitorFormatUpdate);

  // Récupérer des infos
  server->on("/api/diagnostics", HTTP_GET, handleDiagnostics);
  server->on("/api/state", HTTP_GET, handleGetState);

  Serial.println("✓ Handlers web API configurés");
}



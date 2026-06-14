
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
#include <ConnexionWIFI.h>

// Variable definitions (declared in ConnexionWIFI.h)
// const char *ssid = "bmc21";
const char *ssid = "BMC21_WL_5.0_EXT2G";
const char *password = "4525150587";
const char *fileJSON = "/balrob/json/pid.json";

//----------------Initialisation du WIFI---------------------------------------
void initWiFi()
{

  IPAddress IP(192, 168, 20, 243);       // adresse fixe
  IPAddress gateway(192, 168, 20, 1);    // passerelle par défaut
  IPAddress subnet(255, 255, 255, 0);    // masque de sous réseau
  IPAddress dns(8, 8, 8, 8);             // DNS
  WiFi.config(IP, gateway, subnet, dns); // Permet de configurer l’adresse IP et les paramètres de connexion au réseau WiFi local.
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }

  Serial.print("Adress IP = ");
  Serial.println(WiFi.localIP());
  String mac = WiFi.macAddress(); // Récupérer l'adresse MAC de l'ESP32
  Serial.print("Adresse MAC de l'ESP32 : ");
  Serial.println(mac); // Afficher l'adresse MAC
  Serial.println("");
  Serial.println("WiFi connected");
}
//-------------On envoi la page index.html de la carte SD et envoi au client-----------
void initServerWeb()
{
  Serial.println("");
  Serial.println("----------on passe dans initServerWeb()-------------------");
  // Call the central setup in Module-WIFI.cpp to register websocket and routes
  setupWebServer();
}

//------------On récupère les infos JSON de SD et envoi au client-------------
void JSON2Client()

{
  Serial.println("");
  Serial.println("----------on passe dans JSON2Cli()--------------------------");
  server.on("/getpid", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    File fichier = SD.open(fileJSON);
    if (!fichier)
    {
      request->send(404, "text/plain", "JSON file not found");
      return;
    }
    String pidJson;
    while (fichier.available())
    {
      pidJson += (char)fichier.read();
    }
    fichier.close();
    // Envoyer le contenu JSON comme réponse

    request->send(200, "application/json", pidJson);
    //Serial.print("json envoyé = ");
    //Serial.println(pidJson);
   });
 
}
//------------On Récupère les infos de la forme de la page HTML et envoi au serveur----
void JSON2Server()
{
    Serial.println("");
    Serial.println("----------on passe dans JSON2Serv()--------------------------");
    // Route pour gérer la requête POST avec les données JSON{
    server.on("/pidsave", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
              static String jsonBuffer;

              // start of a new body
              if (index == 0) {
                jsonBuffer = "";
                Serial.println("");
                Serial.println("----------Début réception /pidsave--------------------------");
                Serial.print("Taille totale attendue: ");
                Serial.println(total);
              }

              // safety: guard against too large payloads
              const size_t MAX_PAYLOAD = 4096;
              if (total > MAX_PAYLOAD) {
                Serial.println("Payload trop grand");
                request->send(413, "text/plain", "Payload too large");
                return;
              }

              // append received chunk
              for (size_t i = 0; i < len; i++) {
                jsonBuffer += (char)data[i];
              }

              // if not all data received yet, wait for remaining chunks
              if (index + len < total) {
               Serial.print("Reçu ");
               Serial.print(index + len);
                Serial.print(" / ");
                Serial.println(total);
                return;
              }

              // full body received
          //    Serial.print("Corps complet reçu (bytes): ");
          //    Serial.println(jsonBuffer.length());
          //    Serial.print("Contenu: ");
          //    Serial.println(jsonBuffer);

              JsonDocument doc;
              DeserializationError error = deserializeJson(doc, jsonBuffer);
              if (error) {
                Serial.print("Erreur JSON: ");
                Serial.println(error.c_str());
                request->send(400, "text/plain", "Invalid JSON");
                return;
              }

              // remove previous file to ensure truncation
              const char *path = "/balrob/json/pid.json";
              if (SD.exists(path)) {
                SD.remove(path);
                Serial.println("Ancien fichier supprimé");
              }

              File file = SD.open(path, FILE_WRITE);
              if (!file) {
                Serial.println("Erreur: Impossible d'ouvrir le fichier en écriture");
                request->send(500, "text/plain", "Failed to open file for writing");
                return;
              }

              size_t bytesWritten = serializeJson(doc, file);
              file.close();

              Serial.print("Bytes écrits: ");
              Serial.println(bytesWritten);
              if (bytesWritten == 0) {
                Serial.println("Aucun octet écrit (erreur)");
                request->send(500, "text/plain", "Write failed");
                return;
              }

              Serial.println("Fichier sauvegardé avec succès");
              request->send(200, "text/plain", "File updated successfully"); });
}
//------------On envoi au serveur les valeurs PID en temps reel------------------------
void receivePIDenTR()
{
    server.on("/updatePID", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data);
      Serial.println("=== Début de /updatePID ===");
      if (error) {
        Serial.println("Erreur de JSON");
        request->send(400, "text/plain", "Erreur JSON");
        return;
      }

      String param = doc["parameter"];
      float value = doc["value"];

    //  Serial.print("param = ");
    //  Serial.print(param);
    //  Serial.print("           value = ");
    //  Serial.println(value);

      if (param == "TAval") {
        TA = value;
    //    Serial.print("TA = ");
    //    Serial.println(TA);

       
      } else if (param == "KPval") {
        KP = value;
    //    Serial.print("KP = ");
    //    Serial.println(KP);
        
      } else if (param == "KIval") {
        KI = value;
    //    Serial.print("KI = ");
    //    Serial.println(KI);

      } else if (param == "KDval") {
        KD = value;
    //    Serial.print("KD = ");
    //    Serial.println(KD);
      
      }

      request->send(200, "text/plain", "OK"); });
}
//------------On envoi des données X et Y du Joystick---------------------------------
void sendXYJoystick()
{
    server.on("/updateXYJoystick", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {

      String body = "";
      for (size_t i = 0; i < len; i++) {
        body += (char)data[i];
      }

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, body);

      if (error) {
        Serial.println("Erreur de parsing JSON");
        request->send(400, "text/plain", "Erreur JSON");
        return;
      }

       joyX_IHM = doc["valX"].as<String>();
       joyY_IHM = doc["valY"].as<String>();

       joyX = joyX_IHM.toInt();
       joyY = joyY_IHM.toInt();
/*
      Serial.println(""); Serial.println("");     
      Serial.print("joyX_IHM = "); Serial.println(joyX_IHM);
      Serial.print("joyY_IHM = "); Serial.println(joyY_IHM);
      Serial.print("joyX = "); Serial.println(joyX);
      Serial.print("joyY = "); Serial.println(joyY);
      Serial.println(""); Serial.println("");  
*/
      request->send(200, "application/json", "{\"status\":\"OK\"}"); });
}

//------------Initialisation du PID avec les valeurs SD---------------------------------
void initVarPIDfromSD()
{
    File file = SD.open(fileJSON);

    if (!file)
    {
      Serial.println("Erreur lors de l'ouverture du fichier");
      return;
    }
    // Taille du fichier
    size_t fileSize = file.size();
    // Allouer un tampon pour contenir le contenu du fichier
    std::unique_ptr<char[]> buf(new char[fileSize]);
    // Lire le contenu du fichier dans le tampon
    file.readBytes(buf.get(), fileSize);
    // Fermer le fichier
    file.close();

    // Parser le JSON à partir du tampon
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf.get());
    if (error)
    {
      Serial.print("Erreur lors du parsing du JSON : ");
      Serial.println(error.c_str());
      return;
    }
    // Accéder aux données JSON

    TA = float(doc["TAval"]);
    KP = double(doc["KPval"]);
    KI = double(doc["KIval"]);
    KD = double(doc["KDval"]);
    TA_IHM = String(doc["TAval"].as<double>());
    KP_IHM = String(doc["KPval"].as<double>());
    KD_IHM = String(doc["KDval"].as<double>());
    KI_IHM = String(doc["KIval"].as<double>());

    // Afficher les données
    /*
    Serial.print("TargetAngle  : ");
    Serial.println(TA_IHM);
    Serial.print("Proportionnel  : ");
    Serial.println(KP_IHM);
    Serial.print("Integral  : ");
    Serial.println(KI_IHM);
    Serial.print("Derive  : ");
    Serial.println(KD_IHM);
   */
}

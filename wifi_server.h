#pragma once

#include <WiFi.h>
#include "ui_render.h"
#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;
String wifiSsid;
String wifiPass;
bool shouldConnect;
bool isConnecting;
bool hasConnected;
extern const char* apName;
String localIp;
unsigned long connectStartTime;

/* Setup Wi-Fi AP and WebServer routes */
inline void startWifiAndServer() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(
    IPAddress(192, 168, 4, 1),
    IPAddress(192, 168, 4, 1),
    IPAddress(255, 255, 255, 0)
  );
  WiFi.softAP(apName);

  showText(String("WIFI: ") + apName, 1);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", R"rawliteral(
      <h2>Wi-Fi Config</h2>
      <form method='POST' action='/connect'>
        SSID: <input name='ssid'><br>
        Password: <input name='pass' type='password'><br>
        <input type='submit' value='Connect'>
      </form>
    )rawliteral");
  });

  server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true)) {
      wifiSsid = request->getParam("ssid", true)->value();
    }

    if (request->hasParam("pass", true)) {
      wifiPass = request->getParam("pass", true)->value();
    }

    shouldConnect = true;

    request->send(200, "text/html", "Connecting to " + wifiSsid);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
    response->addHeader("Location", "/");
    request->send(response);
  });

  server.begin();
}

/* Handle Wi-Fi STA connection attempts (called from loop) */
inline void handleWifiConnection() {
  if (shouldConnect) {
    shouldConnect = false;

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(wifiSsid.c_str(), wifiPass.c_str());

    isConnecting = true;
    connectStartTime = millis();
  }

  if (isConnecting) {
    if (WiFi.status() == WL_CONNECTED) {
      isConnecting = false;
      hasConnected = true;
      localIp = WiFi.localIP().toString();
      WiFi.softAPdisconnect(true);

      showText("Connect: " + localIp, 1);

    } else if (millis() - connectStartTime > 10000) {
      isConnecting = false;
      showText("Failed!", 1);

    } else {
      showText("Connecting...", 1);
    }
  }
}

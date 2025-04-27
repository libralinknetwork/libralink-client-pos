#include <M5Core2.h>
#include "ui_render.h"
#include "wifi_server.h"
#include <ESPAsyncWebServer.h>
#include "ble_server.h"
#include "file_manager.h"
#include "libralink_utils.h"
#include "mbedtls/base64.h"
#include <pb_decode.h>

#include <LittleFS.h>
#include "random_id.h"
#include "merchant_splash_screen.h"

#include "handler_share_payer_details.h"
#include "handler_signed_payment_request.h"
#include "handler_signed_echeck.h"
#include "handler_read_message.h"

#define IF_DEBUG 1

AsyncWebServer server(80);
const char* apName = generateRandomId();

BLECharacteristic* pPayerDetails = nullptr;
BLECharacteristic* pSignedPaymentRequest = nullptr;
BLECharacteristic* pSignedECheck = nullptr;
BLECharacteristic* pReadBLEMessage = nullptr;

String currentOrderId = "";
double currentOrderAmount = -1;
String currentBleMessage;

void setup() {
  M5.begin();
  M5.Lcd.fillScreen(TFT_WHITE);

  drawSplashScreen();
  delay(3000);

  startWifiAndServer();
  startBLEServer("LibraLink BLE", new MyCallbackPayerDetails(), new MySignedPaymentRequestCallbacks(), new MySignedECheckCallbacks(), new ReadMessageRequestCallbacks());

  setupFileManagerEndpoints(server);

  if (!LittleFS.begin()) {
    LittleFS.format();
    if (LittleFS.begin()) {
    } else {
        Serial.println("LittleFS failed after format");
    }    
  }

  #if IF_DEBUG
    server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "currentOrderId: " + String(currentOrderId) + ", currentOrderAmount: " 
                            + String(currentOrderAmount) + ", currentBleMessage: " + String(currentBleMessage));
    });
  #endif  

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    showText("Welcome", 2);
    request->send(200, "text/html", "OK");
  });

  server.on("/api/v1/orders/create", HTTP_GET, [](AsyncWebServerRequest *request) {

    if (request->hasParam("id")) {
      currentOrderId = request->getParam("id")->value();
    }

    if (request->hasParam("amount")) {
      currentOrderAmount = request->getParam("amount")->value().toDouble();
    }

    // Create folder
    String folderPath = "/" + currentOrderId;
    if (!LittleFS.exists(folderPath)) {
      if (LittleFS.mkdir(folderPath)) {
        showText("$ " + String(currentOrderAmount, 2), 2);
        startAdvertising();
        request->send(200, "text/plain", "Order received: id=" + currentOrderId + ", amount=" + String(currentOrderAmount, 2));

      } else {
        showText("Err: " + folderPath, 1);
        request->send(500, "text/plain", "Unable to register, id=" + currentOrderId);        

      }
    } else {
      showText("Err: Already processed", 1);
      request->send(500, "text/plain", "Already processed");
    }
  });

  server.on("/api/v1/orders/cancel", HTTP_GET, [](AsyncWebServerRequest *request) {
    showText("Welcome", 2);
    stopAdvertising();

    request->send(200, "text/plain", "Active order cancelled");
  });
}

void loop() {
  handleWifiConnection();
  delay(1000);
}

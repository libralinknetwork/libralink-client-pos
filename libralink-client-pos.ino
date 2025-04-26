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

AsyncWebServer server(80);
const char* apName = "LibraLink";

BLECharacteristic* pCharacteristic = nullptr;  // must be defined somewhere

String currentOrderId = "";
double currentOrderAmount = -1;

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {
    std::string value = std::string((char*)pChar->getData(), pChar->getLength());

    /* Step 1: Base64 decode using mbedtls */
    size_t decodedLen = 0;
    size_t estimatedLen = (value.length() * 3) / 4; // rough estimation
    uint8_t* decodedData = (uint8_t*) malloc(estimatedLen);
    if (!decodedData) {
        showText("Failed to allocate memory", 1);
        return;
    }

    int ret = mbedtls_base64_decode(decodedData, estimatedLen, &decodedLen, (const unsigned char*)value.c_str(), value.length());
    if (ret != 0) {
        showText("Base64 decode error", 1);
        free(decodedData);
        return;
    }

    /* Step 2: Protobuf decode */
    io_libralink_client_payment_proto_Envelope envelope = io_libralink_client_payment_proto_Envelope_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(decodedData, decodedLen);

    if (!pb_decode(&stream, io_libralink_client_payment_proto_Envelope_fields, &envelope)) {
        showText("Protobuf decode failed", 1);
        free(decodedData);
        return;
    }

    /* Step 3: Check what entity is inside */
    if (envelope.has_content) {
        if (envelope.content.which_entity == io_libralink_client_payment_proto_EnvelopeContent_check_tag) {
            showText("Received an E-Check", 1);
        } else {
            showText("Entity type: " + String(envelope.content.which_entity), 1);
        }
    } else {
        showText("Envelope has no content", 1);
    }

    free(decodedData);   
  }

  void onRead(BLECharacteristic* pChar) override {

    String base64Data = generatePaymentRequestBase64(
        String(currentOrderAmount, 2),
        "0xFromAddress",
        "0xToAddress",
        "0xProcessor",
        currentOrderId
    );

    createFile("/" + currentOrderId, "payment_request.txt", base64Data);
    pChar->setValue(base64Data.c_str());
  }
};

void setup() {
  M5.begin();
  BLEDevice::init("LibraLink BLE");

  startWifiAndServer();  // Setup Wi-Fi AP and WebServer
  startBLEServer(new MyCallbacks());  // BLE server setup
  setupFileManagerEndpoints(server);

  if (!LittleFS.begin()) {
    LittleFS.format();  // <-- TRY formatting if mount failed
    if (LittleFS.begin()) {
    } else {
        Serial.println("LittleFS failed after format");
    }    
  }

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
        showText("USD " + String(currentOrderAmount, 2), 2);
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
  handleWifiConnection();  // Now only one simple call
  delay(1000);
}

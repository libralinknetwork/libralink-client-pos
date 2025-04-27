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

AsyncWebServer server(80);
const char* apName = generateRandomId();

BLECharacteristic* pCharacteristic = nullptr;  // must be defined somewhere

String currentOrderId = "";
double currentOrderAmount = -1;

String payer;
String payerProc;
String challenge;

bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    char *buffer = (char *)(*arg);
    size_t len = stream->bytes_left;
    if (len > 127) len = 127;
    if (!pb_read(stream, (pb_byte_t*)buffer, len)) return false;
    buffer[len] = '\0';
    return true;
}

/* Capture context to capture SharePayerDetails raw bytes */
struct CaptureContext {
    uint8_t *buffer;
    size_t max_size;
    size_t size;
    pb_size_t *which_entity_ptr; // pointer to which_entity
};

bool capture_entity(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    CaptureContext *ctx = (CaptureContext*)(*arg);

    size_t to_read = stream->bytes_left; // save BEFORE reading
    if (to_read > ctx->max_size) return false;
    if (!pb_read(stream, ctx->buffer, to_read)) return false;

    ctx->size = to_read;  // must be original bytes left
    if (ctx->which_entity_ptr) {
        *(ctx->which_entity_ptr) = field->tag;
    }

    return true;
}

class MyCallbacks : public BLECharacteristicCallbacks {

  void onWrite(BLECharacteristic* pChar) override {
      std::string value = std::string((char*) pChar->getData(), pChar->getLength());

      // For debug purpose only
      createFile("/", String(generateRandomId()) + ".txt", String(value.c_str()));

      /* Step 1: Base64 decode using mbedtls */
      size_t decodedLen = 0;
      size_t estimatedLen = (value.length() * 3) / 4; // rough estimation
      uint8_t* decodedData = (uint8_t*) malloc(estimatedLen);
      if (!decodedData) {
          showText("Failed to allocate memory", 1);
          return;
      }

      int ret = mbedtls_base64_decode(decodedData, estimatedLen, &decodedLen, (const unsigned char*) value.c_str(), value.length());
      if (ret != 0) {
          showText("Base64 decode error", 1);
          free(decodedData);
          return;
      }

    io_libralink_client_payment_proto_Envelope envelope = io_libralink_client_payment_proto_Envelope_init_zero;
    uint8_t entityBuffer[256] = {0};
    CaptureContext captureCtx = {
        .buffer = entityBuffer,
        .max_size = sizeof(entityBuffer),
        .size = 0,
        .which_entity_ptr = &(envelope.content.which_entity)
    };

    envelope.content.entity.sharePayerDetails.funcs.decode = capture_entity;
    envelope.content.entity.sharePayerDetails.arg = &captureCtx;
    envelope.content.entity.check.funcs.decode = capture_entity;
    envelope.content.entity.check.arg = &captureCtx;
    envelope.content.entity.paymentRequest.funcs.decode = capture_entity;
    envelope.content.entity.paymentRequest.arg = &captureCtx;
    envelope.content.entity.errorResponse.funcs.decode = capture_entity;
    envelope.content.entity.errorResponse.arg = &captureCtx;

    // Decode Envelope
    pb_istream_t stream = pb_istream_from_buffer(decodedData, decodedLen);
    if (!pb_decode(&stream, io_libralink_client_payment_proto_Envelope_fields, &envelope)) {
        showText("Failed to decode Envelope", 1);
        free(decodedData);
        return;
    }

    // showText("Entity type: " + String(envelope.content.which_entity), 1);
    // return;

    if (envelope.content.which_entity == io_libralink_client_payment_proto_EnvelopeContent_sharePayerDetails_tag) {

        // Decode SharePayerDetails
        io_libralink_client_payment_proto_SharePayerDetails details = io_libralink_client_payment_proto_SharePayerDetails_init_zero;

        char challengeBuf[128] = {0};
        char fromBuf[128] = {0};
        char fromProcBuf[128] = {0};

        details.challenge.funcs.decode = decode_string;
        details.challenge.arg = challengeBuf;
        details.from.funcs.decode = decode_string;
        details.from.arg = fromBuf;
        details.fromProc.funcs.decode = decode_string;
        details.fromProc.arg = fromProcBuf;

        pb_istream_t entityStream = pb_istream_from_buffer(entityBuffer, captureCtx.size);
        if (pb_decode(&entityStream, io_libralink_client_payment_proto_SharePayerDetails_fields, &details)) {
            
            challenge = String(challengeBuf);
            payer = String(fromBuf);
            payerProc = String(fromProcBuf);

            showText("Success: " + String(payerProc), 1);
        } else {
            showText("Failed to decode SharePayerDetails", 1);
        }
    }

    free(decodedData);
  }

  void onRead(BLECharacteristic* pChar) override {

    String base64Data = generatePaymentRequestBase64(
        String(currentOrderAmount, 2),
        payer,
        "0xToAddress",
        payerProc,
        currentOrderId
    );

    createFile("/" + currentOrderId, "payment_request.txt", base64Data);
    pChar->setValue(base64Data.c_str());
  }
};

void setup() {
  M5.begin();
  M5.Lcd.fillScreen(TFT_WHITE);

  drawSplashScreen();
  delay(3000);

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
  handleWifiConnection();  // Now only one simple call
  delay(1000);
}

#pragma once

#include "config.h"
#include "libralink.pb.h"
#include <pb_decode.h>
#include <mbedtls/base64.h>
#include "libralink_utils.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>
#include "file_manager.h"
#include "ui_render.h"

// External variables (provided by your main sketch)
extern String currentOrderId;
extern double currentOrderAmount;

class MySignedPaymentRequestCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {

      std::string value = std::string((char*) pChar->getData(), pChar->getLength());

      #if IF_DEBUG
        createFile("/", currentOrderId + "_debug_payer_request.txt", String(value.c_str()));
      #endif

      size_t decodedLen = 0;
      size_t estimatedLen = (value.length() * 3) / 4;
      uint8_t* decodedData = (uint8_t*) malloc(estimatedLen);
      if (!decodedData) {
          showText("Failed to allocate memory", 1);
          return;
      }

      if (mbedtls_base64_decode(decodedData, estimatedLen, &decodedLen, (const unsigned char*) value.c_str(), value.length()) != 0) {
          showText("Base64 decode error", 1);
          free(decodedData);
          return;
      }

      // Step 1: Decode Outer Envelope
      io_libralink_client_payment_proto_Envelope outerEnvelope = io_libralink_client_payment_proto_Envelope_init_zero;
      uint8_t outerEntityBuffer[512] = {0};

      CaptureContext outerCaptureCtx = {
          .buffer = outerEntityBuffer,
          .max_size = sizeof(outerEntityBuffer),
          .size = 0,
          .which_entity_ptr = &(outerEnvelope.content.which_entity)
      };

      outerEnvelope.content.entity.envelope.funcs.decode = capture_entity;
      outerEnvelope.content.entity.envelope.arg = &outerCaptureCtx;

      pb_istream_t stream = pb_istream_from_buffer(decodedData, decodedLen);
      if (!pb_decode(&stream, io_libralink_client_payment_proto_Envelope_fields, &outerEnvelope)) {
          showText("Failed to decode Outer Envelope", 1);
          free(decodedData);
          return;
      }

      // Step 2: Decode Inner Envelope
      io_libralink_client_payment_proto_Envelope innerEnvelope = io_libralink_client_payment_proto_Envelope_init_zero;
      uint8_t innerEntityBuffer[512] = {0};

      CaptureContext innerCaptureCtx = {
          .buffer = innerEntityBuffer,
          .max_size = sizeof(innerEntityBuffer),
          .size = 0,
          .which_entity_ptr = &(innerEnvelope.content.which_entity)
      };

      pb_istream_t innerStream = pb_istream_from_buffer(outerEntityBuffer, outerCaptureCtx.size);

      // Setup capture for paymentRequest inside inner envelope
      innerEnvelope.content.entity.paymentRequest.funcs.decode = capture_entity;
      innerEnvelope.content.entity.paymentRequest.arg = &innerCaptureCtx;

      if (!pb_decode(&innerStream, io_libralink_client_payment_proto_Envelope_fields, &innerEnvelope)) {
          showText("Failed to decode Inner Envelope", 1);
          free(decodedData);
          return;
      }

      // Step 3: Decode actual PaymentRequest
      io_libralink_client_payment_proto_PaymentRequest paymentRequest = io_libralink_client_payment_proto_PaymentRequest_init_zero;

      char amountBuf[64] = {0};
      char fromBuf[128] = {0};
      char toBuf[128] = {0};

      paymentRequest.amount.funcs.decode = decode_string;
      paymentRequest.amount.arg = amountBuf;

      paymentRequest.from.funcs.decode = decode_string;
      paymentRequest.from.arg = fromBuf;

      paymentRequest.to.funcs.decode = decode_string;
      paymentRequest.to.arg = toBuf;

      pb_istream_t paymentStream = pb_istream_from_buffer(innerEntityBuffer, innerCaptureCtx.size);

      if (!pb_decode(&paymentStream, io_libralink_client_payment_proto_PaymentRequest_fields, &paymentRequest)) {
          showText("Failed to decode PaymentRequest", 1);
          free(decodedData);
          return;
      }            

      /* ... */
      showOrderAmount("$ " + String(amountBuf, 2));
      free(decodedData);                 
  }
  void onRead(BLECharacteristic* pChar) override {}
};

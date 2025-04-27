#pragma once

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
#include "encryption_utils.h"

// External variables (provided by your main sketch)
extern String currentOrderId;
extern double currentOrderAmount;
extern String currentBleMessage;

/* Class for handling incoming SharePayerDetails */
class MyCallbackPayerDetails : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {
      std::string value = std::string((char*) pChar->getData(), pChar->getLength());

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

      pb_istream_t stream = pb_istream_from_buffer(decodedData, decodedLen);
      if (!pb_decode(&stream, io_libralink_client_payment_proto_Envelope_fields, &envelope)) {
          showText("Failed to decode Envelope", 1);
          free(decodedData);
          return;
      }

      if (envelope.content.which_entity == io_libralink_client_payment_proto_EnvelopeContent_sharePayerDetails_tag) {
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
              String challenge = String(challengeBuf);
              String payer = String(fromBuf);
              String payerProc = String(fromProcBuf);

              String base64Data = generatePaymentRequestBase64(
                  String(currentOrderAmount, 2),
                  payer,
                  "0xToAddress",
                  payerProc,
                  currentOrderId
              );

              currentBleMessage = base64Data;
              showText("$ " + String(currentOrderAmount, 2), 2, TFT_MAROON);
          } else {
              showText("Failed to decode SharePayerDetails", 1);
          }
      }

      free(decodedData);    
  }

  void onRead(BLECharacteristic* pChar) override {}
};

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

class MySignedECheckCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {

      std::string value = std::string((char*) pChar->getData(), pChar->getLength());
      #if IF_DEBUG
        createFile("/", currentOrderId + "_debug_e_check.txt", String(value.c_str()));
      #endif

      /* TODO: compose and save {orderId}_pending_deposit.txt to FS */
  }

  void onRead(BLECharacteristic* pChar) override {}
};

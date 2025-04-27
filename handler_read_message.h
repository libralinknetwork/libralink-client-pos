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

extern String currentBleMessage;

class ReadMessageRequestCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {}

  void onRead(BLECharacteristic* pChar) override {
    pChar->setValue(currentBleMessage.c_str());
  }
};

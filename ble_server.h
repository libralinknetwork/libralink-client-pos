#pragma once

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

extern BLECharacteristic* pCharacteristic;  // declare it here

// UUIDs (you can adjust if needed)
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-123456789abc"

inline void startBLEServer(BLECharacteristicCallbacks* callbacks) {
    BLEDevice::init("LibraLink BLE");

    BLEServer* pServer = BLEDevice::createServer();
    pServer->setCallbacks(new BLEServerCallbacks());  // You can customize if needed

    BLEService* pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_WRITE
                      );
    pCharacteristic->setCallbacks(callbacks);

    pCharacteristic->setValue("Hello BLE");
    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
}

inline void startAdvertising() {
    BLEDevice::getAdvertising()->start();
}

inline void stopAdvertising() {
    BLEDevice::getAdvertising()->stop();
}

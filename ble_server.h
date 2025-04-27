#pragma once

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

extern BLECharacteristic* pPayerDetails;
extern BLECharacteristic* pSignedPaymentRequest;
extern BLECharacteristic* pSignedECheck;
extern BLECharacteristic* pReadBLEMessage;

#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"

#define UUID_WRITE_SHARE_PAYER_DETAILS "2bb6c1d4-5831-4a4e-86bd-3df81fa7c774"
#define UUID_WRITE_SIGNED_PAYMENT_REQUEST "bebbd87f-7327-4552-ae15-3b4bd786b096"
#define UUID_WRITE_SIGNED_E_CHECK "a0dd609f-fbda-4996-ab9c-6d53fe763945"
#define UUID_READ_BLE_MESSAGE "a20d3f9f-1d6e-4936-80ab-cd7cb8c8b550"

inline void startBLEServer(String bleName, BLECharacteristicCallbacks* callbackPayerDetails, BLECharacteristicCallbacks* callbackSignedPaymentRequest, 
                            BLECharacteristicCallbacks* callbacSignedECheck, BLECharacteristicCallbacks* callbacMessageRead) {
    
    BLEDevice::init(bleName.c_str());

    BLEServer* pServer = BLEDevice::createServer();
    pServer->setCallbacks(new BLEServerCallbacks());

    BLEService* pService = pServer->createService(SERVICE_UUID);
    pPayerDetails = pService->createCharacteristic(UUID_WRITE_SHARE_PAYER_DETAILS, BLECharacteristic::PROPERTY_WRITE);
    pPayerDetails->setCallbacks(callbackPayerDetails);

    pSignedPaymentRequest = pService->createCharacteristic(UUID_WRITE_SIGNED_PAYMENT_REQUEST, BLECharacteristic::PROPERTY_WRITE);
    pSignedPaymentRequest->setCallbacks(callbackSignedPaymentRequest);

    pSignedECheck = pService->createCharacteristic(UUID_WRITE_SIGNED_E_CHECK, BLECharacteristic::PROPERTY_WRITE);
    pSignedECheck->setCallbacks(callbacSignedECheck);

    pReadBLEMessage = pService->createCharacteristic(UUID_READ_BLE_MESSAGE, BLECharacteristic::PROPERTY_READ);
    pReadBLEMessage->setCallbacks(callbacMessageRead);    

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

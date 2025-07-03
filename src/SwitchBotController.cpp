/**
 * @file SwitchBotController.cpp
 * @brief SwitchBot制御用クラスの実装
 */

#include "SwitchBotController.h"
#include <Arduino.h>

// 静的メンバーの定義
const NimBLEUUID SwitchBotController::serviceUUID("cba20d00-224d-11e6-9fb8-0002a5d5c51b");
const NimBLEUUID SwitchBotController::charUUID("cba20002-224d-11e6-9fb8-0002a5d5c51b");
const uint8_t SwitchBotController::pressCommand[] = {0x57, 0x01, 0x00};
const size_t SwitchBotController::pressCommandLength = sizeof(pressCommand);


// SwitchBotController の実装
SwitchBotController::SwitchBotController(const char* deviceAddress) 
    : addressString(deviceAddress), pClient(nullptr), pRemoteCharacteristic(nullptr), 
      deviceConnected(false), scanCompleted(false) {
    targetAddress = NimBLEAddress(std::string(deviceAddress), BLE_ADDR_RANDOM);
    Serial.printf("SwitchBotController created for device: %s\n", deviceAddress);
}

SwitchBotController::~SwitchBotController() {
    if (pClient != nullptr && pClient->isConnected()) {
        pClient->disconnect();
    }
    // NimBLEClientは自動的に管理されるため、deleteは不要
    pClient = nullptr;
}

bool SwitchBotController::connectAndSendCommand(uint8_t* command, size_t commandLength) {
    Serial.println("Connecting to SwitchBot...");

    // クライアント作成
    pClient = NimBLEDevice::createClient();
    Serial.println("Created BLE client");

    try {
        // デバイスに接続
        if (!pClient->connect(targetAddress)) {
            Serial.println("Failed to connect to device");
            return false;
        }

        Serial.println("Connected to SwitchBot!");
        deviceConnected = true;

        // サービス取得
        NimBLERemoteService* pRemoteService = pClient->getService(serviceUUID);
        if (pRemoteService == nullptr) {
            Serial.println("Failed to find service");
            pClient->disconnect();
            return false;
        }
        Serial.println("Found service");

        // キャラクタリスティック取得
        pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
        if (pRemoteCharacteristic == nullptr) {
            Serial.println("Failed to find characteristic");
            pClient->disconnect();
            return false;
        }
        Serial.println("Found characteristic");

        // コマンド送信
        Serial.println("Sending command...");
        pRemoteCharacteristic->writeValue(command, commandLength);
        Serial.println("Command sent successfully!");

        // 少し待機
        delay(1000);

        // 切断
        pClient->disconnect();
        deviceConnected = false;
        Serial.println("Disconnected");

        return true;

    } catch (std::exception& e) {
        Serial.print("Exception occurred: ");
        Serial.println(e.what());
        if (pClient->isConnected()) {
            pClient->disconnect();
        }
        return false;
    }
}


bool SwitchBotController::press() {
    Serial.printf("=== SwitchBot Press Operation for %s ===\n", addressString.c_str());

    // 直接接続してコマンド送信
    return connectAndSendCommand(const_cast<uint8_t*>(pressCommand), pressCommandLength);
}

bool SwitchBotController::isConnected() const {
    return deviceConnected;
}

const char* SwitchBotController::getAddress() const {
    return addressString.c_str();
}

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
SwitchBotController::SwitchBotController() 
    : pClient(nullptr), pRemoteCharacteristic(nullptr), deviceConnected(false) {
    Serial.println("SwitchBotController created");
}

SwitchBotController::~SwitchBotController() {
    if (pClient != nullptr) {
        if (pClient->isConnected()) {
            pClient->disconnect();
        }
        NimBLEDevice::deleteClient(pClient);
        pClient = nullptr;
    }
}

bool SwitchBotController::connectAndSendCommand(const char* deviceAddress, uint8_t* command, size_t commandLength) {
    Serial.printf("Connecting to SwitchBot: %s\n", deviceAddress);

    // デバイスアドレスを作成
    NimBLEAddress targetAddress = NimBLEAddress(std::string(deviceAddress), BLE_ADDR_RANDOM);

    // 既存のクライアントがあれば切断して削除
    if (pClient != nullptr) {
        if (pClient->isConnected()) {
            pClient->disconnect();
        }
        NimBLEDevice::deleteClient(pClient);
        pClient = nullptr;
    }

    // 新しいクライアント作成
    pClient = NimBLEDevice::createClient();
    if (pClient == nullptr) {
        Serial.println("Failed to create BLE client");
        return false;
    }
    Serial.println("Created BLE client");

    try {
        // デバイスに接続
        if (!pClient->connect(targetAddress)) {
            Serial.println("Failed to connect to device");
            NimBLEDevice::deleteClient(pClient);
            pClient = nullptr;
            return false;
        }

        Serial.println("Connected to SwitchBot!");
        deviceConnected = true;

        // サービス取得
        NimBLERemoteService* pRemoteService = pClient->getService(serviceUUID);
        if (pRemoteService == nullptr) {
            Serial.println("Failed to find service");
            pClient->disconnect();
            NimBLEDevice::deleteClient(pClient);
            pClient = nullptr;
            deviceConnected = false;
            return false;
        }
        Serial.println("Found service");

        // キャラクタリスティック取得
        pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
        if (pRemoteCharacteristic == nullptr) {
            Serial.println("Failed to find characteristic");
            pClient->disconnect();
            NimBLEDevice::deleteClient(pClient);
            pClient = nullptr;
            deviceConnected = false;
            return false;
        }
        Serial.println("Found characteristic");

        // コマンド送信
        Serial.println("Sending command...");
        pRemoteCharacteristic->writeValue(command, commandLength);
        Serial.println("Command sent successfully!");

        // 少し待機
        delay(1000);

        // 切断とクリーンアップ
        pClient->disconnect();
        NimBLEDevice::deleteClient(pClient);
        pClient = nullptr;
        deviceConnected = false;
        Serial.println("Disconnected");

        return true;

    } catch (std::exception& e) {
        Serial.print("Exception occurred: ");
        Serial.println(e.what());
        if (pClient != nullptr && pClient->isConnected()) {
            pClient->disconnect();
        }
        if (pClient != nullptr) {
            NimBLEDevice::deleteClient(pClient);
            pClient = nullptr;
        }
        deviceConnected = false;
        return false;
    }
}


bool SwitchBotController::press(const char* deviceAddress) {
    Serial.printf("=== SwitchBot Press Operation for %s ===\n", deviceAddress);

    // 直接接続してコマンド送信
    return connectAndSendCommand(deviceAddress, const_cast<uint8_t*>(pressCommand), pressCommandLength);
}

bool SwitchBotController::isConnected() const {
    return deviceConnected;
}

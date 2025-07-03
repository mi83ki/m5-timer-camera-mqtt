/**
 * @file SwitchBotController.h
 * @brief SwitchBot制御用クラス
 * 
 * @details 個別のSwitchBotデバイスを制御するためのクラス
 *          各デバイスごとにインスタンスを作成して使用する
 */

#pragma once

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

/**
 * @class SwitchBotController
 * @brief SwitchBotデバイス制御クラス
 */
class SwitchBotController {
private:
    // SwitchBot BLE設定
    static const BLEUUID serviceUUID;
    static const BLEUUID charUUID;
    static const uint8_t pressCommand[];
    static const size_t pressCommandLength;
    
    // デバイス固有の設定
    BLEAddress targetAddress;
    std::string addressString;
    
    // BLE接続関連
    BLEClient* pClient;
    BLERemoteCharacteristic* pRemoteCharacteristic;
    bool deviceConnected;
    bool scanCompleted;
    
    // プライベートメソッド
    bool connectAndSendCommand(uint8_t* command, size_t commandLength);
    bool scanForSwitchBot();
    
    // スキャン結果処理用内部クラス
    class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    private:
        SwitchBotController* controller;
        
    public:
        AdvertisedDeviceCallbacks(SwitchBotController* ctrl);
        void onResult(BLEAdvertisedDevice advertisedDevice) override;
    };
    
public:
    /**
     * @brief コンストラクタ
     * @param deviceAddress 制御対象のSwitchBotのMACアドレス
     */
    SwitchBotController(const char* deviceAddress);
    
    /**
     * @brief デストラクタ
     */
    ~SwitchBotController();
    
    /**
     * @brief SwitchBotのプレス操作を実行
     * @return 成功時true、失敗時false
     */
    bool press();
    
    /**
     * @brief デバイスの接続状態を取得
     * @return 接続中の場合true、未接続の場合false
     */
    bool isConnected() const;
    
    /**
     * @brief 制御対象デバイスのアドレスを取得
     * @return MACアドレス文字列
     */
    const char* getAddress() const;
};

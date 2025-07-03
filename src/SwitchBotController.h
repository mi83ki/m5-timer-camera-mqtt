/**
 * @file SwitchBotController.h
 * @brief SwitchBot制御用クラス
 *
 * @details 複数のSwitchBotデバイスを1つのインスタンスで制御するクラス
 *          メモリ効率を重視した設計
 */

#pragma once

#include <Log.h>
#include <NimBLEDevice.h>

/**
 * @class SwitchBotController
 * @brief SwitchBotデバイス制御クラス
 */
class SwitchBotController {
 private:
  // SwitchBot BLE設定
  static const NimBLEUUID serviceUUID;
  static const NimBLEUUID charUUID;
  static const uint8_t pressCommand[];
  static const size_t pressCommandLength;

  // BLE接続関連
  NimBLEClient* pClient;
  NimBLERemoteCharacteristic* pRemoteCharacteristic;
  bool deviceConnected;

  // プライベートメソッド
  bool connectAndSendCommand(const char* deviceAddress, uint8_t* command,
                             size_t commandLength);

 public:
  /**
   * @brief コンストラクタ
   */
  SwitchBotController();

  /**
   * @brief デストラクタ
   */
  ~SwitchBotController();

  /**
   * @brief SwitchBotのプレス操作を実行
   * @param deviceAddress 制御対象のSwitchBotのMACアドレス
   * @return 成功時true、失敗時false
   */
  bool press(const char* deviceAddress);

  /**
   * @brief デバイスの接続状態を取得
   * @return 接続中の場合true、未接続の場合false
   */
  bool isConnected() const;
};

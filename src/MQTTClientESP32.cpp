/**
 * @file MQTTClientESP32.h
 * @brief ESP32用MQTT接続クラス
 * @author Tatsuya Miyazaki
 * @date 2024/8/19
 *
 * @details MQTTへの接続を管理するクラス
 */

#include "MQTTClientESP32.h"
#include "Log.h"

/**
 * @brief Construct a new MQTTClientESP32::MQTTClientESP32 object
 *
 * @param mqttHost MQTTブローカーのIPアドレス
 * @param mqttPort MQTTブローカーのポート番号
 * @param callback サブスクライブのコールバック
 */
MQTTClientESP32::MQTTClientESP32(String mqttHost, uint16_t mqttPort)
    : _mqttHost(mqttHost), _mqttPort(mqttPort), _lastReconnectAttempt(0),
      _wifiClient(WiFiClient()), _mqttClient(_wifiClient) {

  logger.info("Start example of MQTTClientESP32 " + _mqttHost + ":" +
                 String(_mqttPort));
  _mqttClient.setServer(_mqttHost.c_str(), _mqttPort);
  // if (callback != nullptr) {
  //   _mqttClient.setCallback(callback);
  // }

  // ランダム関数の初期化
  randomSeed(micros());
}

/**
 * @brief Destroy the MQTTClientESP32::MQTTClientESP32 object
 *
 */
MQTTClientESP32::~MQTTClientESP32() {}

bool MQTTClientESP32::reconnect() {
  String clientId = "ESP8266Client-" + String(random(0xffff), HEX);
  if (_mqttClient.connect(clientId.c_str())) {
    // // Once connected, publish an announcement...
    // _mqttClient.publish("outTopic","hello world");
    // // ... and resubscribe
    // _mqttClient.subscribe("inTopic");
  }
  return _mqttClient.connected();
}

bool MQTTClientESP32::healthCheck(void) {
  if (!_mqttClient.connected()) {
    long now = millis();
    if (now - _lastReconnectAttempt > MQTT_RECONNECT_INTERVAL) {
      _lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        _lastReconnectAttempt = 0;
        return true;
      }
    }
  } else {
    // Client connected
    _mqttClient.loop();
    return true;
  }
  return false;
}

bool MQTTClientESP32::publish(String topic, String payload) {
  return _mqttClient.publish(topic.c_str(), payload.c_str());
}

bool MQTTClientESP32::subscribe(String topic) {
  return _mqttClient.subscribe(topic.c_str());
}

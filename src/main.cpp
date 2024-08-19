/**
 * @file main.cpp
 * @brief M5TimerCameraの画像をMQTTでPubする
 * @date 2024-08-19
 *
 */

#include "Log.h"
#include "M5TimerCAM.h"
#include "MQTTClientESP32.h"
#include "Timer.h"
#include "WiFiESP32.h"
#include "base64.hpp"
#include "config.h"

WiFiESP32 wifi = WiFiESP32(WIFI_SSID, WIFI_PASSWORD);
MQTTClientESP32 *mqttClient;
unsigned char base64Image[32768];
Timer cameraTimer(200);

/**
 * @brief Get the Default Mac Address object
 *
 * @param sepChar 区切り文字(デフォルトは":")
 * @return String MACアドレス
 */
String getDefaultMacAddress(String sepChar = ":") {
  String mac = "";
  unsigned char mac_base[6] = {0};
  if (esp_efuse_mac_get_default(mac_base) == ESP_OK) {
    char buffer[18]; // 6*2 characters for hex + 5 characters for colons + 1
                     // character for null terminator
    sprintf(buffer, "%02x%s%02x%s%02x%s%02x%s%02x%s%02x", mac_base[0], sepChar,
            mac_base[1], sepChar, mac_base[2], sepChar, mac_base[3], sepChar,
            mac_base[4], sepChar, mac_base[5]);
    mac = buffer;
  }
  return mac;
}

/**
 * @brief 初期化
 *
 */
void setup() {
  TimerCAM.begin();

  if (!TimerCAM.Camera.begin()) {
    logger.error("Camera Init Fail");
    return;
  }
  logger.info("Camera Init Success");
  TimerCAM.Camera.sensor->set_pixformat(TimerCAM.Camera.sensor, PIXFORMAT_JPEG);
  TimerCAM.Camera.sensor->set_framesize(TimerCAM.Camera.sensor, FRAMESIZE_VGA);
  TimerCAM.Camera.sensor->set_vflip(TimerCAM.Camera.sensor, 1);
  TimerCAM.Camera.sensor->set_hmirror(TimerCAM.Camera.sensor, 0);

  delay(3000);
  logger.info(
      "MAC address of Wi-Fi Station (using 'esp_efuse_mac_get_default'): " +
      getDefaultMacAddress("-"));

  // WiFi接続
  if (!wifi.begin()) {
    logger.info("WiFi Init Fail");
    ESP.restart();
  }
  mqttClient = new MQTTClientESP32(MQTT_HOST, MQTT_PORT, MQTT_BUFFER_SIZE);
}

/**
 * @brief 繰り返し処理
 *
 */
void loop() {
  static uint32_t last = millis();

  if (wifi.healthCheck() && mqttClient->healthCheck()) {
    if (cameraTimer.isCycleTime() && TimerCAM.Camera.get()) {
      uint32_t now = millis();
      float fps = 1000.0f / (float)(now - last);
      last = now;
      unsigned int base64Length = encode_base64(
          TimerCAM.Camera.fb->buf, TimerCAM.Camera.fb->len, base64Image);
      String pubTopic =
          "m5timercamera/1234/" + getDefaultMacAddress("-") + "/image";
      String payload = "{\"data\":\"" + String((char *)base64Image) + "\"}";
      mqttClient->publish(pubTopic, payload);
      logger.debug("send topic: " + pubTopic + ", base64 length: " +
                   String(base64Length) + ", fps: " + String(fps, 2));
      TimerCAM.Camera.free();
    }
  }
}

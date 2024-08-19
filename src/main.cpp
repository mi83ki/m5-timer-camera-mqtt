/**
 * @file main.cpp
 * @brief M5TimerCameraの画像をMQTTでPubする
 * @date 2024-08-19
 *
 */

#include "Log.h"
#include "M5TimerCAM.h"
#include "MQTTClientESP32.h"
#include "WiFiESP32.h"
#include "base64.hpp"
#include "config.h"

WiFiESP32 wifi = WiFiESP32(WIFI_SSID, WIFI_PASSWORD);
MQTTClientESP32 *mqttClient;
unsigned char base64Image[MQTT_BUFFER_SIZE];

/**
 * @brief 初期化
 *
 */
void setup() {
  TimerCAM.begin();

  if (!TimerCAM.Camera.begin()) {
    Serial.println("Camera Init Fail");
    return;
  }
  Serial.println("Camera Init Success");

  TimerCAM.Camera.sensor->set_pixformat(TimerCAM.Camera.sensor, PIXFORMAT_JPEG);
  TimerCAM.Camera.sensor->set_framesize(TimerCAM.Camera.sensor, FRAMESIZE_QVGA);

  TimerCAM.Camera.sensor->set_vflip(TimerCAM.Camera.sensor, 1);
  TimerCAM.Camera.sensor->set_hmirror(TimerCAM.Camera.sensor, 0);

  // WiFi接続
  if (!wifi.begin()) {
    Serial.println("WiFi Init Fail");
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
  static uint32_t cnt = 0;

  if (wifi.healthCheck() && mqttClient->healthCheck()) {
    if (millis() - last > 1000) {
      last = millis();
      Serial.println("fps: " + String(cnt));
      cnt = 0;
    }
    if (TimerCAM.Camera.get()) {
      cnt++;
      unsigned int base64Length = encode_base64(
          TimerCAM.Camera.fb->buf, TimerCAM.Camera.fb->len, base64Image);
      String pubTopic =
          "m5timercamera/1234/" + mqttClient->getClientId() + "/image";
      mqttClient->publish(pubTopic, (char *)base64Image, base64Length);
      logger.debug("send topic: " + pubTopic +
                   ", base64 length: " + String(base64Length));
      TimerCAM.Camera.free();
    }
  }
}

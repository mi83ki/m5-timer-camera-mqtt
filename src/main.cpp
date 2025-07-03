/**
 * @file main.cpp
 * @brief M5TimerCameraの画像をMQTTでPubする
 * @date 2024-08-19
 *
 */

#include <Log.h>
#include <M5TimerCAM.h>
#include <MQTTClientESP32.h>
#include <MacUtils.h>
#include <NimBLEDevice.h>
#include <Timer.h>
#include <WiFiESP32.h>

#include <base64.hpp>

#include "SwitchBotController.h"
#include "config.h"

WiFiESP32 wifi = WiFiESP32(WIFI_SSID, WIFI_PASSWORD);
MQTTClientESP32 *mqttClient;
unsigned char base64Image[32768];
Timer cameraTimer(500);
/** 画像更新フラグ */
bool isUpdatedImage = false;

// SwitchBotコントローラーのインスタンス
SwitchBotController switchBotController;

/**
 * @brief MQTT送信用Task
 *
 * 画像更新時、MQTTブローカーに Pub する
 *
 * @param[in] pvParameter Taskのパラメータ
 * @return なし
 */
void taskMQTT(void *) {
  mqttClient = new MQTTClientESP32(MQTT_HOST, MQTT_PORT, MQTT_BUFFER_SIZE);
  static uint32_t last = millis();
  while (true) {
    if (wifi.healthCheck() && mqttClient->healthCheck()) {
      if (isUpdatedImage) {
        isUpdatedImage = false;
        uint32_t now = millis();
        float fps = 1000.0f / (float)(now - last);
        last = now;
        String pubTopic =
            "m5timercamera/1234/" + DEFAULT_MAC_ADDRESS + "/image";
        String payload = "{\"data\":\"" + String((char *)base64Image) + "\"}";
        mqttClient->publish(pubTopic, payload);
        logger.debug("send topic: " + pubTopic + ", fps: " + String(fps, 2));
      }
    }
    vTaskDelay(1);
  }
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

  // MQTT送信用Taskを起動 Core 0
  xTaskCreatePinnedToCore(taskMQTT, "taskMQTT", 4096, NULL, 1, NULL, 0);

  delay(3000);
  // BLE初期化
  logger.info("Initializing BLE...");
  NimBLEDevice::init("M5TimerCAM-SwitchBot");
  logger.info("SwitchBot Device 1: " + String(SWITCHBOT_DEVICE_1));
  logger.info("SwitchBot Device 2: " + String(SWITCHBOT_DEVICE_2));
  logger.info("Setup completed");
}

/**
 * @brief 繰り返し処理 Core 1
 *
 */
void loop() {
  static Timer timer = Timer(5000);
  static bool step = false;

  if (cameraTimer.isCycleTime() && TimerCAM.Camera.get()) {
    uint32_t now = millis();
    float fps = 1000.0f / (float)(now - last);
    last = now;
    unsigned int base64Length = encode_base64(
        TimerCAM.Camera.fb->buf, TimerCAM.Camera.fb->len, base64Image);
    isUpdatedImage = true;
    logger.debug("base64 length: " + String(base64Length) +
                 ", fps: " + String(fps, 2));
    TimerCAM.Camera.free();
  }

  if (timer.isCycleTime()) {
    // SwitchBotコントローラーの更新
    if (step) {
      switchBotController.press(SWITCHBOT_DEVICE_1);
      step = false;
    } else {
      switchBotController.press(SWITCHBOT_DEVICE_2);
      step = true;
    }
  }

  vTaskDelay(1);
}

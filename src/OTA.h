#ifdef ESP32
#include <ESPmDNS.h>
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include <ArduinoOTA.h>
#include <WiFiUdp.h>

#if defined(ESP32_RTOS) && defined(ESP32)
void ota_handle(void* parameter) {
  for (;;) {
    ArduinoOTA.handle();
    delay(3500);
  }
}
#endif

void setupOTA(const char* nameprefix, const char* ssid, const char* password, 
              const char* staticIP = nullptr, const char* gateway = nullptr, 
              const char* subnet = nullptr, const char* dns1 = nullptr, 
              const char* dns2 = nullptr) {
  // Configure the hostname
  uint16_t maxlen = strlen(nameprefix) + 7;
  char* fullhostname = new char[maxlen];
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(fullhostname, maxlen, "%s-%02x%02x%02x", nameprefix, mac[3], mac[4],
           mac[5]);
  ArduinoOTA.setHostname(fullhostname);
  delete[] fullhostname;

  // Configure and start the WiFi station
  WiFi.mode(WIFI_STA);
  
  // 引数で固定IPの指定がある場合は固定IP設定を適用
  if (staticIP != nullptr) {
    IPAddress ip, gw, sn;
    
    // 必須パラメータの変換
    if (!ip.fromString(staticIP)) {
      Serial.println("Invalid static IP address");
      return;
    }
    
    // ゲートウェイの設定（必須）
    if (gateway != nullptr) {
      if (!gw.fromString(gateway)) {
        Serial.println("Invalid gateway address");
        return;
      }
    } else {
      Serial.println("Gateway address is required for static IP");
      return;
    }
    
    // サブネットマスクの設定（デフォルト値を使用）
    if (subnet != nullptr) {
      if (!sn.fromString(subnet)) {
        Serial.println("Invalid subnet mask");
        return;
      }
    } else {
      sn.fromString("255.255.255.0"); // デフォルトのサブネットマスク
    }
    
    // DNS設定（オプション）
    if (dns1 != nullptr && dns2 != nullptr) {
      IPAddress dns_1, dns_2;
      if (dns_1.fromString(dns1) && dns_2.fromString(dns2)) {
        WiFi.config(ip, gw, sn, dns_1, dns_2);
        Serial.println("Static IP with dual DNS configured");
      } else {
        WiFi.config(ip, gw, sn);
        Serial.println("Static IP configured (invalid DNS, using router's DNS)");
      }
    } else if (dns1 != nullptr) {
      IPAddress dns_1;
      if (dns_1.fromString(dns1)) {
        WiFi.config(ip, gw, sn, dns_1);
        Serial.println("Static IP with primary DNS configured");
      } else {
        WiFi.config(ip, gw, sn);
        Serial.println("Static IP configured (invalid DNS, using router's DNS)");
      }
    } else {
      WiFi.config(ip, gw, sn);
      Serial.println("Static IP configured (using router's DNS)");
    }
    
    Serial.print("IP: "); Serial.println(ip);
    Serial.print("Gateway: "); Serial.println(gw);
    Serial.print("Subnet: "); Serial.println(sn);
    if (dns1 != nullptr) {
      Serial.print("DNS1: "); Serial.println(dns1);
    }
    if (dns2 != nullptr) {
      Serial.print("DNS2: "); Serial.println(dns2);
    }
  } else {
    Serial.println("Using DHCP (dynamic IP)");
  }
  
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.onStart([]() {
    // NOTE: make .detach() here for all functions called by Ticker.h library -
    // not to interrupt transfer process in any way.
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else  // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using
    // SPIFFS.end()
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("\nAuth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("\nBegin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("\nConnect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("\nReceive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("\nEnd Failed");
  });

  ArduinoOTA.begin();

  Serial.println("OTA Initialized");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

#if defined(ESP32_RTOS) && defined(ESP32)
  xTaskCreate(ota_handle,   /* Task function. */
              "OTA_HANDLE", /* String with name of task. */
              10000,        /* Stack size in bytes. */
              NULL,         /* Parameter passed as input of the task */
              1,            /* Priority of the task. */
              NULL);        /* Task handle. */
#endif
}

# m5-timer-camera-mqtt

M5Stack Timer-CAMで撮影したJPEG画像をBase64形式に変換し、MQTTブローカーへ定期送信するPlatformIOプロジェクトです。

## 主な機能

- Timer-CAMによるJPEG撮影
- 撮影画像のBase64エンコード
- Wi-Fi経由のMQTT送信
- ArduinoOTAによる無線書き込み
- NimBLEによるBLE初期化およびSwitchBot制御用クラス

## 必要なもの

- M5Stack Timer-CAM
- MQTTブローカー
- PlatformIO（VS Code拡張またはCLI）
- 接続先Wi-Fi

## セットアップ

1. Gitサブモジュールを初期化します。

   ```bash
   git submodule update --init
   ```

2. 設定ファイルを作成します。

   ```text
   include/config.h.example → include/config.h
   ```

3. `include/config.h`を環境に合わせて編集します。

   - `WIFI_SSID` / `WIFI_PASSWORD`: Wi-Fi設定
   - `MQTT_HOST` / `MQTT_PORT`: MQTTブローカー設定
   - `MQTT_BUFFER_SIZE`: MQTTバッファサイズ
   - `CAMERA_FRAME_SIZE`: カメラ解像度
   - `USE_STATIC_IP`、`STATIC_IP`、`GATEWAY`、`SUBNET`: 固定IPを使う場合の設定

   `config.h`には認証情報が含まれるため、リポジトリへコミットしないでください。

## MQTT仕様

画像は次のトピックへPublishされます。

```text
m5timercamera/1234/{DEFAULT_MAC_ADDRESS}/image
```

ペイロードはBase64化したJPEG画像を含むJSONです。

```json
{"data":"<Base64 encoded JPEG>"}
```

## ビルドと書き込み

```bash
pio run
pio run --target upload
pio device monitor
```

シリアルモニターの通信速度は`115200bps`です。VS CodeではPlatformIOのビルド・アップロード・モニター機能も利用できます。

## OTA更新

`include/config.h`に固定IPを設定している場合は、`STATIC_IP`、`GATEWAY`、`SUBNET`を環境に合わせて指定します。初回はUSB経由で書き込み、デバイスがネットワークへ接続された後にOTAを利用できます。

PlatformIOの環境設定で次の項目を有効化し、IPアドレスを変更してください。

```ini
upload_protocol = espota
upload_port = <デバイスのIPアドレス>
```

その後、次のコマンドで書き込みます。

```bash
pio run --target upload
```

## ライセンス

MIT License

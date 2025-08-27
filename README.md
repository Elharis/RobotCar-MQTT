# RobotCar-MQTT — STM32 ↔ ESP32 UART + MQTT Köprüsü

> STM32 (motor + sensör) seri hat üzerinden **ESP32**’ye veri gönderir; ESP32 bu veriyi **MQTT**’ye yayınlar ve MQTT’den gelen komutları satır-sonlu (`\n`) olarak STM32’ye iletir.  
> **Failsafe:** Komut yoksa **500 ms → STOP**; mesafe < **50 mm → STOP + buzzer**.  
> **UART:** 115200-8N1 • **Seviye:** 3.3V TTL • **Ortak GND şart**

![System overview](system_overview.png)

---

## İçindekiler
- [Özellikler](#özellikler)
- [Donanım & Pinout](#donanım--pinout)
- [Mimari & Görevler](#mimari--görevler)
- [MQTT Konuları (Topics)](#mqtt-konuları-topics)
- [Dizin Yapısı](#dizin-yapısı)
- [Kurulum & Derleme](#kurulum--derleme)
  - [STM32 (CubeIDE)](#stm32-cubeide)
  - [ESP32 (ESP-IDF)](#esp32-esp-idf)
- [Test Planı](#test-planı)
- [Sorun Giderme](#sorun-giderme)
- [Yol Haritası](#yol-haritası)
- [Lisans](#lisans)

---

## Özellikler
- STM32 (PWM motor + sensör) ↔ ESP32 (Wi-Fi/MQTT) seri köprü
- **LWT:** `robot/status = offline` (bağlanınca `online`)
- **Telemetri rate-limit:** ~10 Hz
- Satır sonu standardı: **`\n`** (her iki uçta da)
- Wi-Fi & MQTT otomatik yeniden bağlanma

---

## Donanım & Pinout
- **UART:**  
  - STM32 **USART2_TX = PD5 → ESP32 RX = GPIO16**  
  - STM32 **USART2_RX = PD6 ← ESP32 TX = GPIO17**  
- **STM32 ek pinler:**  
  Yön: **PA1–PA4** • PWM: **PA5 (TIM2_CH1)** & **PC6 (TIM3_CH1)** •  
  **HC-SR04:** TRIG=**PD10**, ECHO=**PD12** • **Buzzer:** **PA0**

> CubeMX pin görünümü:  
> ![STM32F407 Pinout](docs/pinout_stm32f407.png)

---


**ESP32 görevleri:**
- `wifi_task` — STA bağlan/yeniden bağlan  
- `mqtt_task` — Broker bağlantısı, LWT `robot/status`  
- `uart_rx_task` — STM32’den gelen `SENS,...\n` satırlarını `robot/telemetry`’e publish (10 Hz)  
- `uart_tx_task` — `robot/command` payload’unu `\n` ile UART’a yaz

> ESP-IDF konsol/log görünümü:  
> ![ESP32 console](docs/esp32_console.png)

---

## MQTT Konuları (Topics)
- **Komut:** `robot/command`  Örn: `FORWARD:600`  
- **Telemetri:** `robot/telemetry`  Örn: `SENS,ts=...,dist_mm=...`  
- **Durum:** `robot/status` → `online` / `offline` (LWT)

> Adafruit IO feed örneği:  
> ![Adafruit IO feed](docs/adafruit_feed_robotcar_commands.png)

---

## Dizin Yapısı
```text
.
├─ esp32/
│  ├─ main/                 # app_main.c, app_mqtt.c/.h, uart.c/.h, wifi_connect.c
│  └─ CMakeLists.txt
├─ stm32/
│  ├─ main.c
│  ├─ motor_control.c
│  └─ motor_control.h
├─ docs/
│  ├─ pinout_stm32f407.png
│  ├─ esp32_console.png
│  └─ adafruit_feed_robotcar_commands.png
├─ system_overview.png
├─ .gitattributes
└─ README.md


## Mimari & Görevler

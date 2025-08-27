# RobotCar-MQTT — STM32 ↔ ESP32 (Komut Köprüsü)

## 1) Sistem Açıklaması & Yazılım Mimarisi
Bu proje **komut-temelli** çalışır: **ESP32**, MQTT’den aldığı **sayısal** payload’ları (`"1"`, `"2"`, `"3"` …) **UART1 (115200-8N1, `\n`)** ile **STM32**’ye iletir; **STM32** gelen numarayı **motor fonksiyonlarına** çevirir. Bu sürümde **sensör/telemetri yoktur** (yalnızca **MQTT → UART → Motor** akışı).

**ESP32 (ESP-IDF) modülleri**
- `wifi_connect.[ch]` → STA bağlan/yeniden bağlan (SSID/PASS NVS)
- `app_mqtt.[ch]` → MQTT client (LWT; subscribe/publish API)
- `uart.[ch]` → UART1 satır-sonlu yazım (`\n`)
- `app_main.c` → görevlerin başlatılması

**Görevler**
- `wifi_task` → Wi-Fi bağlan/yeniden bağlan
- `mqtt_task` → Broker’a bağlan; **LWT**: `robot/status=offline` (bağlanınca `online`)
- `uart_tx_task` → `robotcar.commands` (veya `robot/command`) topic’inden gelen **payload’u aynen** `\n` ile STM32’ye yazar
- `uart_rx_task` → **Bu sürümde kullanılmıyor** (telemetri yok)

**STM32 tarafı (özet)**
- `main.c` / `motor_control.c` sayısal komutu yorumlar ve ilgili **motor yön/hız** fonksiyonunu çağırır.
- Örnek eşleme (kendi tablonla değiştir):
  - `1` → ileri, `2` → geri, `3` → sol, `4` → sağ, `5` → dur

---

## 2) Sistem Genel Bakış & Adafruit IO
![Genel görünüm](system_overview.png)

**Adafruit IO Pano**
- **Feed:** `robotcar.commands`
- **Payload:** tam sayı metni (`"1"`, `"2"`, …). ESP32 → UART → STM32.
- **Örnek akış:** `Adafruit IO ► MQTT ► ESP32 ► UART ► STM32 ► Motor`

![Adafruit IO feed](docs/adafruit_feed_robotcar_commands.png)

> Notlar:
> - Sadece **tek yön** (MQTT→UART) vardır; `uart_rx_task` pasif.
> - İstersen `robot/status` topic’iyle **online/offline** durumu yayınlanır (LWT).

---

## 3) Donanımsal Bileşenler & Bağlantılar
**Bileşenler**
- **STM32F407VGT6 (LQFP100)**
- **ESP32** (UART1 kullanılacak)
- Motor sürücü + motor(lar)
- 3.3 V regülatör ve baypas kapasitörleri

**Seviye & Toprak**
- Her iki uç **3.3 V TTL**; **ortak GND şart**.

**UART hattı**
- STM32 **USART2_TX = PD5  →  ESP32 RX = GPIO16**
- STM32 **USART2_RX = PD6  ←  ESP32 TX = GPIO17**
- Baud: **115200-8N1**, satır sonu: **`\n`**

**Motor pinleri (örnek)**
- Yön: **PA1–PA4**
- PWM: **PA5 (TIM2_CH1)** ve/veya **PC6 (TIM3_CH1)**

![STM32 Pinout](docs/pinout_stm32f407.png)

**Pratik ipuçları**
- ESP32 için 3.3 V regülatörde **ani akım** payı bırak (≥ 500–700 mA burst).
- UART kablolarını kısa tut; gerekirse GND ile bükümlü çift yap.
- Satır sonu sözleşmesini iki uçta da **`\n`** olarak sabitle.

---

## 4) Projeden Faydalandıklarım
- **Basit sayısal komut protokolü** ile hızlı ve sağlam bir köprü kurulabileceğini gördüm.
- MQTT tarafında **topic tasarımı** + **LWT** kullanımıyla sistem durumu net izleniyor.
- **Satır-sonlu** veri sözleşmesi olmadan UART üzerinde ayrıştırmanın zorlaştığını deneyimledim.
- Tek yön akış (MQTT→UART) ile **minimum karmaşıklık** ve düşük gecikme elde edildi.

---

## 5) Gelecekteki İyileştirmeler (genel)
- **RTOS düzeni:** Wi-Fi, MQTT, UART-TX için ayrı görevler; **queue** ile besleme, öncelik ve çekirdek pinleme.
- **DMA:** STM32’de **USART2 DMA (ReceiveToIdle)** hazırla (ileride telemetri eklersen CPU yükü düşer).
- **Enerji:** ESP32 **light-sleep**, STM32 **STOP/LP-RUN**; LED’leri GPIO-hold ile kapat.
- **OTA:** ESP32 için **A/B slot** OTA + SHA-256; güncelleme sırasında `robot/status=updating`.
- **Protokol:** Gerekirse **ACK/NACK** + basit **CRC**; komut tablosunu JSON/NVS ile **konfigüre edilebilir** yap.
- **Gözlemlenebilirlik:** `robot/health` ping’i; reconnect, yazılan UART satırı sayacı.
- **Opsiyonel telemetri:** İleride hız/akım vb. ölçümler eklemek istersen `robot/telemetry` ile gönder; 10 Hz rate-limit uygula.

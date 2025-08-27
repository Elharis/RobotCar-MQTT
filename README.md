# RobotCar-MQTT — STM32 ↔ ESP32 (Komut Köprüsü)

## 1) Sistem Açıklaması & Yazılım Mimarisi
Bu proje **komut-temelli** bir köprü kurar: **ESP32**, MQTT’den aldığı **sayısal** değerleri (`"1"`, `"2"`, `"3"` …) **UART1 (115200-8N1, `\n`)** ile **STM32**’ye gönderir; **STM32** bu değeri motor fonksiyonuna çevirip motoru sürer.  


**ESP32 (ESP-IDF) tarafı (yazılım)**
- `wifi_connect.[ch]` → Wi-Fi STA bağlan/yeniden bağlan (SSID/PASS NVS)
- `app_mqtt.[ch]` → MQTT istemcisi (LWT; subscribe/publish API)
- `uart.[ch]` → UART1’e satır-sonlu (`\n`) yazım
- `app_main.c` → görevlerin başlatılması
- Aktif görevler: `wifi_task`, `mqtt_task`, **`uart_tx_task`**  
 

**STM32 tarafı (yazılım)**
- `main.c` / `motor_control.c` sayısal komutu yorumlar; **ileri/geri/sol/sağ/dur** gibi fonksiyonları çağırır.
- Komut eşlemesi projeye göre: `1`→İleri, `2`→Geri, `3`→Sol, `4`→Sağ, `5`→Dur (örnek)

---

## 2) Sistem Genel Bakış & Adafruit IO
![Sistem genel bakış](system_overview.png)

**Adafruit IO Pano**
- **Feed:** `robotcar.commands`
- **Payload:** `"1"`, `"2"`, `"3"` … (tam sayı metni)  
- **Akış:** `Adafruit IO ► MQTT ► ESP32 ► UART ► STM32 ► Motor`

![Adafruit IO feed](docs/adafruit_feed_robotcar_commands.png)

---

## 3) ESP / STM ve Diğer Bileşenler — Donanımsal Bağlantılar
**Bileşenler (özet)**
- **ESP32:** Wi-Fi STA + **MQTT istemcisi**, **UART1** üzerinden STM32’ye komut gönderir
- **STM32:** UART komutlarını **motor sürme** fonksiyonlarına çevirir
- **MQTT Broker:** Komutların yayınlandığı/aboneliklerin yapıldığı sunucu

- UART seviyesi **3.3 V TTL**  
- **Ortak GND**(ESP32 ↔ STM32)

**UART hattı (bağlantı)**
- STM32 **USART2_TX = PD5  →  ESP32 RX = GPIO16**
- STM32 **USART2_RX = PD6  ←  ESP32 TX = GPIO17**
- Baud: **115200-8N1**, satır sonu: **`\n`**

**Motor sürme (STM32)**
- Yön pinleri (örnek): **PA1–PA4**
- PWM : **PA5 (TIM2_CH1)** ve **PC6 (TIM3_CH1)**
- `motor_control.c/.h` içinde ileri/geri/sol/sağ/dur fonksiyonları

> STM32F407 pin görünümü:  
> ![STM32 Pinout](docs/pinout_stm32f407.png)

---

## Yazılımda Öğrendiklerim

### UART
- 115200-8N1 satır-sonlu (`\n`) metin protokolü; örnek: `FORWARD:700\n`
- Satır ayrıştırma, giriş doğrulama ve basit geri bildirim (OK/ERR)

### Wi-Fi & MQTT
- STA bağlan/yeniden bağlan (SSID/PASS NVS’te saklama)
- MQTT client + LWT (online/offline), otomatik reconnect
- Konu yapısı: `robot/command`, `robot/telemetry`, `robot/status`
- Gerekirse telemetri için yayın hızını sınırlama

### Motor Sürme
- Komutları ayrıştırıp yön + PWM’e çevirme

---

## 5) Gelecekteki İyileştirmeler 
- **RTOS kullanmak:** Wi-Fi, MQTT ve UART gönderimi için ayrı görevler; görev önceliklerini düzenlemek.
- **DMA kullanmak:** STM32’de UART için DMA alımını eklemek .
- **Enerji tasarrufu eklemek:** ESP32’de light-sleep, STM32’de düşük güç modlarını denemek.
- **OTA eklemek:** ESP32’ye kablosuz güncelleme (A/B slot, basit doğrulama).
- **Protokolü güçlendirmek:** Gerekirse ACK/NACK ve basit bir hata kontrolü eklemek.
- **İsteğe bağlı telemetri:** İleride hız/akım vs. ölçümleri `robot/telemetry` ile göndermek (oran sınırlı).


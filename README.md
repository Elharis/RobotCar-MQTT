# RobotCar-MQTT — STM32 ↔ ESP32 (Komut-Köprüsü)

![Genel görünüm](system_overview.png)

## Projeyi Ayrıntılı Açıklama
Bu proje, **ESP32** üzerinden gelen **sayısal komutları (1,2,3,...)** seri hat (**UART**) ile **STM32**’ye iletir; STM32 de gelen komuta göre **motor fonksiyonlarını çalıştırır**. Bu sürüm **yalnızca komut köprüsü** içerir; **ultrasonik sensör / telemetri yoktur**.

- **Komut formatı:** ASCII sayı + satır sonu  
  Örn: `1\n`, `2\n`, `3\n`  
- **Eşleme (örnek):** *(Kendi eşlemen farklıysa tabloyu değiştir)*

  | Değer | STM32 Aksiyonu (örnek) |
  |------:|------------------------|
  | 1     | `FORWARD`              |
  | 2     | `BACK`                 |
  | 3     | `LEFT`                 |
  | 4     | `RIGHT`                |
  | 5     | `STOP`                 |

- **Bağlantı dayanıklılığı:** ESP32 Wi-Fi + MQTT otomatik yeniden bağlanma.  
- **Durum yayını (opsiyonel):** MQTT **LWT** ile `robot/status = offline/online`.  
- **Not:** `uart_rx_task` **kullanılmıyor** (telemetri yok). Yalnızca **MQTT → UART (tx)** yönü aktiftir.

---


**ESP32 görevleri**
- `wifi_task` — STA bağlan/yeniden bağlan  
- `mqtt_task` — broker’a bağlan, `robotcar.commands` (veya `robot/command`) abonesi  
- `uart_tx_task` — gelen payload’ı **aynen** `\n` ile **UART1**’e yazar  
- `uart_rx_task` — **bu sürümde devre dışı** (telemetri yok)

> ESP-IDF konsol/log görünümü:  
> ![ESP32 console](docs/esp32_console.png)

---

## Adafruit IO Pano
Komutları kolay vermek için **Adafruit IO** üzerinde bir feed/dash kullanılır.

- **Feed adı (örnek):** `robotcar.commands`  
- **Gönderilen değerler:** `1`, `2`, `3`, … *(STM32 bu sayıyı komuta çevirir)*  
- **Dashboard öğeleri:** Butonlar (Momentary/Toggle) veya Slider (tam sayı)  
- **Akış:** `Adafruit IO ► MQTT ► ESP32 ► UART ► STM32 ► Motor`

> Örnek feed ekranı:  
> ![Adafruit IO feed](docs/adafruit_feed_robotcar_commands.png)

---

## Donanımsal Bilgiler
> STM32F407VGT6 (LQFP100) pin görünümü:  
> ![STM32 Pinout](docs/pinout_stm32f407.png)

- **Seviye:** 3.3 V **TTL**, **ortak GND şart**  
- **UART hattı:**
  - STM32 **USART2_TX = PD5 → ESP32 RX = GPIO16**
  - STM32 **USART2_RX = PD6 ← ESP32 TX = GPIO17**
- **Motor pinleri (örnek):**  
  Yön: **PA1–PA4** · PWM: **PA5 (TIM2_CH1)** ve/veya **PC6 (TIM3_CH1)**  
- **İpuçları:** Kısa UART kablosu, 100 nF yakın bypass; ESP32 için 3.3 V regülatörde yeterli **ani akım** (≥ 500–700 mA burst).

---

## Bu Projeden Öğrendiklerim
- **Basit sayısal komut protokolü** ile hızlı ve güvenilir bir köprü kurulabiliyor.  
- MQTT tarafında **topic tasarımı** ve **LWT** kullanımı sistemi gözlemlenebilir kılıyor.  
- Seri hatta **satır sonu (`\n`) sözleşmesi** olmadan veri ayrıştırmanın zorlaştığını gördüm.  
- Komut-yalnız köprüde “**tek yönlü**” akış (MQTT→UART) çoğu durumda yeterli; ACK/telemetri ihtiyacı doğarsa ikinci kanal eklenebilir.

---

## Gelecekteki İyileştirmeler (genel)
- **RTOS düzeni:** Görevleri ayır (Wi-Fi, MQTT, UART-TX), **kuyruk** ile besle; öncelikleri düzenle.  
- **DMA:** STM32 **USART2 DMA (ReceiveToIdle)** ve/veya ESP32 **UART driver** ile tampon yönetimini iyileştir.  
- **Enerji tasarrufu:** ESP32’de **light-sleep**, STM32’de **STOP/LP-RUN**; gösterge LED’lerini kapat.  
- **OTA:** ESP32’de **A/B slot** OTA ve SHA-256 doğrulama; `robot/status=updating`.  
- **Protokol:** İleride gerekirse **ACK/NACK** ve basit **CRC** ekle; komut tablosunu JSON ile yapılandırılabilir yap.  
- **Gözlemlenebilirlik:** MQTT’de `robot/health` ping’i; basit sayaçlar (reconnect, yazılan UART satırı).  
- **Donanım:** Motor sürücü korumaları (akım, termal); kablolamada noise azaltma; (isteğe bağlı) enkoder ekleyip PID.

---

## Dizin Yapısı (özet)
```text
.
├─ esp32/
│  ├─ main/                 # app_main.c, mqtt/uart/wifi kaynakları
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
└─ README.md


## Sistem Genel Bakış

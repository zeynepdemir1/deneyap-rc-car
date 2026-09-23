- **Dallar Arası Geçiş** 
- git checkout main
- git checkout esp32-surumu

# deneyap-rc-car — ESP32 sürümü

WiFi üzerinden kontrol edilen, mesafe sensörlü model araba.

> **Bu dal (`esp32-surumu`) ESP32-WROOM-32 (ESP32-D DevKit) içindir.**
> Deneyap Kart v2 sürümü `main` dalındadır. İki dal farklı pin haritası ve
> farklı LEDC API sürümü kullanır, kod doğrudan taşınamaz.

---

## Donanım

| Parça | Model / Değer |
|---|---|
| Geliştirme kartı | ESP32-WROOM-32 DevKit |
| Motor sürücü | L298N (çift kanal) |
| Mesafe sensörü | HC-SR04 (5V, P versiyonu değil) |
| Motorlar | 2 adet DC motor |
| Pil | 6 × 1.2V 1300mAh NiMH (7.2V nominal, ~7.6V tam şarj) |
| Encoder | 2 × delikli disk + LM393 optik sayıcı modül *(montaj aşamasında)* |

---

## Pin haritası

### L298N motor sürücü

| L298N | ESP32 GPIO | İşlev |
|---|---|---|
| ENA | 25 | Sol motor hız (PWM) |
| IN1 | 26 | Sol motor yön |
| IN2 | 27 | Sol motor yön |
| ENB | 33 | Sağ motor hız (PWM) |
| IN3 | 32 | Sağ motor yön |
| IN4 | 14 | Sağ motor yön |

### HC-SR04

| HC-SR04 | Bağlantı |
|---|---|
| VCC | Breadboard **+** hattı (L298N'in 5V çıkışından) |
| GND | Breadboard **−** hattı |
| Trig | GPIO13 (doğrudan) |
| Echo | GPIO35 (**gerilim bölücü üzerinden**) |

### Encoder (planlanan)

| Modül | Bağlantı |
|---|---|
| VCC | ESP32 `3V3` |
| GND | Breadboard **−** hattı |
| DO | GPIO18 |
| AO | Kullanılmıyor |

---

## Kablolama

### Besleme

```
Pil paketi (+) ──────► L298N 12V klemensi
Pil paketi (−) ──────► L298N GND klemensi
L298N +5V      ──────► Breadboard + hattı ──► HC-SR04 VCC
```

- L298N üzerindeki **5V regülatör jumper'ı takılı kalmalı** — HC-SR04'ün
  beslemesi buradan geliyor.
- **ENA ve ENB jumper'ları çıkarılmalı** — takılıyken PWM ile hız kontrolü
  yapılamaz.

### Ortak toprak (kritik)

```
ESP32 GND      ──────► Breadboard − hattı
L298N GND      ──────► Breadboard − hattı
Breadboard sol − ────► Breadboard sağ −
```

İki ayrı besleme kaynağı olduğu için ortak toprak zorunlu. Olmadan motor
sürücüye giden sinyaller referanssız kalır ve motorlar rastgele davranır.

### Echo gerilim bölücü

HC-SR04 5V ile çalışır ve Echo pininden 5V çıkarır. ESP32'nin GPIO'ları
3.3V'tur, doğrudan bağlanırsa pin zarar görür.

```
HC-SR04 Echo ──► [satır X]
                    │
                  1kΩ
                    │
                 [satır Y] ──► ESP32 GPIO35
                    │
                  2kΩ
                    │
                    ▼
                 − hattı
```

**X ve Y farklı numaralı satırlar olmalı.** Aynı satıra girerlerse direnç
kısa devre olur ve bölücü çalışmaz.

Çıkış gerilimi: 5V × # deneyap-rc-car — ESP32 sürümü

WiFi üzerinden kontrol edilen, mesafe sensörlü model araba.

> **Bu dal (`esp32-surumu`) ESP32-WROOM-32 (ESP32-D DevKit) içindir.**
> Deneyap Kart v2 sürümü `main` dalındadır. İki dal farklı pin haritası ve
> farklı LEDC API sürümü kullanır, kod doğrudan taşınamaz.

---

## Donanım

| Parça | Model / Değer |
|---|---|
| Geliştirme kartı | ESP32-WROOM-32 DevKit |
| Motor sürücü | L298N (çift kanal) |
| Mesafe sensörü | HC-SR04 (5V, P versiyonu değil) |
| Motorlar | 2 adet DC motor |
| Pil | 6 × 1.2V 1300mAh NiMH (7.2V nominal, ~7.6V tam şarj) |
| Encoder | 2 × delikli disk + LM393 optik sayıcı modül *(montaj aşamasında)* |

---

## Pin haritası

### L298N motor sürücü

| L298N | ESP32 GPIO | İşlev |
|---|---|---|
| ENA | 25 | Sol motor hız (PWM) |
| IN1 | 26 | Sol motor yön |
| IN2 | 27 | Sol motor yön |
| ENB | 33 | Sağ motor hız (PWM) |
| IN3 | 32 | Sağ motor yön |
| IN4 | 14 | Sağ motor yön |

### HC-SR04

| HC-SR04 | Bağlantı |
|---|---|
| VCC | Breadboard **+** hattı (L298N'in 5V çıkışından) |
| GND | Breadboard **−** hattı |
| Trig | GPIO13 (doğrudan) |
| Echo | GPIO35 (**gerilim bölücü üzerinden**) |

### Encoder (planlanan)

| Modül | Bağlantı |
|---|---|
| VCC | ESP32 `3V3` |
| GND | Breadboard **−** hattı |
| DO | GPIO18 |
| AO | Kullanılmıyor |

---

## Kablolama

### Besleme

```
Pil paketi (+) ──────► L298N 12V klemensi
Pil paketi (−) ──────► L298N GND klemensi
L298N +5V      ──────► Breadboard + hattı ──► HC-SR04 VCC
```

- L298N üzerindeki **5V regülatör jumper'ı takılı kalmalı** — HC-SR04'ün
  beslemesi buradan geliyor.
- **ENA ve ENB jumper'ları çıkarılmalı** — takılıyken PWM ile hız kontrolü
  yapılamaz.

### Ortak toprak (kritik)

```
ESP32 GND      ──────► Breadboard − hattı
L298N GND      ──────► Breadboard − hattı
Breadboard sol − ────► Breadboard sağ −
```

İki ayrı besleme kaynağı olduğu için ortak toprak zorunlu. Olmadan motor
sürücüye giden sinyaller referanssız kalır ve motorlar rastgele davranır.

### Echo gerilim bölücü

HC-SR04 5V ile çalışır ve Echo pininden 5V çıkarır. ESP32'nin GPIO'ları
3.3V'tur, doğrudan bağlanırsa pin zarar görür.

```
HC-SR04 Echo ──► [satır X]
                    │
                  1kΩ
                    │
                 [satır Y] ──► ESP32 GPIO35
                    │
                  2kΩ
                    │
                    ▼
                 − hattı
```

**X ve Y farklı numaralı satırlar olmalı.** Aynı satıra girerlerse direnç
kısa devre olur ve bölücü çalışmaz.

Çıkış gerilimi: 5V × 2kΩ / 3kΩ ≈ 3.3V

---

## Kod ayarları

`araba_deneyap.ino` başındaki değerler:

| Değişken | Değer | Not |
|---|---|---|
| `pwm_frekans` | 200 Hz | 1000 Hz'de motorlar düşük hızda tork üretemiyordu |
| `pwm_cozunurluk` | 8 bit | duty aralığı 0–255 |
| `hareket_duty` | 220 | 100 değerinde motorlar kalkışta zorlanıp "dıt" sesi çıkarıyordu |
| `engel_esigi` | 40.0 cm | Kör mesafe + fren mesafesi payı dahil |
| `loop()` gecikmesi | 60 ms | HC-SR04 ardışık ölçümler arasında ~60 ms ister |

### Mod değerleri

| Değer | Anlam |
|---|---|
| 0 | DUR (serbest duruş) |
| 1 | İLERİ |
| 2 | GERİ |
| 3 | SOL |
| 4 | SAĞ |
| 5 | Frenli duruş (`yon_ayarla` içinde tanımlı, henüz kullanılmıyor) |

### Engel algılama mantığı

- `mod` → kullanıcının web arayüzünden gönderdiği istek
- `uygulanan_mod` → motorlara gerçekten giden komut, her döngüde yeniden hesaplanır
- Engel varken **sadece ILERI** engellenir; geri ve dönüş komutları serbest
- Engel kalkınca araba kendiliğinden hareketine devam eder
- Ölçüm alınamadığında (`-1`) engel varmış gibi davranılır — güvenli taraf

---

## Kurulum

### Arduino IDE / VS Code ayarları

- Kart: **ESP32 Dev Module** (Espressif paketi, Deneyap paketi değil)
- Port: `/dev/ttyUSB0` (ESP32 DevKit'ler CH340/CP2102 kullanır)
- Baud rate: 115200

> Arduino-ESP32 çekirdeği **3.x** sürümü gerekiyor. Bu sürümde LEDC API'si
> değişti: `ledcSetup` + `ledcAttachPin` yerine tek bir `ledcAttach(pin,
> frekans, cozunurluk)` kullanılıyor ve `ledcWrite` kanal yerine pin alıyor.

### WiFi bilgileri

`secrets.h` dosyası repoda yok (`.gitignore`'da). Kurulum için:

```bash
cp secrets.example.h secrets.h
```

Sonra kendi değerlerini gir. Şifre **en az 8 karakter** olmalı, yoksa
`softAP` ağı açmaz.

### Kullanım

1. Kartı besle, WiFi ağı birkaç saniye içinde yayına girer
2. Telefondan ağa bağlan
3. Tarayıcıda `192.168.4.1` adresine git

---

## Açık konular

- **ESP32'nin pilden beslenmesi çözülmedi.** Kartta `VIN` etiketli pin
  bulunamadı (`VN` besleme pini değil, GPIO39'un alternatif adı — oraya
  gerilim verilmemeli). Kart şu an USB'den besleniyor. Doğru besleme pini
  bulunduğunda L298N'in 5V çıkışından beslenecek.
- Encoder'ların mekanik montajı ve pulse sayma kodu yapılmadı.
- Arka tarafta mesafe sensörü yok, geri hareket kontrolsüz.
- Frenli duruş (`case 5`) tanımlı ama kullanılmıyor.

---

## Bilinen sorunlar

**Kod yüklenemiyor (`exit status 2`)**
- Seri monitör açıksa kapat, aynı portu iki şey tutamaz
- `ls /dev/ttyUSB*` ile portun görünür olduğunu doğrula
- Otomatik reset çalışmazsa: yükleme başlarken `BOOT` butonunu basılı tut,
  "Connecting..." geçtikten sonra bırak

**Seri monitörde çıktı yok**
- Port seçimini kontrol et — `/dev/ttyS0` değil, `/dev/ttyUSB0`
- `setup()` çok hızlı çalışıp biter; monitörü açtıktan sonra `EN`/`RESET`
  butonuna bas

**USB ve dış besleme birlikte**
- ESP32 DevKit'lerin çoğunda USB ile dış besleme arasında koruma yoktur.
  Kod yüklerken dış besleme kablosunu çıkar, pille çalıştırırken USB'yi çıkar.2kΩ / 3kΩ ≈ 3.3V

---

## Kod ayarları

`araba_deneyap.ino` başındaki değerler:

| Değişken | Değer | Not |
|---|---|---|
| `pwm_frekans` | 200 Hz | 1000 Hz'de motorlar düşük hızda tork üretemiyordu |
| `pwm_cozunurluk` | 8 bit | duty aralığı 0–255 |
| `hareket_duty` | 220 | 100 değerinde motorlar kalkışta zorlanıp "dıt" sesi çıkarıyordu |
| `engel_esigi` | 40.0 cm | Kör mesafe + fren mesafesi payı dahil |
| `loop()` gecikmesi | 60 ms | HC-SR04 ardışık ölçümler arasında ~60 ms ister |

### Mod değerleri

| Değer | Anlam |
|---|---|
| 0 | DUR (serbest duruş) |
| 1 | İLERİ |
| 2 | GERİ |
| 3 | SOL |
| 4 | SAĞ |
| 5 | Frenli duruş (`yon_ayarla` içinde tanımlı, henüz kullanılmıyor) |

### Engel algılama mantığı

- `mod` → kullanıcının web arayüzünden gönderdiği istek
- `uygulanan_mod` → motorlara gerçekten giden komut, her döngüde yeniden hesaplanır
- Engel varken **sadece ILERI** engellenir; geri ve dönüş komutları serbest
- Engel kalkınca araba kendiliğinden hareketine devam eder
- Ölçüm alınamadığında (`-1`) engel varmış gibi davranılır — güvenli taraf

---

## Kurulum

### Arduino IDE / VS Code ayarları

- Kart: **ESP32 Dev Module** (Espressif paketi, Deneyap paketi değil)
- Port: `/dev/ttyUSB0` (ESP32 DevKit'ler CH340/CP2102 kullanır)
- Baud rate: 115200

> Arduino-ESP32 çekirdeği **3.x** sürümü gerekiyor. Bu sürümde LEDC API'si
> değişti: `ledcSetup` + `ledcAttachPin` yerine tek bir `ledcAttach(pin,
> frekans, cozunurluk)` kullanılıyor ve `ledcWrite` kanal yerine pin alıyor.

### WiFi bilgileri

`secrets.h` dosyası repoda yok (`.gitignore`'da). Kurulum için:

```bash
cp secrets.example.h secrets.h
```

Sonra kendi değerlerini gir. Şifre **en az 8 karakter** olmalı, yoksa
`softAP` ağı açmaz.

### Kullanım

1. Kartı besle, WiFi ağı birkaç saniye içinde yayına girer
2. Telefondan ağa bağlan
3. Tarayıcıda `192.168.4.1` adresine git

---

## Açık konular

- **ESP32'nin pilden beslenmesi çözülmedi.** Kartta `VIN` etiketli pin
  bulunamadı (`VN` besleme pini değil, GPIO39'un alternatif adı — oraya
  gerilim verilmemeli). Kart şu an USB'den besleniyor. Doğru besleme pini
  bulunduğunda L298N'in 5V çıkışından beslenecek.
- Encoder'ların mekanik montajı ve pulse sayma kodu yapılmadı.
- Arka tarafta mesafe sensörü yok, geri hareket kontrolsüz.
- Frenli duruş (`case 5`) tanımlı ama kullanılmıyor.

---

## Bilinen sorunlar

**Kod yüklenemiyor (`exit status 2`)**
- Seri monitör açıksa kapat, aynı portu iki şey tutamaz
- `ls /dev/ttyUSB*` ile portun görünür olduğunu doğrula
- Otomatik reset çalışmazsa: yükleme başlarken `BOOT` butonunu basılı tut,
  "Connecting..." geçtikten sonra bırak

**Seri monitörde çıktı yok**
- Port seçimini kontrol et — `/dev/ttyS0` değil, `/dev/ttyUSB0`
- `setup()` çok hızlı çalışıp biter; monitörü açtıktan sonra `EN`/`RESET`
  butonuna bas

**USB ve dış besleme birlikte**
- ESP32 DevKit'lerin çoğunda USB ile dış besleme arasında koruma yoktur.
  Kod yüklerken dış besleme kablosunu çıkar, pille çalıştırırken USB'yi çıkar.
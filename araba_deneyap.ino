#include "secrets.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// ESP32D <-> L298N , HC-SR04 baglantisi:
// ENA-25, IN1-26, IN2-27  (sol motor / Kanal A)
// ENB-33, IN3-32, IN4-14  (sag motor / Kanal B)
// Trig-13, Echo-35 (bolucu uzerinden)

uint8_t sol_pwm = 25, sol_in1 = 26, sol_in2 = 27;
uint8_t sag_pwm = 33, sag_in3 = 32, sag_in4 = 14;
uint8_t hcsr_trig = 13, hcsr_echo = 35;

// Donanımsal PWM ayarları — loop() bloklansa bile PWM bağımsız üretilir
const uint32_t pwm_frekans = 200; // Hz
const uint8_t pwm_cozunurluk = 8;  // bit -> duty 0-255
const float engel_esigi = 40.0;
const uint8_t hareket_duty = 220;

uint8_t mod = 0; // 0=DUR,1=ILERI,2=GERI,3=SOL,4=SAG

const char* ap_ssid = WIFI_SSID;
const char* ap_sifre = WIFI_PASSWORD;  // en az 8 karakter olmalı

WebServer server(80);

void yon_ayarla(uint8_t m){
  switch(m){
    case 1: digitalWrite(sol_in1,LOW);  digitalWrite(sol_in2,HIGH); digitalWrite(sag_in3,LOW);  digitalWrite(sag_in4,HIGH); break;
    case 2: digitalWrite(sol_in1,HIGH); digitalWrite(sol_in2,LOW);  digitalWrite(sag_in3,HIGH); digitalWrite(sag_in4,LOW); break;
    case 3: digitalWrite(sol_in1,LOW);  digitalWrite(sol_in2,HIGH); digitalWrite(sag_in3,HIGH); digitalWrite(sag_in4,LOW); break;
    case 4: digitalWrite(sol_in1,HIGH); digitalWrite(sol_in2,LOW);  digitalWrite(sag_in3,LOW);  digitalWrite(sag_in4,HIGH); break;
    case 5: digitalWrite(sol_in1,HIGH); digitalWrite(sol_in2,HIGH); digitalWrite(sag_in3,HIGH); digitalWrite(sag_in4,HIGH); break; //frenleme 
    default: digitalWrite(sol_in1,LOW); digitalWrite(sol_in2,LOW);  digitalWrite(sag_in3,LOW);  digitalWrite(sag_in4,LOW); break;
  }
}

void pwm_guncelle(uint8_t m){
  uint8_t duty = (m == 0) ? 0 : hareket_duty;
  ledcWrite(sol_pwm, duty);
  ledcWrite(sag_pwm, duty);
}

void handleRoot(){
  String html = "<html><body style='text-align:center;font-family:sans-serif;'>"
    "<h2>Araba Kontrol</h2>"
    "<button onclick=\"fetch('/git?m=1')\" style='font-size:24px;padding:20px;'>ILERI</button><br><br>"
    "<button onclick=\"fetch('/git?m=3')\" style='font-size:24px;padding:20px;'>SOL</button>"
    "<button onclick=\"fetch('/git?m=0')\" style='font-size:24px;padding:20px;'>DUR</button>"
    "<button onclick=\"fetch('/git?m=4')\" style='font-size:24px;padding:20px;'>SAG</button><br><br>"
    "<button onclick=\"fetch('/git?m=2')\" style='font-size:24px;padding:20px;'>GERI</button>"
    "</body></html>";
  server.send(200, "text/html", html);
}

void handleGit(){
  if(server.hasArg("m")){
    long deger = server.arg("m").toInt();
    mod = (deger >= 1 && deger <= 4) ? (uint8_t)deger : 0; // gecersiz/eksik komut -> guvenli DUR
  }
  server.send(200, "text/plain", "OK");
}

float mesafe_olc(){
  // 1. trig'i temizle: LOW yap, 2 mikrosaniye bekle
  //    kullanacağın fonksiyonlar: digitalWrite, delayMicroseconds
  digitalWrite(hcsr_trig , LOW);
  delayMicroseconds(2);
  // 2. trig'i 10 mikrosaniye HIGH yap, sonra tekrar LOW yap
  //    aynı iki fonksiyon
  digitalWrite(hcsr_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(hcsr_trig, LOW);

  // 3. echo'nun HIGH kaldığı süreyi ölç, bir değişkene at
  //    unsigned long sure = pulseIn(...);
  unsigned long sure = pulseIn(hcsr_echo , HIGH , 25000);

  // 4. süreyi mesafeye çevir
  //    ses hızı 0.0343 cm/mikrosaniye
  //    DİKKAT: ölçtüğün süre gidiş-dönüş, mesafe tek yön
  float mesafe = (sure* 0.0343)/2 ;
  if(mesafe > 400 || mesafe < 2){
    return -1;
  }
  // 5. mesafeyi return et
  return mesafe;
}


void setup(){
  Serial.begin(115200);
  Serial.println("BASLADI");

  pinMode(sol_in1, OUTPUT); pinMode(sol_in2, OUTPUT);
  pinMode(sag_in3, OUTPUT); pinMode(sag_in4, OUTPUT);
  pinMode(hcsr_echo, INPUT);
  pinMode(hcsr_trig, OUTPUT);

  ledcAttach(sol_pwm, pwm_frekans, pwm_cozunurluk);
  ledcAttach(sag_pwm, pwm_frekans, pwm_cozunurluk);

  yon_ayarla(mod);
  pwm_guncelle(mod);

  WiFi.softAP(ap_ssid, ap_sifre);
  Serial.print("AP IP adresi: ");
  Serial.println(WiFi.softAPIP());  // genelde 192.168.4.1

  server.on("/", handleRoot);
  server.on("/git", handleGit);
  server.begin();  

}

void loop(){
  server.handleClient();
  float mesafe = mesafe_olc();


  uint8_t uygulanan_mod=mod;

  if((mesafe < 0 || mesafe<=engel_esigi) &&mod==1){
      uygulanan_mod=0;
  }
  yon_ayarla(uygulanan_mod);
  pwm_guncelle(uygulanan_mod);
  delay(50);
}


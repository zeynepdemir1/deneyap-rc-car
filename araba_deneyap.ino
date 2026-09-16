#include "secrets.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Deneyap Kart v2 <-> L298N , HC-SR04 baglantisi:
// ENA-D7, IN1-D6, IN2-D5  (sol motor / Kanal A)
// ENB-D0, IN3-D4, IN4-D1  (sag motor / Kanal B)
uint8_t sol_pwm = D7, sol_in1 = D6, sol_in2 = D5;
uint8_t sag_pwm = D0, sag_in3 = D4, sag_in4 = D1;
uint8_t hcsr_trig = D8 , hcsr_echo = D9 ;

// Donanımsal PWM (LEDC) kanalları — loop() bloklansa bile PWM bağımsız üretilir
uint8_t sol_kanal = 0, sag_kanal = 1;
const uint32_t pwm_frekans = 1000; // Hz
const uint8_t pwm_cozunurluk = 8;  // bit -> duty 0-255

uint8_t hareket_duty = 100;
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
    default: digitalWrite(sol_in1,LOW); digitalWrite(sol_in2,LOW);  digitalWrite(sag_in3,LOW);  digitalWrite(sag_in4,LOW); break;
  }
}

void pwm_guncelle(){
  uint8_t duty = (mod == 0) ? 0 : hareket_duty;
  ledcWrite(sol_kanal, duty);
  ledcWrite(sag_kanal, duty);
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
    yon_ayarla(mod);
    pwm_guncelle();
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

  ledcSetup(sol_kanal, pwm_frekans, pwm_cozunurluk);
  ledcSetup(sag_kanal, pwm_frekans, pwm_cozunurluk);
  ledcAttachPin(sol_pwm, sol_kanal);
  ledcAttachPin(sag_pwm, sag_kanal);

  yon_ayarla(mod);
  pwm_guncelle();

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
  if(mesafe < 0){
    Serial.println("olcum yok");
  }
  else{
  Serial.println(mesafe);
  }
  delay(100);
}


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
uint8_t sol_encoder = 18, sag_encoder = 19;

// Donanımsal PWM ayarları — loop() bloklansa bile PWM bağımsız üretilir
const uint32_t pwm_frekans = 200; // Hz
const uint8_t pwm_cozunurluk = 8;  // bit -> duty 0-255
const float engel_esigi = 40.0;
const uint8_t hareket_duty = 220;
const unsigned long encoder_min_aralik = 3000; // us

uint8_t mod = 0; // 0=DUR,1=ILERI,2=GERI,3=SOL,4=SAG

unsigned long onceki_sag = 0;
unsigned long onceki_sol = 0;

volatile unsigned long sol_sayac = 0, sag_sayac = 0;
volatile unsigned long sol_son_zaman = 0, sag_son_zaman = 0;

int duzeltme = 0;
long son_fark = 0;
float son_mesafe = 0;

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
  uint8_t temel = (m == 0) ? 0 : hareket_duty;
  if(m == 0){
    ledcWrite(sol_pwm, 0);
    ledcWrite(sag_pwm, 0);
    return;
  }
  ledcWrite(sol_pwm, constrain(temel + duzeltme, 0, 255));
  ledcWrite(sag_pwm, constrain(temel - duzeltme, 0, 255));
}

void handleRoot(){
  String html =
  "<!DOCTYPE html><html><head><meta charset='utf-8'>"
  "<meta name='viewport' content='width=device-width,initial-scale=1'>"
  "<title>Araba Kontrol</title><style>"
  "*{box-sizing:border-box}"
  "body{margin:0;padding:24px 16px;font-family:system-ui,-apple-system,sans-serif;"
  "background:#13161a;color:#e9edf2}"
  "h2{text-align:center;font-size:19px;font-weight:600;margin:0 0 22px;color:#9aa4b2;"
  "letter-spacing:.5px}"
  ".pad{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;max-width:330px;"
  "margin:0 auto 26px}"
  "button{padding:24px 0;font-size:16px;font-weight:600;border:0;border-radius:16px;"
  "background:#232830;color:#e9edf2;letter-spacing:.5px}"
  "button:active{background:#333a45;transform:scale(.96)}"
  ".dur{background:#b5372a}"
  ".dur:active{background:#cf4335}"
  ".b1{grid-column:2;grid-row:1}"
  ".b2{grid-column:1;grid-row:2}"
  ".b3{grid-column:2;grid-row:2}"
  ".b4{grid-column:3;grid-row:2}"
  ".b5{grid-column:2;grid-row:3}"
  ".grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;max-width:330px;margin:0 auto}"
  ".k{background:#1b1f26;border-radius:14px;padding:14px 16px}"
  ".k.full{grid-column:span 2}"
  ".k b{display:block;font-size:11px;font-weight:600;color:#78828f;"
  "letter-spacing:1px;margin-bottom:5px}"
  ".k span{font-size:26px;font-weight:600;font-variant-numeric:tabular-nums}"
  ".k.full span{font-size:38px}"
  ".u{font-size:15px;color:#78828f;font-weight:500}"
  "</style></head><body>"
  "<h2>ARABA KONTROL</h2>"
  "<div class='pad'>"
  "<button class='b1' onclick='g(1)'>ILERI</button>"
  "<button class='b2' onclick='g(3)'>SOL</button>"
  "<button class='b3 dur' onclick='g(0)'>DUR</button>"
  "<button class='b4' onclick='g(4)'>SAG</button>"
  "<button class='b5' onclick='g(2)'>GERI</button>"
  "</div>"
  "<div class='grid'>"
  "<div class='k full'><b>MESAFE</b><span id='mesafe'>-</span><span class='u'> cm</span></div>"
  "<div class='k'><b>DUZELTME</b><span id='duzeltme'>-</span></div>"
  "<div class='k'><b>FARK</b><span id='fark'>-</span></div>"
  "<div class='k'><b>SOL</b><span id='sol'>-</span></div>"
  "<div class='k'><b>SAG</b><span id='sag'>-</span></div>"
  "</div>"
  "<script>"
  "function g(m){fetch('/git?m='+m);}"
  "setInterval(function(){"
  "fetch('/durum').then(function(r){return r.json();}).then(function(d){"
  "for(var k in d){var e=document.getElementById(k);if(e)e.innerText=d[k];}"
  "document.getElementById('mesafe').innerText=d.mesafe<0?'--':d.mesafe;"
  "});},400);"
  "</script></body></html>";
  server.send(200, "text/html", html);
}

void handleGit(){
  if(server.hasArg("m")){
    long deger = server.arg("m").toInt();
    mod = (deger >= 1 && deger <= 4) ? (uint8_t)deger : 0; // gecersiz/eksik komut -> guvenli DUR
  }
  server.send(200, "text/plain", "OK");
}

void handleDurum(){
  String s = "{\"mesafe\":" + String(son_mesafe, 1) +
             ",\"duzeltme\":" + String(duzeltme) +
             ",\"fark\":" + String(son_fark) +
             ",\"sol\":" + String(sol_sayac) +
             ",\"sag\":" + String(sag_sayac) +
             ",\"mod\":" + String(mod) + "}";
  server.send(200, "application/json", s);
}

void IRAM_ATTR sol_encoder_isr(){
  unsigned long zaman = micros();
  if(zaman - sol_son_zaman >= encoder_min_aralik){
    sol_son_zaman = zaman;
    sol_sayac++;
}
}

void IRAM_ATTR sag_encoder_isr(){
  unsigned long zaman = micros();
  if(zaman- sag_son_zaman >= encoder_min_aralik){
    sag_son_zaman = zaman;
    sag_sayac++;
  }
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
  server.on("/durum", handleDurum);
  server.begin();  

  pinMode(sol_encoder,INPUT);
  attachInterrupt(digitalPinToInterrupt(sol_encoder),sol_encoder_isr,RISING);

  pinMode(sag_encoder,INPUT);
  attachInterrupt(digitalPinToInterrupt(sag_encoder),sag_encoder_isr,RISING);
 
}

void loop(){
  server.handleClient();

  float mesafe = mesafe_olc();
  son_mesafe = mesafe;

  static unsigned long son_yazma = 0;
  if(millis() - son_yazma >= 1000){
    son_yazma = millis();
    Serial.print(sol_sayac);
    Serial.print("  ");
    Serial.println(sag_sayac);
}

  static unsigned long son_olcum = 0;
  if(millis() - son_olcum >= 200){
    son_olcum = millis();

    unsigned long simdi_sol = sol_sayac;
    unsigned long simdi_sag = sag_sayac;
    unsigned long sol_darbe = simdi_sol - onceki_sol;
    unsigned long sag_darbe = simdi_sag - onceki_sag;
    
    long fark = (long)sol_darbe - (long)sag_darbe;
    son_fark = fark;
    Serial.print("fark:");
    Serial.println(fark);
    
    onceki_sol = simdi_sol;
    onceki_sag = simdi_sag;

    if(mod == 1){
      if(fark > 0) duzeltme -= 2;      // sol hızlı, sol duty'yi düşür
      else if(fark < 0) duzeltme += 2; // sag hızlı, sol duty'yi artır
      duzeltme = constrain(duzeltme, -60, 60);
      Serial.println(duzeltme);
}

  }

  uint8_t uygulanan_mod=mod;

  if((mesafe < 0 || mesafe<=engel_esigi) &&mod==1){
      uygulanan_mod=0;
  }
  yon_ayarla(uygulanan_mod);
  pwm_guncelle(uygulanan_mod);

  static unsigned long son_d = 0;
  if(millis() - son_d >= 1000){
    son_d = millis();
    Serial.print("mod=");
    Serial.print(mod);
    Serial.print(" uyg=");
    Serial.print(uygulanan_mod);
    Serial.print(" mesafe=");
    Serial.println(mesafe);
}

  delay(50);
}


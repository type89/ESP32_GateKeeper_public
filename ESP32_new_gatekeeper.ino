#include <WiFi.h>
#include "SSD1306.h"

SSD1306  display(0x3c, 21, 22); //（I2Cアドレス,SDA,SCL）

const char *ssid = "SSID";
const char *password = "パスワード";
String reboot_time = "23:59:59";
const int TRG = 12;
const int ECH = 14;
const int HIGHTIME = 10;
const int LED = 19;


static WiFiUDP wifiUdp; 
static const char *kRemoteIpadr = "raspberryPiのIP";
static const int kRmoteUdpPort = 9000; //送信先のポート
  
void setup() { 
  static const int kLocalPort = 7000;  //自身のポート
  pinMode(TRG, OUTPUT);
  pinMode(ECH, INPUT);  
  pinMode(LED, OUTPUT);
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (WiFi.begin(ssid, password) != WL_DISCONNECTED) {
    ESP.restart();
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  
  wifiUdp.begin(kLocalPort);
  Serial.println("Connected to the WiFi network!");
  
  configTime(9 * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");//NTPの設定
  
  display.init();
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 20, "Wifi Connected"); 
  display.display();
  delay(1000);
}

int measure(){
  
  int diff;
  float dis;
  
  digitalWrite(TRG, HIGH);
  delayMicroseconds(HIGHTIME);
  digitalWrite(TRG, LOW);
  diff = pulseIn(ECH, HIGH);
  dis = (float)diff * 0.1774;
  Serial.println(dis);
  
  if (dis > 1200){
    dis = 800;
  }
  
  int distance = int(dis);
  return distance;
}

void display_time(){
  struct tm timeInfo;//時刻を格納するオブジェクト
  char yy_mm_dd[15];//文字格納用
  char hh_mm_ss[15];//文字格納用
  
  getLocalTime(&timeInfo);//tmオブジェクトのtimeInfoに現在時刻を入れ込む
  
  sprintf(yy_mm_dd, "%04d / %02d / %02d",
          timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday);//人間が読める形式に変換
  sprintf(hh_mm_ss, "%02d:%02d:%02d",
          timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
          
  String buf = hh_mm_ss;
  
  if (buf.equals(reboot_time)){
    display.clear();
    display.drawString(10, 20, "Rebooting..."); 
    display.display();
    delay(1000);
    ESP.restart();
  }
  
  Serial.println(yy_mm_dd);
  Serial.println(hh_mm_ss);
  display.clear();
  //display.drawString(15, 15, yy_mm_dd);
  display.setFont(ArialMT_Plain_16);
  display.drawString(15, 0, yy_mm_dd);
  display.setFont(ArialMT_Plain_24);
  display.drawString(18, 25, hh_mm_ss);
  //display.drawString(30, 35, hh_mm_ss);
  display.display();
}

void detect(int distance){
  Serial.println("Detect!");
  display.clear();
  display.setFont(ArialMT_Plain_24);
  display.drawString(28, 10, "Detect!");
  String dist = "Dist : " + String(distance) + "mm";
  display.setFont(ArialMT_Plain_16);
  display.drawString(20, 40, dist); 
  display.display();
}

void send_udp() {
  wifiUdp.beginPacket(kRemoteIpadr, kRmoteUdpPort);
  wifiUdp.write('d');
  wifiUdp.endPacket(); 
  Serial.println("Send signal to Raspberry PI...");
}

void loop() {
  for(int i=0; i < 10; i++){
    int distance = measure();
       
    if (distance < 900){
      delay(25);
      int distance = measure();

      if (distance < 900){
        delay(25);
        int distance = measure();
        
        if (distance < 900){
          digitalWrite(LED, HIGH);
          detect(distance);
          send_udp();
          delay(2000);
          for(int i=0; i < 300; i++){
            display_time(); 
            delay(100);
            //Serial.println("i ==> " + String(i));
            }
          digitalWrite(LED, LOW);
        }
      }
    }
    delay(100);
  }
  display_time();  
}

#include <ArduinoJson.h>
#include <CTBot.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>

// WiFi Identity
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASS";

// Static IP Address
IPAddress local_IP(192, 168, 0, 10);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(1, 1, 1, 1);
IPAddress secondaryDNS(1, 0, 0, 1);

// UDP Server
WiFiUDP Udp;
unsigned int localUdpPort = 8888;
char incomingPacket[1];
char replyPacket[] = "0";

// Telegram Bot
String token = "X:YYY"; // Token Bot Telegram
uint32_t userId = 000000000; // ID user/group chat
CTBot myBot;

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// LCD & BUZZER
LiquidCrystal_I2C lcd(0x27,16,2);
const int buzzerPin = D3;
bool buzzerPinState = 0;
const int resetBtn = D4;
bool resetBtnState = 1;

void setup() {
  Serial.begin(115200);
  while(!Serial){}
  if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.setTimeOffset(25200);
  
  Udp.begin(localUdpPort);
  Serial.printf("UDP Server listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  myBot.setTelegramToken(token);
  if (myBot.testConnection()){
    Serial.println("\nTelegram Bot Connection OK --- Waiting Messages");
  } else {
    Serial.println("\nTelegram Bot Connection Not OK");
  }
  
  lcd.begin();
  lcd.setBacklight(LOW);
  pinMode(buzzerPin, OUTPUT);
  pinMode(resetBtn, INPUT);
  
  Serial.print("Setup time: ");
  Serial.println(millis());
  Serial.println();
}

void loop() {
  resetBtnState = digitalRead(resetBtn);
  if(resetBtnState == 0){
    resetOutput();
  }
  
  if(buzzerPinState == 1){
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);
    delay(500);
  } else {
    digitalWrite(buzzerPin, LOW);
  }
  
  timeClient.update();
  String reply;
  int packetSize = Udp.parsePacket();
  if(packetSize){
    Serial.print("Packet received time: ");
    Serial.println(millis());
    String buffTime = timeClient.getFormattedTime();    
    Serial.printf("Received %d bytes from %s, port %d at %s\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort(), buffTime);
    
    int len = Udp.read(incomingPacket, 1);
    if (len > 0){
      incomingPacket[len] = 0;
    }
    
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(replyPacket);
    Udp.endPacket();
    Serial.print("Packet replied at: ");
    Serial.println(millis());
    
    Serial.printf("UDP packet contents: %s\n", incomingPacket);
    showOutput(incomingPacket, buffTime);    
  }

  TBMessage msg;
  if(CTBotMessageText == myBot.getNewMessage(msg)){
    if(msg.text.equalsIgnoreCase("/start")){
      reply = (String)"Selamat Datang " + msg.sender.username + (String)".\nSaya adalah Bot Telegram untuk Wireless Nurse Call System";
      myBot.sendMessage(userId, reply);
    }
    else if(msg.text.equalsIgnoreCase("/reset")){
      resetOutput();
    }
    else {
      reply = (String)"Perintah tidak dikenali\nGunakan client node untuk mengirim pesan\nGunakan perintah /start atau /reset";
      myBot.sendMessage(userId, reply);
      Serial.println(reply);
    }
  }
}

void showOutput(String message, String msgTime){
  String line0 = (String)"NODE " + message + (String)" MEMANGGIL";
  String line1 = (String)"PUKUL " + msgTime;
  String telegramMsg = line0 + (String)"\n" + line1;
  
  Serial.println("SHOW LCD & ACTIVATE BUZZER");
  myBot.sendMessage(userId, telegramMsg);
  
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print(line0);
  lcd.setCursor(0,1);
  lcd.print(line1);
  
  buzzerPinState = 1;
  Serial.print("Output time: ");
  Serial.println(millis());
}

void resetOutput(){
  Serial.print("Reset start: ");
  Serial.println(millis());
  String reply = (String)"RESETTING...\nLCD cleared\nBuzzer stopped";
  Serial.println("RESETTING LCD & DISABLE BUZZER");
  lcd.setBacklight(LOW);
  lcd.clear();  
  buzzerPinState = 0;  
  myBot.sendMessage(userId, reply);
  Serial.print("Reset done: ");
  Serial.println(millis());
  Serial.println();
}

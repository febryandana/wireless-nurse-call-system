#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// WIFI CONNECT
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASS";

// STATIC IP ADDRESS
IPAddress local_IP(192, 168, 0, 11);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// UDP CLIENT
WiFiUDP Udp;
char msg[] = "A";
char incomingPacket[1];
unsigned int localUdpPort = 8888;
const char* udpServer = "192.168.0.10";

// BATTERY VOLTAGE LOG
unsigned int raw = 0;
float volt = 0.0;
char msgBat[8];

void setup() {    
  Serial.begin(115200);
  while(!Serial){}    
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }  
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print(".");
  }
  Serial.println("Connected!");  
  Udp.begin(localUdpPort);  

  pinMode(A0, INPUT);
  raw = analogRead(A0);
  volt=raw/1023.0;
  volt=volt*4.2;
  dtostrf(volt,2,4,msgBat);
  Serial.print("Battery voltage: ");
  Serial.println(msgBat);
  
  Serial.print("Setup time: ");
  Serial.println(millis());
}

void loop() {  
  int packetSize = Udp.parsePacket();
  if(packetSize){
    Serial.print("Packet received at: ");
    Serial.println(millis());
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 1);
    if (len > 0){
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);
    espSleep();
  } else {
      Udp.beginPacket(udpServer, localUdpPort);
      Udp.write(msg);
      Udp.endPacket();
      Serial.print("Packet sent at: ");
      Serial.println(millis());
  }
  delay(1000);
}

void espSleep(){
  Serial.print("Time to sleep: ");
  Serial.println(millis());
  ESP.deepSleep(0);
}

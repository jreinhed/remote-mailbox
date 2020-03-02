#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include "SSD1306Wire.h"

#ifndef STASSID
#define STASSID "<insert SSID here>"
#define STAPSK  "<insert pass here>"
#endif

unsigned int localPort = 8888;      // local port to listen on
int btn = D7;
int red = D1;
int green = D2;
volatile bool btn_pressed = false;

// buffer for receiving data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1]; //buffer to hold incoming packet,

WiFiUDP Udp;
SSD1306Wire  display(0x3c, D6, D5);

ICACHE_RAM_ATTR void btn_ISR() {
	btn_pressed = true;
}

void setup() {
  pinMode(btn, INPUT);
  display.init();
  display.setContrast(255);
  
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(btn), btn_ISR, RISING);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("UDP server on port %d\n", localPort);
  Udp.begin(localPort);

}

void loop() {
  if (btn_pressed) { // or signal received from app
    IPAddress mailbox_IP(192, 168, 10, 210);
    Udp.beginPacket(mailbox_IP, 8888);
    Udp.write("?");
    Udp.endPacket();
    display.setLogBuffer(5, 30);
      
    int packetSize = Udp.parsePacket();
    if (packetSize) {
      Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n",
                    packetSize,
                    Udp.remoteIP().toString().c_str(), Udp.remotePort(),
                    Udp.destinationIP().toString().c_str(), Udp.localPort(),
                    ESP.getFreeHeap());
  
      // read the packet into packetBuffer
      int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      packetBuffer[n] = 0;
      
      Serial.println("Contents:");
      Serial.println(packetBuffer);
      display.clear();
      if (packetBuffer[0] == 0) {
	      display.println("You got no mail :(");
      } else {
	      display.println("You got mail :D");
      }

      display.drawLogBuffer(0, 0);
      display.display();

      delay(3000);
      memset(packetBuffer, 0, n);

      // print to OLED
      display.clear();
      display.display();
    }
    btn_pressed = false;
  }
}

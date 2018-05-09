#include <ESP8266WiFi.h>

unsigned long lastMillis = 0;
String hostname;

void connectToWifi(char* ssid, char* password) {

  delay(10);
  hostname = WiFi.hostname();
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

}




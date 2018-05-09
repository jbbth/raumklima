#include <WiFiUdp.h>
WiFiUDP UDP;

template<int T, int F>
struct influxData{
  int nTags = T;
  int nFields = F;
  String measurement;
  String tagKeys[T];
  String tagValues[T];
  String fieldKeys[F];
  String fieldValues[F];
  String timestamp;
};

template<int T, int F>
void sendToInflux(influxData<T, F>* messdaten, String server, int port) {

  String tags = "";
  for(int i = 0; i < T; ++i){
    tags += "," + messdaten->tagKeys[i] + "=" + messdaten->tagValues[i];  
  }
  
  String fields = "";
  for(int i = 0; i < F; ++i){
    String pre = i == 0 ? " " : ",";
    tags += pre + messdaten->fieldKeys[i] + "=" + messdaten->fieldValues[i];  
  }
  
  String influx = messdaten->measurement + tags + fields + " \n";

  int len = influx.length();
  byte message[len];

  influx.getBytes(message,len);

  UDP.beginPacket(server.c_str(), port);
  UDP.write(message, len);
  UDP.endPacket();
}

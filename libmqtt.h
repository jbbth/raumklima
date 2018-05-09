#include <MQTT.h>

#define T_HELLO "/hello"

WiFiClient wifiClient;
MQTTClient mqttClient;

struct MqttSubscription{
  String topic;
  String payload = "";
  bool changed = false;
  String get() {
      changed = false;
      return payload;
  }
};

MqttSubscription *subs;
int mqttnsubs = 0;

void mqttInit(){
    Serial.begin(115200);
    while(!Serial) {}
}

void mqttCallback(String &topic, String &payload){
  Serial.println("MQTT received: " + topic + " " + payload);
  for(int i = 0; i < mqttnsubs; ++i){
    if(subs[i].topic == topic){
      if(subs[i].payload != payload) subs[i].changed = true;
      subs[i].payload = payload;
    }
  }
}

bool mqttConnect() {
  if (mqttClient.connect(hostname.c_str())) {
    mqttClient.publish(T_HELLO, hostname);
    return true;
  }
  return false;
}

bool connectToBroker(char* broker, int port){
  mqttClient.begin(broker, port, wifiClient);
  mqttClient.onMessage(mqttCallback); 
  return mqttConnect();
}

void mqttPublish(String topic, String payload){
  mqttClient.publish(topic, payload);
}

void mqttSubscribe(MqttSubscription *subscriptions, int nsubs){
  subs = subscriptions;
  mqttnsubs = nsubs;
  for(int i=0; i < mqttnsubs; ++i){
    if(mqttClient.subscribe(subs[i].topic)) {
      Serial.println("Successfully subscribed to: " + subs[i].topic);
    } else {
      Serial.println("Error subscribing to: " + subs[i].topic);
    }
    delay(1);
  }
}

void mqttLoop() {
  if (!mqttClient.connected()) {
    mqttConnect();
  } else {
    mqttClient.loop();
  }
}


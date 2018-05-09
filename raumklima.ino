#include <BME280I2C.h>
#include <Wire.h>
#include "libinflux.h"
#include "libwifi.h"
#include "libmqtt.h"
#include "config.h"

BME280I2C bme;

String configRaum = "default";

int messungen = 1;
long previousMillis = 0;
float temperature, humidity, pressure, altitude, dewPoint;

influxData<1,5> messdaten;

const int nsubs = 5;
MqttSubscription mqttsubs[nsubs];

void setup() {
  Serial.begin(SERIAL_BAUD);
  while(!Serial) {}
  Serial.println("Serial connected!");
  
  while(!bme.begin()){
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }
  Serial.println("BME280 connected!");

  connectToWifi(WIFISSID, PASSWORD);
  Serial.println("WiFi connected!");

  messdaten.measurement = "raumklima";
  
  messdaten.tagKeys[0] = "raum";
  messdaten.tagValues[0] = "default"; // set later via mqtt

  messdaten.fieldKeys[0] = "druck";
  messdaten.fieldKeys[1] = "temperatur";
  messdaten.fieldKeys[2] = "feuchtigkeit";
  messdaten.fieldKeys[3] = "hoehe";
  messdaten.fieldKeys[4] = "taupunkt";
 
  String topicConfig = "/config/" + hostname;
  
  mqttsubs[0].topic = topicConfig + "/raum";
  mqttsubs[1].topic = topicConfig + "/intervall";
  mqttsubs[2].topic = topicConfig + "/messungen";
  mqttsubs[3].topic = topicConfig + "/influx/host";
  mqttsubs[4].topic = topicConfig + "/influx/port";

  if(connectToBroker(BROKER, BROKERPORT)) {
    Serial.println("Broker connected!");
    mqttInit();
    mqttSubscribe(mqttsubs, nsubs);
  } else {
    Serial.println("Broker failed!");
  }
  
  Serial.println("Waiting for configuration...");
}

void loop() {
  unsigned long currentMillis = millis();
 
  if(mqttsubs[0].changed) {
    configRaum = mqttsubs[0].get();
    messdaten.tagValues[0] = configRaum;
    //Serial.println("Received config: " + configRaum);
  }

  if(mqttsubs[1].changed) {
    configIntervall = mqttsubs[1].get().toInt();
    pressure = 0;
    temperature = 0;
    humidity = 0;
    altitude = 0;
    dewPoint = 0;
    messungen = 0;
  }
  if(mqttsubs[2].changed) {
    configMessungen = mqttsubs[2].get().toInt();
    pressure = 0;
    temperature = 0;
    humidity = 0;
    altitude = 0;
    dewPoint = 0;
    messungen = 0;

  }
  if(mqttsubs[3].changed) configInfluxHost = mqttsubs[3].get();
  if(mqttsubs[4].changed) configInfluxPort = mqttsubs[4].get().toInt();
  
  if(currentMillis - previousMillis > configIntervall && configRaum != "default") {
      previousMillis = currentMillis;   
  
      float pres, temp, hum, alt, dew;
      uint8_t pressureUnit(1);                                           // unit: B000 = Pa, B001 = hPa, B010 = Hg, B011 = atm, B100 = bar, B101 = torr, B110 = N/m^2, B111 = psi
      bme.read(pres, temp, hum, true, pressureUnit);                   // Parameters: (float& pressure, float& temp, float& humidity, bool celsius = false, uint8_t pressureUnit = 0x0)
      alt = bme.alt(true);
      dew = bme.dew(true);
      
      pressure += pres;
      temperature += temp;
      humidity += hum;
      altitude += alt;
      dewPoint += dew;
    
      if(messungen == configMessungen) {
  
        messdaten.fieldValues[0] = String(pressure / configMessungen);
        messdaten.fieldValues[1] = String(temperature / configMessungen);
        messdaten.fieldValues[2] = String(humidity / configMessungen);
        messdaten.fieldValues[3] = String(altitude / configMessungen);
        messdaten.fieldValues[4] = String(dewPoint / configMessungen);

        mqttPublish("/wohnung/" + configRaum + "/luftdruck", String(pressure / configMessungen));
        mqttPublish("/wohnung/" + configRaum + "/temperatur", String(temperature / configMessungen));
        mqttPublish("/wohnung/" + configRaum + "/feuchtigkeit", String(humidity / configMessungen));
        mqttPublish("/wohnung/" + configRaum + "/hoehe", String(altitude / configMessungen));
        mqttPublish("/wohnung/" + configRaum + "/taupunkt", String(dewPoint / configMessungen));

        sendToInflux<1,5>(&messdaten, configInfluxHost, configInfluxPort);

        Serial.println("data sent");
        
        pressure = 0;
        temperature = 0;
        humidity = 0;
        altitude = 0;
        dewPoint = 0;
        messungen = 0;
  
      }
      messungen++;
    }
    
    mqttLoop();
    delay(5);
}


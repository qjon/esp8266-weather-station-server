#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>

#include "saved_data.h"
#include "log_operations.h"
#include "configuration.h"

// Initialize DHT sensor.
DHT sensorOne(SENSOR_0_PIN, SENSOR_0_TYPE);
DHT sensorTwo(SENSOR_1_PIN, SENSOR_1_TYPE);

// Initialize Time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.pl.pool.ntp.org");

float periodBetweenMeasure = 60 * 60 / TIMES_PER_HOURE;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  pinMode(SENSOR_0_PIN, INPUT);

  if (isSecondSensor()) {
    pinMode(SENSOR_1_PIN, INPUT);
  }

  sensorOne.begin();

  if (isSecondSensor()) {
    sensorTwo.begin();
  }

  Serial.println("Connecting to ");
  Serial.println(SSID);

  //connect to your local wi-fi network
  WiFi.begin(SSID, PASSWORD);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Got MAC: ");
  Serial.println(WiFi.macAddress());

  timeClient.begin();

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  Serial.println("before force");

  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  //
  //  Dir dir = SPIFFS.openDir("/");
  //  while (dir.next()) {
  //    Serial.print(dir.fileName());
  //    File f = dir.openFile("r");
  //    Serial.println(f.size());
  //  }
}


void loop() {

  // add new item
  int sec = timeClient.getEpochTime();

  Serial.println("time: " + (String)sec);

  String ip = WiFi.localIP().toString();

  if (IS_MQTT_SYNC) {

    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);

    while (!client.connected()) {
      Serial.println("Connecting to MQTT...");
      if (client.connect("ESP8266Client", MQTT_USER, MQTT_PASS)) {

        Serial.println("connected");

      } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
      }
    }

    client.loop();

    client.publish(getDeviceTopic("INFO").c_str(), getMqttBonjureMessage().c_str());

    String msg = saveAndGetDataMqtt(0, SENSOR_0_DATA_FILE, LOG_ITEMS, ITEMS_TO_SYNC_PER_REQUEST, sec, sensorOne, ip);
    String topic = getDeviceTopic("SENSOR");

    Serial.println(msg);

    client.publish(topic.c_str(), msg.c_str());


    Serial.println("MQTT");

    delay(5000);

    client.disconnect();

  } else {

    saveAndSendData(0, SENSOR_0_DATA_FILE, LOG_ITEMS, ITEMS_TO_SYNC_PER_REQUEST, sec, sensorOne, ip);

    if (isSecondSensor()) {
      saveAndSendData(1, SENSOR_1_DATA_FILE, LOG_ITEMS, ITEMS_TO_SYNC_PER_REQUEST, sec, sensorTwo, ip);
    }
  }

  ESP.deepSleep(periodBetweenMeasure * 1e6);
}

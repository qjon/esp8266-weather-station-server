#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>

#include "saved_data.h"
#include "log_operations.h"
#include "data_transport.h"
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

int lastMqttSendTimestamp = 0;

int waitCounter = 10;

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


  //  Dir dir = SPIFFS.openDir("/");
  //  while (dir.next()) {
  //    Serial.print(dir.fileName());
  //    File f = dir.openFile("r");
  //    Serial.println(f.size());
  //  }


  // add new item
  int sec = timeClient.getEpochTime();

  Serial.println("time: " + (String)sec);

  String ip = WiFi.localIP().toString();


  // INIT values

  float temp = sensorOne.readTemperature(); // Gets the values of the temperature
  float hum = sensorOne.readHumidity(); // Gets the values of the humidity

  SavedData sensorData(sec, (String)temp, (String)hum);

  bool isSend = false;

  // #1 - get sensor data

  String uniqId = getDeviceUniqId();


  sendDataForSensor(uniqId, ip, 0, SENSOR_0_DATA_FILE, sensorData);

  if (isSecondSensor()) {
    sendDataForSensor(uniqId, ip, 1, SENSOR_1_DATA_FILE, sensorData);
  }
}

void loop() {

  client.loop();

  delay(4000);


  ESP.deepSleep(periodBetweenMeasure * 1e6);
}

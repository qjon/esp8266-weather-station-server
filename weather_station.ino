#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>

#include "saved_data.h"
#include "log_operations.h"
#include "configuration.h"

// Initialize DHT sensor.
DHT sensorOne(SENSOR_0_PIN, SENSOR_0_TYPE);
DHT sensorTwo(SENSOR_1_PIN, SENSOR_1_TYPE);

// Initialize Time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

float periodBetweenMeasure = 60 * 60 / TIMES_PER_HOURE;

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

  timeClient.begin();

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  // add new item
  int sec = timeClient.getEpochTime();

  String ip = WiFi.localIP().toString();

  saveAndSendData(0, SENSOR_0_DATA_FILE, LOG_ITEMS, ITEMS_TO_SYNC_PER_REQUEST, sec, sensorOne, ip);

  if (isSecondSensor()) {
    saveAndSendData(1, SENSOR_1_DATA_FILE, LOG_ITEMS, ITEMS_TO_SYNC_PER_REQUEST, sec, sensorTwo, ip);
  }

  ESP.deepSleep(periodBetweenMeasure * 1e6);
}


void loop() {
}

void saveAndSendData(int sensor, String dataFile, int logItemsLength, int itemsToSync, int currentTimestamp, DHT dht, String ip) {
  String json;

  // Init Log operastions object
  LogOperations list(dataFile, logItemsLength, itemsToSync);

  // check if log file exist and create it if necessary
  list.createLogFileIfNotExists();

  // read log file
  list.readFromFile();

  float temp = dht.readTemperature(); // Gets the values of the temperature
  float hum = dht.readHumidity(); // Gets the values of the humidity

  list.addData(currentTimestamp, (String)temp, (String)hum);

  // print data
  list.printData();


  // create JSON based on read data
  json = list.toJSON(ip, sensor);

  Serial.println(json);

  HTTPClient http;
  http.begin(SYNC_URL);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(json);

  if (httpCode >= 200 && httpCode < 300) {
    String payload = http.getString();
    list.parseSyncResponse(payload);
  } else {
    Serial.println("Wrong response: " + (String)httpCode);
  }

  list.saveFile();

  http.end();
}

bool isSecondSensor() {
  return SENSOR_1_PIN != NULL;
}

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>

#include "saved_data.h"
#include "log_operations.h"

#define DHTTYPE DHT11                     // Sensor type DHT11 or DHT22
#define DHTPin D4                         // PIN to which is connected sensor

#define TIMES_PER_HOURE 4                 // number of times per hour to read data from sensor
#define DATA_FILE "/data.csv"             // place to store data file

#define SSID "YOUR_WIFI_SSID"             // put your WIFI SSID  
#define PASSWORD "YOUR_WIFI_PASSWORD"     // put your WIFI password

#define LOG_ITEMS 150                     // number of log data records

#define ITEMS_TO_SYNC_PER_REQUEST 15      // max 19, because of max body size 1460 bytes

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Initialize Time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

float periodBetweenMeasure = 60 * 60 / TIMES_PER_HOURE;

const String host = "http://SERVER_ADRESS"; // put your server
const String url = "YOUR_URL";              // put path to ypur server API

void setup() {
  Serial.begin(115200);

  pinMode(DHTPin, INPUT);

  dht.begin();

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

  saveAndSendData(DATA_FILE, LOG_ITEMS, ITEMS_TO_SYNC_PER_REQUEST, sec, dht, ip);


  ESP.deepSleep(periodBetweenMeasure * 1e6);
}


void loop() {
}

void saveAndSendData(String dataFile, int logItemsLength, int itemsToSync, int currentTimestamp, DHT dht, String ip) {
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
  json = list.toJSON(ip);

  Serial.println(json);

  HTTPClient http;
  http.begin(host + url);
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

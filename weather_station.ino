#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

#define LEAP_YEAR(Y)     ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )

#define DHTTYPE DHT22                     // Sensor type DHT11 or DHT22
#define DHTPin D4                         // PIN to which is connected sensor

#define TIMES_PER_HOURE 2                 // number of times per hour to read data from sensor
#define DATA_FILE "/data.csv"             // place to store data file
#define SSID "YOUR_WIFI_ID"               // put your WIFI SSID  
#define PASSWORD "YOUR_WIFI_PASSWORD"     // put your WIFI password

#define PAGE_TITLE "PAGE_TITLE"           // put your page title - device name 

#define LOG_ITEMS 150                     // number of log data records

// Initialize Http Server
ESP8266WebServer server(80);

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Initialize Time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

float periodBetweenMeasure = 60 * 60 / TIMES_PER_HOURE;

float Temperature;
float Humidity;

int lastSec = 0;

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

  server.on("/", handle_OnConnect);                         // display HTML page with current sensor data and save it do DATA file
  server.on("/api", handle_OnApiCall);                      // return JSON with current sensor data and save it to DATA file
  server.on("/api/file/clear", handle_OnApiClearDataFile);  // clear DATA file
  server.on("/api/file/sync", handle_OnSync);               // return JSON with all data stored in DATA file
  server.onNotFound(handle_NotFound);                       // error page

  server.begin();
  Serial.println("HTTP server started");


  timeClient.begin();
  timeClient.setTimeOffset(3600);

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
}


void loop() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  saveData();

  server.handleClient();

}

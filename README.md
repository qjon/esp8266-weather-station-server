# esp8266-weather-station-server
ESP8266 + DHT11 weather station device.

## Description

This program is used to create weather station device based on _ESP8266 nodemcu v3_ and _DHT11/DHT22_. 

Main functionalities:

- get data from sensor (possibility to use 2 sensor in the same time - one outside, one inside)
- if possible send data to server, if not save in log file (sync in the future)
- sleep for some time to save power
- use MQTT or HTTP protocol

## Instalations

- clone this repository
- use Arduino IDE to open project
- create _configuration.h_ file based on _configuration_sample.h_
- change configuration data in _configuration.h_ file
  
        #define IS_MQTT_SYNC true                             // use MQTT or HTTP protocol

        #define SENSOR_0_TYPE DHT11                           // Sensor 0 type DHT11 or DHT22
        #define SENSOR_1_TYPE DHT22                           // Sensor 1 type DHT11 or DHT22

        #define SENSOR_0_PIN D4                               // Sensor 0 PIN
        #define SENSOR_1_PIN D5                               // Sensor 1 PIN

        #define TIMES_PER_HOURE 240                           // number of times per hour to read data from sensor

        #define SENSOR_0_DATA_FILE "/data.csv"                // log file name for sensor 0
        #define SENSOR_1_DATA_FILE "/data22.csv"              // log file name for sensor 1

        #define SSID "WIFI_SSID"                              // put your WIFI SSID  
        #define PASSWORD "WIFI_PASSWORD"                      // put your WIFI password

        #define MQTT_SERVER "192.168.1.1"                     // MQTT server IP
        #define MQTT_PORT 1883                                // MQTT server port
        #define MQTT_USER "ws"                                // MQTT server user
        #define MQTT_PASS "ws"                                // MQTT server user password

        #define LOG_ITEMS 150                                 // number of log data records

        #define ITEMS_TO_SYNC_PER_REQUEST 15                  // max 15, because of max body size 1460 bytes

        #define SYNC_URL "http://SERVER_NAME:PORT/SYNC_URL"   // put path to ypur server API
      
- upload program to your device

## Communication with server

### MQTT protocol

#### Welcome message

        {"ip":"DEVICE_IP","uniqId":"DEVICE_ID","sensors":[{"symbol":0,"name":"11"},{"symbol":1,"name":"22"}]}

#### Sensor data

        {"uniqId":"DEVICE_ID","sensor":0,"payload":{"time":1595343414,"temp":"25.60","hum":"57.00"}}

### HTTP protocol

Each time when device is wake up it try to send data to server in such JSON format:


        {"ip":"DEVICE_IP","sensor": 0, "data":[{"time":1585669406,"temp":"20.80","hum":"66.00"},{"time":1585669428,"temp":"21.00","hum":"61.00"}]}

Response for such request is:


        [{"time":1585669406,"sync":true},{"time":1585669428,"sync":true}]

Each item in response has timestamp value and sync value (bool). If sync value is _true_ then such record is not stored in log file, if it is _false_ record is stored and device will try send it once more next time.
      

## Change log

### v3.0.0

- add possibility to communicate with server via MQTT or HTTP protocol

### v2.1.0

- add possibility to connect second sensor
- change communicatin with server - additional _sensor_ property is send
- change configuration

### v2.0.0

- remove API and HTTP server
- save power - use deep sleep
- read sensor data -> send do server (or save in file - sync in future) -> go deep sleep 
- remember max 150 sensor data reads (if the log file is full oldest record is deleted and new one is added)
- each time when device is sending data to server it try to send first 15 records saved in log file
- fix: epochTime form NTP server sometimes return time from feature - now it is not a problem (such result is stored in log file but it is not parsed on server)

### v1.0.0

- HTML page to display current sensor values
- API to store, display and clear current data
      

# esp8266-weather-station-server
ESP8266 + DHT11 weather station server.

## Description

This program is used to create weather station server based on _ESP8266 nodemcu v3_ and _DHT11/DHT22_. 

Main functionalities:

- get data from sensor
- if possible send data to server, if not save in log file (sync in the future)
- sleep for some time to save power

## Instalations

- clone this repository
- use Arduino IDE to open project
- configure some data in _weather_station.ino_

  
        #define DHTTYPE DHT22                     // Sensor type DHT11 or DHT22
        #define DHTPin D4                         // PIN to which is connected sensor

        #define TIMES_PER_HOURE 2                 // number of times per hour to read data from sensor
        #define DATA_FILE "/data.csv"             // place to store data file
        #define SSID "YOUR_WIFI_ID"               // put your WIFI SSID  
        #define PASSWORD "YOUR_WIFI_PASSWORD"     // put your WIFI password

        #define PAGE_TITLE "PAGE_TITLE"           // put your page title - device name 

        #define LOG_ITEMS 150                     // number of log data records (max: 150)


        const String host = "http://SERVER_ADRESS"; // put your server
        const String url = "YOUR_URL";              // put path to ypur server API
      
- upload program to your device

## Communication with server

Each time when device is wake up it try to send data to server in such JSON format:


        {"ip":"DEVICE_IP","data":[{"time":1585669406,"temp":"20.80","hum":"66.00"},{"time":1585669428,"temp":"21.00","hum":"61.00"}]}

Response for such request is:


        [{"time":1585669406,"sync":true},{"time":1585669428,"sync":true}]

Each item in response has timestamp value and sync value (bool). If sync value is _true_ then such record is not stored in log file, if it is _false_ record is stored and device will try send it once more next time.
      

## Change log
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
      

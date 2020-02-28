# esp8266-weather-station-server
ESP8266 + DHT11 weather station server.

## Description

This program is used to create weather station server based on _ESP8266 nodemcu v3_ and _DHT11/DHT22_. 

Main functionalities:

- display HTML page with current sensor values
- store some data from past in some data file
- create API to get current and saved sensor data

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

      #define LOG_ITEMS 150                     // number of log data records
      
- upload program to your device

## API 

Each response from server is a JSON file. Also each response has _error_ property. If its value is 0 it means that response is ok any other value grater than 0 means that there is some problem.

### Get current data

Request:

      GET /api
      
Response:

- data:
    - humidity: current humidity value
    - temperature: current temperature value
    
      
Response example:

      {"error":0,"data":{"humidity":54,"temperature":21.3}}
      

### Get saved data to sync

Request:

      GET /api/file/sync

Response:

- data: 
    - toSync: list of saved sensor data (timestamp|temperature|humidity)

Response example:

      {"error": 0, "data": {"toSync": ["1582905784|22.10|59.00", "1582907585|21.40|54.00"]}}
      
### Clear saved ddata file

Request:

      GET /api/file/clear

Response example:

      {"error": 0}
      

## Change log

### v1.0.0

- HTML page to display current sensor values
- API to store, display and clear current data
      

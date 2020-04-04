#define SENSOR_0_TYPE DHT11                           // Sensor 0 type DHT11 or DHT22
#define SENSOR_1_TYPE DHT22                           // Sensor 1 type DHT11 or DHT22

#define SENSOR_0_PIN D4                               // Sensor 0 PIN
#define SENSOR_1_PIN D5                               // Sensor 1 PIN

#define TIMES_PER_HOURE 240                           // number of times per hour to read data from sensor

#define SENSOR_0_DATA_FILE "/data.csv"                // log file name for sensor 0
#define SENSOR_1_DATA_FILE "/data22.csv"              // log file name for sensor 1

#define SSID "WIFI_SSID"                              // put your WIFI SSID  
#define PASSWORD "WIFI_PASSWORD"                      // put your WIFI password

#define LOG_ITEMS 150                                 // number of log data records

#define ITEMS_TO_SYNC_PER_REQUEST 15                  // max 15, because of max body size 1460 bytes

#define SYNC_URL "http://SERVER_NAME:PORT/SYNC_URL"   // put path to ypur server API

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

String saveAndGetDataMqtt(int sensor, String dataFile, int logItemsLength, int itemsToSync, int currentTimestamp, DHT dht, String ip) {
  String json;
  SavedData lastResult;

  // Init Log operastions object
  LogOperations list(dataFile, logItemsLength, itemsToSync);

  // check if log file exist and create it if necessary
  list.createLogFileIfNotExists();

  // read log file
  list.readFromFile();

  float temp = dht.readTemperature(); // Gets the values of the temperature
  float hum = dht.readHumidity(); // Gets the values of the humidity

  lastResult = list.addData(currentTimestamp, (String)temp, (String)hum);

  // print data
  list.printData();


  // create JSON based on read data
  json = list.toJSON(ip, sensor);

  Serial.println(json);

  list.saveFile();

  return lastResult.toMqttString(getDeviceUniqId(), sensor);
}

bool isSecondSensor() {
  return SENSOR_1_PIN != NULL;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println("-----------------------");

}


String getDeviceUniqId() {
  String macAddress = WiFi.macAddress();

  return macAddress.substring(9, 11) + macAddress.substring(12, 14) + macAddress.substring(15, 17);
}

String getMqttBonjureMessage() {
  String message;
  IPAddress ip = WiFi.localIP();
  const int capacity = JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 80;

  DynamicJsonDocument root(capacity);

  root["ip"] = ip.toString();
  root["uniqId"] = getDeviceUniqId();

  JsonArray sensors = root.createNestedArray("sensors");

  JsonObject s0 = sensors.createNestedObject();

  s0["symbol"] = 0;
  s0["name"] = (String)SENSOR_0_TYPE;

  if (SENSOR_1_PIN) {
    JsonObject s1 = sensors.createNestedObject();

    s1["symbol"] = 1;
    s1["name"] = (String)SENSOR_1_TYPE;
  }

  serializeJson(root, message);

  return message;
}

String getDeviceTopic(String topic) {
  return "ws/" + getDeviceUniqId() + "/" + topic;
}

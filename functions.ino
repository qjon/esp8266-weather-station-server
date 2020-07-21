
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


  String topicSubscribe = getDeviceTopic("cmd") + "/saved";

  if ((String)topic == topicSubscribe) {
    Serial.println("Get data");
  }

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

String getDeviceTopic(String topic, int sensor) {
  return "ws/" + getDeviceUniqId() + "_" + (String)sensor + "/" + topic;
}

void addSensorDataToLog(String dataFile, int logItemsLength, SavedData sensorData) {
  String json;

  // Init Log operastions object
  LogOperations list(dataFile, logItemsLength);

  // check if log file exist and create it if necessary
  list.createLogFileIfNotExists();

  // read log file
  list.readFromFile();

  list.addData(sensorData);

  // print data
  list.printData();
  list.saveFile();
}


void sendDataForSensor(String uniqId, String ip, int sensor, String dataFile, SavedData sensorData) {
  bool isSend = false;

  // Init LOG FILE
  LogOperations list(dataFile, LOG_ITEMS, ITEMS_TO_SYNC_PER_REQUEST);
  list.readFromFile();
  list.printData();

  // Save data using MQTT or HTTP for first Sensor
  if (IS_MQTT_SYNC) {
    MqttDataTransport dataTransport(MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS, uniqId);
    dataTransport.setClient(&client);

    if (sensor == 0) {
      dataTransport.sendInfo(ip, (String)SENSOR_0_TYPE, SENSOR_1_PIN ? (String)SENSOR_1_TYPE : "");
    }
    isSend = dataTransport.send(sensor, sensorData);

    if (isSend) {
      Serial.println("Data sent");
      dataTransport.sendArchiveData(sensor, list);
    }

  } else {
    HttpDataTransport dataTransport(ip, SYNC_URL);
    isSend = dataTransport.send(sensor, sensorData);
    if (isSend) {
      Serial.println("Data sent");
      dataTransport.sendArchiveData(sensor, list);
    }
  }
  //
  //  Add data to LOG File
  if (!isSend) {
    Serial.println("Data didn't send, try to save to LOG");
    list.addData(sensorData);
    list.saveFile();
  }
}

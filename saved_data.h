#ifndef saved_data_h
#define saved_data_h


class SavedData {
  public:
    int timestamp;
    String temperature;
    String humidity;
    bool isSync;
    bool isValid;
    SavedData();
    SavedData(String data);
    SavedData(int time, String temp, String hum);
    JsonObject toJson(JsonArray *arr);
    String toMqttString(String ip, int sensor);
    String toString();
};

SavedData::SavedData() {
  isValid = false;
}

SavedData::SavedData(int time, String temp, String hum) {
  isValid = true;
  timestamp = time;
  temperature = temp;
  humidity = hum;
  isSync = false;

}

SavedData::SavedData(String data) {
  if (data.length() == 24) {
    isValid = true;
    timestamp = data.substring(0, 10).toInt();
    temperature = data.substring(11, 16);
    humidity = data.substring(17, 22);
    isSync = data.substring(23, 24) == "1";
  } else {
    isValid = false;
  }
}

JsonObject SavedData::toJson(JsonArray *arr) {
  JsonObject obj = (*arr).createNestedObject();
  obj["time"] = timestamp;
  obj["temp"] = temperature;
  obj["hum"] = humidity;

  return obj;
}


String SavedData::toMqttString(String uniqId, int sensor) {
  String message;
  const int capacity = 2 * JSON_OBJECT_SIZE(3) + 70;

  DynamicJsonDocument root(capacity);

  root["uniqId"] = uniqId;
  root["sensor"] = sensor;

  JsonObject data = root.createNestedObject("payload");

  data["time"] = timestamp;
  data["temp"] = temperature;
  data["hum"] = humidity;

  serializeJson(root, message);

  return message;
}

String SavedData::toString() {
  return (String)timestamp + "|" + temperature + "|" + humidity + "|" + (isSync ? "1" : "0");
}
#endif

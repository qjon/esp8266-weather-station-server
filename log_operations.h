#ifndef log_operations_h
#define log_operations_h

#include "saved_data.h"
#include "DHT.h"


SavedData list[150];


class LogOperations {

  public:
    LogOperations(String fileName, int listSize, int itemsToSyncPerRequest);
    void addData(int time, String temp, String hum);
    bool createLogFileIfNotExists();
    void parseSyncResponse(String response);
    void printData();
    void readFromFile();
    void removeLogFile();
    void saveFile();
    String toJSON(String ip);

  private:
    String _dataFileName;
    int _listLength;
    int _itemsToSyncPerRequest;
    SavedData* _list;
    bool markItemAsSync(int time);
    int _readItemsFromFile;
    void removeOldestDataItem();
};

LogOperations::LogOperations(String fileName, int listSize, int itemsToSyncPerRequest) {
  _listLength = listSize;
  _dataFileName = fileName;
  _itemsToSyncPerRequest = itemsToSyncPerRequest;
  _list = list;
  _readItemsFromFile = 0;
}

void LogOperations::addData(int time, String temp, String hum) {
  SavedData s(time, temp, hum);

  if (_readItemsFromFile < _listLength) {
    _list[_readItemsFromFile++] = s;
  } else {
    removeOldestDataItem();
    _list[_listLength - 1] = s;
  }
}

bool LogOperations::createLogFileIfNotExists() {
  File file = SPIFFS.open(_dataFileName, "r");

  if (file) {
    Serial.println("Log file exist.");
    return false;
  } else {
    File file = SPIFFS.open(_dataFileName, "w");

    file.close();
    Serial.println("Log file has been created.");

    return true;
  }
}

void LogOperations::parseSyncResponse(String response) {
  const size_t capacity = JSON_ARRAY_SIZE(_itemsToSyncPerRequest) + _itemsToSyncPerRequest * JSON_OBJECT_SIZE(2) + _itemsToSyncPerRequest * 10 + 300;

  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, response);

  JsonArray array = doc.as<JsonArray>();

  for (JsonObject v : array) {
    int t = v["time"];
    bool sync = v["sync"];

    if (sync) {
      markItemAsSync(t);
    }
  }
}

void LogOperations::printData() {
  for (int i = 0; i < _listLength; i++) {
    SavedData s = _list[i];
    if (s.isValid) {
      Serial.println((String)i + ": " + s.toString());
    }
  }
}

void LogOperations::readFromFile() {
  String message;
  int index = 0;
  const int fileLineLength = 26;

  File file = SPIFFS.open(_dataFileName, "r");
  if (!file) {
    Serial.println("Failed to open data file");
  } else {
    char buffer[fileLineLength];

    while (file.available()) {
      int l = file.readBytesUntil('\n', buffer, sizeof(buffer));
      buffer[l] = 0;
      String line = (String)buffer;
      SavedData data(line.substring(0, fileLineLength - 2));
      _list[index++] = data;

      _readItemsFromFile++;
    }

    file.close();
  }
}

void LogOperations::removeLogFile() {
  File file = SPIFFS.open(_dataFileName, "r");
  if (!file) {
    Serial.println("Failed to open data file");
  } else {
    SPIFFS.remove(_dataFileName);

    Serial.println("Log file has been removed");
  }
}

String LogOperations::toJSON(String ip) {
  String message;
  const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(_itemsToSyncPerRequest) + _itemsToSyncPerRequest * JSON_OBJECT_SIZE(3) + 518;

  DynamicJsonDocument root(capacity);

  root["ip"] = ip;

  JsonArray data = root.createNestedArray("data");

  int notSyncItems = 0;

  for (int i = 0; i < _listLength; i++) {
    if (notSyncItems < _itemsToSyncPerRequest) {

      SavedData s = _list[i];

      if (s.isValid && !s.isSync ) {
        JsonObject obj = s.toJson(&data);

        notSyncItems++;
      }
    }
  }

  serializeJson(root, message);

  return message;
}


void LogOperations::saveFile() {
  String content = "";
  bool isAnySaved = false;

  for (int index = 0; index < _listLength; index++) {
    SavedData s = _list[index];
    if (s.isValid && !s.isSync) {

      if (isAnySaved) {
        content += "\r\n";
      }

      content += s.toString();

      isAnySaved = true;
    }
  }


  File file = SPIFFS.open(_dataFileName, "w");
  if (!file) {
    Serial.println("Failed to open data file for daving");
  } else {
    file.print(content);
    file.close();
  }
}

bool LogOperations::markItemAsSync(int time) {
  int index = -1;
  bool found = false;

  do {
    index++;

    if (_list[index].timestamp == time) {
      _list[index].isSync = true;

      found = true;
    }
  } while (!found && index < _listLength);
}

void LogOperations::removeOldestDataItem() {
  for (int i = 0; i < _listLength - 1; i++) {
    _list[i] = _list[i + 1];
  }
}
#endif

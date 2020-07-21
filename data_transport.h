#ifndef data_transport_h
#define data_transport_h

#include "saved_data.h"
#include <ArduinoJson.h>

class DataTransport {
  public:
    virtual bool send(int sensor, SavedData data);
    virtual bool sendArchiveData(int sensor, LogOperations &list);
};


class MqttDataTransport : public DataTransport {
  public:
    MqttDataTransport(char* url, uint16_t port, char* user, char* password, String uniqId);
    virtual bool send(int sensor, SavedData data);
    void sendInfo(String ip, String sensor0neName, String sensorTwoName);
    void setClient(PubSubClient *client);
    virtual bool sendArchiveData(int sensor, LogOperations &list);
  private:
    PubSubClient* _client;
    bool connect();
    String getDeviceTopic(String topic);
    String getDeviceTopic(String topic, int sensor);
    char* _url;
    uint16_t _port;
    char* _user;
    char* _password;
    String _uniqId;
    int maxNumberOfConnectionTries = 10;
};

//
// MQTT
//
MqttDataTransport::MqttDataTransport(char* url, uint16_t port, char* user, char* password, String uniqId) {
  _url = url;
  _port = port;
  _user = user;
  _password = password;
  _uniqId = uniqId;
}

void MqttDataTransport::sendInfo(String ip, String sensor0neName, String sensorTwoName) {
  if (connect()) {
    String message;
    const int capacity = JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 80;

    DynamicJsonDocument root(capacity);

    root["ip"] = ip;
    root["uniqId"] = _uniqId;

    JsonArray sensors = root.createNestedArray("sensors");

    JsonObject s0 = sensors.createNestedObject();

    s0["symbol"] = 0;
    s0["name"] = sensor0neName;

    if (sensorTwoName != "") {
      JsonObject s1 = sensors.createNestedObject();

      s1["symbol"] = 1;
      s1["name"] = sensorTwoName;
    }

    serializeJson(root, message);

    String topic = getDeviceTopic("INFO");

    _client->publish(topic.c_str(), message.c_str());
  }
}

bool MqttDataTransport::send(int sensor, SavedData data) {
  if (connect()) {
    String message = data.toMqttString(_uniqId, sensor);


    String topic = getDeviceTopic("SENSOR", 0);
    Serial.println(topic);

    _client->publish(topic.c_str(), message.c_str());

    return true;
  }
  else {
    return false;
  }
}


void MqttDataTransport::setClient(PubSubClient *client) {
  _client = client;
}

bool MqttDataTransport::connect() {
  int connectionTries = 0;

  _client->setServer(_url, _port);

  while (!_client->connected() && connectionTries < maxNumberOfConnectionTries) {
    Serial.println("Connecting to MQTT...");
    if (_client->connect("WS", _user, _password)) {

      Serial.println("connected");

    } else {
      Serial.print("failed with state: ");
      Serial.println(_client->state());
      delay(500);
      connectionTries++;
    }
  }

  return _client->connected();
}

String MqttDataTransport::getDeviceTopic(String topic) {
  return "ws/" + _uniqId + "/" + topic;
}

String MqttDataTransport::getDeviceTopic(String topic, int sensor) {
  return "ws/" + _uniqId + "_" + (String)sensor + "/" + topic;
}

bool MqttDataTransport::sendArchiveData(int sensor, LogOperations &list) {
  // read log file
  list.readFromFile();
  
  SavedData* archive = list.getList();

  SavedData* s;

  for (int i = 0; i < list.getSize(); i++) {
    s = &archive[i];

    if (s->isValid) {
      Serial.println("Send archive data");
      send(sensor, *s);
      s->isSync = true;
      Serial.println(s->toString());
    }
  }

  list.saveFile();

  return true;
}


//
// HTTP
//

class HttpDataTransport : public DataTransport {
  public:
    HttpDataTransport(String ip, String url);
    virtual bool send(int sensor, SavedData data);
    virtual bool sendArchiveData(int sensor, LogOperations &list);
    
  private:
    String _ip;
    String _url;
};


HttpDataTransport::HttpDataTransport(String ip, String url) {
  _ip = ip;
  _url = url;
}

bool HttpDataTransport::send(int sensor, SavedData data) {
  String message;
  const int capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(3) + 157;

  bool sendOk = false;

  DynamicJsonDocument root(capacity);

  root["ip"] = _ip;
  root["sensor"] = sensor;

  JsonArray dataObj = root.createNestedArray("data");
 
  JsonObject obj = data.toJson(&dataObj);

  serializeJson(root, message);

  HTTPClient http;
  http.begin(_url);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(message);

  if (httpCode >= 200 && httpCode < 300) {
    sendOk = true;
  }

  http.end();

  return sendOk;
}

bool HttpDataTransport::sendArchiveData(int sensor, LogOperations &list) {  
  String message = list.toJSON(_ip, sensor);  

  HTTPClient http;
  http.begin(_url);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(message);

  if (httpCode >= 200 && httpCode < 300) {
    String payload = http.getString();
    list.parseSyncResponse(payload);
    list.saveFile();
  }

  http.end();

  return true;
}
#endif

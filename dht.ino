/**
   Save data to file on some time interval
*/
void saveData() {
  int sec = timeClient.getEpochTime();

  if (sec - lastSec > periodBetweenMeasure) {

    Temperature = dht.readTemperature(); // Gets the values of the temperature
    Humidity = dht.readHumidity(); // Gets the values of the humidity

    if ((String)Temperature != "nan" && (String)Humidity != "nan") {

      Serial.println((String)Temperature);
      Serial.println((String)Humidity);


      String message = parseDhtData(Temperature, Humidity);
      updateDataFile(message);

      Serial.println(message);
      lastSec = sec;
    } else {
      Serial.println("Error reading sensor values: temp (" + (String)Temperature + "), humidity (" + (String)Humidity + ")");
      delay(5000);
    }
  }
}

/**
   Convert data to log string
*/
String parseDhtData(float Temperature, float Humidity) {
  return (String)timeClient.getEpochTime() + "|" + Temperature + "|" + Humidity;
}

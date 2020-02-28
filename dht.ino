/**
 * Save data to file on some time interval
 */
void saveData() {
  int sec = timeClient.getEpochTime();

  if (sec - lastSec > periodBetweenMeasure) {
  
    Temperature = dht.readTemperature(); // Gets the values of the temperature
    Humidity = dht.readHumidity(); // Gets the values of the humidity

    Serial.println((String)Temperature);
    Serial.println((String)Humidity);
    
    
    String message = parseDhtData(Temperature, Humidity);
    updateDataFile(message);

    Serial.println(message);
    lastSec = sec;
  }
}

/**
 * Convert data to log string
 */
String parseDhtData(float Temperature, float Humidity) {
    return (String)timeClient.getEpochTime() + "|" + Temperature + "|" + Humidity;
}

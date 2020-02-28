/**
 * Append to data file new data
 */
void updateDataFile(String data) {
  String message = readFile();
  String arr[LOG_ITEMS];

  getLastNRowsFromString(message, arr);


  File file = SPIFFS.open(DATA_FILE, "w");
  if (!file) {
    Serial.println("Failed to open data file");
  } else {
    for (int i = 1; i < LOG_ITEMS; i++) {
      if (arr[i] != "") {
        file.println(arr[i]);
      }
    }
    file.println(data);

    file.close();
  }
}

/**
 * Read file data
 */
String readFile() {
  String message = "";
  File file = SPIFFS.open(DATA_FILE, "r");
  if (!file) {
    Serial.println("Failed to open data file");
  } else {
    message = file.readString();

    file.close();
  }

  return message;
}

/**
 * Read file data
 */
bool clearFile() {
  File file = SPIFFS.open(DATA_FILE, "w");
  if (!file) {
    Serial.println("Failed to open data file");
    return false;
  } else {
    file.print("");

    file.close();

    return true;
  }
}

/**
 * Get last N rows data from string
 */
void getLastNRowsFromString(String message, String arr[]) {
  String line;
  int index = LOG_ITEMS - 1;
  String c;
  String d;

  for (int i = message.length() - 1; i >= 0; i = i - 1) {
    c = message.charAt(i - 1);
    d = message.charAt(i);

    if ((c == "\r" &&  d == "\n") || i == 0) {

      if (i == 0) {
        line = d + line;
      }

      if (line != "") {
        arr[index] = line;
        index--;
      }

      line = "";
      i--;
    } else {
      line = d + line;
    }

    if (index == -1) {
      break;
    }
  }

}

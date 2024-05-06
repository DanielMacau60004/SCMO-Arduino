#include "system.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long lcdTime;

DHTesp dht;

//System variables
SystemState currentState = WAITING;
unsigned long lastDate;
unsigned long timeRunnig;
unsigned long timePausing;

unsigned long lastFetch;


void printLCD(String line1, String line2, int duration = 3) {
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  lcd.backlight();
  lcdTime = millis() + duration * 1000;
}

void cloudMessage(DynamicJsonDocument json) {
  sys = json;  //Update data from cloud
  Serial.println("Message received");
}

void fileMessage(DynamicJsonDocument json) {
  if (json == NULL || json.size() == 0)
    return;

  //Copy fields
  if (json.containsKey("data"))
    sys["data"] = json["data"];
  if (json.containsKey("status"))
    sys["status"] = json["status"];

  Serial.println("File received");
}

void startSystem() {
  Wire.begin(SDA, SCL);

  lcd.init();
  dht.setup(DHTPIN, DHTesp::DHT11);
  pinMode(MOTION, INPUT);

  ledcSetup(SERVO_CHN, SERVO_FRQ, SERVO_BIT);
  ledcAttachPin(SERVO_1, SERVO_CHN);
  ledcAttachPin(SERVO_2, SERVO_CHN);

  //Fetch data from the cloud and file
  while (sys == NULL || sys.size() == 0) {
    getRequest(BASE_URL SYSTEM_ID, cloudMessage);

    DynamicJsonDocument doc(1024);
    readFile(SYSTEM_FILE, doc);
    fileMessage(doc);

    sleep(100);
  }

  Serial.println("Data fetched!");
  //deserializeJson(sys, "{\"id\": \"arduino\",\"active\": true,\"duration\": 10,\"hourToStart\": 8,\"rotation\": [true, false, true, true, false, true, false]}");
}

void running(unsigned long currentDate) {
  //Stop state
  if (timeRunnig >= sys["duration"].as<unsigned long>() * 60) {
    currentState = WAITING;
    addStatus();
    return;
  }
  timeRunnig = currentDate - lastDate;
  lastDate = currentDate;

  //Run things...

  //Check if the movement sensor fired
  if (digitalRead(MOTION)) {
    currentState = PAUSED;
    timePausing = 0;
    addStatus();
    return;
  }

  //Move sensors
  ledcWrite(SERVO_CHN, 1);

  //Fetch data from the cloud constantly
  putRequest(BASE_URL SYSTEM_ID PUT_ENDPOINT, sys, cloudMessage);

}

void waiting(unsigned long currentDate) {
  time_t timeInSeconds = currentDate;
  struct tm *local_time = localtime(&timeInSeconds);
  unsigned long hourToStart = sys["hourToStart"].as<unsigned long>();

  bool needsToAct = sys["hourToStart"].as<JsonArray>()[local_time->tm_wday];

  //Day off :D
  if (!needsToAct) return;

  // Starting state
  // TODO Check if shoud act!
  if (local_time->tm_hour == hourToStart) {
    currentState = RUNNING;
    lastDate = currentDate;
    timeRunnig = 0;
    addStatus();
    return;
  }

}

void paused(unsigned long currentDate) {
  timePausing = currentDate - lastDate;

  //Do nothing
  if (timePausing >= TIME_PAUSING) {
    currentState = RUNNING;
    lastDate = currentDate;
    addStatus();
    return;
  }

}

void loopSystem() {

  if (lcdTime <= millis()) {
    lcd.noBacklight();
    lcdTime = ULONG_MAX;
  }

  //If the system was stopped
  bool isActive = sys["active"];
  if (!isActive)
    return;

  unsigned long currentDate = getCurrentDate();

  if (currentState == RUNNING) running(currentDate);
  else if (currentState == WAITING) running(currentDate);
  else if (currentState == PAUSED) paused(currentDate);

  //Do this periodically
  if (currentDate - lastFetch > TIME_FETCHING) {
    writeFile(SYSTEM_FILE, sys);
    putRequest(BASE_URL SYSTEM_ID PUT_ENDPOINT, sys, cloudMessage);
    addData();
    lastFetch = currentDate;
  }

}


void addData() {
  JsonArray jsonArray;
  if (sys.containsKey("data"))
    jsonArray = sys["data"].as<JsonArray>();
  else jsonArray = sys.createNestedArray("data");

  JsonObject root = jsonArray.createNestedObject();
  root["temp"] = dht.getTemperature();
  root["hum"] = dht.getHumidity();
  root["t"] = getCurrentDate();
}

void addStatus() {
  JsonArray jsonArray;
  if (sys.containsKey("status"))
    jsonArray = sys["status"].as<JsonArray>();
  else jsonArray = sys.createNestedArray("status");

  JsonObject root = jsonArray.createNestedObject();
  root["status"] = currentState;
  root["t"] = getCurrentDate();
}

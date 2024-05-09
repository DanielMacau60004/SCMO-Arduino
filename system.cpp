#include "system.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long lcdTime;

DHTesp dht;

//System variables
SystemState currentState = WAITING;
unsigned long lastDate;
unsigned long timeRunning;
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
  sys = json;
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
  Serial.println("Starting System!");
  while (sys == NULL || sys.size() == 0) {
    getRequest(BASE_URL SYSTEM_ID, cloudMessage);

    delay(500);
    Serial.print(".");
  }

  Serial.println("\nSystem Started!");
}

void running(unsigned long currentDate) {
  //Stop state
  if (timeRunning >= sys["duration"].as<unsigned long>() * 60) {
    currentState = WAITING;
    addStatus();
    ledcWrite(SERVO_CHN, 0);
    return;
  }

  timeRunning += currentDate - lastDate;
  lastDate = currentDate;

  unsigned long hours = timeRunning / 3600;
  unsigned long minutes = (timeRunning % 3600) / 60;
  printLCD("Running!", String(hours) + ":" + String(minutes) + "           ", 2);

  //Run things...
  ledcWrite(SERVO_CHN, 1);

  //Check if the movement sensor fired
  if (digitalRead(MOTION)) {
    ledcWrite(SERVO_CHN, 0);
    currentState = PAUSED;
    timePausing = 0;
    addStatus();
    return;
  }
}

void waiting(unsigned long currentDate) {
  time_t timeInSeconds = currentDate;
  struct tm *local_time = localtime(&timeInSeconds);
  unsigned long hourToStart = sys["hourToStart"].as<unsigned long>();
  unsigned long hour = hourToStart % 60;
  unsigned long minutes = hourToStart / 60;

  bool needsToAct = sys["rotation"].as<JsonArray>()[local_time->tm_wday];
  char buffer[80];

  strftime(buffer, sizeof(buffer), "Waiting! %a", local_time);
  printLCD(buffer, String(local_time->tm_hour) + ":" + String(local_time->tm_min) + " " + String(hour) + ":" + String(minutes) + "      ", 2);

  if (!needsToAct)
    return;

  // Starting state
  // TODO Check if shoud act!
  if (local_time->tm_hour == hour) {  //&& local_time->tm_min == minutes
    currentState = RUNNING;
    lastDate = currentDate;
    timeRunning = 0;
    addStatus();
    return;
  }
}

void paused(unsigned long currentDate) {

  timePausing += currentDate - lastDate;
  lastDate = currentDate;

  unsigned long hours = timePausing / 3600;
  unsigned long minutes = (timePausing % 3600) / 60;
  printLCD("Paused!", String(hours) + ":" + String(minutes) + "           ", 2);

  if (digitalRead(MOTION))
    return;

  //Do nothing
  if (timePausing >= TIME_PAUSING * SPEED) {
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

  unsigned long currentDate = getCurrentDate() + millis() / 1000 * SPEED;  //TO SPEED UP

  //Do this periodically
  if (currentDate - lastFetch > TIME_FETCHING) {
    putRequest(BASE_URL SYSTEM_ID PUT_ENDPOINT, sys, cloudMessage);
    addData();
    lastFetch = currentDate;
  }

  //If the system was stopped
  if (!sys["active"])
    return;

  if (currentState == RUNNING)
    running(currentDate);
  else if (currentState == WAITING)
    waiting(currentDate);
  else if (currentState == PAUSED)
    paused(currentDate);
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

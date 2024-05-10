#include "system.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long lcdTime;

DHTesp dht;

//System variables
SystemState currentState;
unsigned long lastDate;
unsigned long timeRunning;
unsigned long timePaused;

unsigned long lastFetch;

void printLCD(String line1, String line2, int duration = 3) {
  lcd.setCursor(0, 0);
  lcd.print(line1 + "           ");
  lcd.setCursor(0, 1);
  lcd.print(line2 + "           ");
  lcd.backlight();
  lcdTime = millis() + duration * 1000;
}

void cloudMessage(DynamicJsonDocument json) {
  sys = json;
  //Serial.println("Data sent/received!");
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
    delay(100);
    Serial.print(".");
  }

  currentState = WAITING;
  Serial.println("\nSystem Started!");
}

void running(unsigned long currentDate, unsigned int state) {
  //Stop state
  if (state == WAITING || timeRunning >= sys["duration"].as<unsigned long>() * 60) {
    currentState = WAITING;
    addStatus(currentDate);
    ledcWrite(SERVO_CHN, 0);
    return;
  }

  timeRunning += currentDate - lastDate;
  lastDate = currentDate;

  unsigned long hours = timeRunning / 3600;
  unsigned long minutes = (timeRunning % 3600) / 60;
  printLCD("Running!", String(hours) + ":" + String(minutes), 2);

  //Run things...
  ledcWrite(SERVO_CHN, 1);

  //Check if the movement sensor fired
  if (state == PAUSED || digitalRead(MOTION)) {
    ledcWrite(SERVO_CHN, 0);
    currentState = PAUSED;
    timePaused = 0;
    addStatus(currentDate);
    return;
  }
}

void waiting(unsigned long currentDate, unsigned int state) {
  time_t timeInSeconds = currentDate;
  struct tm *local_time = localtime(&timeInSeconds);
  unsigned long hourToStart = sys["hourToStart"].as<unsigned long>();
  unsigned long hour = hourToStart % 60;
  unsigned long minutes = hourToStart / 60;

  bool needsToAct = sys["rotation"].as<JsonArray>()[local_time->tm_wday];
  char buffer[80];

  strftime(buffer, sizeof(buffer), "Waiting! %a", local_time);
  printLCD(buffer, String(local_time->tm_hour) + ":" + String(local_time->tm_min), 2);

  if (!needsToAct)
    return;

  // Starting state
  // TODO Check if shoud act!
  if (local_time->tm_hour == hour) {  //&& local_time->tm_min == minutes
    currentState = RUNNING;
    lastDate = currentDate;
    timeRunning = 0;
    addStatus(currentDate);
    return;
  }
}

void paused(unsigned long currentDate, unsigned int state) {

  timePaused += currentDate - lastDate;
  lastDate = currentDate;

  unsigned long hours = timePaused / 3600;
  unsigned long minutes = (timePaused % 3600) / 60;
  printLCD("Paused!", String(hours) + ":" + String(minutes), 2);

  if(state == WAITING) {
    currentState = WAITING;
    addStatus(currentDate);
    ledcWrite(SERVO_CHN, 0);
    return;
  }

  //Keep paused
  if (digitalRead(MOTION))
    return;

  //Back to running mode
  if (timePaused >= TIME_PAUSING) {
    currentState = RUNNING;
    lastDate = currentDate;
    addStatus(currentDate);
    return;
  }
}

void loopSystem() {
  if (lcdTime <= millis()) {
    lcd.noBacklight();
    lcdTime = ULONG_MAX;
  }

  unsigned long currentDate = getCurrentDate() + millis() / 1000 * SPEED;  //TO SPEED UP
  unsigned int state = sys["state"].as<unsigned int>();

  //Do this periodically
  if (currentDate - lastFetch > TIME_FETCHING) {
    putRequest(BASE_URL SYSTEM_ID PUT_ENDPOINT, sys, cloudMessage);
    state = sys["state"].as<unsigned int>();
    addData(currentDate);
    lastFetch = currentDate;
  }

  //If the system was stopped
  if (!sys["active"])
    return;

  if (currentState == RUNNING)
    running(currentDate, state);
  else if (currentState == WAITING)
    waiting(currentDate, state);
  else if (currentState == PAUSED)
    paused(currentDate, state);

  sys["state"] = -1;
}

void addData(unsigned long currentDate) {
  JsonArray jsonArray;
  if (sys.containsKey("data"))
    jsonArray = sys["data"].as<JsonArray>();
  else jsonArray = sys.createNestedArray("data");

  JsonObject root = jsonArray.createNestedObject();
  root["temp"] = dht.getTemperature();
  root["hum"] = dht.getHumidity();
  root["t"] = currentDate;
}

void addStatus(unsigned long currentDate) {
  JsonArray jsonArray;
  if (sys.containsKey("status"))
    jsonArray = sys["status"].as<JsonArray>();
  else jsonArray = sys.createNestedArray("status");

  JsonObject root = jsonArray.createNestedObject();
  root["status"] = currentState;
  root["t"] = currentDate;
}

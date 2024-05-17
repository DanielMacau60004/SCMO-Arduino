#include "esp32-hal.h"
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

unsigned long newDate;
unsigned long newMilli;

unsigned int lastAction;

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

void cloudEmpty(DynamicJsonDocument json) {}

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
 
  Serial.println("\nDeleting old data!");
  getRequest(BASE_URL SYSTEM_ID RESET_ENDPOINT, cloudEmpty);
  currentState = WAITING;
  Serial.println("\nSystem Started!");

  newDate = getCurrentDate();
  newMilli = millis();
}

void running(unsigned int state) {
  //Remote stop
  if(state == WAITING) {
    currentState = WAITING;
    addStatus(CANCELED_USER);
    ledcWrite(SERVO_CHN, 0);
    return;
  }

  //Normal Stop
  if (state == WAITING || timeRunning >= sys["duration"].as<unsigned long>() * 60) {
    currentState = WAITING;
    addStatus(currentState);
    ledcWrite(SERVO_CHN, 0);
    return;
  }

  timeRunning += newDate - lastDate;
  lastDate = newDate;

  unsigned long minutes = (timeRunning % 3600) / 60;
  printLCD("Running!", String(minutes) + " mins", 2);

  //Run things...
  ledcWrite(SERVO_CHN, 1);

  //Check if the movement sensor fired
  if (state == PAUSED || digitalRead(MOTION)) {
    ledcWrite(SERVO_CHN, 0);
    currentState = PAUSED;
    timePaused = 0;
    addStatus(currentState);
    return;
  }
}

void waiting(unsigned int state) {
  time_t timeInSeconds = newDate;
  struct tm *local_time = localtime(&timeInSeconds);
  unsigned long hourToStart = sys["hourToStart"].as<unsigned long>();
  unsigned long hour = hourToStart % 60;
  unsigned long minutes = hourToStart / 60;

  bool needsToAct = sys["rotation"].as<JsonArray>()[local_time->tm_wday];
  char buffer[80];

  strftime(buffer, sizeof(buffer), "Waiting! %a", local_time);
  printLCD(buffer, String(local_time->tm_hour) + ":" + String(local_time->tm_min)+ " T:" + String((int)dht.getTemperature()) + " H:" + String((int)dht.getHumidity()) + "%", 2);

  if (!needsToAct)
    return;

  // Starting state
  if (local_time->tm_hour == hour && local_time->tm_yday > lastAction) {
    if (dht.getHumidity() > HUMIDITY_THRESHOLD || dht.getTemperature() < TEMPERATURE_THRESHOLD) {  //Check weather conditions
      addStatus(CANCELED_SYSTEM);
      return;
    }
    currentState = RUNNING;

    lastAction = local_time->tm_yday;
    newDate = (newDate / (24 *60 *60)) * (24 *60 *60) + hourToStart * 60;
    lastDate = newDate;
    timeRunning = 0;
    addStatus(currentState);
    return;
  }
}

void paused(unsigned int state) {

  timePaused += newDate - lastDate;
  lastDate = newDate;

  unsigned long minutes = (timePaused % 3600) / 60;
  printLCD("Paused!", String(minutes) + " mins", 2);

  if (state == WAITING) {
    currentState = WAITING;
    addStatus(currentState);
    ledcWrite(SERVO_CHN, 0);
    return;
  }

  //Keep paused
  if (digitalRead(MOTION))
    return;

  //Back to running mode
  if (timePaused >= TIME_PAUSING) { //TODO change TIME_PAUSING
    currentState = RUNNING;
    lastDate = newDate;
    addStatus(currentState);
    return;
  }
}

void loopSystem() {
  if (lcdTime <= millis()) {
    lcd.noBacklight();
    lcdTime = ULONG_MAX;
  }

  unsigned long diff = millis() - newMilli;
  newMilli = millis();

  int speedUp = SPEED_UP_ACTING;
  if(currentState == WAITING)
    speedUp = SPEED_UP_WAITING;

  newDate += (speedUp * 60 * diff)/1000; //TODO change 30
  unsigned int state = sys["state"].as<unsigned int>();
  unsigned long currentDate = getCurrentDate();

  //Do this periodically
  if (currentDate - lastFetch > TIME_FETCHING) {
    sys["lastUpdate"] = currentDate;
    sys["currentState"] = currentState;
    putRequest(BASE_URL SYSTEM_ID PUT_ENDPOINT, sys, cloudMessage);
    state = sys["state"].as<unsigned int>();
    addData();
    lastFetch = currentDate;
    newMilli = millis();
  }

  //If the system was stopped
  if (!sys["active"])
    return;

  if (currentState == RUNNING)
    running(state);
  else if (currentState == WAITING)
    waiting(state);
  else if (currentState == PAUSED)
    paused(state);

  sys["state"] = -1;
}

void addData() {
  JsonArray jsonArray;
  if (sys.containsKey("data"))
    jsonArray = sys["data"].as<JsonArray>();
  else jsonArray = sys.createNestedArray("data");

  JsonObject root = jsonArray.createNestedObject();
  root["temp"] = dht.getTemperature();
  root["hum"] = dht.getHumidity();
  root["t"] = newDate;
}

void addStatus(SystemState state) {
  JsonArray jsonArray;
  if (sys.containsKey("status"))
    jsonArray = sys["status"].as<JsonArray>();
  else jsonArray = sys.createNestedArray("status");

  JsonObject root = jsonArray.createNestedObject();
  root["status"] = state;
  root["t"] = newDate;
}

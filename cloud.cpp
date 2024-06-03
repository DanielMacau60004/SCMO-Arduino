#include "HardwareSerial.h"
#include "cloud.h"

unsigned long currentDate = 0;
String timezone = "";

/*
* Create a get request to the cloud
*
* const char* url -> request url
* fetchFunction fun    -> function to handle the data returned
*/
void getRequest(const char* url, fetchFunction fun) {
  if (WiFi.status() != WL_CONNECTED)
    return;

  String requestURL = url;
  HTTPClient http;
  http.begin(requestURL);

  int httpResponseCode = http.GET();
  //Serial.println(httpResponseCode);
  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();
    int responseLength = http.getSize();

    DynamicJsonDocument jsonBuffer(responseLength + 1);
    DeserializationError error = deserializeJson(jsonBuffer, response);

    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      return;
    }

    fun(jsonBuffer);
  }

  http.end();
}

/*
* Create a post request to the cloud
*
* const char* url -> request url
* JsonObject& obj      -> json object to be sent
* fetchFunction fun    -> function to handle the data returned
*/
void postRequest(const char* url, DynamicJsonDocument& obj, fetchFunction fun) {
  if (WiFi.status() != WL_CONNECTED) return;

  String requestURL = url;
  HTTPClient http;
  http.begin(requestURL);
  http.addHeader("Content-Type", "application/json");

  String jsonString;
  serializeJson(obj, jsonString);

  int httpResponseCode = http.POST(jsonString);
  //Serial.println(httpResponseCode);
  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();
    int responseLength = http.getSize();

    DynamicJsonDocument jsonBuffer(responseLength + 1);
    DeserializationError error = deserializeJson(jsonBuffer, response);

    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      return;
    }

    fun(jsonBuffer);
  }

  http.end();
}

void putRequest(const char* url, DynamicJsonDocument& obj, fetchFunction fun) {
  if (WiFi.status() != WL_CONNECTED) return;

  String requestURL = url;
  HTTPClient http;
  http.begin(requestURL);
  http.addHeader("Content-Type", "application/json");

  String jsonString;
  serializeJson(obj, jsonString);

  int httpResponseCode = http.PUT(jsonString);
  //Serial.println(httpResponseCode);
  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();
    int responseLength = http.getSize();

    DynamicJsonDocument jsonBuffer(responseLength + 1);
    DeserializationError error = deserializeJson(jsonBuffer, response);

    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      return;
    }

    fun(jsonBuffer);
  }

  http.end();
}

void updateCurrentDate(DynamicJsonDocument json) {
  unsigned long date = json["unixtime"].as<unsigned long>() + json["dst_offset"].as<unsigned long>();
  timezone = json["timezone"].as<String>();
  currentDate = date - millis() / 1000;

  Serial.print("Current date: ");
  Serial.println(currentDate);
}

String getTimeZone() {
  return timezone;
}

void fetchCurrentDate() {
  getRequest(URL_DATETIME_UTC, updateCurrentDate);
}

unsigned long getCurrentDate() {
  return currentDate + millis() / 1000;
}

#include "cloud.h"

const char* URL = "http://85.244.117.107";
const char* END_POINT_GET = "/test/McMacau";
const char* END_POINT_POST = "/test";

void handleGet(DynamicJsonDocument json) {
  String dataValue = json["result"]["uuid"];
  Serial.print("UUID: ");
  Serial.println(dataValue);
}

// Temporary method...
void initCloud() {

  //Get request
  Serial.println("\nPerform GET");
  getRequest(END_POINT_GET, handleGet);

  //Post request
  Serial.println("Perform POST");
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["uuid"] = "1212";
  root["username"] = "daniel";
  root["password"] = "pwd32132";
  root["ip"] = "23.212.213.21";
  postRequest(END_POINT_POST, root, handleGet);
  
}

/*
* Create a get request to the cloud
*
* const char* endpoint -> request endpoint
* fetchFunction fun    -> function to handle the data returned
*/
void getRequest(const char* endpoint, fetchFunction fun) {
  if (WiFi.status() != WL_CONNECTED)
    return;

  String requestURL = String(URL) + endpoint;
  HTTPClient http;
  http.begin(requestURL);

  int httpResponseCode = http.GET();
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
* const char* endpoint -> request endpoint
* JsonObject& obj      -> json object to be sent
* fetchFunction fun    -> function to handle the data returned
*/
void postRequest(const char* endpoint, JsonObject& obj, fetchFunction fun) {
  if (WiFi.status() != WL_CONNECTED)
    return;

  String requestURL = String(URL) + endpoint;
  HTTPClient http;
  http.begin(requestURL);
  http.addHeader("Content-Type", "application/json");

  String jsonString;
  serializeJson(obj, jsonString);
  int httpResponseCode = http.POST(jsonString);

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

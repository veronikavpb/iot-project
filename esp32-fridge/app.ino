#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

#define SENSOR_PIN 4

const char* serverUrl = "http://10.245.98.87:3000/door-event"; //phone ip: 10.245.98.87 laptop ip: 192.168.129.2

// Adjust these if your sensor logic is reversed
const int DOOR_CLOSED_STATE = LOW;
const int DOOR_OPEN_STATE = HIGH;

int lastState = -1;
unsigned long doorOpenTime = 0;
bool alertShown = false;

void sendDoorEvent(const char* status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String json = "{\"device\":\"esp32-fridge\",\"status\":\"" + String(status) + "\"}";

    int httpResponseCode = http.POST(json);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    String response = http.getString();
    Serial.println(response);
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Still connecting...");
  }

  Serial.println("Connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  int state = digitalRead(SENSOR_PIN);

  if (state != lastState) {
    if (state == DOOR_OPEN_STATE) {
      Serial.println("DOOR OPEN");
      sendDoorEvent("OPEN");
      doorOpenTime = millis();
      alertShown = false;
    } 
    else if (state == DOOR_CLOSED_STATE) {
      Serial.println("DOOR CLOSED");
      sendDoorEvent("CLOSED");
      doorOpenTime = 0;
      alertShown = false;
    }

    lastState = state;
  }

  if (state == DOOR_OPEN_STATE && !alertShown) {
    if (millis() - doorOpenTime >= 15000) {
      Serial.println("ALERT: DOOR OPEN TOO LONG");
      sendDoorEvent("ALERT_OPEN_TOO_LONG");
      alertShown = true;
    }
  }

  delay(100);
}
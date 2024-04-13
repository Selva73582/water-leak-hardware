#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

SoftwareSerial uno1Serial(D5, D6);  

const char* ssid = "pugal";
const char* password = "heaven123";
const String serverName = "http://192.168.227.28:3001/sensor-data/";

void setup() {
  Serial.begin(9600);
  uno1Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (uno1Serial.available()) {
    processArduinoData(uno1Serial, "NOD1");
  }

  
}

void processArduinoData(SoftwareSerial& arduinoSerial, const char* nodeIdentifier) {
  // Read data from Arduino
  String data = arduinoSerial.readStringUntil('\n');
  Serial.print(data);
  // Create a WiFiClient object to use for the HTTPClient
  WiFiClient client;

  // Send HTTP POST request to server
  HTTPClient http;
  http.begin(client, serverName);  // Specify the URL of your Node.js server
  http.addHeader("Content-Type", "application/json");

  
  int httpResponseCode = http.POST(data);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error sending POST request. HTTP error code: ");
    Serial.println(httpResponseCode);
  }

  http.end(); 
}

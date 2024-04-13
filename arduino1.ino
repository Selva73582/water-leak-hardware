#include <ArduinoJson.h>
#include <SoftwareSerial.h>

const int sensorPinFloor3 = 2;  
const int sensorPinFloor4 = 3;  

volatile unsigned int pulseCountFloor3 = 0;
volatile unsigned int pulseCountFloor4 = 0;

unsigned long lastPulseTimeFloor3 = 0;
unsigned long lastPulseTimeFloor4 = 0;

unsigned long previousMillis = 0;
const int interval = 10000;  
const float pulsesPerLiter = 4850.0; 

float totalVolumeFloor3 = 0.0;
float totalVolumeFloor4 = 0.0;


SoftwareSerial nodeMcuSerial(10, 11);  

void setup() {
  Serial.begin(9600);

  // Initialize SoftwareSerial for communication with NodeMCU (MCU2)
  nodeMcuSerial.begin(9600);

  pinMode(sensorPinFloor3, INPUT_PULLUP);
  pinMode(sensorPinFloor4, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(sensorPinFloor3), countPulseFloor3, FALLING);
  attachInterrupt(digitalPinToInterrupt(sensorPinFloor4), countPulseFloor4, FALLING);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    detachInterrupts();

    // Calculate flow rate and volume for each floor
    float flowRateFloor3 = (pulseCountFloor3 / pulsesPerLiter) * 60.0;
    float litersPassedFloor3 = (flowRateFloor3 / 60.0) * (interval / 1000.0);
    totalVolumeFloor3 += litersPassedFloor3;

    float flowRateFloor4 = (pulseCountFloor4 / pulsesPerLiter) * 60.0;
    float litersPassedFloor4 = (flowRateFloor4 / 60.0) * (interval / 1000.0);
    totalVolumeFloor4 += litersPassedFloor4;

    sendJSONData(totalVolumeFloor3, totalVolumeFloor4);

    // Reset total volume variables
    totalVolumeFloor3 = 0.0;
    totalVolumeFloor4 = 0.0;

    resetVariablesAndAttachInterrupts();
    previousMillis = currentMillis;
  }
}

void sendJSONData(float totalVolumeFloor3, float totalVolumeFloor4) {
  DynamicJsonDocument doc(400);
  JsonArray jsonArray = doc.to<JsonArray>();

  JsonObject floor3Object = jsonArray.createNestedObject();
  floor3Object["apartmentId"] = "65b0de6cee33a7c611308669";
  floor3Object["sensorId"] = 3;
  floor3Object["volume"] = totalVolumeFloor3;
  floor3Object["averageFlowRate"] = (pulseCountFloor3 / pulsesPerLiter) * 60.0;
  floor3Object["floorNumber"] = 3;

  JsonObject floor4Object = jsonArray.createNestedObject();
  floor4Object["apartmentId"] = "65b0de6cee33a7c611308669";
  floor4Object["sensorId"] = 4;
  floor4Object["volume"] = totalVolumeFloor4;
  floor4Object["averageFlowRate"] = (pulseCountFloor4 / pulsesPerLiter) * 60.0;
  floor4Object["floorNumber"] = 4;

  // Serialize JSON to string
  String jsonString;
  serializeJson(doc, jsonString);

  // Send JSON data to NodeMCU (mcu2) via SoftwareSerial
  nodeMcuSerial.println(jsonString);  // Print a newline to indicate the end of data
}

void resetVariablesAndAttachInterrupts() {
  pulseCountFloor3 = 0;
  pulseCountFloor4 = 0;

  attachInterrupt(digitalPinToInterrupt(sensorPinFloor3), countPulseFloor3, FALLING);
  attachInterrupt(digitalPinToInterrupt(sensorPinFloor4), countPulseFloor4, FALLING);
}

void detachInterrupts() {
  detachInterrupt(digitalPinToInterrupt(sensorPinFloor3));
  detachInterrupt(digitalPinToInterrupt(sensorPinFloor4));
}

void countPulseFloor3() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastPulseTimeFloor3 > 50) {
    pulseCountFloor3++;
    lastPulseTimeFloor3 = currentMillis;
  }
}

void countPulseFloor4() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastPulseTimeFloor4 > 50) {
    pulseCountFloor4++;
    lastPulseTimeFloor4 = currentMillis;
  }
}

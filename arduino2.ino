#include <ArduinoJson.h>
#include <SoftwareSerial.h>

const int sensorPinFloor1 = 2;  // Connect the flow sensor for Floor 3 to digital pin 2
const int sensorPinFloor2 = 3;  // Connect the flow sensor for Floor 4 to digital pin 3

volatile unsigned int pulseCountFloor1 = 0;
volatile unsigned int pulseCountFloor2 = 0;

unsigned long lastPulseTimeFloor1 = 0;
unsigned long lastPulseTimeFloor2 = 0;

unsigned long previousMillis = 0;
const int interval = 10000;  // Interval for calculations and data transmission (in milliseconds)

const float pulsesPerLiter = 4850.0;  // Adjust as per your sensor specifications

float totalVolumeFloor1 = 0.0;
float totalVolumeFloor2 = 0.0;

// SoftwareSerial for communication with NodeMCU (MCU2)
SoftwareSerial nodeMcuSerial(10, 11);  // RX, TX for communication with NodeMCU (MCU2)

void setup() {
  Serial.begin(9600);

  // Initialize SoftwareSerial for communication with NodeMCU (MCU2)
  nodeMcuSerial.begin(9600);

  pinMode(sensorPinFloor1, INPUT_PULLUP);
  pinMode(sensorPinFloor2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(sensorPinFloor1), countPulseFloor1, FALLING);
  attachInterrupt(digitalPinToInterrupt(sensorPinFloor2), countPulseFloor2, FALLING);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    detachInterrupts();

    // Calculate flow rate and volume for each floor
    float flowRateFloor1 = (pulseCountFloor1 / pulsesPerLiter) * 60.0;
    float litersPassedFloor1 = (flowRateFloor1 / 60.0) * (interval / 1000.0);
    totalVolumeFloor1 += litersPassedFloor1;

    float flowRateFloor2 = (pulseCountFloor2 / pulsesPerLiter) * 60.0;
    float litersPassedFloor2 = (flowRateFloor2 / 60.0) * (interval / 1000.0);
    totalVolumeFloor2 += litersPassedFloor2;

    sendJSONData(totalVolumeFloor1, totalVolumeFloor2);

    // Reset total volume variables
    totalVolumeFloor1 = 0.0;
    totalVolumeFloor2 = 0.0;

    resetVariablesAndAttachInterrupts();
    previousMillis = currentMillis;
  }
}

void sendJSONData(float totalVolumeFloor3, float totalVolumeFloor4) {
  DynamicJsonDocument doc(400);
  JsonArray jsonArray = doc.to<JsonArray>();

  JsonObject floor3Object = jsonArray.createNestedObject();
  floor3Object["apartmentId"] = "65b0de6cee33a7c611308669";
  floor3Object["sensorId"] = 1;
  floor3Object["volume"] = totalVolumeFloor1;
  floor3Object["averageFlowRate"] = (pulseCountFloor1 / pulsesPerLiter) * 60.0;
  floor3Object["floorNumber"] = 1;

  JsonObject floor4Object = jsonArray.createNestedObject();
  floor4Object["apartmentId"] = "65b0de6cee33a7c611308669";
  floor4Object["sensorId"] = 2;
  floor4Object["volume"] = totalVolumeFloor2;
  floor4Object["averageFlowRate"] = (pulseCountFloor2 / pulsesPerLiter) * 60.0;
  floor4Object["floorNumber"] = 2;

  // Serialize JSON to string
  String jsonString;
  serializeJson(doc, jsonString);

  // Send JSON data to NodeMCU (mcu2) via SoftwareSerial
  nodeMcuSerial.println(jsonString);  // Print a newline to indicate the end of data
}

void resetVariablesAndAttachInterrupts() {
  pulseCountFloor1 = 0;
  pulseCountFloor2 = 0;

  attachInterrupt(digitalPinToInterrupt(sensorPinFloor1), countPulseFloor1, FALLING);
  attachInterrupt(digitalPinToInterrupt(sensorPinFloor2), countPulseFloor2, FALLING);
}

void detachInterrupts() {
  detachInterrupt(digitalPinToInterrupt(sensorPinFloor1));
  detachInterrupt(digitalPinToInterrupt(sensorPinFloor2));
}

void countPulseFloor1() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastPulseTimeFloor1 > 50) {
    pulseCountFloor1++;
    lastPulseTimeFloor1 = currentMillis;
  }
}

void countPulseFloor2() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastPulseTimeFloor2 > 50) {
    pulseCountFloor2++;
    lastPulseTimeFloor2 = currentMillis;
  }
}

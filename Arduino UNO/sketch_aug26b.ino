#define RLOAD 22.0
// Calibration resistance at atmospheric CO2 level
#define RZERO 400 

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

#include "MQ135.h" 
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>

Adafruit_BMP280 bmp; // I2C

MQ135 gasSensor = MQ135(A0); 

StaticJsonDocument<200> doc;

int val; 
int sensorPin = A0; 
int sensorValue = 0; 
void setup() { 
  Serial.begin(9600);

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  
  pinMode(sensorPin, INPUT); 
} 
 
void loop() {
  doc["raw"] = analogRead(A0);
  doc["rzero"] = gasSensor.getRZero();
  doc["pm25"] = gasSensor.getPPM();

  doc["temperature"] = bmp.readTemperature();
  doc["pressure"] = bmp.readPressure();
  doc["altitude"] = bmp.readAltitude(1013.25);
  
  serializeJson(doc, Serial); 
  //Serial.println();
  delay(10000); 
} 

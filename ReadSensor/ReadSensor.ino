/*
  MKR THERM Shield - Read Sensors

  This example reads the temperatures measured by the thermocouple
  connected to the MKR THERM shield and prints them to the Serial Monitor
  once a second.
  
  The circuit:
  - Arduino MKR board
  - Arduino MKR THERM Shield attached
  - A K Type thermocouple temperature sensor connected to the shield
  
  MKR ENV Shield - Read Sensors

  This example reads the sensors on-board the MKR ENV shield
  and prints them to the Serial Monitor once a second.

  The circuit:
  - Arduino MKR board
  - Arduino MKR ENV Shield attached

  This example code is in the public domain.
  This example code is in the public domain.
*/

#include <Arduino_MKRTHERM.h>
#include <Arduino_MKRENV.h>

void setup() {

  Serial.begin(9600);

  while (!Serial);

  if (!THERM.begin()) {
    Serial.println("Failed to initialize MKR THERM shield!");
    while (1);
  }

  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV shield!");
    while (1);
  }
}

void loop() {

  // read all the sensor values
  float temperature = ENV.readTemperature();
  float humidity    = ENV.readHumidity();
  float pressure    = ENV.readPressure();
  float illuminance = ENV.readIlluminance();
  float uva         = ENV.readUVA();
  float uvb         = ENV.readUVB();
  float uvIndex     = ENV.readUVIndex();

  float tctemperature = THERM.readTemperature()
  float tcreftemp = THERM.readReferenceTemperature()

  // print each of the sensor values
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity    = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Pressure    = ");
  Serial.print(pressure);
  Serial.println(" kPa");

  Serial.print("Illuminance = ");
  Serial.print(illuminance);
  Serial.println(" lx");

  Serial.print("UVA         = ");
  Serial.println(uva);

  Serial.print("UVB         = ");
  Serial.println(uvb);

  Serial.print("UV Index    = ");
  Serial.println(uvIndex);

  // print an empty line
  Serial.println();
  
  Serial.print("Reference temperature ");
  Serial.print(tcreftemp);
  Serial.println(" °C");

  Serial.print("Temperature ");
  Serial.print(tctemperature);
  Serial.println(" °C");

  // print an empty line
  Serial.println();

  // wait 1 second to print again
  delay(1000);
}

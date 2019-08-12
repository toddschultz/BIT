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

  WriteMultipleFields
  
  Description: Writes values to fields 1,2,3,4 and status in a single ThingSpeak update every 20 seconds.
  
  Hardware: Arduino MKR WiFi 1010
  
  !!! IMPORTANT - Modify the secrets.h file for this project with your network connection and ThingSpeak channel details. !!!
  
  Note:
  - Requires WiFiNINA library
  - This example is written for a network using WPA encryption. For WEP or WPA, change the WiFi.begin() call accordingly.
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize, and 
  analyze live data streams in the cloud. Visit https://www.thingspeak.com to sign up for a free account and create a channel.  
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the README.md folder where the library was installed.
  See https://www.mathworks.com/help/thingspeak/index.html for the full ThingSpeak documentation.
  
  For licensing information, see the accompanying license file.
  
  Copyright 2018, The MathWorks, Inc.
*/

#include <Arduino_MKRTHERM.h>
#include <Arduino_MKRENV.h>
#include "ThingSpeak.h"
#include <WiFiNINA.h>

#include "secrets.h" 

char ssid[] = SECRET_SSID;     // your network SSID (name)
char password[] = SECRET_PWD;  // your network key

WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize values
String myStatus = "";
float limTempUp = 135; // Upper temperature range for smoker
float limTempLow = 105; // Lower temperature range for smoker
unsigned long time; // Time in milliseconds since program started

void setup() {

  Serial.begin(115200);

  while (!Serial);

  if (!THERM.begin()) {
    Serial.println("Failed to initialize MKR THERM shield!");
    while (1);
  }

  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV shield!");
    while (1);
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  ThingSpeak.begin(client);  //Initialize ThingSpeak
}

void loop() {

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, password); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  
  // read all the sensor values
  float temperature = ENV.readTemperature();
  float humidity    = ENV.readHumidity();
  float pressure    = ENV.readPressure();
  float illuminance = ENV.readIlluminance();
  float uva         = ENV.readUVA();
  float uvb         = ENV.readUVB();
  float uvIndex     = ENV.readUVIndex();

  float smokertemperature = THERM.readTemperature();
  float smokerreftemp = THERM.readReferenceTemperature();

  // set the fields with the values
  //Channel BBQed Data
  // Fields Env pressure (kPa), Env RH (%), Env Temp (C), Smoker Temp (C)
  ThingSpeak.setField(1, pressure);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, temperature);
  ThingSpeak.setField(4, smokertemperature);
  
  // Set status message based on smoker temperature
  if (smokertemperature > limTempUp){
    myStatus = String("Smoker running too hot!");
  }
  else if (smokertemperature < limTempLow){
    myStatus = String("Smoker running too cold!");
  }
  else {
    mySatus = String("Smoker temperature running normal.");
  }
  ThingSpeak.setStatus(myStatus);

  time = millis();
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }  
  
  // print each of the sensor values
  Serial.print("Env Temperature = ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Env Humidity    = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Env Pressure    = ");
  Serial.print(pressure);
  Serial.println(" kPa");

  Serial.print("Env Illuminance = ");
  Serial.print(illuminance);
  Serial.println(" lx");

  Serial.print("Env UVA         = ");
  Serial.println(uva);

  Serial.print("Env UVB         = ");
  Serial.println(uvb);

  Serial.print("UV Index    = ");
  Serial.println(uvIndex);

  // print an empty line
  Serial.println();
  
  Serial.print("Reference temperature ");
  Serial.print(smokerreftemp);
  Serial.println(" °C");

  Serial.print("Temperature ");
  Serial.print(smokertemperature);
  Serial.println(" °C");

  // print an empty line
  Serial.println();

  // wait 30 second to print again
  delay(30*1000);
}

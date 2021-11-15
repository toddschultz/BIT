/*
  BASEBBQDATAWTIME - log Arduino BBQ data to ThingSpeak with time control
  baseBBQDatawTime.ino

  Built from:
  Time_SerialMonitor.ino

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
// #include <ArduinoLowPower.h>
#include "ThingSpeak.h"
#include <WiFiNINA.h>
#include <RTCZero.h>

#include "secrets.h" 
char ssid[] = SECRET_SSID;     // your network SSID (name)
char password[] = SECRET_PWD;  // your network key

// USER SETTINGS
const int GMT = 0; //change this to adapt it to your time zone, Seattle -7
const int dateOrder = 1;  // 1 = MDY; 0 for DMY
// END USER SETTINGS

WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

RTCZero rtc; // create instance of real time clock
int myhours, mins, secs, myday, mymonth, myyear;

// Initialize values
String myStatus = "";
float limTempUp = 135; // Upper temperature range for smoker
float limTempLow = 105; // Lower temperature range for smoker
unsigned long time; // Time in milliseconds since program started

void setup() {

  Serial.begin(115200);
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
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
  
  // while (!Serial);
  // Initialize shields
  if (!THERM.begin()) {
    Serial.println("Failed to initialize MKR THERM shield!");
    while (1);
  }

  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV shield!");
    while (1);
  }

  //Initialize ThingSpeak  
  ThingSpeak.begin(client); 

  // wait 1 second for sensors to settle
  delay(1*1000);

// Initialize real time clock
  rtc.begin();
  setRTC();  // get Epoch time from Internet Time Service
  fixTimeZone();
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
  
  // update date/time
  //myyear = rtc.getYear();
  //mymonth = rtc.getMonth();
  //myday = rtc.getDay();
  //myhours = rtc.getHours();
  //mins = rtc.getMinutes();
  //secs == rtc.getSeconds();
  
// Request seconds and deal with date/time
  secs = rtc.getSeconds();
  if (secs == 0) fixTimeZone(); // when secs is 0, update everything and correct for time zone
  // otherwise everything else stays the same.
  printDate();
  printTime();
  Serial.println();
  if (secs == 15 || secs == 45) {
    Serial.println("15 or 45 seconds!\n");
  }
  while (secs == rtc.getSeconds()) delay(10); // wait until seconds change
  if (mins==59 && secs ==0) setRTC(); // get NTP time every hour at minute 59

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
    myStatus = String("Smoker temperature running normal.");
  }
  ThingSpeak.setStatus(myStatus);

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


void setRTC() { // get the time from Internet Time Service
  unsigned long epoch;
  int numberOfTries = 0, maxTries = 6;
  do {
    epoch = WiFi.getTime(); // The RTC is set to GMT or 0 Time Zone and stays at GMT.
    numberOfTries++;
  }
  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {
    Serial.print("NTP unreachable!!");
    while (1);  // hang
  }
  else {
    Serial.print("Epoch Time = ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);
    Serial.println();
  }
}

void printDate()
{
  if (dateOrder == 0) {
    Serial.print(myday);
    Serial.print("/");
  }
  Serial.print(mymonth);
  Serial.print("/");
  if (dateOrder == 1) {
    Serial.print(myday);
    Serial.print("/");
  }
  Serial.print("20");
  Serial.print(myyear);
  Serial.print(" ");
}

void printTime()
{
  print2digits(myhours);
  Serial.print(":");
  print2digits(mins);
  Serial.print(":");
  print2digits(secs);
  Serial.println();
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

/* There is more to adjusting for time zone that just changing the hour.
   Sometimes it changes the day, which sometimes chnages the month, which
   requires knowing how many days are in each month, which is different
   in leap years, and on Near Year's Eve, it can even change the year! */
void fixTimeZone() {
  int daysMon[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (myyear % 4 == 0) daysMon[2] = 29; // fix for leap year
  myhours = rtc.getHours();
  mins = rtc.getMinutes();
  myday = rtc.getDay();
  mymonth = rtc.getMonth();
  myyear = rtc.getYear();
  myhours +=  GMT; // initial time zone change is here
  if (myhours < 0) {  // if hours rolls negative
    myhours += 24; // keep in range of 0-23
    myday--;  // fix the day
    if (myday < 1) {  // fix the month if necessary
      mymonth--;
      if (mymonth == 0) mymonth = 12;
      myday = daysMon[mymonth];
      if (mymonth == 12) myyear--; // fix the year if necessary
    }
  }
  if (myhours > 23) {  // if hours rolls over 23
    myhours -= 24; // keep in range of 0-23
    myday++; // fix the day
    if (myday > daysMon[mymonth]) {  // fix the month if necessary
      mymonth++;
      if (mymonth > 12) mymonth = 1;
      myday = 1;
      if (mymonth == 1) myyear++; // fix the year if necessary
    }
  }
}
/*
  MEATBBQDATAWTIME - log Arduino BBQ data to ThingSpeak with time control
  meatBBQDatawTime.ino

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
#include "ThingSpeak.h"
#include <WiFiNINA.h>
#include <RTCZero.h>

#include "secrets.h" 
char ssid[] = SECRET_SSID;     // your network SSID (name)
char password[] = SECRET_PWD;  // your network key
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// USER SETTINGS
const int GMT = 0; //change this to adapt it to your time zone, Seattle -7
const int dateOrder = 1;  // 1 = MDY; 0 for DMY
// END USER SETTINGS

WiFiClient client;

RTCZero rtc; // create instance of real time clock
int myhours, mins, secs, myday, mymonth, myyear;

// Initialize values
String myStatus = "";
float limTempUp = 57; // 135 C doneness temperature for salmon

// Sensor variables for running sum
int ncounter = 0; // Loop counter
float meattemperature = 0;

void setup() {

  // Initialize WiFi    
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
  
  // Reconnect to WiFi if needed
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

  // Get time 
  secs = rtc.getSeconds();
  mins = rtc.getMinutes();
  // Fix time if needed
  if (mins == 59 && secs == 0) setRTC();

  // Get sample of data and add to running sum
  ncounter = ncounter + 1;  //Increment counter
  meattemperature = meattemperature + THERM.readTemperature();

  // Every 60 seconds average and write to ThingSpeak
  if (secs == 30) {
    // Compute average, upload to ThingSpeak, and reset counter and sums
    //Channel BBQed Data
    // Fields Meat Temp (C)
    ThingSpeak.setField(5, meattemperature/(float)ncounter);

    // Set status message based on meat temperature
    if (meattemperature/(float)ncounter >= limTempUp){
      myStatus = String("Salmon is done!");
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

    // Display values
    printDate();
    printTime();

    // print each of the sensor values
    Serial.print("Meat Temperature = ");
    Serial.print(meattemperature/(float)ncounter);
    Serial.println(" °C");
    
    Serial.print("Counter = ");
    Serial.println(ncounter);
    Serial.println(myStatus);    
    Serial.println();
    
    // Reset counter and running sumsS
    ncounter = 0;
    meattemperature = 0;
  }

  // Wait for new second
  while (secs == rtc.getSeconds()) delay(10); // wait until seconds change
}


void setRTC() { // get the time from Internet Time Service
  unsigned long epoch;
  int numberOfTries = 0, maxTries = 10;
  // Reconnect to WiFi if needed
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
  
  do {
    epoch = WiFi.getTime(); // The RTC is set to GMT or 0 Time Zone and stays at GMT.
    numberOfTries++;
  }
  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {
    Serial.print("NTP unreachable!!\n");
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
#include <WiFiNINA.h>
#include <RTCZero.h>
#include <HT16K33.h>

RTCZero rtc; // create instance of real time clock
HT16K33 seg(0x70); // create an instance of 7 segment display

// USER SETTINGS
char ssid[] = "WiFi Network";  // Your WiFi network
char pass[] = "Wifi Password";  // Your WiFi password
const int GMT = -7; //change this to adapt it to your time zone
const int myClock = 12;  // can be 24 hour or 12 hour
// END USER SETTINGS

int status = WL_IDLE_STATUS;
int myhours, mins, secs, myday, mymonth, myyear;
bool IsPM = false;

void setup() {
  while ( status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    delay(6000);
  }
  rtc.begin();
  setRTC();  // get Epoch time from Internet Time Service
  seg.begin();
  Wire.setClock(100000);
  seg.displayOn();
  seg.setDigits(4);
  seg.brightness(15);  // 0-15
  fixTimeZone();
}

void loop() {
  secs = rtc.getSeconds();
  if (secs == 0) fixTimeZone(); // when secs is 0, update everything and correct for time zone
  // otherwise everything else stays the same.;
  seg.displayTime(myhours, mins);
  seg.displayColon(1);
  delay(500);
  seg.displayColon(0);
  while (secs == rtc.getSeconds())delay(10); // wait until seconds change
  if (mins==59 && secs ==0) setRTC(); // get NTP time every hour at minute 59
}

void setRTC() { // get the time from Internet Time Service
  unsigned long epoch;
  int numberOfTries = 0, maxTries = 10;
  do {
    epoch = WiFi.getTime();
    numberOfTries++;
  }
  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {
    while (1);  // hang
  }
  else {
    rtc.setEpoch(epoch);
  }
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
      if (mymonth == 1)myyear++; // fix the year if necessary
    }
  }
  if (myClock == 12) {  // this is for 12 hour clock
    IsPM = false;
    if (myhours > 11)IsPM = true;
    myhours = myhours % 12; // convert to 12 hour clock
    if (myhours == 0) myhours = 12;  // show noon or midnight as 12
  }
}

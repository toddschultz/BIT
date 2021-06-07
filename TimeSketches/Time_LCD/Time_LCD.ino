#include <WiFiNINA.h>
#include <RTCZero.h>
#include <LCD_I2C.h>
#include <avr/dtostrf.h>  // used to convert numbers to strings for display on LCD display

// USER SETTINGS
char ssid[] = "WiFi Network";  // Your WiFi network
char pass[] = "Wifi Password";  // Your WiFi password
const int GMT = -7; //change this to adapt it to your time zone
const int myClock = 12;  // can be 24 or 12 hour clock
const int dateOrder = 1;  // 1 = MDY; 0 for DMY
// END USER SETTINGS

RTCZero rtc; // create instance of real time clock
LCD_I2C lcd(0x27); // Default address of most PCF8574 modules
int status = WL_IDLE_STATUS;
int myhours, mins, secs, myday, mymonth, myyear;
bool IsPM = false;

void setup() {
  while ( status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
  }
  delay(6000);
  rtc.begin();
  setRTC();  // get Epoch time from Internet Time Service
  lcd.begin(); // start LCD
  lcd.backlight();
  fixTimeZone();
}

void loop() {
  secs = rtc.getSeconds();
  if (secs == 0) fixTimeZone(); // when secs is 0, update everything and correct for time zone
  lcd.setCursor(0, 0);          // otherwise everything else stays the same.
  print2digits(myhours);
  lcd.print(":");
  lcd.setCursor(3, 0);
  print2digits(mins);
  lcd.print(":");
  lcd.setCursor(6, 0);
  print2digits(secs);
  if (myClock == 12) {  // add AM or PM if 12 hour clock
    lcd.setCursor(9, 0);
    if (IsPM)lcd.print("PM");
    else lcd.print("AM");
  }
  if (dateOrder == 1) {
    lcd.setCursor(0, 1);
    printMonth(mymonth);
    lcd.setCursor(4, 1);
    print2digits(myday);
  }
  else {
    lcd.setCursor(0, 1);
    print2digits(myday);
    lcd.setCursor(3, 1);
    printMonth(mymonth);
  }
  lcd.setCursor(6, 1);
  lcd.print(", 20");
  lcd.print(myyear);
  while (secs == rtc.getSeconds())delay(10); // wait until seconds change
  lcd.clear();
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
    rtc.setEpoch(epoch); // The RTC is set to GMT or 0 Time Zone and stays at GMT.
  }
}

void print2digits(int number) { // adds a leading 0 if number is less than 10
  if (number < 10) {
    lcd.print("0");
    char mydigit[1];
    dtostrf(number, 1, 0, mydigit); // convert hours to a string
    lcd.print(mydigit);
  }
  else {
    char mydigits[2];
    dtostrf(number, 2, 0, mydigits); // convert hours to a string
    lcd.print(mydigits);
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

void printMonth(int number) {  // print abbreviation of month instead of number
  switch (number) {
    case 1:
      lcd.print("Jan");
      break;
    case 2:
      lcd.print("Feb");
      break;
    case 3:
      lcd.print("Mar");
      break;
    case 4:
      lcd.print("Apr");
      break;
    case 5:
      lcd.print("May");
      break;
    case 6:
      lcd.print("Jun");
      break;
    case 7:
      lcd.print("Jul");
      break;
    case 8:
      lcd.print("Aug");
      break;
    case 9:
      lcd.print("Sep");
      break;
    case 10:
      lcd.print("Oct");
      break;
    case 11:
      lcd.print("Nov");
      break;
    case 12:
      lcd.print("Dec");
      break;
  }
}

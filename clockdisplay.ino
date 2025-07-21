#include <WiFi.h>
#include <TM1637Display.h>
#include <time.h>
#include <RTClib.h>

#define CLK 15
#define DIO 4

RTC_DS3231 rtc;
TM1637Display display(CLK, DIO);

const char *ssid = "";
const char *password = "";
const char *ntpServer1 = "";
const char *ntpServer2 = "";
const long gmtOffset_sec = -21600;
const int daylightOffset_sec = 3600;

void setup()
{
  Serial.begin(115200);
  randomSeed(analogRead(19));           // unused pin for random noise, this is for setting the rtc sync interval later
  initdevices();                        // initialize the rtc and display
  syncrtc();                            // perform initial sync with ntp and set rtc 
}

void loop()
{
  DateTime now = rtc.now();             // display
  int displaytime = (now.hour() * 100) + now.minute();
  display.showNumberDecEx(displaytime, 0b11100000, true);
  printLocalTime();
  delay(900);
}

void initdevices()
{
  if (! rtc.begin()){                   // check if rtc is present
  Serial.println("Couldn't find RTC");
  Serial.flush();
  while (1) delay(10);
  }
  rtc.disable32K();                     // disable the 32K pin, we don't need it
  rtc.clearAlarm(1);                    // clear out both alarms
  rtc.clearAlarm(2);                    
  rtc.writeSqwPinMode(DS3231_OFF);      // disable the square wave pin, we will monitor alarm state using rtc.alarmFired(1)
  rtc.disableAlarm(2);                  // disable alarm 2

  display.clear();                      // clear the display, set the brightness, set to 0000, then clear again
  display.setBrightness(3);
  display.showNumberDec(0, true);
  display.clear();
}

void connectwifi()
{
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" WiFi connected.");
}

void syncrtc()
{
  connectwifi();                        // (re)connect to wifi
  int ntpinterval = random(6,23);       // interval, in hours.  this gets called each time we sync, so new interval each sync
  Serial.println("Syncing RTC...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  printLocalTime();
  time_t now;                           // unix time in seconds, stored to now
  tm tm;                                // struct tm
  time(&now);                           // fill that variable we just made
  localtime_r(&now, &tm);               // update the tm structure with current (local) time.  gmtime_r for utc
  rtc.adjust(DateTime(tm.tm_year + 1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec)); 

  if(!rtc.setAlarm1(
    rtc.now() + TimeSpan(0, ntpinterval, 0, 0),
    DS3231_A1_Hour                      // A1_Hour matches on HH:mm.ss.  modify above line if this is anything but Hour
    )) Serial.println("Error, alarm wasn't set!");
  Serial.println("RTC synced!");
  Serial.println();
  Serial.printf("next NTP sync set for %i hours from now", ntpinterval);
  Serial.println();
  dropwifi();                           // disconnect wifi to save power
}

void printLocalTime()
{
  tm tm;                                // struct tm 
  if(!getLocalTime(&tm)){               // make sure we have a sync, and correct that if we do not
    Serial.println("syncing to NTP...");
    syncrtc();
    return;
  }
  DateTime alarm1 = rtc.getAlarm1();
  Ds3231Alarm1Mode alarm1mode = rtc.getAlarm1Mode();
  char alarm1Date[12] = "DD hh:mm:ss";
  alarm1.toString(alarm1Date);
  Serial.print(&tm, "%A, %B %d %Y %H:%M:%S");
  Serial.print(" [next NTP sync ");
  Serial.print(alarm1Date);
  Serial.print("]");

    if (rtc.alarmFired(1)) {
        rtc.clearAlarm(1);              // clear the alarm, and then back to syncrtc() to get a new sync interval
        Serial.print(" - alarm cleared, ");
        syncrtc();
    }
  Serial.println();
}

void dropwifi()
{
  Serial.printf("synced NTP, disconnecting from %s ", ssid);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println(" ... WiFi disconnected.");
  Serial.println();
}

# ClockDisplay
an ESP32, DS3231, and TM1637 based 4 digit clock display

this project connects to user-set NTP servers via wifi to discipline a DS3231 RTC breakout module.
the RTC time is then displayed on a TM1637 four digit 7 segment display.
after a random time interval (default is 6 to 23 hours), the device will reconnect to NTP and reset the RTC.
wifi is disconnected when not in use.

use a LIR2032 battery in the DS3231 (can use a CR2032 if you desolder the charging diode)

wiring:

ESP32 SCL <-> SCL DS3231
ESP32 SDA <-> SDA DS3231
ESP32 3v3 <-> VIN DS3231
ESP32 GND <-> GND DS3231

ESP32 D15 <-> CLK TM1637
ESP32 D4  <-> DIO TM1637
ESP32 3v3 <-> VIN DS3231
ESP32 GND <-> GND DS3231

![a cool clock](/clocky.jpg?raw=true "Clock Display")

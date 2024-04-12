/*
* This script reads the time from a DS3231 RTC module and prints to serial.
*
* Working: 12/04/2024
*
* Pin connections:
*   SCL of RTC -> SCL (PIN 22) of ESP32
*   SDA of RTC -> SDA (PIN 21) of ESP32
*   GND of RTC -> GND of ESP32
*   Vcc of RTC -> 5V of ESP32
*/

//------------------INCLUDES------------------------
#include "Wire.h"
#include "RTClib.h"

//--------------------GLOBAL VARIABLES-----------------
RTC_DS3231 rtc; // instantiate an RTClib object

void setup() {
  Serial.begin(9600);
  Wire.begin(); // required by the RTClib library because I2C is used

  // Check if RTC is connected
  if (! rtc.begin()) {
    Serial.println("RTC not found.");
    while (1);
  }

  // Write PC time to RTC
  Serial.println("Setting RTC.");
  rtc.adjust(DateTime(__DATE__, __TIME__));

  Serial.println("RTC started.");
}

void loop() {
  // Get current time from RTC
  DateTime now = rtc.now();
  
  // Serial print the datetime
  char rtc_time_str[20];
  sprintf(rtc_time_str, "%02d:%02d:%02d %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year()); 
  Serial.print(F("Date/Time: "));
  Serial.println(rtc_time_str);

}

/*
* This script runs loadcell calibration on startup and saves the TIMESTAMPED HX711 readings to a file on an SD card. 
*
* Working: 19/04/2024
*
* Pin connections:
*   SCL of RTC -> SCL (PIN 22) of ESP32
*   SDA of RTC -> SDA (PIN 21) of ESP32
*   GND of RTC -> GND of ESP32
*   Vcc of RTC -> 5V of ESP32
*
*   SCL of HX -> SCL (PIN 33) of ESP32
*   SDA of HX -> SDA (PIN 32) of ESP32
*   GND of HX -> GND of ESP32
*   Vcc of HX -> 5V of ESP32

*   MOSI of SD module -> PIN 23 of ESP32
*   MISO of SD module -> PIN 19 of ESP32
*   CLK of SD module -> PIN 18 of ESP32
*   CS of SD module -> PIN 5 of ESP32
*   Vcc of SD module -> 5V of ESP32
*   GND of SD module -> GND of ESP32
*   ----* SD card needs to be FAT16 or FAT32 formatted.
*/

//------------------INCLUDES------------------------
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

// RTC
#include "Wire.h"
#include "RTClib.h"

// SD Card
#include "SD.h"
#include "SPI.h"
#include "FS.h"

//--------------------DEFINES-------------------------
#define eigthseconds (millis()/125) // Taring delay

//--------------------GLOBAL VARIABLES-----------------
// HX711 pins:
const int HX711_dout = 32; // D32 mcu > HX711 dout pin 
const int HX711_sck = 33; // D33 mcu > HX711 sck pin

HX711_ADC LoadCell(HX711_dout, HX711_sck); // instantiate an HX711 object
RTC_DS3231 rtc; // instantiate an RTClib object

// EEPROM address
const int calVal_eepromAdress = 0;

int num_readings = 1; // scale readings counter

// RTC time
DateTime now;

// Tare msg
char tare_msg[50];
//--------------------HELPER FUNCTIONS-----------------
// SCALE
void calibrate() {
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      if (Serial.available() > 0) {
        char inByte = Serial.read();
        if (inByte == 't') LoadCell.tareNoDelay();
      }
    }
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
      _resume = true;
    }
  }

  Serial.println("Now, place your known mass on the loadcell.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }

  LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass); //get the new calibration value

  Serial.print("New calibration value has been set to: ");
  Serial.print(newCalibrationValue);
  Serial.println(", use this as calibration value (calFactor) in your project sketch.");
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;

      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }

  Serial.println("End calibration");
  Serial.println("***");
  Serial.println("To re-calibrate, send 'r' from serial monitor.");
  Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
  Serial.println("***");
}

void changeSavedCalFactor() {
  float oldCalibrationValue = LoadCell.getCalFactor();
  boolean _resume = false;
  Serial.println("***");
  Serial.print("Current value is: ");
  Serial.println(oldCalibrationValue);
  Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
  float newCalibrationValue;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(newCalibrationValue);
        LoadCell.setCalFactor(newCalibrationValue);
        _resume = true;
      }
    }
  }
  _resume = false;
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }
  Serial.println("End change calibration value");
  Serial.println("***");
}

// SD CARD
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    // if(!file){
    //     Serial.println("Failed to open file for reading");
    //     return;
    // }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    // Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    // if(!file){
    //     Serial.println("Failed to open file for appending");
    //     return;
    // }
    file.print(message);
    // if(file.print(message)){
    //     Serial.println("Message appended");
    // } else {
    //     Serial.println("Append failed");
    // }
    file.close();
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}
//------------------------------------------------------

void setup() {
  // SERIAL SETUP
  Serial.begin(9600); // Initialise baud rate with PC

  //--------------------RTC SETUP-----------------------
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
  
  //--------------------SD CARD SETUP-----------------------
  // Check if the SD card module is connected
  if(!SD.begin()){
    Serial.println("Card Mount Failed.");
    return;
  }

  // Check if the SD card is inserted/correctly formatted
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached.");
    return;
  }

  Serial.println("SD card mounted.");

  // Create the storage file
  //createDir(SD, "/weight_readings");
  writeFile(SD, "weight_readings.txt", "Start\n"); // create the file
  
  //--------------------LOADCELL SETUP--------------------
  LoadCell.begin();
  // LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);

  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  // else {
  //   LoadCell.setCalFactor(1.0); // user set calibration value (float), initial value 1.0 may be used for this sketch
  //   Serial.println("Startup is complete");
  // }
  Serial.println("Startup is complete");
  while (!LoadCell.update());
  calibrate(); //start calibration procedure
  //------------------------------------------------------
}

void loop() {
  static boolean newDataReady = 0;

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  if (newDataReady) {
      float reading = LoadCell.getData();
      if(reading<1 & eigthseconds%80 == 0){ //Condition to set new tare every 1000 milliseconds
        boolean _resume = false;
        boolean _tarewait = true;
        while (_resume == false) {
          LoadCell.update();
          if(_tarewait){              
            LoadCell.tareNoDelay();
            _tarewait = false;
          }
          if (LoadCell.getTareStatus() == true) {
            Serial.println("Next Tare Complete");
            // Record tare event on SD card
            sprintf(tare_msg, "%02d:%02d:%02d,%02d/%02d/%02d,%d,tare\n",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), 0); 
            appendFile(SD, "/weight_readings.txt", tare_msg);
            _resume = true;
          }
        }
      }
      else if (reading>1){ // If a bird or heavy object is detected on the scale
        // Get current time from RTC
        now = rtc.now();
    
        // Serial print the reading 
        char msg[50];
        sprintf(msg, "%02d:%02d:%02d,%02d/%02d/%02d,%d,%f\n",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), num_readings, reading); 
        // Save to text file on SD card
        appendFile(SD, "/weight_readings.txt", msg);

        newDataReady = 0;
        num_readings++;
      }
    
  }

  // receive command from serial terminal
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay(); //tare
    else if (inByte == 'c') calibrate(); //calibrate
    else if (inByte == 'x') changeSavedCalFactor(); //edit calibration value manually
    else if (inByte == 'l') listDir(SD, "/", 0); // list all files on level 0
    else if (inByte == 'r') readFile(SD, "/weight_readings.txt"); // read the file
    else if (inByte == 'd') deleteFile(SD, "/weight_readings.txt"); // delete the file
    else if (inByte == 'w') writeFile(SD, "/weight_readings.txt", "Start\n"); // write the file
  }

  // check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");

    // Record tare event on SD card
    sprintf(tare_msg, "%02d:%02d:%02d,%02d/%02d/%02d,%d,tare\n",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), 0); 
    appendFile(SD, "/weight_readings.txt", tare_msg);
  }

}

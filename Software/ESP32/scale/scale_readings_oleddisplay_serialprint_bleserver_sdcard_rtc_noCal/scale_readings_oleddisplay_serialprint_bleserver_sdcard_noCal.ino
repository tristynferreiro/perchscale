/*
* SCRIPT DESCRIPTION
*
* Working: 
*
* Pin connections:
*   SCL of OLED -> SCL (PIN 22) of ESP32
*   SDA of OLED -> SDA (PIN 21) of ESP32
*   GND of OLED -> GND of ESP32
*   Vcc of OLED -> 3.3V of ESP32
*
*   SCL of HX -> SCL (PIN 33) of ESP32
*   SDA of HX -> SDA (PIN 32) of ESP32
*   GND of HX -> GND of ESP32
*   Vcc of HX -> 5V of ESP32
*
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
// OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// String
#include <string>
// BLE Server
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
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
#define MSG_BUFFER_SIZE 1000 
#define file_name_path "/weight_readings_11102024_test.txt"
#define calibrate_file_name_path "/calibrate_values_11102024_test.txt"

// OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// BLE Server
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define SERVICE_UUID_WEIGH         "1b5fbbda-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_READ  "00000003-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_TARE  "00000004-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_CALIBRATE  "00000005-7e47-40e7-8ac0-9512595ab3fa"

//--------------------GLOBAL VARIABLES-----------------
// pins:
const int HX711_dout = 32; // D32 mcu > HX711 dout pin 
const int HX711_sck = 33; // D33 mcu > HX711 sck pin

HX711_ADC LoadCell(HX711_dout, HX711_sck); // instantiate an HX711 object

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Instantiate an SSD1306 display connected to I2C

RTC_DS3231 rtc; // instantiate an RTClib object
// RTC time
DateTime now;

// SD card msg
char msg[MSG_BUFFER_SIZE];
char calibrate_msg[1000];

// EEPROM address
const int calVal_eepromAdress = 0;

int data_num_readings = 0; // scale readings counter
int calibration_num_points = 0; // Keep track of calibration points.

// BLE Server
bool deviceConnected = false;
int value = 0;
static String rstMSG = "rst";
static String okMSG = "ok";
static String nokMSG = "nok";
BLECharacteristic *readCharacteristic;
BLECharacteristic *tareCharacteristic;
BLECharacteristic *calibrateCharacteristic;
String status = "-1";

// Calibration
float calibration_points[10]; // size = how many calibration weights used
int num_points=0; // counter to keep track of calibration points
bool calibrate_complete_flag = false;

//------------------------------------------------------
//------------------HELPER FUNCTIONS------------------------
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device Connected.");
      status = "connected"; // OLED set the status
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      status = "-1"; // OLED reset the status
      Serial.println("Device Disconnected.");
      delay(500); // give the bluetooth stack the chance to get things ready

      // RESET ALL CHARACTERISTICS
      readCharacteristic->setValue("read");
      tareCharacteristic->setValue("tare"); 
      calibrateCharacteristic->setValue("calibrate"); 
      Serial.println("Reset characteristics");

      BLEDevice::startAdvertising();
      Serial.println("Started re-advertising.");


    }
};
//------------------------------------------------------

void setup() {
  // SERIAL SETUP
  Serial.begin(9600); // Initialise baud rate with PC
  
  //----------LOADCELL SETUP------------------------
  LoadCell.begin();
  // LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);

  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }  // else {
  //   LoadCell.setCalFactor(1.0); // user set calibration value (float), initial value 1.0 may be used for this sketch
  //   Serial.println("Startup is complete");
  // }
  Serial.println("Loadcell Startup is complete");

  //----------OLED SETUP------------------------
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  Serial.println("OLED Startup is complete");

  //--------------------RTC SETUP-----------------------
  Wire.begin(); // required by the RTClib library because I2C is used

  // Check if RTC is connected
  if (!rtc.begin()) {
    Serial.println("RTC not found.");
    while (1);
  }

  // Write PC time to RTC
  Serial.println("Setting RTC.");
  rtc.adjust(DateTime(__DATE__, __TIME__));
  now = rtc.now();
  char rtc_time_str[20];
  sprintf(rtc_time_str, "%02d:%02d:%02d %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year()); 
  Serial.println(rtc_time_str);
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
  //createDir(SD, file_name_path);
  writeFile(SD, file_name_path, "Start\n"); // create the file


  //----------BLE SERVER SETUP------------------------
  Serial.println("Starting BLE work!");
  BLEDevice::init("PerchScale");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  //****** CONNECT SERVICE
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic =
    pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setValue("connect");
  pService->start();
  //****** WEIGH SERVICE
  BLEService *weighService = pServer->createService(SERVICE_UUID_WEIGH);
  
  readCharacteristic =
    weighService->createCharacteristic(CHARACTERISTIC_UUID_READ, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  weighService->addCharacteristic(readCharacteristic);
  readCharacteristic->setValue("read");
  
  tareCharacteristic =
    weighService->createCharacteristic(CHARACTERISTIC_UUID_TARE, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  weighService->addCharacteristic(tareCharacteristic); 
  tareCharacteristic->setValue("tare"); 

  calibrateCharacteristic =
    weighService->createCharacteristic(CHARACTERISTIC_UUID_CALIBRATE, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  weighService->addCharacteristic(calibrateCharacteristic); 
  calibrateCharacteristic->setValue("calibrate"); 

  weighService->start();
   
  // BLE Server: Begin advertising services & characteristics
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->addServiceUUID(SERVICE_UUID_WEIGH);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
  // ----------------------------------------------- 

  // Output that startup is complete on display
  writeToDisplayCentre(2.5, WHITE, "Startup complete");

  //while (!LoadCell.update());
  //calibrate(); //start calibration procedure
}

void loop() {
  //----------CONTROLLER MODE------------------------
  // If BLE device is connected, respond according to commands
  if(deviceConnected){ 
    if(status == "connected"){
      writeToDisplayCentre(2.5, WHITE, "Controller connected");
    }else if (status == "tare"){
      writeToDisplayCentre(2.5, WHITE, "Taring");
    }else if (status == "calibrate"){
      writeToDisplayCentre(2.5, WHITE, "Calibrating");
    }else if(status == "read"){
    }

    //*********** CONTROLLER Read
    String readVal = readCharacteristic->getValue().c_str(); // BLE: read characteristic
    // Serial.print("Read:"); Serial.println(readVal);

    if (readVal == rstMSG) {
      Serial.println("Resetting read flag.");
      readCharacteristic->setValue("read"); // BLE: set characteristic
      status = "connected"; // OLED reset the status
    } 
    else if (readVal != "read" && readVal != rstMSG) {
      // Serial.println("Reading scale: ");
      static boolean newDataReady = 0;

      // check for new data/start next conversion:
      if (LoadCell.update()) newDataReady = true;

      if (newDataReady) {
          float reading = LoadCell.getData();
          if(reading<1 & eigthseconds%80 == 0){ //Condition to set new tare every 1000 milliseconds
            boolean _resume = false;
            boolean _tarewait = true;
            // RESET readings counter between events
            data_num_readings = 1;

            while (_resume == false) {
              LoadCell.update();
              if(_tarewait){              
                LoadCell.tareNoDelay();
                _tarewait = false;
              }
              if (LoadCell.getTareStatus() == true) {
                readCharacteristic->setValue(String("Tared").c_str()); // BLE: Updating value (placeholder)
                writeToDisplayCentre(2.5, WHITE, "Tare done");
                // writeToDisplay(2.5, WHITE, 0, 10, "Tare done");

                Serial.println("Next Tare Complete");
                
                // Get current time from RTC
                now = rtc.now();
                // Record tare event on SD card
                sprintf(msg, "%s%02d:%02d:%02d,%02d/%02d/%02d,%d,tare\n", msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), 0);
                if(MSG_BUFFER_SIZE-strlen(msg)<=28){ //28 is the size of one recorded event string
                  // Save to text file on SD card
                  appendFile(SD, file_name_path, msg);
                  strcpy(msg, ""); //memset(msg, 0, MSG_BUFFER_SIZE);
                  Serial.println("Written to file.");
                  writeToDisplayCentre(3, WHITE, "Written to file.");
                }
                _resume = true;
                // delay(2000);
              }
            }
          }
          else if (reading>1){ //If a bird or heavy object is detected on the scale
              // Get current time from RTC
              now = rtc.now();
              // Append the reading to the buffer
              sprintf(msg, "%s%02d:%02d:%02d,%02d/%02d/%02d,%03d,%.4f\n", msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), data_num_readings, reading); 
              Serial.println(msg);
              Serial.println(String(strlen(msg)));
              if(MSG_BUFFER_SIZE-strlen(msg)<=28){
                // Save to text file on SD card
                appendFile(SD, file_name_path, msg);
                strcpy(msg, "");
                //memset(msg, 0, MSG_BUFFER_SIZE);
                Serial.println("Written to file. Number of readings: "+ String(data_num_readings));
                writeToDisplayCentre(3, WHITE, "Written to file.");
              }
              String reading_msg = String(reading) + "g";
             
              writeToDisplayCentre(3, WHITE, reading_msg.c_str());
              // writeToDisplay(3, WHITE, 0, 4, reading_msg.c_str());

              readCharacteristic->setValue(String(reading_msg).c_str()); // BLE: Updating value to scale reading

              newDataReady = 0;
              data_num_readings++;
          }
        
      }
      status="read";
    } 

    //*********** CONTROLLER Tare
    String tareVal = tareCharacteristic->getValue().c_str(); // BLE: read characteristic
    // Serial.print("Tare:");
    // Serial.println(tareVal);
    if (tareVal != okMSG && tareVal != rstMSG && tareVal != "t" && tareVal != "tare") {
      Serial.println("Unacceptable tare value received.");
      calibrateCharacteristic->setValue(nokMSG.c_str()); // BLE: set characteristic
    } 
    if (tareVal == "t") {
      status = "tare"; // OLED set the status
      Serial.println("Tare command received");
      LoadCell.tareNoDelay(); //tare

      writeToDisplayCentre(2.5, WHITE, "Tare done");
      
      // Get current time from RTC
      now = rtc.now();
      // Record tare event
      sprintf(msg, "%s%02d:%02d:%02d,%02d/%02d/%02d,%d,tare\n",  msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), 0); 
      Serial.println(msg);
      Serial.println(String(strlen(msg)));
      if(MSG_BUFFER_SIZE-strlen(msg)<=28){
        // Save to text file on SD card
        appendFile(SD, file_name_path, msg);
        strcpy(msg, "");
        //memset(msg, 0, MSG_BUFFER_SIZE);
        Serial.println("Written to file.");
        writeToDisplayCentre(3, WHITE, "Written to file.");
      }

      tareCharacteristic->setValue(okMSG.c_str()); // BLE: send OK 
      Serial.println("Tare complete");
    } 
    else if (tareVal == rstMSG) {
      Serial.println("Resetting tare flag.");
      tareCharacteristic->setValue("tare"); // BLE: set characteristic
      status = "connected"; // OLED reset the status
    }

    //*********** CONTROLLER Calibrate 
    String calibrateVal = calibrateCharacteristic->getValue().c_str(); // BLE: read characteristic
    // Serial.print("Calibrate:");
    // Serial.println(calibrateVal);

    if (calibrateVal == rstMSG) {
      Serial.println("Resetting calibrate flag.");
      calibrateCharacteristic->setValue("calibrate"); // BLE: set characteristic
      calibrate_complete_flag = false;
    }


    if (calibrateVal == "10") {
      Serial.println("Calibrate 10g command received");
     
      if(!calibrate_complete_flag){
        calibrate(calibrateVal.toFloat());
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
      // If can't read from load cell, send fail
      // calibrateCharacteristic->setValue(nokMSG);
    }
    if (calibrateVal == "20") {
      Serial.println("Calibrate 20g command received");
      if(!calibrate_complete_flag){
        calibrate(calibrateVal.toFloat());
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
      // If can't read from load cell, send fail
      // calibrateCharacteristic->setValue(nokMSG);    
    }  
    if (calibrateVal == "200") {
      Serial.println("Calibrate 200g command received");
      if(!calibrate_complete_flag){
        calibrate(calibrateVal.toFloat());
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
      // If can't read from load cell, send fail
      // calibrateCharacteristic->setValue(nokMSG);  
    }
    if (calibrateVal == "save"){
      Serial.println("Saving Calibration value");
      // CAL SAVE CALIBRATE FUNCTION --> STILL NEEDS TO BE WRITTEN
      if(!calibrate_complete_flag){
      #if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
      #endif
        EEPROM.put(calVal_eepromAdress, calibration_points[2]);
      #if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
      #endif
        EEPROM.get(calVal_eepromAdress, calibration_points[2]);
        Serial.print("Value ");
        Serial.print(calibration_points[2]);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        // Set 'ok' for succesfully saving calibration value
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
        Serial.println("Wrote ok to characteristic");
      }
    }

    if (calibrateVal == "done") {
      Serial.println("Calibrate DONE command received");
      if(!calibrate_complete_flag){
        Serial.println("Setting to ok");
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
    }

    else if (calibrateVal != okMSG && calibrateVal != rstMSG && calibrateVal != "10" && calibrateVal != "20" && calibrateVal != "200" && calibrateVal != "calibrate" && calibrateVal != "done" && calibrateVal!= "save") {
      Serial.println("Unacceptable calibrate value received.");
      calibrateCharacteristic->setValue(nokMSG.c_str()); // BLE: set characteristic
    }
    
    delay(200);
  }
  //----------STANDBY MODE------------------------
  // If no controller is connected, read values and save to SD card (no OLED)
  else if(!deviceConnected){
    static boolean newDataReady = 0;

    // check for new data/start next conversion:
    if (LoadCell.update()) newDataReady = true;

    if (newDataReady) {
        float reading = LoadCell.getData();
        if(reading<1 & eigthseconds%80 == 0){ //Condition to set new tare every 1000 milliseconds
          boolean _resume = false;
          boolean _tarewait = true;
          data_num_readings = 0;
          while (_resume == false) {
            LoadCell.update();
            if(_tarewait){              
              LoadCell.tareNoDelay();
              _tarewait = false;
            }
            if (LoadCell.getTareStatus() == true) {
              
              writeToDisplayCentre(2.5, WHITE, "Tare done");
              // writeToDisplay(2.5, WHITE, 0, 10, "Tare done");

              // Get current time from RTC
              now = rtc.now();
              // Record tare event on SD card
              sprintf(msg, "%s%02d:%02d:%02d,%02d/%02d/%02d,%d,tare\n", msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), 0);
              if(MSG_BUFFER_SIZE-strlen(msg)<=28){ //28 is the size of one recorded event string
                // Save to text file on SD card
                appendFile(SD, file_name_path, msg);
                strcpy(msg, ""); //memset(msg, 0, MSG_BUFFER_SIZE);
                Serial.println("Written to file.");
                writeToDisplayCentre(3, WHITE, "Written to file.");
              }

              Serial.println("Next Tare Complete");
              _resume = true;
              // delay(2000);
            }
          }
        }
        else if (reading>1){ //If a bird or heavy object is detected on the scale
            String reading_msg = String(reading) + "g";
            writeToDisplayCentre(3, WHITE, reading_msg.c_str());
            
            // Get current time from RTC
            now = rtc.now();
            // Append the reading to the buffer
            sprintf(msg, "%s%02d:%02d:%02d,%02d/%02d/%02d,%03d,%.4f\n", msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), data_num_readings, reading); 
            Serial.println(msg);
            Serial.println(String(strlen(msg)));
            if(MSG_BUFFER_SIZE-strlen(msg)<=28){
              // Save to text file on SD card
              appendFile(SD, file_name_path, msg);
              strcpy(msg, "");
              //memset(msg, 0, MSG_BUFFER_SIZE);
              Serial.println("Written to file. Number of readings: "+ String(data_num_readings));
              writeToDisplayCentre(3, WHITE, "Written to file.");
            }  
            newDataReady = 0;
            data_num_readings++;
        }
      
    }

    // check if last tare operation is complete
    if (LoadCell.getTareStatus() == true) {
      writeToDisplayCentre(2.5, WHITE, "Tare done");
      // writeToDisplay(2.5, WHITE, 0, 10, "Tare done");

      // Get current time from RTC
      now = rtc.now();
      // Record tare event
      sprintf(msg, "%s%02d:%02d:%02d,%02d/%02d/%02d,%d,tare\n",  msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), 0); 
      if(MSG_BUFFER_SIZE-strlen(msg)<=28){
        // Save to text file on SD card
        appendFile(SD, file_name_path, msg);
        strcpy(msg, "");
        //memset(msg, 0, MSG_BUFFER_SIZE);
        Serial.println("Written to file.");
        writeToDisplayCentre(3, WHITE, "Written to file.");
      }
       
      Serial.println("Tare complete");
    }
  }
  

}

//------------------HELPER FUNCTIONS------------------------
void writeToDisplay(int textSize, char textColour, int cursorX, int cursorY, const char* msgToPrint) {
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(textColour);
  display.setCursor(cursorX, cursorY);
  display.print(msgToPrint);
  display.display(); 
}

void writeToDisplayCentre(int textSize, char textColour, const char* text) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  // display on horizontal and vertical center
  display.clearDisplay(); // clear display
  display.setTextSize(textSize);
  display.setTextColor(textColour);
  display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);
  display.println(text); // text to display
  display.display();
}


void calibrate(float calibration_weight) {
  writeToDisplayCentre(2, WHITE, "Calibrating...");

  Serial.println("***");
  Serial.println("Starting calibration for "+ String(calibration_weight) + "g");

  // Taring has been done before this method is called

  float known_mass = calibration_weight;
  bool _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (known_mass != 0) {
      Serial.println("Known mass is: " + String(known_mass));
       _resume = true;
    }  
  }

  LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correctly
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass); //get the new calibration value
  Serial.println("Calibration value = " + String(newCalibrationValue));
  
  // Store the point in the calibration array
  calibration_points[calibration_num_points] = newCalibrationValue; 
  calibration_num_points += 1;
  calibrate_complete_flag = true;

  Serial.println("Stored calibration value for " + String(calibration_weight) + "g");
  Serial.println("***");
//   Serial.print("New calibration value has been set to: ");
//   Serial.print(newCalibrationValue);
//   Serial.print("Save this value to EEPROM address ");
//   Serial.print(calVal_eepromAdress);
//   Serial.println("? y/n");

//   _resume = false;
//   while (_resume == false) {
//     if (Serial.available() > 0) {
//       char inByte = Serial.read();
//       if (inByte == 'y') {
// #if defined(ESP8266)|| defined(ESP32)
//         EEPROM.begin(512);
// #endif
//         EEPROM.put(calVal_eepromAdress, newCalibrationValue);
// #if defined(ESP8266)|| defined(ESP32)
//         EEPROM.commit();
// #endif
//         EEPROM.get(calVal_eepromAdress, newCalibrationValue);
//         Serial.print("Value ");
//         Serial.print(newCalibrationValue);
//         Serial.print(" saved to EEPROM address: ");
//         Serial.println(calVal_eepromAdress);
//         _resume = true;

//       }
//       else if (inByte == 'n') {
//         Serial.println("Value not saved to EEPROM");
//         _resume = true;
//       }
//     }
//   }

//   writeToDisplayCentre(2, WHITE, "Calibration complete!");

//   Serial.println("End calibration");
//   Serial.println("***");
//   Serial.println("To re-calibrate, send 'r' from serial monitor.");
//   Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
//   Serial.println("***");

  delay(2000);
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
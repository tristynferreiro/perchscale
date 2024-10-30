/*
* This contains the PerchScale scale code. 
*
* Working: 
*   v1 - MM/DD/YYYY; Components: OLED display, BLE server, RTC, SD card, HX711.  
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
*
*   SCL of RTC -> SCL (PIN 22) of ESP32
*   SDA of RTC -> SDA (PIN 21) of ESP32
*   GND of RTC -> GND of ESP32
*   Vcc of RTC -> 5V of ESP32
*/

//-----------------CODE SETUP---------------------
#define OLED_CONNECTED // comment if the OLED will not be attached to the device
#define DEFAULT_THRESHOLD 100 //this is the threshold trigger value for a reading event
// BLE Server
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define SERVICE_UUID_WEIGH         "1b5fbbda-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_READ  "00000003-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_TARE  "00000004-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_CALIBRATE  "00000005-7e47-40e7-8ac0-9512595ab3fa"

//------------------INCLUDES------------------------
#if defined(ESP32)
  #include <HX711_ADC.h>

  #include <EEPROM.h>
  // OLED
  #ifdef OLED_CONNECTED
    #include <Wire.h>
    #include <Adafruit_GFX.h>
    #include <Adafruit_SSD1306.h>
  #endif
  // String
  #include <String>
  // BLE Server
  #include <BLEDevice.h>
  #include <BLEUtils.h>
  #include <BLEServer.h>

  // RTC
  #include "Wire.h"
  #include "RTClib.h"

  // SD Card
  #include "SD.h"
  #include "SPI.h"
  #include "FS.h"
#endif

//--------------------DEFINES-------------------------
#define eigthseconds (millis()/125) // Taring delay
#define MSG_BUFFER_SIZE 4000 
#define TIME_STRING_SIZE 11

// OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Loadcell
#define CALIBRATION_FACTOR 1.0

//--------------------GLOBAL VARIABLES-----------------
// DATA FILES
char file_name_path[31] = "/weight_readings_test.txt";
char calibrate_file_name_path[31] = "/calibrate_values_test.txt";

// SCALE
const int HX711_dout = 32; // D32 mcu > HX711 dout pin 
const int HX711_sck = 33; // D33 mcu > HX711 sck pin

HX711_ADC LoadCell(HX711_dout, HX711_sck); // instantiate an HX711 object

// Calibration
const int NUM_POINTS = 7; // number of items in the list
const int MAX_ITEM_LENGTH = 4; // maximum characters for the item name

char calibration_weights [NUM_POINTS] [MAX_ITEM_LENGTH] = {  // List of calibration weights used (in grams)
  { "105" }, 
  { "120" }, 
  { "155" },
  { "190" },
  { "225" },
  { "260" },
  { "275" }
 };
bool calibrate_complete_flag = false;
float reading_threshold;

// EEPROM address
const int calVal_eepromAdress = 0;

// OLED
#ifdef OLED_CONNECTED
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); //Instantiate an SSD1306 display connected to I2C
#endif

// RTC time
RTC_DS3231 rtc; // instantiate an RTClib object
DateTime now;
char rtc_time_str[TIME_STRING_SIZE];
int old_hour; // This is used to check if a new file needs to be created on the SD card

// SD card msg
char msg[MSG_BUFFER_SIZE];
char calibrate_msg[1000];

int data_num_readings = 1; // how many readings added to message counter
int reading_number = 1; // scale readings counter
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

#define log_file_path "/log.txt"
#define LOG_BUFFER_SIZE 500
char log_buffer[LOG_BUFFER_SIZE];
enum Loglevel {
  INFO_LEVEL,
  WARNING_LEVEL,
  ERROR_LEVEL
};

#define MIN_LOG_LEVEL INFO_LEVEL

//------------------------------------------------------
//------------------HELPER FUNCTIONS------------------------
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE: Device Connected.");
      status = "connected"; // OLED set the status
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      status = "-1"; // OLED reset the status
      Serial.println("BLE: Device Disconnected.");
      delay(500); // give the bluetooth stack the chance to get things ready

      // RESET ALL CHARACTERISTICS
      readCharacteristic->setValue("read");
      tareCharacteristic->setValue("tare"); 
      calibrateCharacteristic->setValue("calibrate"); 
      Serial.println("BLE: Reset characteristics.");

      BLEDevice::startAdvertising();
      Serial.println("BLE: Started re-advertising.");


    }
};
void updateFilePath(fs::FS &fs, DateTime now){
  sprintf(rtc_time_str, "%02d%02d%02d%02d",now.year(), now.month(), now.day(), now.hour());
  
  sprintf(calibrate_file_name_path, "/calibrate_%s.txt", rtc_time_str); 
  sprintf(file_name_path, "/weights_%s.txt", rtc_time_str); 
  
  // Print the file names
  Serial.println("Data file path: " + String(file_name_path));
  Serial.println("Calibration values file path: " + String(calibrate_file_name_path));
    
  // Check if the file already exists so that it is not overwritten
  File file = fs.open(file_name_path, FILE_APPEND);
  if(!file){
    writeFile(fs, file_name_path, "Start\n"); // create the file
  }
  file = fs.open(calibrate_file_name_path, FILE_APPEND);
  if(!file){
    writeFile(fs, calibrate_file_name_path, "Start\n"); // create the file
  }
  old_hour = now.hour();
};
void writeFile(fs::FS &fs,  const char * path, const char * message){
    // Serial.printf("Writing file: %s\n", path);

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
};
void appendFile(fs::FS &fs,  const char * path, const char * message){
    // Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        writeFile(fs, path, message); // create the file
        return;
    }
    file.print(message);
    // if(file.print(message)){
    //     Serial.println("Message appended");
    // } else {
    //     Serial.println("Append failed");
    // }
    file.close();
};
void deleteFile(fs::FS &fs,  const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
};

#ifdef OLED_CONNECTED
void writeToDisplay(int textSize, char textColour, int cursorX, int cursorY, const char* msgToPrint) {
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(textColour);
  display.setCursor(cursorX, cursorY);
  display.print(msgToPrint);
  display.display(); 
};

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
};
#endif

void calibrate(String calibration_weight) {
  #ifdef OLED_CONNECTED
    writeToDisplayCentre(2, WHITE, "Calibrating...");
  #endif

  Serial.println("***");
  Serial.println("Starting calibration for "+ calibration_weight + "g");
  
  LoadCell.update();
  LoadCell.refreshDataSet(); //refresh the dataset to be sure that the mass is measured correctly
  LoadCell.update();
  float calibration_value = LoadCell.getData();
  Serial.println(calibration_value);
  if(!calibration_weight.equals(calibration_weights[0])){ 
    sprintf(calibrate_msg, "%s,%s,%.2f", calibrate_msg, calibration_weight, calibration_value); 
  }
  else {
    sprintf(calibrate_msg, "%s,%.2f", calibration_weight, calibration_value); 
    
    // Update the reading threshold value based on the lowest calibration value
    reading_threshold = calibration_value; 
    #if defined(ESP32)
      EEPROM.begin(512);
      EEPROM.put(calVal_eepromAdress, reading_threshold);
      EEPROM.commit();
      Serial.println("Threshold value is "+ String(EEPROM.get(calVal_eepromAdress, reading_threshold)));
    #endif
  }

  calibrate_complete_flag = true;
  Serial.println(calibrate_msg);
  Serial.println("Stored calibration value for " + String(calibration_weight) + "g");
  Serial.println("***");
};

//  // Not used in the current version of the system but will be necessary for calibration factor update in future.
// void changeSavedCalFactor() {
//   float oldCalibrationValue = LoadCell.getCalFactor();
//   boolean _resume = false;
//   Serial.println("***");
//   Serial.print("Current value is: ");
//   Serial.println(oldCalibrationValue);
//   Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
//   float newCalibrationValue;
//   while (_resume == false) {
//     if (Serial.available() > 0) {
//       newCalibrationValue = Serial.parseFloat();
//       if (newCalibrationValue != 0) {
//         Serial.print("New calibration value is: ");
//         Serial.println(newCalibrationValue);
//         LoadCell.setCalFactor(newCalibrationValue);
//         _resume = true;
//       }
//     }
//   }
//   _resume = false;
//   Serial.print("Save this value to EEPROM adress ");
//   Serial.print(calVal_eepromAdress);
//   Serial.println("? y/n");
//   while (_resume == false) {
//     if (Serial.available() > 0) {
//       char inByte = Serial.read();
//       if (inByte == 'y') {
// #if defined(ESP32)
//         EEPROM.begin(512);
// #endif
//         EEPROM.put(calVal_eepromAdress, newCalibrationValue);
// #if defined(ESP32)
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
//   Serial.println("End change calibration value");
//   Serial.println("***");
// }

void logMessage(fs::FS &fs, const char * message, Loglevel level){
  // Check if the message level meets the minimum log level
  if (level < MIN_LOG_LEVEL) return;

  now = rtc.now();
  char log_msg [100];
  sprintf(log_msg, "%02d:%02d:%02d,%02d/%02d/%02d [%s] %s\n", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), getLogLevelString(level), message);
  
  if(sizeof(log_buffer)+sizeof(log_msg)<LOG_BUFFER_SIZE){
    strcat(log_buffer,log_msg);
  }else{
    appendFile(SD,  log_file_path, log_buffer);
    memset(log_buffer, 0, LOG_BUFFER_SIZE); // Set all bytes in buffer to 0
    strcat(log_buffer,log_msg);
  }
};

// Helper function to convert log level to a string
const char* getLogLevelString(Loglevel level) {
  switch (level) {
    case INFO_LEVEL: return "INFO";
    case WARNING_LEVEL: return "WARNING";
    case ERROR_LEVEL: return "ERROR";
    default: return "?";
  };
};

void controllerRead(){
  //*********** CONTROLLER Read
    String readVal = readCharacteristic->getValue().c_str(); // BLE: read characteristic
    // Serial.print("Read:"); Serial.println(readVal);

    if (readVal == rstMSG) {
      Serial.println("Read mode: Resetting read flag.");
      readCharacteristic->setValue("read"); // BLE: set characteristic
      status = "connected"; // OLED reset the status
    } 
    else if (readVal != "read" && readVal != rstMSG) {
      static boolean newDataReady = 0;
      // check for new data/start next conversion:
      if (LoadCell.update()) newDataReady = true;
      if (newDataReady) {
          float reading = LoadCell.getData();
          if(reading<reading_threshold){
            readCharacteristic->setValue(String("No bird").c_str()); // BLE: No bird detected on scale 
            reading_number = 1; // RESET readings counter between events
            if(eigthseconds%80 == 0){ //Condition to set new tare every 1000 milliseconds
            
              boolean _resume = false;
              boolean _tarewait = true;

              while (_resume == false) {
                LoadCell.update();
                if(_tarewait){              
                  LoadCell.tareNoDelay();
                  _tarewait = false;
                }
                if (LoadCell.getTareStatus() == true) {
                  readCharacteristic->setValue(String("Tared").c_str()); // BLE: Updating value (placeholder)
                  #ifdef OLED_CONNECTED
                  writeToDisplayCentre(2.5, WHITE, "Tare done");
                  #endif

                  Serial.println("Read mode: Next Tare Complete");
                  
                  // Get current time from RTC
                  now = rtc.now();
                  // Record tare event on SD card
                  sprintf(msg, "%s%02d:%02d:%02d,%02d/%02d/%02d,%d,tare,c\n", msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), 0);
                  if(MSG_BUFFER_SIZE-strlen(msg)<=30){ //28 is the size of one recorded event string
                    // Save to text file on SD card
                    appendFile(SD, file_name_path, msg);
                    strcpy(msg, ""); //memset(msg, 0, MSG_BUFFER_SIZE);
                    Serial.println("Read mode: Written to file. Number of readings: "+ String(data_num_readings));
                  }
                  _resume = true;
                  // delay(2000);
                }
              }
            }
          }
          else if (reading>reading_threshold){ //If a bird or heavy object is detected on the scale
              // Get current time from RTC
              now = rtc.now();
              // Append the reading to the buffer
              sprintf(msg, "%s%02d:%02d:%02d,%02d/%02d/%02d,%03d,%.4f,c\n", msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), reading_number, reading); 
            
              if(MSG_BUFFER_SIZE-strlen(msg)<=30){
                // Save to text file on SD card
                appendFile(SD, file_name_path, msg);
                strcpy(msg, "");
                //memset(msg, 0, MSG_BUFFER_SIZE);
                Serial.println("Read mode: Written to file. Number of readings: "+ String(data_num_readings));
              }
              String reading_msg = String(reading) + "g";
               #ifdef OLED_CONNECTED
              writeToDisplayCentre(3, WHITE, reading_msg.c_str());
              #endif

              readCharacteristic->setValue(String(reading_msg).c_str()); // BLE: Updating value to scale reading

              newDataReady = 0;
              reading_number++;
              data_num_readings++;
          }
        
      }
      status="read";
    } 
}

void controllerTare(){
  String tareVal = tareCharacteristic->getValue().c_str(); // BLE: read characteristic

    if (tareVal != okMSG && tareVal != rstMSG && tareVal != "t" && tareVal != "tare") {
      Serial.println("Tare mode: Unacceptable tare value received.");
      tareCharacteristic->setValue(nokMSG.c_str()); // BLE: set characteristic
    } 
    if (tareVal == "t") {
      status = "tare"; // OLED set the status
      Serial.println("Tare mode: Tare command received");
      LoadCell.tareNoDelay(); //tare
      
      #ifdef OLED_CONNECTED
      writeToDisplayCentre(2.5, WHITE, "Tare done");
      #endif
      
      // Get current time from RTC
      now = rtc.now();
      // Record tare event
      sprintf(msg, "%s%02d:%02d:%02d,%02d/%02d/%02d,%d,tare,c\n",  msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), 0); 

      if(MSG_BUFFER_SIZE-strlen(msg)<=30){
        // Save to text file on SD card
        appendFile(SD, file_name_path, msg);
        strcpy(msg, "");
        //memset(msg, 0, MSG_BUFFER_SIZE);
        Serial.println("Tare mode: Written to file.");

        #ifdef OLED_CONNECTED
        writeToDisplayCentre(3, WHITE, "Written to file.");
        #endif
      }

      tareCharacteristic->setValue(okMSG.c_str()); // BLE: send OK 
      Serial.println("Tare mode: Tare complete");
    } 
    else if (tareVal == rstMSG) {
      Serial.println("Tare mode: Resetting tare flag.");
      tareCharacteristic->setValue("tare"); // BLE: set characteristic
      status = "connected"; // OLED reset the status
    }
}

void controllerCalibrate(){
  String calibrateVal = calibrateCharacteristic->getValue().c_str(); // BLE: read characteristic

    if (calibrateVal == rstMSG) {
      Serial.println("Calibrate mode: Resetting calibrate flag.");
      calibrateCharacteristic->setValue("calibrate"); // BLE: set characteristic
      calibrate_complete_flag = false;
    }

    if (calibrateVal.equals(calibration_weights[0])) {
      Serial.println(String("Calibrate mode: Calibrate ")+ calibration_weights[0] + String("command received"));
     
      if(!calibrate_complete_flag){
        calibrate(calibrateVal);
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
    }
    if (calibrateVal.equals(calibration_weights[1])) {
      Serial.println(String("Calibrate mode: Calibrate ")+ calibration_weights[1] + String("command received"));
     
      if(!calibrate_complete_flag){
        calibrate(calibrateVal);
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
    }
    if (calibrateVal.equals(calibration_weights[2])) {
      Serial.println(String("Calibrate mode: Calibrate ")+ calibration_weights[2] + String("command received"));
     
      if(!calibrate_complete_flag){
        calibrate(calibrateVal);
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
    }
    if (calibrateVal.equals(calibration_weights[3])) {
      Serial.println(String("Calibrate mode: Calibrate ")+ calibration_weights[3] + String("command received"));
      if(!calibrate_complete_flag){
        calibrate(calibrateVal);
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
    }
    if (calibrateVal.equals(calibration_weights[4])) {
      Serial.println(String("Calibrate mode: Calibrate ")+ calibration_weights[4] + String("command received"));
      if(!calibrate_complete_flag){
        calibrate(calibrateVal);
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
    }
    if (calibrateVal.equals(calibration_weights[5])) {
      Serial.println(String("Calibrate mode: Calibrate ")+ calibration_weights[5] + String("command received"));
      if(!calibrate_complete_flag){
        calibrate(calibrateVal);
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
    }
    if (calibrateVal.equals(calibration_weights[6])) {
      Serial.println(String("Calibrate mode: Calibrate ")+ calibration_weights[6] + String("command received"));
     
      if(!calibrate_complete_flag){
        calibrate(calibrateVal);
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
    }
    if (calibrateVal == "save"){
      Serial.println("Calibrate mode: Saving Calibration point values");

      sprintf(calibrate_msg, "%s, %02d:%02d:%02d %02d/%02d/%02d\n", calibrate_msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year()); 
      Serial.println(strlen(calibrate_msg));
      // Save to text file on SD card
      appendFile(SD, calibrate_file_name_path, calibrate_msg);
      appendFile(SD, file_name_path, calibrate_msg);
      strcpy(calibrate_msg, "");
      //memset(msg, 0, MSG_BUFFER_SIZE);
      Serial.println("Calibrate mode: Written to file.");

      #ifdef OLED_CONNECTED
        writeToDisplayCentre(3, WHITE, "Written to file.");
      #endif
      
      // Set 'ok' for succesfully saving calibration value
      calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      Serial.println("Calibrate mode: Wrote ok to characteristic");
    }

    if (calibrateVal == "done") {
      Serial.println("Calibrate mode: Calibrate DONE command received");
      if(!calibrate_complete_flag){
        Serial.println("Calibrate mode: Setting flag to ok");
        calibrateCharacteristic->setValue(okMSG.c_str()); // BLE: set characteristic
      }
    }

    if (calibrateVal != okMSG && calibrateVal != rstMSG && !calibrateVal.equals(calibration_weights[0]) && !calibrateVal.equals(calibration_weights[1]) && !calibrateVal.equals(calibration_weights[2]) && !calibrateVal.equals(calibration_weights[3]) && !calibrateVal.equals(calibration_weights[4]) && !calibrateVal.equals(calibration_weights[5]) && !calibrateVal.equals(calibration_weights[6]) && calibrateVal != "calibrate" && calibrateVal != "done" && calibrateVal!= "save") {
      Serial.println("Calibrate mode: Unacceptable calibrate value received.");
      calibrateCharacteristic->setValue(nokMSG.c_str()); // BLE: set characteristic
    }
    
    delay(200);
}

void logData(fs::FS &fs, String mode, String type, float reading){ 
  
  // Check the date and time to creat new file paths.
  now = rtc.now();
  if(now.hour()!=old_hour){
    updateFilePath(SD, now);
  }

  if(mode.equals("s")){
    if(type.equals("tare")){
      sprintf(msg, "%s%02d:%02d:%02d,%02d/%02d/%02d,%d,tare,s\n", msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), 0);
    }
    else if(type.equals("read")){
      // Append the reading to the buffer
      sprintf(msg, "%s%02d:%02d:%02d,%02d/%02d/%02d,%03d,%.4f,s\n", msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), reading_number, reading); 
      reading_number++;
    }
    data_num_readings++;
  }


  if(MSG_BUFFER_SIZE-strlen(msg)<=50){
    // Save to text file on SD card
    appendFile(SD, file_name_path, msg);
    // Serial.println(msg);

    strcpy(msg, "");
    //memset(msg, 0, MSG_BUFFER_SIZE);
    Serial.println("Written to file. Number of readings: "+ String(reading_number));

    #ifdef OLED_CONNECTED
      writeToDisplayCentre(3, WHITE, "Written to file.");
    #endif
    
    data_num_readings = 0;
  }

};

//*******************************************************************************************************************************
//

void setup() {
  //----------SERIAL SETUP------------------------
  Serial.begin(9600); // Initialise baud rate with PC
  Serial.println("\n*************************");
  Serial.println("Beginning startup routine.");

  //----------OLED SETUP------------------------
  #ifdef OLED_CONNECTED
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
      Serial.println("Error: OLED not detected/connected.");
      for(;;);
    }
    delay(2000);
    display.clearDisplay();
    Serial.println("OLED startup completed.");
  #endif

  //--------------------RTC SETUP-----------------------
  Wire.begin(); // required by the RTClib library because I2C is used

  // Check if RTC is connected
  if (!rtc.begin()) {
    Serial.println("RTC not found.");
    while (1);
  }

  // Write PC time to RTC
  Serial.println("Setting RTC using connected PC.");
  rtc.adjust(DateTime(__DATE__, __TIME__));
  
  now = rtc.now();
  char temp[20];
  sprintf(temp, "%02d:%02d:%02d %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year()); 
  Serial.println(temp);

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
  updateFilePath(SD, now);
  File file = SD.open(log_file_path, FILE_APPEND);
  if(!file){
    writeFile(SD, log_file_path, "Start\n"); // create the file
  }

  // logMessage(SD, "SD card setup.", INFO_LEVEL);

  //----------LOADCELL SETUP------------------------
  LoadCell.begin();
  // LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  LoadCell.start(stabilizingtime, true);

  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    // logMessage(SD, "Timeout, check MCU>HX711 wiring and pin designations", ERROR_LEVEL);
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  } else {
    LoadCell.setCalFactor(CALIBRATION_FACTOR);
  //   LoadCell.setCalFactor(1.0); // user set calibration value (float), initial value 1.0 may be used for this sketch
  //   Serial.println("Startup is complete");
  }
  
  // Set the threshold value for taking readings based on the last value saved in EEPROM
  EEPROM.begin(512);
  EEPROM.get(calVal_eepromAdress, reading_threshold);
  if(isnan(reading_threshold)){
    reading_threshold = DEFAULT_THRESHOLD; // ADJUST ACCORDING TO WHAT MAKES SENSE
  }
  Serial.println("\nReading threshold value is " + String(reading_threshold));
  Serial.println("Loadcell startup is complete");

  
  // logMessage(SD, strcat("Threshold value set to",String(reading_threshold).c_str()),INFO_LEVEL);

  // Serial.println(log_buffer);
 
  // logMessage(SD, "Loadcell startup is complete",INFO_LEVEL);

  // Serial.println(log_buffer);


  //----------BLE SERVER SETUP------------------------
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
  Serial.println("BLE startup is complete. Characteristics are defined and advertised.");
  // logMessage(SD, "BLE startup is complete. Characteristics are defined and advertised.",INFO_LEVEL);
  // ----------------------------------------------- 

  // Output that startup is complete on display
  #ifdef OLED_CONNECTED
    writeToDisplayCentre(2.5, WHITE, "Startup complete");
  #endif
  // logMessage(SD, "Startup complete",INFO_LEVEL);
  Serial.println("*************************");
  // Serial.println(log_buffer);
};

void loop() {
  //----------CONTROLLER MODE------------------------
  // If BLE device is connected, respond according to commands
  if(deviceConnected){ 
    if(status == "connected"){
       #ifdef OLED_CONNECTED
      writeToDisplayCentre(2.5, WHITE, "Controller connected");
      #endif
      //Serial.println("BLE: Controller connected");
    }else if (status == "tare"){
       #ifdef OLED_CONNECTED
      writeToDisplayCentre(2.5, WHITE, "Taring");
      #endif
      //Serial.println("Controller mode = tarring");
    }else if (status == "calibrate"){
       #ifdef OLED_CONNECTED
      writeToDisplayCentre(2.5, WHITE, "Calibrating");
      #endif
      //Serial.println("Controller mode = calibrate");
    }else if(status == "read"){
      // Do nothing because it is reading the scale
    }

    //*********** CONTROLLER Read
    controllerRead();
    
    //*********** CONTROLLER Tare
    controllerTare();

    //*********** CONTROLLER Calibrate 
    controllerCalibrate();

  } // End of case where device is connected
  //----------STANDBY MODE------------------------
  // If no controller is connected, read values and save to SD card (no OLED)
  else if(!deviceConnected){
    // Save the readings to file on SD card
    appendFile(SD, file_name_path, msg); // Save the readings to file on SD card

    // check for new data/start next conversion:
    static boolean newDataReady = 0;
    if (LoadCell.update()) newDataReady = true;

    if (newDataReady) {
        float reading = LoadCell.getData();
        if(reading<reading_threshold){
          #ifdef OLED_CONNECTED
            writeToDisplayCentre(2.5, WHITE, "No bird");
          #endif
          reading_number = 1;
          if(eigthseconds%160 == 0){ //Condition to set new tare every 1000 milliseconds
            boolean _resume = false;
            boolean _tarewait = true;
            while (_resume == false) {
              LoadCell.update();
              if(_tarewait){              
                LoadCell.tareNoDelay();
                _tarewait = false;
              }
              if (LoadCell.getTareStatus() == true) {
                #ifdef OLED_CONNECTED
                  writeToDisplayCentre(2.5, WHITE, "Tare done");
                #endif

                logData(SD, "s", "tare",0);

                Serial.println("Next Tare Complete");
                _resume = true;
                // delay(2000);
              }
            }
          }
        }
        else if (reading>reading_threshold){ //If a bird or heavy object is detected on the scale
            String reading_msg = String(reading) + "g";
            #ifdef OLED_CONNECTED
              writeToDisplayCentre(3, WHITE, reading_msg.c_str());
            #endif

            logData(SD, "s", "read", reading);

            newDataReady = 0;
        }
      
    }

    // check if last tare operation is complete
    if (LoadCell.getTareStatus() == true) {

      #ifdef OLED_CONNECTED
      writeToDisplayCentre(2.5, WHITE, "Tare done");
      #endif

      logData(SD, "s", "tare",0);
       
      Serial.println("Tare complete");
    }
  } // End of case where device is not connected
  
}; // End of main loop





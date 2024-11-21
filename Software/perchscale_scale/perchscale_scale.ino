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
//#define OLED_CONNECTED // comment if the OLED will not be attached to the device

// Harry: I changed the reading threshold to be able to weight Fork-tailed Drongos, can be adjusted based on scenario.
#define READING_THRESHOLD 35 // [in g] this is the reading event threshold 

// Harry: I have been changing this manually at initial startup in the field as the calibration process has been difficult in some circumstances.
float calibration_factor =1; // default factor 

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
#define TIME_STRING_SIZE 11

#define MSG_BUFFER_SIZE 4000 
#define LOG_BUFFER_SIZE 1000
#define log_file_path "/log.txt"
#define MIN_LOG_LEVEL INFO_LEVEL

// OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//--------------------GLOBAL VARIABLES-----------------
// DATA FILES
char file_name_path[31] = "/weight_readings_test.txt";
char calibrate_file_name_path[31] = "/calibrate_values_test.txt";

// SCALE
const int HX711_dout = 32; // D32 mcu > HX711 dout pin 
const int HX711_sck = 33; // D33 mcu > HX711 sck pin
float reading;

HX711_ADC LoadCell(HX711_dout, HX711_sck); // instantiate an HX711 object

// Calibration
const int NUM_POINTS = 5; // number of items in the list
const int MAX_ITEM_LENGTH = 4; // maximum characters for the item name
char known_calibration_masses [NUM_POINTS] [MAX_ITEM_LENGTH] = {  // List of calibration weights used (in grams)
  // { "50" }, 
  // { "60" }, 
  { "120" },
  { "150" },
  { "200" },
  { "250" },
  { "300" }
 };
float calibration_weight_values [NUM_POINTS]; // used to determine the calibration factor
bool calibrate_complete_flag = false;

// EEPROM address
const int calVal_eepromAdress = 0;
// const int threshhold_eepromAdress = 1;

// OLED
#ifdef OLED_CONNECTED
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); //Instantiate an SSD1306 display connected to I2C
#endif

// RTC time
RTC_DS3231 rtc; // instantiate an RTClib object
DateTime now;
char rtc_time_str[TIME_STRING_SIZE];
int old_hour; // This is used to check if a new file needs to be created on the SD card

// SD card data_msg
char data_msg[MSG_BUFFER_SIZE];
char calibrate_data_msg[1000];
char log_buffer[LOG_BUFFER_SIZE];
char msg[100]; // this is a temporary variable used in some places when string formatting is needed.

int data_num_readings = 1; // how many readings added to message before it is saved to the SD card.
int reading_number = 1; // this is the number of readings in a reading event
boolean new_data_ready = 0;

// BLE Server
bool device_connected = false;
int value = 0;
static String ble_rst_msg = "rst";
static String ble_ok_msg = "ok";
static String ble_nok_msg = "nok";
static String ble_disable_msg = "disable";
BLECharacteristic *pCharacteristic; // connect characteristic; also used for disabling the BLE
BLECharacteristic *read_characteristic;
BLECharacteristic *tare_characteristic;
BLECharacteristic *calibrate_characteristic;
String status = "-1";

enum Loglevel {
  INFO_LEVEL,
  WARNING_LEVEL,
  ERROR_LEVEL
};

//------------------------------------------------------
//------------------HELPER FUNCTIONS------------------------
void logMessage(fs::FS &fs, const char * message, Loglevel level){
  /*
  * This is used to add log messages to the log.txt file on the SD card. This is used as a replacement for Serial prints.
  */

  // Check if the message level meets the minimum log level
  if (level < MIN_LOG_LEVEL) return;

  // Compile the log message
  now = rtc.now();
  char log_data_msg [100];
  sprintf(log_data_msg, "%02d:%02d:%02d,%02d/%02d/%02d [%s] %s\n", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), getLogLevelString(level), message);
  
  // Add it to the buffer AND/OR add it to the log file
  if(strlen(log_buffer)+strlen(log_data_msg)<LOG_BUFFER_SIZE){
    strcat(log_buffer,log_data_msg);
  }else{
    // start writing from the start of the buffer
    appendFile(SD,  log_file_path, log_buffer);
    strcpy(log_buffer,"");
    strcat(log_buffer,log_data_msg);
    Serial.println("Written to log file.");
  }

  // NEED TO DO: IF x HOURS HAVE PASSED THEN WRITE TO THE FILE - MAYBE AFTER THE FILES HAVE BEEN UPDATED?
};
const char* getLogLevelString(Loglevel level) {
  /*
  * This is used to convert the log level to a string
  */

  switch (level) {
    case INFO_LEVEL: return "INFO";
    case WARNING_LEVEL: return "WARNING";
    case ERROR_LEVEL: return "ERROR";
    default: return "?";
  };
};

class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      device_connected = true;
      Serial.println("BLE: Device Connected.");
      logMessage(SD, "BLE: Device Connected.",INFO_LEVEL);
      status = "connected"; // OLED set the status
    };

    void onDisconnect(BLEServer* pServer) {
      device_connected = false;
      status = "-1"; // OLED reset the status
      Serial.println("BLE: Device Disconnected.");
      logMessage(SD, "BLE: Device Disconnected.",INFO_LEVEL);
      delay(500); // give the bluetooth stack the chance to get things ready

      // RESET ALL CHARACTERISTICS
      read_characteristic->setValue("read");
      tare_characteristic->setValue("tare"); 
      calibrate_characteristic->setValue("calibrate"); 
      Serial.println("BLE: Reset characteristics.");
      logMessage(SD, "BLE: Reset characteristics.",INFO_LEVEL);

      BLEDevice::startAdvertising();
      Serial.println("BLE: Started re-advertising.");
      logMessage(SD, "BLE: Started re-advertising.",INFO_LEVEL);
    }
};

void updateFilePath(fs::FS &fs, DateTime now){
  /*
  * This is used to create a new files for data and calibration after a certain amount of time has passed.
  */

  // Update the RTC time string
  sprintf(rtc_time_str, "%02d%02d%02d%02d",now.year(), now.month(), now.day(), now.hour());
  
  // Update the file path names
  sprintf(calibrate_file_name_path, "/calibrate_%s.txt", rtc_time_str); 
  sprintf(file_name_path, "/weights_%s.txt", rtc_time_str); 
  
  // Log the update
  Serial.println("Data file path: " + String(file_name_path));
  Serial.println("Calibration values file path: " + String(calibrate_file_name_path));
  logMessage(SD, "SD: Updated data and calibration file names",INFO_LEVEL);
  
  // Check if the file already exists so that it is not overwritten
  File file = fs.open(file_name_path, FILE_APPEND);
  if(!file){
    writeFile(fs, file_name_path, "Start\n"); // create the file
  }

  file = fs.open(calibrate_file_name_path, FILE_APPEND);
  if(!file){
    writeFile(fs, calibrate_file_name_path, "Start\n"); // create the file
  }

  old_hour = now.hour(); // update the tracking variable (for when a new file is created)
};

void writeFile(fs::FS &fs,  const char * path, const char * message){
  /*
  *  This is used to write a message to the specified file on the SD card
  */

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        logMessage(SD, ("SD: Failed to open file for writing, " + String(path)).c_str(),ERROR_LEVEL);
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
        logMessage(SD, ("SD: Written to file, " + String(path)).c_str(),INFO_LEVEL);
    }
    file.close();
};
void appendFile(fs::FS &fs,  const char * path, const char * message){
  /*
  *  This is used to append a message to the specified file on the SD card
  */

  File file = fs.open(path, FILE_APPEND);
  if(!file){
      Serial.println("Failed to open file for appending");
      logMessage(SD, strcat("SD: Failed to open file for appending, ", String(path).c_str()),ERROR_LEVEL);
      writeFile(fs, path, message); // create the file if it does not exist
      return;
  }if(file.print(message)){
    Serial.println("Appended to file.");
  }
  file.close();
};

// void deleteFile(fs::FS &fs,  const char * path){
//   /*
//   *  This is used to delete a file on the SD card.
//   */
//     Serial.printf("Deleting file: %s\n", path);
//     if(fs.remove(path)){
//         Serial.println("File deleted");
//         logMessage(SD, ("SD: Deleted file, " + String(path)).c_str(),INFO_LEVEL);
//     } else {
//         Serial.println("Delete failed");
//         logMessage(SD, ("SD: Failed to delete file, " + String(path)).c_str(),WARNING_LEVEL);
//     }
// };


void read(bool controller_mode){
  /*
  * This function is the main operation of the scale. Here the HX711 is polled and when it returns a value (reading), if that value is greater than the bird threshold, it is saved to the text file, otherwise the reading is ignored and periodically the scale is tared.
  * bool controller_mode is used to ensure that the BLE characteristics are set
  */

  // check for new data/start next conversion:
  if (LoadCell.update()) new_data_ready = true;
  
  if (new_data_ready) {
    reading = LoadCell.getData();

    Serial.println("Test print (THIS IS NOT ACTUALLY CONSIDERED A READ YET): " + String(reading) + ", raw: " + String(reading*calibration_factor)); // Temporary print
      
    if((reading>READING_THRESHOLD)){ // If bird is on the scale
      String reading_data_msg = String(reading) + "g";
        
      #ifdef OLED_CONNECTED
        writeToDisplayCentre(3, WHITE, reading_data_msg.c_str());
      #endif

      Serial.println("This is considered a reading: " + reading_data_msg);
      logData(SD, "s", "read", reading);

      new_data_ready = 0;  

      if(controller_mode) read_characteristic->setValue(String(reading_data_msg).c_str()); // BLE: Updating value to scale reading  
      
    }     
    else{ // IF THERE IS NO BIRD
      #ifdef OLED_CONNECTED
        writeToDisplayCentre(2.5, WHITE, "No bird");
      #endif

      if(controller_mode)read_characteristic->setValue(String("No bird").c_str()); // BLE: No bird detected on scale 
        
      reading_number = 1; // reset the number of readings in the current reading event

      if(now.hour() != old_hour){ //Condition to trigger tare every hour
        LoadCell.update();
        LoadCell.tare();
        #ifdef OLED_CONNECTED
          writeToDisplayCentre(2.5, WHITE, "Tare done");
        #endif
          
        if(controller_mode)read_characteristic->setValue(String("Tared").c_str()); // BLE: Updating value

        logData(SD, "s", "tare",0);

        Serial.println("Next Tare Complete");
      }

    }
    
  }
}

void controllerRead(){
  /*
  * This function is called to execute certain functions based on the BLE read characteristic value (sent from the PerchScale controller)
  */

  String ble_readVal = read_characteristic->getValue().c_str(); // BLE: read characteristic
  // Serial.println("Read characteristic value: " + ble_readVal);
  
  if (ble_readVal == ble_rst_msg) {
    Serial.println("Controller mode = read: Resetting read flag.");
    read_characteristic->setValue("read"); // BLE: set characteristic
    status = "connected"; // OLED reset the status
  } 
  else if (ble_readVal != "read" && ble_readVal != ble_rst_msg) {
    read(1);
    status="read";
  } 
}
void controllerTare(){
  /*
  * This function is called to execute certain functions based on the BLE tare characteristic value (sent from the PerchScale controller)
  */

  String ble_tareVal = tare_characteristic->getValue().c_str(); // BLE: read characteristic

  if (ble_tareVal != ble_ok_msg && ble_tareVal != ble_rst_msg && ble_tareVal != "t" && ble_tareVal != "tare") {
    Serial.println("Controller mode = tare: Unacceptable tare value received.");
      logMessage(SD, "Controller mode = tare: Unacceptable tare value received.", ERROR_LEVEL);
      tare_characteristic->setValue(ble_nok_msg.c_str()); // BLE: set characteristic
    } 
    if (ble_tareVal == "t") {
      status = "tare"; // OLED set the status
      Serial.println("Controller mode = tare: Tare command received");
      LoadCell.update();
      LoadCell.tare(); //tare
      
      tare_characteristic->setValue(ble_ok_msg.c_str()); // BLE: send OK 
      #ifdef OLED_CONNECTED
      writeToDisplayCentre(2.5, WHITE, "Tare done");
      #endif
      
      logData(SD, "c","tare",0);
      
      Serial.println("Controller mode = tare: Tare complete");
    } 
    else if (ble_tareVal == ble_rst_msg) {
      Serial.println("Controller mode = tare: Resetting tare flag.");
      tare_characteristic->setValue("tare"); // BLE: set characteristic
      logMessage(SD, "Controller mode = tare: Tare complete.", INFO_LEVEL);
      status = "connected"; // OLED reset the status
    }
};

void controllerCalibrate(){
  /*
  * This function is called to execute certain functions based on the BLE calibrate characteristic value (sent from the PerchScale controller)
  */

  String ble_calibrateVal = calibrate_characteristic->getValue().c_str(); // BLE: check calibrate characteristic

  // Note: I have no idea how to make this more efficient. I do not think that I can so for now, this is what it is. Will look into it in future because it works for now.
  if (ble_calibrateVal != ble_ok_msg && ble_calibrateVal != ble_rst_msg && !ble_calibrateVal.equals(known_calibration_masses[0]) && !ble_calibrateVal.equals(known_calibration_masses[1]) && !ble_calibrateVal.equals(known_calibration_masses[2]) && !ble_calibrateVal.equals(known_calibration_masses[3]) && !ble_calibrateVal.equals(known_calibration_masses[4]) && !ble_calibrateVal.equals(known_calibration_masses[5]) && !ble_calibrateVal.equals(known_calibration_masses[6]) && ble_calibrateVal != "calibrate" && ble_calibrateVal != "done" && ble_calibrateVal!= "save") {
      Serial.println("Calibrate mode: Unacceptable calibrate value received.");
      logMessage(SD, ("Calibrate mode: Unacceptable calibrate value received, "+ ble_calibrateVal).c_str(),ERROR_LEVEL);
      calibrate_characteristic->setValue(ble_nok_msg.c_str()); // BLE: set characteristic
  }

  if (ble_calibrateVal == ble_rst_msg) {
    Serial.println("Calibrate mode: Resetting calibrate flag.");
    calibrate_characteristic->setValue("calibrate"); // BLE: set characteristic
    calibrate_complete_flag = false;
  }else if(ble_calibrateVal == "save"){
    // Get calibration factor
    calibration_factor = setCalibrationFactor();
    LoadCell.setCalFactor(calibration_factor); // reset the calibration factor.
    Serial.println("Loadcell: set new calibration factor as " + String(calibration_factor));

    // Save the calibration points and the calibration factor
    sprintf(calibrate_data_msg, "%s, %02d:%02d:%02d %02d/%02d/%02d, %f\n", calibrate_data_msg, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(),calibration_factor); 
    // Serial.println(calibrate_data_msg);

    // Save to text file on SD card
    Serial.println("Calibrate mode: saving calibration points and factor to file."); 
    appendFile(SD, calibrate_file_name_path, calibrate_data_msg);
    appendFile(SD, file_name_path, calibrate_data_msg); // save it to two places for safety
    strcpy(calibrate_data_msg, ""); // reset the buffer

    Serial.println("Calibrate mode: calibration points and factor written to file.");
    logMessage(SD,"Calibrate mode: calibration points and factor written to file.", INFO_LEVEL );
    #ifdef OLED_CONNECTED
      writeToDisplayCentre(3, WHITE, "Written to file.");
    #endif

    // Set 'ok' for succesfully saving calibration value
    calibrate_characteristic->setValue(ble_ok_msg.c_str()); // BLE: set characteristic
    Serial.println("Calibrate mode: Wrote ok to characteristic");
  }
  else if(ble_calibrateVal == "done") {
    Serial.println("Calibrate mode: Calibrate DONE command received");
    if(!calibrate_complete_flag){
      Serial.println("Calibrate mode: Setting flag to ok");
      logMessage(SD, "Calibrate mode: calibration successful",INFO_LEVEL);
      calibrate_characteristic->setValue(ble_ok_msg.c_str()); // BLE: set characteristic
    }
  }else{
    // Run calibration
    // Reset the calibration value to 1 to get raw data
    LoadCell.setCalFactor(1.00);

    // Get the new data
    LoadCell.refreshDataSet(); //refresh the dataset to be sure that the mass is measured correctly

    for (int calibration_point_index = 0; calibration_point_index < NUM_POINTS; calibration_point_index++){
      if (ble_calibrateVal.equals(known_calibration_masses[calibration_point_index])) {
        Serial.println(String("Calibrate mode: Calibrate ")+ ble_calibrateVal + String("g command received"));
        
        if(!calibrate_complete_flag){
          getCalibrationPoint(ble_calibrateVal,calibration_point_index);
          calibrate_characteristic->setValue(ble_ok_msg.c_str()); // BLE: set characteristic
        }
      }
    }
  }
  delay(100);
};

void getCalibrationPoint(String known_calibration_mass, int calibration_point_index) {
  /*
  * This function is used to get the scale reading which matches the specified known calibration weight
  */

  #ifdef OLED_CONNECTED
    writeToDisplayCentre(2, WHITE, "Calibrating...");
  #endif
  Serial.println("***");
  Serial.println("Starting calibration for "+ known_calibration_mass + "g");

  // Read value
  LoadCell.update();
  // Justin code:
  float tempy;
  uint8_t average = 1;
  for (uint8_t i = 0;i<average;i++) {
    tempy += LoadCell.getData()/average;
  }
  calibration_weight_values[calibration_point_index] = tempy; // append to the array

  if(!known_calibration_mass.equals(known_calibration_masses[0])){ 
    sprintf(calibrate_data_msg, "%s,%s,%.2f", calibrate_data_msg, known_calibration_mass, calibration_weight_values[calibration_point_index]); 
  }
  else {
    sprintf(calibrate_data_msg, "%s,%.2f", known_calibration_mass, calibration_weight_values[calibration_point_index]); 
  }

  calibrate_complete_flag = true;
  
  // Debugging prints
  Serial.println(calibrate_data_msg);
  Serial.println("Stored calibration value for " + String(known_calibration_mass) + "g");
  Serial.println("***");
};
float setCalibrationFactor() {
  Serial.println("Controller mode = calibrate: calculating new calibration factor.");
  
  float factor = (calibration_weight_values[NUM_POINTS-1]-calibration_weight_values[1])/(atoi(known_calibration_masses[NUM_POINTS-1])-atoi(known_calibration_masses[1]));

  #if defined(ESP32)
    EEPROM.begin(512);
    EEPROM.put(calVal_eepromAdress, factor);
    EEPROM.commit();
    factor = EEPROM.get(calVal_eepromAdress, factor);
    Serial.println("New calibration factor set to "+ String(factor));
    logMessage(SD, ("New calibration factor set to "+ String(factor)).c_str(),INFO_LEVEL);
  #endif
  
  return factor;
};



void logData(fs::FS &fs, String mode, String type, float reading){ 
  /*
  * This function is used to save readings to a buffer in the specified format based on the type of reading. The buffer is then saved to a file on the SD card when it reaches a certain size.
  */

  char msg[100]; // temp buffer

  // Check the date and time to create new file paths.
  now = rtc.now();
  if(now.hour()!=old_hour){
    updateFilePath(SD, now);
  }

  // Format the data message
  if(mode.equals("s")){
    if(type.equals("tare")){
      sprintf(msg, "%02d:%02d:%02d,%02d/%02d/%02d,%d,tare,s\n", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), 0);
    }
    else if(type.equals("read")){
      // Append the reading to the buffer
      sprintf(msg, "%02d:%02d:%02d,%02d/%02d/%02d,%03d,%.4f,s\n", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), reading_number, reading); 
      reading_number++;
    }
  }else if(mode.equals("c")){
    if(type.equals("tare")){
      sprintf(msg, "%02d:%02d:%02d,%02d/%02d/%02d,%d,tare,c\n", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), 0);
    }
    else if(type.equals("read")){
      // Append the reading to the buffer
      sprintf(msg, "%02d:%02d:%02d,%02d/%02d/%02d,%03d,%.4f,c\n", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), reading_number, reading); 
      reading_number++;
    }
  }

  // Save to SD card
  if(strlen(data_msg)+strlen(msg)> MSG_BUFFER_SIZE){
    // Save to file on SD card
    appendFile(SD, file_name_path, data_msg);

    // Log the save
    Serial.println("Written to file. Number of readings: "+ String(data_num_readings));
    logMessage(SD, ("Written to file. Number of readings: "+ String(data_num_readings)).c_str(),INFO_LEVEL);
    #ifdef OLED_CONNECTED
      writeToDisplayCentre(3, WHITE, "Written to file.");
    #endif
    
    // Update variables
    strcpy(data_msg, ""); // clear data buffer
    strcat(data_msg,msg); // append the data message to the data buffer
    data_num_readings = 1;

  }else{ // append the data message to the data buffer
    strcat(data_msg,msg);
    data_num_readings++;
  }
};


#ifdef OLED_CONNECTED 
  void writeToDisplay(int textSize, char textColour, int cursorX, int cursorY, const char* data_msgToPrint) {
    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(textColour);
    display.setCursor(cursorX, cursorY);
    display.print(data_msgToPrint);
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

//*******************************************************************************************************************************
//

void setup() {
  //----------SERIAL SETUP------------------------
  Serial.begin(9600); // Initialise baud rate with PC

  // Harry: This delay helped with reading the whole startup routine on the Serial monitor
  delay(2000);

  Serial.println("\n*************************");
  Serial.println("Beginning startup routine.");

  //----------OLED SETUP------------------------
  #ifdef OLED_CONNECTED
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
      Serial.println("Error: OLED not detected/connected.");
    }
    delay(2000);
    display.clearDisplay();
    Serial.println("OLED startup completed.");
  #endif

  //--------------------RTC SETUP-----------------------
  // Initialize EEPROM with specified size
  EEPROM.begin(512);

  // Initialize I2C communication for RTC
  Wire.begin();

  // Check if RTC is connected
  if (!rtc.begin()) {
    Serial.println("RTC not found.");
    while (1); // Stop if RTC not found
  }

  //RTC EEPROM setup added to ensure that RTC did not reset, to value from last time code uploaded, at each power up.
  //Can be commented to reset RTC

  // Read the setup flag from EEPROM at address 1
  byte setupCompleted = EEPROM.read(1);

  // If setup flag is not set, run initial setup
  if (setupCompleted != 1) {
    Serial.println("First-time setup: Setting RTC with PC time.");
    rtc.adjust(DateTime(__DATE__, __TIME__)); // Set RTC with compile time

    // Set the flag in EEPROM to indicate initial setup is done
    EEPROM.write(1, 1);
    EEPROM.commit();  // Commit changes to save in EEPROM
  } else {
    Serial.println("RTC setup has already been completed.");
  }

  // Print the current date and time from RTC
  DateTime now = rtc.now();
  char temp[20];
  sprintf(temp, "%02d:%02d:%02d %02d/%02d/%02d", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());
  Serial.println(temp);

  Serial.println("Startup routine completed.");


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

  logMessage(SD, "SD: setup complete.", INFO_LEVEL);

  //----------LOADCELL SETUP------------------------
  LoadCell.begin();
  // LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  LoadCell.start(stabilizingtime, true);

  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    logMessage(SD, "Loadcell: Timeout, check MCU->HX711 wiring and pin designations", ERROR_LEVEL);
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  } else {
    EEPROM.begin(512);
    EEPROM.get(calVal_eepromAdress, calibration_factor);
    if(isnan(calibration_factor)) {
      calibration_factor = 1;
    }
    LoadCell.setCalFactor(calibration_factor); // originally set to a default value and then updated later.
    Serial.println(String("Loadcell: calibration factor set to ") + String(calibration_factor));
  }
  Serial.println("Loadcell: Startup is complete");
 
  Serial.println("\nReading threshold value is " + String(READING_THRESHOLD));
  logMessage(SD, ("Reading threshold value is set to "+ String(READING_THRESHOLD)).c_str(), INFO_LEVEL);
  Serial.println("Loadcell startup is complete");
  logMessage(SD, "Loadcell: startup complete", INFO_LEVEL);

  //----------BLE SERVER SETUP------------------------
  BLEDevice::init("PerchScale");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  //****** CONNECT SERVICE
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic =
    pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setValue("connect");
  pService->start();
  //****** WEIGH SERVICE
  BLEService *weighService = pServer->createService(SERVICE_UUID_WEIGH);
  
  read_characteristic =
    weighService->createCharacteristic(CHARACTERISTIC_UUID_READ, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  weighService->addCharacteristic(read_characteristic);
  read_characteristic->setValue("read");
  
  tare_characteristic =
    weighService->createCharacteristic(CHARACTERISTIC_UUID_TARE, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  weighService->addCharacteristic(tare_characteristic); 
  tare_characteristic->setValue("tare"); 

  calibrate_characteristic =
    weighService->createCharacteristic(CHARACTERISTIC_UUID_CALIBRATE, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  weighService->addCharacteristic(calibrate_characteristic); 
  calibrate_characteristic->setValue("calibrate"); 

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
  logMessage(SD, "BLE startup is complete. Characteristics are defined and advertised.",INFO_LEVEL);
  // ----------------------------------------------- 

  // Output that startup is complete on display
  #ifdef OLED_CONNECTED
    writeToDisplayCentre(2.5, WHITE, "Startup complete");
  #endif
  logMessage(SD, "Startup complete",INFO_LEVEL);
  Serial.println("Startup complete");
  Serial.println("*************************");
  // Serial.println(log_buffer);
  
  // Harry: extra error checking for time of RTC at end of Startup
  //Serial.println(temp);

  // Harry: Setting old_hour to an time at Startup ensured that its operation did not escape 1st hour of use when old_hour is set to null.
  old_hour=now.hour();
};

void loop() {
  // Harry: Load cell would lose calibration factor when controller connected so to ensure that each reading used calibration factor it is set in each loop
  // Could be a more effiicient way.
  LoadCell.setCalFactor(calibration_factor);
  
  //Harry: Not sure if necessary but when added it worked and did not want to remove the setting of now each loop
  now = rtc.now();

  if(device_connected){ 

    String ble_connectVal = pCharacteristic->getValue().c_str(); // BLE: read 
    if(ble_connectVal.equals(ble_disable_msg)){
      BLEDevice::startAdvertising();
      Serial.println("Stopped advertising");
      delay(200);
      BLEDevice::deinit(true);
      Serial.println("De-initialised");
    }
    //----------CONTROLLER MODE------------------------
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
    // NOTE: calling all of these functions every iteration of the loop is likely inefficient and there is possibly a better way of doing it but this is the best I can do for the time being.
    controllerRead(); // checks the BLE read characteristic value
    controllerTare(); // checks the BLE tare characteristic value
    controllerCalibrate(); // checks the BLE calibrate characteristic value

  } // End of case where device is connected
  else if(!device_connected){
    //----------STAND-ALONE MODE------------------------
    read(0); // Just run the reading function

    // Note: I have no idea why this is here, I am too scared to remove it because I am concerned it will introduce a bug. I do not have enough time to figure it out so for now it is harmlessly existing here. It is harmless because it does not trigger a tare but rather checks if one has been triggered.
    if (LoadCell.getTareStatus() == true) {  // check if last tare operation is complete
      #ifdef OLED_CONNECTED
        writeToDisplayCentre(2.5, WHITE, "Tare done");
      #endif
      logData(SD, "s", "tare",0);
      Serial.println("Tare complete. IDK what this does");
    }
  }

  
}; // End of main loop
  





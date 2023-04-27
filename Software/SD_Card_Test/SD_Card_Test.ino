/*********
  This is project writes a simple string to a file on the SD card
  **************************************************************************
  Used Rui Santos' project to get an idea on how to write to the SD card. Their complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-save-microsd-card
*********/

#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"

char buf[] = "Hello Ben";

void setup() {
 
  Serial.begin(9600);
  //Serial.setDebugOutput(true);
  //Serial.println();
  
   /***************** Mount SD Card *****************************/
  Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed, please insert SD Card");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }


  /***************** Read or Write to data files *****************************/
  
  //SD_MMC.remove("/Test/test_write.txt"); // delete the file if existed

  // Define the file path
  String path = "/test_write.txt";
  fs::FS &fs = SD_MMC; 
  Serial.printf("Opening file: %s\n", path.c_str());

  File file = fs.open(path.c_str(), FILE_WRITE); // create/open file to write to
  
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.println(buf); // write to the file
    Serial.printf("Saved file to path: %s\n", path.c_str());
  }
  file.close();

   /***************** Put ESP32 to sleep *****************************/
  delay(2000);
  Serial.println("Going to sleep now");
  delay(2000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {
  
}
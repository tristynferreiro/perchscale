/*
* This project takes the data received over UART and saves it to a text file on the SD card on the ESP32
*/

#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"

void setup() {
  Serial.begin(9600); // Set Baud Rate to communicate with ESP32Dev

  //--------------------------- Mount SD Card ---------------------------/
  // Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    // Serial.println("SD Card Mount Failed, please insert SD Card");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    // Serial.println("No SD Card attached");
    return;
  }
}

void loop() {
  if (Serial.available()) 
  {
    String data = Serial.readString();
    //----------------------- Read or Write to data files ---------------------------/
    // Define the file path
    String path = "/test_uart.txt";
    fs::FS &fs = SD_MMC; 
    // Serial.printf("Opening file: %s\n", path.c_str());

    File file = fs.open(path.c_str(), FILE_APPEND); // open file to write to
    
    if(!file){
      // Serial.println("Failed to open file in writing mode");
    } 
    else {
      file.println(data); // write to the file
      // Serial.printf("Saved file to path: %s\n", path.c_str());
    }
    file.close();

  }
  delay(1500);
}

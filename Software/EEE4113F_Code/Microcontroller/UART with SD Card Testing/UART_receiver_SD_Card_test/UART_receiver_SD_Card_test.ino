/*
* This project takes the data received over UART and saves it to a text file on the SD card on the ESP32
*/

#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
File file;
uint16_t deelay = 500;
void setup() {
  Serial.begin(9600); // Set Baud Rate to communicate with ESP32Dev
  while (!Serial) {
     // Wait for Serial port to connect
  }
  
  //pinMode(16, OUTPUT);

  //--------------------------- Mount SD Card ---------------------------/
  Serial.println("Starting SD Card");
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
  // digitalWrite(16, HIGH); //Turn on
  // delay (1000); //Wait 1 sec
  // Serial.printf("\nHello %d",deelay);
  // digitalWrite(16, LOW); //Turn off
  // delay (deelay); //Wait 1 sec
  if (Serial.available()) 
  {
    //deelay = deelay*2;
    
    String data = Serial.readString();
    //----------------------- Read or Write to data files ---------------------------/
    // Define the file path
    String path = "/test_uart.txt";
    fs::FS &fs = SD_MMC; 
    // Serial.printf("Opening file: %s\n", path.c_str());
    file = fs.open(path.c_str(), FILE_APPEND); // open file to write to
    
    if(file){
      file.println(data); // write to the file
      // Serial.printf("Saved file to path: %s\n", path.c_str());
    }
    file.close();
    
  }
}

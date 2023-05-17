/*
* This project takes the data received over UART and stores all in an array then saves it to a text file on the SD card on the ESP32
*/

#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
File file;
uint16_t deelay = 500;
// Array scale stuff
// String data_arr[1000];
int counter = 0;
// int fileCounter = 0;
// bool done = false;

void setup() {
  Serial.begin(9600); // Set Baud Rate to communicate with ESP32Dev
  while (!Serial) {
     // Wait for Serial port to connect
  }

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
  if (Serial.available()) 
  {
    String data = Serial.readString();
    //----------------------- Read or Write to data files ---------------------------/
    // Define the file path
    String path = "/test_scale.txt";
    fs::FS &fs = SD_MMC; 
    // Serial.printf("Opening file: %s\n", path.c_str());
    file = fs.open(path.c_str(), FILE_APPEND); // open file to write to
    
    if(file){
      file.println(counter); // write to the file
      file.println(data); // write to the file
      // Serial.printf("Saved file to path: %s\n", path.c_str());
    }
    
  }
  counter = counter +1;
  file.println(counter); // write to the file
  file.close();
  // if(done){
  //     //-----------------------\Write to data files ---------------------------/
  //     // Define the file path
  //     String path = "/dataset_";
  //     path.concat(fileCounter);
  //     path.concat(".txt");
  //     fs::FS &fs = SD_MMC; 
  //     // Serial.printf("Opening file: %s\n", path.c_str());
  //     file = fs.open(path.c_str(), FILE_APPEND); // open file to write to
      
  //     if(file){
  //       for ( int i = 0; i<counter; i++){
  //         file.println(data_arr[i]); // write to the file
  //         // Serial.printf("Saved file to path: %s\n", path.c_str());
  //       }
  //     }
  //     file.close();
      
  //     fileCounter = fileCounter+1;
  //     counter = 0;
  //     done = false;
  //   }
  }


#include "FS.h"
#include "SD.h"
#include <SPI.h>


// Define CS pin for the SD card module
#define SD_CS 5

void setup() {
  // put your setup code here, to run once:
  // Initialize SD card
  SD.begin(SD_CS);  
  if(!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}

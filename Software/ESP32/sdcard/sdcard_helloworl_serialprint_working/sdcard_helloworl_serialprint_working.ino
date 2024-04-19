/*
* This script saves a hello world text file to an external SD card module.
* 
* Working: 19/04/2024
*
* Tutorial: https://www.electronicwings.com/esp32/microsd-card-interfacing-with-esp32
* Pin connections:
*   MOSI of SD module -> PIN 23 of ESP32
*   MISO of SD module -> PIN 19 of ESP32
*   CLK of SD module -> PIN 18 of ESP32
*   CS of SD module -> PIN 5 of ESP32
*   Vcc of SD module -> 5V of ESP32
*   GND of SD module -> GND of ESP32
*
* SD card needs to be FAT16 or FAT32 formatted.
*/

//------------------INCLUDES------------------------
#include "SD.h"
#include "SPI.h"
#include "FS.h"

//--------------------GLOBAL VARIABLES-----------------


//--------------------HELPER FUNCTIONS-----------------
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
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

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
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
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
  Serial.begin(9600);

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

    // Display information about the
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    // Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    // Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

}

void loop() {
  // receive command from serial terminal
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 'l') listDir(SD, "/", 0); // list all files on level 0
    else if (inByte == 'c') createDir(SD, "/test"); //create the dir
    else if (inByte == 'x') removeDir(SD, "/test"); //create the dir
    else if (inByte == 'r') readFile(SD, "/test/hello_world.txt"); // read the file
    else if (inByte == 'd') deleteFile(SD, "/test/hello_world.txt"); // delete the file
    else if (inByte == 'w') writeFile(SD, "/test/hello_world.txt", "Hello "); // write the file
    else if (inByte == 'a') appendFile(SD, "/test/hello_world.txt", "World!\n"); // append to the file
  }
}
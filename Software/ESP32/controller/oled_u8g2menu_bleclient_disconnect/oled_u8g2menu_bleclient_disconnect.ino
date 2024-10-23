/*
* WRITE DESCRIPTIONS
*
* Working: 15/06/2024
*
* Pin connections:
*   OLED:
*   GND -> GND
*   VCC -> 3V3
*   SCL -> D22
*   SDA -> D21
*   DPAD:
*   Up Button -> D15
*   Select Button -> D27
*   Down Button -> D26
*   Back Button -> D4
*/

//------------------INCLUDES------------------------
// OLED
#include "U8g2lib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Wire.h>
// String
#include <string>

// BLE
#include "BLEDevice.h"

// // EEPROM
// #include <EEPROM.h>
//--------------------------------------------------


//--------------------DEFINES-------------------------
// OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DELAY_COUNTER 60 // Delay for text on screen


// DPAD Buttons
#define BUTTON_UP_PIN 15 // pin for UP button 
#define BUTTON_SELECT_PIN 27 // pin for SELECT button
#define BUTTON_DOWN_PIN 26 // pin for DOWN button
#define BUTTON_BACK_PIN 4 // pin for BACK button

//----------------------------------------------------

// ------------------ GENERATED BITMAPS ---------------------------------

// 'scrollbar_background', 8x64px
const unsigned char bitmap_scrollbar_background [] PROGMEM = {
  0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 
  0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 
  0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 
  0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 
  0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 
  0x00, 0x40, 0x00, 0x00, };


// 'item_sel_outline', 128x21px
const unsigned char bitmap_item_sel_outline [] PROGMEM = {
  0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x0C, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x02, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x0C, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x02, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x0C, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x02, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x0C, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x02, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x0C, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x02, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x0C, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0xF8, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 
  };


// ----------------------------------------------------------------------

//--------------------GLOBAL VARIABLES--------------------------------

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Instantiate an SSD1306 display connected to I2C

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0); // [full framebuffer, size = 1024 bytes]

// EEPROM address
const int calVal_eepromAdress = 0;

// Menu variables

const int NUM_ITEMS = 5; // number of items in the list
const int MAX_ITEM_LENGTH = 11; // maximum characters for the item name

char menu_items [NUM_ITEMS] [MAX_ITEM_LENGTH] = {  // array with item names
  { "Connect" }, 
  { "Read" }, 
  { "Tare" },
  { "Calibrate" },
  { "Disconnect" }
 };

int current_screen = 0; // screen number which the menu is on
int item_selected = 0; // which item in the menu is selected
int calibrate_weight = 10;
int item_sel_previous; // previous item - used in the menu screen to draw the item before the selected one
int item_sel_next; // next item - used in the menu screen to draw next item after the selected one

// Sub-menu and display delays

int display_counter = 0;
int process_screen = 0;

// Button states

int button_up_clicked = 0; // only perform action when button is clicked, and wait until another press
int button_select_clicked = 0; // same as above
int button_down_clicked = 0; // same as above
int button_back_clicked = 0; // same as above

// ************* BLE CLIENT ******************************
static String rstMsg = "rst";
// Connect service:
BLEClient*  pClient;
// The remote service we wish to connect to.
static BLEUUID connectServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// Weigh service:
static BLEUUID weighServiceUUID("1b5fbbda-7e47-40e7-8ac0-9512595ab3fa");
static BLEUUID    readCharUUID("00000003-7e47-40e7-8ac0-9512595ab3fa");
static BLERemoteCharacteristic* readRemoteCharacteristic;
static BLEUUID    tareCharUUID("00000004-7e47-40e7-8ac0-9512595ab3fa");
static BLERemoteCharacteristic* tareRemoteCharacteristic;
static BLEUUID    calibrateCharUUID("00000005-7e47-40e7-8ac0-9512595ab3fa");
static BLERemoteCharacteristic* calibrateRemoteCharacteristic;


//-------------------------------------------------------------------------------------
//--------------------HELPER FUNCTIONS-----------------
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.write(pData, length);
    Serial.println();
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("Disconnected from Server");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
    //--------------------CONNECT SERVICE------------------------------------------
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(connectServiceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(connectServiceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify()) {
      pRemoteCharacteristic->registerForNotify(notifyCallback);
    }

    connected = true;
    
    //---------------------WEIGH SERVICE----------------
    BLERemoteService* weighRemoteService = pClient->getService(weighServiceUUID);
    if (weighRemoteService == nullptr) {
      Serial.print("Failed to find our WEIGH service UUID: ");
      Serial.println(weighServiceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found WEIGH service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    readRemoteCharacteristic = weighRemoteService->getCharacteristic(readCharUUID);
    if (readRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our READ characteristic UUID: ");
      Serial.println(readCharUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our READ characteristic");

    // Read the value of the characteristic.
    if(readRemoteCharacteristic->canRead()) {
      std::string value = readRemoteCharacteristic->readValue();
      Serial.print("The READ characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(readRemoteCharacteristic->canNotify())
      readRemoteCharacteristic->registerForNotify(notifyCallback);

    // TARE: Obtain a reference to the characteristic in the service of the remote BLE server.
    tareRemoteCharacteristic = weighRemoteService->getCharacteristic(tareCharUUID);
    if (tareRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our TARE characteristic UUID: ");
      Serial.println(tareCharUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our TARE characteristic");

    // Read the value of the characteristic.
    if(tareRemoteCharacteristic->canRead()) {
      std::string value = tareRemoteCharacteristic->readValue();
      Serial.print("The TARE characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(tareRemoteCharacteristic->canNotify())
      tareRemoteCharacteristic->registerForNotify(notifyCallback);
    
    // CALIBRATE: Obtain a reference to the characteristic in the service of the remote BLE server.
    calibrateRemoteCharacteristic = weighRemoteService->getCharacteristic(calibrateCharUUID);
    if (calibrateRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our CALIBRATE characteristic UUID: ");
      Serial.println(calibrateCharUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our TARE characteristic");

    // Read the value of the characteristic.
    if(calibrateRemoteCharacteristic->canRead()) {
      std::string value = calibrateRemoteCharacteristic->readValue();
      Serial.print("The CALIBRATE characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(calibrateRemoteCharacteristic->canNotify())
      calibrateRemoteCharacteristic->registerForNotify(notifyCallback);
    //-----------------------------------------------------------------
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(connectServiceUUID)) {
      Serial.print("Server match");
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
    else if (advertisedDevice.haveServiceUUID() || advertisedDevice.isAdvertisingService(connectServiceUUID)){
      Serial.print("Server not match");
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = false;
    }
  } // onResult
}; // MyAdvertisedDeviceCallbacks

//-----------------------------------------------------
void setup() {

  //--------------------SERIAL SETUP-----------------------
  Serial.begin(9600); // Initialise baud rate with PC

  //--------------------OLED SETUP-----------------------
  u8g2.setColorIndex(1);  // set the color to white
  u8g2.begin();
  u8g2.setBitmapMode(1);

  //--------------------DPAD SETUP-----------------------
  pinMode(BUTTON_UP_PIN, INPUT); // up button
  pinMode(BUTTON_SELECT_PIN, INPUT); // select button
  pinMode(BUTTON_DOWN_PIN, INPUT); // down button
  pinMode(BUTTON_BACK_PIN, INPUT); // down button
  
  Serial.println("Setup Complete!");

   //--------------------BLE CLIENT-----------------------
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
}


void loop() {

  //--------------------MAIN MENU-----------------------

  if (current_screen == 0) { // MENU SCREEN

      // up and down buttons only work for the menu screen
      //--------------------UP BUTTON-----------------------
      if ((digitalRead(BUTTON_UP_PIN) == LOW) && (button_up_clicked == 0)) { // up button clicked - jump to previous menu item
        item_selected = item_selected - 1; // select previous item
        Serial.print("Item Selected:"); // DEBUGGING CODE
        Serial.println(String(menu_items[item_selected]));        
        button_up_clicked = 1; // set button to clicked to only perform the action once
        if (item_selected < 0) { // if first item was selected, jump to last item
          item_selected = NUM_ITEMS-1;
        }
      }
      //--------------------DOWN BUTTON-----------------------
      else if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && (button_down_clicked == 0)) { // down button clicked - jump to next menu item       
        item_selected = item_selected + 1; // select next item
        Serial.print("Item Selected:"); // DEBUGGING CODE
        Serial.println(String(menu_items[item_selected]));       
        button_down_clicked = 1; // set button to clicked to only perform the action once
        if (item_selected >= NUM_ITEMS) { // last item was selected, jump to first menu item
          item_selected = 0;
          }
      } 

      if ((digitalRead(BUTTON_UP_PIN) == HIGH) && (button_up_clicked == 1)) { // unclick 
        button_up_clicked = 0;
      }
      if ((digitalRead(BUTTON_DOWN_PIN) == HIGH) && (button_down_clicked == 1)) { // unclick
        button_down_clicked = 0;
      }

  }

  //--------------------SELECT BUTTON-----------------------
  if ((digitalRead(BUTTON_SELECT_PIN) == LOW) && (button_select_clicked == 0)) { // select button clicked, jump between screens
    button_select_clicked = 1; // set button to clicked to only perform the action once
    
    if ((current_screen == 0)) {current_screen = 1;} // main menu --> scale screen
    //  else if (current_screen == 1 && String(menu_items[item_selected])!="Calibrate") {current_screen = 0;} // scale screen --> main menu
    else if (current_screen == 1 && process_screen ==0 && connected && String(menu_items[item_selected])=="Connect") {
      current_screen = 0; 
      process_screen = 0;} // If on calibrate, move to sub-menu
     else if (current_screen == 1) {current_screen = 2;} // If on calibrate, move to sub-menu
     else if (current_screen == 2 && process_screen == 5 && String(menu_items[item_selected])=="Calibrate") {
      process_screen = 6;
      current_screen = 2;}
    else if (current_screen == 2 && process_screen == 7 && calibrate_weight == 120 && String(menu_items[item_selected])=="Calibrate") {
    process_screen = 0;
    current_screen = 3;}
    else if (current_screen == 2 && process_screen == 7 && calibrate_weight == 155 && String(menu_items[item_selected])=="Calibrate") {
    process_screen = 0;
    current_screen = 4;}
    else if (current_screen == 2 && process_screen == 7 && calibrate_weight == 190 && String(menu_items[item_selected])=="Calibrate") {
    process_screen = 0;
    current_screen = 6;}
    else if (current_screen == 2 && process_screen == 7 && calibrate_weight == 225 && String(menu_items[item_selected])=="Calibrate") {
    process_screen = 0;
    current_screen = 7;}
    else if (current_screen == 2 && process_screen == 7 && calibrate_weight == 265 && String(menu_items[item_selected])=="Calibrate") {
    process_screen = 0;
    current_screen = 5;} 
    else if (current_screen == 2 && process_screen == 7 && String(menu_items[item_selected])=="Calibrate") {
    process_screen = 0;
    current_screen = 0;}        
    else if (current_screen == 3 && process_screen ==0 && String(menu_items[item_selected])=="Calibrate") {
      current_screen = 3;
      process_screen = 1;}
    else if (current_screen == 4 && process_screen ==0 && String(menu_items[item_selected])=="Calibrate") {
      current_screen = 4;
      process_screen = 1;}
    //else if (current_screen == 4 && process_screen ==1 && String(menu_items[item_selected])=="Calibrate") {current_screen = 5;}
    //  else {current_screen = 0;} // qr codes screen --> menu items screen
  }
  if ((digitalRead(BUTTON_SELECT_PIN) == HIGH) && (button_select_clicked == 1)) { // unclick 
    button_select_clicked = 0;
  }

  //--------------------BACK BUTTON-----------------------
  if ((digitalRead(BUTTON_BACK_PIN) == LOW) && (button_back_clicked == 0)) { // back button clicked, return to main menu
     button_back_clicked = 1; // set button to clicked to only perform the action once
    if (current_screen == 1 && process_screen == 1 && String(menu_items[item_selected])=="Read"){
      process_screen = 2;
    }
    else if (current_screen == 1) {current_screen = 0;} // sub-menu 1 screen --> main menu
     else if (current_screen == 2) {current_screen = 0;}
     else if (current_screen == 3) {current_screen = 0;}
     else if (current_screen == 4) {current_screen = 0;}
     else if (current_screen == 5) {current_screen = 0;}
     else {current_screen = 0;}
  }
  if ((digitalRead(BUTTON_BACK_PIN) == HIGH) && (button_back_clicked == 1)) { // unclick 
    button_back_clicked = 0;
  }  

  // set correct values for the previous and next items
  item_sel_previous = item_selected - 1;
  if (item_sel_previous < 0) {item_sel_previous = NUM_ITEMS - 1;} // previous item would be below first = make it the last
  item_sel_next = item_selected + 1;  
  if (item_sel_next >= NUM_ITEMS) {item_sel_next = 0;} // next item would be after last = make it the first


  u8g2.clearBuffer();  // clear buffer for storing display content in RAM

    //--------------------MAIN MENU DISPLAY-----------------------
    if (current_screen == 0) { // MENU SCREEN

      // selected item background
      u8g2.drawXBMP(0, 22, 128, 21, bitmap_item_sel_outline);

      // draw previous item as icon + label
      u8g2.setFont(u8g_font_7x14);
      u8g2.drawStr(4, 15, menu_items[item_sel_previous]); 

      // draw selected item as icon + label in bold font
      u8g2.setFont(u8g_font_7x14B);    
      u8g2.drawStr(4, 15+20+2, menu_items[item_selected]);        

      // draw next item as icon + label
      u8g2.setFont(u8g_font_7x14);     
      u8g2.drawStr(4, 15+20+20+2+2, menu_items[item_sel_next]);   

      // draw scrollbar background
      u8g2.drawXBMP(128-8, 0, 8, 64, bitmap_scrollbar_background);

      // draw scrollbar handle
      u8g2.drawBox(125, 64/NUM_ITEMS * item_selected, 3, 64/NUM_ITEMS); 
      // u8g2.writeBufferXBM(Serial);
      // draw bottom right logo
      // u8g2.drawXBMP(128-16-4, 64-4, 16, 4, upir_logo);               

    } 

    //------------------------- READ SCALE -------------------------------     

    // Scale: Read
    else if ((current_screen == 1) && String(menu_items[item_selected])=="Read" && process_screen == 0) { // READ SCALE SUB-MENU SCREEN
      Serial.println("In Read screen"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      //------------------------- BLE REQUEST SCALE READINGS --------------------------------  
      if (connected) { // Read values from server
        String newValue = "r";
        // Set the characteristic's value to be the array of bytes that is actually a string.
        readRemoteCharacteristic->writeValue(newValue.c_str(),newValue.length());
        process_screen = 1;
      }else{
        process_screen = 3;
      }
      //-----------------------------------------------------------------------------  
      
    }
    else if((current_screen == 1) && String(menu_items[item_selected])=="Read" && process_screen == 1){// READ SCALE SUB-MENU SCREEN
      Serial.println("In Read screen, process screen 1"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      //------------------------- BLE RECEIVE SCALE READINGS -------------------------------- 
      if (connected) { // Read values from server
        std::string value = readRemoteCharacteristic->readValue();
        Serial.print("Read value from server: ");
        Serial.println(value.c_str());
        u8g2.drawStrX2(0, 32, value.c_str());
        // u8g2.print(value.c_str()); 
      }else{
        process_screen = 3;
      } 
        
    }
    else if((current_screen == 1) && String(menu_items[item_selected])=="Read" && process_screen == 2){// READ SCALE SUB-MENU SCREEN
      Serial.println("In Read screen, process screen 2"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      //------------------------- BLE RESET READING -------------------------------- 
      if (connected) { // Read values from server
        u8g2.print("Going back..."); 
        Serial.println("Going back...");
        std::string value = readRemoteCharacteristic->readValue();
        if(value != "rst"){
          Serial.println("Sending reset command");
          readRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
        }
        if(value == "read"){ // check if reset successful at server
          current_screen = 0; // return to home screen
          process_screen = 0;
        }
      }else{
        process_screen = 3;
      } 
        
    }
    else if ((current_screen == 1) && String(menu_items[item_selected])=="Read" && process_screen == 3) { // READ SCALE SUB-MENU SCREEN
      Serial.println("In Read screen, process screen 3"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Disconnected..."); 
      Serial.println("Not connected to server.");
      if(display_counter == DELAY_COUNTER){ // If tared
        process_screen = 0;
        display_counter = 0;
        current_screen = 0;
      }
      display_counter++;
    }
    //------------------------- TARE -------------------------------   

    // Scale: Tare confirm
    else if ((current_screen == 1) && String(menu_items[item_selected])=="Tare" && process_screen == 0) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In Tare screen"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Click enter to");
      u8g2.setCursor(0, 30);
      u8g2.println("begin taring");       
    }

    // Scale: Taring
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Tare" && process_screen == 0) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In Tare screen 2, process screen 0"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Taring..."); 
      u8g2.setCursor(0, 30);
      u8g2.print("Please wait");      
      //------------------------- TELL SCALE TO TARE--------------------------------  
      // If we are connected to a peer BLE Server, update the TARE characteristic
      if (connected) {
        String newValue = "t";
        // Set the characteristic's value to be the array of bytes that is actually a string.
        tareRemoteCharacteristic->writeValue(newValue.c_str(),newValue.length());
        Serial.println("Updated value."); // DEBUGGING CODE
        process_screen = 1;
      }   
    }
    // WAIT for scale response
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Tare" && process_screen == 1) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In Tare screen 2, process screen 1"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Taring..."); 
      u8g2.setCursor(0, 30);
      u8g2.print("Please wait");      
      //------------------------- WAIT FOR SCALE TARE CONFIRMATION --------------------------------  
      // If we are connected to a peer BLE Server, update the TARE characteristic
      Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // Delay
        
        if (connected) { // Read values from server
          std::string value = tareRemoteCharacteristic->readValue();
          Serial.println(value.c_str());
          if(value == "ok"){
            process_screen = 2;
            Serial.println("Tare successful");
          }else{
            process_screen = 3;
            Serial.println("FAILED to tare");
          }
        }else{
          process_screen = 4;
        }
        display_counter = 0;
        tareRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
      }
      display_counter++;
      
    }
    // Scale: Tare successful
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Tare" && process_screen == 2) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In tare screen 2. Process screen 2"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Tare"); 
      u8g2.setCursor(0, 30);
      u8g2.print("Compelete");          
      //------------------------- DELAY -------------------------------   
      //Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // Delay
        process_screen = 0;
        display_counter = 0;
        current_screen = 0;
      }
      display_counter++;
      //----------------------------------------------------------------------------     
    }  
    // Scale: FAILED to tare
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Tare" && process_screen == 3) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In tare screen 2. Process screen 3"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Tare"); 
      u8g2.setCursor(0, 30);
      u8g2.print("Failed.");          
      //------------------------- DELAY -------------------------------   
      //Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // Delay
        process_screen = 0;
        display_counter = 0;
        current_screen = 0;
      }
      display_counter++;
      //----------------------------------------------------------------------------     
    }  
    // Scale: disconnected
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Tare" && process_screen == 4) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In tare screen 2. Process screen 4"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Disconnected...");       
      //------------------------- DELAY -------------------------------   
      Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // Delay
        process_screen = 0;
        display_counter = 0;
        current_screen = 0;
      }
      display_counter++;
      //----------------------------------------------------------------------------     
    }   

    //------------------------- CALIBRATE -------------------------------   

    // Calibration: Remove all weight off scale
    else if ((current_screen == 1) && String(menu_items[item_selected])=="Calibrate" && process_screen == 0) { // CALIBRATION SUB-MENU SCREEN
      // Serial.println("In calibrate screen"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Clear scale");
      u8g2.setCursor(0, 30);
      u8g2.println("Enter once clear");    
    }
    // Calibration: Taring
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 0) { // CALIBRATION SUB-MENU SCREEN
      // Serial.println("In calibrate screen 2"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Taring..."); 
      u8g2.setCursor(0, 30);
      u8g2.print("Please wait");      
      //------------------------- TELL SCALE TO TARE--------------------------------  
      // If we are connected to a peer BLE Server, update the TARE characteristic
      if (connected) {
        String newValue = "t";
        // Set the characteristic's value to be the array of bytes that is actually a string.
        tareRemoteCharacteristic->writeValue(newValue.c_str(),newValue.length());
        Serial.println("Updated value."); // DEBUGGING CODE
        process_screen = 1;
      }else{
        process_screen = 4;
      }   
    }
    // Calibration: WAIT for scale response
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 1) { // TARE SCALE SUB-MENU SCREEN
      // Serial.println("In calibrate screen 2, process screen 1"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Taring..."); 
      u8g2.setCursor(0, 30);
      u8g2.print("Please wait");      
      //------------------------- WAIT FOR SCALE TARE CONFIRMATION --------------------------------  
      // If we are connected to a peer BLE Server, update the TARE characteristic
      if(display_counter == DELAY_COUNTER){ // Delay
        if (connected) { // Read values from server
          std::string value = tareRemoteCharacteristic->readValue();
          Serial.println(value.c_str());
          if(value == "ok"){
            process_screen = 2;
            Serial.println("Tare successful");
          }else{
            process_screen = 3;
            Serial.println("FAILED to tare");
          }
        }else{
          process_screen = 4;
        }
        display_counter = 0;
        tareRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
      }
      display_counter++;
      
    }
    // Calibration Scale: Tare successful
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 2) { // CALIBRATION SUB-MENU SCREEN
      //Serial.println("In calibrate screen 2. Process screen 2"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Tare"); 
      u8g2.setCursor(0, 30);
      u8g2.print("Compelete");         
      //------------------------- DELAY -------------------------------   
      //Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER/2){ // Delay
        process_screen = 5;
        display_counter = 0;
      }
      display_counter++;
      //----------------------------------------------------------------------------     
    }  
    // Calibration Scale: FAILED to tare
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 3) { // CALIBRATION SUB-MENU SCREEN
      //Serial.println("In calibrate screen 2. Process screen 3"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Tare"); 
      u8g2.setCursor(0, 30);
      u8g2.print("Failed.");          
      //------------------------- DELAY -------------------------------   
      //Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // Delay
        process_screen = 0;
        display_counter = 0;
        current_screen = 0;
      }
      display_counter++;
      //----------------------------------------------------------------------------     
    }  
    // Calibration Scale: disconnected
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 4) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In tare screen 2. Process screen 4"); // DEBUGGING CODE
      Serial.println("Disconnected from server");
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Disconnected...");       
      //------------------------- DELAY -------------------------------   
      //Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // Delay
        process_screen = 0;
        display_counter = 0;
        current_screen = 0;
      }
      display_counter++;
      //----------------------------------------------------------------------------     
    }  
    // Calibration: Calibrate with 120g weight
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 5) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 2. Process screen 5"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Place a 120g"); 
      u8g2.setCursor(0, 30);
      u8g2.print("weight on scale");  
      u8g2.setCursor(0, 45);
      u8g2.println("Enter once placed");    
      // Wait for yser confirmation (clicked button)    
      
    } // WAIT for scale to take 120g calibration reading
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 6) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 2, process screen 6"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Reading..."); 
      //------------------------- CALIBRATE WITH 10g --------------------------------
      if (connected) { // Read values from server
        calibrate_weight = 120;
        String newValue = "120";
        // Set the characteristic's value to be the array of bytes that is actually a string.
        calibrateRemoteCharacteristic->writeValue(newValue.c_str(),newValue.length()); 
        process_screen = 9;
      }else{
        process_screen = 4;
      }
      
    }
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 9) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 2, process screen 9"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Reading..."); 
      //------------------------- WAIT FOR SCALE READING CONFIRMATION --------------------------------  
      // If we are connected to a peer BLE Server, update the TARE characteristic
        if (connected) { // Read values from server
          std::string value = calibrateRemoteCharacteristic->readValue(); 
          Serial.println(value.c_str());
          if(value == "ok"){
            calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
            current_screen = 2; process_screen = 7;
            Serial.println("120g reading recieved at server");
          }else if (value =="nok"){
            calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
            current_screen = 2; process_screen = 8;
            Serial.println("FAILED to take 120g reading");
          }
        }else{
          process_screen = 4;
        }
      //--------------------------------------------  
    }
    // SUCCESSFUL read
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 7) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 2, process screen 7"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Success. Remove weight. "); 
      u8g2.setCursor(0, 30);
      u8g2.print("Enter to continue.");    
    }
    // FAILED to read
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 8) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 2, process screen 8"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Failed to"); 
      u8g2.setCursor(0, 30);
      u8g2.print("read."); 
      if(display_counter == DELAY_COUNTER){ // Delay
        process_screen = 0;
        current_screen == 0;
        display_counter = 0;
      }
      display_counter++;     
    }

    // Failed to save to EEPROM
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 9) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 2, process screen 9"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Failed to"); 
      u8g2.setCursor(0, 30);
      u8g2.print("save calVal."); 
      if(display_counter == DELAY_COUNTER){ // Delay
        process_screen = 0;
        current_screen == 0;
        display_counter = 0;
      }
      display_counter++;     
    }

    // Calibration: Calibrate with 155g weight
    else if ((current_screen == 3) && String(menu_items[item_selected])=="Calibrate" && process_screen == 0) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 3. Process screen 0"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Place a 155g"); 
      u8g2.setCursor(0, 30);
      u8g2.print("weight on scale");  
      u8g2.setCursor(0, 45);
      u8g2.println("Enter once placed");    
      // Wait for yes confirmation (clicked button)    
      
    } // WAIT for scale to take 155g calibration reading
    else if ((current_screen == 3) && String(menu_items[item_selected])=="Calibrate" && process_screen == 1) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 3, process screen 1"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Reading..."); 
      //------------------------- CALIBRATE WITH 20g --------------------------------
      if (connected) { // Read values from server
        calibrate_weight = 155;
        String newValue = "155";
        // Set the characteristic's value to be the array of bytes that is actually a string.
        calibrateRemoteCharacteristic->writeValue(newValue.c_str(),newValue.length()); 
        current_screen = 3;
        process_screen = 2;
      }else{
        current_screen = 2;
        process_screen = 4;
      }
      
    }
    else if ((current_screen == 3) && String(menu_items[item_selected])=="Calibrate" && process_screen == 2) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 3, process screen 2"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Reading..."); 
      //------------------------- WAIT FOR SCALE READING CONFIRMATION --------------------------------  
      // If we are connected to a peer BLE Server, update the TARE characteristic
        if (connected) { // Read values from server
          std::string value = calibrateRemoteCharacteristic->readValue(); 
          Serial.println(value.c_str());
          if(value == "ok"){
            calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
            current_screen = 2; process_screen = 7;
            Serial.println("155g reading recieved at server");
          }else if (value =="nok"){
            calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
            current_screen = 2; process_screen = 8;
            Serial.println("FAILED to take 155g reading");
          }
        }else{
          current_screen = 2; process_screen = 4;
        }
      //--------------------------------------------  
    }

    // Calibration: Calibrate with 190g weight
    else if ((current_screen == 4) && String(menu_items[item_selected])=="Calibrate" && process_screen == 0) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 4. Process screen 0"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Place a 190g"); 
      u8g2.setCursor(0, 30);
      u8g2.print("weight on scale");  
      u8g2.setCursor(0, 45);
      u8g2.println("Enter once placed");    
      // Wait for yes confirmation (clicked button)    
      
    } // WAIT for scale to take 190g calibration reading
    else if ((current_screen == 4) && String(menu_items[item_selected])=="Calibrate" && process_screen == 1) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 4, process screen 1"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Reading..."); 
      //------------------------- CALIBRATE WITH 190g --------------------------------
      if (connected) { // Read values from server
        calibrate_weight = 190;
        String newValue = "190";
        // Set the characteristic's value to be the array of bytes that is actually a string.
        calibrateRemoteCharacteristic->writeValue(newValue.c_str(),newValue.length()); 
        current_screen = 4;
        process_screen = 2;
      }else{
        current_screen = 2;
        process_screen = 4;
      }
    }

    else if ((current_screen == 4) && String(menu_items[item_selected])=="Calibrate" && process_screen == 2) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 4, process screen 2"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Reading..."); 
      //------------------------- WAIT FOR SCALE READING CONFIRMATION --------------------------------  
      // If we are connected to a peer BLE Server, update the TARE characteristic
        if (connected) { // Read values from server
          std::string value = calibrateRemoteCharacteristic->readValue(); 
          Serial.println(value.c_str());
          if(value == "ok"){
            calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
            current_screen = 2; process_screen = 7;
            Serial.println("190g reading recieved at server");
          }else if (value =="nok"){
            calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
            current_screen = 2; process_screen = 8;
            Serial.println("FAILED to take 190g reading");
          }
        }else{
          current_screen = 2;
        process_screen = 4;
        }
      //-------------------------------------------- 
    }
    // Calibration: Calibrate with 225g weight
    else if ((current_screen == 6) && String(menu_items[item_selected])=="Calibrate" && process_screen == 0) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 6. Process screen 0"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Place a 225g"); 
      u8g2.setCursor(0, 30);
      u8g2.print("weight on scale");  
      u8g2.setCursor(0, 45);
      u8g2.println("Enter once placed");    
      // Wait for yes confirmation (clicked button)    
      
    } // WAIT for scale to take 225g calibration reading
    else if ((current_screen == 6) && String(menu_items[item_selected])=="Calibrate" && process_screen == 1) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 6, process screen 1"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Reading..."); 
      //------------------------- CALIBRATE WITH 225g --------------------------------
      if (connected) { // Read values from server
        calibrate_weight = 225;
        String newValue = "225";
        // Set the characteristic's value to be the array of bytes that is actually a string.
        calibrateRemoteCharacteristic->writeValue(newValue.c_str(),newValue.length()); 
        current_screen = 6;
        process_screen = 2;
      }else{
        current_screen = 2;
        process_screen = 4;
      }      
    }

    else if ((current_screen == 6) && String(menu_items[item_selected])=="Calibrate" && process_screen == 2) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 6, process screen 2"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Reading..."); 
      //------------------------- WAIT FOR SCALE READING CONFIRMATION --------------------------------  
      // If we are connected to a peer BLE Server, update the TARE characteristic
        if (connected) { // Read values from server
          std::string value = calibrateRemoteCharacteristic->readValue(); 
          Serial.println(value.c_str());
          if(value == "ok"){
            calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
            current_screen = 2; process_screen = 7;
            Serial.println("225g reading recieved at server");
          }else if (value =="nok"){
            calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
            current_screen = 2; process_screen = 8;
            Serial.println("FAILED to take 225g reading");
          }
        }else{
          current_screen = 2;
          process_screen = 4;
        }
      //--------------------------------------------     
    }
    // Calibration: Calibrate with 265g weight
    else if ((current_screen == 7) && String(menu_items[item_selected])=="Calibrate" && process_screen == 0) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 7. Process screen 0"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Place a 265g"); 
      u8g2.setCursor(0, 30);
      u8g2.print("weight on scale");  
      u8g2.setCursor(0, 45);
      u8g2.println("Enter once placed");    
      // Wait for yes confirmation (clicked button)    
      
    } // WAIT for scale to take 265g calibration reading
    else if ((current_screen == 7) && String(menu_items[item_selected])=="Calibrate" && process_screen == 1) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 7, process screen 1"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Reading..."); 
      //------------------------- CALIBRATE WITH 265g --------------------------------
      if (connected) { // Read values from server
        calibrate_weight = 265;
        String newValue = "265";
        // Set the characteristic's value to be the array of bytes that is actually a string.
        calibrateRemoteCharacteristic->writeValue(newValue.c_str(),newValue.length()); 
        current_screen = 7;
        process_screen = 2;
      }else{
        current_screen = 2;
        process_screen = 4;
      }      
    }

    else if ((current_screen == 7) && String(menu_items[item_selected])=="Calibrate" && process_screen == 2) { // TARE SCALE SUB-MENU SCREEN
      Serial.println("In calibrate screen 7, process screen 2"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Reading..."); 
      //------------------------- WAIT FOR SCALE READING CONFIRMATION --------------------------------  
      // If we are connected to a peer BLE Server, update the TARE characteristic
        if (connected) { // Read values from server
          std::string value = calibrateRemoteCharacteristic->readValue(); 
          Serial.println(value.c_str());
          if(value == "ok"){
            calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
            current_screen = 2; process_screen = 7;
            Serial.println("265g reading recieved at server");
          }else if (value =="nok"){
            calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
            current_screen = 2; process_screen = 8;
            Serial.println("FAILED to take 265g reading");
          }
        }else{
          current_screen = 2;
          process_screen = 4;
        }
      //--------------------------------------------  


    }
    // Calibration: Save calibration value
    else if ((current_screen == 5) && String(menu_items[item_selected])=="Calibrate" && process_screen == 0) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 5. Process screen 0"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Saving calibration"); 
      u8g2.setCursor(0, 30);
      u8g2.print("value now...");          
      //------------------------- SAVE CAL VAL -------------------------------
      if (connected){
        std::string value = calibrateRemoteCharacteristic->readValue(); 
        Serial.println(value.c_str());
        static String saveMsg = "save";
        calibrateRemoteCharacteristic->writeValue(saveMsg.c_str(),saveMsg.length()); // tell the server to begin saving value
        Serial.println("save val sent to server");
        current_screen = 5; process_screen = 1;
      }else{
        current_screen = 2;
      process_screen = 4;
      }
      //----------------------------------------------------------------------------     
    }

    // Calibration: Save calibration value
    else if ((current_screen == 5) && String(menu_items[item_selected])=="Calibrate" && process_screen == 1) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 5. Process screen 1"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Saving calibration"); 
      u8g2.setCursor(0, 30);
      u8g2.print("value now...");          
      //------------------------- SAVE CAL VAL -------------------------------
      if (connected){
        std::string value = calibrateRemoteCharacteristic->readValue(); 
        Serial.println(value.c_str());
        if(value == "ok"){
          Serial.println("Sucessfully saved EEPROM value");
          calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to resetcharacteristic value
          current_screen = 5; process_screen = 2;
        }else if (value =="nok"){
          calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
          current_screen = 2; process_screen = 9;
          Serial.println("FAILED to save Calibration Value");
        }
      }else{
        current_screen = 2;
      process_screen = 4;
      }
      //----------------------------------------------------------------------------     
    }

    // Calibration: Saved cal val to EEPROM
    else if ((current_screen == 5) && String(menu_items[item_selected])=="Calibrate" && process_screen == 2) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 5. Process screen 2"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Saved Calibration"); 
      u8g2.setCursor(0, 30);
      u8g2.print("Value to EEPROM");          
      //------------------------- CONFIRMATION OF SAVE CAL VAL -------------------------------   
      // If we are connected to a peer BLE Server, update the TARE characteristic
      if (connected) { // Read values from server
        std::string value = calibrateRemoteCharacteristic->readValue(); 
        Serial.println(value.c_str());
        static String doneMsg = "done";
        calibrateRemoteCharacteristic->writeValue(doneMsg.c_str(),doneMsg.length()); // tell the server to begin saving value
        Serial.println("done val sent to server");
        current_screen = 5; process_screen = 3;
      }else{
        current_screen = 2; process_screen = 4;
      }
      //----------------------------------------------------------------------------     
    }   

    // Calibration: Complete calibration
    else if ((current_screen == 5) && String(menu_items[item_selected])=="Calibrate" && process_screen == 3) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 5. Process screen 3"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Calibration"); 
      u8g2.setCursor(0, 30);
      u8g2.print("Complete");          
      //------------------------- SAVE CAL VAL ------------------------------- 
      if (connected) { // Read values from server
        std::string value = calibrateRemoteCharacteristic->readValue(); 
        Serial.println(value.c_str());
        if(value == "ok"){
          calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
          Serial.println("Calibration complete");
        }else if (value =="nok"){
          calibrateRemoteCharacteristic->writeValue(rstMsg.c_str(),rstMsg.length()); // tell the server to reset the characteristic value
          current_screen = 2; process_screen = 8;
          Serial.println("FAILED to complete calibbration");
        }
      }else{
        current_screen = 2; process_screen = 4;
      }  
      Serial.println(display_counter);
      // Delay
      if(display_counter == DELAY_COUNTER/2){ // Delay
        process_screen = 0;
        display_counter = 0;
        current_screen = 0;
      }
      display_counter++;
      //----------------------------------------------------------------------------     
    }             

    //------------------------- CONNECT -------------------------------   
    else if ((current_screen == 1) && !connected && String(menu_items[item_selected])=="Connect"  && process_screen == 0) { // SCALE SUB-MENU SCREEN
      Serial.println("In Connect screen"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.println("Searching for");
      u8g2.setCursor(0, 30);
      u8g2.println("scale...");
      //------------------------- BLE SERVER SEARCH --------------------------------  
      // Retrieve a Scanner and set the callback we want to use to be informed when we
      // have detected a new device.  Specify that we want active scanning and start the
      // scan to run for 5 seconds.
      BLEScan* pBLEScan = BLEDevice::getScan();
      pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
      pBLEScan->setInterval(1349);
      pBLEScan->setWindow(449);
      pBLEScan->setActiveScan(true);
      pBLEScan->start(0); // Do single scan for devices/servers

      // If the flag "doConnect" is true then we have scanned for and found the desired
      // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
      // connected we set the connected flag to be true.
      Serial.println(doConnect);
      if (doConnect) {
        if (connectToServer()) { // IF connected --> NEXT SCREEN
          Serial.println("We are now connected to the BLE Server.");
          process_screen =1; // Move to next screen
        }else { // IF failed to connect --> NEXT SCREEN
          Serial.println("Failed to connect to server.");
          process_screen=2;
        }
      } else { // IF failed to connect --> NEXT SCREEN
        Serial.println("Failed to find server.");
        process_screen=2;
      }
      //doConnect=false;
      //----------------------------------------------------------------------------
    }    
    else if ((current_screen == 1) && String(menu_items[item_selected])=="Connect" && process_screen ==1) { // SCALE SUB-MENU SCREEN
      Serial.println("Connected");
      // u8g2.clear();
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Connected To");
      u8g2.setCursor(0, 30);
      u8g2.print(myDevice->getName().c_str());
      if(display_counter == DELAY_COUNTER){ // Delay did not work :(
        process_screen = 3;
        display_counter = 0;
      }
      display_counter++;
    }
    else if ((current_screen == 1) && String(menu_items[item_selected])=="Connect" && process_screen ==2) { // SCALE SUB-MENU SCREEN
      Serial.println("Server not found");
      // u8g2.clear();
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Connect Failed.");  // Add Perch name (get from server)
      if(display_counter == DELAY_COUNTER){ // Delay did not work :(
        process_screen = 3;
        display_counter = 0;
      }
      display_counter++;
    }
    else if ((current_screen == 1) && String(menu_items[item_selected])=="Connect" && process_screen ==3) { // SCALE SUB-MENU SCREEN
      Serial.println("Exit");
      current_screen = 0;
      process_screen = 0;
    }
    else if ((current_screen == 1) && connected && String(menu_items[item_selected])=="Connect"  && process_screen == 0) { // SCALE SUB-MENU SCREEN
      Serial.println("In Already connected screen"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.println("Already");
      u8g2.setCursor(0, 30);
      u8g2.println("connected");
      //---------------DELAY---------------------------
      Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // Delay
        process_screen = 0;
        current_screen == 0;
        display_counter = 0;
      }
      display_counter++; 
          
    
    }

    //------------------------- DISCONNECT -------------------------------   
    else if ((current_screen == 1) && connected && String(menu_items[item_selected])=="Disconnect"  && process_screen == 0) { // SCALE SUB-MENU SCREEN
      Serial.println("In Disconnect screen"); // DEBUGGING CODE
      Serial.print("Attempting to disconnecting from  ");
      Serial.println(myDevice->getAddress().toString().c_str());

      // Disconnect from the BLE Server.
      pClient->disconnect();
     
      process_screen = 1;
      //----------------------------------------------------------------------------
    }// Sucessfullly disconnected
    else if ((current_screen == 1) && !connected && String(menu_items[item_selected])=="Disconnect"  && process_screen == 1) { // SCALE SUB-MENU SCREEN
       Serial.println("Disconnected");
       
       u8g2.setFont(u8g_font_7x14B);
       u8g2.setCursor(0, 15);
       u8g2.print("Successfully");
       u8g2.setCursor(0, 30);
       u8g2.print("disconnected.");
       if(display_counter == DELAY_COUNTER){ // Exit to main menu
        current_screen = 0;
        process_screen = 0;
        display_counter = 0;
      }
        display_counter++;
      //----------------------------------------------------------------------------
    }// Unsucessfullly disconnected
    else if ((current_screen == 1) && connected && String(menu_items[item_selected])=="Disconnect"  && process_screen == 1) { // SCALE SUB-MENU SCREEN
       Serial.println("Disconnected");
       
       u8g2.setFont(u8g_font_7x14B);
       u8g2.setCursor(0, 15);
       u8g2.print("Still");
       u8g2.setCursor(0, 30);
       u8g2.print("connected.");
       if(display_counter == DELAY_COUNTER){ // Exit to main menu
        current_screen = 0;
        process_screen = 0;
        display_counter = 0;
      }
        display_counter++;
      //----------------------------------------------------------------------------
    }

  u8g2.sendBuffer(); // send buffer from RAM to display controller

}

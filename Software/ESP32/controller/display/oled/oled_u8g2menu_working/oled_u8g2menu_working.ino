// simple project using Arduino UNO and 128x64 OLED Display to display a menu
// created by upir, 2022
// youtube channel: https://www.youtube.com/upir_upir

// YOUTUBE VIDEO: https://youtu.be/HVHVkKt-ldc
// YOUTUBE VIDEO u8g2 version: https://youtu.be/K5e0lFRvZ2E

// links from the video:
// Flipper Zero menu - https://docs.flipperzero.one/basics/control#M5BZO
// WOKWI start project progress bar - https://wokwi.com/projects/300867986768527882
// image2cpp - https://javl.github.io/image2cpp/
// 128x64 SSD1306 OLED Display: https://s.click.aliexpress.com/e/_DCKdvnh
// Transparent OLED display: https://s.click.aliexpress.com/e/_Dns6eLz
// Arduino UNO: https://s.click.aliexpress.com/e/_AXDw1h
// Arduino UNO MINI: https://store.arduino.cc/products/uno-mini-le
// Big OLED Display: https://s.click.aliexpress.com/e/_ADL0T9
// Arduino breadboard prototyping shield: https://s.click.aliexpress.com/e/_ApbCwx
// u8g fonts (fonts available for u8g library): https://nodemcu-build.com/u8g-fonts.php
// u8g documentation: https://github.com/olikraus/u8glib/wiki/userreference
// Photopea (online Photoshop-like tool): https://www.photopea.com/
// image2cpp (convert images into C code): https://javl.github.io/image2cpp/
// Push buttons - https://s.click.aliexpress.com/e/_DmXS8B9
// LCD displays: https://s.click.aliexpress.com/e/_DBgR45P
// u8g2 documentation: https://github.com/olikraus/u8g2/wiki/u8gvsu8g2
// Image Magick: https://imagemagick.org/index.php
// LCD Image converter: https://lcd-image-converter.riuson.com/en/about/


// Related videos:
// Arduino Parking Sensor - https://youtu.be/sEWw087KOj0
// Turbo pressure gauge with Arduino and OLED display - https://youtu.be/JXmw1xOlBdk
// Arduino Car Cluster with OLED Display - https://youtu.be/El5SJelwV_0
// Knob over OLED Display - https://youtu.be/SmbcNx7tbX8
// Arduino + OLED = 3D ? - https://youtu.be/kBAcaA7NAlA
// Arduino OLED Gauge - https://youtu.be/xI6dXTA02UQ
// Smaller & Faster Arduino - https://youtu.be/4GfPQoIRqW8



#include "U8g2lib.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// String
#include <string>
//--------------------------------------------------


//--------------------DEFINES-------------------------
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DELAY_COUNTER 60 // Delay for text on screen
//----------------------------------------------------

//--------------------GLOBAL VARIABLES-----------------

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Instantiate an SSD1306 display connected to I2C

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0); // [full framebuffer, size = 1024 bytes]

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



// ------------------ end generated bitmaps from image2cpp ---------------------------------



const int NUM_ITEMS = 4; // number of items in the list
const int MAX_ITEM_LENGTH = 10; // maximum characters for the item name

char menu_items [NUM_ITEMS] [MAX_ITEM_LENGTH] = {  // array with item names
  { "Read" }, 
  { "Tare" }, 
  { "Calibrate" },
  { "Connect" }
 };
// note - when changing the order of items above, make sure the other arrays referencing bitmaps
// also have the same order, for example array "bitmap_icons" for icons, and other arrays for screenshots and QR codes

#define BUTTON_UP_PIN 15 // pin for UP button 
#define BUTTON_SELECT_PIN 27 // pin for SELECT button
#define BUTTON_DOWN_PIN 26 // pin for DOWN button
#define BUTTON_BACK_PIN 4 // pin for BACK button

int display_counter = 0;
int process_screen = 0;

int button_up_clicked = 0; // only perform action when button is clicked, and wait until another press
int button_select_clicked = 0; // same as above
int button_down_clicked = 0; // same as above
int button_back_clicked = 0; // same as above

int item_selected = 0; // which item in the menu is selected

int item_sel_previous; // previous item - used in the menu screen to draw the item before the selected one
int item_sel_next; // next item - used in the menu screen to draw next item after the selected one

int current_screen = 0;

void setup() {

  // SERIAL SETUP
  Serial.begin(9600); // Initialise baud rate with PC

  u8g2.setColorIndex(1);  // set the color to white
  u8g2.begin();
  u8g2.setBitmapMode(1);

  // define pins for buttons
  // INPUT_PULLUP means the button is HIGH when not pressed, and LOW when pressed
  // since it´s connected between some pin and GND
  pinMode(BUTTON_UP_PIN, INPUT); // up button
  pinMode(BUTTON_SELECT_PIN, INPUT); // select button
  pinMode(BUTTON_DOWN_PIN, INPUT); // down button
  pinMode(BUTTON_BACK_PIN, INPUT); // down button
  
  Serial.println("Setup Complete!");
}


void loop() {

  if (current_screen == 0) { // MENU SCREEN

      // up and down buttons only work for the menu screen
      if ((digitalRead(BUTTON_UP_PIN) == LOW) && (button_up_clicked == 0)) { // up button clicked - jump to previous menu item
        item_selected = item_selected - 1; // select previous item
        Serial.print("Item Selected:");
        Serial.println(String(menu_items[item_selected]));        
        button_up_clicked = 1; // set button to clicked to only perform the action once
        if (item_selected < 0) { // if first item was selected, jump to last item
          item_selected = NUM_ITEMS-1;
        }
      }
      else if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && (button_down_clicked == 0)) { // down button clicked - jump to next menu item       
        item_selected = item_selected + 1; // select next item
        Serial.print("Item Selected:");
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

// Select click
  if ((digitalRead(BUTTON_SELECT_PIN) == LOW) && (button_select_clicked == 0)) { // select button clicked, jump between screens
    button_select_clicked = 1; // set button to clicked to only perform the action once
    
    if ((current_screen == 0)) {current_screen = 1;} // main menu --> scale screen
    //  else if (current_screen == 1 && String(menu_items[item_selected])!="Calibrate") {current_screen = 0;} // scale screen --> main menu
     else if (current_screen == 1 && String(menu_items[item_selected])=="Calibrate") {current_screen = 2;} // COMMENT
     else if (current_screen == 2 && String(menu_items[item_selected])=="Calibrate") {
      process_screen = 0;
      current_screen = 3;}
     else if (current_screen == 3 && String(menu_items[item_selected])=="Calibrate") {current_screen = 4;}
     else if (current_screen == 4 && String(menu_items[item_selected])=="Calibrate") {current_screen = 5;}
    //  else {current_screen = 0;} // qr codes screen --> menu items screen
  }
  if ((digitalRead(BUTTON_SELECT_PIN) == HIGH) && (button_select_clicked == 1)) { // unclick 
    button_select_clicked = 0;
  }
// Back click
  if ((digitalRead(BUTTON_BACK_PIN) == LOW) && (button_back_clicked == 0)) { // select button clicked, jump between screens
     button_back_clicked = 1; // set button to clicked to only perform the action once
     if (current_screen == 1) {current_screen = 0;} // scale screen --> main menu
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

      // draw bottom right logo
      // u8g2.drawXBMP(128-16-4, 64-4, 16, 4, upir_logo);               

    } 

    //------------------------- READ SCALE -------------------------------     

    // Scale: Read
    else if ((current_screen == 1) && String(menu_items[item_selected])=="Read" && process_screen == 0) { // READ SCALE SUB-MENU SCREEN
      Serial.println("In Read screen"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("read weight");    
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
      Serial.println("In Tare screen 2"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Taring..."); 
      u8g2.setCursor(0, 30);
      u8g2.print("Please wait");      
      //------------------------- TARE SCALE --------------------------------   
      Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // If tared
        process_screen = 1;
        display_counter = 0;
        current_screen = 2;
      }
      display_counter++;
      //----------------------------------------------------------------------------      
    }

    // Scale: Tare complete
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Tare" && process_screen == 1) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In tare screen 2. Process screen 1"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Tare"); 
      u8g2.setCursor(0, 30);
      u8g2.print("Compelete");          
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
      Serial.println("In calibrate screen"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Clear scale");
      u8g2.setCursor(0, 30);
      u8g2.println("Enter once clear");    
    }

    // Calibration: Taring
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 0) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 2"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Taring..."); 
      u8g2.setCursor(0, 30);
      u8g2.print("Please wait");      
      //------------------------- TARE SCALE --------------------------------   
      Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // If tared
        process_screen = 1;
        display_counter = 0;
        current_screen = 2;
      }
      display_counter++;
      //----------------------------------------------------------------------------     
    }    

    // Calibration: Calibrate with 10g weight
    else if ((current_screen == 2) && String(menu_items[item_selected])=="Calibrate" && process_screen == 1) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 2. Process screen 1"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Place a 10g"); 
      u8g2.setCursor(0, 30);
      u8g2.print("weight on scale");  
      u8g2.setCursor(0, 45);
      u8g2.println("Enter once placed");          
      //------------------------- CALIBRATE WITH 10g --------------------------------   
      Serial.println(display_counter);
      // if(display_counter == DELAY_COUNTER){ // If succesfully got cal val for 10g
      //   process_screen = 0;
      //   display_counter = 0;
      //   current_screen = 3;
      // }
      display_counter++;
      //----------------------------------------------------------------------------     
    }   

    // Calibration: Calibrate with 20g weight
    else if ((current_screen == 3) && String(menu_items[item_selected])=="Calibrate" && process_screen == 0) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 3. Process screen 0"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Place a 20g"); 
      u8g2.setCursor(0, 30);
      u8g2.print("weight on scale");  
      u8g2.setCursor(0, 45);
      u8g2.println("Enter once placed");          
      //------------------------- CALIBRATE WITH 20g --------------------------------   
      // Serial.println(display_counter);
      // if(display_counter == DELAY_COUNTER){ // If succesfully got cal val for 20g
      //   process_screen = 0;
      //   display_counter = 0;
      //   current_screen = 4;
      // }
      // display_counter++;
      //----------------------------------------------------------------------------     
    }       

    // Calibration: Calibrate with 200g weight
    else if ((current_screen == 4) && String(menu_items[item_selected])=="Calibrate" && process_screen == 0) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 4. Process screen 0"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Place a 200g"); 
      u8g2.setCursor(0, 30);
      u8g2.print("weight on scale");  
      u8g2.setCursor(0, 45);
      u8g2.println("Enter once placed");          
      //------------------------- CALIBRATE WITH 200g -------------------------------   
      // Serial.println(display_counter);
      // if(display_counter == DELAY_COUNTER){ // If succesfully got cal val for 200g
      //   process_screen = 0;
      //   display_counter = 0;
      //   current_screen = 5;
      // }
      // display_counter++;
      //----------------------------------------------------------------------------     
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
      Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // If can save overall cal val
        process_screen = 1;
        display_counter = 0;
        current_screen = 5;
      }
      display_counter++;
      //----------------------------------------------------------------------------     
    }             

    // Calibration: Complete calibration
    else if ((current_screen == 5) && String(menu_items[item_selected])=="Calibrate" && process_screen == 1) { // CALIBRATION SUB-MENU SCREEN
      Serial.println("In calibrate screen 5. Process screen 1"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Calibration"); 
      u8g2.setCursor(0, 30);
      u8g2.print("Compelte");          
      //------------------------- SAVE CAL VAL -------------------------------   
      Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // Delay
        process_screen = 0;
        display_counter = 0;
        current_screen = 0;
      }
      display_counter++;
      //----------------------------------------------------------------------------     
    }             

    //------------------------- CONNECT -------------------------------   


    else if ((current_screen == 1) && String(menu_items[item_selected])=="Connect"  && process_screen == 0) { // SCALE SUB-MENU SCREEN
      Serial.println("In Connect screen"); // DEBUGGING CODE
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.println("Searching for");
      u8g2.setCursor(0, 30);
      u8g2.println("scale...");
      //------------------------- BLE SERVER SEARCH --------------------------------   
      Serial.println(display_counter);
      if(display_counter == DELAY_COUNTER){ // If can find server
        process_screen =1;
        display_counter = 0;
      }
      display_counter++;
      //----------------------------------------------------------------------------
    }    
    else if ((current_screen == 1) && String(menu_items[item_selected])=="Connect" && process_screen ==1) { // SCALE SUB-MENU SCREEN
      Serial.println("Connected");
      // u8g2.clear();
      u8g2.setFont(u8g_font_7x14B);
      u8g2.setCursor(0, 15);
      u8g2.print("Connected To xxx.");  // Add Perch name (get from server)
      if(display_counter == 60){ // Delay did not work :(
        process_screen = 2;
      }
      display_counter++;
    }
    else if ((current_screen == 1) && String(menu_items[item_selected])=="Connect" && process_screen ==2) { // SCALE SUB-MENU SCREEN
      Serial.println("Exit");
      current_screen = 0;
      process_screen = 0;
      display_counter = 0;
    }
    else if (current_screen == 2) { // SUB-MENU SCREEN
      display.clearDisplay();
      display.setTextSize(3);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.print("Connect"); 
    }


  u8g2.sendBuffer(); // send buffer from RAM to display controller


}
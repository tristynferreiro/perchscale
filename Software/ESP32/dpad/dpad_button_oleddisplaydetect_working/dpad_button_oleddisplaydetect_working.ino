/*
* Script to output the state of the DPAD buttons to the OLED screen
*
* Working: 07/06/24
*
* Pin connections:
*   SCL of OLED -> SCL (PIN 22) of ESP32
*   SDA of OLED -> SDA (PIN 21) of ESP32
*   GND of OLED -> GND of ESP32
*   Vcc of OLED -> 3.3V of ESP32
*
*   DPAD_UP -> D15
*   DPAD_R -> D2
*   DPAD_L -> D4
*   DPAD_DOWN -> D13
*/
// Interesting resource explaining how BLE works: https://devzone.nordicsemi.com/guides/short-range-guides/b/bluetooth-low-energy/posts/ble-characteristics-a-beginners-tutorial

//------------------INCLUDES------------------------
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
// OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// String
#include <string>
#endif
//--------------------------------------------------


//--------------------DEFINES-------------------------
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
//----------------------------------------------------

//--------------------GLOBAL VARIABLES-----------------

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Instantiate an SSD1306 display connected to I2C

// Pins:
const int DPAD_UP = 15;
const int DPAD_R = 27;
const int DPAD_L = 4;
const int DPAD_DOWN = 26;

// Define default state of DPAD buttons
int dpad_up_state = 0;
int dpad_r_state = 0;
int dpad_l_state = 0;
int dpad_down_state = 0;
//-----------------------------------------------------

//--------------------HELPER FUNCTIONS-----------------
// Add your helper functions here
//-----------------------------------------------------

void setup() {

  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
  Serial.println(F("SSD1306 allocation failed"));
  for(;;);
  }
  delay(2000);

  // Setup Pins
  pinMode(DPAD_UP,INPUT);
  pinMode(DPAD_R,INPUT);
  pinMode(DPAD_L,INPUT);
  pinMode(DPAD_DOWN,INPUT);
}

void loop() {
  
  dpad_up_state = digitalRead(DPAD_UP);
  dpad_r_state = digitalRead(DPAD_R);
  dpad_l_state = digitalRead(DPAD_L);
  dpad_down_state = digitalRead(DPAD_DOWN);

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("UP:");
  display.println(String(dpad_up_state));
  display.print("R:");
  display.println(String(dpad_r_state));
  display.print("L:");
  display.println(String(dpad_l_state));
  display.print("DOWN:");
  display.println(String(dpad_down_state));
  display.display(); 
  delay(1000);
}

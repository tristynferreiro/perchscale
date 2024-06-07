/*
* WRITE DESCRIPTIONS
*
* Working: DATE of last working
*
* Pin connections:
*   DPAD_UP = D15
    DPAD_R = D2
    DPAD_L = D4
    DPAD_DOWN = D5
*/
// Interesting resource explaining how BLE works: https://devzone.nordicsemi.com/guides/short-range-guides/b/bluetooth-low-energy/posts/ble-characteristics-a-beginners-tutorial

//------------------INCLUDES------------------------
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
//--------------------------------------------------


//--------------------DEFINES-------------------------
// Add your global defines here
//----------------------------------------------------

//--------------------GLOBAL VARIABLES-----------------
// Pins:
const int DPAD_UP = 15;
const int DPAD_R = 2;
const int DPAD_L = 4;
const int DPAD_DOWN = 5;

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
  Serial.println("UP:" + String(dpad_up_state));
  Serial.println("R:" + String(dpad_r_state));
  Serial.println("L:" + String(dpad_l_state));
  Serial.println("DOWN:" + String(dpad_down_state));
  delay(1000);
}

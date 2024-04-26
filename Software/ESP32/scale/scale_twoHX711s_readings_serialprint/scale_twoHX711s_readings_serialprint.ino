/*
* This script runs calibration on startup and then takes readings from two HX711s (connected to two load cells) and prints them to serial. 
*
* 26/04/2024: Created
*/

//------------------INCLUDES------------------------
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

//--------------------DEFINES-------------------------
#define eigthseconds (millis()/125) // Taring delay

//--------------------GLOBAL VARIABLES-----------------
// pins:
const int HX711_1_dout = 32; // D32 mcu > HX711_1 dout pin 
const int HX711_1_sck = 33; // D33 mcu > HX711_1 sck pin
const int HX711_2_dout = 4; // D4 mcu > HX711_2 dout pin 
const int HX711_2_sck = 32; // D32 mcu > HX711_2 sck pin (Should be the same for multiple HX711s)

HX711_ADC LoadCell1(HX711_1_dout, HX711_1_sck); // instantiate an HX711 object for the first HX711
HX711_ADC LoadCell2(HX711_2_dout, HX711_2_sck); // instantiate an HX711 object for the second HX711

// EEPROM address
const int calVal_eepromAdress_1 = 0;
const int calVal_eepromAdress_2 = 1;


int num_readings = 0; // scale readings counter
//------------------------------------------------------

void setup() {
  // SERIAL SETUP
  Serial.begin(9600); // Initialise baud rate with PC
  
  // LOADCELL SETUP
  LoadCell1.begin();
  LoadCell2.begin();
  // LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell1.start(stabilizingtime, _tare);
  LoadCell2.start(stabilizingtime, _tare);

  if (LoadCell1.getTareTimeoutFlag() || LoadCell1.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711_1 wiring and pin designations");
    while (1);
  }
  if (LoadCell2.getTareTimeoutFlag() || LoadCell2.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711_2 wiring and pin designations");
    while (1);
  }
  // else {
  //   LoadCell.setCalFactor(1.0); // user set calibration value (float), initial value 1.0 may be used for this sketch
  //   Serial.println("Startup is complete");
  // }
  Serial.println("Startup is complete");
  while (!LoadCell1.update() && !LoadCell2.update());
  calibrate(); //start calibration procedure
}

void loop() {

  static boolean newDataReady = 0;

  // check for new data/start next conversion:
  if (LoadCell1.update() && LoadCell2.update()) newDataReady = true;

  if (newDataReady) {
      float reading_1 = LoadCell1.getData();
      float reading_2 = LoadCell2.getData();
      reading = reading_1 + reading_2;
      if(reading<1 & eigthseconds%80 == 0){ //Condition to set new tare every 1000 milliseconds
        boolean _resume = false;
        boolean _tarewait = true;
        while (_resume == false) {
          LoadCell1.update();
          LoadCell2.update();
          if(_tarewait){              
            LoadCell1.tareNoDelay();
            LoadCell2.tareNoDelay();
            _tarewait = false;
          }
          if (LoadCell1.getTareStatus() == true) {
            Serial.println("Next Tare of HX711_1 Complete");
            // _resume = true;
          }
          else if (LoadCell2.getTareStatus() == true) {
            Serial.println("Next Tare of HX711_2 Complete");
            _resume = true;
          }
        }
      }
      else if (reading>1){ //If a bird or heavy object is detected on the scale
         Serial.println(String(num_readings) + "," + String(reading));
          newDataReady = 0;
          num_readings++;
      }
    
  }

  // receive command from serial terminal
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell1.tareNoDelay() // tare load cell 1
      LoadCell2.tareNoDelay(); //tare load cell 2
    }
    else if (inByte == 'r') calibrate(); //calibrate
    else if (inByte == 'c') changeSavedCalFactor(); //edit calibration value manually
  }

  // check if last tare operation is complete
  if (LoadCell1.getTareStatus()) {
    Serial.println("Tare of HX711_1 complete");
  }
  else if (LoadCell2.getTareStatus() == true) {
    Serial.println("Tare of HX711_2 complete");
  }

}

//------------------HELPER FUNCTIONS------------------------
void calibrate() {
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    LoadCell1.update();
    LoadCell2.update();
    if (Serial.available() > 0) {
      if (Serial.available() > 0) {
        char inByte = Serial.read();
        if (inByte == 't') {
          LoadCell1.tareNoDelay();
          LoadCell2.tareNoDelay();
      }
    }
    if (LoadCell1.getTareStatus()) {
      Serial.println("Tare of HX711_1 complete");
      // _resume = true;
    }
    else if (LoadCell2.getTareStatus() == true) {
      Serial.println("Tare of HX711_2 complete");
      _resume = true;
    }
  }

  Serial.println("Now, place your known mass on the perch.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell1.update();
    LoadCell2.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }

  LoadCell1.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  LoadCell2.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  float newCalibrationValue1 = LoadCell1.getNewCalibration(known_mass); //get the new calibration value for HX711_1
  float newCalibrationValue2 = LoadCell2.getNewCalibration(known_mass); //get the new calibration value for HX711_2

  // Print out calibration value for HX711s
  // HX711_1
  Serial.print("New calibration value for HX711_1 has been set to: ");
  Serial.print(newCalibrationValue1);
  Serial.println(", use this as calibration value (calFactor) in your project sketch.");
  // HX711_2
  Serial.print("New calibration value for HX711_2 has been set to: ");
  Serial.print(newCalibrationValue2);
  Serial.println(", use this as calibration value (calFactor) in your project sketch.");  
  Serial.print("Save these values to EEPROM address ");
  Serial.print(calVal_eepromAdress_1);
  Serial.print(calVal_eepromAdress_2);
  Serial.print("respectively");
  Serial.println("? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress_1, newCalibrationValue1);
        EEPROM.put(calVal_eepromAdress_2, newCalibrationValue2);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress_1, newCalibrationValue1);
        Serial.print("HX711_1 Calibration Value ");
        Serial.print(newCalibrationValue1);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress_1);
        EEPROM.get(calVal_eepromAdress_2, newCalibrationValue2);
        Serial.print("HX711_2 Calibration Value ");
        Serial.print(newCalibrationValue2);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress_2);        
        _resume = true;

      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }

  Serial.println("End calibration");
  Serial.println("***");
  Serial.println("To re-calibrate, send 'r' from serial monitor.");
  Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
  Serial.println("***");
}

void changeSavedCalFactor() {
  // Get old calibration values for both HX711s
  float oldCalibrationValue1 = LoadCell1.getCalFactor();
  float oldCalibrationValue2 = LoadCell2.getCalFactor();
  boolean _resume = false;

  Serial.println("***");
  Serial.print("Current value on HX711_1 is: ");
  Serial.println(oldCalibrationValue1);
  Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
  float newCalibrationValue1;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue1 = Serial.parseFloat();
      if (newCalibrationValue1 != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(newCalibrationValue1);
        LoadCell.setCalFactor(newCalibrationValue1);
        // _resume = true;
      }
    }
  }
  Serial.println("***");
  Serial.print("Current value on HX711_2 is: ");
  Serial.println(oldCalibrationValue2);
  Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
  float newCalibrationValue2;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue2 = Serial.parseFloat();
      if (newCalibrationValue2 != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(newCalibrationValue2);
        LoadCell.setCalFactor(newCalibrationValue2);
        _resume = true;
      }
    }
  }  
  _resume = false;
  // Save HX711_1 calibratin value to EEPROM
  Serial.print("Save this value to EEPROM address ");
  Serial.print(calVal_eepromAdress_1);
  Serial.println("? y/n");
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress_1, newCalibrationValue1);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress_1, newCalibrationValue1);
        Serial.print("Value ");
        Serial.print(newCalibrationValue1);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress_1);
        // _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        // _resume = true;
      }
    }
  }

  // Save HX711_2 calibration value to EEPROM
  Serial.print("Save this value to EEPROM address ");
  Serial.print(calVal_eepromAdress_2);
  Serial.println("? y/n");
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress_2, newCalibrationValue2);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress_2, newCalibrationValue2);
        Serial.print("Value ");
        Serial.print(newCalibrationValue2);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress_2);
        _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }
  Serial.println("End change calibration value");
  Serial.println("***");
}
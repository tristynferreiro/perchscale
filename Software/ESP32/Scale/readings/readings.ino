#include "HX711_ADC.h"

#define calibration_factor 795.11 //This value is obtained using the SparkFun_HX711_Calibration sketch

// Setup I2C -- HX711 pins:
const int HX711_dout = 32; // D32 mcu > HX711 dout pin 
const int HX711_sck = 33; // D33 mcu > HX711 sck pin

// Create HX711 object (to use the library)
HX711_ADC scale(HX711_dout, HX711_sck);

unsigned int reading_counter = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 scale demo");

  scale.begin();
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  scale.start(stabilizingtime, _tare);

  if (scale.getTareTimeoutFlag() || scale.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    scale.setCalFactor(calibration_factor); // user set calibration value (float), initial value 1.0 may be used for this sketch
    Serial.println("Startup is complete");
  }

}

void loop() {
  //Serial.print(scale.getCalFactor());
  /*
  if (reading_counter ==0){
    scale.tare();
  }
  Serial.print("Reading: ");
  float i = scale.getData();
  Serial.print(i*10, 3); //scale.get_units() returns a float
  Serial.print(" g"); //You can change this to kg but you'll need to refactor the calibration_factor
  Serial.println();
  */
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (scale.update()) newDataReady = true;


  // get smoothed value from the dataset:
  if (newDataReady) {
   
      float i = scale.getData();
      if(i<5 & millis()%100 == 0){ //Condition to set new tare every 100 milliseconds
        boolean _resume = false;
        boolean _tarewait = true;
        while (_resume == false) {
          scale.update();
          if(_tarewait){              
            scale.tareNoDelay();
            _tarewait = false;
          }
          //Serial.println("In the tare");
          if (scale.getTareStatus() == true) {
            Serial.println("Next Tare Complete");
            _resume = true;
          }
        }
      }
      else if (i>10){ //If a bird or heavy object is detected on the scale
        Serial.print("Load_cell output val: ");
        Serial.println(i);
        newDataReady = 0;
     
      }
      else{ 
        newDataReady = 0;
      }
  }

  // check if last tare operation is complete
  if (scale.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
}

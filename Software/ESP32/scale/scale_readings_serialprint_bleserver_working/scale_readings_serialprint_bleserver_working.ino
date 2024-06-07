/*
* This script uses commands from BLE to trigger functionality on scale and prints to serial.
* BLE Commands:
*   calibrate characteristic: any msg from client will activate calibration
*   tare characteristic: any msg from client will activate tare
*   read characteristic: if msg = 0, will stop displaying readings else any msg from client will start taking readings
*
* Working: 07/06/2024

* Pin connections:
*   SCL of RTC -> SCL (PIN 22) of ESP32
*   SDA of RTC -> SDA (PIN 21) of ESP32
*   GND of RTC -> GND of ESP32
*   Vcc of RTC -> 5V of ESP32
*
*   SCL of HX -> SCL (PIN 33) of ESP32
*   SDA of HX -> SDA (PIN 32) of ESP32
*   GND of HX -> GND of ESP32
*   Vcc of HX -> 5V of ESP32
*/

//------------------INCLUDES------------------------
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
// BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//--------------------DEFINES-------------------------
#define eigthseconds (millis()/125) // Taring delay

// BLE:
// See the following for generating UUIDs: https://www.uuidgenerator.net/
#define SERVICE_UUID_CONNECT  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SERVICE_UUID_WEIGH    "1b5fbbda-7e47-40e7-8ac0-9512595ab3fa"

#define CHARACTERISTIC_UUID_CONNECT  "00000001-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_CALIBRATE "00000001-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_TARE  "00000002-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_READ  "00000003-7e47-40e7-8ac0-9512595ab3fa"

//--------------------GLOBAL VARIABLES-----------------
// Loadcell:
const int HX711_dout = 32; // D32 mcu > HX711 dout pin 
const int HX711_sck = 33; // D33 mcu > HX711 sck pin
HX711_ADC LoadCell(HX711_dout, HX711_sck); // instantiate an HX711 object
int num_readings = 0; // scale readings counter

// EEPROM address
const int calVal_eepromAdress = 0;

// BLE:
BLEServer* perchServer = NULL;

BLEService *connectService = NULL;
BLEService *weighService = NULL;

BLECharacteristic* connectCharacteristic = NULL;
BLECharacteristic* calibrateCharacteristic = NULL;
BLECharacteristic* tareCharacteristic = NULL;
BLECharacteristic* readCharacteristic = NULL;

bool bleDeviceConnected = false;
uint32_t connectTest = 0;
bool calibrateCommand = false;
bool tareCommand = false;
bool readCommand = false;
//------------------------------------------------------
//------------------HELPER FUNCTIONS------------------------
// LOADCELL:
void calibrate() {
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      if (Serial.available() > 0) {
        char inByte = Serial.read();
        if (inByte == 't') LoadCell.tareNoDelay();
      }
    }
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
      _resume = true;
    }
  }

  Serial.println("Now, place your known mass on the loadcell.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }

  LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass); //get the new calibration value

  Serial.print("New calibration value has been set to: ");
  Serial.print(newCalibrationValue);
  Serial.println(", use this as calibration value (calFactor) in your project sketch.");
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
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
  float oldCalibrationValue = LoadCell.getCalFactor();
  boolean _resume = false;
  Serial.println("***");
  Serial.print("Current value is: ");
  Serial.println(oldCalibrationValue);
  Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
  float newCalibrationValue;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(newCalibrationValue);
        LoadCell.setCalFactor(newCalibrationValue);
        _resume = true;
      }
    }
  }
  _resume = false;
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
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

void read() {
  static boolean newDataReady = 0;

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  if (newDataReady) {
      float reading = LoadCell.getData();
      if(reading<1 & eigthseconds%80 == 0){ //Condition to set new tare every 1000 milliseconds
        boolean _resume = false;
        boolean _tarewait = true;
        while (_resume == false) {
          LoadCell.update();
          if(_tarewait){              
            LoadCell.tareNoDelay();
            _tarewait = false;
          }
          if (LoadCell.getTareStatus() == true) {
            Serial.println("Next Tare Complete");
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
}
// BLE:
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      bleDeviceConnected = true;
      Serial.println("Device Connected.");
    };

    void onDisconnect(BLEServer* pServer) {
      bleDeviceConnected = false;
      Serial.println("Device Disconnected.");
      delay(500); // give the bluetooth stack the chance to get things ready
      BLEDevice::startAdvertising();
      Serial.println("Started re-advertising.");
    }
};

class CalibrateCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pcharacteristic) {
    std::string rxValue = pcharacteristic->getValue(); 

    if (rxValue.length() > 0) {
      // Serial.println("BLE calibrate characteristic requested.");
      calibrateCommand=true;
    }
  }
};

class TareCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pcharacteristic) {
    std::string rxValue = pcharacteristic->getValue(); 

    if (rxValue.length() > 0) {
      Serial.println("BLE tare characteristic requested.");
      tareCommand = true;
    }
  }
  // void onStatus(BLECharacteristic *pcharacteristic) {
  //   Serial.print("Indicated");
  //    tareCommand = true;
  // }
};

class ReadCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pcharacteristic) {
    std::string rxValue = pcharacteristic->getValue(); 

    if (rxValue.length() > 0) {
      Serial.println("BLE read characteristic requested.");
      if(rxValue.find("0")!=-1){
        readCommand = false;
      }else{
        readCommand = true;
      } 
    }
  }
};

void setup() {
  ///--------------------SERIAL SETUP-----------------------
  Serial.begin(9600); // Initialise baud rate with PC
  
  //--------------------LOADCELL SETUP-----------------------
  LoadCell.begin();
  // LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);

  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  // else {
  //   LoadCell.setCalFactor(1.0); // user set calibration value (float), initial value 1.0 may be used for this sketch
  //   Serial.println("Startup is complete");
  // }
  Serial.println("Loadcell startup is complete");
  while (!LoadCell.update());
  
  //--------------------BLE SETUP-----------------------
  // Create the BLE Device
  BLEDevice::init("PerchScale");

  // Create the BLE Server
  perchServer = BLEDevice::createServer();
  perchServer->setCallbacks(new ServerCallbacks());

  //************* CONNECT SERVICE ************************
  // Create the connect BLE Service
  connectService = perchServer->createService(SERVICE_UUID_CONNECT);
  // Create a BLE Characteristic
  connectCharacteristic = connectService->createCharacteristic(
                      CHARACTERISTIC_UUID_CONNECT ,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  // connectCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  connectService->start();
  //******************************************************
  //************* WEIGH SERVICE ************************
  // Create the connect BLE Service
  weighService = perchServer->createService(SERVICE_UUID_WEIGH);
  // Create a calibrate BLE Characteristic
  calibrateCharacteristic = weighService->createCharacteristic(
                      CHARACTERISTIC_UUID_CALIBRATE ,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  //calibrateCharacteristic->addDescriptor(new BLE2902());
  calibrateCharacteristic->setCallbacks(new CalibrateCallbacks());
  
  // Create a tare BLE Characteristic
  tareCharacteristic = weighService->createCharacteristic(
                      CHARACTERISTIC_UUID_TARE ,
                      // BLECharacteristic::PROPERTY_INDICATE
                      BLECharacteristic::PROPERTY_WRITE
                    );
  //tareCharacteristic->addDescriptor(new BLE2902());
  tareCharacteristic->setCallbacks(new TareCallbacks());

  // Create a read BLE Characteristic
  readCharacteristic = weighService->createCharacteristic(
                      CHARACTERISTIC_UUID_READ,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  // readCharacteristic->addDescriptor(new BLE2902());
  readCharacteristic->setCallbacks(new ReadCallbacks());

  // Start the service
  weighService->start();
  //******************************************************

  // Start advertising
  BLEAdvertising *perchAdvertising = BLEDevice::getAdvertising();
  perchAdvertising->addServiceUUID(SERVICE_UUID_CONNECT);
  perchAdvertising->addServiceUUID(SERVICE_UUID_WEIGH);
  perchAdvertising->setScanResponse(false);
  perchAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting for client to connect...");

}

void loop() {

  if(calibrateCommand){
    calibrate(); //start calibration procedure
    calibrateCommand = false;
  } 
  if(tareCommand){
    LoadCell.tareNoDelay(); // tare 
    Serial.println("Tare complete.");
    tareCommand = false;
  }
  if(readCommand){
    read();
  }
}
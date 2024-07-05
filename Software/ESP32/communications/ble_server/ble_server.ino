/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <string.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SERVICE_UUID_WEIGH         "1b5fbbda-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_READ  "00000003-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_TARE  "00000004-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_CALIBRATE  "00000005-7e47-40e7-8ac0-9512595ab3fa"

bool deviceConnected = false;
int value = 0;
static String rstMSG = "rst";
static String okMSG = "ok";
static String nokMSG = "nok";
BLECharacteristic *readCharacteristic;
BLECharacteristic *tareCharacteristic;
BLECharacteristic *calibrateCharacteristic;

class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device Connected.");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device Disconnected.");
      delay(500); // give the bluetooth stack the chance to get things ready
      BLEDevice::startAdvertising();
      Serial.println("Started re-advertising.");
    }
};

void setup() {
  Serial.begin(9600);
  Serial.println("Starting BLE work!");

  BLEDevice::init("PerchScale");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  //----------CONNECT SERVICE------------------------
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic =
    pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  // -----------------------------------------------
  //----------WEIGH SERVICE------------------------
  BLEService *weighService = pServer->createService(SERVICE_UUID_WEIGH);
  
  readCharacteristic =
    weighService->createCharacteristic(CHARACTERISTIC_UUID_READ, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  weighService->addCharacteristic(readCharacteristic);
  readCharacteristic->setValue("read");
  
  tareCharacteristic =
    weighService->createCharacteristic(CHARACTERISTIC_UUID_TARE, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  weighService->addCharacteristic(tareCharacteristic); 
  tareCharacteristic->setValue("tare"); 

  calibrateCharacteristic =
    weighService->createCharacteristic(CHARACTERISTIC_UUID_CALIBRATE, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  weighService->addCharacteristic(calibrateCharacteristic); 
  calibrateCharacteristic->setValue("calibrate"); 

  weighService->start();
  // -----------------------------------------------  
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->addServiceUUID(SERVICE_UUID_WEIGH);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  readCharacteristic->setValue(String(value).c_str()); // Updating value (placeholder)
  String tareVal = tareCharacteristic->getValue().c_str(); 
  Serial.print("Tare:");
  Serial.println(tareVal);
  String calibrateVal = calibrateCharacteristic->getValue().c_str();
  Serial.print("Calibrate:");
  Serial.println(calibrateVal);

  if (tareVal == rstMSG) {
    Serial.println("Resetting tare value.");
    tareCharacteristic->setValue("tare");
  }
  if (calibrateVal == rstMSG) {
    Serial.println("Resetting calibrate value.");
    calibrateCharacteristic->setValue("calibrate");
  }
  if (tareVal == "t") {
    Serial.println("Tare command received");
    tareCharacteristic->setValue(okMSG.c_str());
  }
  if (calibrateVal == "10") {
    Serial.println("Calibrate 10g command received");
    calibrateCharacteristic->setValue(okMSG.c_str());
    // If can't read from load cell, send fail
    // calibrateCharacteristic->setValue(nokMSG);
  }
  if (calibrateVal == "20") {
    Serial.println("Calibrate 20g command received");
    calibrateCharacteristic->setValue(okMSG.c_str());
    // If can't read from load cell, send fail
    // calibrateCharacteristic->setValue(nokMSG);    
  }  
  if (calibrateVal == "200") {
  Serial.println("Calibrate 200g command received");
  calibrateCharacteristic->setValue(okMSG.c_str());
    // If can't read from load cell, send fail
    // calibrateCharacteristic->setValue(nokMSG);  
  }

  else if (tareVal != okMSG && tareVal != rstMSG && tareVal != "t" && tareVal != "tare") {
    Serial.println("Unacceptable tare value received.");
    calibrateCharacteristic->setValue(nokMSG.c_str());
  }  

  else if (calibrateVal != okMSG && calibrateVal != rstMSG && calibrateVal != "10" && calibrateVal != "20" && calibrateVal != "200" && calibrateVal != "calibrate") {
    Serial.println("Unacceptable calibrate value received.");
    calibrateCharacteristic->setValue(nokMSG.c_str());
  }
  
    delay(1000);
  // if (deviceConnected) {
  //       connectCharacteristic->setValue((uint8_t*)&connectTest, 4);
  //       connectCharacteristic->notify();
  //       connectTest++;
  //       delay(10); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  // }
  value ++;
}

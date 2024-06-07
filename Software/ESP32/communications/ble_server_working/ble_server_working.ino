/*
* Create BLE server on an ESP32 with 2 services (each having different characteristics). These do not do anything, they just print values to serial
* You need to use a Bluetooth scanner on Mobile to test this or an ESP with the BLE_client script booted onto it to test functionality
* 
* Working: 07/06/2024
*
* Pin connections:
*   None
*
* Interesting BLE resources:
*   https://devzone.nordicsemi.com/guides/short-range-guides/b/bluetooth-low-energy/posts/ble-characteristics-a-beginners-tutorial
*   https://novelbits.io/bluetooth-gatt-services-characteristics/
*   BLE UUID SIG Standards: https://btprodspecificationrefs.blob.core.windows.net/assigned-numbers/Assigned%20Number%20Types/Assigned_Numbers.pdf
*/

//------------------INCLUDES------------------------
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
//--------------------------------------------------

//--------------------DEFINES-------------------------
// See the following for generating UUIDs: https://www.uuidgenerator.net/
#define SERVICE_UUID_CONNECT  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SERVICE_UUID_WEIGH    "1b5fbbda-7e47-40e7-8ac0-9512595ab3fa"
// #define SERVICE_UUID_WEIGH    "0000181D-0000-1000-8000-00805F9B34FB" // BLE-SIG Standard: Weight Scale Service
//#define SERVICE_UUID_BATTERY  "0000180F-0000-1000-8000-00805F9B34FB" // BLE-SIG Standard: Battery Service

#define CHARACTERISTIC_UUID_CONNECT  "00000001-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_CALIBRATE "00000001-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_TARE  "00000002-7e47-40e7-8ac0-9512595ab3fa"
#define CHARACTERISTIC_UUID_READ  "00000003-7e47-40e7-8ac0-9512595ab3fa"
//#define CHARACTERISTIC_UUID_BATTERYLEVEL  "00002A19-0000-1000-8000-00805F9B34FB" // BLE-SIG Standard: Battery Level Characteristic
/// #define CHARACTERISTIC_UUID_BATTERYLEVEL  "00002BED-0000-1000-8000-00805F9B34FB" BLE-SIG Standard: Battery Level Status Characteristic
//----------------------------------------------------

//--------------------GLOBAL VARIABLES-----------------
BLEServer* perchServer = NULL;

BLEService *connectService = NULL;
BLEService *weighService = NULL;
BLEService *batteryService = NULL;

BLECharacteristic* connectCharacteristic = NULL;
BLECharacteristic* calibrateCharacteristic = NULL;
BLECharacteristic* tareCharacteristic = NULL;
BLECharacteristic* readCharacteristic = NULL;
BLECharacteristic* batterylevelCharacteristic = NULL;


bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t connectTest = 0;
//-----------------------------------------------------

//--------------------HELPER FUNCTIONS-----------------
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

class CalibrateCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pcharacteristic) {
    std::string rxValue = pcharacteristic->getValue(); 

    if (rxValue.length() > 0) {
      Serial.println("Calibrate");
    }
  }
};

class TareCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pcharacteristic) {
    std::string rxValue = pcharacteristic->getValue(); 

    if (rxValue.length() > 0) {
      Serial.println("Tare");
    }
  }
};

// void setupCharacteristic(BLEService *pService, BLECharacteristic *pCharacteristic, const char *characteristicUUID,  ){
//   // Create a BLE Characteristic
//   pCharacteristic = pService->createCharacteristic(
//                       characteristicUUID,
//                       BLECharacteristic::PROPERTY_READ   |
//                       BLECharacteristic::PROPERTY_WRITE  |
//                       BLECharacteristic::PROPERTY_NOTIFY |
//                       BLECharacteristic::PROPERTY_INDICATE
//                     );
//   // Create a BLE Descriptor
//   pCharacteristic->addDescriptor(new BLE2902());
//   // Start the service
//   pCharacteristic->start();
// }
//-----------------------------------------------------

void setup() {
  //--------------------SERIAL SETUP-----------------------
  Serial.begin(9600);

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
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  //tareCharacteristic->addDescriptor(new BLE2902());
  tareCharacteristic->setCallbacks(new TareCallbacks());

  // Create a read BLE Characteristic
  readCharacteristic = weighService->createCharacteristic(
                      CHARACTERISTIC_UUID_READ,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  readCharacteristic->addDescriptor(new BLE2902());

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
  //--------------------Z SETUP-----------------------

}

void loop() {
  // notify changed value
    if (deviceConnected) {
        connectCharacteristic->setValue((uint8_t*)&connectTest, 4);
        connectCharacteristic->notify();
        connectTest++;
        delay(10); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }

}

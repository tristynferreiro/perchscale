/*
* This project is used as a test that the ESP32Dev receives data over UART
* When testing, this code was run on the ESP32Dev (because it has multiple UARTs ) but in final system the 
* ESP32Cam is the receiver
*/

#include <HardwareSerial.h>

HardwareSerial SerialPort(2); // initialise UART2 

void setup() {
  Serial.begin(9600); // Communicates with PC

  SerialPort.begin(9600,SERIAL_8N1, 16, 17); // Communicates with ESP32Cam
  pinMode(2, OUTPUT); // to allow onboard LED to be toggled
}

void loop() {
  
  if (SerialPort.available()) 
  {
    String data = SerialPort.readString();
    digitalWrite(2, HIGH); // toggle LED to show data being recieved
    Serial.println(data); // Print ESP32 cam data to PC
    
  }else{
    Serial.println("nothing"); // If ESP32Cam not seending data/ UART2 not being used
  }
  delay(1500);
  digitalWrite(2, LOW); // toggle LED off
  
}

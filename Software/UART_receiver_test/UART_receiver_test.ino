/*
* This project is used as a test to send data to the receving microcontroller
* When testing, this code was run on the ESP32Dev, however in the final system it will be the sender
*/

#include <HardwareSerial.h>

HardwareSerial SerialPort(2); // use UART2 

void setup() {
  Serial.begin(9600); //Communicates with ESP32Cam

  SerialPort.begin(115200,SERIAL_8N1, 16, 17); // Communicates to PC what it is getting from Cam
  pinMode(2, OUTPUT); // to allow onboard LED to be toggled
}

void loop() {
  String data = "failed";
  if (Serial.available()) 
  {
    data = Serial.readString();
    digitalWrite(2, HIGH); // toggle LED to show data being recieved
  }
  SerialPort.print(data);
}

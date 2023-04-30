/*
* This project is used as a test to send data to the receving microcontroller
* When testing, this code was run on the ESP32Dev, however in the final system it will be the sender
*/

#include <HardwareSerial.h>

HardwareSerial SerialPort(2); // use UART2 

void setup() {
  Serial.begin(9600); //Communicates with PC

  SerialPort.begin(9600,SERIAL_8N1, 16, 17); //Communicates with ESP32Cam
  pinMode(2, OUTPUT); // to allow onboard LED to be toggled
}

void loop() {
  
  if (SerialPort.available()) 
  {
    String data = SerialPort.readString();
    digitalWrite(2, HIGH); // toggle LED to show data being recieved
    Serial.println(data);
    
  }else{
    Serial.println("nothing");
  }
  delay(1500);
  digitalWrite(2, LOW); // toggle LED off
  
}

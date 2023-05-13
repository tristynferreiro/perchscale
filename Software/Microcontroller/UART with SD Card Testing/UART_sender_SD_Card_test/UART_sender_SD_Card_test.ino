/*
* This project is used as a test to send data to the receving microcontroller (ESP32Cam)
*/

#include <HardwareSerial.h> // to setup UART for comm with ESP32Cam
HardwareSerial SerialPort(2);// initialize UART2

void setup() {
  SerialPort.begin(9600);
  //SerialPort.begin(9600, SERIAL_8N1, 16, 17);  // Initialise baud rate with ESP32Cam
  pinMode(2,OUTPUT); // onboard LED used to show data transfer to cam
}

void loop() {
  digitalWrite(2, HIGH);
  SerialPort.print("\n Hello Ben");
  delay(1000);
  digitalWrite(2, LOW);
  delay(1000);
  SerialPort.print("\n Ben?");
  delay(1000);
}

/*
* This project is used as a test to send data to the receving microcontroller
* When testing, this code was run on the ESP32Cam, however in the final system the CAM will be the recevier
*/
void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("\n Hello Ben");
  delay(2000);
  Serial.print("\n Ben?");
  delay(2000);
}

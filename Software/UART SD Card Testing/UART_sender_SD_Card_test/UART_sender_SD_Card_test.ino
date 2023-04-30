/*
* This project is used as a test to send data to the receving microcontroller (ESP32Cam)
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

/*
* This is a basic Hello World (with a twist) program. It is used to ensure that you have correctly
* connected the pins from the FTDI to the ESP32.
*/

void setup() {
  Serial.begin(115200);

}

void loop() {
  Serial.println("Hello Ben");

}

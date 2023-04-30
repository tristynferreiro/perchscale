/*
* This the the Scale UART sender project. It reads the scale data and transmits it over UART
* First perch-scale submodule (scale and data/microcontroller) integration test 
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

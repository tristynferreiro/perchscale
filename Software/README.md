# Steps for running the code
## Running Arduino Test Code
- Start with the Test_Code.
- Download Arduino IDE.
- Follow the steps on "Installing the Esp32 Library" in this [tutorial](https://all3dp.com/2/esp32-cam-arduino-tutorial/)
- Connect the pins as follows:
    FTDI    -> CAM
    5V      -> 5V
    GND     -> GND
    TXD     -> V0R
    RXD     -> V0T
  You will need another connector on the ID0 pin. When you want to **flash code** onto the ESP32 Cam you need to **connect ID0 to GND** on the ESP32.
- open the [Test_Code]() project in the IDE and upload it to the ESP32. Then disconnect the ID0 -> GND cable
- open the serial monitor and click the reset button on the ESP32.

IMPORTANT!!! 
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode


    
    
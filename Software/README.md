# Steps for running the code
## Running Arduino Code
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
    
    
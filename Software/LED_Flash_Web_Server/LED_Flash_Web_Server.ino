/*********
  This program sets up an ESP32 access point and starts a basic webserver that allows you to press a button to toggle the
  onboard LED on/off.
  ***************************************  
  Original Project:
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/


// Load Wi-Fi library
#include <WiFi.h>
#include <WebServer.h>
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems

// Replace with your network credentialss
const char* ssid           = "Hornbill Net";
const char* password       = NULL;                    //set to null so it is an open network
// const int   channel        = 10;                   // WiFi Channel number between 1 and 13
// const bool  hide_SSID      = false;                // To disable SSID broadcast -> SSID will not appear in a basic WiFi scan
// const int   max_connection = 2;                    // Maximum simultaneous connected clients on the AP 

// Set web server port number to 80
WiFiServer server(80);
String header; // Variable to store the HTTP request
String output4State = "off"; // Auxiliar variables to store the current output state of LED on Cam board
const int output4 = 4; // Assign output variables to GPIO pins

// Variables for connection testing
unsigned long currentTime = millis(); // Current time
unsigned long previousTime = 0;  // Previous time
const long timeoutTime = 2000; // Define timeout time in milliseconds (example: 2000ms = 2s)

void setup() {
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Turn-off the 'brownout detector'

  Serial.begin(9600); // Set baud rate;
  // Initialize the output variables as output
  pinMode(output4, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output4, LOW);

  // Create Wi-Fi network with SSID and password
  Serial.println("\n Hold onto your feathers..."); // Creating AP message
  WiFi.mode(WIFI_AP);
  Serial.println("\n Done");
  WiFi.softAP(ssid, password);
  Serial.println("\n Done again");
  
  Serial.print("\n AP Created with IP Gateway "); // Print local IP address 
  Serial.print(WiFi.softAPIP());
  //start web server
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output4State=="off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
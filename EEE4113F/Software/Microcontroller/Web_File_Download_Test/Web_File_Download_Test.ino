/*  Version 1
 *  
 *  ESP32/ESP8266 example of downloading a file from the device's SD Filing System
 *  
 This software, the ideas and concepts is Copyright (c) David Bird 2018. All rights to this software are reserved.
 
 Any redistribution or reproduction of any part or all of the contents in any form is prohibited other than the following:
 1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
 2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
 3. You may not, except with my express written permission, distribute or commercially exploit the content.
 4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.

 The above copyright ('as annotated') notice and this permission notice shall be included in all copies or substantial portions of the Software and where the
 software use is visible to an end-user.
 
 THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY 
 OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 See more at http://www.dsbird.org.uk
 *
*/

#include <WiFi.h>              
#include <WebServer.h>   
#include <ESPmDNS.h>
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <SPI.h>

#include "Network.h"
#include "Sys_Variables.h"
#include "CSS.h"

// Set network credentials
const char* ssid           = "Hornbill Net";
const char* password       = NULL;                    //set to null so it is an open network

WebServer server(80); // Set Webserver host port

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup(void){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Turn-off the 'brownout detector'
  
  Serial.begin(9600); // Set the baud rate

  // ------------------------ SD Card Initialisation --------------------------------
  Serial.print(F("Initializing SD card...")); 
  if(!SD_MMC.begin()){
    Serial.println("\n SD Card Mount Failed, please insert SD Card");
    return;
  }else{
    Serial.println("\n SD Card Mounted Successfully");
    SD_present = true; 
  }

  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("\n No SD Card attached");
    return;
  }
  
  // ------------------------ Wifi AP setup --------------------------------------
  // Create Wi-Fi network with SSID and password
  Serial.println("\n Hold onto your feathers..."); // Creating AP message
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  Serial.print("\n AP Created with IP Gateway "); // Print local IP address 
  Serial.print(WiFi.softAPIP());

  //-------------------------- Web Server Commands -------------------------------    
  server.on("/",         HomePage);
  server.on("/download", File_Download);
  
  server.begin(); //start web server
  Serial.println("\n Web server started");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop(void){
  server.handleClient(); // Listen for client connections
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Supporting Functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HomePage(){
  SendHTML_Header();
  webpage += F("<a href='/download'><button>Download</button></a>");
  SendHTML_Content();
  SendHTML_Stop(); // Stop is needed because no content length was sent
}
//------------------------------------------------------------------------------------
void File_Download(){ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("download")) SD_file_download(server.arg(0));
  }
  else SelectInput("File Download","Enter filename to download","download","download");
}
//------------------------------------------------------------------------------------
void SD_file_download(String filename){
  if (SD_present) { 
    File download = SD_MMC.open("/"+filename);
    if (download) {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    } else ReportFileNotPresent("download"); 
  } else ReportSDNotPresent();
}
//------------------------------------------------------------------------------------
void SendHTML_Header(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves. 
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}
//------------------------------------------------------------------------------------
void SendHTML_Content(){
  server.sendContent(webpage);
  webpage = "";
}
//------------------------------------------------------------------------------------
void SendHTML_Stop(){
  server.sendContent("");
  server.client().stop(); // Stop is needed because no content length was sent
}
//------------------------------------------------------------------------------------
void SelectInput(String heading1, String heading2, String command, String arg_calling_name){
  SendHTML_Header();
  webpage += F("<h3 class='rcorners_m'>");webpage += heading1+"</h3><br>";
  webpage += F("<h3>"); webpage += heading2 + "</h3>"; 
  webpage += F("<FORM action='/"); webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
  webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br><br>");
  SendHTML_Content();
  SendHTML_Stop();
}
//------------------------------------------------------------------------------------
void ReportSDNotPresent(){
  SendHTML_Header();
  webpage += F("<h3>No SD Card present</h3>"); 
  webpage += F("<a href='/'>[Back]</a><br><br>");
  SendHTML_Content();
  SendHTML_Stop();
}
//------------------------------------------------------------------------------------
void ReportFileNotPresent(String target){
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  SendHTML_Content();
  SendHTML_Stop();
}
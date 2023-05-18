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
const char* ssid = "Hornbill Net";
const char* password = NULL;                    //set to null so it is an open network

WebServer server(80); // Set Webserver host port

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup(void) {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Turn-off the 'brownout detector'

  Serial.begin(9600); // Set the baud rate

  // ------------------------ SD Card Initialization --------------------------------
  Serial.print(F("Initializing SD card..."));
  if (!SD_MMC.begin()) {
    Serial.println("\n SD Card Mount Failed, please insert SD Card");
    return;
  }
  else {
    Serial.println("\n SD Card Mounted Successfully");
    SD_present = true;
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
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
  server.on("/", HomePage);
  server.on("/download", File_Download);

  server.begin(); //start web server
  Serial.println("\n Web server started");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop(void) {
  server.handleClient(); // Listen for client connections
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Supporting Functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HomePage() {
  SendHTML_Header();
  webpage += F("<a href='/download'><button>Download</button></a>");
  
  if (SD_present) {
    webpage += F("<h3>File List:</h3><br>");
    webpage += GetFileList();
  } else {
    webpage += F("<h3>No SD Card present</h3>");
  }
  
  SendHTML_Content();
  SendHTML_Stop(); // Stop is needed because no content length was sent
}

String GetFileList() {
  String fileNames;

  File root = SD_MMC.open("/");
  File file = root.openNextFile();

  while (file) {
    fileNames += file.name();
    fileNames += "<br>";
    file = root.openNextFile();
  }

  root.close();
  
  return fileNames;
}

//------------------------------------------------------------------------------------
void File_Download() { // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  if (server.args() > 0) { // Arguments were received
    if (server.hasArg("download")) SD_file_download(server.arg(0));
  }
  else SelectInput("File Download", "Enter filename to download", "download", "download");
}

//------------------------------------------------------------------------------------
void SD_file_download(String filename) {
  if (SD_present) {
    File download = SD_MMC.open("/" + filename);
    if (download) {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename=" + filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    }
    else {
      ReportFileNotPresent("download");
    }
  }
  else {
    ReportSDNotPresent();
  }
}
//------------------------------------------------------------------------------------
void SendHTML_Header() {
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
void SendHTML_Content() {
  server.sendContent(webpage);
  webpage = "";
}
//------------------------------------------------------------------------------------
void SendHTML_Stop() {
  server.sendContent("");
  server.client().stop(); // Stop is needed because no content length was sent
}
//------------------------------------------------------------------------------------
void SelectInput(String heading1, String heading2, String command, String arg_calling_name) {
  SendHTML_Header();
  webpage += F("<h3 class='rcorners_m'>"); webpage += heading1 + "</h3><br>";
  webpage += F("<h3>"); webpage += heading2 + "</h3>";
  webpage += F("<FORM action='/"); webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
  webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br><br>");
  SendHTML_Content();
  SendHTML_Stop();
}
//------------------------------------------------------------------------------------
void ReportSDNotPresent() {
  SendHTML_Header();
  webpage += F("<h3>No SD Card present</h3>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  SendHTML_Content();
  SendHTML_Stop();
}
//------------------------------------------------------------------------------------
void ReportFileNotPresent(String target) {
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>");
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  SendHTML_Content();
  SendHTML_Stop();
}

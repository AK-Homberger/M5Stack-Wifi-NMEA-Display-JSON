/*
  Demo: NMEA -> M5Stack display
  Reads JSON Data from NMEA 200 Wifi Gatway and displays is on the M5Stack module
  Data will be stored in BoatData struct.

  Version 0.2 / 17.10.2017
*/


#include <Arduino.h>
#include <M5Stack.h>
#include <Time.h>
#include <sys/time.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include "BoatData.h"


WiFiMulti WiFiMulti;
int port = 90; // JSON Server port of NMEA 2000 Gateway
const char * host = "192.168.15.1"; // Server IP

WiFiClient client;  // Create TCP client

tBoatData BoatData;  // Struct to store Boat Data (from BoatData.h)

// Task handle for JSON Client (Core 0 on ESP32)
TaskHandle_t Task1;

double t = 0;
int page = 0;  // Initial page to show
int pages = 5; // Number of pages -1
int LCD_Brightness = 250;

long MyTime = 0;  // System Time from NMEA2000
bool TimeSet = false;

double FridgeTemperature = 0;
double BatteryVoltage = 0;


void setup() {
  M5.begin();
  M5.Power.begin();
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Lcd.setCursor(0, 0, 1);

  Serial.begin(115200); delay(500);

  M5.Lcd.setTextSize(2);
  WiFiMulti.addAP("MyESP32", "password");

  M5.Lcd.print("Waiting for WiFi. ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    M5.Lcd.print(".");
    delay(500);
  }

  M5.Lcd.println("");
  M5.Lcd.println("WiFi connected");
  M5.Lcd.println("IP address: ");
  M5.Lcd.println(WiFi.localIP());

  Display_Main();

  // Create task for core 0, loop() runs on core 1
  xTaskCreatePinnedToCore(
    Get_JSON_Data, /* Function to implement the task */
    "Task1", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    2,  /* Priority of the task */
    &Task1,  /* Task handle. */
    0); /* Core where the task should run */
}


void Get_JSON_Data(void * parameter) {

  // Allocate JsonBuffer
  // Use arduinojson.org/assistant to compute the capacity.

  DynamicJsonBuffer jsonBuffer(800);

  WiFiClient client;
  client.setTimeout(1000);

  while (true) {

    // Serial.println(F("Connecting..."));

    // Connect to HTTP server

    if (!client.connect("192.168.15.1", 90)) {
      Serial.println(F("Connection failed"));
      return;
    }

    // Serial.println(F("Connected!"));

    // Send HTTP request
    client.println(F("GET /example.json HTTP/1.0"));
    client.println(F("Host: arduinojson.org"));
    client.println(F("Connection: close"));
    if (client.println() == 0) {
      Serial.println(F("Failed to send request"));
      return;
    }

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.0 200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      return;
    }


    // Parse JSON object
    JsonObject& root = jsonBuffer.parseObject(client);
    if (!root.success()) {
      Serial.println(F("Parsing failed!"));
      return;
    }

    // Extract values

    BoatData.Latitude = root["Latitude"] ;
    BoatData.Longitude = root["Longitude"];
    BoatData.Heading = root["Heading"];
    BoatData.COG = root["COG"];
    BoatData.SOG = root["SOG"];
    BoatData.STW = root["STW"];
    BoatData.AWS = root["AWS"];
    BoatData.TWS = root["TWS"];
    BoatData.MaxAws = root["MaxAws"];
    BoatData.MaxTws = root["MaxTws"];
    BoatData.AWA = root["AWA"];
    BoatData.TWA = root["TWA"];
    BoatData.TWD = root["TWD"];
    BoatData.TripLog = root["TripLog"];
    BoatData.Log = root["Log"];
    BoatData.RudderPosition = root["RudderPosition"];
    BoatData.WaterTemperature = root["WaterTemperature"];
    BoatData.WaterDepth = root["WaterDepth"];
    BoatData.Variation = root["Variation"];
    BoatData.Altitude = root["Altitude"];
    BoatData.GPSTime = root["GPSTime"];
    BoatData.DaysSince1970 = root["DaysSince1970"];
    FridgeTemperature = root["FridgeTeperature"];
    BatteryVoltage = root["BatteryVoltage"];

    // Disconnect
    client.stop();
    jsonBuffer.clear();
    delay(1000);
  }
}


void set_system_time(void) {
  MyTime = (BoatData.DaysSince1970 * 3600 * 24) + BoatData.GPSTime;

  if (MyTime != 0 && !TimeSet) { // Set system time from valid NMEA2000 time (only once)
    struct timeval tv;
    tv.tv_sec = MyTime;
    settimeofday(&tv, NULL);
    TimeSet = true;
  }
}



void loop() {
  M5.update();

  if (millis() > t + 1000) {
    t = millis();

    if (page == 0) Page_1();
    if (page == 1) Page_2();
    if (page == 2) Page_3();
    if (page == 3) Page_4();
    if (page == 4) Page_5();
    if (page == 5) Page_6();

    set_system_time();
    DiplayDateTime();
  }


  if (M5.BtnB.wasPressed() == true) {
    page++;                        // Button B pressed -> Next page
    if (page > pages) page = 0;
    t -= 1000;
  }


  if (M5.BtnA.wasPressed() == true) {
    page--;
    if (page < 0) page = pages;
    t -= 1000;
  }



  if (M5.BtnC.wasPressed() == true)                         /* Button C pressed ? --> Change brightness */
  {
    //      M5.Speaker.tone(NOTE_DH2, 1);
    if (LCD_Brightness < 250)                               /* Maximum brightness not reached ? */
    {
      LCD_Brightness = LCD_Brightness + 10;                 /* Increase brightness */
    }
    else                                                    /* Maximum brightness reached ? */
    {
      LCD_Brightness = 10;                                  /* Set brightness to lowest value */
    }
    M5.Lcd.setBrightness(LCD_Brightness);                   /* Change brightness value */
  }
}


void Page_6(void) {
  char buffer[40];

  M5.Lcd.fillRect(0, 31, 320, 178, 0x439);
  M5.Lcd.setCursor(0, 45, 2);
  M5.Lcd.setTextSize(3);
  sprintf(buffer, " Batt %2.1f V", BatteryVoltage);
  M5.Lcd.print(buffer);
  M5.Lcd.println("  ");
  sprintf(buffer, " Fridge %2.1f", FridgeTemperature);
  M5.Lcd.print(buffer);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print(" o ");
  M5.Lcd.setTextSize(3);
  M5.Lcd.println("C ");
  sprintf(buffer, " ALT %2.1f m", BoatData.Altitude);
  M5.Lcd.println(buffer);
}


void Page_5(void) {
  char buffer[40];

  M5.Lcd.fillRect(0, 31, 320, 178, 0x439);
  M5.Lcd.setCursor(0, 45, 2);
  M5.Lcd.setTextSize(3);
  sprintf(buffer, " Water %3.0f", BoatData.WaterTemperature);
  M5.Lcd.print(buffer);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print(" o ");
  M5.Lcd.setTextSize(3);
  M5.Lcd.println("C  ");
  sprintf(buffer, " Trip  %4.1f nm", BoatData.TripLog);
  M5.Lcd.print(buffer);
  M5.Lcd.println("  ");
  sprintf(buffer, " Log %5.0f nm", BoatData.Log);
  M5.Lcd.println(buffer);
}



void Page_4(void) {
  char buffer[40];

  M5.Lcd.fillRect(0, 31, 320, 178, 0x439);
  M5.Lcd.setCursor(0, 45, 2);
  M5.Lcd.setTextSize(3);
  sprintf(buffer, " STW   %3.1f kn", BoatData.STW);
  M5.Lcd.print(buffer);
  M5.Lcd.println("  ");
  sprintf(buffer, " Depth %4.1f m", BoatData.WaterDepth);
  M5.Lcd.print(buffer);
  M5.Lcd.println("  ");
  sprintf(buffer, " Rudder %2.1f", BoatData.RudderPosition);
  M5.Lcd.print(buffer);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print(" o ");
  M5.Lcd.setTextSize(3);
}



void Page_3(void) {
  char buffer[40];

  M5.Lcd.fillRect(0, 31, 320, 178, 0x439);
  M5.Lcd.setCursor(0, 45, 2);
  M5.Lcd.setTextSize(3);
  sprintf(buffer, " HDG %03.0f", BoatData.Heading);
  M5.Lcd.print(buffer);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print(" o ");
  M5.Lcd.setTextSize(3);
  M5.Lcd.println("  ");
  sprintf(buffer, " COG %03.0f", BoatData.COG);
  M5.Lcd.print(buffer);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print(" o ");
  M5.Lcd.setTextSize(3);
  M5.Lcd.println("  ");
  sprintf(buffer, " SOG %2.1f kn", BoatData.SOG);
  M5.Lcd.println(buffer);
}

void Page_2(void) {
  char buffer[40];
  double minutes;

  M5.Lcd.fillRect(0, 31, 320, 178, 0x439);
  M5.Lcd.setCursor(0, 40, 2);
  M5.Lcd.setTextSize(2);

  minutes = BoatData.Latitude - trunc(BoatData.Latitude);
  minutes = minutes / 100 * 60;

  M5.Lcd.println("LAT");
  M5.Lcd.setTextSize(3);
  sprintf(buffer, "   %02.0f", trunc(BoatData.Latitude));
  M5.Lcd.print(buffer);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print("o ");
  M5.Lcd.setTextSize(3);
  sprintf(buffer, "%06.3f", minutes * 100);
  M5.Lcd.print(buffer);
  if (BoatData.Latitude > 0 ) M5.Lcd.println("'N"); else M5.Lcd.println("'S");

  minutes = BoatData.Longitude - trunc(BoatData.Longitude);
  minutes = minutes / 100 * 60;

  M5.Lcd.setTextSize(2);
  M5.Lcd.println("LON");
  M5.Lcd.setTextSize(3);
  sprintf(buffer, "  %03.0f", trunc(BoatData.Longitude));
  M5.Lcd.print(buffer);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print("o ");
  M5.Lcd.setTextSize(3);
  sprintf(buffer, "%06.3f", minutes * 100);
  M5.Lcd.print(buffer);
  if (BoatData.Longitude > 0 ) M5.Lcd.println("'E"); else M5.Lcd.println("'W");
}

void Page_1(void) {
  char buffer[40];

  double AWS = 0;

  if (BoatData.AWS > 0) AWS = BoatData.AWS;

  M5.Lcd.fillRect(0, 31, 320, 178, 0x439);
  M5.Lcd.setCursor(0, 50, 2);
  M5.Lcd.setTextSize(4);
  sprintf(buffer, " AWS %3.0f kn", AWS);
  M5.Lcd.println(buffer);
  sprintf(buffer, " MAX %3.0f kn", BoatData.MaxAws);
  M5.Lcd.println(buffer);
}

void DiplayDateTime(void) {
  char buffer[50];

  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextSize(2);                                      /* Set text size to 2 */
  M5.Lcd.setTextColor(WHITE);                                 /* Set text color to white */

  M5.Lcd.fillRect(265, 0, 320, 30, 0x1E9F);

  M5.Lcd.setCursor(265, 7);
  sprintf(buffer, "%3.0d", M5.Power.getBatteryLevel());
  M5.Lcd.print(buffer);
  M5.Lcd.print("%");


  M5.Lcd.fillRect(0, 210, 320, 30, 0x1E9F);
  M5.Lcd.setCursor(10, 218);

  if (MyTime != 0) {

    time_t rawtime = MyTime; // Create time from NMEA 2000 time (UTC)
    struct tm  ts;
    ts = *localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d.%m.%Y - %H:%M:%S UTC", &ts); // Create directory name from date
    M5.Lcd.print(buffer);
  }

}



void Display_Main (void)
{
  M5.Lcd.setTextFont(1);
  M5.Lcd.fillRect(0, 0, 320, 30, 0x1E9F);                      /* Upper dark blue area */
  M5.Lcd.fillRect(0, 31, 320, 178, 0x439);                   /* Main light blue area */
  M5.Lcd.fillRect(0, 210, 320, 30, 0x1E9F);                    /* Lower dark blue area */
  M5.Lcd.fillRect(0, 30, 320, 4, 0xffff);                     /* Upper white line */
  M5.Lcd.fillRect(0, 208, 320, 4, 0xffff);                    /* Lower white line */
  //  M5.Lcd.fillRect(0, 92, 320, 4, 0xffff);                     /* First vertical white line */
  //  M5.Lcd.fillRect(0, 155, 320, 4, 0xffff);                    /* Second vertical white line */
  //  M5.Lcd.fillRect(158, 30, 4, 178, 0xffff);                   /* First horizontal white line */
  M5.Lcd.setTextSize(2);                                      /* Set text size to 2 */
  M5.Lcd.setTextColor(WHITE);                                 /* Set text color to white */
  M5.Lcd.setCursor(10, 7);                                    /* Display header info */
  M5.Lcd.print("NMEA Display");
  M5.Lcd.setCursor(210, 7);
  M5.Lcd.print("Batt");
}

# M5Stack-Wifi-NMEA-Display-JSON
A display for the M5Stack connecting via JSON to the NMEA 2000 WiFi Gateway.

This repository shows how to use the JSON Interface of the NMEA 2000 WiFi Gateway to show data on the display.
The include file BoatData.h and the JSON Identifiers should be the same as within the Gateway code.

Using JSON as the interface to transfer data is much easier than to use NMEA2000 or NMEA0183 protocols for this topic.

- With left two buttons you can select the different pages.
- The right button is switching the brightness.
- On/Off button works like standard of M5Stack.

For the M5Stack the Board Software has to be installed: https://docs.m5stack.com/#/en/arduino/arduino_home_page
In addition the ArduinoJson library has to be installed with the Arduiono IDE Library Manager.

![Display1](https://github.com/AK-Homberger/M5Stack-Wifi-NMEA-Diaplay-JSON/blob/master/IMG_1149-1.jpg)

![Display2](https://github.com/AK-Homberger/M5Stack-Wifi-NMEA-Display-JSON/blob/master/IMG_1150.jpg)

# Updates:

Version 0.4: 27.01.2020 - Added WiFi reconnect. Or restart if reconnect is not working.

Version 0.3: 27.01.2020 - Static JSON buffer. And non-return task for second core (to avoid re-starts in case of JSON errors). 

Version 0.2: 17.10.2019 - Increasd JSON buffer to 800 and changed position format to "00°00.000X".

Version 0.1: 14.10.2019

/*====================================================================================
  This sketch loads jpeg images which have been stored as files in the
  built-in FLASH memory on an ESP32 rendering the images onto a 160 x 80 pixel TFT screen.

  The images are stored in the SPI FLASH Filing System (SPIFFS), which effectively
  functions like a tiny "hard drive". This filing system is built into the ESP32
  Core that can be loaded from the IDE "Boards manager" menu option.

  The size of the SPIFFS partition can be set in the IDE as 1Mbyte or 3Mbytes. Either
  will work with this sketch.

  The Jpeg library can be found here:
  https://github.com/Bodmer/JPEGDecoder

  Place the images inside the sketch folder, in a folder called "Data".  Then upload
  all the files in the folder using the Arduino IDE "ESP32 Sketch Data Upload" option
  in the "Tools" menu
  
  This takes some time, but the SPIFFS content is not altered when a new sketch is
  uploaded, so there is no need to upload the same files again!
  Note: If open, you must close the "Serial Monitor" window to upload data to SPIFFS!

  The IDE will not copy the "data" folder with the sketch if you save the sketch under
  another name. It is necessary to manually make a copy and place it in the sketch
  folder.


  Created by Bodmer 24th Jan 2017 - Tested in Arduino IDE 1.8.0 esp8266 Core 2.3.0
  Modified by madsing 14 May 2020
  ==================================================================================*/

//====================================================================================
//                                  Libraries
//====================================================================================
// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#include <SPIFFS.h>

// JPEG decoder library
#include <JPEGDecoder.h>

// SPI library, built into IDE
#include <SPI.h>

// Call up the TFT library
#include <TFT_eSPI.h> // Hardware-specific library for ESP8266
// The TFT control pins are set in the User_Setup.h file <<<<<<<<<<<<<<<<< NOTE!
// that can be found in the "src" folder of the library

// Invoke TFT library
TFT_eSPI tft = TFT_eSPI();

//====================================================================================
//                                    Setup
//====================================================================================
void setup()
{
  uint16_t i;
  char jpegFile[20];
  uint32_t startTime;

  Serial.begin(115200); // Used for messages

  delay(10);
  Serial.println("Application starting");

  tft.begin();
  tft.setRotation(0); // 0 & 2 Portrait. 1 & 3 landscape
  tft.fillScreen(TFT_BLACK);

  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS initialisation failed!");
    while (1)
      yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");
  Serial.print("SPIFFS Total Bytes: "); Serial.println(SPIFFS.totalBytes());
  Serial.print("SPIFFS Used Bytes: ");  Serial.println(SPIFFS.usedBytes());

  // Note the / before the SPIFFS file name must be present, this means the file is in
  // the root directory of the SPIFFS, e.g. "/Tiger.jpg" for a file called "Tiger.jpg"

  drawJpeg("/back.jpeg", 0, 0);
  while (1)
    for (i = 1; i < 1000; i++)
    {
      sprintf(jpegFile, "/video%03d.jpeg", i);
      startTime = millis();
      if (!drawJpeg(jpegFile, 0, 0))
        break;  // exit for loop
      // Serial.println(millis() - startTime);
      while (millis() - startTime < 100);
    }
}

void loop()
{
}

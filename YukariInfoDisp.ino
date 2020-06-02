/*====================================================================================
  Sketch used for the tourist information display

  Hardware components used:
  - ESP32 (Lolin32) with 4BM Flash memory
  - 0.96" 80x180 RGB IPS display with ST7735 driver (only the top 80x80 pixels are 
    used and visible)
  
  Connections:
  DISPLAY     ESP32
  --------------------
  GND         GND
  VCC         3V3
  SCL         SCK       SPI Clock
  SDA         MOSI      SPI Data (to slave)
  RES         GPIO4     Reset
  DC          GPIO2     Data/Command
  CS          SS/5      Chip Select
  BLK         GPIO15    Blank (TFT backlight control)

  Support for the ESP32 board is the official Arduino core for the ESP32
  https://github.com/espressif/arduino-esp32
  Parameters:
  - Selected board: ESP32 Dev Module (esp32)
  - PSRAM: Disabled
  - Partition Scheme: No OTA (1MB APP/3MB SPIFFS)
  - CPU Frequency: 240MHz (WiFi/BT)
  - Flash Mode: QIO
  - Flash Frequency: 80MHz
  - Flash Size: 4MB (32Mb)
  - Upload Speed: 921600
  - Core Debug Level: None

  (Note: Lolin32 also works as the selected board, but it does not provide as many
  options for the partition scheme)

  Uploading files to the ESP32 flash:
  Install ESP32 Filesystem Uploader in Arduino IDE
  https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
  https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/

  The size of the ESP32 SPIFFS partition can be set in the IDE as 1Mbyte or 3Mbytes.

  Place the images inside the sketch folder, in a folder called "Data".  Then upload
  all the files in the folder using the Arduino IDE "ESP32 Sketch Data Upload" option
  in the "Tools" menu
  
  This takes some time, but the SPIFFS content is not altered when a new sketch is
  uploaded, so there is no need to upload the same files again!
  Note: The "Serial Monitor" window must be closed to upload data to SPIFFS!

  This sketch loads
  - an 80x80 pixel background image (back.jpeg) once at the beginning
  - a sequence of up to one thousand 40x80 images (videoNNN.jpeg) stored in the
    built-in flash memory.
  
  The videoNNN.jpeg files are built as follows:
  - Scale and crop the source video to 40x80 (portrait) with Handbrake
  - Extract the .jpeg files with ffmpeg:
    ./ffmpeg -i video.mp4 -s 40x80 -r 10 video%03d.jpeg

  ==================================================================================*/

// Return the minimum and maximum of two values a and b
#define minimum(a, b) (((a) < (b)) ? (a) : (b))
#define maximum(a, b) (((a) > (b)) ? (a) : (b))

// #define DEBUG
#define FRAME_PERIOD 100 // Time between two frames in ms

// SPIFFS flash filing system and SPI library (part of the ESP Core)
#include <SPIFFS.h>
#include <SPI.h>

// JPEG decoder library from https://github.com/Bodmer/JPEGDecoder
#include <JPEGDecoder.h>

// TFT library from https://github.com/Bodmer/TFT_eSPI
// Faster and better for bitmap transfer than the Adafruit library
// The TFT control pins are set in the User_Setup.h file that can be found in the "src"
// folder of the library (/Users/XXX/Documents/Arduino/libraries/TFT_eSPI/User_Setup.h)
// Current settings for Yukari Tourist Information display are:
// #define ST7735_DRIVER
// #define TFT_WIDTH  80
// #define TFT_HEIGHT 160
// #define ST7735_GREENTAB160x80 // For 160 x 80 display (BGR, inverted, 26 offset)
// #define TFT_CS    5  // Chip select control pin
// #define TFT_DC    2  // Data Command control pin
// #define TFT_RST   4  // Reset pin (could connect to RST pin)
// #define SPI_FREQUENCY  27000000 // Actually sets it to 26.67MHz = 80/3
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

// the number of the LED pin
const int blkPin = 15;  // GPIO15

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

void setup()
{
  uint16_t i;
  char jpegFilename[20];
  uint32_t startTime, frameTime;
  uint32_t statMin, statMax, statSum;

  #ifdef DEBUG
  Serial.begin(115200); // Used for debugging messages
  delay(10);
  Serial.println("Application starting");
  #endif

  tft.begin();
  tft.setRotation(0); // 0 & 2 Portrait. 1 & 3 landscape
  tft.fillScreen(TFT_BLACK);

  if (!SPIFFS.begin())
  {
    #ifdef DEBUG
    Serial.println("SPIFFS initialisation failed!");
    #endif
    while (1)
      yield(); // Stay here twiddling thumbs waiting
  }
  #ifdef DEBUG
  Serial.println("TFT and SPIFFS initialisation done");
  Serial.print("SPIFFS Total Bytes: "); Serial.println(SPIFFS.totalBytes());
  Serial.print("SPIFFS Used Bytes: ");  Serial.println(SPIFFS.usedBytes());
  #endif

  // Configure BLK LED PWM
  ledcSetup(ledChannel, freq, resolution);
  
  // Attach the channel to the GPIO to be controlled and set the duty cycle
  // to control the TFT back light
  ledcAttachPin(blkPin, ledChannel);
  ledcWrite(ledChannel, 40);
 
  // Note the / before the SPIFFS file name must be present, this means the file is in
  // the root directory of the SPIFFS
  // The first file is named video001.jpeg. The last file may be up to video999.jpeg
  // The for loop will break and restart at video001.jpeg as soon as a file is not
  // found or cannot be opened
  drawJpeg("/back.jpeg", 0, 0, 0);
  while (1)
  {
    #ifdef DEBUG
    statMin = 999999;
    statMax = 0;
    statSum = 0;
    #endif

    for (i = 1; i < 1000; i++)
    {
      #ifdef DEBUG
      startTime = millis();
      #endif

      sprintf(jpegFilename, "/video%03d.jpeg", i);
      if (!drawJpeg(jpegFilename, 0, 0, FRAME_PERIOD))
        break;  // File does not exist, exit 'for' loop
    
      #ifdef DEBUG
      frameTime = millis() - startTime;
      statMin = minimum(statMin, frameTime);
      statMax = maximum(statMax, frameTime);
      statSum += frameTime;
      #endif
    }
  #ifdef DEBUG
  Serial.println("Decoding Statistics ------");
  Serial.print("Minimum time: "); Serial.print(statMin); Serial.println("ms");
  Serial.print("Maximum time: "); Serial.print(statMax); Serial.println("ms");
  Serial.print("Average time: "); Serial.print(statSum/i); Serial.println("ms");
  #endif
  }
}

void loop()
{
}

/*====================================================================================
  This sketch contains support functions to render the Jpeg images.
  ==================================================================================*/

// Return the minimum of two values a and b
#define minimum(a, b) (((a) < (b)) ? (a) : (b))

//====================================================================================
//   Opens the image file and calls the Jpeg decoder
//   !!!! SPIFFS.open() always returns a File, even if the file is not found. This
//   !!!! function cannot be used to test if a file exists.
//   !!!! SPIFFS.exists() is used to check if the file exists.
//====================================================================================
bool drawJpeg(const char *filename, int xpos, int ypos, uint32_t framePeriod)
{
  uint32_t startTime;
  // We assume that the rendering time is 10ms
  const uint32_t renderTime = 10;
  
  if (framePeriod < renderTime)
    framePeriod = renderTime;
  startTime = millis();

  if (!SPIFFS.exists(filename))
  {
    Serial.print(filename);
    Serial.println(": File not found");
    return false;
  }
  // Open the named file (the Jpeg decoder library will close it after rendering the image)
  File jpegFile = SPIFFS.open(filename, "r");

  // Use one of the three following methods to initialise the decoder:
  boolean decoded = JpegDec.decodeFsFile(jpegFile); // Pass a SPIFFS file handle to the decoder,
  //boolean decoded = JpegDec.decodeSdFile(jpegFile); // or pass the SD file handle to the decoder,
  //boolean decoded = JpegDec.decodeFsFile(filename);  // or pass the filename (leading / distinguishes SPIFFS files)
  // Note: the filename can be a String or character array type
  if (!decoded)
  {
    Serial.print(filename);
    Serial.println(": Jpeg file format not supported");
    return false;
  }

  // Wait for the right time to display the resulting image
  // This may be zero if the decoding time was too long
  while ((millis() - startTime) < (framePeriod - renderTime));
  
  // Render the image onto the screen at given coordinates
  jpegRender(xpos, ypos);
  return true;
}

//====================================================================================
//   Decode and render the Jpeg image onto the TFT screen
//====================================================================================
void jpegRender(int xpos, int ypos)
{
  // Retrieve information about the image
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while (JpegDec.readSwappedBytes())
  { // Swap byte order so the SPI buffer can be used
    // save a pointer to the image block
    pImg = JpegDec.pImage;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos; // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x)
      win_w = mcu_w;
    else
      win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y)
      win_h = mcu_h;
    else
      win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // draw image MCU block only if it will fit on the screen
    if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
      tft.pushRect(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ((mcu_y + win_h) >= tft.height())
      JpegDec.abort();
  }
}

//====================================================================================
//   Print information decoded from the Jpeg image
//====================================================================================
void jpegInfo()
{
  Serial.println("===============");
  Serial.println("JPEG image info");
  Serial.println("===============");
  Serial.print("Width      :");
  Serial.println(JpegDec.width);
  Serial.print("Height     :");
  Serial.println(JpegDec.height);
  Serial.print("Components :");
  Serial.println(JpegDec.comps);
  Serial.print("MCU / row  :");
  Serial.println(JpegDec.MCUSPerRow);
  Serial.print("MCU / col  :");
  Serial.println(JpegDec.MCUSPerCol);
  Serial.print("Scan type  :");
  Serial.println(JpegDec.scanType);
  Serial.print("MCU width  :");
  Serial.println(JpegDec.MCUWidth);
  Serial.print("MCU height :");
  Serial.println(JpegDec.MCUHeight);
  Serial.println("===============");
  Serial.println("");
}

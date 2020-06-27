#if !defined(MY_DISPLAY_H)
#define MY_DISPLAY_H

#if !defined(ST7789_DRIVER)
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#else
#include <TFT_eSPI.h>
#endif
#include <SPI.h>

#if !defined(ST7789_DRIVER)
SPIClass tft_spi(PB15, PB14, PB13);
auto tft = Adafruit_ST7789(&tft_spi, -1, DISPLAY_DC, DISPLAY_RST);
#else
auto tft = TFT_eSPI();
#endif

#if !defined(ST7789_DRIVER)
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#else
#include <TFT_eSPI.h>
#endif
#include <SPI.h>

void display_init() {
    #if !defined(ST7789_DRIVER)    
    tft.init(240, 240, SPI_MODE3);                // Init ST7789 240x240
    //tft.setSPISpeed(64000000L);
    tft.setRotation(1);  
    tft.setFont(&FreeMonoBoldOblique12pt7b);
    #else
    SPI.setMISO(TFT_MISO);
    SPI.setMOSI(TFT_MOSI);
    SPI.setSCLK(TFT_SCLK);
    tft.init();
    tft.setTextFont(2);
    tft.setRotation(1);
    tft.setCursor(0, 0);
    #endif
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
}

#endif
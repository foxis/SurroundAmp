#include <Arduino.h>
#include <math.h>
#include "pins.h"
#include "settings.h"
#include "soundctrl.h"
#include "menu/home.h"
#include "menu/volume.h"
#include "menu/quick.h"
#include "menu/main.h"


Home menu_home({
  new VolumeMaster(),
  new VolumeBass(),
  new QuickMenu(),
  new MainMenu(),
});
static uint16_t smooth_vu[6] = {0};
static int16_t vu_offset[] = {2048-2, 2048-2, 2048+1, 2048-3, 2048-4, 2048-1};

bool test(uint8_t addr) {
    Wire.beginTransmission(addr);
    return !Wire.endTransmission();
}

void discover(size_t num_results, size_t addr_start) {
    tft.setCursor(0,0);
    for (size_t i = addr_start; i < (addr_start + num_results); i++)
    {
      if (test(i))
      {
        tft.print(i);
        tft.println();
      }
    }
}


void setup() {
  pinMode(DISPLAY_BL, OUTPUT);
  pinMode(AMP_MUTE, OUTPUT);
  pinMode(AMP_POWER, OUTPUT);
  pinMode(PA0, INPUT_ANALOG);
  pinMode(PA1, INPUT_ANALOG);
  pinMode(PA2, INPUT_ANALOG);
  pinMode(PA3, INPUT_ANALOG);
  pinMode(PA4, INPUT_ANALOG);
  pinMode(PA5, INPUT_ANALOG);
  pinMode(PA6, INPUT_ANALOG);

  analogReadResolution(12);
  analogWrite(DISPLAY_BL, DISPLAY_BL_DEFAULT); // Backlight on
  analogWriteFrequency(50000);

  control_left.setAccelerationEnabled(true);
  control_right.setAccelerationEnabled(true);

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

  tft.println("5.1 channel amplifier");
  tft.println("Build: " __DATE__);
  tft.println("Reading setup");
  read_settings(&settings);
  tft.println("Setting up sound processor");
  setup_soundctrl();

  tft.println("Muting...");
  snd_selector.mute_all(true);

  tft.println("Reading no signal level");
  const int N = 1000;
  for (int j = 0; j < 6; j++) {
    uint32_t acc = 0;
    for (int i = 0; i < 1000; i++) {
      acc += analogRead(PA0 + j);
    }
    vu_offset[j] = acc / N;
  }

  mute_changed(false);
  tft.println("Done");
  menu_home.begin(micros());
}

void read_controls(unsigned long now) {
  static unsigned long last_now = micros();

  if (now - last_now > 1000) {
    control_right.service();
    control_left.service();
    last_now = now;
  }
}

void on_activity() {
  last_activity = millis();
  if (back_light_level == 0) {
    back_light_level = DISPLAY_BL_MIN;
  }
}

void control_backlight(unsigned long now) {
  static unsigned long last_bl_level = 0;
  static unsigned long last_now = micros();
  unsigned long now_millis = millis();

  if (now_millis - last_user_input > DISPLAY_BL_TIMEOUT) {
    if (now - last_now > 50000L && back_light_level > DISPLAY_BL_MIN) {
      last_now = now;
      back_light_level--;
    }
  }
  // if (now_millis - last_activity > DISPLAY_BL_OFF_TIMEOUT) {
  //   back_light_level = DISPLAY_BL_SLEEP;
  //   if (digitalRead(AMP_POWER))
  //     menu_home.enter(MENU_SLEEP);
  // }
  if (last_bl_level != back_light_level) {
    analogWrite(DISPLAY_BL, back_light_level);
    last_bl_level = back_light_level;
  }
}


void read_audio(unsigned long now) {
  static int16_t smooth_raw[6] = {0};
  static unsigned long last_now = micros();
  static float MIN_DB = log10(2048.0f);
  uint16_t total = 0;

  if (now - last_now < 100)
    return;
  last_now = now;

  #define SMOOTH_RATE 30
  for (int i = 0; i < 6; i++) {
    smooth_raw[i] = (SMOOTH_RATE * (int32_t)smooth_raw[i] + ((int32_t)analogRead(PA0 + i) - vu_offset[i])) / (SMOOTH_RATE + 1);
  }

  // Use Peak to Peak
  for (int i = 0; i < 6; i++) {
    uint16_t amp = abs(smooth_raw[i]);
    float lg = max(0.0f, log10(2048.0f / (float)(amp + 1)) / MIN_DB);
    amp = min((uint16_t)255, (uint16_t)((1 - lg) * 255));
    //w_debug.get<WidgetLabel>(i)->set_text(String("raw: ") + amp);
    // amp = (amp * 255) / 2048;
    smooth_vu[i] = amp;
    total += amp;
  }
  total = total / 6;

  // TODO: determine if signal is coming

  // static int x = 0;
  // tft.drawFastVLine(x, 2*240/3, 2*240/3, WC_BLACK);
  // tft.drawPixel(x, 2*240/3 + (240/3)/2 - (total * (240/3)/2) / 256, WC_WHITE);
  // x = (x+1)%240;

  // FFT processing, etc.
  if (!digitalRead(AMP_POWER))
    return;

}

void loop() {
  unsigned long now = micros();
  static unsigned long last_now = micros();

    // while(true){
    //     tft.fillScreen(WC_BLACK);
    //     discover(255, 0);
    // }

  read_controls(now);
  control_backlight(now);
  read_audio(now);

  if (now - last_now > 10000) {
    if (settings.master.mute.all) {
      w_home_vu.get<WidgetHBarVolume>(0)->set_value(128 + 127 * sin(now / 500000.0));
      w_home_vu.get<WidgetHBarVolume>(1)->set_value(128 + 127 * sin(now / 1000000.0));
      w_home_vu.get<WidgetHBarVolume>(2)->set_value(128 + 127 * sin(now / 1100000.0));
      w_home_vu.get<WidgetHBarVolume>(3)->set_value(128 + 127 * sin(now / 4000000.0));
      w_home_vu.get<WidgetHBarVolume>(4)->set_value(128 + 127 * sin(now / 5000000.0));
      w_home_vu.get<WidgetHBarVolume>(5)->set_value(128 + 127 * sin(now / 300000.0));
    } else {
      for (int i = 0; i < 6; i++) {
        int16_t val = w_home_vu.get<WidgetHBarVolume>(i)->value;
        if (smooth_vu[i] >= val)
          w_home_vu.get<WidgetHBarVolume>(i)->set_value(smooth_vu[i]);
        else 
          w_home_vu.get<WidgetHBarVolume>(i)->add_value(min((int16_t)(val - (int16_t)smooth_vu[i]), (int16_t)-10));
      }
    }

    menu_home.loop(now);
    last_now = now;
  }
}

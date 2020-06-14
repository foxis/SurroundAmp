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


void setup() {
  pinMode(DISPLAY_BL, OUTPUT);
  analogWrite(DISPLAY_BL, DISPLAY_BL_DEFAULT); // Backlight on

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
  #endif
  tft.setTextSize(1);

  read_settings(&settings);
  setup_soundctrl();
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
    analogWrite(DISPLAY_BL, back_light_level);
  }
}

void control_backlight(unsigned long now) {
  static unsigned long last_now = micros();
  unsigned long now_millis = millis();

  if (now_millis - last_user_input > DISPLAY_BL_TIMEOUT) {
    if (now - last_now > 50000L && back_light_level > DISPLAY_BL_MIN) {
      last_now = now;
      back_light_level--;
      analogWrite(DISPLAY_BL, back_light_level);
    }
  }
  if (now_millis - last_activity > DISPLAY_BL_OFF_TIMEOUT) {
    back_light_level = 0;
    analogWrite(DISPLAY_BL, back_light_level);
  }
}

void read_audio(unsigned long now) {
  static int smooth_vu[6] = {0};
  static unsigned long last_now = micros();
  static float MIN_DB = log10(1 / 2048.0);
  int total = 0;

  if (now - last_now < 1000)
    return;
  last_now = now;

  int raw[] = {
    analogRead(PA0),
    analogRead(PA1),
    analogRead(PA2),
    analogRead(PA3),
    analogRead(PA4),
    analogRead(PA5),
  };
  // Use Peak to Peak
  for (int i = 0; i < 6; i++) {
    uint16_t amp = abs(raw[i] - 2048);
    uint16_t tmp = amp > 0 ? 255 - min((uint8_t)255, (uint8_t)((20 * log10(amp / 2048.0) / MIN_DB) * 255)) : 0;
    smooth_vu[i] = (smooth_vu[i] + tmp) / 2;
    total += tmp;
  }


  // FFT processing, etc.
  if (back_light_level == 0)
    return;

  if (settings.master.mute.all) {
    w_home_vu.get<WidgetHBarVolume>(0)->set_value(128 + 127 * sin(now / 500000.0));
    w_home_vu.get<WidgetHBarVolume>(1)->set_value(128 + 127 * sin(now / 1000000.0));
    w_home_vu.get<WidgetHBarVolume>(2)->set_value(128 + 127 * sin(now / 1100000.0));
    w_home_vu.get<WidgetHBarVolume>(3)->set_value(128 + 127 * sin(now / 4000000.0));
    w_home_vu.get<WidgetHBarVolume>(4)->set_value(128 + 127 * sin(now / 5000000.0));
    w_home_vu.get<WidgetHBarVolume>(5)->set_value(128 + 127 * sin(now / 300000.0));
  } else {
    for (int i = 0; i < 6; i++)
      w_home_vu.get<WidgetHBarVolume>(i)->set_value(smooth_vu[i]);
  }
}

void loop() {
  unsigned long now = micros();
  static unsigned long last_now = micros();

  read_controls(now);
  control_backlight(now);
  read_audio(now);

  if (now - last_now > 10000) {
    menu_home.loop(now);
    last_now = now;
  }
}

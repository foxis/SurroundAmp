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

// bool test(uint8_t addr) {
//     Wire.beginTransmission(addr);
//     return !Wire.endTransmission();
// }

// void discover(size_t num_results, size_t addr_start) {
//     tft.setCursor(0,0);
//     for (size_t i = addr_start; i < (addr_start + num_results); i++)
//     {
//       if (test(i))
//       {
//         tft.print(i);
//         tft.println();
//       }
//     }
// }


void setup() {
  //pinMode(DISPLAY_RST, OUTPUT);
  pinMode(DISPLAY_BL, OUTPUT);
  //pinMode(DISPLAY_CS, OUTPUT);
  pinMode(AMP_MUTE, OUTPUT);
  pinMode(AMP_FAULT, INPUT);
  pinMode(AMP_POWER, OUTPUT);
  pinMode(PA0, INPUT_ANALOG);
  pinMode(PA1, INPUT_ANALOG);
  pinMode(PA2, INPUT_ANALOG);
  pinMode(PA3, INPUT_ANALOG);
  pinMode(PA4, INPUT_ANALOG);
  pinMode(PA5, INPUT_ANALOG);
  pinMode(PA6, INPUT_ANALOG);

  digitalWrite(DISPLAY_RST, HIGH);

  analogReadResolution(12);
  analogWrite(DISPLAY_BL, DISPLAY_BL_DEFAULT); // Backlight on
  analogWriteFrequency(50000);

  control_left.setAccelerationEnabled(true);
  control_right.setAccelerationEnabled(true);

  display_init();
  
  tft.setTextSize(2);
  tft.setTextColor(WC_GOLD);
  tft.println("5.1 amplifier");
  tft.println(__DATE__);
  tft.setTextSize(1);
  read_settings(&settings);
  setup_soundctrl();

  snd_selector.mute_all(true);
  snd_processor.mute_all(true);

  tft.setTextColor(WC_WHITE);
  tft.print("Current: ");
  uint32_t acc = 0;
  for (int i = 0; i < 100; i++) {
    acc += analogRead(PA6);
    delayMicroseconds(100);
  }
  tft.setTextColor(WC_YELLOW);
  tft.println(acc / 100);
  tft.setTextColor(WC_WHITE);

  mute_changed(false);

  delay(1000);

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
    for (int i = 0; i < 6; i++) {
      int16_t val = w_home_vu.get<WidgetHBarVolume>(i)->value;
      if (smooth_vu[i] >= val)
        w_home_vu.get<WidgetHBarVolume>(i)->set_value(smooth_vu[i]);
      else 
        w_home_vu.get<WidgetHBarVolume>(i)->add_value(min((int16_t)(val - (int16_t)smooth_vu[i]), (int16_t)-10));
    }

    menu_home.loop(now);
    last_now = now;
  }
}

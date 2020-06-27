#if !defined(SOUND_CONTROL_H)
#define SOUND_CONTROL_H

#include <Wire.h>
#include "settings.h"
#include "display.h"

#include "PT2322.h"
#include "PT2323.h"


PT2322 snd_processor;
PT2323 snd_selector;
static uint16_t smooth_vu[6] = {0};
static int16_t vu_offset[] = {2048-2, 2048-2, 2048+1, 2048-3, 2048-4, 2048-1};

void mute_changed(bool reset_preset);
void input_changed(bool reset_preset);
void volume_changed(bool reset_preset);
void tone_changed(bool reset_preset);
void channel_trim_changed(bool reset_preset);


void setup_soundctrl() {
    Wire.begin();//AMP_SDA, AMP_SCL);
    Wire.setClock(100000);
    delay(500);
    snd_selector.begin();
    snd_processor.begin();

    snd_selector.mute_all(true);
    snd_processor.mute_all(true);

    digitalWrite(AMP_MUTE, HIGH);
    digitalWrite(AMP_POWER, HIGH);
    delay(1000);
    digitalWrite(AMP_MUTE, LOW);

    const int N = 1000;
    for (int j = 0; j < 6; j++) {
        tft.setTextColor(TFT_WHITE);
        tft.printf("Channel: %i offset: ", j + 1);
        uint32_t acc = 0;
        for (int i = 0; i < N; i++) {
            acc += analogRead(PA0 + j);
            delayMicroseconds(100);
        }
        vu_offset[j] = acc / N;
        tft.setTextColor(TFT_GREEN);
        tft.println(vu_offset[j]);
    }

    channel_trim_changed(false);
    tone_changed(false);
    volume_changed(false);
    input_changed(false);
    mute_changed(false);
}

void selected_preset_changed() {
    if (settings.selected_preset == 0xFF)
        return;
    preset_t * preset = &(settings.presets[settings.selected_preset]);
    memcpy(settings.master.channels, preset->channels, sizeof(preset->channels));
    memcpy(&settings.master.input, &preset->input, sizeof(preset->input));
    memcpy(settings.master.tone, preset->tone, sizeof(preset->tone));
    input_changed(false);
    channel_trim_changed(false);
    tone_changed(false);
}

void mute_changed(bool reset_preset=true) {
    snd_processor.function(settings.master.mute);
    snd_selector.mute(settings.master.mute);
    if (reset_preset)
        settings.selected_preset = 0xFF;
}

void input_changed(bool reset_preset=true) {
    snd_selector.input(settings.master.input);
    if (reset_preset)
        settings.selected_preset = 0xFF;
}

void volume_changed(bool reset_preset=true) {
    snd_processor.master(settings.master.volume);
    if (reset_preset)
        settings.selected_preset = 0xFF;
}

void tone_changed(bool reset_preset=true) {
    snd_processor.tone(settings.master.tone);
    if (reset_preset)
        settings.selected_preset = 0xFF;
}

void channel_trim_changed(bool reset_preset=true) {
    snd_processor.trim(settings.master.channels);
    if (reset_preset)
        settings.selected_preset = 0xFF;
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

#endif

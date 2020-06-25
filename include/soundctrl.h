#if !defined(SOUND_CONTROL_H)
#define SOUND_CONTROL_H

#include <Wire.h>
#include "settings.h"

#include "PT2322.h"
#include "PT2323.h"


PT2322 snd_processor;
PT2323 snd_selector;

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

    channel_trim_changed(false);
    tone_changed(false);
    volume_changed(false);
    input_changed(false);
    mute_changed(false);
    digitalWrite(AMP_POWER, HIGH);
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

#endif

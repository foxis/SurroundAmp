#if !defined(QUICK_MENU_H)
#define QUICK_MENU_H

#include "soundctrl.h"
#include "menu/menu.h"
#include "widgets/label.h"
#include "widgets/bar_horizontal.h"
#include "widgets/bar_horizontal_signed.h"

WidgetContainer w_quick(WidgetContainer::VERTICAL, {
    new WidgetTitleLabel(236, 30, "QUICK MENU"),
    new WidgetLabel(236, 20, "Mute"),
    new WidgetSeparator(236),
    new WidgetLabel(236, 20, "> Channel TRIM"),
    new WidgetLabel(236, 20, "> Adjust Tone"),
    new WidgetSeparator(236),
    new WidgetLabel(236, 20, "Preset 1"),
    new WidgetLabel(236, 20, "Preset 2"),
    new WidgetLabel(236, 20, "Preset 3"),
    new WidgetLabel(236, 20, "Preset 4"),
    new WidgetSeparator(236),
    new WidgetLabel(236, 20, "Sleep"),
    new WidgetLabel(236, 20, "Back"),
});

WidgetContainer w_quick_channels(WidgetContainer::VERTICAL, {
    new WidgetTitleLabel(236, 30, "CHANNEL TRIM"),
    new WidgetContainer(WidgetContainer::HORIZONTAL, {
        new WidgetLabel(72, 20, "FL"),
        new WidgetHBar(150, 20, 0, MAX_CHANNEL_TRIM),
    }),
    new WidgetContainer(WidgetContainer::HORIZONTAL, {
        new WidgetLabel(72, 20, "FR"),
        new WidgetHBar(150, 20, 0, MAX_CHANNEL_TRIM),
    }),
    new WidgetContainer(WidgetContainer::HORIZONTAL, {
        new WidgetLabel(72, 20, "CT"),
        new WidgetHBar(150, 20, 0, MAX_CHANNEL_TRIM),
    }),
    new WidgetContainer(WidgetContainer::HORIZONTAL, {
        new WidgetLabel(72, 20, "LFE"),
        new WidgetHBar(150, 20, 0, MAX_CHANNEL_TRIM),
    }),
    new WidgetContainer(WidgetContainer::HORIZONTAL, {
        new WidgetLabel(72, 20, "SL"),
        new WidgetHBar(150, 20, 0, MAX_CHANNEL_TRIM),
    }),
    new WidgetContainer(WidgetContainer::HORIZONTAL, {
        new WidgetLabel(72, 20, "SR"),
        new WidgetHBar(150, 20, 0, MAX_CHANNEL_TRIM),
    }),
    new WidgetSeparator(236),
    new WidgetLabel(236, 20, "Back"),
});


WidgetContainer w_quick_tone(WidgetContainer::VERTICAL, {
    new WidgetTitleLabel(236, 30, "TONE"),
    new WidgetContainer(WidgetContainer::HORIZONTAL, {
        new WidgetLabel(72, 20, "Bass"),
        new WidgetHBarSigned(150, 20, 0, MIN_TONE, MAX_TONE),
    }),
    new WidgetContainer(WidgetContainer::HORIZONTAL, {
        new WidgetLabel(72, 20, "Mid"),
        new WidgetHBarSigned(150, 20, 0, MIN_TONE, MAX_TONE),
    }),
    new WidgetContainer(WidgetContainer::HORIZONTAL, {
        new WidgetLabel(72, 20, "Tre"),
        new WidgetHBarSigned(150, 20, 0, MIN_TONE, MAX_TONE),
    }),
    new WidgetLabel(236, 20, "Tone Enable"),
    new WidgetSeparator(236),
    new WidgetLabel(236, 20, "Back"),
});

class QuickMute : public WidgetMenuItem<Widget> {
public:
    QuickMute(Widget * widget) : WidgetMenuItem<Widget>(MENU_MUTE, widget) {}
    virtual void on_enter(unsigned long now) {
        settings.master.mute.all ^= 1;
        mute_changed();
        leave(now);
    }
};
class QuickSleep : public WidgetMenuItem<Widget> {
public:
    QuickSleep(Widget * widget) : WidgetMenuItem<Widget>(MENU_SLEEP, widget) {}
    virtual void on_enter(unsigned long now) {
        reset(now);
    }
};

class QuickChannelTrim : public RotaryEncoderMenu<Widget> {
    channel_t * channels;

public:
    QuickChannelTrim(Widget * widget, ClickEncoder * ctrl, channel_t * channels) : RotaryEncoderMenu<Widget>(MENU_NONE, widget, ctrl, {
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(1), ctrl, channels, channel_trim_changed),
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(2), ctrl, channels + 1, channel_trim_changed),
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(3), ctrl, channels + 2, channel_trim_changed),
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(4), ctrl, channels + 3, channel_trim_changed),
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(5), ctrl, channels + 4, channel_trim_changed),
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(6), ctrl, channels + 5, channel_trim_changed),
        new WidgetBackMenuItem(w_quick_channels[8]),
    }) {
        this->channels = channels;
        SETUP_MAIN_CONTAINER(w_quick_channels)
    }

    virtual void on_enter(unsigned long now) {
        for (int i = 0; i < 6; i++) {
            w_quick_channels.get<WidgetContainer>(i + 1)->get<WidgetHBar>(1)->set_value(channels[i]);
        }
        RotaryEncoderMenu<Widget>::on_enter(now);
        w_quick_channels.set_redraw_all();
        w_quick_channels.draw(&tft);
    }
};

class QuickTone : public RotaryEncoderMenu<Widget> {
    tone_t * tone;
public:
    QuickTone(Widget * widget, ClickEncoder * ctrl, tone_t * tone) : RotaryEncoderMenu<Widget>(MENU_NONE, widget, ctrl, {
        new HBarMenuItem<tone_t, WidgetHBarSigned>(w_quick_tone.get<WidgetContainer>(1), ctrl, tone, tone_changed),
        new HBarMenuItem<tone_t, WidgetHBarSigned>(w_quick_tone.get<WidgetContainer>(2), ctrl, tone + 1, tone_changed),
        new HBarMenuItem<tone_t, WidgetHBarSigned>(w_quick_tone.get<WidgetContainer>(3), ctrl, tone + 2, tone_changed),
        new WidgetToneDefeatSelectionMenuItem(w_quick_tone[4], &settings.master.mute),
        new WidgetBackMenuItem(w_quick_tone[6]),
    }) {
        this->tone = tone;
        SETUP_MAIN_CONTAINER(w_quick_tone);
    }

    virtual void on_enter(unsigned long now) {
        for (int i = 0; i < 3; i++) {
            w_quick_tone.get<WidgetContainer>(i + 1)->get<WidgetHBarSigned>(1)->set_value(tone[i]);
        }
        w_quick_tone[4]->set_background_color(settings.master.mute.tone ? WC_BLACK : WC_DARK_YELLOW);
        RotaryEncoderMenu::on_enter(now);
        w_quick_tone.set_redraw_all();
        w_quick_tone.draw(&tft);
    }
};

class QuickMenu : public RotaryEncoderMenu<Widget>
{
public:
    QuickMenu() : RotaryEncoderMenu<Widget>(MENU_QUICK, NULL, &control_left, {
        new QuickMute(w_quick[1]),
        new QuickChannelTrim(w_quick[3], &control_left, settings.master.channels),
        new QuickTone(w_quick[4], &control_left, settings.master.tone),
        new WidgetSelectionMenuItem<uint8_t>(w_quick[6], &settings.selected_preset, 0, selected_preset_changed),
        new WidgetSelectionMenuItem<uint8_t>(w_quick[7], &settings.selected_preset, 1, selected_preset_changed),
        new WidgetSelectionMenuItem<uint8_t>(w_quick[8], &settings.selected_preset, 2, selected_preset_changed),
        new WidgetSelectionMenuItem<uint8_t>(w_quick[9], &settings.selected_preset, 3, selected_preset_changed),
        new QuickSleep(w_quick[11]),
        new WidgetBackMenuItem(w_quick[12]),
    }) {
        SETUP_MAIN_CONTAINER(w_quick);
    }

    virtual void on_enter(unsigned long now) {
        if (settings.master.mute.all) {
            w_quick[1]->set_background_color(WC_DARK_RED);
            w_quick.get<WidgetLabel>(1)->set_text("Unmute");
        } else {
            w_quick[1]->set_background_color(WC_BLACK);
            w_quick.get<WidgetLabel>(1)->set_text("Mute");
        }
        for (int i = 0; i < 4; i++) {
            if (settings.selected_preset == i)
                w_quick[i + 6]->set_background_color(WC_DARK_YELLOW);
            else
                w_quick[i + 6]->set_background_color(WC_BLACK);
        }
        RotaryEncoderMenu<Widget>::on_enter(now);
        w_quick.set_redraw_all();
        w_quick.draw(&tft);
    }
};

#endif

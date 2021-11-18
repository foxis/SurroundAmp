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
    new WidgetLabel(236, 20, "Music"),
    new WidgetLabel(236, 20, "Music 1"),
    new WidgetLabel(236, 20, "Movies"),
    new WidgetLabel(236, 20, "Movies 1"),
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

#define SLEEP_BALLS_COUNT 11
class QuickSleep : public WidgetMenuItem<Widget> {
    unsigned long last_now;
    struct {
        float dx, dy;
    } velocities[SLEEP_BALLS_COUNT]; 
    bool collisions[SLEEP_BALLS_COUNT * SLEEP_BALLS_COUNT];
    struct {
        uint16_t x, y;
        int16_t dx, dy;
        uint8_t r;
        uint16_t c;
    } balls[SLEEP_BALLS_COUNT];
    float total_energy;
public:
    QuickSleep(Widget * widget) : WidgetMenuItem<Widget>(MENU_SLEEP, widget) {}

    virtual void on_handle(unsigned long now) {
        auto btn = control_right.getButton();
        auto rot = control_right.getValue() + control_left.getValue();
        back_light_level = DISPLAY_BL_SLEEP;
        if (btn == ClickEncoder::Released || digitalRead(AMP_POWER)) {
            tft.init();
            digitalWrite(AMP_POWER, HIGH);
            delay(1000);
            digitalWrite(AMP_MUTE, HIGH);
            mute_changed(false);
            tft.fillScreen(WC_BLACK);
            reset(now);
            return;
        } else if (btn == ClickEncoder::Clicked || btn == ClickEncoder::Held || rot != 0) {
            on_user_input();
        }

        memset(collisions, 0, sizeof(collisions));
        memset(velocities, 0, sizeof(velocities));

        // first ball
        for (int i = 0; i < SLEEP_BALLS_COUNT; i++) {
            // second ball
            for (int j = 0; j < SLEEP_BALLS_COUNT; j++) {
                if (i == j) continue;
                if (collisions[i * SLEEP_BALLS_COUNT + j]) continue;

                uint16_t x1 = balls[i].x + balls[i].dx;
                uint16_t x2 = balls[j].x + balls[j].dx;
                uint16_t y1 = balls[i].y + balls[i].dy;
                uint16_t y2 = balls[j].y + balls[j].dy;

                float dx = ((int32_t)x2 - (int32_t)x1) / 256.0f;
                float dy = ((int32_t)y2 - (int32_t)y1) / 256.0f;
                float d2 = dx * dx + dy * dy;
                float m1m2 = balls[i].r + balls[j].r;
                if (d2 > m1m2 * m1m2) continue;
                collisions[i * SLEEP_BALLS_COUNT + j] = collisions[j * SLEEP_BALLS_COUNT + i] = true;

                float vx1 = balls[i].dx / 256.0f;
                float vy1 = balls[i].dy / 256.0f;
                float vx2 = balls[j].dx / 256.0f;
                float vy2 = balls[j].dy / 256.0f;
                float m1 = 2 * balls[j].r / m1m2;
                float m2 = 2 * balls[i].r / m1m2;

                float dotv1 = ((vx1 - vx2) * (-dx) + (vy1 - vy2) * (-dy)) / d2;
                float dotv2 = ((vx2 - vx1) * dx + (vy2 - vy1) * dy) / d2;

                velocities[i].dx -= m1 * (-dx) * dotv1; 
                velocities[i].dy -= m1 * (-dy) * dotv1; 
                velocities[j].dx -= m2 * dx * dotv2; 
                velocities[j].dy -= m2 * dy * dotv2; 
            }
        }

        for (int i = 0; i < SLEEP_BALLS_COUNT; i++) {
            balls[i].dx += (int16_t)(velocities[i].dx * 256);
            balls[i].dy += (int16_t)(velocities[i].dy * 256);
        }

        for (int i = 0; i < SLEEP_BALLS_COUNT; i++) {
            uint16_t x = balls[i].x + balls[i].dx;
            uint16_t y = balls[i].y + balls[i].dy;
            uint16_t r = balls[i].r; 

            tft.fillCircle(balls[i].x >> 8, balls[i].y >> 8, balls[i].r, WC_BLACK);

            if ( x <= (r << 8) || x >= ((240 - r) << 8) ) 
                balls[i].dx = -balls[i].dx;
            else
                balls[i].x = x;
            if ( y <= (3 << 8) || y >= (237 << 8) ) 
                balls[i].dy = -balls[i].dy;
            else
                balls[i].y = y;

            tft.fillCircle(balls[i].x >> 8, balls[i].y >> 8, balls[i].r, balls[i].c);
        }
    }

    virtual void on_enter(unsigned long now) {
        last_now = now;
        control_left.getValue();
        control_right.getValue();
        control_left.getButton();
        control_right.getButton();
        snd_processor.mute_all(true);
        tft.init();
        digitalWrite(AMP_MUTE, LOW);
        delay(500);
        digitalWrite(AMP_POWER, LOW);
        tft.fillScreen(WC_BLACK);
        const uint16_t colors[] = {
            WC_GREEN_YELLOW,
            WC_RED,
            WC_BLUE,
            WC_GOLD,
            WC_VIOLET,
            WC_PINK,
            WC_GREEN,
            WC_YELLOW,
            WC_WHITE,
        };
        total_energy = 0;
        for (int i = 0; i < SLEEP_BALLS_COUNT; i++) {
            balls[i].x = (30 + i * 200 / SLEEP_BALLS_COUNT) << 8;
            balls[i].y = random(3, 237) << 8;
            balls[i].dx = 0x0100 - (random(255) << 1);
            balls[i].dy = 0x0100 - (random(255) << 1);
            balls[i].r = random(3, 7);
            balls[i].c = colors[random(sizeof(colors) / 2)];
        }
    }
};

class QuickChannelTrim : public RotaryEncoderMenu<Widget> {
    channel_t * channels;

public:
    QuickChannelTrim(Widget * widget, channel_t * channels) : RotaryEncoderMenu<Widget>(MENU_NONE, widget, {
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(1), channels, [channels](){if (channels == settings.master.channels) channel_trim_changed();}),
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(2), channels + 1, [channels](){if (channels == settings.master.channels) channel_trim_changed();}),
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(3), channels + 2, [channels](){if (channels == settings.master.channels) channel_trim_changed();}),
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(4), channels + 3, [channels](){if (channels == settings.master.channels) channel_trim_changed();}),
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(5), channels + 4, [channels](){if (channels == settings.master.channels) channel_trim_changed();}),
        new HBarMenuItem<channel_t, WidgetHBar>(w_quick_channels.get<WidgetContainer>(6), channels + 5, [channels](){if (channels == settings.master.channels) channel_trim_changed();}),
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
    QuickTone(Widget * widget, tone_t * tone) : RotaryEncoderMenu<Widget>(MENU_NONE, widget, {
        new HBarMenuItem<tone_t, WidgetHBarSigned>(w_quick_tone.get<WidgetContainer>(1), tone, [tone](){if (tone == settings.master.tone) tone_changed();}),
        new HBarMenuItem<tone_t, WidgetHBarSigned>(w_quick_tone.get<WidgetContainer>(2), tone + 1, [tone](){if (tone == settings.master.tone) tone_changed();}),
        new HBarMenuItem<tone_t, WidgetHBarSigned>(w_quick_tone.get<WidgetContainer>(3), tone + 2, [tone](){if (tone == settings.master.tone) tone_changed();}),
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
    QuickMenu() : RotaryEncoderMenu<Widget>(MENU_QUICK, NULL, {
        new QuickMute(w_quick[1]),
        new QuickChannelTrim(w_quick[3], settings.master.channels),
        new QuickTone(w_quick[4], settings.master.tone),
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

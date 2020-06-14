#include "settings.h"
#include "menu.h"
#include "quick.h"
#include "widgets/label.h"

WidgetContainer w_main(WidgetContainer::VERTICAL, {
    new WidgetTitleLabel(236, 30, "MENU"),
    new WidgetLabel(236, 20, "> Input channel"),
    new WidgetLabel(236, 20, "> Channel TRIM"),
    new WidgetLabel(236, 20, "> Adjust Tone"),
    new WidgetLabel(236, 20, "> Presets"),
    new WidgetSeparator(236, 5),
    new WidgetLabel(236, 20, "Load settings"),
    new WidgetLabel(236, 20, "Save settings"),
    new WidgetLabel(236, 20, "Sleep"),
    new WidgetLabel(236, 20, "Back"),
});

WidgetContainer w_main_input(WidgetContainer::VERTICAL, {
    new WidgetTitleLabel(236, 30, "INPUT CHANNEL"),
    new WidgetLabel(236, 20, "Stereo 1"),
    new WidgetLabel(236, 20, "Stereo 2"),
    new WidgetLabel(236, 20, "Stereo 3"),
    new WidgetLabel(236, 20, "Stereo 4"),
    new WidgetLabel(236, 20, "Surround pass-through"),
    new WidgetSeparator(236, 5),
    new WidgetLabel(236, 20, "Enhanced Surround"),
    new WidgetLabel(236, 20, "Mixed"),
    new WidgetSeparator(236, 5),
    new WidgetLabel(236, 20, "Back"),
});

WidgetContainer w_main_presets(WidgetContainer::VERTICAL, {
    new WidgetTitleLabel(236, 30, "PRESETS"),
    new WidgetLabel(236, 20, "Preset 1"),
    new WidgetLabel(236, 20, "Preset 2"),
    new WidgetLabel(236, 20, "Preset 3"),
    new WidgetLabel(236, 20, "Preset 4"),
    new WidgetSeparator(236, 5),
    new WidgetLabel(236, 20, "Back"),
});

WidgetContainer w_main_preset(WidgetContainer::VERTICAL, {
    new WidgetTitleLabel(236, 30, ""),
    new WidgetLabel(236, 20, "> Input channel"),
    new WidgetLabel(236, 20, "> Channel TRIM"),
    new WidgetLabel(236, 20, "> Adjust Tone"),
    new WidgetSeparator(236, 5),
    new WidgetLabel(236, 20, "Set as default"),
    new WidgetLabel(236, 20, "Back"),
});


class MainLoad : public WidgetMenuItem<Widget> {
public:
    MainLoad(Widget * widget) : WidgetMenuItem<Widget>(MENU_NONE, widget) {}
    virtual void on_enter(unsigned long now) {
        read_settings(&settings);
        leave(now);
    }
};
class MainSave : public WidgetMenuItem<Widget> {
public:
    MainSave(Widget * widget) : WidgetMenuItem<Widget>(MENU_NONE, widget) {}
    virtual void on_enter(unsigned long now) {
        write_settings(&settings);
        leave(now);
    }
};

class MainInput : public RotaryEncoderMenu<Widget>
{
public:
    MainInput(Widget * widget, input_t * input) : RotaryEncoderMenu<Widget>(MENU_NONE, widget, &control_right, {
        new WidgetInputSelectionMenuItem(w_main_input[1], input, 0),
        new WidgetInputSelectionMenuItem(w_main_input[2], input, 1),
        new WidgetInputSelectionMenuItem(w_main_input[3], input, 2),
        new WidgetInputSelectionMenuItem(w_main_input[4], input, 3),
        new WidgetInputSelectionMenuItem(w_main_input[5], input, 4),
        new WidgetESSelectionMenuItem(w_main_input[7], input),
        new WidgetMixedSelectionMenuItem(w_main_input[8], input),
        new WidgetBackMenuItem(w_main_input[10]),
    }) {
        SETUP_MAIN_CONTAINER(w_main_input);
    }

    virtual void on_enter(unsigned long now) {
        RotaryEncoderMenu<Widget>::on_enter(now);
        for (int i = 0; i < 5; i++) {
            if (settings.master.input.channel == i)
                w_main_input[i + 1]->set_background_color(WC_DARK_YELLOW);
            else
                w_main_input[i + 1]->set_background_color(WC_BLACK);
        }
        w_main_input[7]->set_background_color(settings.master.input.es ? WC_DARK_YELLOW : WC_BLACK);
        w_main_input[8]->set_background_color(settings.master.input.mixed ? WC_DARK_YELLOW : WC_BLACK);

        w_main_input.set_redraw_all();
        w_main_input.draw(&tft);
    }
};

class MainPreset : public RotaryEncoderMenu<Widget>
{
    uint8_t id;
public:
    MainPreset(Widget * widget, uint8_t id) : RotaryEncoderMenu<Widget>(MENU_NONE, widget, &control_right, {
        new MainInput(w_main_preset[1], &settings.presets[id].input),
        new QuickChannelTrim(w_main_preset[2], &control_right, settings.presets[id].channels),
        new QuickTone(w_main_preset[3], &control_right, settings.presets[id].tone),
        new WidgetSelectionMenuItem<uint8_t>(w_main_preset[5], &settings.selected_preset, id, selected_preset_changed),
        new WidgetBackMenuItem(w_main_preset[6]),
    }) {
        this->id = id;
        SETUP_MAIN_CONTAINER(w_main_preset);
    }

    virtual void on_enter(unsigned long now) {
        RotaryEncoderMenu<Widget>::on_enter(now);
        w_main_preset.get<WidgetLabel>(0)->set_text(String("PRESET ") + (this->id + 1));
        w_main_preset.set_redraw_all();
        w_main_preset.draw(&tft);
    }
};

class MainPresets : public RotaryEncoderMenu<Widget>
{
public:
    MainPresets(Widget * widget) : RotaryEncoderMenu<Widget>(MENU_NONE, widget, &control_right, {
        new MainPreset(w_main_presets[1], 0),
        new MainPreset(w_main_presets[2], 1),
        new MainPreset(w_main_presets[3], 2),
        new MainPreset(w_main_presets[4], 3),
        new WidgetBackMenuItem(w_main_presets[6]),
    }) {
        SETUP_MAIN_CONTAINER(w_main_presets);
    }

    virtual void on_enter(unsigned long now) {
        RotaryEncoderMenu<Widget>::on_enter(now);
        for (int i = 0; i < 4; i++) {
            if (settings.selected_preset == i)
                w_main_presets[i + 1]->set_background_color(WC_DARK_YELLOW);
            else
                w_main_presets[i + 1]->set_background_color(WC_BLACK);
        }

        w_main_presets.set_redraw_all();
        w_main_presets.draw(&tft);
    }
};

class MainMenu : public RotaryEncoderMenu<Widget>
{
public:
    MainMenu() : RotaryEncoderMenu<Widget>(MENU_MAIN, NULL, &control_right, {
        new MainInput(w_main[1], &settings.master.input),
        new QuickChannelTrim(w_main[2], &control_right, settings.master.channels),
        new QuickTone(w_main[3], &control_right, settings.master.tone),
        new MainPresets(w_main[4]),
        new MainLoad(w_main[6]),
        new MainSave(w_main[7]),
        new QuickSleep(w_main[8]),
        new WidgetBackMenuItem(w_main[9]),
    }) {
        SETUP_MAIN_CONTAINER(w_main);
    }

    virtual void on_enter(unsigned long now) {
        RotaryEncoderMenu<Widget>::on_enter(now);
        w_main.set_redraw_all();
        w_main.draw(&tft);
    }
};

#include "menu.h"
#include "widgets/bar_horizontal_volume.h"
#include "widgets/label.h"
#include <stdint.h>

WidgetContainer w_home_vu(0, 60, WidgetContainer::VERTICAL, {
    new WidgetHBarVolume(230, 10, 0, 255, 23),
    new WidgetHBarVolume(230, 10, 0, 255, 23),
    new WidgetHBarVolume(230, 10, 0, 255, 23),
    new WidgetHBarVolume(230, 10, 0, 255, 23),
    new WidgetHBarVolume(230, 10, 0, 255, 23),
    new WidgetHBarVolume(230, 10, 0, 255, 23),
}, 2);
WidgetContainer w_home_status(28, 10, WidgetContainer::HORIZONTAL, {
    new WidgetLabel(0, 0, 24, 20, "M"),  // Mute
    new WidgetLabel(0, 0, 24, 20, ""),  // Input Channel
    new WidgetLabel(0, 0, 24, 20, "T"),  // Tone control
    new WidgetLabel(0, 0, 24, 20, "E"),  // Enhanced Surround
    new WidgetLabel(0, 0, 24, 20, "M"),  // Mix
    new WidgetLabel(0, 0, 24, 20, ""),  // Preset
    //new WidgetLabel(0, 0, 70, 24, ""),  // misc
}, 5);

// WidgetContainer w_debug(0, w_home_vu.y + w_home_vu.h, WidgetContainer::VERTICAL, {
//     new WidgetLabel(240, 16, ""),
//     new WidgetLabel(240, 16, ""),
//     new WidgetLabel(240, 16, ""),
//     new WidgetLabel(240, 16, ""),
//     new WidgetLabel(240, 16, ""),
//     new WidgetLabel(240, 16, ""),
// }, 0);

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
 
int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

const String channel_map[] = {"S1", "S2", "S3", "S4", "6c"};
const String preset_map[] = {"P1", "P2", "P3", "P4", "--"};

class Home : public MenuItemBase
{
public:
    Home(std::initializer_list<MenuItemBase*> items) : MenuItemBase(MENU_HOME, items) {
        for (auto i : w_home_vu.widgets) {
            i->set_padding(0);
            i->set_border(0);
        }
        w_home_vu.set_border(0, 0);
        w_home_status.set_border(0, 0);
        w_home_status.set_background_color(WC_DARK_BLUE);
        w_home_status.set_border_color(WC_DARK_BLUE);
        for (auto i : w_home_status.widgets) {
            i->set_border(0, 1);
            i->set_border_color(WC_DARK_BLUE);
            i->set_background_color(WC_DARK_BLUE);
            i->set_content_color(WC_LIGHT_BLUE);
        }
        // for (auto i: w_debug.widgets) {
        //     i->set_border(0,0);
        // }
    }

    virtual void on_handle(unsigned long now) {
        w_home_status.get<WidgetLabel>(0)->set_content_color(settings.master.mute.all ? WC_RED : WC_BLUE);
        w_home_status.get<WidgetLabel>(1)->set_text(channel_map[settings.master.input.channel]);
        w_home_status.get<WidgetLabel>(2)->set_content_color(settings.master.mute.tone ? WC_BLUE : WC_LIGHT_BLUE);
        w_home_status.get<WidgetLabel>(3)->set_content_color(settings.master.input.es ? WC_LIGHT_BLUE : WC_BLUE);
        w_home_status.get<WidgetLabel>(4)->set_content_color(settings.master.input.mixed ? WC_LIGHT_BLUE : WC_BLUE);
        w_home_status.get<WidgetLabel>(5)->set_text(preset_map[min((uint8_t)4, settings.selected_preset)]);

        //w_home_status.get<WidgetLabel>(6)->set_text(String(freeMemory()));
        w_home_vu.get<WidgetHBarVolume>(0)->set_background_color(settings.master.mute.fl ? WC_DARK_RED : WC_BLACK);
        w_home_vu.get<WidgetHBarVolume>(1)->set_background_color(settings.master.mute.fr ? WC_DARK_RED : WC_BLACK);
        w_home_vu.get<WidgetHBarVolume>(2)->set_background_color(settings.master.mute.ct ? WC_DARK_RED : WC_BLACK);
        w_home_vu.get<WidgetHBarVolume>(3)->set_background_color(settings.master.mute.lfe ? WC_DARK_RED : WC_BLACK);
        w_home_vu.get<WidgetHBarVolume>(4)->set_background_color(settings.master.mute.sl ? WC_DARK_RED : WC_BLACK);
        w_home_vu.get<WidgetHBarVolume>(5)->set_background_color(settings.master.mute.sr ? WC_DARK_RED : WC_BLACK);

        w_home_vu.draw(&tft);
        w_home_status.draw(&tft);
        // w_debug.draw(&tft);

        auto left = control_left.getValue();
        auto right = control_right.getValue();

        if (left != 0)
            enter(now, MENU_MASTER_BASS);
        else if (right != 0) 
            enter(now, MENU_MASTER_VOLUME);

        auto rb = control_right.getButton();
        auto lb = control_left.getButton();

        if (rb == ClickEncoder::DoubleClicked)
            enter(now, MENU_MUTE);
        if (rb == ClickEncoder::Held)
            back_light_level = DISPLAY_BL_MIN;
        if (rb == ClickEncoder::Released)
            enter(now, MENU_SLEEP);
        else if (rb == ClickEncoder::Clicked)
            enter(now, MENU_MAIN);
        else if (lb == ClickEncoder::Clicked)
            enter(now, MENU_QUICK);
    }

    virtual void on_enter(unsigned long now) {
        tft.fillScreen(WC_BLACK);
        control_left.getValue();
        control_right.getValue();
        control_left.getButton();
        control_right.getButton();
        w_home_vu.set_redraw_all();
        w_home_status.set_redraw_all();
        tft.fillScreen(WC_BLACK);
    }
};

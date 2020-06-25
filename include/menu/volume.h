#include "menu.h"
#include "widgets/bar_horizontal_signed.h"



class VolumeMaster : public MenuItemBase
{
    WidgetHBar volume;
    WidgetTitleLabel label;
    unsigned long last_now;
public:
    VolumeMaster() 
    : MenuItemBase(MENU_MASTER_VOLUME, {}), 
      volume(0, 240/2 - 50/2, 240, 50, 0, MAX_MASTER_VOLUME),
      label(0, 240/2 - 50/2 - 35, 240, 35, "  Master Volume") {
        volume.set_content_color(WC_GREEN_YELLOW);
    }

    virtual void on_handle(unsigned long now) {
        auto change = control_right.getValue();
        if (change != 0) {
            on_user_input();
            volume.add_value(change);
            settings.master.volume = volume.value;
            last_now = now;
            volume_changed();
        }

        if (control_right.getButton() == ClickEncoder::Clicked || now - last_now > 3000000) {
            leave(now);
        }
        volume.draw(&tft);
    }

    virtual void on_enter(unsigned long now) {
        volume.set_value(settings.master.volume);
        volume.set_redraw_all();
        label.set_redraw_all();
        volume.draw(&tft);
        label.draw(&tft);
        last_now = now;
    }
};


class VolumeBass : public MenuItemBase
{
    WidgetHBar bass;
    WidgetTitleLabel label;
    unsigned long last_now;
public:
    VolumeBass() 
    : MenuItemBase(MENU_MASTER_BASS, {}), 
      bass(0, 240/2 - 50/2, 240, 50, 30, MAX_CHANNEL_TRIM),
      label(0, 240/2 - 50/2 - 35, 240, 35, "   LFE Trim") {
        bass.set_content_color(WC_GREEN_YELLOW);
        label.set_content_color(WC_GOLD);
        label.set_background_color(WC_DARK_BLUE);
    }

    virtual void on_handle(unsigned long now) {
        auto change = control_left.getValue();
        if (change != 0) {
            on_user_input();
            bass.add_value(change);
            settings.master.channels[3] = bass.value;
            channel_trim_changed();
            last_now = now;
        }

        if (control_left.getButton() == ClickEncoder::Clicked || now - last_now > 3000000) {
            leave(now);
        }
        bass.draw(&tft);
    }

    virtual void on_enter(unsigned long now) {
        bass.set_value(settings.master.channels[3]);
        bass.set_redraw_all();
        label.set_redraw_all();
        bass.draw(&tft);
        label.draw(&tft);
        last_now = now;
    }
};

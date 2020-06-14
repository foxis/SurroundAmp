#if !defined(MY_WIDGET_BAR_VOLUME_H)
#define MY_WIDGET_BAR_VOLUME_H

#include "widget.h"
#include "bar_horizontal.h"


class WidgetHBarVolume: public WidgetHBar {
    uint8_t bars;
public:
    WidgetHBarVolume(SIZE_DECL, uint8_t value, uint8_t max, uint8_t bars = 12) : WidgetHBarVolume(0, 0, w, h, value, max) {}
    WidgetHBarVolume(RECT_DECL, uint8_t value, uint8_t max, uint8_t bars = 12) : WidgetHBar(x, y, w, h, value, max) {
        this->bars = bars;
        set_value(value);
    }

    virtual uint8_t calculate_screen_value(uint8_t val) const {
        return (uint8_t)(((uint16_t)(val + bars) * (uint16_t)(bars)) / (uint16_t)maximum);
    }

    virtual void draw_background(GRAPHICS_CLASS* gfx) {
        WidgetHBar::draw_background(gfx);
        uint8_t x, y, w, h;
        get_client_area(&x, &y, &w, &h);
        uint8_t bs = w / bars;

        for (int i = 0; i < bars - 1; i++)
            gfx->fillRect(x + bs - 3 + i * bs, y, 3, h, this->bg_color);
    }

    uint16_t get_color(uint8_t val) {        
        uint8_t green = min((uint16_t)(((uint16_t)(val << 1) << 8) / bars), (uint16_t)255);
        uint8_t red = (10 * bars) / 11;
        uint16_t light = alphaBlend(green, WC_YELLOW, WC_GREEN);
        if (val > red) {
            uint16_t remainder = bars - red;
            red = ((uint16_t)(val - red) * 255) / remainder;
            light = alphaBlend(red, WC_RED, light);
        }

        return screen_current >= val ? light : alphaBlend(102, light, WC_BLACK);
    }

    inline void draw_bars(GRAPHICS_CLASS* gfx, int min, int max) {
        uint8_t x, y, w, h;
        get_client_area(&x, &y, &w, &h);
        uint8_t bs = w / bars;

        for (int i = min; i < max; i++)
            gfx->fillRect(x + i * bs, y, bs - 3, h, get_color(i + 1));
    }

    virtual void draw_contents(GRAPHICS_CLASS* gfx) {
        if (is_content_redraw()) {
            draw_bars(gfx, 0, bars);
        } else if (this->screen_current != this->screen_prev) {
            auto a = (this->screen_current-1);
            auto b = (this->screen_prev-1);
            draw_bars(gfx, min(a, b), max(a, b)+1);
        }
    }
};

#endif
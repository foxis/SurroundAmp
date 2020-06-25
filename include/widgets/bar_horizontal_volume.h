#if !defined(MY_WIDGET_BAR_VOLUME_H)
#define MY_WIDGET_BAR_VOLUME_H

#include <math.h>
#include "widget.h"
#include "bar_horizontal.h"


class WidgetHBarVolume: public WidgetHBar {
    uint8_t bars;
public:
    WidgetHBarVolume(SIZE_DECL, uint8_t value, uint8_t max, uint8_t bars = 12) : WidgetHBarVolume(0, 0, w, h, value, max, bars) {}
    WidgetHBarVolume(RECT_DECL, uint8_t value, uint8_t max, uint8_t bars = 12) : WidgetHBar(x, y, w, h, value, max) {
        this->bars = bars;
        set_value(value);
    }

    virtual uint8_t calculate_screen_value(uint8_t val) const {
        float norm = val / (float)maximum;
        uint8_t bar = ceil(norm * bars);
        return bar;
    }

    virtual void draw_background(GRAPHICS_CLASS* gfx) {
        WidgetHBar::draw_background(gfx);
        uint8_t x, y, w, h;
        get_client_area(&x, &y, &w, &h);
        uint8_t bs = w / bars;

        for (int i = 0; i < bars; i++)
            gfx->fillRect(x + bs - 3 + i * bs, y, 3, h, this->bg_color);
    }

    uint16_t get_color(uint8_t val) {        
        uint8_t green = min((uint16_t)(((uint16_t)(val << 1) << 8) / bars), (uint16_t)255);
        uint16_t light = alphaBlend(green, WC_YELLOW, WC_GREEN);
        uint8_t red = (2 * bars) / 3;
        if (val >= red) {
            uint16_t remainder = bars - red - 1;
            red = ((uint16_t)(val - red) * 255) / remainder;
            light = alphaBlend(red, WC_RED, light);
        }

        return value > (((uint16_t)val * maximum) / bars) ? light : alphaBlend(102, light, WC_BLACK);
    }

    inline void draw_bars(GRAPHICS_CLASS* gfx, uint8_t min, uint8_t max) {
        uint8_t x, y, w, h;
        get_client_area(&x, &y, &w, &h);
        uint8_t bs = w / bars;

        for (uint8_t i = min; i <= max; i++)
            gfx->fillRect(x + i * bs, y, bs - 3, h, get_color(i));
    }

    virtual void draw_contents(GRAPHICS_CLASS* gfx) {
        if (is_content_redraw()) {
            draw_bars(gfx, 0, bars - 1);
        } else if (this->screen_current != this->screen_prev) {
            auto a = this->screen_current;
            auto b = this->screen_prev;
            draw_bars(gfx, max((int8_t)0, (int8_t)(min(a, b) - 1)), max(a, b) - 1);
        }
    }
};

#endif
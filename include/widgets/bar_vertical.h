#if !defined(MY_WIDGET_BAR_VERTICAL_H)
#define MY_WIDGET_BAR_VERTICAL_H

#include "widget.h"
#include "bar_horizontal.h"

class WidgetVBar: public WidgetHBar {
public:
    WidgetVBar(uint8_t w, uint8_t h, uint8_t value, uint8_t max = 100) : WidgetHBar(0, 0, w, h, value, max) {}
    WidgetVBar(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t value, uint8_t max = 100) : WidgetHBar(x, y, w, h, value, max) {}

    virtual uint8_t calculate_screen_value(uint8_t val) const {
        uint8_t h = get_client_height();
        return (uint8_t)((uint16_t)val * (uint16_t)h / (uint16_t)maximum);
    }

    virtual void draw_contents(GRAPHICS_CLASS* gfx) {
        uint8_t x, y, w, h;
        get_client_area(&x, &y, &w, &h);
        if (is_content_redraw()) {
            gfx->fillRect(x, y, w, this->screen_current, this->bg_color);
            gfx->fillRect(x, y + this->screen_current, w, h - this->screen_current, this->fg_color);
        } else if (this->screen_current > this->screen_prev) {
            gfx->fillRect(x, y + this->screen_prev, w, this->screen_current - this->screen_prev, this->bg_color);
        } else if (this->screen_current < this->screen_prev) {
            gfx->fillRect(x, y + this->screen_current, w, this->screen_prev - this->screen_current, this->fg_color);
        }
    }
};

#endif
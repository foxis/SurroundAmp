#if !defined(MY_WIDGET_BAR_HORIZONTAL_H)
#define MY_WIDGET_BAR_HORIZONTAL_H

#include "widget.h"
#include "bar.h"


class WidgetHBar: public WidgetBarBase<uint8_t, int8_t> {
public:
    uint8_t maximum;
public:
    WidgetHBar(SIZE_DECL, uint8_t value, uint8_t max = 100) : WidgetHBar(0, 0, w, h, value, max) {}
    WidgetHBar(RECT_DECL, uint8_t value, uint8_t max = 100) : WidgetBarBase(x, y, w, h, value) {
        this->maximum = max;
        set_value(value);
    }

    virtual uint8_t calculate_screen_value(uint8_t val) const {
        uint8_t w = get_client_width();
        return (uint8_t)(((uint16_t)val * (uint16_t)w) / (uint16_t)maximum);
    }
    virtual uint8_t add(uint8_t val, int8_t val1) const { 
        int16_t tmp = (int16_t)val + val1;
        return (uint8_t)min((int16_t)maximum, max(tmp, (int16_t)0)); 
    }

    virtual void draw_contents(GRAPHICS_CLASS* gfx) {
        uint8_t x, y, w, h;
        get_client_area(&x, &y, &w, &h);
        if (is_content_redraw()) {
            gfx->fillRect(x, y, this->screen_current, h, this->fg_color);
            gfx->fillRect(x + this->screen_current, y, w - this->screen_current, h, this->bg_color);
        } else if (this->screen_current > this->screen_prev) {
            gfx->fillRect(x + this->screen_prev, y, this->screen_current - this->screen_prev, h, this->fg_color);
        } else if (this->screen_current < this->screen_prev) {
            gfx->fillRect(x + this->screen_current, y, this->screen_prev - this->screen_current, h, this->bg_color);
        }
    }
};

#endif
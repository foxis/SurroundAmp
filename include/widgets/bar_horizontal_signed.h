#if !defined(MY_WIDGET_BAR_SIGNED_H)
#define MY_WIDGET_BAR_SIGNED_H

#include "widget.h"
#include "bar.h"

class WidgetHBarSigned: public WidgetBarBase<int16_t, int16_t> {
public:
    int16_t maximum;
    int16_t minimum;
    uint16_t fg_color_minus;
public:
    WidgetHBarSigned(SIZE_DECL, int16_t value, int16_t min, int16_t max) : WidgetHBarSigned(0, 0, w, h, value, min, max) {}
    WidgetHBarSigned(RECT_DECL, int16_t value, int16_t min, int16_t max) : WidgetBarBase(x, y, w, h, value) {
        this->maximum = max;
        this->minimum = min;
        fg_color_minus = WC_BLUE;
        set_value(value);
    }

    void set_content_color(uint16_t fg, uint16_t fg_minus) {
        fg_color_minus = fg_minus;
        Widget::set_content_color(fg);
    }

    virtual uint8_t calculate_screen_value(int16_t val) const {
        int16_t w = get_client_width();
        return (uint8_t)((val * w) / (maximum - minimum) + (w >> 1));
    }
    virtual int16_t add(int16_t val, int16_t val1) const { 
        int16_t tmp = val + val1;
        return min(maximum, max(tmp, minimum)); 
    }

    uint16_t get_color() { return this->value <= 0 ? this->fg_color_minus : this->fg_color; }

    virtual void draw_contents(GRAPHICS_CLASS* gfx) {
        uint8_t x, y, w, h;
        get_client_area(&x, &y, &w, &h);
        uint8_t w2 = w >> 1;
        
        if (is_content_redraw()) {
             gfx->fillRect(x + min(w2, this->screen_current), y, abs((int16_t)w2 - (int16_t)this->screen_current), h, get_color());
             gfx->fillRect(x, y, min(w2, this->screen_current), h, this->bg_color);
             gfx->fillRect(x + max(w2, this->screen_current), y, w - max(w2, this->screen_current), h, this->bg_color);
        } else if (this->screen_current > this->screen_prev) {
            gfx->fillRect(x + this->screen_prev, y, this->screen_current - this->screen_prev, h, this->screen_prev >= w2 ? get_color() : this->bg_color);
        } else if (this->screen_current < this->screen_prev) {
            gfx->fillRect(x + this->screen_current, y, this->screen_prev - this->screen_current, h, this->screen_current >= w2 ? this->bg_color : get_color());
        }
    }
};

#endif
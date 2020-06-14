#if !defined(MY_WIDGET_BAR_BASE_H)
#define MY_WIDGET_BAR_BASE_H

#include "widget.h"

template<class T, class T1>
class WidgetBarBase: public Widget {
public:
    T value;
    uint8_t screen_prev, screen_current;
public:
    WidgetBarBase(RECT_DECL, T value,
        uint16_t border_color = DEFAULT_BORDER_COLOR, uint16_t bg_color = DEFAULT_BACKGROUND_COLOR, uint16_t fg_color = DEFAULT_CONTENT_COLOR,
        uint8_t border_width = DEFAULT_BORDER_WIDTH, uint8_t padding = DEFAULT_PADDING) : Widget(x, y, w, h, border_color, bg_color, fg_color, border_width, padding) {
        screen_prev = screen_current = 0;
        this->value = value;
    }

    virtual uint8_t calculate_screen_value(T val) const = 0;
    virtual T add(T val, T1 val1) const { return val + val1; }

    void set_value(T val) {
        value = val = add(val, 0);
        auto tmp = calculate_screen_value(val);
        if (tmp != this->screen_current) {
            this->screen_prev = screen_current;
            this->screen_current = tmp;
            set_content_changed();
        }
    }
    void add_value(T1 val) {
        T v = add(this->value, val);
        set_value(v);
    }

    virtual void draw_background(GRAPHICS_CLASS* gfx) {
        uint8_t bw = get_border();
        uint8_t pw = get_padding();
        for (uint8_t i = 0; i < pw; i++)
            gfx->drawRect(
                this->x + i + bw, this->y + i + bw, 
                this->w - i * 2 - bw * 2, this->h - i * 2 - bw * 2, 
                this->bg_color);
    }
};

#endif
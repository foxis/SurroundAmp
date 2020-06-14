#if !defined(MY_WIDGET_LABEL_H)
#define MY_WIDGET_LABEL_H

#include "widget.h"

class WidgetLabel: public Widget {
public:
    String current;
    bool resize;

public:
    WidgetLabel(const String& current) : WidgetLabel(0, 0, 0, 0, current, true) {}
    WidgetLabel(uint8_t w, uint8_t h, const String& current) : WidgetLabel(0, 0, w, h, current, true) {}
    WidgetLabel(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const String& current, bool resize=false) : Widget(x, y, w, h) {
        this->resize = resize;
        fg_color = WC_WHITE; 
        bg_color = WC_BLACK;
        border_color = WC_BLACK;
        set_border(2, 2); 
        set_text(current);
    }

    void printf(const char *format, ...) {
        va_list arg;
        va_start(arg, format);
        char temp[64];
        vsnprintf(temp, sizeof(temp), format, arg);
        va_end(arg);
        set_text(temp);
    }

    void set_text(const String& val) {
        if (val != current) {
            current = val;
            set_background_changed(true);
            set_content_changed(true);
        }
    }
    virtual void draw_contents(GRAPHICS_CLASS* gfx) {
        #if defined(GET_TEXT_BOUNDS)
        int16_t x, y;
        uint16_t w;
        #endif
        uint16_t h;

        uint8_t bw = this->get_border();
        uint8_t pw = this->get_padding();
        #if defined(GET_TEXT_BOUNDS)
        gfx->getTextBounds(this->current, (int16_t)this->x, (int16_t)this->y, &x, &y, &w, &h);
        #else
        h = 0;
        #endif
        gfx->setCursor(this->x + bw + pw, this->y + bw + pw + h);
        gfx->setTextColor(this->fg_color);
        gfx->print(this->current);

        #if defined(GET_TEXT_BOUNDS)
        w +=  (pw + bw) * 2;
        h +=  (pw + bw) * 2;
        if ((w > this->w || h > this->h) && this->resize) {
            this->w = w;
            this->h = h;
            set_border_changed(true);
            set_background_changed(true);
            set_content_changed(true);
        }
        #endif
    }
};

class WidgetTitleLabel : public WidgetLabel {
public:
    WidgetTitleLabel(const String& current) : WidgetTitleLabel(0, 0, 0, 0, current, true) {}
    WidgetTitleLabel(uint8_t w, uint8_t h, const String& current) : WidgetTitleLabel(0, 0, w, h, current, true) {}
    WidgetTitleLabel(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const String& current, bool resize=false) : WidgetLabel(x, y, w, h, current, resize) {
        bg_color = WC_DARK_BLUE;
        fg_color = WC_GOLD;
        set_border(0, 4);
    }

    virtual void draw_contents(GRAPHICS_CLASS* gfx) {
        gfx->setTextSize(2);
        WidgetLabel::draw_contents(gfx);
        gfx->setTextSize(1);
    }
};

#endif
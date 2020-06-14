#if !defined(MY_WIDGET_H)
#define MY_WIDGET_H

#include <Arduino.h>
#include <vector>
#include <initializer_list>

#if defined(_ADAFRUIT_GFX_H)
#define GRAPHICS_CLASS Adafruit_GFX
#define GET_TEXT_BOUNDS
#elif defined(_TFT_eSPIH_)
#define GRAPHICS_CLASS TFT_eSPI
#else
#error Either Adafruit_GFX or TFT_eSPI must be included prior
#endif

#define BORDER_WIDTH_MASK 0x03
#define PADDING_WIDTH_MASK (0x03<<2)
#define BORDER_CHANGED 0x80
#define BACKGROUND_CHANGED 0x40 
#define CONTENT_REDRAW 0x20
#define CONTENT_CHANGED 0x10

#define RGB_TO_RGB565(r, g, b) (((((uint16_t)(r)) >> 3) << 11) | ((((uint16_t)(g))>>2) << 5) | (((uint16_t)(b))>>3)) 
// Some ready-made 16-bit ('565') color settings:
#define WC_BLACK 0x0000
#define WC_DARK_GRAY RGB_TO_RGB565(64, 64, 64)
#define WC_GRAY RGB_TO_RGB565(128, 128, 128)
#define WC_LIGHT_GRAY RGB_TO_RGB565(192, 192, 192)
#define WC_WHITE 0xFFFF
#define WC_DARK_RED RGB_TO_RGB565(102, 0, 0)
#define WC_LIGHT_RED RGB_TO_RGB565(255, 153, 153)
#define WC_RED 0xF800
#define WC_DARK_GREEN RGB_TO_RGB565(0, 102, 0)
#define WC_LIGHT_GREEN RGB_TO_RGB565(153, 255, 153)
#define WC_GREEN 0x07E0
#define WC_DARK_BLUE RGB_TO_RGB565(0, 0, 102)
#define WC_LIGHT_BLUE RGB_TO_RGB565(153, 153, 255)
#define WC_BLUE 0x001F
#define WC_CYAN 0x07FF
#define WC_MAGENTA 0xF81F
#define WC_DARK_YELLOW RGB_TO_RGB565(102, 102, 0)
#define WC_LIGHT_YELLOW RGB_TO_RGB565(255, 255, 153)
#define WC_YELLOW 0xFFE0
#define WC_ORANGE 0xFC00
#define WC_NAVY        0x000F      /*   0,   0, 128 */
#define WC_DARK_CYAN    0x03EF      /*   0, 128, 128 */
#define WC_MAROON      0x7800      /* 128,   0,   0 */
#define WC_PURPLE      0x780F      /* 128,   0, 128 */
#define WC_OLIVE       0x7BE0      /* 128, 128,   0 */
#define WC_DARK_GREEN_YELLOW RGB_TO_RGB565(64, 102, 0)
#define WC_GREEN_YELLOW 0xB7E0      /* 180, 255,   0 */
#define WC_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F      
#define WC_BROWN       0x9A60      /* 150,  75,   0 */
#define WC_GOLD        0xFEA0      /* 255, 215,   0 */
#define WC_SILVER      0xC618      /* 192, 192, 192 */
#define WC_SKY_BLUE     0x867D      /* 135, 206, 235 */
#define WC_VIOLET      0x915C      /* 180,  46, 226 */

#if !defined(DEFAULT_BORDER_COLOR)
#define DEFAULT_BORDER_COLOR WC_SILVER
#endif
#if !defined(DEFAULT_BACKGROUND_COLOR)
#define DEFAULT_BACKGROUND_COLOR WC_BLACK
#endif
#if !defined(DEFAULT_CONTENT_COLOR)
#define DEFAULT_CONTENT_COLOR WC_SILVER
#endif
#if !defined(DEFAULT_BORDER_WIDTH)
#define DEFAULT_BORDER_WIDTH 1
#endif
#if !defined(DEFAULT_PADDING)
#define DEFAULT_PADDING 1
#endif
#if !defined(DEFAULT_MARGIN)
#define DEFAULT_MARGIN 0
#endif

#define RECT_DECL uint8_t x, uint8_t y, uint8_t w, uint8_t h
#define SIZE_DECL uint8_t w, uint8_t h
#define RECT_PARAM x, y, w, h

class Widget {
public:
    uint8_t x, y, w, h;

protected:
    uint16_t border_color;
    uint16_t bg_color;
    uint16_t fg_color;
    uint8_t config;

public:
    Widget(RECT_DECL,
        uint16_t border_color = DEFAULT_BORDER_COLOR, uint16_t bg_color = DEFAULT_BACKGROUND_COLOR, uint16_t fg_color = DEFAULT_CONTENT_COLOR,
        uint8_t border_width = DEFAULT_BORDER_WIDTH, uint8_t padding = DEFAULT_PADDING) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        this->border_color = border_color;
        this->bg_color = bg_color;
        this->fg_color = fg_color;
        this->config = (border_width & BORDER_WIDTH_MASK) | ((padding << 2) & PADDING_WIDTH_MASK) | BORDER_CHANGED | BACKGROUND_CHANGED | CONTENT_REDRAW;
    }

    inline uint8_t get_border() const { return config & BORDER_WIDTH_MASK; }
    inline uint8_t get_padding() const { return (config & PADDING_WIDTH_MASK) >> 2; }
    inline uint8_t get_client_width() const { 
        uint8_t bw = get_border();
        uint8_t pw = get_padding();
        return this->w - (bw + pw) * 2; 
    }
    inline uint8_t get_client_height() const { 
        uint8_t bw = get_border();
        uint8_t pw = get_padding();
        return this->h - (bw + pw) * 2; 
    }
    inline void get_client_area(uint8_t *x, uint8_t *y, uint8_t *w, uint8_t *h) const {
        uint8_t bw = get_border();
        uint8_t pw = get_padding();
        *x = this->x + bw + pw;
        *y = this->y + bw + pw;
        *w = this->w - (bw + pw) * 2;
        *h = this->h - (bw + pw) * 2;
    }

    inline bool is_border_changed() const { return config & BORDER_CHANGED; }
    inline bool is_background_changed() const { return config & BACKGROUND_CHANGED; }
    inline bool is_content_changed() const { return config & (CONTENT_CHANGED | CONTENT_REDRAW); }
    inline bool is_content_redraw() const { return config & CONTENT_REDRAW; }

    inline void set_border_changed(bool val=true) { config = (config & (~BORDER_CHANGED)) | (val?BORDER_CHANGED:0); }
    inline void set_background_changed(bool val=true) { config = (config & (~BACKGROUND_CHANGED)) | (val?BACKGROUND_CHANGED:0); }
    inline void set_content_changed(bool val=true) { config = (config & (~CONTENT_CHANGED)) | (val?CONTENT_CHANGED:0); }
    inline void set_content_redraw(bool val=true) { config = (config & (~CONTENT_REDRAW)) | (val?CONTENT_REDRAW:0); }
    inline void set_redraw_all(bool val=true) {
        set_border_changed(val);
        set_background_changed(val);
        set_content_changed(val);
        set_content_redraw(val);
    }
    virtual void set_pos(uint8_t x, uint8_t y) {
        if (this->x != x || this->y != y) {
            set_border_changed();
            set_background_changed();
            set_content_changed();
            set_content_redraw();
        }
        this->x = x;
        this->y = y;
    }
    void set_size(uint8_t w, uint8_t h) {
        if (this->w != w || this->h != h)
            set_redraw_all();
        this->w = w;
        this->h = h;
    }

    void set_border_color(uint16_t col) {
        if (border_color != col)
            set_border_changed(true);
        border_color = col;
    }
    void set_background_color(uint16_t col) {
        if (bg_color != col) {
            set_background_changed(true);
            set_content_redraw(true);
        }
        bg_color = col;
    }
    void set_content_color(uint16_t col) {
        if (fg_color != col)
            set_content_redraw(true);
        fg_color = col;
    }
    void set_border(uint8_t w) {
        this->config = (this->config & (~BORDER_WIDTH_MASK)) | (w & BORDER_WIDTH_MASK);
        set_border_changed(true);
        set_background_changed(true);
        set_content_redraw(true);
    }
    void set_border(uint8_t w, uint8_t padding) {
        set_border(w);
        set_padding(padding);
    }
    void set_padding(uint8_t w) {
        this->config = (this->config & (~PADDING_WIDTH_MASK)) | ((w<<2) & PADDING_WIDTH_MASK);
        set_background_changed(true);
        set_content_redraw(true);
    }

    virtual void _draw(GRAPHICS_CLASS* gfx) {
        if (is_border_changed()) {
            set_border_changed(false);
            draw_border(gfx);
        }
        if (is_background_changed()) {
            set_background_changed(false);
            draw_background(gfx);
        }
        if (is_content_changed()) {
            set_content_changed(false);
            draw_contents(gfx);
            if (!(config & CONTENT_CHANGED))
                set_content_redraw(false);
        }
    }

    virtual void draw(GRAPHICS_CLASS* gfx) {
        while (is_border_changed() || is_background_changed() || is_content_changed()) {
            _draw(gfx);
        }
    }

    virtual void draw_border(GRAPHICS_CLASS* gfx) {
        uint8_t bw = get_border();
        for (uint8_t i = 0; i < bw; i++)
            gfx->drawRect(this->x + i, this->y + i, 
                this->w - i * 2, this->h - i * 2, 
                this->border_color);
    }
    virtual void draw_background(GRAPHICS_CLASS* gfx) {
        uint8_t bw = get_border();
        uint8_t bw2 = bw * 2;
        gfx->fillRect(this->x + bw, this->y + bw, 
            this->w - bw2, this->h - bw2, 
            this->bg_color);
    }
    virtual void draw_contents(GRAPHICS_CLASS* gfx) = 0;

    // method taken from TFT_eSPI so it could be used together with Adafruit_GFX
    // alpha =   0 = 100% background colour
    // alpha = 255 = 100% foreground colour
    inline uint16_t alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc)
    {
        // For speed use fixed point maths and rounding to permit a power of 2 division
        uint16_t fgR = ((fgc >> 10) & 0x3E) + 1;
        uint16_t fgG = ((fgc >>  4) & 0x7E) + 1;
        uint16_t fgB = ((fgc <<  1) & 0x3E) + 1;

        uint16_t bgR = ((bgc >> 10) & 0x3E) + 1;
        uint16_t bgG = ((bgc >>  4) & 0x7E) + 1;
        uint16_t bgB = ((bgc <<  1) & 0x3E) + 1;

        // Shift right 1 to drop rounding bit and shift right 8 to divide by 256
        uint16_t r = (((fgR * alpha) + (bgR * (255 - alpha))) >> 9);
        uint16_t g = (((fgG * alpha) + (bgG * (255 - alpha))) >> 9);
        uint16_t b = (((fgB * alpha) + (bgB * (255 - alpha))) >> 9);

        // Combine RGB565 colours into 16 bits
        //return ((r&0x18) << 11) | ((g&0x30) << 5) | ((b&0x18) << 0); // 2 bit greyscale
        //return ((r&0x1E) << 11) | ((g&0x3C) << 5) | ((b&0x1E) << 0); // 4 bit greyscale
        return (r << 11) | (g << 5) | (b << 0);
    }    
};


class WidgetContainer: public Widget {
public:
    typedef enum {
        NONE,
        HORIZONTAL,
        VERTICAL,
        RELATIVE,
    } Arrange_enum;

    uint8_t margin;
    Arrange_enum arrange;
    std::vector<Widget*> widgets;
public:
    WidgetContainer(Arrange_enum arrange, std::initializer_list<Widget*> widgets, uint8_t margin=DEFAULT_MARGIN) : WidgetContainer(0, 0, arrange, widgets, margin) {}
    WidgetContainer(uint8_t x, uint8_t y, Arrange_enum arrange, std::initializer_list<Widget*> widgets, uint8_t margin=DEFAULT_MARGIN)
        : WidgetContainer(x, y, arrange, margin) {
        this->widgets.reserve(widgets.size());
        for (auto i : widgets) 
            add(i);
    }
    WidgetContainer(uint8_t x, uint8_t y, Arrange_enum arrange=NONE, uint8_t margin=2) : Widget(x, y, 10, 10) {
        this->margin = margin;
        this->arrange = arrange;
    }

    template<class T>
    T* get(uint8_t i) const {
        return (T*)widgets[i];
    }
    Widget* operator[] (uint8_t i) const {
        return widgets[i];
    }

    virtual void set_pos(uint8_t x, uint8_t y) {
        int16_t dx = (int16_t)x - (int16_t)this->x;
        int16_t dy = (int16_t)y - (int16_t)this->y;
        for (auto i : widgets) {
            int8_t nx = i->x + dx;
            int8_t ny = i->y + dy;
            i->set_pos(nx, ny);
        }
        Widget::set_pos(x, y);
    }


    void add(Widget* widget) {
        uint8_t x, y, w, h;
        get_client_area(&x, &y, &w, &h);

        switch(arrange) {
            case HORIZONTAL:
                x += w + margin;
                y += widget->y;
                w += widget->w + margin;
                h = max(h, (uint8_t)(widget->y + widget->h));
                break;
            case VERTICAL:
                x += widget->x;
                y += h + margin;
                h += widget->h + margin;
                w = max(w, (uint8_t)(widget->x + widget->w));
                break;
            case RELATIVE:
                x += widget->x;
                y += widget->y;
                w = max(w, (uint8_t)(widget->x + widget->w));
                h = max(h, (uint8_t)(widget->y + widget->h));
                break;
            case NONE:
                x = widget->x;
                y = widget->y;
        }
        widget->set_pos(x, y);
        this->set_size(w + (get_border() + get_padding()) * 2, h + (get_border() + get_padding()) * 2);
        widgets.push_back(widget);
    }

    virtual void draw(GRAPHICS_CLASS* gfx) {
        if (is_border_changed()) {
            set_border_changed(false);
            draw_border(gfx);
        }
        auto bc = is_background_changed();
        if (bc) {
            set_background_changed(false);
            draw_background(gfx);
        }
        for (auto i: widgets) {
            if (bc)
                i->set_redraw_all();
            i->draw(gfx);
        }
    }

    virtual void draw_contents(GRAPHICS_CLASS* gfx) {
    }
};

class WidgetSeparator : public Widget {
public:
    WidgetSeparator(uint8_t w, uint8_t h=5) : Widget(0, 0, w, h) {
        set_border(0);
        set_padding(0);
    }

    virtual void draw_contents(GRAPHICS_CLASS* gfx) {}
};

#endif
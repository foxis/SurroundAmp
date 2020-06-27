#if !defined(MY_MENU_H)
#define MY_MENU_H

#include <Arduino.h>
#include <vector>
#include <initializer_list>
#include <ClickEncoder.h>
#include "pins.h"
#include "settings.h"
#include "display.h"
#include "soundctrl.h"
#include "widgets/widget.h"


ClickEncoder control_left(BUTTON_LEFT_A, BUTTON_LEFT_B, BUTTON_LEFT_C);
ClickEncoder control_right(BUTTON_RIGHT_A, BUTTON_RIGHT_B, BUTTON_RIGHT_C);


unsigned long last_user_input = 0;
unsigned long last_activity = 0;
unsigned long back_light_level = DISPLAY_BL_DEFAULT;


void on_user_input() {
  last_activity = last_user_input = millis();
  if (back_light_level != DISPLAY_BL_MAX) {
    back_light_level = DISPLAY_BL_MAX;
  }
}

typedef enum {
    MENU_HOME = 1,
    MENU_MASTER_VOLUME,
    MENU_MASTER_BASS,
    MENU_INPUT_SELECT,
    MENU_QUICK, 
    MENU_MAIN,
    MENU_MUTE,
    MENU_SLEEP,
    MENU_NONE = 255,
} menu_name_t;

typedef std::function<void()> on_changed_func;

class MenuItemBase {
    MenuItemBase() {}
public:
    bool is_active;
    bool is_selected;

    uint8_t name;
    MenuItemBase *parent;
    std::vector<MenuItemBase*> items;
    std::vector<MenuItemBase*>::iterator current;

    MenuItemBase(menu_name_t name, std::initializer_list<MenuItemBase*> items) {
        this->parent = NULL;
        this->name = name;
        this->items.reserve(items.size());
        for (auto i : items) {
            this->items.push_back(i);
        }
        this->current = this->items.begin();
        is_active = false;
        is_selected = false;
    }

    void select(unsigned long now, bool val=true) {
        auto tmp = is_selected;
        is_selected = val;
        if (val == true && tmp == false)
            on_select(now);
        else if (val == false && tmp == true)
            on_deselect(now);
        on_user_input();
    }
    void activate(unsigned long now, bool val=true) {
        auto tmp = is_active;
        is_active = val;
        if (val == true && tmp == false) {
            on_enter(now);
        } else if (val == false && tmp == true) {
            on_leave(now);
        }
        on_user_input();
    }

    MenuItemBase * find(menu_name_t name) {
        if (name == MENU_HOME && this->parent == NULL)
            return this;
        if (this->name == name)
            return this;
        for (auto i : items) {
            auto item = i->find(name);
            if (item)
                return item;
        }
        return NULL;
    }
    inline void begin(unsigned long now) {
        enter(now, NULL);        
    }
    void enter(unsigned long now) {
        if (items.size() && current != items.end())
            (*current)->enter(now, this); 
    }
    void enter(unsigned long now, menu_name_t name) {
        auto item = this;
        while (item->parent)
            item = item->parent;
        item = item->find(name);
        if (item) {
            item->enter(now, this);
        }
    }
    void enter(unsigned long now, MenuItemBase* parent) {
        current = items.begin();
        this->parent = parent;
        if (parent)
            parent->activate(now, false);
        activate(now, true);
        if (items.size()) 
            (*current)->select(now, true);
    }
    void leave(unsigned long now) {
        if (this->parent == NULL) return;
        if (items.size()) 
            (*current)->select(now, false);
        activate(now, false);
        this->parent->activate(now, true);
    }
    void back(unsigned long now) {
        if (this->parent == NULL) return;
        if (items.size()) 
            (*current)->select(now, false);
        activate(now, false);
        this->parent->leave(now);
    }

    void prev(unsigned long now) {
        if (items.size() == 0) return;
        if (current == items.begin()) return;
        (*current)->select(now, false);
        (*(--current))->select(now, true);
    }
    void next(unsigned long now) {
        if (items.size() == 0) return;
        if (current + 1 == items.end()) return;
        (*current)->select(now, false);
        (*(++current))->select(now, true);
    }
    void reset(unsigned long now) {
        auto item = this;
        while (item) {
            item->activate(now, item->parent == NULL);
            if (item->items.size()) 
                (*item->current)->select(now, false);
            item = item->parent;
        }
    }

    virtual void on_enter(unsigned long now) {}
    virtual void on_leave(unsigned long now) {}
    virtual void on_select(unsigned long now) {}
    virtual void on_deselect(unsigned long now) {}
    virtual void on_handle(unsigned long now) {}

    void loop(unsigned long now) {
        if (is_active)
            on_handle(now);
        else {
            for (auto i: items) {
                i->loop(now);
            }
        }
    }
};


template<class T>
class WidgetMenuItem : public MenuItemBase {
public:
    T* widget;
    unsigned long last_now;

    WidgetMenuItem(menu_name_t name, T * widget) : WidgetMenuItem(name, widget, {}) {}
    WidgetMenuItem(menu_name_t name, T * widget, std::initializer_list<MenuItemBase*> items) : MenuItemBase(name, items) { 
        this->widget=widget; 
        if (widget) {
            // default color is reserved for not implemented menu items
            // Note though, that some widgets may be reused
            widget->set_content_color(WC_WHITE); 
            widget->set_border_color(WC_BLACK);
        }
    }

    virtual void on_select(unsigned long now) { if (widget) { widget->set_border_color(WC_SILVER); widget->draw(&tft); } }
    virtual void on_deselect(unsigned long now) { if (widget) { widget->set_border_color(WC_BLACK); widget->draw(&tft); } }
    virtual void on_enter(unsigned long now) { leave(now); }
    virtual void on_handle(unsigned long now) { 
        if (now - last_now > 13000000)
            reset(now);
    }
};

class WidgetBackMenuItem : public WidgetMenuItem<Widget> {
public:
    WidgetBackMenuItem(Widget * widget) : WidgetMenuItem<Widget>(MENU_NONE, widget, {}) {}

    virtual void on_enter(unsigned long now) { 
        back(now); 
    }
};

template<class T = Widget>
class RotaryEncoderMenu : public WidgetMenuItem<T> {
public:
    RotaryEncoderMenu(menu_name_t name, T * widget, std::initializer_list<MenuItemBase*> items) : WidgetMenuItem<T>(name, widget, items) {
    }

    virtual void on_handle(unsigned long now) {
        auto change = control_left.getValue() + control_right.getValue();
        auto btn = control_left.getButton();
        auto btnr = control_right.getButton();

        if (change > 0) 
            this->next(now);
        else if (change < 0)
            this->prev(now);

        if (change != 0) {
            on_user_input();
            this->last_now = now;
        }

        if (btn == ClickEncoder::Held || btnr == ClickEncoder::Held) {
            this->leave(now);
        } else if (btn == ClickEncoder::Clicked || btnr == ClickEncoder::Clicked) {
            this->enter(now);
        }

        WidgetMenuItem<T>::on_handle(now);
    }
    virtual void on_enter(unsigned long now) {
        this->last_now = now;
        control_left.getValue();
        control_right.getValue();
        control_left.getButton();
        control_right.getButton();
    }
};

template<class T, class T1>
class HBarMenuItem : public WidgetMenuItem<WidgetContainer> {
    T * p_value;
    on_changed_func on_changed;
public:
    HBarMenuItem(WidgetContainer* widget, T * p_value, on_changed_func on_changed=NULL) : WidgetMenuItem<WidgetContainer>(MENU_NONE, widget) {
        this->p_value = p_value;
        this->on_changed = on_changed;
        for (auto i : widget->widgets) {
            i->set_border(0);
            i->set_content_color(WC_WHITE);
        }
        (*widget)[1]->set_border(1);
    }
    virtual void on_handle(unsigned long now) { 
        auto change = control_left.getValue() + control_right.getValue();
        auto btn = control_left.getButton();
        auto btnr = control_right.getButton();

        if (change != 0) {
            widget->get<T1>(1)->add_value(change);
            *p_value = widget->get<T1>(1)->value;
            on_user_input();
            if (on_changed) on_changed();
        }

        if (btn == ClickEncoder::Clicked || btnr == ClickEncoder::Clicked) {
            leave(now);
            return;
        }

        WidgetMenuItem::on_handle(now);
        widget->draw(&tft);
    }
    virtual void on_enter(unsigned long now) { 
        widget->get<T1>(1)->set_value(*p_value);
        widget->set_border_color(WC_GREEN);
        control_left.getValue();
        control_right.getValue();
        control_left.getButton();
        control_right.getButton();
        last_now = now;
    }
    virtual void on_leave(unsigned long now) {
        widget->set_border_color(WC_SILVER);
        control_left.getValue();
        control_right.getValue();
        control_left.getButton();
        control_right.getButton();
        WidgetMenuItem::on_leave(now);
    }
};

template<class T>
class WidgetSelectionMenuItem : public WidgetMenuItem<Widget> {
    T value;
    T *p_value;
    on_changed_func on_changed;

public:
    WidgetSelectionMenuItem(Widget * widget, T * p_value, T value, on_changed_func on_changed=NULL) : WidgetMenuItem<Widget>(MENU_NONE, widget) { 
        this->value = value; 
        this->p_value = p_value;
        this->on_changed = on_changed;
    }

    virtual void on_enter(unsigned long now) { 
        *p_value = value;
        if (on_changed) on_changed();
        leave(now); 
    }
};


class WidgetInputSelectionMenuItem : public WidgetMenuItem<Widget> {
    uint8_t value;
    input_t *p_value;
public:
    WidgetInputSelectionMenuItem(Widget * widget, input_t * p_value, uint8_t value) : WidgetMenuItem<Widget>(MENU_NONE, widget) { 
        this->value = value; 
        this->p_value = p_value;
    }

    virtual void on_enter(unsigned long now) { 
        p_value->channel = value;
        if (p_value == &settings.master.input)
            input_changed();
        leave(now); 
    }
};

class WidgetToneDefeatSelectionMenuItem : public WidgetMenuItem<Widget> {
    mute_t *p_value;

public:
    WidgetToneDefeatSelectionMenuItem(Widget * widget, mute_t * p_value) : WidgetMenuItem<Widget>(MENU_NONE, widget) { 
        this->p_value = p_value;
    }

    virtual void on_enter(unsigned long now) { 
        p_value->tone ^= 1;
        if (p_value == &settings.master.mute)
            mute_changed();
        leave(now); 
    }
};
class WidgetESSelectionMenuItem : public WidgetMenuItem<Widget> {
    input_t *p_value;

public:
    WidgetESSelectionMenuItem(Widget * widget, input_t * p_value) : WidgetMenuItem<Widget>(MENU_NONE, widget) { 
        this->p_value = p_value;
    }

    virtual void on_enter(unsigned long now) { 
        p_value->es ^= 1;
        if (p_value == &settings.master.input)
            input_changed();
        leave(now); 
    }
};
class WidgetMixedSelectionMenuItem : public WidgetMenuItem<Widget> {
    input_t *p_value;

public:
    WidgetMixedSelectionMenuItem(Widget * widget, input_t * p_value) : WidgetMenuItem<Widget>(MENU_NONE, widget) { 
        this->p_value = p_value;
    }

    virtual void on_enter(unsigned long now) { 
        p_value->mixed ^= 1;
        if (p_value == &settings.master.input)
            input_changed();
        leave(now); 
    }
};


#define SETUP_MAIN_CONTAINER(WIDGET) \
    WIDGET.set_border_color(WC_BLACK); \
    WIDGET.set_size(240, 240);

#define DRAW_MAIN_MENU(WIDGET) \
    WIDGET.set_redraw_all(); \
    WIDGET.draw(&tft);

#endif
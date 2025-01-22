#include "TouchPoint.hpp"

#include "Window.hpp"

TouchPoint::TouchPoint(int id, Window& focus)
    :_id(id)
    ,_focus(&focus)
{}

void TouchPoint::clear_events() {
    _events.clear();
}

void TouchPoint::send_events() {
    if (_focus) {
        _focus->touch_events(_id, _events);
    }

    clear_events();
}
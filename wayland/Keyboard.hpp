#pragma once

#include "WaylandPointer.hpp"
#include "XkbPointer.hpp"

class Display;
class Seat;
class Window;

class Keyboard {
public:
    explicit Keyboard(Seat& seat);
    Keyboard(const Keyboard&) = delete;
    Keyboard(Keyboard&&) noexcept = delete;
    ~Keyboard() = default;

    Keyboard& operator=(const Keyboard&) = delete;
    Keyboard& operator=(Keyboard&&) noexcept = delete;
private:
    Display& _display;
    Window *_focus;

    WaylandPointer<wl_keyboard> _keyboard;
    XkbPointer<xkb_keymap> _keymap;
    XkbPointer<xkb_state> _state;
};

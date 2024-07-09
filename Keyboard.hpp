#pragma once

#include "WaylandPointer.hpp"

class Seat;

class Keyboard {
public:
    explicit Keyboard(Seat& seat);
    Keyboard(const Keyboard&) = delete;
    Keyboard(Keyboard&&) noexcept = delete;
    ~Keyboard() = default;

    Keyboard& operator=(const Keyboard&) = delete;
    Keyboard& operator=(Keyboard&&) noexcept = delete;
private:
    WaylandPointer<wl_keyboard> _keyboard;
};

#pragma once

#include "Keyboard.hpp"
#include "Pointer.hpp"
#include "Touch.hpp"

#include <optional>

class Display;

class Seat {
    friend class Keyboard;
    friend class Pointer;
    friend class Touch;
public:
    Seat(Display& display, wl_seat *seat);
    Seat(const Seat&) = delete;
    Seat(Seat&&) noexcept = delete;
    ~Seat() = default;

    Seat& operator=(const Seat&) = delete;
    Seat& operator=(Seat&&) noexcept = delete;

private:
    Display& _display;
    WaylandPointer<wl_seat> _seat;

    std::optional<Touch> _touch;
    std::optional<Pointer> _pointer;
    std::optional<Keyboard> _keyboard;
};

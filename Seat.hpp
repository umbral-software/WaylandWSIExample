#pragma once

#include "Keyboard.hpp"
#include "Pointer.hpp"

#include <optional>

class Display;

class Seat {
    friend class Keyboard;
    friend class Pointer;
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

    std::optional<Keyboard> _keyboard;
    std::optional<Pointer> _pointer;
};

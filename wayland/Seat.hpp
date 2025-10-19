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
    Seat(Display& display, wl_seat *seat, uint32_t global_name);
    Seat(const Seat&) = delete;
    Seat(Seat&&) noexcept = delete;
    ~Seat() = default;

    Seat& operator=(const Seat&) = delete;
    Seat& operator=(Seat&&) noexcept = delete;

    uint32_t global_name() const noexcept;

private:
    Display& _display;
    WaylandPointer<wl_seat> _seat;
    const uint32_t _name;

    std::optional<Touch> _touch;
    std::optional<Pointer> _pointer;
    std::optional<Keyboard> _keyboard;
};

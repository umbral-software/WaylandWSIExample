#pragma once

#include "WaylandPointer.hpp"

class Seat;

class Pointer {
public:
    explicit Pointer(Seat& seat);
    Pointer(const Pointer&) = delete;
    Pointer(Pointer&&) noexcept = delete;
    ~Pointer() = default;

    Pointer& operator=(const Pointer&) = delete;
    Pointer& operator=(Pointer&&) noexcept = delete;

private:
    Seat& _seat;

    WaylandPointer<wl_pointer> _pointer;
    
    wl_cursor_image *_cursor_image;
    WaylandPointer<wl_surface> _cursor_surface;
};

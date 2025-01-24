#pragma once

#include "events/PointerEvents.hpp"
#include "WaylandPointer.hpp"

#include <vector>

class CursorBase;
class Display;
class EventBase;
class Seat;
class Window;

class Pointer {
public:
    explicit Pointer(Seat& seat);
    Pointer(const Pointer&) = delete;
    Pointer(Pointer&&) noexcept = delete;
    ~Pointer() = default;

    Pointer& operator=(const Pointer&) = delete;
    Pointer& operator=(Pointer&&) noexcept = delete;

private:
    Display& _display;
    Window *_focus;

    WaylandPointer<wl_pointer> _pointer;
    std::unique_ptr<CursorBase> _cursor;
    std::vector<std::unique_ptr<PointerEventBase>> _events;
};

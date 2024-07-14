#pragma once

#include "Seat.hpp"
#include "XkbPointer.hpp"

#include <forward_list>

class Seat;
class Display {
    friend class Keyboard;
    friend class Pointer;
    friend class Seat;
    friend class Window;
public:
    Display();
    Display(const Display&) = delete;
    Display(Display&&) noexcept = delete;
    ~Display() = default;

    Display& operator=(const Display&) = delete;
    Display& operator=(Display&&) noexcept = delete;

    void poll_events();

private:
    WaylandPointer<wl_display> _display;
    WaylandPointer<wl_registry> _registry;

    WaylandPointer<wl_compositor> _compositor;
    WaylandPointer<wl_shm> _shm;
    WaylandPointer<xdg_wm_base> _wm_base;

    std::forward_list<Seat> _seats;

    // Optional protocols
    WaylandPointer<wp_content_type_manager_v1> _content_type_manager; 
    WaylandPointer<zxdg_decoration_manager_v1> _decoration_manager;

    WaylandPointer<wl_cursor_theme> _cursor_theme;
    XkbPointer<xkb_context> _xkb_context;
};

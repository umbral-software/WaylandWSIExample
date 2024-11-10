#pragma once

#include "cursor/CursorManagerBase.hpp"
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
    WaylandPointer<xdg_wm_base> _wm_base;

    std::unique_ptr<CursorManagerBase> _cursor_manager;
    std::forward_list<Seat> _seats;

    // Optional protocols
    WaylandPointer<wl_shm> _shm; // Only needed for wl-cursor theme cursors
    WaylandPointer<wp_content_type_manager_v1> _content_type_manager;
    WaylandPointer<wp_fractional_scale_manager_v1> _fractional_scale_manager;
    WaylandPointer<wp_viewporter> _viewporter;
    WaylandPointer<zxdg_decoration_manager_v1> _decoration_manager;

    XkbPointer<xkb_context> _xkb_context;

    bool _has_fractional_scale;
};

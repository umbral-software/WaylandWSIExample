#pragma once

#include "MemFd.hpp"
#include "WaylandPointer.hpp"

class Display;

class Window {
public:
    explicit Window(Display& display);
    Window(const Window&) = delete;
    Window(Window&&) noexcept = delete;
    ~Window() = default;

    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) noexcept = delete;

    void render();
    bool should_close() const noexcept;

private:
    MemFd _shm_file;

    WaylandPointer<wl_shm_pool> _shm_pool;
    WaylandPointer<wl_buffer> _buffer;
    WaylandPointer<wl_surface> _surface;
    WaylandPointer<wp_viewport> _viewport;
    WaylandPointer<xdg_surface> _wm_surface;
    WaylandPointer<xdg_toplevel> _toplevel;

    // Optional protocols
    WaylandPointer<wp_content_type_v1> _content_type;
    WaylandPointer<zxdg_toplevel_decoration_v1> _toplevel_decoration;

    bool _closed = false;
};

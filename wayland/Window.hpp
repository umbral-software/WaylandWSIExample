#pragma once

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

    void keysym(uint32_t keysym, bool shift, bool ctrl, bool alt) noexcept;
    void text(std::string_view str) const noexcept;

    // Numerator of a fraction with DEFAULT_SCALE_DPI as the denominator
    uint32_t buffer_scale() const noexcept;
    std::pair<uint32_t, uint32_t> buffer_size() const noexcept;

    wl_display *display() noexcept;

    bool should_close() const noexcept;

    wl_surface *surface() noexcept;

public:
    static constexpr uint32_t DEFAULT_SCALE_DPI = 120;

private:
    void toggle_fullscreen() noexcept;

private:
    Display& _display;

    WaylandPointer<wl_surface> _surface;
    WaylandPointer<xdg_surface> _wm_surface;
    WaylandPointer<xdg_toplevel> _toplevel;

    // Optional protocols
    WaylandPointer<wp_content_type_v1> _content_type;
    WaylandPointer<zxdg_toplevel_decoration_v1> _toplevel_decoration;

    bool _closed, _fullscreen, _has_server_decorations;
    int32_t _integer_scale;

    std::pair<uint32_t, uint32_t> _actual_surface_size, _desired_surface_size;
};

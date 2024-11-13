#pragma once

#include "WaylandPointer.hpp"

#include <xkbcommon/xkbcommon.h>

class Display;

class Window {
public:
    explicit Window(Display& display);
    Window(const Window&) = delete;
    Window(Window&&) noexcept = delete;
    ~Window() = default;

    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) noexcept = delete;

    void key_down(xkb_keysym_t keysym) noexcept;
    void key_up(xkb_keysym_t keysym) noexcept;
    void key_modifiers(bool shift, bool ctrl, bool alt) noexcept;
    void pointer_click(uint32_t button, wl_pointer_button_state state) noexcept;
    void pointer_motion(float x, float y) noexcept;
    void text(const std::string& str) const noexcept;

    // Numerator of a fraction with DEFAULT_SCALE_DPI as the denominator
    uint32_t buffer_scale() const noexcept;
    std::pair<uint32_t, uint32_t> buffer_size() const noexcept;

    wl_display *display() noexcept;
    bool should_close() const noexcept;
    wl_surface *surface() noexcept;

    void update_ui();

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
    WaylandPointer<wp_fractional_scale_v1> _fractional_scale;
    WaylandPointer<wp_viewport> _viewport;
    WaylandPointer<zxdg_toplevel_decoration_v1> _toplevel_decoration;

    bool _closed, _fullscreen, _has_server_decorations;
    int32_t _actual_integer_scale, _desired_integer_scale;
    uint32_t _actual_fractional_scale, _desired_fractional_scale;

    std::pair<int32_t, int32_t> _actual_surface_size, _desired_surface_size;
};
